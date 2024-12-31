/*
 * Copyright 2023-2024 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <DeepSea/SceneAnimation/SceneSkinningData.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Math/Core.h>

#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/MaterialType.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/Resources/Texture.h>

#include <DeepSea/SceneAnimation/SceneAnimationTreeNode.h>

#include <string.h>

#define FRAME_DELAY 3

// 256 KB blocks with 4096 nodes.
#define TEXTURE_SIZE 128
#define NODE_ELEMENTS (uint32_t)(sizeof(dsAnimationJointTransform)/sizeof(dsVector4f))
#define MAX_TEXTURE_NODES ((TEXTURE_SIZE*TEXTURE_SIZE)/NODE_ELEMENTS)

typedef enum SkinningMethod
{
	SkinningMethod_Buffers,
	SkinningMethod_BufferTextureCopy,
	SkinningMethod_Textures
} SkinningMethod;

typedef struct BufferInfo
{
	dsGfxBuffer* buffer;
	uint64_t lastUsedFrame;
} BufferInfo;

// Keep within 32 bytes for cache friendliness.
typedef struct InstanceData
{
	const dsAnimationTree* animationTree;
	union
	{
		struct
		{
			size_t offset;
			size_t size;
		};
		dsVector2f instanceOffsetStep;
	};
	dsTexture* texture;
} InstanceData;

typedef struct dsSceneSkinningData
{
	dsSceneInstanceData instanceData;

	dsAllocator* resourceAllocator;
	dsResourceManager* resourceManager;
	dsGfxFormat format;
	dsGfxBufferUsage bufferUsage;
	SkinningMethod skinningMethod;
	uint32_t skinningDataVar;
	uint32_t skinningTextureInfoVar;
	size_t textureSize;

	BufferInfo* buffers;
	uint32_t bufferCount;
	uint32_t maxBuffers;

	dsTexture** textures;
	uint32_t textureCount;
	uint32_t maxTextures;

	dsGfxBuffer* curBuffer;
	dsAnimationJointTransform* tempTextureData;
	dsShaderVariableGroupDesc* fallbackTextureInfoDesc;
	dsShaderVariableGroup* fallbackTextureInfo;

	InstanceData* instances;
	uint32_t instanceCount;
	uint32_t maxInstances;
} dsSceneSkinningData;

static dsShaderVariableElement textureInfoElements[] =
{
	{"instanceOffsetStep", dsMaterialType_Vec2, 0}
};

static dsGfxBuffer* getBuffer(dsSceneInstanceData* instanceData, size_t requestedSize)
{
	dsSceneSkinningData* skinningData = (dsSceneSkinningData*)instanceData;
	dsResourceManager* resourceManager = skinningData->resourceManager;
	uint64_t frameNumber = resourceManager->renderer->frameNumber;

	// Look for any buffer large enough that's FRAME_DELAY number of frames earlier than the current
	// one.
	dsGfxBuffer* foundBuffer = NULL;
	for (uint32_t i = 0; i < skinningData->bufferCount;)
	{
		BufferInfo* bufferInfo = skinningData->buffers + i;

		// Skip over all buffers that are still in use, even if too small.
		if (bufferInfo->lastUsedFrame + FRAME_DELAY > frameNumber)
		{
			++i;
			continue;
		}

		if (bufferInfo->buffer->size >= requestedSize)
		{
			// Found. Only take the first one, and continue so that smaller buffers can be removed.
			if (!foundBuffer)
			{
				bufferInfo->lastUsedFrame = frameNumber;
				foundBuffer = bufferInfo->buffer;
			}
			++i;
			continue;
		}

		// This buffer is too small. Delete it now since a new one will need to be allocated.
		if (!dsGfxBuffer_destroy(bufferInfo->buffer))
			return NULL;

		// Constant-time removal since order doesn't matter.
		*bufferInfo = skinningData->buffers[--skinningData->bufferCount];
	}

	if (foundBuffer)
		return foundBuffer;

	// Create a new buffer if no suitable one has beenf ound.
	uint32_t index = skinningData->bufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(instanceData->allocator, skinningData->buffers,
			skinningData->bufferCount, skinningData->maxBuffers, 1))
	{
		return NULL;
	}

	BufferInfo* bufferInfo = skinningData->buffers + index;
	bufferInfo->buffer = dsGfxBuffer_create(resourceManager, skinningData->resourceAllocator,
		skinningData->bufferUsage, dsGfxMemory_Stream | dsGfxMemory_Synchronize, NULL,
		requestedSize);
	if (!bufferInfo->buffer)
	{
		--skinningData->bufferCount;
		return NULL;
	}

	bufferInfo->lastUsedFrame = frameNumber;
	return bufferInfo->buffer;
}

static bool populateBufferData(dsSceneInstanceData* instanceData)
{
	dsSceneSkinningData* skinningData = (dsSceneSkinningData*)instanceData;
	uint32_t alignment = skinningData->resourceManager->minUniformBufferAlignment;
	size_t bufferSize = 0;
	for (uint32_t i = 0; i < skinningData->instanceCount; ++i)
	{
		const dsAnimationTree* animationTree = skinningData->instances[i].animationTree;
		if (animationTree)
		{
			bufferSize += DS_CUSTOM_ALIGNED_SIZE(
				animationTree->nodeCount*sizeof(dsAnimationJointTransform), alignment);
		}
	}

	dsGfxBuffer* buffer = getBuffer(instanceData, bufferSize);
	if (!buffer)
		return false;

	uint8_t* bufferData =
		(uint8_t*)dsGfxBuffer_map(buffer, dsGfxBufferMap_Write, 0, DS_MAP_FULL_BUFFER);
	if (!bufferData)
		return false;

	skinningData->curBuffer = buffer;
	size_t offset = 0;
	for (uint32_t i = 0; i < skinningData->instanceCount; ++i)
	{
		InstanceData* instance = skinningData->instances + i;
		const dsAnimationTree* animationTree = instance->animationTree;
		if (!animationTree)
			continue;

		size_t copySize = animationTree->nodeCount*sizeof(dsAnimationJointTransform);
		instance->offset = offset;
		instance->size = copySize;
		memcpy(bufferData + offset, animationTree->jointTransforms, copySize);
		offset += DS_CUSTOM_ALIGNED_SIZE(copySize, alignment);
	}

	DS_VERIFY(dsGfxBuffer_unmap(buffer));
	return true;
}

static uint32_t countTextures(dsSceneInstanceData* instanceData)
{
	dsSceneSkinningData* skinningData = (dsSceneSkinningData*)instanceData;
	// Will only be called if we have at least one texture.
	uint32_t textureCount = 1;
	uint32_t curTextureNodes = 0;
	float step = 1.0f/(float)TEXTURE_SIZE;
	for (uint32_t i = 0; i < skinningData->instanceCount; ++i)
	{
		InstanceData* instance = skinningData->instances + i;
		const dsAnimationTree* animationTree = instance->animationTree;
		if (!animationTree)
			continue;

		uint32_t startOffset;
		if (curTextureNodes + animationTree->nodeCount > MAX_TEXTURE_NODES)
		{
			++textureCount;
			startOffset = 0;
			curTextureNodes = animationTree->nodeCount;
		}
		else
		{
			startOffset = curTextureNodes;
			curTextureNodes += animationTree->nodeCount;
		}

		instance->instanceOffsetStep.x = (float)startOffset*NODE_ELEMENTS*step;
		instance->instanceOffsetStep.y = step;
	}

	return textureCount;
}

static bool createTextures(dsSceneInstanceData* instanceData, uint32_t textureCount)
{
	dsSceneSkinningData* skinningData = (dsSceneSkinningData*)instanceData;
	if (skinningData->textureCount >= textureCount)
		return true;

	uint32_t startIndex = skinningData->textureCount;
	uint32_t addCount = textureCount - startIndex;
	if (!DS_RESIZEABLE_ARRAY_ADD(instanceData->allocator, skinningData->textures,
			skinningData->textureCount, skinningData->maxTextures, addCount))
	{
		return false;
	}

	dsTextureInfo textureInfo =
	{
		skinningData->format, dsTextureDim_2D, TEXTURE_SIZE, TEXTURE_SIZE, 0, 1, 0
	};
	for (uint32_t i = startIndex; i < skinningData->textureCount; ++i)
	{
		skinningData->textures[i] = dsTexture_create(skinningData->resourceManager,
			skinningData->resourceAllocator, dsTextureUsage_Texture | dsTextureUsage_CopyTo,
			dsGfxMemory_Stream | dsGfxMemory_GPUOnly, &textureInfo, NULL, 0);
		if (!skinningData->textures[i])
		{
			skinningData->textureCount = i;
			return false;
		}
	}

	return true;
}

static void populateTextureInfoData(dsSceneInstanceData* instanceData, uint8_t* bufferData,
	uint32_t stride)
{
	dsSceneSkinningData* skinningData = (dsSceneSkinningData*)instanceData;
	size_t offset = 0;
	for (uint32_t i = 0; i < skinningData->instanceCount; ++i)
	{
		InstanceData* instance = skinningData->instances + i;
		if (!instance->animationTree)
			continue;

		*((dsVector2f*)bufferData) = instance->instanceOffsetStep;
		instance->offset = offset;
		offset += stride;
		bufferData += stride;
	}
}

static bool populateBufferTextureCopyData(dsSceneInstanceData* instanceData,
	dsCommandBuffer* commandBuffer, uint32_t usedInstanceCount)
{
	dsSceneSkinningData* skinningData = (dsSceneSkinningData*)instanceData;
	dsResourceManager* resourceManager = skinningData->resourceManager;
	uint32_t textureCount = countTextures(instanceData);
	if (!createTextures(instanceData, textureCount))
		return false;

	// Minimum size for a single element.
	uint32_t textureInfoStride =
		dsMax((uint32_t)dsMaterialType_blockSize(dsMaterialType_Vec2, false),
		resourceManager->minUniformBlockAlignment);
	size_t bufferSize = textureCount*skinningData->textureSize;
	if (!skinningData->fallbackTextureInfo)
		bufferSize += textureInfoStride*usedInstanceCount;

	dsGfxBuffer* buffer = getBuffer(instanceData, bufferSize);
	if (!buffer)
		return false;

	uint8_t* bufferData =
		(uint8_t*)dsGfxBuffer_map(buffer, dsGfxBufferMap_Write, 0, DS_MAP_FULL_BUFFER);
	if (!bufferData)
		return false;

	size_t bufferOffset = 0;
	skinningData->curBuffer = buffer;
	if (!skinningData->fallbackTextureInfo)
	{
		populateTextureInfoData(instanceData, bufferData, textureInfoStride);
		bufferOffset = textureInfoStride*usedInstanceCount;
		bufferData += bufferOffset;
	}

	uint32_t curTexture = 0;
	uint32_t curTextureNodes = 0;
	for (uint32_t i = 0; i < skinningData->instanceCount; ++i)
	{
		InstanceData* instance = skinningData->instances + i;
		const dsAnimationTree* animationTree = instance->animationTree;
		if (!animationTree)
			continue;

		size_t startOffset;
		if (curTextureNodes + animationTree->nodeCount > MAX_TEXTURE_NODES)
		{
			dsGfxBufferTextureCopyRegion region =
			{
				bufferOffset, 0, 0, {dsCubeFace_None, 0, 0, 0, 0}, TEXTURE_SIZE, TEXTURE_SIZE, 1
			};
			if (!dsGfxBuffer_copyToTexture(commandBuffer, buffer,
					skinningData->textures[curTexture], &region, 1))
			{
				dsGfxBuffer_unmap(buffer);
				return false;
			}
			++curTexture;
			bufferOffset += skinningData->textureSize;
			bufferData += skinningData->textureSize;
			startOffset = 0;
			curTextureNodes = animationTree->nodeCount;
		}
		else
		{
			startOffset = curTextureNodes*sizeof(dsAnimationJointTransform);
			curTextureNodes += animationTree->nodeCount;
		}

		instance->texture = skinningData->textures[curTexture];
		memcpy(bufferData + startOffset, animationTree->jointTransforms,
			animationTree->nodeCount*sizeof(dsAnimationJointTransform));
	}

	DS_ASSERT(curTexture == skinningData->textureCount - 1);
	dsGfxBufferTextureCopyRegion region =
	{
		bufferOffset, 0, 0, {dsCubeFace_None, 0, 0, 0, 0}, TEXTURE_SIZE, TEXTURE_SIZE, 1
	};
	bool success = dsGfxBuffer_copyToTexture(
		commandBuffer, buffer, skinningData->textures[curTexture], &region, 1);
	return dsGfxBuffer_unmap(buffer) && success;
}

static bool populateTextureData(dsSceneInstanceData* instanceData,
	dsCommandBuffer* commandBuffer, uint32_t usedInstanceCount)
{
	dsSceneSkinningData* skinningData = (dsSceneSkinningData*)instanceData;
	dsResourceManager* resourceManager = skinningData->resourceManager;
	uint32_t textureCount = countTextures(instanceData);
	if (!createTextures(instanceData, textureCount))
		return false;

	// Minimum size for a single element.
	if (!skinningData->fallbackTextureInfo)
	{
		uint32_t textureInfoStride =
			dsMax((uint32_t)sizeof(dsVector4f), resourceManager->minUniformBlockAlignment);
		size_t bufferSize = textureInfoStride*usedInstanceCount;
		dsGfxBuffer* buffer = getBuffer(instanceData, bufferSize);
		if (!buffer)
			return false;

		uint8_t* bufferData =
			(uint8_t*)dsGfxBuffer_map(buffer, dsGfxBufferMap_Write, 0, DS_MAP_FULL_BUFFER);
		if (!bufferData)
			return false;

		skinningData->curBuffer = buffer;
		populateTextureInfoData(instanceData, bufferData, textureInfoStride);
		if (!dsGfxBuffer_unmap(buffer))
			return false;
	}

	uint32_t curTexture = 0;
	uint32_t curTextureNodes = 0;
	for (uint32_t i = 0; i < skinningData->instanceCount; ++i)
	{
		InstanceData* instance = skinningData->instances + i;
		const dsAnimationTree* animationTree = instance->animationTree;
		if (!animationTree)
			continue;

		uint32_t startOffset;
		if (curTextureNodes + animationTree->nodeCount > MAX_TEXTURE_NODES)
		{
			dsTexturePosition position = {dsCubeFace_None, 0, 0, 0, 0};
			if (!dsTexture_copyData(skinningData->textures[curTexture], commandBuffer, &position,
					TEXTURE_SIZE, TEXTURE_SIZE, 1, skinningData->tempTextureData,
					skinningData->textureSize))
			{
				return false;
			}
			++curTexture;
			startOffset = 0;
			curTextureNodes = animationTree->nodeCount;
		}
		else
		{
			startOffset = curTextureNodes;
			curTextureNodes += animationTree->nodeCount;
		}

		instance->texture = skinningData->textures[curTexture];
		memcpy(skinningData->tempTextureData + startOffset, animationTree->jointTransforms,
			animationTree->nodeCount*sizeof(dsAnimationJointTransform));
	}

	DS_ASSERT(curTexture == skinningData->textureCount - 1);
	dsTexturePosition position = {dsCubeFace_None, 0, 0, 0, 0};
	return dsTexture_copyData(skinningData->textures[curTexture], commandBuffer, &position,
		TEXTURE_SIZE, TEXTURE_SIZE, 1, skinningData->tempTextureData, skinningData->textureSize);
}

static bool dsSceneSkinningData_populateData(dsSceneInstanceData* instanceData, const dsView* view,
	dsCommandBuffer* commandBuffer, const dsSceneTreeNode* const* instances, uint32_t instanceCount)
{
	DS_UNUSED(view);
	dsSceneSkinningData* skinningData = (dsSceneSkinningData*)instanceData;
	DS_PROFILE_FUNC_START();

	// First get the initial instance data with the valid animation trees.
	if (skinningData->instanceCount != 0)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_SCENE_ANIMATION_LOG_TAG, "Attempting to populate scene skinning data before "
			"calling dsSceneInstanceData_finish() for the last usage.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!DS_RESIZEABLE_ARRAY_ADD(instanceData->allocator, skinningData->instances,
			skinningData->instanceCount, skinningData->maxInstances, instanceCount))
	{
		DS_PROFILE_FUNC_RETURN(false);
	}

	uint32_t usedInstances = 0;
	for (uint32_t i = 0; i < instanceCount; ++i)
	{
		InstanceData* instance = skinningData->instances + i;
		const dsAnimationTree* animationTree =
			dsSceneAnimationTreeNode_getAnimationTreeForInstance(instances[i]);
		if (animationTree && animationTree->jointTransforms)
		{
			instance->animationTree = animationTree;
			++usedInstances;
			if (animationTree->nodeCount > MAX_TEXTURE_NODES)
			{
				errno = EPERM;
				DS_LOG_ERROR_F(DS_SCENE_ANIMATION_LOG_TAG,
					"Animation tree has %u nodes, more than the maximum of %u nodes.",
					animationTree->nodeCount, MAX_TEXTURE_NODES);
				DS_PROFILE_FUNC_RETURN(false);
			}
		}
		else
			instance->animationTree = NULL;
		instance->offset = 0;
		instance->size = 0;
		instance->texture = NULL;
	}

	if (usedInstances == 0)
		DS_PROFILE_FUNC_RETURN(true);

	bool success = false;
	switch (skinningData->skinningMethod)
	{
		case SkinningMethod_Buffers:
			success = populateBufferData(instanceData);
			break;
		case SkinningMethod_BufferTextureCopy:
			DS_ASSERT(commandBuffer);
			success = populateBufferTextureCopyData(instanceData, commandBuffer, usedInstances);
			break;
		case SkinningMethod_Textures:
			DS_ASSERT(commandBuffer);
			success = populateTextureData(instanceData, commandBuffer, usedInstances);
			break;
	}

	DS_PROFILE_FUNC_RETURN(success);
}

static bool dsSceneSkinningData_bindInstance(dsSceneInstanceData* instanceData, uint32_t index,
	dsSharedMaterialValues* values)
{
	dsSceneSkinningData* skinningData = (dsSceneSkinningData*)instanceData;
	if (index >= skinningData->instanceCount)
	{
		errno = EINDEX;
		return false;
	}

	InstanceData* instance = skinningData->instances + index;
	// Don't error out of the instance doesn't have any skinning info.
	if (!instance->animationTree)
		return true;

	if (skinningData->skinningMethod == SkinningMethod_Buffers)
	{
		return dsSharedMaterialValues_setBufferID(values, skinningData->skinningDataVar,
			skinningData->curBuffer, instance->offset, instance->size);
	}

	if (skinningData->fallbackTextureInfo)
	{
		DS_VERIFY(dsShaderVariableGroup_setElementData(skinningData->fallbackTextureInfo, 0,
			&instance->instanceOffsetStep, dsMaterialType_Vec2, 0, 1));
		DS_VERIFY(dsShaderVariableGroup_commitWithoutBuffer(skinningData->fallbackTextureInfo));
		if (!dsSharedMaterialValues_setVariableGroupID(values, skinningData->skinningTextureInfoVar,
				skinningData->fallbackTextureInfo))
		{
			return false;
		}
	}
	else if (!dsSharedMaterialValues_setBufferID(values, skinningData->skinningTextureInfoVar,
		skinningData->curBuffer, instance->offset,
		dsMaterialType_blockSize(dsMaterialType_Vec2, false)))
	{
		return false;
	}
	return dsSharedMaterialValues_setTextureID(values, skinningData->skinningDataVar,
		instance->texture);
}

static bool dsSceneSkinningData_finish(dsSceneInstanceData* instanceData)
{
	dsSceneSkinningData* skinningData = (dsSceneSkinningData*)instanceData;
	skinningData->curBuffer = NULL;
	skinningData->instanceCount = 0;
	return true;
}

static bool dsSceneSkinningData_destroy(dsSceneInstanceData* instanceData)
{
	dsSceneSkinningData* skinningData = (dsSceneSkinningData*)instanceData;

	for (uint32_t i = 0; i < skinningData->bufferCount; ++i)
	{
		if (!dsGfxBuffer_destroy(skinningData->buffers[i].buffer))
		{
			DS_ASSERT(i == 0);
			return false;
		}
	}
	DS_VERIFY(dsAllocator_free(instanceData->allocator, skinningData->buffers));

	for (uint32_t i = 0; i < skinningData->textureCount; ++i)
	{
		if (!dsTexture_destroy(skinningData->textures[i]))
			return false;
	}
	DS_VERIFY(dsAllocator_free(instanceData->allocator, skinningData->textures));

	DS_VERIFY(dsAllocator_free(instanceData->allocator, skinningData->tempTextureData));
	DS_VERIFY(dsShaderVariableGroup_destroy(skinningData->fallbackTextureInfo));
	DS_VERIFY(dsShaderVariableGroupDesc_destroy(skinningData->fallbackTextureInfoDesc));

	DS_VERIFY(dsAllocator_free(instanceData->allocator, skinningData->instances));
	DS_VERIFY(dsAllocator_free(instanceData->allocator, instanceData));
	return true;
}

const char* const dsSceneSkinningData_typeName = "SkinningData";

bool dsSceneSkinningData_useBuffers(dsResourceManager* resourceManager)
{
	return resourceManager && (resourceManager->supportedBuffers & dsGfxBufferUsage_UniformBuffer);
}

dsShaderVariableGroupDesc* dsSceneSkinningData_createTextureInfoShaderVariableGroupDesc(
	dsResourceManager* resourceManager, dsAllocator* allocator)
{
	if (!resourceManager)
	{
		errno = EINVAL;
		return NULL;
	}

	return dsShaderVariableGroupDesc_create(resourceManager, allocator, textureInfoElements,
		DS_ARRAY_SIZE(textureInfoElements));
}

bool dsSceneSkinningData_isTextureInfoShaderVariableGroupCompatible(
	const dsShaderVariableGroupDesc* textureInfoDesc)
{
	return textureInfoDesc &&
		dsShaderVariableGroup_areElementsEqual(textureInfoElements,
			DS_ARRAY_SIZE(textureInfoElements),
			textureInfoDesc->elements, textureInfoDesc->elementCount);
}

dsSceneInstanceData* dsSceneSkinningData_create(dsAllocator* allocator,
	dsAllocator* resourceAllocator, dsResourceManager* resourceManager)
{
	if (!allocator || !resourceManager)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_ANIMATION_LOG_TAG,
			"Skinning data allocator must support freeing memory.");
		return NULL;
	}

	dsSceneSkinningData* skinningData = DS_ALLOCATE_OBJECT(allocator, dsSceneSkinningData);
	if (!skinningData)
		return NULL;

	bool useBuffers = dsSceneSkinningData_useBuffers(resourceManager);
	bool shaderVariableGroupBuffers = dsShaderVariableGroup_useGfxBuffer(resourceManager);

	dsSceneInstanceData* instanceData = (dsSceneInstanceData*)skinningData;
	instanceData->allocator = dsAllocator_keepPointer(allocator);
	if (useBuffers || !shaderVariableGroupBuffers)
		instanceData->valueCount = 1;
	else
		instanceData->valueCount = 2;
	instanceData->needsCommandBuffer = !useBuffers;
	instanceData->populateDataFunc = &dsSceneSkinningData_populateData;
	instanceData->bindInstanceFunc = &dsSceneSkinningData_bindInstance;
	instanceData->finishFunc = &dsSceneSkinningData_finish;
	instanceData->destroyFunc = &dsSceneSkinningData_destroy;

	skinningData->resourceAllocator = resourceAllocator ? resourceAllocator : allocator;
	skinningData->resourceManager = resourceManager;
	skinningData->format = dsGfxFormat_decorate(dsGfxFormat_R32G32B32A32, dsGfxFormat_Float);
	if (useBuffers)
	{
		skinningData->bufferUsage = dsGfxBufferUsage_UniformBuffer;
		skinningData->skinningMethod = SkinningMethod_Buffers;
	}
	else if (dsGfxFormat_copyBufferToTextureSupported(resourceManager, skinningData->format))
	{
		skinningData->bufferUsage = dsGfxBufferUsage_CopyFrom;
		if (shaderVariableGroupBuffers)
			skinningData->bufferUsage |= dsGfxBufferUsage_UniformBlock;
		skinningData->skinningMethod = SkinningMethod_BufferTextureCopy;
	}
	else
	{
		skinningData->bufferUsage = shaderVariableGroupBuffers ? dsGfxBufferUsage_UniformBlock : 0;
		skinningData->skinningMethod = SkinningMethod_Textures;
	}
	skinningData->skinningDataVar = dsUniqueNameID_create(dsSceneSkinningData_typeName);
	skinningData->skinningTextureInfoVar =
		useBuffers ? 0 : dsUniqueNameID_create("SkinningTextureInfo");
	dsTextureInfo textureInfo =
	{
		skinningData->format, dsTextureDim_2D, TEXTURE_SIZE, TEXTURE_SIZE, 0, 1, 0
	};
	skinningData->textureSize = dsTexture_size(&textureInfo);

	skinningData->buffers = NULL;
	skinningData->bufferCount = 0;
	skinningData->maxBuffers = 0;

	skinningData->curBuffer = NULL;
	skinningData->tempTextureData = NULL;
	skinningData->fallbackTextureInfoDesc = NULL;
	skinningData->fallbackTextureInfo = NULL;

	skinningData->textures = NULL;
	skinningData->textureCount = 0;
	skinningData->maxTextures = 0;

	skinningData->instances = NULL;
	skinningData->instanceCount = 0;
	skinningData->maxInstances = 0;

	if (skinningData->skinningMethod == SkinningMethod_Textures)
	{
		skinningData->tempTextureData = (dsAnimationJointTransform*)dsAllocator_alloc(allocator,
			skinningData->textureSize);
		if (!skinningData->tempTextureData)
		{
			dsSceneSkinningData_destroy(instanceData);
			return NULL;
		}
	}

	if (skinningData->skinningMethod != SkinningMethod_Buffers && !shaderVariableGroupBuffers)
	{
		skinningData->fallbackTextureInfoDesc =
			dsSceneSkinningData_createTextureInfoShaderVariableGroupDesc(resourceManager,
				allocator);
		if (!skinningData->fallbackTextureInfoDesc)
		{
			dsSceneSkinningData_destroy(instanceData);
			return NULL;
		}

		skinningData->fallbackTextureInfo = dsShaderVariableGroup_create(
			resourceManager, allocator, NULL, skinningData->fallbackTextureInfoDesc);
		if (!skinningData->fallbackTextureInfo)
		{
			dsSceneSkinningData_destroy(instanceData);
			return NULL;
		}
	}

	return instanceData;
}

/*
 * Copyright 2023-2026 Aaron Barany
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
#include <DeepSea/Render/Resources/StreamingGfxBufferList.h>
#include <DeepSea/Render/Resources/Texture.h>

#include <DeepSea/SceneAnimation/SceneAnimationTreeNode.h>

#include <string.h>

// 256 KB blocks with 4096 nodes.
#define TEXTURE_SIZE 128
#define NODE_ELEMENTS (uint32_t)(sizeof(dsAnimationJointTransform)/sizeof(dsVector4f))
#define MAX_TEXTURE_NODES ((TEXTURE_SIZE*TEXTURE_SIZE)/NODE_ELEMENTS)

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
	dsMaterialType skinningMethod;
	uint32_t skinningDataVar;
	uint32_t skinningTextureInfoVar;
	uint32_t textureInfoStride;
	size_t textureSize;

	BufferInfo* buffers;
	uint32_t bufferCount;
	uint32_t maxBuffers;

	dsTexture** textures;
	uint32_t textureCount;
	uint32_t maxTextures;

	BufferInfo* textureBuffers;
	uint32_t textureBufferCount;
	uint32_t maxTextureBuffers;

	dsGfxBuffer* curBuffer;
	dsGfxBuffer* curTextureBuffer;
	dsAnimationJointTransform* tempTextureData;
	const dsShaderVariableGroupDesc* fallbackTextureInfoDesc;
	dsShaderVariableGroup* fallbackTextureInfo;

	InstanceData* instances;
	uint32_t instanceCount;
	uint32_t maxInstances;
} dsSceneSkinningData;

static dsShaderVariableElement textureBufferInfoElements[] =
{
	{"offset", dsMaterialType_UInt, 0}
};

static dsShaderVariableElement textureInfoElements[] =
{
	{"instanceOffsetStep", dsMaterialType_Vec2, 0}
};

static dsGfxBuffer* getBuffer(dsSceneSkinningData* skinningData, size_t requestedSize)
{
	dsSceneInstanceData* instanceData = (dsSceneInstanceData*)skinningData;
	dsResourceManager* resourceManager = skinningData->resourceManager;
	uint64_t frameNumber = resourceManager->renderer->frameNumber;

	// Look for an existing buffer we can re-use.
	uint32_t index = dsStreamingGfxBufferList_findNext(skinningData->buffers,
		&skinningData->bufferCount, sizeof(BufferInfo), offsetof(BufferInfo, buffer),
		offsetof(BufferInfo, lastUsedFrame), NULL, requestedSize,
		DS_DEFAULT_STREAMING_GFX_BUFFER_FRAME_DELAY, frameNumber);
	if (index != DS_NO_STREAMING_GFX_BUFFER)
		return skinningData->buffers[index].buffer;

	// Create a new buffer if no suitable one has beenf ound.
	index = skinningData->bufferCount;
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

static dsGfxBuffer* getTextureBuffer(dsSceneSkinningData* skinningData, size_t requestedSize)
{
	dsSceneInstanceData* instanceData = (dsSceneInstanceData*)skinningData;
	dsResourceManager* resourceManager = skinningData->resourceManager;
	uint64_t frameNumber = resourceManager->renderer->frameNumber;

	// Look for an existing buffer we can re-use.
	uint32_t index = dsStreamingGfxBufferList_findNext(skinningData->textureBuffers,
		&skinningData->textureBufferCount, sizeof(BufferInfo), offsetof(BufferInfo, buffer),
		offsetof(BufferInfo, lastUsedFrame), NULL, requestedSize,
		DS_DEFAULT_STREAMING_GFX_BUFFER_FRAME_DELAY, frameNumber);
	if (index != DS_NO_STREAMING_GFX_BUFFER)
		return skinningData->textureBuffers[index].buffer;

	// Create a new buffer if no suitable one has beenf ound.
	index = skinningData->textureBufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(instanceData->allocator, skinningData->textureBuffers,
			skinningData->textureBufferCount, skinningData->maxTextureBuffers, 1))
	{
		return NULL;
	}

	BufferInfo* bufferInfo = skinningData->textureBuffers + index;
	bufferInfo->buffer = dsGfxBuffer_create(resourceManager, skinningData->resourceAllocator,
		dsGfxBufferUsage_Texture, dsGfxMemory_Stream | dsGfxMemory_Synchronize, NULL,
		requestedSize);
	if (!bufferInfo->buffer)
	{
		--skinningData->bufferCount;
		return NULL;
	}

	bufferInfo->lastUsedFrame = frameNumber;
	return bufferInfo->buffer;
}

static bool populateBufferData(dsSceneSkinningData* skinningData)
{
	uint32_t alignment = skinningData->resourceManager->minUniformBufferAlignment;
	size_t bufferSize = 0;
	for (uint32_t i = 0; i < skinningData->instanceCount; ++i)
	{
		const dsAnimationTree* animationTree = skinningData->instances[i].animationTree;
		if (animationTree &&
			!dsAddAlignedArraySize(&bufferSize, sizeof(dsAnimationJointTransform),
				animationTree->nodeCount, alignment))
		{
			return false;
		}
	}

	dsGfxBuffer* buffer = getBuffer(skinningData, bufferSize);
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
		offset += DS_ALIGNED_SIZE(copySize, alignment);
	}

	DS_VERIFY(dsGfxBuffer_unmap(buffer));
	return true;
}

static bool populateTextureBufferInfo(dsSceneSkinningData* skinningData, uint32_t usedInstanceCount)
{
	size_t offsetStride = skinningData->textureInfoStride;
	size_t bufferSize = offsetStride*usedInstanceCount;

	dsGfxBuffer* buffer = getBuffer(skinningData, bufferSize);
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
		if (!instance->animationTree)
			continue;

		*(uint32_t*)(bufferData + offset) = (uint32_t)instance->offset;
		instance->offset = offset;
		offset += offsetStride;
	}

	DS_VERIFY(dsGfxBuffer_unmap(buffer));
	return true;
}

static bool populateTextureBufferData(dsSceneSkinningData* skinningData, uint32_t usedInstanceCount)
{
	const dsResourceManager* resourceManager = skinningData->resourceManager;
	size_t bufferSize = 0;
	for (uint32_t i = 0; i < skinningData->instanceCount; ++i)
	{
		const dsAnimationTree* animationTree = skinningData->instances[i].animationTree;
		if (animationTree &&
			!dsAddAlignedArraySize(&bufferSize, sizeof(dsAnimationJointTransform),
				animationTree->nodeCount, sizeof(dsVector4f)))
		{
			return false;
		}
	}

	bufferSize = DS_ALIGNED_SIZE(bufferSize, resourceManager->minTextureBufferAlignment);
	dsGfxBuffer* buffer = getTextureBuffer(skinningData, bufferSize);
	if (!buffer)
		return false;

	uint8_t* bufferData =
		(uint8_t*)dsGfxBuffer_map(buffer, dsGfxBufferMap_Write, 0, DS_MAP_FULL_BUFFER);
	if (!bufferData)
		return false;

	skinningData->curTextureBuffer = buffer;
	size_t offset = 0;
	for (uint32_t i = 0; i < skinningData->instanceCount; ++i)
	{
		InstanceData* instance = skinningData->instances + i;
		const dsAnimationTree* animationTree = instance->animationTree;
		if (!animationTree)
			continue;

		size_t thisSize = animationTree->nodeCount*sizeof(dsAnimationJointTransform);
		size_t elementOffset = offset/sizeof(dsVector4f);
		if (elementOffset > UINT32_MAX)
		{
			errno = ERANGE;
			return false;
		}

		instance->offset = elementOffset;
		instance->size = thisSize/sizeof(dsVector4f);
		memcpy(bufferData + offset, animationTree->jointTransforms, thisSize);
		offset += thisSize;
	}

	DS_VERIFY(dsGfxBuffer_unmap(buffer));
	return skinningData->fallbackTextureInfo ||
		populateTextureBufferInfo(skinningData, usedInstanceCount);
}

static uint32_t countTextures(dsSceneSkinningData* skinningData)
{
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

static bool createTextures(dsSceneSkinningData* skinningData, uint32_t textureCount)
{
	dsSceneInstanceData* instanceData = (dsSceneInstanceData*)skinningData;
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

static bool populateTextureInfo(dsSceneSkinningData* skinningData, uint32_t usedInstanceCount)
{
	uint32_t textureInfoStride = skinningData->textureInfoStride;
	size_t bufferSize = textureInfoStride*usedInstanceCount;
	dsGfxBuffer* buffer = getBuffer(skinningData, bufferSize);
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
		if (!instance->animationTree)
			continue;

		*((dsVector2f*)(bufferData + offset)) = instance->instanceOffsetStep;
		instance->offset = offset;
		offset += textureInfoStride;
	}

	DS_VERIFY(dsGfxBuffer_unmap(buffer));
	return true;
}

static bool populateTextureData(dsSceneSkinningData* skinningData,
	dsCommandBuffer* commandBuffer, uint32_t usedInstanceCount)
{
	uint32_t textureCount = countTextures(skinningData);
	if (!createTextures(skinningData, textureCount))
		return false;

	if (!skinningData->fallbackTextureInfo && !populateTextureInfo(skinningData, usedInstanceCount))
		return false;

	uint32_t curTexture = 0;
	uint32_t curTextureNodes = 0;
	dsTexturePosition position = {dsCubeFace_None, 0, 0, 0, 0};
	for (uint32_t i = 0; i < skinningData->instanceCount; ++i)
	{
		InstanceData* instance = skinningData->instances + i;
		const dsAnimationTree* animationTree = instance->animationTree;
		if (!animationTree)
			continue;

		uint32_t startOffset;
		if (curTextureNodes + animationTree->nodeCount > MAX_TEXTURE_NODES)
		{
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
	return dsTexture_copyData(skinningData->textures[curTexture], commandBuffer, &position,
		TEXTURE_SIZE, TEXTURE_SIZE, 1, skinningData->tempTextureData, skinningData->textureSize);
}

static bool dsSceneSkinningData_populateData(dsSceneInstanceData* instanceData, const dsView* view,
	dsCommandBuffer* commandBuffer,  const dsViewRenderPassParams* renderPassParams,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount)
{
	DS_UNUSED(view);
	DS_UNUSED(renderPassParams);
	dsSceneSkinningData* skinningData = (dsSceneSkinningData*)instanceData;
	DS_PROFILE_FUNC_START();

	// First get the initial instance data with the valid animation trees.
	if (skinningData->instanceCount != 0)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_SCENE_ANIMATION_LOG_TAG, "Attempting to populate scene skinning data "
			"before calling dsSceneInstanceData_finish() for the last usage.");
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
		case dsMaterialType_UniformBuffer:
			success = populateBufferData(skinningData);
			break;
		case dsMaterialType_TextureBuffer:
			success = populateTextureBufferData(skinningData, usedInstances);
			break;
		case dsMaterialType_Texture:
			DS_ASSERT(commandBuffer);
			success = populateTextureData(skinningData, commandBuffer, usedInstances);
			break;
		default:
			DS_ASSERT(false);
			break;
	}

	DS_PROFILE_FUNC_RETURN(success);
}

static bool dsSceneSkinningData_bindInstance(
	dsSceneInstanceData* instanceData, uint32_t index, dsSharedMaterialValues* values)
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

	switch (skinningData->skinningMethod)
	{
		case dsMaterialType_UniformBuffer:
			DS_ASSERT(skinningData->curBuffer);
			return dsSharedMaterialValues_setBufferID(values, skinningData->skinningDataVar,
				skinningData->curBuffer, instance->offset, instance->size);
		case dsMaterialType_TextureBuffer:
			DS_ASSERT(skinningData->fallbackTextureInfo || skinningData->curBuffer);
			if (skinningData->fallbackTextureInfo)
			{
				DS_VERIFY(dsShaderVariableGroup_setElementData(skinningData->fallbackTextureInfo, 0,
					&instance->offset, dsMaterialType_UInt, 0, 1));
				DS_VERIFY(dsShaderVariableGroup_commitWithoutBuffer(
					skinningData->fallbackTextureInfo));
				if (!dsSharedMaterialValues_setVariableGroupID(values,
						skinningData->skinningTextureInfoVar, skinningData->fallbackTextureInfo))
				{
					return false;
				}
			}
			else if (!dsSharedMaterialValues_setBufferID(values,
					skinningData->skinningTextureInfoVar, skinningData->curBuffer, instance->offset,
					skinningData->textureInfoStride))
			{
				return false;
			}
			DS_ASSERT(skinningData->curTextureBuffer);
			return dsSharedMaterialValues_setTextureBufferID(
				values, skinningData->skinningDataVar, skinningData->curTextureBuffer,
				skinningData->format, 0, skinningData->curTextureBuffer->size/sizeof(dsVector4f));
		case dsMaterialType_Texture:
			DS_ASSERT(skinningData->fallbackTextureInfo || skinningData->curBuffer);
			if (skinningData->fallbackTextureInfo)
			{
				DS_VERIFY(dsShaderVariableGroup_setElementData(skinningData->fallbackTextureInfo, 0,
					&instance->instanceOffsetStep, dsMaterialType_Vec2, 0, 1));
				DS_VERIFY(dsShaderVariableGroup_commitWithoutBuffer(
					skinningData->fallbackTextureInfo));
				if (!dsSharedMaterialValues_setVariableGroupID(values,
						skinningData->skinningTextureInfoVar, skinningData->fallbackTextureInfo))
				{
					return false;
				}
			}
			else if (!dsSharedMaterialValues_setBufferID(values,
					skinningData->skinningTextureInfoVar, skinningData->curBuffer, instance->offset,
					skinningData->textureInfoStride))
			{
				return false;
			}
			return dsSharedMaterialValues_setTextureID(
				values, skinningData->skinningDataVar, instance->texture);
		default:
			DS_ASSERT(false);
			return false;
	}
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
		{
			DS_ASSERT(i == 0);
			return false;
		}
	}
	DS_VERIFY(dsAllocator_free(instanceData->allocator, skinningData->textures));

	for (uint32_t i = 0; i < skinningData->textureBufferCount; ++i)
	{
		if (!dsGfxBuffer_destroy(skinningData->textureBuffers[i].buffer))
		{
			DS_ASSERT(i == 0);
			return false;
		}
	}
	DS_VERIFY(dsAllocator_free(instanceData->allocator, skinningData->textureBuffers));

	DS_VERIFY(dsAllocator_free(instanceData->allocator, skinningData->tempTextureData));
	DS_VERIFY(dsShaderVariableGroup_destroy(skinningData->fallbackTextureInfo));

	DS_VERIFY(dsAllocator_free(instanceData->allocator, skinningData->instances));
	DS_VERIFY(dsAllocator_free(instanceData->allocator, instanceData));
	return true;
}

const char* const dsSceneSkinningData_typeName = "SkinningData";
const char* const dsSceneSkinningData_uniformName = "dsSkinningData";
const char* const dsSceneSkinningData_textureDataUniformName = "dsSkinningTextureData";

static dsSceneInstanceDataType instanceDataType =
{
	.populateDataFunc = &dsSceneSkinningData_populateData,
	.bindInstanceFunc = &dsSceneSkinningData_bindInstance,
	.finishFunc = &dsSceneSkinningData_finish,
	.destroyFunc = &dsSceneSkinningData_destroy
};

const dsSceneInstanceDataType* dsSceneSkinningData_type(void)
{
	return &instanceDataType;
}

dsMaterialType dsSceneSkinningData_materialType(const dsResourceManager* resourceManager)
{
	if (!resourceManager)
		return dsMaterialType_Texture;

	if ((resourceManager->supportedBuffers & dsGfxBufferUsage_UniformBuffer) &&
		(resourceManager->uniformBufferSupportedStages & dsGfxPipelineStage_VertexShader))
	{
		return dsMaterialType_UniformBuffer;
	}
	else if (resourceManager->supportedBuffers & dsGfxBufferUsage_Texture)
		return dsMaterialType_TextureBuffer;

	return dsMaterialType_Texture;
}

dsShaderVariableGroupDesc* dsSceneSkinningData_createTextureInfoShaderVariableGroupDesc(
	dsResourceManager* resourceManager, dsAllocator* allocator)
{
	if (!resourceManager)
	{
		errno = EINVAL;
		return NULL;
	}

	switch (dsSceneSkinningData_materialType(resourceManager))
	{
		case dsMaterialType_TextureBuffer:
			return dsShaderVariableGroupDesc_create(resourceManager, allocator,
				textureBufferInfoElements, DS_ARRAY_SIZE(textureBufferInfoElements));
		case dsMaterialType_Texture:
			return dsShaderVariableGroupDesc_create(resourceManager, allocator,
				textureInfoElements, DS_ARRAY_SIZE(textureInfoElements));
		default:
			errno = EPERM;
			return NULL;
	}
}

bool dsSceneSkinningData_isTextureInfoShaderVariableGroupCompatible(
	const dsResourceManager* resourceManager, const dsShaderVariableGroupDesc* textureInfoDesc)
{
	if (!resourceManager)
		return false;

	dsMaterialType materialType = dsSceneSkinningData_materialType(resourceManager);
	switch (materialType)
	{
		case dsMaterialType_TextureBuffer:
			return textureInfoDesc && dsShaderVariableGroupDesc_areElementsEqual(
				textureBufferInfoElements, DS_ARRAY_SIZE(textureBufferInfoElements),
				textureInfoDesc->elements, textureInfoDesc->elementCount);
		case dsMaterialType_Texture:
			return textureInfoDesc && dsShaderVariableGroupDesc_areElementsEqual(
				textureInfoElements, DS_ARRAY_SIZE(textureInfoElements), textureInfoDesc->elements,
				textureInfoDesc->elementCount);
		default:
			return !textureInfoDesc;
	}
}

dsSceneInstanceData* dsSceneSkinningData_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, dsAllocator* resourceAllocator,
	const dsShaderVariableGroupDesc* textureInfoDesc)
{
	if (!allocator || !resourceManager)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		DS_LOG_ERROR(DS_SCENE_ANIMATION_LOG_TAG,
			"Skinning data allocator must support freeing memory.");
		errno = EINVAL;
		return NULL;
	}

	if (!dsSceneSkinningData_isTextureInfoShaderVariableGroupCompatible(
			resourceManager, textureInfoDesc))
	{
		DS_LOG_ERROR(DS_SCENE_ANIMATION_LOG_TAG,
			"Skinning data's texture info shader variable group description must have been created "
			"with dsSceneSkinningData_createTextureInfoShaderVariableGroupDesc().");
		errno = EINVAL;
		return NULL;
	}

	dsSceneSkinningData* skinningData = DS_ALLOCATE_OBJECT(allocator, dsSceneSkinningData);
	if (!skinningData)
		return NULL;

	dsMaterialType skinningMethod = dsSceneSkinningData_materialType(resourceManager);
	bool shaderVariableGroupBuffers = dsShaderVariableGroup_useGfxBuffer(resourceManager);

	dsSceneInstanceData* instanceData = (dsSceneInstanceData*)skinningData;
	instanceData->allocator = dsAllocator_keepPointer(allocator);
	instanceData->type = dsSceneSkinningData_type();
	if (skinningMethod == dsMaterialType_UniformBuffer || !shaderVariableGroupBuffers)
		instanceData->valueCount = 1;
	else
		instanceData->valueCount = 2;
	instanceData->needsCommandBuffer = skinningMethod == dsMaterialType_Texture;

	skinningData->resourceAllocator = resourceAllocator ? resourceAllocator : allocator;
	skinningData->resourceManager = resourceManager;
	skinningData->format = dsGfxFormat_decorate(dsGfxFormat_R32G32B32A32, dsGfxFormat_Float);
	if (skinningMethod == dsMaterialType_UniformBuffer)
		skinningData->bufferUsage = dsGfxBufferUsage_UniformBuffer;
	else if (shaderVariableGroupBuffers)
	{
		skinningData->bufferUsage = dsGfxBufferUsage_UniformBlock;

		dsMaterialType textureInfoType;
		if (skinningMethod == dsMaterialType_TextureBuffer)
			textureInfoType = dsMaterialType_UInt;
		else
			textureInfoType = dsMaterialType_Vec2;
		skinningData->textureInfoStride = dsMaterialType_blockSize(textureInfoType, false);
		skinningData->textureInfoStride = DS_ALIGNED_SIZE(
			skinningData->textureInfoStride, resourceManager->minUniformBlockAlignment);
	}
	else
		skinningData->bufferUsage = 0;
	skinningData->skinningMethod = skinningMethod;
	skinningData->skinningDataVar = dsUniqueNameID_create(dsSceneSkinningData_uniformName);
	skinningData->skinningTextureInfoVar = skinningMethod == dsMaterialType_UniformBuffer ?
		0 : dsUniqueNameID_create(dsSceneSkinningData_textureDataUniformName);
	dsTextureInfo textureInfo =
	{
		skinningData->format, dsTextureDim_2D, TEXTURE_SIZE, TEXTURE_SIZE, 0, 1, 0
	};
	skinningData->textureSize = dsTexture_size(&textureInfo);

	skinningData->buffers = NULL;
	skinningData->bufferCount = 0;
	skinningData->maxBuffers = 0;

	skinningData->curBuffer = NULL;
	skinningData->curTextureBuffer = NULL;
	skinningData->tempTextureData = NULL;
	skinningData->fallbackTextureInfoDesc = NULL;
	skinningData->fallbackTextureInfo = NULL;

	skinningData->textures = NULL;
	skinningData->textureCount = 0;
	skinningData->maxTextures = 0;

	skinningData->textureBuffers = NULL;
	skinningData->textureBufferCount = 0;
	skinningData->maxTextureBuffers = 0;

	skinningData->instances = NULL;
	skinningData->instanceCount = 0;
	skinningData->maxInstances = 0;

	if (skinningMethod == dsMaterialType_Texture)
	{
		skinningData->tempTextureData = (dsAnimationJointTransform*)dsAllocator_alloc(
			allocator, skinningData->textureSize);
		if (!skinningData->tempTextureData)
		{
			dsSceneSkinningData_destroy(instanceData);
			return NULL;
		}
	}

	if (skinningData->skinningMethod != dsMaterialType_UniformBuffer && !shaderVariableGroupBuffers)
	{
		skinningData->fallbackTextureInfoDesc = textureInfoDesc;
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

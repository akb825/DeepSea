/*
 * Copyright 2023 Aaron Barany
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

#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/Resources/Texture.h>

#include <DeepSea/SceneAnimation/SceneAnimationTreeNode.h>

#include <string.h>

#define FRAME_DELAY 3

typedef struct BufferInfo
{
	dsGfxBuffer* buffer;
	uint64_t lastUsedFrame;
} BufferInfo;

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
		dsTexture* texture;
	};
} InstanceData;

typedef struct dsSceneSkinningData
{
	dsSceneInstanceData instanceData;

	dsAllocator* resourceAllocator;
	dsResourceManager* resourceManager;
	bool useBuffers;
	uint32_t shaderVar;

	BufferInfo* buffers;
	uint32_t bufferCount;
	uint32_t maxBuffers;

	dsGfxBuffer* curBuffer;

	InstanceData* instances;
	uint32_t instanceCount;
	uint32_t maxInstances;
} dsSceneSkinningData;

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
		dsGfxBufferUsage_UniformBlock, dsGfxMemory_Stream | dsGfxMemory_Synchronize, NULL,
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
	uint32_t alignment = skinningData->resourceManager->minUniformBlockAlignment;
	size_t bufferSize = 0;
	for (uint32_t i = 0; i < skinningData->instanceCount; ++i)
	{
		const dsAnimationTree* animationTree = skinningData->instances[i].animationTree;
		if (animationTree)
		{
			bufferSize +=
				DS_CUSTOM_ALIGNED_SIZE(animationTree->nodeCount*sizeof(dsMatrix44f), alignment);
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

		size_t copySize = animationTree->nodeCount*sizeof(dsMatrix44f);
		instance->offset = offset;
		instance->size = copySize;
		memcpy(bufferData + offset, animationTree->jointTransforms, copySize);
		offset += DS_CUSTOM_ALIGNED_SIZE(copySize, alignment);
	}

	DS_VERIFY(dsGfxBuffer_unmap(buffer));
	return true;
}

static bool populateTextureData(dsSceneInstanceData* instanceData)
{
	// NOTE: would ideally re-use textures, but currently no good way to do so given that populating
	// data can be done inside a render pass. Expect this fallback to be seldom used, if ever, so
	// don't worry too much about the inefficiency for now.
	dsSceneSkinningData* skinningData = (dsSceneSkinningData*)instanceData;
	dsResourceManager* resourceManager = skinningData->resourceManager;
	for (uint32_t i = 0; i < skinningData->instanceCount; ++i)
	{
		InstanceData* instance = skinningData->instances + i;
		const dsAnimationTree* animationTree = instance->animationTree;
		if (!animationTree)
			continue;

		dsTextureInfo textureInfo =
		{
			dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float),
			dsTextureDim_2D, 4, animationTree->nodeCount, 0, 1, 0
		};
		instance->texture = dsTexture_create(resourceManager, skinningData->resourceAllocator,
			dsTextureUsage_Texture, dsGfxMemory_Stream, &textureInfo,
			animationTree->jointTransforms, animationTree->nodeCount*sizeof(dsMatrix44f));
		if (!instance->texture)
			return false;
	}

	return true;
}

static bool dsSceneSkinningData_populateData(dsSceneInstanceData* instanceData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount)
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

	bool anyInstances = false;
	for (uint32_t i = 0; i < instanceCount; ++i)
	{
		InstanceData* instance = skinningData->instances + i;
		const dsAnimationTree* animationTree =
			dsSceneAnimationTreeNode_getAnimationTreeForInstance(instances[i]);
		if (animationTree && animationTree->jointTransforms)
		{
			instance->animationTree = animationTree;
			anyInstances = true;
		}
		else
			instance->animationTree = NULL;
		instance->offset = 0;
		instance->size = 0;
	}

	if (!anyInstances)
		DS_PROFILE_FUNC_RETURN(true);

	bool success;
	if (skinningData->useBuffers)
		success = populateBufferData(instanceData);
	else
		success = populateTextureData(instanceData);

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

	if (skinningData->useBuffers)
	{
		return dsSharedMaterialValues_setBufferID(values, skinningData->shaderVar,
			skinningData->curBuffer, instance->offset, instance->size);
	}
	else
	{
		return dsSharedMaterialValues_setTextureID(values, skinningData->shaderVar,
			instance->texture);
	}
}

static bool dsSceneSkinningData_finish(dsSceneInstanceData* instanceData)
{
	dsSceneSkinningData* skinningData = (dsSceneSkinningData*)instanceData;
	if (skinningData->useBuffers)
		skinningData->curBuffer = NULL;
	else
	{
		for (uint32_t i = 0; i < skinningData->instanceCount; ++i)
		{
			InstanceData* instance = skinningData->instances + i;
			if (!dsTexture_destroy(instance->texture))
				return false;
		}
	}

	skinningData->instanceCount = 0;
	return true;
}

static bool dsSceneSkinningData_destroy(dsSceneInstanceData* instanceData)
{
	dsSceneSkinningData* skinningData = (dsSceneSkinningData*)instanceData;

	if (skinningData->useBuffers)
	{
		for (uint32_t i = 0; i < skinningData->bufferCount; ++i)
		{
			if (!dsGfxBuffer_destroy(skinningData->buffers[i].buffer))
			{
				DS_ASSERT(i == 0);
				return false;
			}
		}
		DS_VERIFY(dsAllocator_free(instanceData->allocator, skinningData->buffers));
	}
	else
	{
		for (uint32_t i = 0; i < skinningData->instanceCount; ++i)
		{
			if (!dsTexture_destroy(skinningData->instances[i].texture))
				return false;
		}
	}

	DS_VERIFY(dsAllocator_free(instanceData->allocator, skinningData->instances));
	DS_VERIFY(dsAllocator_free(instanceData->allocator, instanceData));
	return true;
}

const char* const dsSceneSkinningData_typeName = "SkinningData";

bool dsSceneSkinningData_useBuffers(dsResourceManager* resourceManager)
{
	return resourceManager && (resourceManager->supportedBuffers & dsGfxBufferUsage_UniformBlock);
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

	dsSceneInstanceData* instanceData = (dsSceneInstanceData*)skinningData;
	instanceData->allocator = dsAllocator_keepPointer(allocator);
	instanceData->valueCount = 1;
	instanceData->populateDataFunc = &dsSceneSkinningData_populateData;
	instanceData->bindInstanceFunc = &dsSceneSkinningData_bindInstance;
	instanceData->finishFunc = &dsSceneSkinningData_finish;
	instanceData->destroyFunc = &dsSceneSkinningData_destroy;

	skinningData->resourceAllocator = resourceAllocator ? resourceAllocator : allocator;
	skinningData->resourceManager = resourceManager;
	skinningData->useBuffers = dsSceneSkinningData_useBuffers(resourceManager);
	skinningData->shaderVar = dsHashString(dsSceneSkinningData_typeName);

	skinningData->buffers = NULL;
	skinningData->bufferCount = 0;
	skinningData->maxBuffers = 0;

	skinningData->curBuffer = NULL;

	skinningData->instances = NULL;
	skinningData->instanceCount = 0;
	skinningData->maxInstances = 0;

	return instanceData;
}

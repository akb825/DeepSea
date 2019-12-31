/*
 * Copyright 2019 Aaron Barany
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

#include <DeepSea/Scene/SceneLoadScratchData.h>

#include "SceneTypes.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Scene/SceneResources.h>

#include <string.h>

dsSceneLoadScratchData* dsSceneLoadScratchData_create(dsAllocator* allocator,
	dsCommandBuffer* commandBuffer)
{
	if (!allocator)
		return NULL;

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG,
			"Scene load scratch data allocator must support freeing memory.");
		return NULL;
	}

	dsSceneLoadScratchData* scratchData = DS_ALLOCATE_OBJECT(allocator, dsSceneLoadScratchData);
	if (!scratchData)
		return NULL;

	scratchData->allocator = dsAllocator_keepPointer(allocator);
	scratchData->commandBuffer = commandBuffer;
	scratchData->readBuffer = NULL;
	scratchData->readBufferSize = 0;
	scratchData->readBufferUsed = false;
	scratchData->sceneResources = NULL;
	scratchData->sceneResourceCount = 0;
	scratchData->maxSceneResources = 0;
	return scratchData;
}

dsAllocator* dsSceneLoadScratchData_getAllocator(dsSceneLoadScratchData* scratchData)
{
	if (!scratchData)
	{
		errno = EINVAL;
		return NULL;
	}

	return scratchData->allocator;
}

void* dsSceneLoadScratchData_readUntilEnd(size_t* outSize, dsSceneLoadScratchData* scratchData,
	dsStream* stream)
{
	if (!outSize || !scratchData || !stream)
	{
		errno = EINVAL;
		return NULL;
	}

	if (scratchData->readBufferUsed)
		return dsStream_readUntilEnd(outSize, stream, scratchData->allocator);
	else
	{
		if (!dsStream_readUntilEndReuse(&scratchData->readBuffer, outSize,
				&scratchData->readBufferSize, stream, scratchData->allocator))
		{
			return NULL;
		}

		scratchData->readBufferUsed = true;
		return scratchData->readBuffer;
	}
}

bool dsSceneLoadScratchData_freeReadBuffer(dsSceneLoadScratchData* scratchData, void* buffer)
{
	if (!scratchData)
	{
		errno = EINVAL;
		return false;
	}

	if (buffer == scratchData->readBuffer)
	{
		DS_ASSERT(scratchData->readBufferUsed);
		scratchData->readBufferUsed = false;
	}
	else
		DS_VERIFY(dsAllocator_free(scratchData->allocator, buffer));
	return true;
}

bool dsSceneLoadScratchData_pushSceneResources(dsSceneLoadScratchData* scratchData,
	dsSceneResources** resources, uint32_t resourceCount)
{
	if (!scratchData || (!resources && resourceCount == 0))
	{
		errno = EINVAL;
		return false;
	}

	if (resourceCount == 0)
		return true;

	for (uint32_t i = 0; i < resourceCount; ++i)
	{
		if (!resources[i])
		{
			errno = EINVAL;
			return false;
		}
	}

	uint32_t offset = scratchData->sceneResourceCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(scratchData->allocator, scratchData->sceneResources,
			scratchData->sceneResourceCount, scratchData->maxSceneResources, resourceCount))
	{
		return false;
	}

	for (uint32_t i = 0; i < resourceCount; ++i)
		scratchData->sceneResources[offset + i] = dsSceneResources_addRef(resources[i]);
	return true;
}

bool dsSceneLoadScratchData_popSceneResources(dsSceneLoadScratchData* scratchData,
	uint32_t resourceCount)
{
	if (!scratchData || resourceCount > scratchData->sceneResourceCount)
	{
		errno = EINVAL;
		return false;
	}

	scratchData->sceneResourceCount -= resourceCount;
	for (uint32_t i = 0; i < resourceCount; ++i)
		dsSceneResources_freeRef(scratchData->sceneResources[scratchData->sceneResourceCount + i]);
	return true;
}

dsSceneResources** dsSceneLoadScratchData_getSceneResources(uint32_t* outResourceCount,
	const dsSceneLoadScratchData* scratchData)
{
	if (!scratchData)
		return NULL;

	if (outResourceCount)
		*outResourceCount = scratchData->sceneResourceCount;
	return scratchData->sceneResources;
}

bool dsSceneLoadScratchData_findResource(dsSceneResourceType* outType,
	void** outResource, const dsSceneLoadScratchData* scratchData, const char* name)
{
	if (!scratchData || !name)
		return false;

	for (uint32_t i = scratchData->sceneResourceCount; i-- > 0;)
	{
		if (dsSceneResources_findResource(
				outType, outResource, scratchData->sceneResources[i], name))
		{
			return true;
		}
	}

	return false;
}

dsCommandBuffer* dsSceneLoadScratchData_getCommandBuffer(const dsSceneLoadScratchData* scratchData)
{
	if (!scratchData)
		return NULL;

	return scratchData->commandBuffer;
}

bool dsSceneLoadScratchData_setCommandBuffer(dsSceneLoadScratchData* scratchData,
	dsCommandBuffer* commandBuffer)
{
	if (!scratchData)
	{
		errno = EINVAL;
		return false;
	}

	scratchData->commandBuffer = commandBuffer;
	return true;
}

void dsSceneLoadScratchData_destroy(dsSceneLoadScratchData* scratchData)
{
	if (!scratchData)
		return;

	DS_VERIFY(dsAllocator_free(scratchData->allocator, scratchData->readBuffer));
	DS_VERIFY(dsAllocator_free(scratchData->allocator, scratchData->sceneResources));
	DS_VERIFY(dsAllocator_free(scratchData->allocator, scratchData));
}

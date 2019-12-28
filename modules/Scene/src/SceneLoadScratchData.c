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
	scratchData->data = NULL;
	scratchData->dataSize = 0;
	scratchData->maxDataSize = 0;
	scratchData->sceneResources = NULL;
	scratchData->sceneResourceCount = 0;
	scratchData->maxSceneResources = 0;
	return scratchData;
}

void* dsSceneLoadScratchData_allocate(dsSceneLoadScratchData* scratchData, uint32_t size)
{
	size = DS_ALIGNED_SIZE(size);
	if (!scratchData || size == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	uint32_t offset = scratchData->dataSize;
	if (!DS_RESIZEABLE_ARRAY_ADD(scratchData->allocator, scratchData->data, scratchData->dataSize,
			scratchData->maxDataSize, size))
	{
		return NULL;
	}

	return scratchData->data + offset;
}

void* dsSceneLoadScratchData_readUntilEnd(uint32_t* outSize, dsSceneLoadScratchData* scratchData,
	dsStream* stream)
{
	if (!outSize || !scratchData || !stream)
	{
		errno = EINVAL;
		return NULL;
	}

	uint32_t offset = scratchData->dataSize;
	if (stream->seekFunc && stream->tellFunc)
	{
		uint64_t position = dsStream_tell(stream);
		if (position == DS_STREAM_INVALID_POS || !dsStream_seek(stream, 0, dsStreamSeekWay_End))
			return NULL;

		uint64_t end = dsStream_tell(stream);
		if (end == DS_STREAM_INVALID_POS ||
			!dsStream_seek(stream, position, dsStreamSeekWay_Beginning))
		{
			return NULL;
		}

		uint32_t readSize = (uint32_t)(end - position);
		if (!DS_RESIZEABLE_ARRAY_ADD(scratchData->allocator, scratchData->data,
				scratchData->dataSize, scratchData->maxDataSize, readSize))
		{
			return NULL;
		}

		size_t read = dsStream_read(stream, scratchData->data + offset, readSize);
		if (read != readSize)
		{
			errno = EIO;
			return NULL;
		}
	}
	else
	{
		uint8_t tempBuffer[1024];
		do
		{
			uint32_t readSize = (uint32_t)dsStream_read(stream, tempBuffer, sizeof(tempBuffer));
			if (readSize == 0)
				break;

			uint32_t curOffset = scratchData->dataSize;
			if (!DS_RESIZEABLE_ARRAY_ADD(scratchData->allocator, scratchData->data,
					scratchData->dataSize, scratchData->maxDataSize, readSize))
			{
				return NULL;
			}

			memcpy(scratchData->data + curOffset, tempBuffer, readSize);
		} while (true);
	}

	*outSize = scratchData->dataSize - offset;

	// Keep data aligned.
	uint32_t alignedSize = DS_ALIGNED_SIZE(*outSize);
	if (!DS_RESIZEABLE_ARRAY_ADD(scratchData->allocator, scratchData->data, scratchData->dataSize,
			scratchData->maxDataSize, alignedSize - *outSize))
	{
		return NULL;
	}
	return scratchData->data + offset;
}

bool dsSceneLoadScratchData_popData(dsSceneLoadScratchData* scratchData, uint32_t size)
{
	size = DS_ALIGNED_SIZE(size);
	if (!scratchData || size > scratchData->dataSize)
	{
		errno = EINVAL;
		return false;
	}

	scratchData->dataSize -= size;
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

	DS_VERIFY(dsAllocator_free(scratchData->allocator, scratchData->data));
	DS_VERIFY(dsAllocator_free(scratchData->allocator, scratchData->sceneResources));
	DS_VERIFY(dsAllocator_free(scratchData->allocator, scratchData));
}

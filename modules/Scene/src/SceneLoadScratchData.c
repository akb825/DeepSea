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
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

dsSceneLoadScratchData* dsSceneLoadScratchData_create(dsAllocator* allocator)
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
	scratchData->data = NULL;
	scratchData->dataSize = 0;
	scratchData->maxDataSize = 0;
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

bool dsSceneLoadScratchData_popData(dsSceneLoadScratchData* scratchData, uint32_t size)
{
	size = DS_ALIGNED_SIZE(size);
	if (!scratchData || size == 0 || size > scratchData->dataSize)
	{
		errno = EINVAL;
		return false;
	}

	scratchData->dataSize -= size;
	return true;
}

void dsSceneLoadScratchData_destroy(dsSceneLoadScratchData* scratchData)
{
	if (!scratchData)
		return;

	DS_VERIFY(dsAllocator_free(scratchData->allocator, scratchData->data));
	DS_VERIFY(dsAllocator_free(scratchData->allocator, scratchData));
}

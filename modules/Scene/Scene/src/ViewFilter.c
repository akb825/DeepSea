/*
 * Copyright 2026 Aaron Barany
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

#include <DeepSea/Scene/ViewFilter.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Sort.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <string.h>

struct dsViewFilter
{
	dsAllocator* allocator;
	uint32_t* viewNameIDs;
	uint32_t viewCount;
	bool invert;
};

static int compareNameIDs(const void* left, const void* right)
{
	uint32_t leftID = *(const uint32_t*)left;
	uint32_t rightID = *(const uint32_t*)right;
	return DS_CMP(leftID, rightID);
}

static int searchCompareNameIDs(const void* left, const void* right, void* context)
{
	DS_UNUSED(context);
	uint32_t leftID = *(const uint32_t*)left;
	uint32_t rightID = *(const uint32_t*)right;
	return DS_CMP(leftID, rightID);
}

inline static bool containsNameID(const uint32_t* viewNameIDs, uint32_t viewCount, uint32_t nameID)
{
	return dsBinarySearch(
		&nameID, viewNameIDs, viewCount, sizeof(uint32_t), &searchCompareNameIDs, NULL) != NULL;
}

size_t dsViewFilter_sizeof(void)
{
	return sizeof(dsViewFilter);
}

size_t dsViewFilter_fullAllocSize(uint32_t viewNameCount)
{
	return DS_ALIGNED_SIZE(sizeof(dsViewFilter)) + DS_ALIGNED_SIZE(sizeof(uint32_t)*viewNameCount);
}

dsViewFilter* dsViewFilter_create(
	dsAllocator* allocator, const char* const* viewNames, uint32_t viewNameCount, bool invert)
{
	if (!allocator || !viewNames || viewNameCount == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	for (uint32_t i = 0; i < viewNameCount; ++i)
	{
		if (!viewNames[i])
		{
			errno = EINVAL;
			return NULL;
		}
	}

	size_t fullSize = dsViewFilter_fullAllocSize(viewNameCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsViewFilter* filter = DS_ALLOCATE_OBJECT(&bufferAlloc, dsViewFilter);
	DS_ASSERT(filter);

	filter->allocator = dsAllocator_keepPointer(allocator);
	filter->viewNameIDs = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, uint32_t, viewNameCount);
	for (uint32_t i = 0; i < viewNameCount; ++i)
		filter->viewNameIDs[i] = dsUniqueNameID_create(viewNames[i]);
	qsort(filter->viewNameIDs, viewNameCount, sizeof(uint32_t), &compareNameIDs);
	filter->viewCount = viewNameCount;
	// Force it to 0 or 1.
	filter->invert = invert != false;
	return filter;
}

bool dsViewFilter_containsName(const dsViewFilter* filter, const char* name)
{
	if (!filter || !name)
		return name != NULL;

	return containsNameID(
		filter->viewNameIDs, filter->viewCount, dsUniqueNameID_get(name)) != filter->invert;
}

bool dsViewFilter_containsID(const dsViewFilter* filter, uint32_t nameID)
{
	if (!filter || nameID == 0)
		return nameID != 0;

	return containsNameID(filter->viewNameIDs, filter->viewCount, nameID) != filter->invert;
}

uint32_t dsViewFilter_hash(const dsViewFilter* filter, uint32_t commonHash)
{
	if (!filter)
		return commonHash;

	uint32_t hash = dsHashCombineBytes(
		commonHash, filter->viewNameIDs, sizeof(uint32_t)*filter->viewCount);
	return dsHashCombine8(hash, &filter->invert);
}

bool dsViewFilter_equal(const dsViewFilter* left, const dsViewFilter* right)
{
	if (!left || !right || left == right)
		return left == right;

	if (left->viewCount != right->viewCount || left->invert != right->invert)
		return false;

	return memcmp(left->viewNameIDs, right->viewNameIDs, sizeof(uint32_t)*left->viewCount) == 0;
}

void dsViewFilter_destroy(dsViewFilter* filter)
{
	if (filter && filter->allocator)
		DS_VERIFY(dsAllocator_free(filter->allocator, filter));
}

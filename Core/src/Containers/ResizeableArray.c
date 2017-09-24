/*
 * Copyright 2017 Aaron Barany
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

#include <DeepSea/Core/Containers/ResizeableArray.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <string.h>

bool dsResizeableArray_add(dsAllocator* allocator, void** buffer, size_t* elementCount,
	size_t* maxElements, size_t elementSize, size_t addCount)
{
	if (!allocator || !buffer || !elementCount || !maxElements || elementSize == 0 ||
		(!*buffer && *maxElements > 0) || *elementCount > *maxElements)
	{
		errno = EINVAL;
		return false;
	}

	if (*elementCount + addCount <= *maxElements)
	{
		*elementCount += addCount;
		return true;
	}

	size_t newMaxElements = *maxElements*2;
	if (newMaxElements < *elementCount + addCount)
		newMaxElements = *elementCount + addCount;

	const size_t minElements = 16;
	if (newMaxElements < minElements)
		newMaxElements = minElements;

	DS_ASSERT(newMaxElements >= *elementCount + addCount);
	void* newBuffer = dsAllocator_alloc(allocator, newMaxElements*elementSize);
	if (!newBuffer)
		return false;

	if (*buffer)
	{
		memcpy(newBuffer, *buffer, *elementCount*elementSize);
		DS_VERIFY(dsAllocator_free(allocator, *buffer));
	}

	*elementCount += addCount;
	*maxElements = newMaxElements;
	*buffer = newBuffer;
	return true;
}

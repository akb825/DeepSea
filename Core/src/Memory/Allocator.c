/*
 * Copyright 2016 Aaron Barany
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

#include <DeepSea/Core/Memory/Allocator.h>
#include <string.h>

void* dsAllocator_reallocWithFallback(dsAllocator* allocator, void* ptr, size_t origSize,
	size_t newSize)
{
	if (!allocator || (!allocator->reallocFunc && (!allocator->allocFunc || !allocator->freeFunc)))
	{
		errno = EINVAL;
		return NULL;
	}

	if (allocator->reallocFunc)
		return allocator->reallocFunc(allocator, ptr, newSize, DS_ALLOC_ALIGNMENT);

	void* newPtr = NULL;
	if (newSize > 0)
	{
		newPtr = allocator->allocFunc(allocator, newSize, DS_ALLOC_ALIGNMENT);
		if (!newPtr)
			return NULL;
		if (ptr)
		{
			size_t copySize = origSize;
			if (newSize < copySize)
				copySize = newSize;
			memcpy(newPtr, ptr, copySize);
		}
	}

	allocator->freeFunc(allocator, ptr);
	return newPtr;
}

void* dsAllocator_alloc(dsAllocator* allocator, size_t size);
void* dsAllocator_realloc(dsAllocator* allocator, void* ptr, size_t size);
bool dsAllocator_free(dsAllocator* allocator, void* ptr);
dsAllocator* dsAllocator_keepPointer(dsAllocator* allocator);

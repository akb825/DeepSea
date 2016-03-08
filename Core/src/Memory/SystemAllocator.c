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

#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Memory/Memory.h>
#include <stdlib.h>
#include <stdint.h>

#if DS_APPLE
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

static size_t getMallocSize(void* ptr)
{
#if DS_WINDOWS
	return _msize(ptr);
#elif DS_APPLE
	return malloc_size(ptr);
#else
	return malloc_usable_size(ptr);
#endif
}

bool dsSystemAllocator_initialize(dsSystemAllocator* allocator)
{
	if (!allocator)
		return false;

	((dsAllocator*)allocator)->size = 0;
	((dsAllocator*)allocator)->allocFunc = (dsAllocatorAllocFunction)&dsSystemAllocator_alloc;
	((dsAllocator*)allocator)->freeFunc = (dsAllocatorFreeFunction)&dsSystemAllocator_free;

	return true;
}

void* dsSystemAllocator_alloc(dsSystemAllocator* allocator, size_t size)
{
	if (!allocator)
		return NULL;

	void* ptr;
#if DS_WINDOWS
	ptr = _aligned_malloc(size, DS_ALLOC_ALIGNMENT);
#elif DS_APPLE
	ptr = malloc(size);
#else
	ptr = aligned_alloc(16, DS_ALIGNED_SIZE(size));
#endif

	if (!ptr)
		return NULL;

	((dsAllocator*)allocator)->size += getMallocSize(ptr);
	return ptr;
}

bool dsSystemAllocator_free(dsSystemAllocator* allocator, void* ptr)
{
	if (!allocator)
		return false;

	if (ptr)
		((dsAllocator*)allocator)->size -= getMallocSize(ptr);
	free(ptr);
	return true;
}

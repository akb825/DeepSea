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
#include <DeepSea/Core/Memory/GenericAllocator.h>
#include <stdlib.h>

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

bool dsSystemAllocator_initialize(dsGenericAllocator* allocator)
{
	return dsGenericAllocator_initialize(allocator, NULL, &dsSystemAllocator_alloc,
		&dsSystemAllocator_alignedAlloc, &dsSystemAllocator_realloc,
		&dsSystemAllocator_free, &dsSystemAllocator_destroy);
}

void* dsSystemAllocator_alloc(dsGenericAllocator* allocator, size_t size)
{
	if (!allocator)
		return NULL;

	void* ptr = malloc(size);
	if (!ptr)
		return NULL;

	allocator->size += getMallocSize(ptr);
	return ptr;
}

void* dsSystemAllocator_alignedAlloc(dsGenericAllocator* allocator, size_t alignment,
	size_t size)
{
	if (!allocator)
		return NULL;

#if DS_WINDOWS
	void* ptr = _aligned_malloc(size, alignment);
#elif DS_APPLE

	if (alignment != 1 && alignment != 2 && alignment != 4 && alignment != 8 && alignment != 16)
		return NULL;
	void* ptr = malloc(size);

#else
	void* ptr = aligned_alloc(alignment, size);
#endif

	if (!ptr)
		return NULL;

	allocator->size += getMallocSize(ptr);
	return ptr;
}

void* dsSystemAllocator_realloc(dsGenericAllocator* allocator, void* ptr,
	size_t size)
{
	if (!allocator)
		return NULL;

	if (ptr)
		allocator->size -= getMallocSize(ptr);
	ptr = realloc(ptr, size);

	if (!ptr)
		return NULL;

	allocator->size += getMallocSize(ptr);
	return ptr;
}

bool dsSystemAllocator_free(dsGenericAllocator* allocator, void* ptr)
{
	if (!allocator)
		return false;

	if (ptr)
		allocator->size -= getMallocSize(ptr);
	free(ptr);
	return true;
}

void dsSystemAllocator_destroy(void* userData)
{
	DS_UNUSED(userData);
}

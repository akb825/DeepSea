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

#include "MTLTempBuffer.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>

#import <Metal/MTLBuffer.h>

dsMTLTempBuffer* dsMTLTempBuffer_create(dsAllocator* allocator, id<MTLDevice> device)
{
	MTLResourceOptions options = MTLResourceCPUCacheModeDefaultCache;
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
	options |= MTLResourceStorageModeShared;
#endif
	id<MTLBuffer> mtlBuffer = [device newBufferWithLength: DS_TEMP_BUFFER_CAPACITY
		options: options];
	if (!mtlBuffer)
	{
		errno = ENOMEM;
		return NULL;
	}

	dsMTLTempBuffer* buffer = DS_ALLOCATE_OBJECT(allocator, dsMTLTempBuffer);
	if (!buffer)
		return NULL;

	dsLifetime* lifetime = dsLifetime_create(allocator, buffer);
	if (!lifetime)
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, buffer));
		return NULL;
	}

	buffer->allocator = dsAllocator_keepPointer(allocator);
	buffer->lifetime = lifetime;
	buffer->mtlBuffer = CFBridgingRetain(mtlBuffer);
	buffer->contents = (uint8_t*)mtlBuffer.contents;
	buffer->lastUsedSubmit = DS_NOT_SUBMITTED;
	buffer->size = 0;
	return buffer;
}

void* dsMTLTempBuffer_allocate(uint32_t* outOffset, id<MTLBuffer>* outMTLBuffer,
	dsMTLTempBuffer* buffer, uint32_t size, uint32_t alignment)
{
	DS_ASSERT(size <= DS_MAX_TEMP_BUFFER_ALLOC);
	uint32_t offset = DS_CUSTOM_ALIGNED_SIZE(buffer->size, alignment);
	if (!DS_IS_BUFFER_RANGE_VALID(offset, size, DS_TEMP_BUFFER_CAPACITY))
		return NULL;

	buffer->size = offset + size;
	*outOffset = offset;
	*outMTLBuffer = (__bridge id<MTLBuffer>)buffer->mtlBuffer;
	return buffer->contents + offset;
}

void dsMTLTempBuffer_finish(dsMTLTempBuffer* buffer, uint64_t submitCount)
{
	DS_ATOMIC_STORE64(&buffer->lastUsedSubmit, &submitCount);
}

bool dsMTLTempBuffer_reset(dsMTLTempBuffer* buffer, uint64_t finishedSubmitCount)
{
	uint64_t lastUsedSubmit;
	DS_ATOMIC_LOAD64(&buffer->lastUsedSubmit, &lastUsedSubmit);
	if (lastUsedSubmit > finishedSubmitCount)
		return false;

	// Shouldn't be any thread contention at this point for lastUsedSubmit.
	buffer->lastUsedSubmit = DS_NOT_SUBMITTED;
	buffer->size = 0;
	return true;
}

void dsMTLTempBuffer_destroy(dsMTLTempBuffer* buffer)
{
	dsLifetime_destroy(buffer->lifetime);
	CFRelease(buffer->mtlBuffer);
	if (buffer->allocator)
		DS_VERIFY(dsAllocator_free(buffer->allocator, buffer));
}

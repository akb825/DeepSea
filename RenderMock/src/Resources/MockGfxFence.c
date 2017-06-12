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

#include "Resources/MockGfxBuffer.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>

dsGfxFence* dsMockGfxFence_create(dsResourceManager* resourceManager, dsAllocator* allocator)
{
	DS_ASSERT(resourceManager);
	DS_UNUSED(resourceManager);

	dsGfxFence* fence = (dsGfxFence*)dsAllocator_alloc(allocator, sizeof(dsGfxFence));
	if (!fence)
		return NULL;

	fence->resourceManager = resourceManager;
	fence->allocator = dsAllocator_keepPointer(allocator);
	return fence;
}

bool dsMockGfxFence_set(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxFence* fences, uint32_t fenceCount, bool bufferReadback)
{
	DS_ASSERT(resourceManager);
	DS_UNUSED(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(fences);
	DS_UNUSED(fences);
	DS_ASSERT(fenceCount > 0);
	DS_UNUSED(fenceCount);
	DS_UNUSED(bufferReadback);

	return true;
}

dsGfxFenceResult dsMockGfxFence_wait(dsResourceManager* resourceManager, dsGfxFence* fence,
	uint64_t timeout)
{
	DS_ASSERT(resourceManager);
	DS_UNUSED(resourceManager);
	DS_ASSERT(fence);
	DS_UNUSED(fence);
	DS_UNUSED(timeout);

	return dsGfxFenceResult_Success;
}

bool dsMockGfxFence_reset(dsResourceManager* resourceManager, dsGfxFence* fence)
{
	DS_ASSERT(resourceManager);
	DS_UNUSED(resourceManager);
	DS_ASSERT(fence);
	DS_UNUSED(fence);

	return true;
}

bool dsMockGfxFence_destroy(dsResourceManager* resourceManager, dsGfxFence* fence)
{
	DS_ASSERT(resourceManager);
	DS_UNUSED(resourceManager);
	DS_ASSERT(fence);
	if (fence->allocator)
		return dsAllocator_free(fence->allocator, fence);

	return true;
}

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

#include "Resources/MTLGfxFence.h"

#include "MTLCommandBuffer.h"
#include "MTLRendererInternal.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Math/Core.h>

dsGfxFence* dsMTLGfxFence_create(dsResourceManager* resourceManager, dsAllocator* allocator)
{
	dsMTLGfxFence* fence = DS_ALLOCATE_OBJECT(allocator, dsMTLGfxFence);
	if (!fence)
		return NULL;

	dsGfxFence* baseFence = (dsGfxFence*)fence;
	baseFence->resourceManager = resourceManager;
	baseFence->allocator = dsAllocator_keepPointer(allocator);

	fence->lifetime = dsLifetime_create(allocator, fence);
	if (!fence->lifetime)
	{
		dsMTLGfxFence_destroy(resourceManager, baseFence);
		return NULL;
	}

	fence->lastUsedSubmit = DS_NOT_SUBMITTED;
	return baseFence;
}

bool dsMTLGfxFence_set(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxFence** fences, uint32_t fenceCount, bool bufferReadback)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(bufferReadback);
	for (uint32_t i = 0; i < fenceCount; ++i)
	{
		if (!dsMTLCommandBuffer_addFence(commandBuffer, fences[i]))
			return false;
	}

	dsMTLCommandBuffer_submitFence(commandBuffer);
	return true;
}

dsGfxFenceResult dsMTLGfxFence_wait(dsResourceManager* resourceManager, dsGfxFence* fence,
	uint64_t timeout)
{
	dsMTLGfxFence* vkFence = (dsMTLGfxFence*)fence;
	uint64_t submit;
	DS_ATOMIC_LOAD64(&vkFence->lastUsedSubmit, &submit);

	double milliseconds = round((double)timeout/1000000.0);
	unsigned int intMS = (unsigned int)dsMin((double)UINT_MAX, milliseconds);
	return dsMTLRenderer_waitForSubmit(resourceManager->renderer, submit, intMS);
}

bool dsMTLGfxFence_reset(dsResourceManager* resourceManager, dsGfxFence* fence)
{
	DS_UNUSED(resourceManager);
	dsMTLGfxFence* vkFence = (dsMTLGfxFence*)fence;
	uint64_t submit = DS_NOT_SUBMITTED;
	DS_ATOMIC_STORE64(&vkFence->lastUsedSubmit, &submit);
	return true;
}

bool dsMTLGfxFence_destroy(dsResourceManager* resourceManager, dsGfxFence* fence)
{
	DS_UNUSED(resourceManager);
	dsMTLGfxFence* mtlFence = (dsMTLGfxFence*)fence;
	dsLifetime_destroy(mtlFence->lifetime);
	if (fence->allocator)
		DS_VERIFY(dsAllocator_free(fence->allocator, fence));
	return true;
}

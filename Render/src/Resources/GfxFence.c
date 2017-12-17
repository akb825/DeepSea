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

#include <DeepSea/Render/Resources/GfxFence.h>

#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Types.h>

extern const char* dsResourceManager_noContextError;

dsGfxFence* dsGfxFence_create(dsResourceManager* resourceManager, dsAllocator* allocator)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	if (!resourceManager->createFenceFunc || !resourceManager->destroyFenceFunc ||
		!resourceManager->hasFences)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Fences aren't supported on the current target.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsGfxFence* fence = resourceManager->createFenceFunc(resourceManager, allocator);
	if (fence)
		DS_ATOMIC_FETCH_ADD32(&resourceManager->fenceCount, 1);
	DS_PROFILE_FUNC_RETURN(fence);
}

bool dsGfxFence_set(dsGfxFence* fence, dsCommandBuffer* commandBuffer, bool bufferReadback)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !fence || !fence->resourceManager ||
		!fence->resourceManager->setFencesFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->usage & (dsCommandBufferUsage_MultiSubmit | dsCommandBufferUsage_MultiFrame))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Fences cannot be set on a command buffers that can be submitted multiple times.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = fence->resourceManager;
	bool retVal = resourceManager->setFencesFunc(resourceManager, commandBuffer, &fence, 1,
		bufferReadback);
	DS_PROFILE_FUNC_RETURN(retVal);
}

bool dsGfxFence_setMultiple(dsCommandBuffer* commandBuffer, dsGfxFence** fences,
	uint32_t fenceCount, bool bufferReadback)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !commandBuffer->renderer || !commandBuffer->renderer->resourceManager ||
		!commandBuffer->renderer->resourceManager->setFencesFunc || (!fences && fenceCount > 0))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	for (uint32_t i = 0; i < fenceCount; ++i)
	{
		if (!fences[i])
		{
			errno = EINVAL;
			DS_PROFILE_FUNC_RETURN(false);
		}
	}

	if (commandBuffer->usage & (dsCommandBufferUsage_MultiSubmit | dsCommandBufferUsage_MultiFrame))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Fences cannot be set on a command buffers that can be submitted multiple times.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (fenceCount == 0)
		DS_PROFILE_FUNC_RETURN(true);

	dsResourceManager* resourceManager = commandBuffer->renderer->resourceManager;
	bool retVal = resourceManager->setFencesFunc(resourceManager, commandBuffer, fences, fenceCount,
		bufferReadback);
	DS_PROFILE_FUNC_RETURN(retVal);
}

dsGfxFenceResult dsGfxFence_wait(dsGfxFence* fence, uint64_t timeout)
{
	DS_PROFILE_WAIT_START(__FUNCTION__);
	if (!fence || !fence->resourceManager || !fence->resourceManager->waitFenceFunc)
	{
		errno = EINVAL;
		DS_PROFILE_WAIT_END();
		return dsGfxFenceResult_Error;
	}

	dsResourceManager* resourceManager = fence->resourceManager;
	dsGfxFenceResult retVal = resourceManager->waitFenceFunc(resourceManager, fence, timeout);
	DS_PROFILE_WAIT_END();
	return retVal;
}

bool dsGfxFence_reset(dsGfxFence* fence)
{
	DS_PROFILE_FUNC_START();

	if (!fence || !fence->resourceManager || !fence->resourceManager->resetFenceFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = fence->resourceManager;
	bool retVal = resourceManager->resetFenceFunc(resourceManager, fence);
	DS_PROFILE_FUNC_RETURN(retVal);
}

bool dsGfxFence_destroy(dsGfxFence* fence)
{
	DS_PROFILE_FUNC_START();

	if (!fence || !fence->resourceManager || !fence->resourceManager->destroyBufferFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = fence->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->destroyFenceFunc(resourceManager, fence);
	if (success)
		DS_ATOMIC_FETCH_ADD32(&resourceManager->fenceCount, -1);;
	DS_PROFILE_FUNC_RETURN(success);
}

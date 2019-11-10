/*
 * Copyright 2017-2019 Aaron Barany
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

#include <DeepSea/Render/CommandBufferPool.h>

#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

dsCommandBufferPool* dsCommandBufferPool_create(dsRenderer* renderer, dsAllocator* allocator,
	dsCommandBufferUsage usage)
{
	DS_PROFILE_FUNC_START();

	if (!renderer || (!allocator && !renderer->allocator) ||
		!renderer->createCommandBufferPoolFunc || !renderer->destroyCommandBufferPoolFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = renderer->allocator;

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Command buffer pool allocator must support freeing memory.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsCommandBufferPool* pool = renderer->createCommandBufferPoolFunc(renderer, allocator, usage);
	DS_PROFILE_FUNC_RETURN(pool);
}

dsCommandBuffer** dsCommandBufferPool_createCommandBuffers(dsCommandBufferPool* pool,
	uint32_t count)
{
	DS_PROFILE_FUNC_START();

	if (!pool || !pool->renderer || !pool->renderer->createCommandBuffersFunc || count == 0)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsRenderer* renderer = pool->renderer;
	uint32_t offset = pool->count;
	if (!renderer->createCommandBuffersFunc(renderer, pool, count))
		DS_PROFILE_FUNC_RETURN(NULL);

	// Make sure that the internal state of the command buffer is properly initialized.
	DS_ASSERT(pool->commandBuffers && pool->count == offset + count);
	dsCommandBuffer** commandBuffers = pool->commandBuffers + offset;
	for (uint32_t i = 0; i < count; ++i)
	{
		DS_ASSERT(commandBuffers[i] && commandBuffers[i]->usage == pool->usage);
		commandBuffers[i]->frameActive = true;
		commandBuffers[i]->boundSurface = NULL;
		commandBuffers[i]->boundFramebuffer = NULL;
		commandBuffers[i]->boundRenderPass = NULL;
		commandBuffers[i]->activeRenderSubpass = 0;
		commandBuffers[i]->boundShader = NULL;
		commandBuffers[i]->boundComputeShader = NULL;
	}
	DS_PROFILE_FUNC_RETURN(commandBuffers);
}

bool dsCommandBufferPool_reset(dsCommandBufferPool* pool)
{
	DS_PROFILE_FUNC_START();

	if (!pool || !pool->renderer || !pool->renderer->resetCommandBufferPoolFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsRenderer* renderer = pool->renderer;
	bool success = renderer->resetCommandBufferPoolFunc(renderer, pool);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsCommandBufferPool_destroy(dsCommandBufferPool* pool)
{
	if (!pool)
		return true;

	DS_PROFILE_FUNC_START();

	if (!pool || !pool->renderer || !pool->renderer->destroyCommandBufferPoolFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsRenderer* renderer = pool->renderer;
	bool success = renderer->destroyCommandBufferPoolFunc(renderer, pool);
	DS_PROFILE_FUNC_RETURN(success);
}

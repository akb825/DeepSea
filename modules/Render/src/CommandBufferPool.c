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

#include <DeepSea/Render/CommandBufferPool.h>

#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

dsCommandBufferPool* dsCommandBufferPool_create(dsRenderer* renderer, dsAllocator* allocator,
	dsCommandBufferUsage usage, uint32_t count)
{
	DS_PROFILE_FUNC_START();

	if (!renderer || (!allocator && !renderer->allocator) ||
		!renderer->createCommandBufferPoolFunc || !renderer->destroyCommandBufferPoolFunc ||
		count == 0)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = renderer->allocator;

	if (!dsThread_equal(dsThread_thisThreadID(), renderer->mainThread))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Command buffer pools may only be created on the main thread.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsCommandBufferPool* pool = renderer->createCommandBufferPoolFunc(renderer, allocator, usage,
		count);
	if (!pool)
		DS_PROFILE_FUNC_RETURN(pool);

	// Internally managed states, so guarantee they are properly initialized.
	for (uint32_t i = 0; i < pool->count; ++i)
	{
		dsCommandBuffer* commandBuffer = pool->currentBuffers[i];
		commandBuffer->frameActive = true;
		commandBuffer->boundSurface = NULL;
		commandBuffer->boundFramebuffer = NULL;
		commandBuffer->boundRenderPass = NULL;
		commandBuffer->activeRenderSubpass = 0;
		commandBuffer->indirectCommands = false;
		commandBuffer->boundShader = NULL;
		commandBuffer->boundComputeShader = NULL;
	}
	DS_PROFILE_FUNC_RETURN(pool);
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
	if (!dsThread_equal(dsThread_thisThreadID(), renderer->mainThread))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Command buffer pools may only be reset on the main thread.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->resetCommandBufferPoolFunc(renderer, pool);
	if (!success)
		DS_PROFILE_FUNC_RETURN(success);

	// Guarantee that a freshly reset command buffer can bind a render pass.
	for (uint32_t i = 0; i < pool->count; ++i)
	{
		dsCommandBuffer* commandBuffer = pool->currentBuffers[i];
		commandBuffer->frameActive = true;
		commandBuffer->boundSurface = NULL;
		commandBuffer->boundFramebuffer = NULL;
		commandBuffer->boundRenderPass = NULL;
		commandBuffer->activeRenderSubpass = 0;
		commandBuffer->indirectCommands = false;
		commandBuffer->boundShader = NULL;
		commandBuffer->boundComputeShader = NULL;
	}
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
	if (!dsThread_equal(dsThread_thisThreadID(), renderer->mainThread))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Command buffer pools may only be destroyed on the main thread.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->destroyCommandBufferPoolFunc(renderer, pool);
	DS_PROFILE_FUNC_RETURN(success);
}

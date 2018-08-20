/*
 * Copyright 2017-2018 Aaron Barany
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

#include <DeepSea/Render/RenderSurface.h>

#include "GPUProfileContext.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <stdio.h>

#define SCOPE_SIZE 256

static void beginSurfaceScope(const dsRenderSurface* renderSurface)
{
#if DS_PROFILING_ENABLED
	if (renderSurface)
	{
		char buffer[SCOPE_SIZE];
		int result = snprintf(buffer, SCOPE_SIZE, "Surface: %s", renderSurface->name);
		DS_UNUSED(result);
		DS_ASSERT(result > 0 && result < SCOPE_SIZE);
		DS_PROFILE_DYNAMIC_SCOPE_START(buffer);
	}
#else
	DS_UNUSED(renderSurace);
#endif
}

static void endSurfaceScope(const dsRenderSurface* renderSurface)
{
#if DS_PROFILING_ENABLED
	if (renderSurface)
	{
		DS_PROFILE_SCOPE_END();
	}
#else
	DS_UNUSED(renderSurace);
#endif
}

dsRenderSurface* dsRenderSurface_create(dsRenderer* renderer, dsAllocator* allocator,
	const char* name, void* osHandle, dsRenderSurfaceType type)
{
	DS_PROFILE_FUNC_START();

	if (!renderer || (!allocator && !renderer->allocator) || !renderer->createRenderSurfaceFunc ||
		!renderer->destroyRenderSurfaceFunc || !name)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = renderer->allocator;

	if (!dsThread_equal(dsThread_thisThreadId(), renderer->mainThread))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Render surfaces may only be created on the main thread.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsRenderSurface* renderSurface = renderer->createRenderSurfaceFunc(renderer, allocator, name,
		osHandle, type);
	DS_PROFILE_FUNC_RETURN(renderSurface);
}

bool dsRenderSurface_update(dsRenderSurface* renderSurface)
{
	DS_PROFILE_FUNC_START();

	if (!renderSurface || !renderSurface->renderer ||
		!renderSurface->renderer->updateRenderSurfaceFunc)
	{
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!dsThread_equal(dsThread_thisThreadId(), renderSurface->renderer->mainThread))
	{
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Render surfaces may only be updated on the main thread.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool resized = renderSurface->renderer->updateRenderSurfaceFunc(renderSurface->renderer,
		renderSurface);
	DS_PROFILE_FUNC_RETURN(resized);
}

bool dsRenderSurface_beginDraw(const dsRenderSurface* renderSurface, dsCommandBuffer* commandBuffer)
{
	beginSurfaceScope(renderSurface);
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !renderSurface || !renderSurface->renderer ||
		!renderSurface->renderer->beginRenderSurfaceFunc ||
		!renderSurface->renderer->endRenderSurfaceFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_END();
		endSurfaceScope(renderSurface);
		return false;
	}

	if (!commandBuffer->frameActive)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot begin drawing to a render surface outside of a frame.");
		return false;
	}

	if (commandBuffer->boundSurface)
	{
		errno = EPERM;
		DS_PROFILE_FUNC_END();
		endSurfaceScope(renderSurface);
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot begin drawing to a render surface when one is already bound.");
		return false;
	}

	if (commandBuffer->boundRenderPass)
	{
		errno = EPERM;
		DS_PROFILE_FUNC_END();
		endSurfaceScope(renderSurface);
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot begin drawing to a render surface inside of a render pass.");
		return false;
	}

	if (commandBuffer->boundComputeShader)
	{
		errno = EPERM;
		DS_PROFILE_FUNC_END();
		endSurfaceScope(renderSurface);
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot begin drawing to a render surface while a compute shader is bound.");
		return false;
	}

	dsRenderer* renderer = renderSurface->renderer;
	bool begun = renderer->beginRenderSurfaceFunc(renderer, commandBuffer, renderSurface);
	DS_PROFILE_FUNC_END();
	if (begun)
	{
		dsGPUProfileContext_beginSurface(renderer->_profileContext, commandBuffer,
			renderSurface->name);
		commandBuffer->boundSurface = renderSurface;
	}
	else
		endSurfaceScope(renderSurface);
	return begun;
}

bool dsRenderSurface_endDraw(const dsRenderSurface* renderSurface, dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !renderSurface || !renderSurface->renderer ||
		!renderSurface->renderer->endRenderSurfaceFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundSurface != renderSurface)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Can only end drawing to the currently bound render surface.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundRenderPass)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot end drawing to a render surface inside of a render pass.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundComputeShader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot end drawing to a render surface while a compute shader is bound.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsRenderer* renderer = renderSurface->renderer;
	bool ended = renderer->endRenderSurfaceFunc(renderer, commandBuffer, renderSurface);
	DS_PROFILE_FUNC_END();
	if (ended)
	{
		dsGPUProfileContext_endSurface(renderer->_profileContext, commandBuffer);
		endSurfaceScope(renderSurface);
		commandBuffer->boundSurface = NULL;
	}
	return ended;
}

bool dsRenderSurface_swapBuffers(dsRenderSurface** renderSurfaces, size_t count)
{
	DS_PROFILE_WAIT_START(__FUNCTION__);

	if (count > 0 && !renderSurfaces)
	{
		errno = EINVAL;
		DS_PROFILE_WAIT_END();
		return false;
	}

	if (count == 0)
	{
		DS_PROFILE_WAIT_END();
		return true;
	}

	if (!renderSurfaces[0] || !renderSurfaces[0]->renderer ||
		!renderSurfaces[0]->renderer->swapRenderSurfaceBuffersFunc)
	{
		errno = EINVAL;
		DS_PROFILE_WAIT_END();
		return false;
	}

	for (size_t i = 1; i < count; ++i)
	{
		if (!renderSurfaces[i] || renderSurfaces[i]->renderer != renderSurfaces[0]->renderer)
		{
			errno = EINVAL;
			DS_PROFILE_WAIT_END();
			return false;
		}
	}

	dsRenderer* renderer = renderSurfaces[0]->renderer;
	if (!dsThread_equal(dsThread_thisThreadId(), renderer->mainThread))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Render surfaces may only be swapped on the main thread.");
		DS_PROFILE_WAIT_END();
		return false;
	}

	bool swapped = renderer->swapRenderSurfaceBuffersFunc(renderer, renderSurfaces, count);
	DS_PROFILE_WAIT_END();
	return swapped;
}

bool dsRenderSurface_destroy(dsRenderSurface* renderSurface)
{
	if (!renderSurface)
		return true;

	DS_PROFILE_FUNC_START();

	if (!renderSurface->renderer || !renderSurface->renderer->destroyRenderSurfaceFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsRenderer* renderer = renderSurface->renderer;
	if (!dsThread_equal(dsThread_thisThreadId(), renderer->mainThread))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Render surfaces may only be destroyed on the main thread.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool destroyed = renderer->destroyRenderSurfaceFunc(renderer, renderSurface);
	DS_PROFILE_FUNC_RETURN(destroyed);
}

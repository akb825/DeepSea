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

#include <DeepSea/Render/RenderSurface.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

dsRenderSurface* dsRenderSurface_create(dsRenderer* renderer, dsAllocator* allocator,
	void* osHandle, dsRenderSurfaceType type)
{
	DS_PROFILE_FUNC_START();

	if (!renderer || (!allocator && !renderer->allocator) || !renderer->createRenderSurfaceFunc ||
		!renderer->destroyRenderSurfaceFunc)
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

	dsRenderSurface* renderSurface = renderer->createRenderSurfaceFunc(renderer, allocator,
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
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Render surfaces may only be updated on the main thread.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool resized = renderSurface->renderer->updateRenderSurfaceFunc(renderSurface->renderer,
		renderSurface);
	DS_PROFILE_FUNC_RETURN(resized);
}

bool dsRenderSurface_destroy(dsRenderSurface* renderSurface)
{
	DS_PROFILE_FUNC_START();

	if (!renderSurface || !renderSurface->renderer ||
		!renderSurface->renderer->destroyRenderSurfaceFunc)
	{
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!dsThread_equal(dsThread_thisThreadId(), renderSurface->renderer->mainThread))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Render surfaces may only be destroyed on the main thread.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool destroyed = renderSurface->renderer->destroyRenderSurfaceFunc(renderSurface->renderer,
		renderSurface);
	DS_PROFILE_FUNC_RETURN(destroyed);
}

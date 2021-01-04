/*
 * Copyright 2017-2021 Aaron Barany
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

#include <DeepSea/Render/Resources/Renderbuffer.h>

#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Types.h>

extern const char* dsResourceManager_noContextError;

static size_t framebufferSize(dsGfxFormat format, uint32_t width, uint32_t height, uint32_t samples)
{
	dsTextureInfo texInfo = {format, dsTextureDim_2D, width, height, 0, 1, samples};
	return dsTexture_size(&texInfo);
}

dsRenderbuffer* dsRenderbuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsRenderbufferUsage usage, dsGfxFormat format, uint32_t width, uint32_t height,
	uint32_t samples)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createRenderbufferFunc || !resourceManager->destroyRenderbufferFunc ||
		width == 0 || height == 0)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	if (samples == DS_SURFACE_ANTIALIAS_SAMPLES)
		samples = resourceManager->renderer->surfaceSamples;
	else if (samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
		samples = resourceManager->renderer->defaultSamples;
	samples = dsMax(1U, samples);
	if (samples > resourceManager->renderer->maxSurfaceSamples)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Surface samples is above the maximum.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (width > resourceManager->maxRenderbufferSize ||
		height > resourceManager->maxRenderbufferSize)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid renderbuffer dimensions.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsGfxFormat_renderTargetSupported(resourceManager, format))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Format not supported for renderbuffers.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsRenderbuffer* renderbuffer = resourceManager->createRenderbufferFunc(resourceManager,
		allocator, usage, format, width, height, samples);
	if (renderbuffer)
	{
		size_t size = framebufferSize(format, width, height, samples);
		DS_ATOMIC_FETCH_ADD32(&resourceManager->renderbufferCount, 1);
		DS_ATOMIC_FETCH_ADD_SIZE(&resourceManager->renderbufferMemorySize, size);
	}
	DS_PROFILE_FUNC_RETURN(renderbuffer);
}

bool dsRenderbuffer_destroy(dsRenderbuffer* renderbuffer)
{
	if (!renderbuffer)
		return true;

	DS_PROFILE_FUNC_START();

	if (!renderbuffer->resourceManager || !renderbuffer->resourceManager->destroyRenderbufferFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = renderbuffer->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(false);
	}

	size_t size = framebufferSize(renderbuffer->format, renderbuffer->width, renderbuffer->height,
		renderbuffer->samples);
	bool success = resourceManager->destroyRenderbufferFunc(resourceManager, renderbuffer);
	if (success)
	{
		DS_ATOMIC_FETCH_ADD32(&resourceManager->renderbufferCount, -1);
		DS_ATOMIC_FETCH_ADD_SIZE(&resourceManager->renderbufferMemorySize, -size);
	}
	DS_PROFILE_FUNC_RETURN(success);
}

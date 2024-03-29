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

#include <DeepSea/Render/Resources/Framebuffer.h>

#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Types.h>

extern const char* dsResourceManager_noContextError;

dsFramebuffer* dsFramebuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	const char* name, const dsFramebufferSurface* surfaces, uint32_t surfaceCount, uint32_t width,
	uint32_t height, uint32_t layers)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createFramebufferFunc || !resourceManager->destroyFramebufferFunc ||
		!name || (surfaceCount > 0 && !surfaces))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	layers = dsMax(1U, layers);
	if (layers > resourceManager->maxFramebufferLayers)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Framebuffer layers exceeds supported maximum.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	bool hasColorSurface = false;
	dsRenderSurface* renderSurface = NULL;
	for (uint32_t i = 0; i < surfaceCount; ++i)
	{
		if (!surfaces[i].surface)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot use a NULL surface with a framebuffer.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		dsGfxFormat surfaceFormat;
		uint32_t surfaceWidth, surfaceHeight, surfaceLayers;
		switch (surfaces[i].surfaceType)
		{
			case dsGfxSurfaceType_ColorRenderSurfaceLeft:
			case dsGfxSurfaceType_ColorRenderSurfaceRight:
				if (!resourceManager->renderer->stereoscopic)
				{
					errno = EPERM;
					DS_LOG_ERROR(DS_RENDER_LOG_TAG,
						"Attempting to use a stereoscopic render surface for a framebuffer when "
						"not using stereoscopic rendering.");
					DS_PROFILE_FUNC_RETURN(NULL);
				}
				// fall through
			case dsGfxSurfaceType_ColorRenderSurface:
			{
				dsRenderSurface* surface = (dsRenderSurface*)surfaces[i].surface;
				if (renderSurface && surface != renderSurface)
				{
					errno = EPERM;
					DS_LOG_ERROR(DS_RENDER_LOG_TAG,
						"Cannot use multiple dsRenderSurface instances in a framebuffer.");
					DS_PROFILE_FUNC_RETURN(NULL);
				}
				renderSurface = surface;

				surfaceFormat = surface->renderer->surfaceColorFormat;
				surfaceWidth = surface->preRotateWidth;
				surfaceHeight = surface->preRotateHeight;
				surfaceLayers = 1;
				break;
			}
			case dsGfxSurfaceType_DepthRenderSurfaceLeft:
			case dsGfxSurfaceType_DepthRenderSurfaceRight:
				if (!resourceManager->renderer->stereoscopic)
				{
					errno = EPERM;
					DS_LOG_ERROR(DS_RENDER_LOG_TAG,
						"Attempting to use a stereoscopic render surface for a framebuffer when "
						"not using stereoscopic rendering.");
					DS_PROFILE_FUNC_RETURN(NULL);
				}
				// fall through
			case dsGfxSurfaceType_DepthRenderSurface:
			{
				dsRenderSurface* surface = (dsRenderSurface*)surfaces[i].surface;
				if (renderSurface && surface != renderSurface)
				{
					errno = EPERM;
					DS_LOG_ERROR(DS_RENDER_LOG_TAG,
						"Cannot use multiple dsRenderSurface instances in a framebuffer.");
					DS_PROFILE_FUNC_RETURN(NULL);
				}
				renderSurface = surface;

				surfaceFormat = surface->renderer->surfaceDepthStencilFormat;
				surfaceWidth = surface->preRotateWidth;
				surfaceHeight = surface->preRotateHeight;
				surfaceLayers = 1;
				break;
			}
			case dsGfxSurfaceType_Offscreen:
			{
				dsOffscreen* surface = (dsOffscreen*)surfaces[i].surface;
				if (!surface->offscreen)
				{
					errno = EINVAL;
					DS_LOG_ERROR(DS_RENDER_LOG_TAG,
						"Attempting to use a non-offscreen texture for a framebuffer.");
					DS_PROFILE_FUNC_RETURN(NULL);
				}

				surfaceFormat = surface->info.format;
				uint32_t totalLayers = dsMax(1U, surface->info.depth);
				if (surface->resolve)
				{
					surfaceLayers = 1;
					if (surfaces[i].mipLevel != 0)
					{
						errno = EINVAL;
						DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Can only draw to the first mip level of a "
							"resolved offscreen in a framebuffer.");
						DS_PROFILE_FUNC_RETURN(NULL);
					}
				}
				else
				{
					surfaceLayers = dsMax(1U, surface->info.depth);
					if (surface->info.dimension == dsTextureDim_Cube)
						surfaceLayers *= 6;
					else if (surface->info.dimension == dsTextureDim_3D)
						surfaceLayers = 1;
				}

				if (surface->info.dimension == dsTextureDim_Cube)
					totalLayers *= 6;

				if (surfaces[i].mipLevel >= surface->info.mipLevels)
				{
					errno = EINDEX;
					DS_LOG_ERROR(DS_RENDER_LOG_TAG,
						"Mip level out of range for offscreen within a framebuffer.");
					DS_PROFILE_FUNC_RETURN(NULL);
				}

				surfaceWidth = surface->info.width >> surfaces[i].mipLevel;
				surfaceWidth = dsMax(1U, surfaceWidth);
				surfaceHeight = surface->info.height >> surfaces[i].mipLevel;
				surfaceHeight = dsMax(1U, surfaceHeight);

				uint32_t layer = surfaces[i].layer;
				if (surface->info.dimension == dsTextureDim_Cube)
					layer = layer*6 + surfaces[i].cubeFace;
				if (layers == 1 && surface->info.depth > 0 && layer >= totalLayers)
				{
					errno = EINDEX;
					DS_LOG_ERROR(DS_RENDER_LOG_TAG,
						"Texture layer out of range for offscreen within a framebuffer.");
					DS_PROFILE_FUNC_RETURN(NULL);
				}

				break;
			}
			case dsGfxSurfaceType_Renderbuffer:
			{
				dsRenderbuffer* surface = (dsRenderbuffer*)surfaces[i].surface;
				surfaceFormat = surface->format;
				surfaceWidth = surface->width;
				surfaceHeight = surface->height;
				surfaceLayers = 1;
				break;
			}
			default:
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Unknown surface type.");
				DS_PROFILE_FUNC_RETURN(NULL);
		}

		if (layers != 1 && surfaceLayers != layers)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Surface layer count don't match framebuffer layer count.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		if (layers > 1 && (surfaces[i].layer != 0 || surfaces[i].cubeFace != dsCubeFace_None))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Surface layer and cube face must be 0 when binidng multiple layers.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		if (surfaceWidth != width || surfaceHeight != height)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Surface dimensions don't match framebuffer dimensions.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		if (!dsGfxFormat_isValid(surfaceFormat))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Surface format is invalid.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		if (!(surfaceFormat & (dsGfxFormat_D16 | dsGfxFormat_X8D24 | dsGfxFormat_D32_Float |
			dsGfxFormat_S8 | dsGfxFormat_D16S8 | dsGfxFormat_D24S8 | dsGfxFormat_D32S8_Float)))
		{
			hasColorSurface = true;
		}
	}

	if (!hasColorSurface && resourceManager->requiresColorBuffer)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Current target requires at least one color target for a framebuffer.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsFramebuffer* framebuffer = resourceManager->createFramebufferFunc(resourceManager, allocator,
		name, surfaces, surfaceCount, width, height, layers);
	if (framebuffer)
		DS_ATOMIC_FETCH_ADD32(&resourceManager->framebufferCount, 1);
	DS_PROFILE_FUNC_RETURN(framebuffer);
}

dsGfxFormat dsFramebuffer_getSurfaceFormat(const dsRenderer* renderer,
	const dsFramebufferSurface* surface)
{
	if (!renderer || !surface)
	{
		errno = EINVAL;
		return dsGfxFormat_Unknown;
	}

	switch (surface->surfaceType)
	{
		case dsGfxSurfaceType_ColorRenderSurface:
		case dsGfxSurfaceType_ColorRenderSurfaceLeft:
		case dsGfxSurfaceType_ColorRenderSurfaceRight:
			return renderer->surfaceColorFormat;
		case dsGfxSurfaceType_DepthRenderSurface:
		case dsGfxSurfaceType_DepthRenderSurfaceLeft:
		case dsGfxSurfaceType_DepthRenderSurfaceRight:
			return renderer->surfaceDepthStencilFormat;
		case dsGfxSurfaceType_Offscreen:
			return ((const dsOffscreen*)surface->surface)->info.format;
		case dsGfxSurfaceType_Renderbuffer:
			return ((const dsRenderbuffer*)surface->surface)->format;
		default:
			DS_ASSERT(false);
			return dsGfxFormat_Unknown;
	}
}

bool dsFramebuffer_destroy(dsFramebuffer* framebuffer)
{
	if (!framebuffer)
		return true;

	DS_PROFILE_FUNC_START();

	if (!framebuffer->resourceManager || !framebuffer->resourceManager->destroyRenderbufferFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = framebuffer->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->destroyFramebufferFunc(resourceManager, framebuffer);
	if (success)
		DS_ATOMIC_FETCH_ADD32(&resourceManager->framebufferCount, -1);
	DS_PROFILE_FUNC_RETURN(success);
}

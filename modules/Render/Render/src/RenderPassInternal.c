/*
 * Copyright 2025-2026 Aaron Barany
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

#include "RenderPassInternal.h"

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/Framebuffer.h>

static uint32_t getSurfaceSamples(dsRenderer* renderer, const dsFramebufferSurface* surface)
{
	switch (surface->surfaceType)
	{
		case dsGfxSurfaceType_ColorRenderSurface:
		case dsGfxSurfaceType_ColorRenderSurfaceLeft:
		case dsGfxSurfaceType_ColorRenderSurfaceRight:
		case dsGfxSurfaceType_DepthRenderSurface:
		case dsGfxSurfaceType_DepthRenderSurfaceLeft:
		case dsGfxSurfaceType_DepthRenderSurfaceRight:
			return renderer->surfaceSamples;
		case dsGfxSurfaceType_Offscreen:
			return ((dsOffscreen*)surface->surface)->info.samples;
		case dsGfxSurfaceType_Renderbuffer:
			return ((dsRenderbuffer*)surface->surface)->samples;
		default:
			DS_ASSERT(false);
			return 0;
	}
}

static bool canKeepSurface(const dsFramebufferSurface* surface)
{
	switch (surface->surfaceType)
	{
		case dsGfxSurfaceType_ColorRenderSurface:
		case dsGfxSurfaceType_ColorRenderSurfaceLeft:
		case dsGfxSurfaceType_ColorRenderSurfaceRight:
			return true;
		case dsGfxSurfaceType_DepthRenderSurface:
		case dsGfxSurfaceType_DepthRenderSurfaceLeft:
		case dsGfxSurfaceType_DepthRenderSurfaceRight:
		{
			const dsRenderSurface* renderSurface = (const dsRenderSurface*)surface->surface;
			return (renderSurface->usage &
				(dsRenderSurfaceUsage_ContinueDepthStencil |
					dsRenderSurfaceUsage_BlitDepthStencilFrom)) != 0;
		}
		case dsGfxSurfaceType_Offscreen:
			return true;
		case dsGfxSurfaceType_Renderbuffer:
		{
			const dsRenderbuffer* renderbuffer = (const dsRenderbuffer*)surface->surface;
			return (renderbuffer->usage &
				(dsRenderbufferUsage_Continue | dsRenderbufferUsage_BlitFrom)) != 0;
		}
		default:
			DS_ASSERT(false);
			return true;
	}
}

bool dsRenderPass_canUseFramebuffer(const dsRenderPass* renderPass,
	const dsCommandBuffer* commandBuffer, const dsFramebuffer* framebuffer)
{
	DS_ASSERT(renderPass);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(framebuffer);

	dsRenderer* renderer = renderPass->renderer;
	for (uint32_t i = 0; i < framebuffer->surfaceCount; ++i)
	{
		if (dsFramebuffer_getSurfaceFormat(renderer, framebuffer->surfaces + i) !=
			renderPass->attachments[i].format)
		{
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Framebuffer surface format doesn't match attachment format.");
			return false;
		}

		uint32_t samples = renderPass->attachments[i].samples;
		if (samples == DS_SURFACE_ANTIALIAS_SAMPLES)
			samples = renderer->surfaceSamples;
		else if (samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
			samples = renderer->defaultSamples;
		if (getSurfaceSamples(renderer, framebuffer->surfaces + i) != samples)
		{
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Framebuffer surface samples don't match attachment samples.");
			return false;
		}

		if ((renderPass->attachments[i].usage & dsAttachmentUsage_KeepAfter) &&
			!canKeepSurface(framebuffer->surfaces + i))
		{
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Can't use dsAttachmentUsage_KeepAfter with a "
				"surface without the continue or blit from usage flag.");
			return false;
		}

		dsGfxSurfaceType surfaceType = framebuffer->surfaces[i].surfaceType;
		if ((commandBuffer->usage & dsCommandBufferUsage_MultiFrame) &&
			surfaceType >= dsGfxSurfaceType_ColorRenderSurface &&
			surfaceType <= dsGfxSurfaceType_DepthRenderSurfaceRight)
		{
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Can't draw a render pass to a framebuffer containing "
				"a render surface when using a multiframe command buffer.");
			return false;
		}
	}

	return true;
}

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

#include <DeepSea/Render/RenderPass.h>

#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Render/Resources/GfxFormat.h>

typedef enum SurfaceType
{
	SurfaceType_Unset = 0,
	SurfaceType_Left = 0x1,
	SurfaceType_Right = 0x2,
	SurfaceType_Other = 0x4
} SurfaceType;

static bool isDepthStencil(dsGfxFormat format)
{
	return format >= dsGfxFormat_D16 && format <= dsGfxFormat_D32S8_Float;
}

static SurfaceType getSurfaceType(dsFramebufferSurfaceType framebufferSurfaceType)
{
	switch (framebufferSurfaceType)
	{
		case dsFramebufferSurfaceType_ColorRenderSurface:
		case dsFramebufferSurfaceType_ColorRenderSurfaceLeft:
		case dsFramebufferSurfaceType_DepthRenderSurface:
		case dsFramebufferSurfaceType_DepthRenderSurfaceLeft:
			return SurfaceType_Left;
		case dsFramebufferSurfaceType_ColorRenderSurfaceRight:
		case dsFramebufferSurfaceType_DepthRenderSurfaceRight:
			return SurfaceType_Right;
		case dsFramebufferSurfaceType_Offscreen:
		case dsFramebufferSurfaceType_Renderbuffer:
			return SurfaceType_Other;
		default:
			DS_ASSERT(false);
			return SurfaceType_Unset;
	}
}

static bool hasMultipleSurfaceTypes(SurfaceType surfaceType)
{
	unsigned int count = 0;
	count += (surfaceType & SurfaceType_Left) != 0;
	count += (surfaceType & SurfaceType_Right) != 0;
	count += (surfaceType & SurfaceType_Other) != 0;
	return count > 0;
}

static dsGfxFormat getSurfaceFormat(dsRenderer* renderer, const dsFramebufferSurface* surface)
{
	switch (surface->surfaceType)
	{
		case dsFramebufferSurfaceType_ColorRenderSurface:
		case dsFramebufferSurfaceType_ColorRenderSurfaceLeft:
		case dsFramebufferSurfaceType_ColorRenderSurfaceRight:
			return renderer->surfaceColorFormat;
		case dsFramebufferSurfaceType_DepthRenderSurface:
		case dsFramebufferSurfaceType_DepthRenderSurfaceLeft:
		case dsFramebufferSurfaceType_DepthRenderSurfaceRight:
			return renderer->surfaceDepthStencilFormat;
		case dsFramebufferSurfaceType_Offscreen:
			return ((dsOffscreen*)surface->surface)->format;
		case dsFramebufferSurfaceType_Renderbuffer:
			return ((dsRenderbuffer*)surface->surface)->format;
		default:
			DS_ASSERT(false);
			return dsGfxFormat_Unknown;
	}
}

static uint16_t getSurfaceSamples(dsRenderer* renderer, const dsFramebufferSurface* surface)
{
	switch (surface->surfaceType)
	{
		case dsFramebufferSurfaceType_ColorRenderSurface:
		case dsFramebufferSurfaceType_ColorRenderSurfaceLeft:
		case dsFramebufferSurfaceType_ColorRenderSurfaceRight:
		case dsFramebufferSurfaceType_DepthRenderSurface:
		case dsFramebufferSurfaceType_DepthRenderSurfaceLeft:
		case dsFramebufferSurfaceType_DepthRenderSurfaceRight:
			return renderer->surfaceSamples;
		case dsFramebufferSurfaceType_Offscreen:
			return ((dsOffscreen*)surface->surface)->samples;
		case dsFramebufferSurfaceType_Renderbuffer:
			return ((dsRenderbuffer*)surface->surface)->samples;
		default:
			DS_ASSERT(false);
			return 0;
	}
}

dsRenderPass* dsRenderPass_create(dsRenderer* renderer, dsAllocator* allocator,
	const dsAttachmentInfo* attachments, uint32_t attachmentCount,
	const dsRenderSubpassInfo* subpasses, uint32_t subpassCount,
	const dsSubpassDependency* dependencies, uint32_t dependencyCount)
{
	DS_PROFILE_FUNC_START();

	if (!renderer || (!allocator && !renderer->allocator) || !renderer->createRenderPassFunc ||
		!renderer->destroyRenderPassFunc || (!attachments && attachmentCount > 0) || !subpasses ||
		subpassCount == 0 || (!dependencies && dependencyCount > 0))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	for (uint32_t i = 0; i < attachmentCount; ++i)
	{
		if (!dsGfxFormat_offscreenSupported(renderer->resourceManager, attachments[i].format))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Input attachment format cannot be rendered to.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}
	}

	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		if (!subpasses[i].inputAttachments && subpasses[i].inputAttachmentCount > 0)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid subpass input attachments.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		for (uint32_t j = 0; j < subpasses[i].inputAttachmentCount; ++j)
		{
			if (subpasses[i].inputAttachments[j] >= attachmentCount)
			{
				errno = EINDEX;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Subpass input attachment out of range.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}
		}

		if (!subpasses[i].colorAttachments && subpasses[i].colorAttachmentCount > 0)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid subpass color attachments.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		if (renderer->resourceManager->requiresColorBuffer)
		{
			bool allColorAttachmentsSet = subpasses[i].colorAttachmentCount > 0;
			for (uint32_t j = 0; j < subpasses[j].colorAttachmentCount; ++j)
			{
				if (subpasses[i].colorAttachments[j].attachmentIndex == DS_NO_ATTACHMENT)
				{
					allColorAttachmentsSet = false;
					break;
				}
			}

			if (!allColorAttachmentsSet)
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Current target requires at color attachments to "
					"be set for each render subplass.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}
		}

		if (subpasses[i].colorAttachmentCount > renderer->maxColorAttachments)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Render subpass color attachments exceeds the maximum for the current target.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		for (uint32_t j = 0; j < subpasses[i].colorAttachmentCount; ++j)
		{
			uint32_t attachment = subpasses[i].colorAttachments[j].attachmentIndex;
			if (attachment == DS_NO_ATTACHMENT)
				continue;

			if (attachment >= attachmentCount)
			{
				errno = EINDEX;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Subpass color attachment out of range.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}

			if (isDepthStencil(attachments[attachment].format))
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG,
					"Cannot use a depth-stencil surface as a color attachment.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}
		}

		if (subpasses[i].depthStencilAttachment != DS_NO_ATTACHMENT)
		{
			if (subpasses[i].depthStencilAttachment >= attachmentCount)
			{
				errno = EINDEX;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Subpass depth-stencil attachment out of range.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}

			if (!isDepthStencil(attachments[subpasses[i].depthStencilAttachment].format))
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG,
					"Cannot use a color surface as a depth-stencil attachment.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}
		}
	}

	for (uint32_t i = 0; i < dependencyCount; ++i)
	{
		if (dependencies[i].srcSubpass >= subpassCount ||
			dependencies[i].dstSubpass >= subpassCount)
		{
			errno = EINDEX;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Subpass dependencies out of range.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}
	}

	if (!allocator)
		allocator = renderer->allocator;

	if (!dsThread_equal(dsThread_thisThreadId(), renderer->mainThread))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Render passes may only be created on the main thread.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsRenderPass* renderPass = renderer->createRenderPassFunc(renderer, allocator, attachments,
		attachmentCount, subpasses, subpassCount, dependencies, dependencyCount);
	DS_PROFILE_FUNC_RETURN(renderPass);
}

bool dsRenderPass_begin(dsCommandBuffer* commandBuffer, const dsRenderPass* renderPass,
	const dsFramebuffer* framebuffer, const dsAlignedBox3f* viewport,
	const dsSurfaceClearValue* clearValues, uint32_t clearValueCount, bool indirectCommands)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !renderPass || !renderPass->renderer ||
		!renderPass->renderer->beginRenderPassFunc ||
		!renderPass->renderer->nextRenderSubpassFunc || !renderPass->renderer->endRenderPassFunc ||
		!framebuffer)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (framebuffer->surfaceCount != renderPass->attachmentCount)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Framebuffer not compatible with render pass attachments.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsRenderer* renderer = renderPass->renderer;
	bool needsClear = false;
	for (uint32_t i = 0; i < framebuffer->surfaceCount; ++i)
	{
		if (getSurfaceFormat(renderer, framebuffer->surfaces + i) !=
			renderPass->attachments[i].format)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Framebuffer surface format doesn't match attachment format.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		uint32_t samples = renderPass->attachments[i].samples;
		if (samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
			samples = renderer->surfaceSamples;
		if (getSurfaceSamples(renderer, framebuffer->surfaces + i) != samples)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Framebuffer surface samples don't match attachment samples.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		if (renderPass->attachments[i].usage & dsAttachmentUsage_Clear)
			needsClear = true;
	}

	if (!renderer->resourceManager->canMixWithRenderSurface)
	{
		for (uint32_t i = 0; i < renderPass->subpassCount; ++i)
		{
			const dsRenderSubpassInfo* subpass = renderPass->subpasses + i;
			for (uint32_t j = 0; j < subpass->colorAttachmentCount; ++j)
			{
				SurfaceType surfaceTypes = SurfaceType_Unset;
				bool hasAnyColorSurface = false;
				for (uint32_t k = 0; k < subpass->colorAttachmentCount; ++k)
				{
					uint32_t attachment = subpass->colorAttachments[k].attachmentIndex;
					if (attachment != DS_NO_ATTACHMENT)
					{
						hasAnyColorSurface = true;
						surfaceTypes = (SurfaceType)(surfaceTypes |
							getSurfaceType(framebuffer->surfaces[attachment].surfaceType));
					}
				}

				if (subpass->depthStencilAttachment != DS_NO_ATTACHMENT)
				{
					surfaceTypes = (SurfaceType)(surfaceTypes | getSurfaceType(
						framebuffer->surfaces[subpass->depthStencilAttachment].surfaceType));
				}

				if (renderer->resourceManager->requiresAnySurface &&
					!hasAnyColorSurface && subpass->depthStencilAttachment == DS_NO_ATTACHMENT )
				{
					errno = EPERM;
					DS_LOG_ERROR(DS_RENDER_LOG_TAG,
						"Current target requires at least one surface in a subpass.");
					DS_PROFILE_FUNC_RETURN(false);
				}

				if (hasMultipleSurfaceTypes(surfaceTypes))
				{
					errno = EPERM;
					DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Current target doesn't support mixing the "
						"main framebuffer and other render surfaces.");
					DS_PROFILE_FUNC_RETURN(false);
				}
			}
		}
	}

	if (viewport && (viewport->min.x < 0 || viewport->min.y < 0 || viewport->min.z < 0 ||
		viewport->max.x > framebuffer->width || viewport->max.y > framebuffer->height ||
		viewport->max.z > 1))
	{
		errno = ERANGE;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Viewport is out of range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (needsClear && clearValueCount == 0)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"No clear values provided for render pass that clears attachments.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if ((!clearValues && clearValueCount > 0) ||
		(clearValues != 0 && clearValueCount != renderPass->attachmentCount))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"When clear values are provided they must equal the number of attachments.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->beginRenderPassFunc(renderer, commandBuffer, renderPass, framebuffer,
		viewport, clearValues, clearValueCount, indirectCommands);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsRenderPass_nextSubpass(dsCommandBuffer* commandBuffer, const dsRenderPass* renderPass,
	bool indirectCommands)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !renderPass || !renderPass->renderer ||
		!renderPass->renderer->nextRenderSubpassFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsRenderer* renderer = renderPass->renderer;
	bool success = renderer->nextRenderSubpassFunc(renderer, commandBuffer, renderPass,
		indirectCommands);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsRenderPass_end(dsCommandBuffer* commandBuffer, const dsRenderPass* renderPass)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !renderPass || !renderPass->renderer ||
		!renderPass->renderer->endRenderPassFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsRenderer* renderer = renderPass->renderer;
	bool success = renderer->endRenderPassFunc(renderer, commandBuffer, renderPass);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsRenderPass_destroy(dsRenderPass* renderPass)
{
	DS_PROFILE_FUNC_START();

	if (!renderPass || !renderPass->renderer ||
		!renderPass->renderer->destroyRenderPassFunc)
	{
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsRenderer* renderer = renderPass->renderer;
	if (!dsThread_equal(dsThread_thisThreadId(), renderer->mainThread))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Render passes may only be destroyed on the main thread.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool destroyed = renderer->destroyRenderPassFunc(renderer, renderPass);
	DS_PROFILE_FUNC_RETURN(destroyed);
}

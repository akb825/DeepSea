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

#include <DeepSea/Render/RenderPass.h>

#include "GPUProfileContext.h"
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Render/Resources/Framebuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <stdio.h>

typedef enum SurfaceType
{
	SurfaceType_Unset = 0,
	SurfaceType_Left = 0x1,
	SurfaceType_Right = 0x2,
	SurfaceType_Other = 0x4
} SurfaceType;

#define SCOPE_SIZE 256

static bool startRenderPassScope(const dsRenderPass* renderPass, dsCommandBuffer* commandBuffer,
	const dsFramebuffer* framebuffer, const dsAlignedBox3f* viewport)
{
	// Error checking for this will be later.
	if (!renderPass || !commandBuffer || !framebuffer)
		return true;

	if (!commandBuffer->frameActive)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "A render pass must start within a frame.");
		return false;
	}

	if (commandBuffer->boundRenderPass)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot start a render pass when another render pass is active.");
		return false;
	}

	if (commandBuffer->boundComputeShader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot start a render pass when a compute shader is bound.");
		return false;
	}

	if (viewport && (viewport->min.x < 0 || viewport->min.y < 0 || viewport->min.z < 0 ||
		viewport->max.x > framebuffer->width || viewport->max.y > framebuffer->height ||
		viewport->max.z > 1))
	{
		errno = ERANGE;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Viewport is out of range.");
		return false;
	}

#if DS_PROFILING_ENABLED

	char buffer[SCOPE_SIZE];
	int result = snprintf(buffer, SCOPE_SIZE, "%s: %s", framebuffer->name,
		renderPass->subpasses[0].name);
	DS_UNUSED(result);
	DS_ASSERT(result > 0 && result < SCOPE_SIZE);

	DS_ASSERT(renderPass->subpassCount > 0);
	DS_PROFILE_DYNAMIC_SCOPE_START(buffer);

#endif

	commandBuffer->boundFramebuffer = framebuffer;
	commandBuffer->boundRenderPass = renderPass;
	commandBuffer->activeRenderSubpass = 0;
	if (viewport)
		commandBuffer->viewport = *viewport;
	else
	{
		commandBuffer->viewport.min.x = 0.0f;
		commandBuffer->viewport.min.y = 0.0f;
		commandBuffer->viewport.min.z = 0.0f;
		commandBuffer->viewport.max.x = (float)framebuffer->width;
		commandBuffer->viewport.max.y = (float)framebuffer->height;
		commandBuffer->viewport.max.z = 1.0f;
	}
	return true;
}

static bool nextSubpassScope(const dsRenderPass* renderPass, dsCommandBuffer* commandBuffer)
{
	// Error checking for this will be later.
	if (!renderPass || !commandBuffer)
		return true;

	if (commandBuffer->boundRenderPass != renderPass)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Can only go to the next subpass for the currently active render pass.");
		return false;
	}

	if (commandBuffer->activeRenderSubpass + 1 >= renderPass->subpassCount)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Already at the end of the current render pass.");
		return false;
	}

	if (commandBuffer->boundShader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot go to the next subpass while a shader is bound.");
		return false;
	}

	++commandBuffer->activeRenderSubpass;

#if DS_PROFILING_ENABLED

	char buffer[SCOPE_SIZE];
	int result = snprintf(buffer, SCOPE_SIZE, "%s: %s", commandBuffer->boundFramebuffer->name,
		renderPass->subpasses[commandBuffer->activeRenderSubpass].name);
	DS_UNUSED(result);
	DS_ASSERT(result > 0 && result < SCOPE_SIZE);

	DS_PROFILE_SCOPE_END();
	DS_PROFILE_DYNAMIC_SCOPE_START(buffer);

#endif

	return true;
}

static void restorePreviousSubpassScope(const dsRenderPass* renderPass,
	dsCommandBuffer* commandBuffer)
{
	if (!renderPass || !commandBuffer || commandBuffer->boundRenderPass != renderPass)
		return;

	DS_ASSERT(commandBuffer->activeRenderSubpass > 0);
	--commandBuffer->activeRenderSubpass;

#if DS_PROFILING_ENABLED

	char buffer[SCOPE_SIZE];
	int result = snprintf(buffer, SCOPE_SIZE, "%s: %s", commandBuffer->boundFramebuffer->name,
		renderPass->subpasses[commandBuffer->activeRenderSubpass].name);
	DS_UNUSED(result);
	DS_ASSERT(result > 0 && result < SCOPE_SIZE);

	DS_PROFILE_SCOPE_END();
	DS_PROFILE_DYNAMIC_SCOPE_START(buffer);

#endif
}

static void endRenderPassScope(dsCommandBuffer* commandBuffer)
{
	if (!commandBuffer || !commandBuffer->boundRenderPass)
		return;

	DS_PROFILE_SCOPE_END();
	commandBuffer->boundFramebuffer = NULL;
	commandBuffer->boundRenderPass = NULL;
	commandBuffer->activeRenderSubpass = 0;;
}

static SurfaceType getSurfaceType(dsGfxSurfaceType surfaceType)
{
	switch (surfaceType)
	{
		case dsGfxSurfaceType_ColorRenderSurface:
		case dsGfxSurfaceType_ColorRenderSurfaceLeft:
		case dsGfxSurfaceType_DepthRenderSurface:
		case dsGfxSurfaceType_DepthRenderSurfaceLeft:
			return SurfaceType_Left;
		case dsGfxSurfaceType_ColorRenderSurfaceRight:
		case dsGfxSurfaceType_DepthRenderSurfaceRight:
			return SurfaceType_Right;
		case dsGfxSurfaceType_Offscreen:
		case dsGfxSurfaceType_Renderbuffer:
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
	return count > 1;
}

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

static bool canResolveSurface(const dsFramebufferSurface* surface)
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
			return false;
		case dsGfxSurfaceType_Offscreen:
			return ((const dsOffscreen*)surface->surface)->resolve;
		case dsGfxSurfaceType_Renderbuffer:
			return false;
		default:
			DS_ASSERT(false);
			return true;
	}
}

static bool canKeepRenderbuffer(const dsFramebufferSurface* surface)
{
	switch (surface->surfaceType)
	{
		case dsGfxSurfaceType_ColorRenderSurface:
		case dsGfxSurfaceType_ColorRenderSurfaceLeft:
		case dsGfxSurfaceType_ColorRenderSurfaceRight:
		case dsGfxSurfaceType_DepthRenderSurface:
		case dsGfxSurfaceType_DepthRenderSurfaceLeft:
		case dsGfxSurfaceType_DepthRenderSurfaceRight:
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

static bool canContinueOffscreen(const dsFramebufferSurface* surface)
{
	switch (surface->surfaceType)
	{
		case dsGfxSurfaceType_ColorRenderSurface:
		case dsGfxSurfaceType_ColorRenderSurfaceLeft:
		case dsGfxSurfaceType_ColorRenderSurfaceRight:
		case dsGfxSurfaceType_DepthRenderSurface:
		case dsGfxSurfaceType_DepthRenderSurfaceLeft:
		case dsGfxSurfaceType_DepthRenderSurfaceRight:
		case dsGfxSurfaceType_Renderbuffer:
			return false;
		case dsGfxSurfaceType_Offscreen:
		{
			const dsOffscreen* offscreen = (const dsOffscreen*)surface->surface;
			return (offscreen->usage & dsTextureUsage_OffscreenContinue) != 0;
		}
		default:
			DS_ASSERT(false);
			return true;
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
		subpassCount == 0 || (!dependencies && dependencyCount > 0 &&
			dependencyCount != DS_DEFAULT_SUBPASS_DEPENDENCIES))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	for (uint32_t i = 0; i < attachmentCount; ++i)
	{
		if (attachments[i].format != renderer->surfaceColorFormat &&
			attachments[i].format != renderer->surfaceDepthStencilFormat &&
			!dsGfxFormat_offscreenSupported(renderer->resourceManager, attachments[i].format))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attachment format cannot be rendered to.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		if (attachments[i].samples == 0)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Attachment samples must be set to a value greater than 0.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}
	}

	dsResourceManager* resourceManager = renderer->resourceManager;
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

		if (subpasses[i].colorAttachmentCount > renderer->maxColorAttachments)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Render subpass color attachments exceeds the maximum for the current target.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		bool anyColorAttachmentSet = false;
		uint32_t samples = 0;
		for (uint32_t j = 0; j < subpasses[i].colorAttachmentCount; ++j)
		{
			uint32_t attachment = subpasses[i].colorAttachments[j].attachmentIndex;
			if (attachment == DS_NO_ATTACHMENT)
				continue;

			anyColorAttachmentSet = true;
			if (attachment >= attachmentCount)
			{
				errno = EINDEX;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Subpass color attachment out of range.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}

			if (dsGfxFormat_isDepthStencil(attachments[attachment].format))
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG,
					"Cannot use a depth-stencil surface as a color attachment.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}

			if (samples == 0)
				samples = attachments[attachment].samples;
			else if (samples != attachments[attachment].samples)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "All color and depth attachments must have the "
					"same number of anti-alias samples.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}
		}

		if (resourceManager->requiresColorBuffer && !anyColorAttachmentSet)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Current target requires a color buffer to be set in a render subpass.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}

		if (subpasses[i].depthStencilAttachment != DS_NO_ATTACHMENT)
		{
			if (subpasses[i].depthStencilAttachment >= attachmentCount)
			{
				errno = EINDEX;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Subpass depth-stencil attachment out of range.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}

			if (!dsGfxFormat_isDepthStencil(
				attachments[subpasses[i].depthStencilAttachment].format))
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG,
					"Cannot use a color surface as a depth-stencil attachment.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}

			if (samples && samples != attachments[subpasses[i].depthStencilAttachment].samples)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "All color and depth attachments must have the "
					"same number of anti-alias samples.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}
		}

		if (resourceManager->requiresAnySurface && !anyColorAttachmentSet &&
			subpasses[i].depthStencilAttachment == DS_NO_ATTACHMENT)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Current target requires at least one surface in a render subpass.");
			DS_PROFILE_FUNC_RETURN(NULL);
		}
	}

	if (dependencyCount != DS_DEFAULT_SUBPASS_DEPENDENCIES)
	{
		for (uint32_t i = 0; i < dependencyCount; ++i)
		{
			if (dependencies[i].srcSubpass == DS_EXTERNAL_SUBPASS &&
				dependencies[i].dstSubpass == DS_EXTERNAL_SUBPASS)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG,
					"Source and destination subpasses for a dependency cannot both be external.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}

			if ((dependencies[i].srcSubpass != DS_EXTERNAL_SUBPASS &&
					dependencies[i].srcSubpass >= subpassCount) ||
				(dependencies[i].dstSubpass != DS_EXTERNAL_SUBPASS &&
					dependencies[i].dstSubpass >= subpassCount))
			{
				errno = EINDEX;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Subpass dependencies out of range.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}

			if (dependencies[i].srcSubpass != DS_EXTERNAL_SUBPASS &&
				dependencies[i].dstSubpass != DS_EXTERNAL_SUBPASS &&
				dependencies[i].srcSubpass > dependencies[i].dstSubpass)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Subpasses may only depend on previous subpasses.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}

			if (dependencies[i].srcStages == 0 || dependencies[i].dstStages == 0)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG,
					"At least one stage flag must be provided for subpass dependencies.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}
		}
	}

	if (!allocator)
		allocator = renderer->allocator;

	dsRenderPass* renderPass = renderer->createRenderPassFunc(renderer, allocator, attachments,
		attachmentCount, subpasses, subpassCount, dependencies, dependencyCount);
	DS_PROFILE_FUNC_RETURN(renderPass);
}

bool dsRenderPass_begin(const dsRenderPass* renderPass, dsCommandBuffer* commandBuffer,
	const dsFramebuffer* framebuffer, const dsAlignedBox3f* viewport,
	const dsSurfaceClearValue* clearValues, uint32_t clearValueCount)
{
	if (!startRenderPassScope(renderPass, commandBuffer, framebuffer, viewport))
		return false;

	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !renderPass || !renderPass->renderer ||
		!renderPass->renderer->beginRenderPassFunc ||
		!renderPass->renderer->nextRenderSubpassFunc || !renderPass->renderer->endRenderPassFunc ||
		!framebuffer)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_END();
		endRenderPassScope(commandBuffer);
		return false;
	}

	if (framebuffer->surfaceCount != renderPass->attachmentCount)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Framebuffer not compatible with render pass attachments.");
		DS_PROFILE_FUNC_END();
		endRenderPassScope(commandBuffer);
		return false;
	}

	dsRenderer* renderer = renderPass->renderer;
	bool needsClear = false;
	for (uint32_t i = 0; i < framebuffer->surfaceCount; ++i)
	{
		if (dsFramebuffer_getSurfaceFormat(renderer, framebuffer->surfaces + i) !=
			renderPass->attachments[i].format)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Framebuffer surface format doesn't match attachment format.");
			DS_PROFILE_FUNC_END();
			endRenderPassScope(commandBuffer);
			return false;
		}

		uint32_t samples = renderPass->attachments[i].samples;
		if (samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
			samples = renderer->surfaceSamples;
		if (getSurfaceSamples(renderer, framebuffer->surfaces + i) != samples)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Framebuffer surface samples don't match attachment samples.");
			DS_PROFILE_FUNC_END();
			endRenderPassScope(commandBuffer);
			return false;
		}

		if (renderPass->attachments[i].usage & dsAttachmentUsage_Clear)
			needsClear = true;

		if ((renderPass->attachments[i].usage & dsAttachmentUsage_KeepAfter) &&
			!canKeepRenderbuffer(framebuffer->surfaces + i))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Can't use dsAttachmentUsage_KeepAfter with a "
				"dsRenderbuffer without the dsRenderbufferUsage_Continue or "
				"dsRenderbufferUsage_BlitFrom usage flag.");
			DS_PROFILE_FUNC_END();
			endRenderPassScope(commandBuffer);
			return false;
		}

		if ((renderPass->attachments[i].usage & dsAttachmentUsage_Resolve) &&
			!canContinueOffscreen(framebuffer->surfaces + i))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resolving an offscreen after the full render pass "
				"requires the dsTextureUsage_OffscreenContinue usage flag.");
			DS_PROFILE_FUNC_END();
			endRenderPassScope(commandBuffer);
			return false;
		}
	}

	for (uint32_t i = 0; i < renderPass->subpassCount; ++i)
	{
		const dsRenderSubpassInfo* subpass = renderPass->subpasses + i;
		for (uint32_t j = 0; j < subpass->inputAttachmentCount; ++j)
		{
			if (framebuffer->surfaces[subpass->inputAttachments[j]].surfaceType !=
				dsGfxSurfaceType_Offscreen)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Subpass inputs must be offscreens.");
				DS_PROFILE_FUNC_END();
				endRenderPassScope(commandBuffer);
				return false;
			}
		}

		if (!renderer->resourceManager->canMixWithRenderSurface)
		{
			SurfaceType surfaceTypes = SurfaceType_Unset;
			for (uint32_t j = 0; j < subpass->colorAttachmentCount; ++j)
			{
				uint32_t attachment = subpass->colorAttachments[j].attachmentIndex;
				if (attachment != DS_NO_ATTACHMENT)
				{
					surfaceTypes = (SurfaceType)(surfaceTypes |
						getSurfaceType(framebuffer->surfaces[attachment].surfaceType));
				}
			}

			if (subpass->depthStencilAttachment != DS_NO_ATTACHMENT)
			{
				surfaceTypes = (SurfaceType)(surfaceTypes | getSurfaceType(
					framebuffer->surfaces[subpass->depthStencilAttachment].surfaceType));
			}

			if (hasMultipleSurfaceTypes(surfaceTypes))
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Current target doesn't support mixing the "
					"main framebuffer and other render surfaces.");
				DS_PROFILE_FUNC_END();
				endRenderPassScope(commandBuffer);
				return false;
			}
		}

		for (uint32_t j = 0; j < subpass->colorAttachmentCount; ++j)
		{
			if (!subpass->colorAttachments[j].resolve)
				continue;

			uint32_t attachment = subpass->colorAttachments[j].attachmentIndex;
			if (attachment == DS_NO_ATTACHMENT)
				continue;

			// Don't check for resolve when no anti-aliasing since offscreens no longer resolve,
			// which would give a false positive.
			uint32_t samples = renderPass->attachments[attachment].samples;
			if (samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
				samples = renderer->surfaceSamples;

			if (samples > 1 && !canResolveSurface(framebuffer->surfaces + attachment))
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG,
					"Color attachment set to resolve used with unresolvable framebuffer surface.");
				DS_PROFILE_FUNC_END();
				endRenderPassScope(commandBuffer);
				return false;
			}
		}
	}

	if (needsClear && clearValueCount == 0)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"No clear values provided for render pass that clears attachments.");
		DS_PROFILE_FUNC_END();
		endRenderPassScope(commandBuffer);
		return false;
	}

	if ((!clearValues && clearValueCount > 0) ||
		(clearValueCount != 0 && clearValueCount != renderPass->attachmentCount))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"When clear values are provided they must equal the number of attachments.");
		DS_PROFILE_FUNC_END();
		endRenderPassScope(commandBuffer);
		return false;
	}

	dsGPUProfileContext_beginSubpass(renderer->_profileContext, commandBuffer,
		framebuffer->name, renderPass->subpasses[0].name);
	bool success = renderer->beginRenderPassFunc(renderer, commandBuffer, renderPass, framebuffer,
		viewport, clearValues, clearValueCount);
	DS_PROFILE_FUNC_END();
	if (!success)
	{
		dsGPUProfileContext_endSubpass(renderer->_profileContext, commandBuffer);
		endRenderPassScope(commandBuffer);
	}
	return success;
}

bool dsRenderPass_nextSubpass(const dsRenderPass* renderPass, dsCommandBuffer* commandBuffer)
{
	// End the previous scope
	if (!nextSubpassScope(renderPass, commandBuffer))
		return false;

	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !renderPass || !renderPass->renderer ||
		!renderPass->renderer->nextRenderSubpassFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_END();
		restorePreviousSubpassScope(renderPass, commandBuffer);
		return false;
	}

	dsRenderer* renderer = renderPass->renderer;
	bool success = renderer->nextRenderSubpassFunc(renderer, commandBuffer, renderPass,
		commandBuffer->activeRenderSubpass);
	if (success)
	{
		dsGPUProfileContext_nextSubpass(renderer->_profileContext, commandBuffer,
			renderPass->subpasses[commandBuffer->activeRenderSubpass].name);
	}
	DS_PROFILE_FUNC_END();
	if (!success)
		restorePreviousSubpassScope(renderPass, commandBuffer);
	return success;
}

bool dsRenderPass_end(const dsRenderPass* renderPass, dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !renderPass || !renderPass->renderer ||
		!renderPass->renderer->endRenderPassFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundRenderPass != renderPass)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Can only end the currently active render pass.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->activeRenderSubpass != renderPass->subpassCount - 1)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Must draw all render subpasses.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundShader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot end a render pass while a shader is bound.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsRenderer* renderer = renderPass->renderer;
	bool success = renderer->endRenderPassFunc(renderer, commandBuffer, renderPass);
	if (success)
		dsGPUProfileContext_endSubpass(renderer->_profileContext, commandBuffer);
	DS_PROFILE_FUNC_END();
	if (success)
		endRenderPassScope(commandBuffer);
	return success;
}

bool dsRenderPass_destroy(dsRenderPass* renderPass)
{
	if (!renderPass)
		return true;

	DS_PROFILE_FUNC_START();

	if (!renderPass->renderer || !renderPass->renderer->destroyRenderPassFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsRenderer* renderer = renderPass->renderer;
	bool destroyed = renderer->destroyRenderPassFunc(renderer, renderPass);
	DS_PROFILE_FUNC_RETURN(destroyed);
}

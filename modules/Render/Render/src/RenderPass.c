/*
 * Copyright 2017-2026 Aaron Barany
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
#include "RenderPassInternal.h"

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

typedef enum AttachmentUsage
{
	AttachmentUsage_Unused = 0,
	AttachmentUsage_Input = 0x1,
	AttachmentUsage_Color = 0x2,
	AttachmentUsage_DepthStencil = 0x4,
} AttachmentUsage;

#define SCOPE_SIZE 256

static bool hasAttachment(const dsRenderSubpassInfo* subpass, uint32_t attachment, bool checkInputs)
{
	if (attachment == DS_NO_ATTACHMENT)
		return false;

	if (checkInputs)
	{
		for (uint32_t i = 0; i < subpass->inputAttachmentCount; ++i)
		{
			if (subpass->inputAttachments[i] == attachment)
				return true;
		}
	}

	for (uint32_t i = 0; i < subpass->colorAttachmentCount; ++i)
	{
		if (subpass->colorAttachments[i].attachmentIndex == attachment)
			return true;
	}

	return subpass->depthStencilAttachment.attachmentIndex == attachment;
}

static AttachmentUsage getAttachmentUsage(const dsRenderSubpassInfo* subpass, uint32_t attachment)
{
	AttachmentUsage attachmentUsage = AttachmentUsage_Unused;
	if (attachment == DS_NO_ATTACHMENT)
		return attachmentUsage;

	for (uint32_t i = 0; i < subpass->inputAttachmentCount; ++i)
	{
		if (subpass->inputAttachments[i] == attachment)
		{
			attachmentUsage |= AttachmentUsage_Input;
			break;
		}
	}

	for (uint32_t i = 0; i < subpass->colorAttachmentCount; ++i)
	{
		if (subpass->colorAttachments[i].attachmentIndex == attachment)
		{
			attachmentUsage |= AttachmentUsage_Color;
			break;
		}
	}

	if (subpass->depthStencilAttachment.attachmentIndex == attachment)
		attachmentUsage |= AttachmentUsage_DepthStencil;

	return attachmentUsage;
}

static bool sharesAttachments(
	const dsRenderSubpassInfo* firstSubpass, const dsRenderSubpassInfo* secondSubpass)
{
	for (uint32_t i = 0; i < firstSubpass->inputAttachmentCount; ++i)
	{
		// Don't care about sharing input attachments.
		if (hasAttachment(secondSubpass, firstSubpass->inputAttachments[i], false))
			return true;
	}

	for (uint32_t i = 0; i < firstSubpass->colorAttachmentCount; ++i)
	{
		if (hasAttachment(secondSubpass, firstSubpass->colorAttachments[i].attachmentIndex, true))
			return true;
	}

	return hasAttachment(secondSubpass, firstSubpass->depthStencilAttachment.attachmentIndex, true);
}

static bool startRenderPassScope(const dsRenderPass* renderPass, dsCommandBuffer* commandBuffer,
	const dsFramebuffer* framebuffer, const dsAlignedBox3f* viewport, bool secondary)
{
	// Error checking for this will be later.
	if (!renderPass || !commandBuffer || !framebuffer)
		return true;

	if (commandBuffer->usage & dsCommandBufferUsage_Resource)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"A render pass cannot be used with a resource command buffer.");
		return false;
	}

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
		viewport->max.x > (float)framebuffer->width || viewport->max.y > (float)framebuffer->height ||
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

	commandBuffer->secondaryRenderPassCommands = secondary;
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

static bool nextSubpassScope(
	const dsRenderPass* renderPass, dsCommandBuffer* commandBuffer, bool secondary)
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

	commandBuffer->secondaryRenderPassCommands = secondary;
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

static void restorePreviousSubpassScope(
	const dsRenderPass* renderPass, dsCommandBuffer* commandBuffer)
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
	commandBuffer->activeRenderSubpass = 0;
	commandBuffer->secondaryRenderPassCommands = false;
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

static bool isResolveValid(const dsRenderer* renderer, const dsAttachmentInfo* attachments,
	const dsFramebuffer* framebuffer, uint32_t attachment)
{
	// Don't check for resolve when no anti-aliasing since offscreens no longer resolve,
	// which would give a false positive.
	uint32_t samples = attachments[attachment].samples;
	if (samples == DS_SURFACE_ANTIALIAS_SAMPLES)
		samples = renderer->surfaceSamples;
	else if (samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
		samples = renderer->defaultSamples;

	if (samples > 1 && !canResolveSurface(framebuffer->surfaces + attachment))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Color attachment set to resolve used with unresolvable framebuffer surface.");
		return false;
	}

	return true;
}

bool dsRenderPass_addFirstSubpassDependencyFlags(dsSubpassDependency* dependency)
{
	if (!dependency || dependency->srcSubpass != DS_EXTERNAL_SUBPASS)
	{
		errno = EINVAL;
		return false;
	}

	dependency->srcStages |= dsGfxPipelineStage_CommandBuffer;
	dependency->dstStages |= dsGfxPipelineStage_AllGraphics;
	dependency->dstAccess |= dsGfxAccess_InputAttachmentRead | dsGfxAccess_ColorAttachmentRead |
		dsGfxAccess_ColorAttachmentWrite | dsGfxAccess_DepthStencilAttachmentRead |
		dsGfxAccess_DepthStencilAttachmentWrite;
	return true;
}

bool dsRenderPass_addLastSubpassDependencyFlags(dsSubpassDependency* dependency)
{
	if (!dependency || dependency->dstSubpass != DS_EXTERNAL_SUBPASS)
	{
		errno = EINVAL;
		return false;
	}

	dependency->srcStages |= dsGfxPipelineStage_AllGraphics;
	dependency->dstStages |= dsGfxPipelineStage_CommandBuffer;
	dependency->srcAccess |= dsGfxAccess_InputAttachmentRead | dsGfxAccess_ColorAttachmentRead |
		dsGfxAccess_ColorAttachmentWrite | dsGfxAccess_DepthStencilAttachmentRead |
		dsGfxAccess_DepthStencilAttachmentWrite;
	return true;
}

uint32_t dsRenderPass_countDefaultDependencies(
	const dsRenderSubpassInfo* subpasses, uint32_t subpassCount)
{
	if (!subpasses || subpassCount == 0)
		return 0;

	uint32_t count = 0;
	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		// Add a dependency for each earlier subpass that shares attachments.
		bool hasEarlierDependency = false;
		for (uint32_t j = 0; j < i; ++j)
		{
			if (sharesAttachments(subpasses + i, subpasses + j))
			{
				++count;
				hasEarlierDependency = true;
			}
		}

		// Implicit dependency if no earlier dependencies.
		if (!hasEarlierDependency)
			++count;

		// Implicit dependency if no later dependencies.
		bool hasLaterDependency = false;
		for (uint32_t j = i + 1; j < subpassCount; ++j)
		{
			if (sharesAttachments(subpasses + i, subpasses + j))
			{
				hasLaterDependency = true;
				break;
			}
		}

		if (!hasLaterDependency)
			++count;
	}

	return count;
}

bool dsRenderPass_setDefaultDependencies(dsSubpassDependency* outDependencies,
	uint32_t dependencyCount, const dsRenderSubpassInfo* subpasses, uint32_t subpassCount)
{
	if (!outDependencies || (subpassCount > 0 && !subpasses))
	{
		errno = EINVAL;
		return false;
	}

	uint32_t index = 0;
	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		// Add a dependency for each earlier subpass that shares attachments.
		bool hasEarlierDependency = false;
		for (uint32_t j = 0; j < i; ++j)
		{
			AttachmentUsage prevUsage = AttachmentUsage_Unused;
			AttachmentUsage curUsage = AttachmentUsage_Unused;
			const dsRenderSubpassInfo* prevSubpass = subpasses + j;
			const dsRenderSubpassInfo* curSubpass = subpasses + i;
			// Check if we might write to an attachment that the previous subpass reads from.
			bool writesPrevInput = false;

			for (uint32_t k = 0; k < curSubpass->inputAttachmentCount; ++k)
			{
				AttachmentUsage thisPrevUsage = getAttachmentUsage(prevSubpass,
					curSubpass->inputAttachments[k]);
				if (thisPrevUsage)
				{
					prevUsage |= thisPrevUsage;
					curUsage |= AttachmentUsage_Input;
				}
			}

			for (uint32_t k = 0; k < curSubpass->colorAttachmentCount; ++k)
			{
				AttachmentUsage thisPrevUsage = getAttachmentUsage(prevSubpass,
					curSubpass->colorAttachments[k].attachmentIndex);
				if (thisPrevUsage)
				{
					if (thisPrevUsage & AttachmentUsage_Input)
						writesPrevInput = true;
					prevUsage |= thisPrevUsage;
					curUsage |= AttachmentUsage_Color;
				}
			}


			AttachmentUsage thisPrevUsage = getAttachmentUsage(prevSubpass,
				curSubpass->depthStencilAttachment.attachmentIndex);
			if (thisPrevUsage)
			{
				if (thisPrevUsage & AttachmentUsage_Input)
					writesPrevInput = true;
				prevUsage |= thisPrevUsage;
				curUsage |= AttachmentUsage_DepthStencil;
			}

			// Check if either previous or current subpass can write to the attachment.
			AttachmentUsage writeUses = AttachmentUsage_Color | AttachmentUsage_DepthStencil;
			if (!(prevUsage & writeUses) && !(curUsage & writeUses))
				continue;

			if (index >= dependencyCount)
			{
				errno = ESIZE;
				return false;
			}

			dsSubpassDependency* dependency = outDependencies + index++;
			dependency->srcSubpass = j;
			dependency->srcStages = 0;
			dependency->srcAccess = 0;
			if (writesPrevInput)
			{
				dependency->srcStages |= dsGfxPipelineStage_FragmentShader;
				dependency->srcAccess |= dsGfxAccess_InputAttachmentRead;
			}
			if (prevUsage & AttachmentUsage_Color)
			{
				dependency->srcStages |= dsGfxPipelineStage_ColorOutput;
				dependency->srcAccess |= dsGfxAccess_ColorAttachmentWrite;
				if (curUsage & AttachmentUsage_Color)
					dependency->srcAccess |= dsGfxAccess_ColorAttachmentRead;
			}
			if (prevUsage & AttachmentUsage_DepthStencil)
			{
				dependency->srcStages |= dsGfxPipelineStage_PostFragmentShaderTests |
					dsGfxPipelineStage_PreFragmentShaderTests;
				dependency->srcAccess |= dsGfxAccess_DepthStencilAttachmentWrite;
				if (curUsage & AttachmentUsage_DepthStencil)
					dependency->srcAccess |= dsGfxAccess_DepthStencilAttachmentRead;
			}

			dependency->dstSubpass = i;
			dependency->dstStages = 0;
			dependency->dstAccess = 0;
			if (curUsage & AttachmentUsage_Input)
			{
				dependency->dstStages |= dsGfxPipelineStage_FragmentShader;
				dependency->dstAccess |= dsGfxAccess_InputAttachmentRead;
			}
			if (curUsage & AttachmentUsage_Color)
			{
				dependency->dstStages |= dsGfxPipelineStage_ColorOutput;
				dependency->dstAccess |= dsGfxAccess_ColorAttachmentRead |
					dsGfxAccess_ColorAttachmentWrite;
			}
			if (curUsage & AttachmentUsage_DepthStencil)
			{
				dependency->dstStages |= dsGfxPipelineStage_PreFragmentShaderTests |
					dsGfxPipelineStage_PostFragmentShaderTests;
				dependency->dstAccess |= dsGfxAccess_DepthStencilAttachmentRead |
					dsGfxAccess_DepthStencilAttachmentWrite;
			}

			dependency->regionDependency = true;
			hasEarlierDependency = true;
		}

		// Implicit dependency if no earlier dependencies.
		if (!hasEarlierDependency)
		{
			if (index >= dependencyCount)
			{
				errno = ESIZE;
				return false;
			}

			dsSubpassDependency* dependency = outDependencies + index++;
			dependency->srcSubpass = DS_EXTERNAL_SUBPASS;
			dependency->srcStages = 0;
			dependency->srcAccess = dsGfxAccess_None;
			dependency->dstSubpass = i;
			dependency->dstStages = 0;
			dependency->dstAccess = dsGfxAccess_None;
			dependency->regionDependency = false;
			dsRenderPass_addFirstSubpassDependencyFlags(dependency);
		}

		// Implicit dependency if no later dependencies.
		bool hasLaterDependency = false;
		for (uint32_t j = i + 1; j < subpassCount; ++j)
		{
			if (sharesAttachments(subpasses + i, subpasses + j))
			{
				hasLaterDependency = true;
				break;
			}
		}

		if (!hasLaterDependency)
		{
			if (index >= dependencyCount)
			{
				errno = ESIZE;
				return false;
			}

			dsSubpassDependency* dependency = outDependencies + index++;
			dependency->srcSubpass = i;
			dependency->srcStages = 0;
			dependency->srcAccess = dsGfxAccess_None;
			dependency->dstSubpass = DS_EXTERNAL_SUBPASS;
			dependency->dstStages = 0;
			dependency->dstAccess = dsGfxAccess_None;
			dependency->regionDependency = false;
			dsRenderPass_addLastSubpassDependencyFlags(dependency);
		}
	}

	if (index != dependencyCount)
	{
		errno = ESIZE;
		return false;
	}
	return true;
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
			!dsGfxFormat_renderTargetSupported(renderer->resourceManager, attachments[i].format))
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

		const dsAttachmentRef* depthStencilAttachment = &subpasses[i].depthStencilAttachment;
		if (depthStencilAttachment->attachmentIndex != DS_NO_ATTACHMENT)
		{
			if (depthStencilAttachment->attachmentIndex >= attachmentCount)
			{
				errno = EINDEX;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Subpass depth-stencil attachment out of range.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}

			if (!dsGfxFormat_isDepthStencil(
					attachments[depthStencilAttachment->attachmentIndex].format))
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG,
					"Cannot use a color surface as a depth-stencil attachment.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}

			if (samples && samples != attachments[depthStencilAttachment->attachmentIndex].samples)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "All color and depth attachments must have the "
					"same number of anti-alias samples.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}

			if (depthStencilAttachment->resolve && !renderer->hasDepthStencilMultisampleResolve)
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG, "The current target doesn't allow resolving "
					"multisampled depth/stencil offscreens.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}
		}

		if (resourceManager->requiresAnySurface && !anyColorAttachmentSet &&
			depthStencilAttachment->attachmentIndex == DS_NO_ATTACHMENT)
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
					"Pipeline stage flags cannot be empty for subpass dependencies.");
				DS_PROFILE_FUNC_RETURN(NULL);
			}

			if (dependencies[i].srcSubpass == dependencies[i].dstSubpass &&
				!dependencies[i].regionDependency)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_RENDER_LOG_TAG,
					"Subpasses that depend on themselves must have region dependencies.");
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
	const dsSurfaceClearValue* clearValues, uint32_t clearValueCount, bool secondary)
{
	if (!startRenderPassScope(renderPass, commandBuffer, framebuffer, viewport, secondary))
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

	if (!dsRenderPass_canUseFramebuffer(renderPass, commandBuffer, framebuffer))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_END();
		endRenderPassScope(commandBuffer);
		return false;
	}

	bool needsClear = false;
	for (uint32_t i = 0; i < framebuffer->surfaceCount; ++i)
	{
		if (renderPass->attachments[i].usage & dsAttachmentUsage_Clear)
			needsClear = true;
	}

	dsRenderer* renderer = renderPass->renderer;
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

		const dsAttachmentRef* depthStencilAttachment = &subpass->depthStencilAttachment;
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

			if (depthStencilAttachment->attachmentIndex != DS_NO_ATTACHMENT)
			{
				surfaceTypes = (SurfaceType)(surfaceTypes | getSurfaceType(
					framebuffer->surfaces[depthStencilAttachment->attachmentIndex].surfaceType));
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
			if (attachment != DS_NO_ATTACHMENT &&
				!isResolveValid(renderer, renderPass->attachments, framebuffer, attachment))
			{
				DS_PROFILE_FUNC_END();
				endRenderPassScope(commandBuffer);
				return false;
			}
		}

		if (depthStencilAttachment->attachmentIndex != DS_NO_ATTACHMENT &&
			depthStencilAttachment->resolve && !isResolveValid(renderer, renderPass->attachments,
				framebuffer, depthStencilAttachment->attachmentIndex))
		{
			DS_PROFILE_FUNC_END();
			endRenderPassScope(commandBuffer);
			return false;
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

	// HACK: Render pass not actually active yet, so ensure the checks against strict secondary
	// command buffer usage don't get triggered.
	commandBuffer->secondaryRenderPassCommands = false;
	dsGPUProfileContext_beginSubpass(renderer->_profileContext, commandBuffer,
		framebuffer->name, renderPass->subpasses[0].name, secondary);
	commandBuffer->secondaryRenderPassCommands = secondary;
	bool success = renderer->beginRenderPassFunc(renderer, commandBuffer, renderPass, framebuffer,
		viewport, clearValues, clearValueCount, secondary);
	DS_PROFILE_FUNC_END();
	if (!success)
	{
		dsGPUProfileContext_endSubpass(renderer->_profileContext, commandBuffer);
		endRenderPassScope(commandBuffer);
	}
	return success;
}

bool dsRenderPass_nextSubpass(
	const dsRenderPass* renderPass, dsCommandBuffer* commandBuffer, bool secondary)
{
	// End the previous scope
	if (!nextSubpassScope(renderPass, commandBuffer, secondary))
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
		commandBuffer->activeRenderSubpass, secondary);
	if (success)
	{
		dsGPUProfileContext_nextSubpass(renderer->_profileContext, commandBuffer,
			renderPass->subpasses[commandBuffer->activeRenderSubpass].name, secondary);
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
	{
		// HACK: Render pass ended, so avoid checks against strict secondary command buffer
		// management. Will get cleared anyway, so don't bother restoring the flag.
		commandBuffer->secondaryRenderPassCommands = false;
		dsGPUProfileContext_endSubpass(renderer->_profileContext, commandBuffer);
	}
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

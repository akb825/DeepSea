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

#include <DeepSea/Render/CommandBuffer.h>

#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Math/Vector3.h>

bool dsCommandBuffer_begin(dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !commandBuffer->renderer ||
		!commandBuffer->renderer->beginCommandBufferFunc ||
		!commandBuffer->renderer->endCommandBufferFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsRenderer* renderer = commandBuffer->renderer;
	if (commandBuffer == renderer->mainCommandBuffer)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot begin the main command buffer.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->usage & dsCommandBufferUsage_Secondary)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Must use dsCommandBuffer_beginSecondary() to begin a secondary command buffer.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->beginCommandBufferFunc(renderer, commandBuffer);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsCommandBuffer_beginSecondary(dsCommandBuffer* commandBuffer,
	const dsFramebuffer* framebuffer, const dsRenderPass* renderPass, uint32_t subpass,
	const dsAlignedBox3f* viewport)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !commandBuffer->renderer ||
		!commandBuffer->renderer->beginSecondaryCommandBufferFunc ||
		!commandBuffer->renderer->endCommandBufferFunc || !renderPass)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (subpass >= renderPass->subpassCount)
	{
		errno = EINDEX;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!framebuffer && !viewport)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Either framebuffer or viewport must be specified.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsRenderer* renderer = commandBuffer->renderer;
	if (commandBuffer == renderer->mainCommandBuffer)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot begin the main command buffer.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!(commandBuffer->usage & dsCommandBufferUsage_Secondary))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Must use dsCommandBuffer_begin() to begin a non-secondary command buffer.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (viewport && (viewport->min.x < 0 || viewport->min.y < 0 || viewport->min.z < 0 ||
		viewport->max.z > 1 || (framebuffer && (viewport->max.x > framebuffer->width ||
			viewport->max.y > framebuffer->height))))
	{
		errno = ERANGE;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Viewport is out of range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->beginSecondaryCommandBufferFunc(renderer, commandBuffer, framebuffer,
		renderPass, subpass, viewport);
	if (!success)
		DS_PROFILE_FUNC_RETURN(false);

	commandBuffer->secondaryRenderPassCommands = false;
	commandBuffer->boundFramebuffer = framebuffer;
	commandBuffer->boundRenderPass = renderPass;
	commandBuffer->activeRenderSubpass = subpass;
	if (viewport)
		commandBuffer->viewport = *viewport;
	else
	{
		DS_ASSERT(framebuffer);
		commandBuffer->viewport.min.x = 0.0f;
		commandBuffer->viewport.min.y = 0.0f;
		commandBuffer->viewport.min.z = 0.0f;
		commandBuffer->viewport.max.x = (float)framebuffer->width;
		commandBuffer->viewport.max.y = (float)framebuffer->height;
		commandBuffer->viewport.max.z = 1.0f;
	}
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsCommandBuffer_end(dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !commandBuffer->renderer ||
		!commandBuffer->renderer->endCommandBufferFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsRenderer* renderer = commandBuffer->renderer;
	if (commandBuffer == renderer->mainCommandBuffer)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot end the main command buffer.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundSurface)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot end a command buffer while a render surface is bound.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundRenderPass && !(commandBuffer->usage & dsCommandBufferUsage_Secondary))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot end a command buffer inside of a render pass.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundShader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot end a command buffer while a shader is bound.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundComputeShader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot end a command buffer while a compute shader is bound.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->endCommandBufferFunc(renderer, commandBuffer);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsCommandBuffer_submit(dsCommandBuffer* commandBuffer, dsCommandBuffer* submitBuffer)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !submitBuffer || !commandBuffer->renderer ||
		!commandBuffer->renderer->submitCommandBufferFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer == submitBuffer)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot submit a command buffer to itself.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (((commandBuffer->usage & dsCommandBufferUsage_MultiSubmit) &&
			!(submitBuffer->usage & dsCommandBufferUsage_MultiSubmit)) ||
		((commandBuffer->usage & dsCommandBufferUsage_MultiFrame) &&
			!(submitBuffer->usage & dsCommandBufferUsage_MultiFrame)))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot submit a single-use command buffer to a multi-use command buffer.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->usage & dsCommandBufferUsage_Secondary)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot submit to a secondary command buffer.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsRenderer* renderer = commandBuffer->renderer;
	if (submitBuffer == renderer->mainCommandBuffer)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot submit the main command buffer to another command buffer.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool submitIsSecondary = (submitBuffer->usage & dsCommandBufferUsage_Secondary) != 0;
	if (commandBuffer->boundRenderPass && !submitIsSecondary)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Can only submit a secondary command buffer inside of a render pass.");
		DS_PROFILE_FUNC_RETURN(false);
	}
	else if (submitIsSecondary)
	{
		if ((submitBuffer->boundFramebuffer &&
				commandBuffer->boundFramebuffer != submitBuffer->boundFramebuffer) ||
			commandBuffer->boundRenderPass != submitBuffer->boundRenderPass ||
			commandBuffer->activeRenderSubpass != submitBuffer->activeRenderSubpass)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Secondary command buffer must have the same framebuffer, render pass, and subpass "
				"as the command buffer it's being submitted to.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		if (!commandBuffer->secondaryRenderPassCommands)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "A render subpass must be begun with the secondary "
				"flag set to true to accept secondary command buffers.");
			DS_PROFILE_FUNC_RETURN(false);
		}

		if (!dsVector3_equal(commandBuffer->viewport.min, submitBuffer->viewport.min) ||
			!dsVector3_equal(commandBuffer->viewport.max, submitBuffer->viewport.max))
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Viewport must match between render pass begin in main command buffer and "
				"secondary command buffer begin.");
			DS_PROFILE_FUNC_RETURN(false);
		}
	}

	if (commandBuffer->boundShader || commandBuffer->boundComputeShader ||
		submitBuffer->boundShader || commandBuffer->boundComputeShader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot submit a command buffer when a shader is bound.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!commandBuffer->frameActive)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Submitting a command buffer must be performed within a frame.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->submitCommandBufferFunc(renderer, commandBuffer, submitBuffer);
	DS_PROFILE_FUNC_RETURN(success);
}

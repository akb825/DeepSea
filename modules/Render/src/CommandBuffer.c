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

bool dsCommandBuffer_isIndirect(const dsCommandBuffer* commandBuffer)
{
	if (commandBuffer->indirectCommands)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "May only use indirect commands from secondary command "
			"buffers inside this render subpass.");
		return true;
	}

	return false;
}

bool dsCommandBuffer_begin(dsCommandBuffer* commandBuffer, const dsRenderPass* renderPass,
	uint32_t subpassIndex, const dsFramebuffer* framebuffer)
{
	DS_PROFILE_FUNC_START();

	if (!commandBuffer || !commandBuffer->renderer ||
		!commandBuffer->renderer->beginCommandBufferFunc ||
		!commandBuffer->renderer->endCommandBufferFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if ((commandBuffer->usage & dsCommandBufferUsage_Subpass) && !renderPass)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Must provide a render pass for a subpass command buffer");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (renderPass && subpassIndex >= renderPass->subpassCount)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Subpass index out of range.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsRenderer* renderer = commandBuffer->renderer;
	if (commandBuffer == renderer->mainCommandBuffer)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot begin the main command buffer.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = renderer->beginCommandBufferFunc(renderer, commandBuffer, renderPass,
		subpassIndex, framebuffer);
	if (!success || !(commandBuffer->usage & dsCommandBufferUsage_Subpass))
		DS_PROFILE_FUNC_RETURN(success);

	// Guarantee a consistent starting state.
	commandBuffer->boundSurface = NULL;
	commandBuffer->boundRenderPass = renderPass;
	commandBuffer->activeRenderSubpass = subpassIndex;
	commandBuffer->indirectCommands = false;
	commandBuffer->boundFramebuffer = framebuffer;
	commandBuffer->boundShader = NULL;
	commandBuffer->boundComputeShader = NULL;
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

	if (!(commandBuffer->usage & dsCommandBufferUsage_Subpass) && commandBuffer->boundRenderPass)
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

	dsRenderer* renderer = commandBuffer->renderer;
	if (submitBuffer == renderer->mainCommandBuffer)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Cannot submit the main command buffer to another command buffer.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (commandBuffer->boundRenderPass && !commandBuffer->indirectCommands)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Can only submit a command buffer inside of a render pass "
			"if indirectCommands is set to true.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	// TODO: look into checking for render pass compatibility.

	bool success = renderer->submitCommandBufferFunc(renderer, commandBuffer, submitBuffer);
	DS_PROFILE_FUNC_RETURN(success);
}

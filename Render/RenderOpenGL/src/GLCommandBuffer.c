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

#include "GLCommandBuffer.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/VolatileMaterialValues.h>

static bool insideRenderPass(const dsCommandBuffer* commandBuffer)
{
	const dsGLCommandBuffer* glCommandBuffer = (const dsGLCommandBuffer*)commandBuffer;
	return glCommandBuffer->subpassOnly || glCommandBuffer->boundRenderPass;
}

static uint32_t getSubpassSamples(const dsRenderPass* renderPass, uint32_t subpassIndex)
{
	const dsRenderSubpassInfo* subpass = renderPass->subpasses + subpassIndex;
	for (uint32_t i = 0; i < subpass->colorAttachmentCount; ++i)
	{
		if (subpass->colorAttachments[i].attachmentIndex == DS_NO_ATTACHMENT)
			continue;

		const dsAttachmentInfo* attachment = renderPass->attachments +
			subpass->colorAttachments[i].attachmentIndex;
		if (attachment->samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
			return renderPass->renderer->surfaceSamples;
		return attachment->samples;
	}

	if (subpass->depthStencilAttachment != DS_NO_ATTACHMENT)
	{
		const dsAttachmentInfo* attachment = renderPass->attachments +
			subpass->depthStencilAttachment;
		if (attachment->samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
			return renderPass->renderer->surfaceSamples;
		return attachment->samples;
	}

	return 0;
}

void dsGLCommandBuffer_initialize(dsCommandBuffer* commandBuffer, bool subpassOnly)
{
	DS_ASSERT(commandBuffer);
	DS_ASSERT(commandBuffer->allocator);

	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	glCommandBuffer->commitCounts = NULL;
	glCommandBuffer->commitCountSize = 0;
	glCommandBuffer->subpassOnly = subpassOnly;
	glCommandBuffer->subpassIndex = 0;
	glCommandBuffer->subpassSamples = 0;
	glCommandBuffer->boundRenderPass = NULL;
	glCommandBuffer->boundShader = NULL;
	glCommandBuffer->boundSurface = NULL;
}

void dsGLCommandBuffer_shutdown(dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(commandBuffer);

	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	dsAllocator_free(commandBuffer->allocator, glCommandBuffer->commitCounts);
}

bool dsGLCommandBuffer_copyBufferData(dsCommandBuffer* commandBuffer, dsGfxBuffer* buffer,
	size_t offset, const void* data, size_t size)
{
	if (insideRenderPass(commandBuffer))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Copying of buffers must be done outside of a render pass.");
		return false;
	}

	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->copyBufferDataFunc(commandBuffer, buffer, offset, data, size);
}

bool dsGLCommandBuffer_copyBuffer(dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer,
	size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset, size_t size)
{
	if (insideRenderPass(commandBuffer))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Copying of buffers must be done outside of a render pass.");
		return false;
	}

	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->copyBufferFunc(commandBuffer, srcBuffer, srcOffset, dstBuffer, dstOffset,
		size);
}

bool dsGLCommandBuffer_copyTextureData(dsCommandBuffer* commandBuffer, dsTexture* texture,
	const dsTexturePosition* position, uint32_t width, uint32_t height, uint32_t layers,
	const void* data, size_t size)
{
	if (insideRenderPass(commandBuffer))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Copying of textures must be done outside of a render pass.");
		return false;
	}

	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->copyTextureDataFunc(commandBuffer, texture, position, width, height, layers,
		data, size);
}

bool dsGLCommandBuffer_copyTexture(dsCommandBuffer* commandBuffer, dsTexture* srcTexture,
	dsTexture* dstTexture, const dsTextureCopyRegion* regions, size_t regionCount)
{
	if (insideRenderPass(commandBuffer))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Copying of textures must be done outside of a render pass.");
		return false;
	}

	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->copyTextureFunc(commandBuffer, srcTexture, dstTexture, regions, regionCount);
}

bool dsGLCommandBuffer_generateTextureMipmaps(dsCommandBuffer* commandBuffer, dsTexture* texture)
{
	if (insideRenderPass(commandBuffer))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Generating of mipmaps must be done outside of a render pass.");
		return false;
	}

	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->generateTextureMipmapsFunc(commandBuffer, texture);
}

bool dsGLCommandBuffer_setFenceSyncs(dsCommandBuffer* commandBuffer, dsGLFenceSyncRef** syncs,
	uint32_t syncCount, bool bufferReadback)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->setFenceSyncsFunc(commandBuffer, syncs, syncCount, bufferReadback);
}

bool dsGLCommandBuffer_bindShaderAndMaterial(dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsMaterial* material, const dsVolatileMaterialValues* volatileValues,
	const dsDynamicRenderStates* renderStates)
{
	DS_ASSERT(commandBuffer);
	DS_ASSERT(shader);
	DS_ASSERT(material);

	if (!dsGLCommandBuffer_bindShader(commandBuffer, shader, renderStates))
		return false;

	dsGLShader* glShader = (dsGLShader*)shader;
	bool useGfxBuffers = dsShaderVariableGroup_useGfxBuffer(shader->resourceManager);
	const dsMaterialDesc* materialDesc = shader->materialDesc;
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		if (materialDesc->elements[i].isVolatile)
			continue;

		switch (materialDesc->elements[i].type)
		{
			case dsMaterialType_Texture:
			case dsMaterialType_Image:
			case dsMaterialType_SubpassInput:
			{
				if (glShader->uniforms[i].location < 0)
					continue;

				dsTexture* texture = dsMaterial_getTexture(material, i);
				if (texture)
					dsGLCommandBuffer_setTexture(commandBuffer, shader, i, texture);
				else
				{
					dsGfxFormat format;
					size_t offset, count;
					dsGfxBuffer* buffer = dsMaterial_getTextureBuffer(&format, &offset, &count,
						material, i);
					if (buffer)
					{
						dsGLCommandBuffer_setTextureBuffer(commandBuffer, shader, i, buffer,
							format, offset, count);
					}
					else
						dsGLCommandBuffer_setTexture(commandBuffer, shader, i, NULL);
				}
				break;
			}
			case dsMaterialType_UniformBlock:
			case dsMaterialType_UniformBuffer:
			{
				if (glShader->uniforms[i].location < 0)
					continue;

				size_t offset, size;
				dsGfxBuffer* buffer = dsMaterial_getBuffer(&offset, &size, material, i);
				if (!buffer)
				{
					errno = EPERM;
					DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG,
						"No buffer set for material value '%s'", materialDesc->elements[i].name);
					dsGLCommandBuffer_unbindShader(commandBuffer, shader);
					return false;
				}
				dsGLCommandBuffer_setShaderBuffer(commandBuffer, shader, i, buffer, offset, size);
				break;
			}
			case dsMaterialType_VariableGroup:
			{
				dsShaderVariableGroup* variableGroup = dsMaterial_getVariableGroup(material, i);
				if (!variableGroup)
				{
					errno = EPERM;
					DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG,
						"No variable group set for material value '%s'",
						materialDesc->elements[i].name);
					dsGLCommandBuffer_unbindShader(commandBuffer, shader);
					return false;
				}

				if (useGfxBuffers)
				{
					if (glShader->uniforms[i].location < 0)
						continue;

					dsGfxBuffer* buffer = dsShaderVariableGroup_getGfxBuffer(variableGroup);
					DS_ASSERT(buffer);
					dsGLCommandBuffer_setShaderBuffer(commandBuffer, shader, i, buffer, 0,
						buffer->size);
				}
				else
				{
					const dsShaderVariableGroupDesc* groupDesc =
						materialDesc->elements[i].shaderVariableGroupDesc;
					DS_ASSERT(groupDesc);
					for (uint32_t j = 0; j < groupDesc->elementCount; ++j)
					{
						if (glShader->uniforms[i].groupLocations[j] < 0)
							continue;

						dsGLCommandBuffer_setUniform(commandBuffer,
							glShader->uniforms[i].groupLocations[j], groupDesc->elements[j].type,
							groupDesc->elements[j].count,
							dsShaderVariableGroup_getRawElementData(variableGroup, j));
					}
				}
				break;
			}
			default:
				if (glShader->uniforms[i].location < 0)
					continue;

				dsGLCommandBuffer_setUniform(commandBuffer, glShader->uniforms[i].location,
					materialDesc->elements[i].type, materialDesc->elements[i].count,
					dsMaterial_getRawElementData(material, i));
				break;
		}
	}

	if (!useGfxBuffers)
	{
		dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
		if (!glCommandBuffer->commitCounts ||
			materialDesc->elementCount > glCommandBuffer->commitCountSize)
		{
			dsAllocator_free(commandBuffer->allocator, glCommandBuffer->commitCounts);
			glCommandBuffer->commitCounts = DS_ALLOCATE_OBJECT_ARRAY(commandBuffer->allocator,
				dsCommitCountInfo, materialDesc->elementCount);
			glCommandBuffer->commitCountSize = materialDesc->elementCount;
			if (!glCommandBuffer->commitCounts)
			{
				dsGLCommandBuffer_unbindShader(commandBuffer, shader);
				return false;
			}
		}

		for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
		{
			glCommandBuffer->commitCounts[i].variableGroup = NULL;
			glCommandBuffer->commitCounts[i].commitCount = DS_VARIABLE_GROUP_UNSET_COMMIT;
		}
	}

	if (!dsGLCommandBuffer_setVolatileMaterialValues(commandBuffer, shader, volatileValues))
	{
		dsGLCommandBuffer_unbindShader(commandBuffer, shader);
		return false;
	}

	return true;
}

bool dsGLCommandBuffer_bindShader(dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsDynamicRenderStates* renderStates)
{
	if (!insideRenderPass(commandBuffer))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Shader operations must be done within a render pass.");
		return false;
	}

	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	if (glCommandBuffer->boundShader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Shader cannot be bound while another shader is already bound.");
		return false;
	}

	uint32_t shaderSamples = shader->samples;
	if (shader->samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
		shaderSamples = commandBuffer->renderer->surfaceSamples;
	if (glCommandBuffer->subpassSamples && glCommandBuffer->subpassSamples != shaderSamples)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Shader anti-alias samples don't match the "
			"attachments for the current render subpass.");
		return false;
	}

	const CommandBufferFunctionTable* functions = glCommandBuffer->functions;
	if (!functions->bindShaderFunc(commandBuffer, shader, renderStates))
		return false;

	glCommandBuffer->boundShader = shader;
	return true;
}

bool dsGLCommandBuffer_setTexture(dsCommandBuffer* commandBuffer, const dsShader* shader,
	uint32_t element, dsTexture* texture)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->setTextureFunc(commandBuffer, shader, element, texture);
}

bool dsGLCommandBuffer_setTextureBuffer(dsCommandBuffer* commandBuffer, const dsShader* shader,
	uint32_t element, dsGfxBuffer* buffer, dsGfxFormat format, size_t offset, size_t count)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->setTextureBufferFunc(commandBuffer, shader, element, buffer, format, offset,
		count);
}

bool dsGLCommandBuffer_setShaderBuffer(dsCommandBuffer* commandBuffer, const dsShader* shader,
	uint32_t element, dsGfxBuffer* buffer, size_t offset, size_t size)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->setShaderBufferFunc(commandBuffer, shader, element, buffer, offset, size);
}

bool dsGLCommandBuffer_setUniform(dsCommandBuffer* commandBuffer, GLint location,
	dsMaterialType type, uint32_t count, const void* data)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->setUniformFunc(commandBuffer, location, type, count, data);
}

bool dsGLCommandBuffer_setVolatileMaterialValues(dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsVolatileMaterialValues* volatileValues)
{
	DS_ASSERT(commandBuffer);
	DS_ASSERT(shader);

	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	if (glCommandBuffer->boundShader != shader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Volatile material values must only be set on the bound shader.");
		return false;
	}

	if (!volatileValues)
		return true;

	bool useGfxBuffers = dsShaderVariableGroup_useGfxBuffer(shader->resourceManager);
	const dsGLShader* glShader = (const dsGLShader*)shader;
	const dsMaterialDesc* materialDesc = shader->materialDesc;
	DS_ASSERT(useGfxBuffers || glCommandBuffer->commitCountSize >= materialDesc->elementCount);
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		if (!materialDesc->elements[i].isVolatile)
			continue;

		uint32_t nameId = materialDesc->elements[i].nameId;
		switch (materialDesc->elements[i].type)
		{
			case dsMaterialType_Texture:
			case dsMaterialType_Image:
			case dsMaterialType_SubpassInput:
			{
				if (glShader->uniforms[i].location < 0)
					continue;

				dsTexture* texture = dsVolatileMaterialValues_getTextureId(volatileValues, nameId);
				if (texture)
					dsGLCommandBuffer_setTexture(commandBuffer, shader, i, texture);
				else
				{
					dsGfxFormat format;
					size_t offset, count;
					dsGfxBuffer* buffer = dsVolatileMaterialValues_getTextureBufferId(&format,
						&offset, &count, volatileValues, i);
					if (buffer)
					{
						dsGLCommandBuffer_setTextureBuffer(commandBuffer, shader, i, buffer,
							format, offset, count);
					}
					else
						dsGLCommandBuffer_setTexture(commandBuffer, shader, i, NULL);
				}
				break;
			}
			case dsMaterialType_UniformBlock:
			case dsMaterialType_UniformBuffer:
			{
				if (glShader->uniforms[i].location < 0)
					continue;

				size_t offset, size;
				dsGfxBuffer* buffer = dsVolatileMaterialValues_getBufferId(&offset, &size,
					volatileValues, nameId);
				if (!buffer)
				{
					errno = EPERM;
					DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG,
						"No buffer set for volatile material value '%s'",
						materialDesc->elements[i].name);
					dsGLCommandBuffer_unbindShader(commandBuffer, shader);
					return false;
				}
				dsGLCommandBuffer_setShaderBuffer(commandBuffer, shader, i, buffer, offset, size);
				break;
			}
			case dsMaterialType_VariableGroup:
			{
				dsShaderVariableGroup* variableGroup = dsVolatileMaterialValues_getVariableGroupId(
					volatileValues, nameId);
				if (!variableGroup)
				{
					errno = EPERM;
					DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG,
						"No variable group set for material value '%s'",
						materialDesc->elements[i].name);
					return false;
				}

				if (useGfxBuffers)
				{
					if (glShader->uniforms[i].location < 0)
						continue;

					dsGfxBuffer* buffer = dsShaderVariableGroup_getGfxBuffer(variableGroup);
					DS_ASSERT(buffer);
					dsGLCommandBuffer_setShaderBuffer(commandBuffer, shader, i, buffer, 0,
						buffer->size);
				}
				else
				{
					const dsShaderVariableGroupDesc* groupDesc =
						materialDesc->elements[i].shaderVariableGroupDesc;
					DS_ASSERT(groupDesc);

					uint64_t commitCount = DS_VARIABLE_GROUP_UNSET_COMMIT;
					if (glCommandBuffer->commitCounts[i].variableGroup == variableGroup)
						commitCount = glCommandBuffer->commitCounts[i].commitCount;

					for (uint32_t j = 0; j < groupDesc->elementCount; ++j)
					{
						if (glShader->uniforms[i].groupLocations[j] < 0 ||
							!dsShaderVariableGroup_isElementDirty(variableGroup, j, commitCount))
						{
							continue;
						}

						dsGLCommandBuffer_setUniform(commandBuffer,
							glShader->uniforms[i].groupLocations[j], groupDesc->elements[j].type,
							groupDesc->elements[j].count,
							dsShaderVariableGroup_getRawElementData(variableGroup, j));
					}

					glCommandBuffer->commitCounts[i].variableGroup = variableGroup;
					glCommandBuffer->commitCounts[i].commitCount =
						dsShaderVariableGroup_getCommitCount(variableGroup);
				}
				break;
			}
			default:
				DS_ASSERT(false);
				break;
		}
	}

	return true;
}

bool dsGLCommandBuffer_unbindShader(dsCommandBuffer* commandBuffer, const dsShader* shader)
{
	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	if (glCommandBuffer->boundShader != shader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Can only unbind the currently bound shader.");
		return false;
	}

	const CommandBufferFunctionTable* functions = glCommandBuffer->functions;
	if (!functions->unbindShaderFunc(commandBuffer, shader))
		return false;

	glCommandBuffer->boundShader = NULL;
	return true;
}

bool dsGLCommandBuffer_beginRenderSurface(dsCommandBuffer* commandBuffer, void* glSurface)
{
	if (insideRenderPass(commandBuffer))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"The current render surface cannot be changed during a render pass.");
		return false;
	}

	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	if (glCommandBuffer->boundSurface)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Cannot begin drawing to a render surface when one is already bound.");
		return false;
	}

	const CommandBufferFunctionTable* functions = glCommandBuffer->functions;
	if (!functions->beginRenderSurfaceFunc(commandBuffer, glSurface))
		return false;

	glCommandBuffer->boundSurface = glSurface;
	return true;
}

bool dsGLCommandBuffer_endRenderSurface(dsCommandBuffer* commandBuffer, void* glSurface)
{
	if (insideRenderPass(commandBuffer))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"The current render surface cannot be changed during a render pass.");
		return false;
	}

	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	if (glCommandBuffer->boundSurface != glSurface)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Can only end drawing to the currently bound render surface.");
		return false;
	}

	const CommandBufferFunctionTable* functions = glCommandBuffer->functions;
	if (!functions->endRenderSurfaceFunc(commandBuffer, glSurface))
		return false;

	glCommandBuffer->boundSurface = NULL;
	return true;
}

bool dsGLCommandBuffer_beginRenderPass(dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, const dsFramebuffer* framebuffer,
	const dsAlignedBox3f* viewport, const dsSurfaceClearValue* clearValues,
	uint32_t clearValueCount)
{
	if (insideRenderPass(commandBuffer))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Cannot begin a render pass when already within a render pass.");
		return false;
	}

	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	DS_ASSERT(!glCommandBuffer->boundShader);
	const CommandBufferFunctionTable* functions = glCommandBuffer->functions;
	if (!functions->beginRenderPassFunc(commandBuffer, renderPass, framebuffer, viewport,
		clearValues, clearValueCount))
	{
		return false;
	}

	glCommandBuffer->boundRenderPass = renderPass;
	glCommandBuffer->subpassIndex = 0;
	glCommandBuffer->subpassSamples = getSubpassSamples(glCommandBuffer->boundRenderPass,
		glCommandBuffer->subpassIndex);
	return true;
}

bool dsGLCommandBuffer_nextRenderSubpass(dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass)
{
	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	if (glCommandBuffer->boundRenderPass != renderPass)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Can only move to the next subpass of the currently bound render pass.");
		return false;
	}

	if (glCommandBuffer->subpassIndex + 1 >= renderPass->subpassCount)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Already reached the last subpass of the current render pass.");
		return false;
	}

	if (glCommandBuffer->boundShader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Cannot end a subpass while a shader is bound.");
		return false;
	}

	const CommandBufferFunctionTable* functions = glCommandBuffer->functions;
	if (!functions->nextRenderSubpassFunc(commandBuffer, renderPass,
		glCommandBuffer->subpassIndex + 1))
	{
		return false;
	}

	++glCommandBuffer->subpassIndex;
	glCommandBuffer->subpassSamples = getSubpassSamples(glCommandBuffer->boundRenderPass,
		glCommandBuffer->subpassIndex);
	return true;
}

bool dsGLCommandBuffer_endRenderPass(dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass)
{
	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	if (glCommandBuffer->boundRenderPass != renderPass)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Can only move to the next subpass of the currently bound render pass.");
		return false;
	}

	if (glCommandBuffer->subpassIndex != renderPass->subpassCount - 1)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Can only end a render pass on the last subpass.");
		return false;
	}

	if (glCommandBuffer->boundShader)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Cannot end a render pass while a shader is bound.");
		return false;
	}

	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	if (!functions->endRenderPassFunc(commandBuffer, renderPass))
		return false;

	glCommandBuffer->boundRenderPass = NULL;
	return true;
}

bool dsGLCommandBuffer_clearColorSurface(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsFramebufferSurface* surface,
	const dsSurfaceColorValue* colorValue)
{
	DS_UNUSED(renderer);
	if (insideRenderPass(commandBuffer))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Surfaces cannot be explicitly cleared inside a render pass.");
		return false;
	}

	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	DS_ASSERT(surface);
	if (surface->surfaceType != dsGfxSurfaceType_Texture &&
		surface->surfaceType != dsGfxSurfaceType_Renderbuffer)
	{
		if (((dsGLRenderSurface*)surface->surface)->glSurface != glCommandBuffer->boundSurface)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
				"Only the currently bound surface can be bound.");
			return false;
		}
	}

	const CommandBufferFunctionTable* functions = glCommandBuffer->functions;
	return functions->clearColorSurfaceFunc(commandBuffer, surface, colorValue);
}

bool dsGLCommandBuffer_clearDepthStencilSurface(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsFramebufferSurface* surface,
	dsClearDepthStencil surfaceParts, const dsDepthStencilValue* depthStencilValue)
{
	DS_UNUSED(renderer);
	if (insideRenderPass(commandBuffer))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Surfaces cannot be explicitly cleared inside a render pass.");
		return false;
	}

	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	DS_ASSERT(surface);
	if (surface->surfaceType != dsGfxSurfaceType_Texture &&
		surface->surfaceType != dsGfxSurfaceType_Renderbuffer)
	{
		if (((dsGLRenderSurface*)surface->surface)->glSurface != glCommandBuffer->boundSurface)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
				"Only the currently bound surface can be bound.");
			return false;
		}
	}

	const CommandBufferFunctionTable* functions = glCommandBuffer->functions;
	return functions->clearDepthStencilSurfaceFunc(commandBuffer, surface, surfaceParts,
		depthStencilValue);
}

bool dsGLCommandBuffer_draw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawRange* drawRange)
{
	DS_UNUSED(renderer);
	if (!insideRenderPass(commandBuffer))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Drawing must happen within a render pass.");
		return false;
	}

	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->drawFunc(commandBuffer, geometry, drawRange);
}

bool dsGLCommandBuffer_drawIndexed(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawIndexedRange* drawRange)
{
	DS_UNUSED(renderer);
	if (!insideRenderPass(commandBuffer))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Drawing must happen within a render pass.");
		return false;
	}

	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->drawIndexedFunc(commandBuffer, geometry, drawRange);
}

bool dsGLCommandBuffer_drawIndirect(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride)
{
	DS_UNUSED(renderer);
	if (!insideRenderPass(commandBuffer))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Drawing must happen within a render pass.");
		return false;
	}

	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->drawIndirectFunc(commandBuffer, geometry, indirectBuffer, offset, count,
		stride);
}

bool dsGLCommandBuffer_drawIndexedIndirect(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride)
{
	DS_UNUSED(renderer);
	if (!insideRenderPass(commandBuffer))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Drawing must happen within a render pass.");
		return false;
	}

	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->drawIndirectFunc(commandBuffer, geometry, indirectBuffer, offset, count,
		stride);
}

bool dsGLCommandBuffer_dispatchCompute(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	uint32_t x, uint32_t y, uint32_t z)
{
	DS_UNUSED(renderer);
	if (!insideRenderPass(commandBuffer))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Drawing must happen within a render pass.");
		return false;
	}

	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->dispatchComputeFunc(commandBuffer, x, y, z);
}

bool dsGLCommandBuffer_dispatchComputeIndirect(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsGfxBuffer* indirectBuffer, size_t offset)
{
	DS_UNUSED(renderer);
	if (!insideRenderPass(commandBuffer))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Drawing must happen within a render pass.");
		return false;
	}

	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->dispatchComputeIndirectFunc(commandBuffer, indirectBuffer, offset);
}

bool dsGLCommandBuffer_blitSurface(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsGfxSurfaceType srcSurfaceType, void* srcSurface, dsGfxSurfaceType dstSurfaceType,
	void* dstSurface, const dsSurfaceBlitRegion* regions, size_t regionCount, dsBlitFilter filter)
{
	DS_UNUSED(renderer);
	if (insideRenderPass(commandBuffer))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
			"Blitting of surfaces must be done outside of a render pass.");
		return false;
	}

	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	DS_ASSERT(srcSurface);
	if (srcSurfaceType != dsGfxSurfaceType_Texture &&
		srcSurfaceType != dsGfxSurfaceType_Renderbuffer)
	{
		if (((dsGLRenderSurface*)srcSurface)->glSurface != glCommandBuffer->boundSurface)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Only the currently bound render surface, or "
				"texture or renderbuffer surface, can be blit.");
			return false;
		}
	}

	DS_ASSERT(dstSurface);
	if (dstSurfaceType != dsGfxSurfaceType_Texture &&
		dstSurfaceType != dsGfxSurfaceType_Renderbuffer)
	{
		if (((dsGLRenderSurface*)dstSurface)->glSurface != glCommandBuffer->boundSurface)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, "Only the currently bound render surface, or "
				"texture or renderbuffer surface, can be blit.");
			return false;
		}
	}

	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->blitSurfaceFunc(commandBuffer, srcSurfaceType, srcSurface, dstSurfaceType,
		dstSurface, regions, regionCount, filter);
}

bool dsGLCommandBuffer_begin(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, uint32_t subpassIndex, const dsFramebuffer* framebuffer)
{
	DS_UNUSED(renderer);
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->beginFunc(commandBuffer, renderPass, subpassIndex, framebuffer);
}

bool dsGLCommandBuffer_end(dsRenderer* renderer, dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(renderer);
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->endFunc(commandBuffer);
}

bool dsGLCommandBuffer_submit(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer)
{
	DS_UNUSED(renderer);
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)submitBuffer)->functions;
	return functions->submitFunc(commandBuffer, submitBuffer);
}

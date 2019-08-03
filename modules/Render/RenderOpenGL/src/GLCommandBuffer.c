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
#include <DeepSea/Render/Resources/SharedMaterialValues.h>

static bool setSharedMaterialValues(dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsSharedMaterialValues* sharedValues, dsMaterialBinding binding)
{
	DS_ASSERT(commandBuffer);
	DS_ASSERT(shader);

	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	if (!sharedValues)
		return true;

	bool useGfxBuffers = dsShaderVariableGroup_useGfxBuffer(shader->resourceManager);
	const dsGLShader* glShader = (const dsGLShader*)shader;
	const dsMaterialDesc* materialDesc = shader->materialDesc;
	DS_ASSERT(useGfxBuffers || glCommandBuffer->commitCountSize >= materialDesc->elementCount);
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		if (materialDesc->elements[i].binding != binding)
			continue;

		uint32_t nameID = materialDesc->elements[i].nameID;
		switch (materialDesc->elements[i].type)
		{
			case dsMaterialType_Texture:
			case dsMaterialType_Image:
			case dsMaterialType_SubpassInput:
			{
				if (glShader->uniforms[i].location < 0)
					continue;

				dsTexture* texture = dsSharedMaterialValues_getTextureId(sharedValues, nameID);
				if (texture)
					dsGLCommandBuffer_setTexture(commandBuffer, shader, i, texture);
				else
					dsGLCommandBuffer_setTexture(commandBuffer, shader, i, NULL);
				break;
			}
			case dsMaterialType_TextureBuffer:
			case dsMaterialType_ImageBuffer:
			{
				if (glShader->uniforms[i].location < 0)
					continue;

				dsGfxFormat format;
				size_t offset, count;
				dsGfxBuffer* buffer = dsSharedMaterialValues_getTextureBufferId(&format,
					&offset, &count, sharedValues, i);
				if (buffer)
				{
					dsGLCommandBuffer_setTextureBuffer(commandBuffer, shader, i, buffer,
						format, offset, count);
				}
				else
					dsGLCommandBuffer_setTexture(commandBuffer, shader, i, NULL);
				break;
			}
			case dsMaterialType_UniformBlock:
			case dsMaterialType_UniformBuffer:
			{
				if (glShader->uniforms[i].location < 0)
					continue;

				size_t offset, size;
				dsGfxBuffer* buffer = dsSharedMaterialValues_getBufferId(&offset, &size,
					sharedValues, nameID);
				if (!buffer)
				{
					errno = EPERM;
					DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG,
						"No buffer set for shared material value '%s'",
						materialDesc->elements[i].name);
					return false;
				}
				dsGLCommandBuffer_setShaderBuffer(commandBuffer, shader, i, buffer, offset, size);
				break;
			}
			case dsMaterialType_VariableGroup:
			{
				if (useGfxBuffers)
				{
					if (glShader->uniforms[i].location < 0)
						continue;

					size_t offset, size;
					dsGfxBuffer* buffer = dsSharedMaterialValues_getBufferId(&offset, &size,
						sharedValues, nameID);
					if (!buffer)
					{
						errno = EPERM;
						DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG,
							"No buffer set for shared material value '%s'",
							materialDesc->elements[i].name);
						return false;
					}
					dsGLCommandBuffer_setShaderBuffer(commandBuffer, shader, i, buffer, offset,
						size);
				}
				else
				{
					dsShaderVariableGroup* variableGroup =
						dsSharedMaterialValues_getVariableGroupId(sharedValues, nameID);
					if (!variableGroup)
					{
						errno = EPERM;
						DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG,
							"No variable group set for material value '%s'",
							materialDesc->elements[i].name);
						return false;
					}

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

static bool bindMaterial(dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsMaterial* material, const dsSharedMaterialValues* globalValues)
{
	dsGLShader* glShader = (dsGLShader*)shader;
	bool useGfxBuffers = dsShaderVariableGroup_useGfxBuffer(shader->resourceManager);
	const dsMaterialDesc* materialDesc = shader->materialDesc;
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		if (materialDesc->elements[i].binding != dsMaterialBinding_Material)
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
			case dsMaterialType_TextureBuffer:
			case dsMaterialType_ImageBuffer:
			{
				if (glShader->uniforms[i].location < 0)
					continue;

				dsGfxFormat format;
				size_t offset, count;
				dsGfxBuffer* buffer = dsMaterial_getTextureBuffer(&format, &offset, &count, material,
					i);
				if (buffer)
				{
					dsGLCommandBuffer_setTextureBuffer(commandBuffer, shader, i, buffer,
						format, offset, count);
				}
				else
					dsGLCommandBuffer_setTexture(commandBuffer, shader, i, NULL);
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
				return false;
		}

		for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
		{
			glCommandBuffer->commitCounts[i].variableGroup = NULL;
			glCommandBuffer->commitCounts[i].commitCount = DS_VARIABLE_GROUP_UNSET_COMMIT;
		}
	}

	if (!setSharedMaterialValues(commandBuffer, shader, globalValues, dsMaterialBinding_Global))
		return false;

	return true;
}

void dsGLCommandBuffer_initialize(dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(commandBuffer);
	DS_ASSERT(commandBuffer->allocator);

	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	glCommandBuffer->commitCounts = NULL;
	glCommandBuffer->commitCountSize = 0;
	glCommandBuffer->boundSurface = NULL;
}

void dsGLCommandBuffer_shutdown(dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(commandBuffer);

	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	dsAllocator_free(commandBuffer->allocator, glCommandBuffer->commitCounts);
}

void dsGLCommandBuffer_reset(dsCommandBuffer* commandBuffer)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	functions->resetCommandBuffer(commandBuffer);
}

bool dsGLCommandBuffer_copyBufferData(dsCommandBuffer* commandBuffer, dsGfxBuffer* buffer,
	size_t offset, const void* data, size_t size)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->copyBufferDataFunc(commandBuffer, buffer, offset, data, size);
}

bool dsGLCommandBuffer_copyBuffer(dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer,
	size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset, size_t size)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->copyBufferFunc(commandBuffer, srcBuffer, srcOffset, dstBuffer, dstOffset,
		size);
}

bool dsGLCommandBuffer_copyBufferToTexture(dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer,
	dsTexture* dstTexture, const dsGfxBufferTextureCopyRegion* regions, uint32_t regionCount)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->copyBufferToTextureFunc(commandBuffer, srcBuffer, dstTexture, regions,
		regionCount);
}

bool dsGLCommandBuffer_copyTextureData(dsCommandBuffer* commandBuffer, dsTexture* texture,
	const dsTexturePosition* position, uint32_t width, uint32_t height, uint32_t layers,
	const void* data, size_t size)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->copyTextureDataFunc(commandBuffer, texture, position, width, height, layers,
		data, size);
}

bool dsGLCommandBuffer_copyTextureToBuffer(dsCommandBuffer* commandBuffer, dsTexture* srcTexture,
	dsGfxBuffer* dstBuffer, const dsGfxBufferTextureCopyRegion* regions, uint32_t regionCount)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->copyTextureToBufferFunc(commandBuffer, srcTexture, dstBuffer, regions,
		regionCount);
}

bool dsGLCommandBuffer_copyTexture(dsCommandBuffer* commandBuffer, dsTexture* srcTexture,
	dsTexture* dstTexture, const dsTextureCopyRegion* regions, uint32_t regionCount)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->copyTextureFunc(commandBuffer, srcTexture, dstTexture, regions, regionCount);
}

bool dsGLCommandBuffer_generateTextureMipmaps(dsCommandBuffer* commandBuffer, dsTexture* texture)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->generateTextureMipmapsFunc(commandBuffer, texture);
}

bool dsGLCommandBuffer_setFenceSyncs(dsCommandBuffer* commandBuffer, dsGLFenceSyncRef** syncs,
	uint32_t syncCount, bool bufferReadback)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->setFenceSyncsFunc(commandBuffer, syncs, syncCount, bufferReadback);
}

bool dsGLCommandBuffer_beginQuery(dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries,
	uint32_t query)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->beginQueryFunc(commandBuffer, queries, query);
}

bool dsGLCommandBuffer_endQuery(dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries,
	uint32_t query)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->endQueryFunc(commandBuffer, queries, query);
}

bool dsGLCommandBuffer_queryTimestamp(dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries,
	uint32_t query)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->queryTimestampFunc(commandBuffer, queries, query);
}

bool dsGLCommandBuffer_copyQueryValues(dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries,
	uint32_t first, uint32_t count, dsGfxBuffer* buffer, size_t offset, size_t stride,
	size_t elementSize, bool checkAvailability)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->copyQueryValuesFunc(commandBuffer, queries, first, count, buffer, offset,
		stride, elementSize, checkAvailability);
}

bool dsGLCommandBuffer_bindShaderAndMaterial(dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsMaterial* material, const dsSharedMaterialValues* globalValues,
	const dsDynamicRenderStates* renderStates)
{
	DS_ASSERT(commandBuffer);
	DS_ASSERT(shader);
	DS_ASSERT(material);

	if (!dsGLCommandBuffer_bindShader(commandBuffer, shader, renderStates))
		return false;

	if (!bindMaterial(commandBuffer, shader, material, globalValues))
	{
		dsGLCommandBuffer_unbindShader(commandBuffer, shader);
		return false;
	}

	return true;
}

bool dsGLCommandBuffer_bindShader(dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsDynamicRenderStates* renderStates)
{
	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	const CommandBufferFunctionTable* functions = glCommandBuffer->functions;
	return functions->bindShaderFunc(commandBuffer, shader, renderStates);
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

bool dsGLCommandBuffer_setInstanceMaterialValues(dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsSharedMaterialValues* instanceValues)
{
	return setSharedMaterialValues(commandBuffer, shader, instanceValues,
		dsMaterialBinding_Instance);
}

bool dsGLCommandBuffer_updateDynamicRenderStates(dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsDynamicRenderStates* renderStates)
{
	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	const CommandBufferFunctionTable* functions = glCommandBuffer->functions;
	return functions->updateDynamicRenderStatesFunc(commandBuffer, shader, renderStates);
}

bool dsGLCommandBuffer_unbindShader(dsCommandBuffer* commandBuffer, const dsShader* shader)
{
	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	const CommandBufferFunctionTable* functions = glCommandBuffer->functions;
	return functions->unbindShaderFunc(commandBuffer, shader);
}

bool dsGLCommandBuffer_bindComputeShaderAndMaterial(dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsMaterial* material,
	const dsSharedMaterialValues* sharedValues)
{
	DS_ASSERT(commandBuffer);
	DS_ASSERT(shader);
	DS_ASSERT(material);

	if (!dsGLCommandBuffer_bindComputeShader(commandBuffer, shader))
		return false;

	if (!bindMaterial(commandBuffer, shader, material, sharedValues))
	{
		dsGLCommandBuffer_unbindComputeShader(commandBuffer, shader);
		return false;
	}

	return true;
}

bool dsGLCommandBuffer_setComputeInstanceMaterialValues(dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsSharedMaterialValues* instanceValues)
{
	return setSharedMaterialValues(commandBuffer, shader, instanceValues,
		dsMaterialBinding_Instance);
}

bool dsGLCommandBuffer_bindComputeShader(dsCommandBuffer* commandBuffer, const dsShader* shader)
{
	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	const CommandBufferFunctionTable* functions = glCommandBuffer->functions;
	return functions->bindComputeShaderFunc(commandBuffer, shader);
}

bool dsGLCommandBuffer_unbindComputeShader(dsCommandBuffer* commandBuffer, const dsShader* shader)
{
	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	const CommandBufferFunctionTable* functions = glCommandBuffer->functions;
	return functions->unbindComputeShaderFunc(commandBuffer, shader);
}

bool dsGLCommandBuffer_beginRenderSurface(dsCommandBuffer* commandBuffer, void* glSurface)
{
	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	DS_ASSERT(!glCommandBuffer->boundSurface);
	const CommandBufferFunctionTable* functions = glCommandBuffer->functions;
	if (!functions->beginRenderSurfaceFunc(commandBuffer, glSurface))
		return false;

	glCommandBuffer->boundSurface = glSurface;
	return true;
}

bool dsGLCommandBuffer_endRenderSurface(dsCommandBuffer* commandBuffer, void* glSurface)
{
	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	DS_ASSERT(glCommandBuffer->boundSurface == glSurface);
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
	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;

	const CommandBufferFunctionTable* functions = glCommandBuffer->functions;
	if (!functions->beginRenderPassFunc(commandBuffer, renderPass, framebuffer, viewport,
		clearValues, clearValueCount))
	{
		return false;
	}
	return true;
}

bool dsGLCommandBuffer_nextRenderSubpass(dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, uint32_t index)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->nextRenderSubpassFunc(commandBuffer, renderPass, index);
}

bool dsGLCommandBuffer_endRenderPass(dsCommandBuffer* commandBuffer, const dsRenderPass* renderPass)
{
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->endRenderPassFunc(commandBuffer, renderPass);
}

bool dsGLCommandBuffer_clearColorSurface(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsFramebufferSurface* surface,
	const dsSurfaceColorValue* colorValue)
{
	DS_UNUSED(renderer);
	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	const CommandBufferFunctionTable* functions = glCommandBuffer->functions;
	return functions->clearColorSurfaceFunc(commandBuffer, surface, colorValue);
}

bool dsGLCommandBuffer_clearDepthStencilSurface(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsFramebufferSurface* surface,
	dsClearDepthStencil surfaceParts, const dsDepthStencilValue* depthStencilValue)
{
	DS_UNUSED(renderer);
	dsGLCommandBuffer* glCommandBuffer = (dsGLCommandBuffer*)commandBuffer;
	const CommandBufferFunctionTable* functions = glCommandBuffer->functions;
	return functions->clearDepthStencilSurfaceFunc(commandBuffer, surface, surfaceParts,
		depthStencilValue);
}

bool dsGLCommandBuffer_draw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawRange* drawRange, dsPrimitiveType primitiveType)
{
	DS_UNUSED(renderer);
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->drawFunc(commandBuffer, geometry, drawRange, primitiveType);
}

bool dsGLCommandBuffer_drawIndexed(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawIndexedRange* drawRange,
	dsPrimitiveType primitiveType)
{
	DS_UNUSED(renderer);
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->drawIndexedFunc(commandBuffer, geometry, drawRange, primitiveType);
}

bool dsGLCommandBuffer_drawIndirect(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType)
{
	DS_UNUSED(renderer);
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->drawIndirectFunc(commandBuffer, geometry, indirectBuffer, offset, count,
		stride, primitiveType);
}

bool dsGLCommandBuffer_drawIndexedIndirect(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType)
{
	DS_UNUSED(renderer);
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->drawIndirectFunc(commandBuffer, geometry, indirectBuffer, offset, count,
		stride, primitiveType);
}

bool dsGLCommandBuffer_dispatchCompute(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	uint32_t x, uint32_t y, uint32_t z)
{
	DS_UNUSED(renderer);
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->dispatchComputeFunc(commandBuffer, x, y, z);
}

bool dsGLCommandBuffer_dispatchComputeIndirect(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsGfxBuffer* indirectBuffer, size_t offset)
{
	DS_UNUSED(renderer);
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->dispatchComputeIndirectFunc(commandBuffer, indirectBuffer, offset);
}

bool dsGLCommandBuffer_blitSurface(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsGfxSurfaceType srcSurfaceType, void* srcSurface, dsGfxSurfaceType dstSurfaceType,
	void* dstSurface, const dsSurfaceBlitRegion* regions, uint32_t regionCount, dsBlitFilter filter)
{
	DS_UNUSED(renderer);
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->blitSurfaceFunc(commandBuffer, srcSurfaceType, srcSurface, dstSurfaceType,
		dstSurface, regions, regionCount, filter);
}

bool dsGLCommandBuffer_pushDebugGroup(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const char* name)
{
	DS_UNUSED(renderer);
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->pushDebugGroupFunc(commandBuffer, name);
}

bool dsGLCommandBuffer_popDebugGroup(dsRenderer* renderer, dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(renderer);
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->popDebugGroupFunc(commandBuffer);
}

bool dsGLCommandBuffer_memoryBarrier(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsGfxMemoryBarrier* barriers, uint32_t barrierCount)
{
	DS_UNUSED(renderer);
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)commandBuffer)->functions;
	return functions->memoryBarrierFunc(commandBuffer, barriers, barrierCount);
}

bool dsGLCommandBuffer_begin(dsRenderer* renderer, dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(commandBuffer != renderer->mainCommandBuffer);
	DS_UNUSED(renderer);
	dsGLCommandBuffer_reset(commandBuffer);
	return true;
}

bool dsGLCommandBuffer_beginSecondary(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsFramebuffer* framebuffer, const dsRenderPass* renderPass, uint32_t subpass,
	const dsAlignedBox3f* viewport)
{
	DS_ASSERT(commandBuffer != renderer->mainCommandBuffer);
	DS_UNUSED(renderer);
	DS_UNUSED(framebuffer);
	DS_UNUSED(renderPass);
	DS_UNUSED(subpass);
	DS_UNUSED(viewport);
	dsGLCommandBuffer_reset(commandBuffer);
	return true;
}

bool dsGLCommandBuffer_end(dsRenderer* renderer, dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(commandBuffer != renderer->mainCommandBuffer);
	DS_UNUSED(renderer);
	DS_UNUSED(commandBuffer);
	return true;
}

bool dsGLCommandBuffer_submit(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer)
{
	DS_UNUSED(renderer);
	const CommandBufferFunctionTable* functions = ((dsGLCommandBuffer*)submitBuffer)->functions;
	return functions->submitFunc(commandBuffer, submitBuffer);
}

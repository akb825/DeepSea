/*
 * Copyright 2016 Aaron Barany
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

#include <DeepSea/Text/TextRenderBuffer.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Renderer.h>

dsTextRenderBuffer* dsTextRenderBuffer_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, uint32_t maxGlyphs, const dsVertexFormat* vertexFormat,
	bool tessellationShader, dsGlyphDataFunction glyphDataFunc, void* userData)
{
	if (!allocator || !resourceManager || maxGlyphs == 0 || !vertexFormat || !glyphDataFunc)
	{
		errno = EINVAL;
		return NULL;
	}

	if (tessellationShader && !resourceManager->renderer->hasTessellationShaders)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Tessellation shaders aren't supported by the renderer.");
		return NULL;
	}

	const uint32_t max16BitIndices = 1 << 16;
	uint32_t indexSize = (uint32_t)(maxGlyphs*4 < max16BitIndices ? sizeof(uint16_t) :
		sizeof(uint32_t));
	uint32_t vertexCount = tessellationShader ? 1 : 4;
	uint32_t vertexBufferSize = vertexFormat->size*maxGlyphs*vertexCount;
	uint32_t indexBufferSize = tessellationShader ? 0 : indexSize*maxGlyphs*6;
	unsigned int usage = dsGfxBufferUsage_Vertex | dsGfxBufferUsage_CopyTo;
	if (!tessellationShader)
		usage |= dsGfxBufferUsage_Index;
	dsGfxBuffer* gfxBuffer = dsGfxBuffer_create(resourceManager, allocator, usage,
		dsGfxMemory_Stream | dsGfxMemory_Draw, NULL, vertexBufferSize + indexBufferSize);
	if (!gfxBuffer)
		return NULL;

	dsVertexBuffer vertexBuffer = {gfxBuffer, 0, maxGlyphs*vertexCount, *vertexFormat};
	dsVertexBuffer* vertexBufferPtrs[DS_MAX_GEOMETRY_VERTEX_BUFFERS] =
		{&vertexBuffer, NULL, NULL, NULL};
	dsIndexBuffer indexBuffer = {gfxBuffer, vertexBufferSize, maxGlyphs*6, indexSize};
	dsDrawGeometry* geometry = dsDrawGeometry_create(resourceManager, allocator, vertexBufferPtrs,
		tessellationShader ? NULL : &indexBuffer);
	if (!geometry)
	{
		DS_VERIFY(dsGfxBuffer_destroy(gfxBuffer));
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsTextRenderBuffer)) +
		DS_ALIGNED_SIZE(vertexBufferSize + indexBufferSize);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		DS_VERIFY(dsDrawGeometry_destroy(geometry));
		DS_VERIFY(dsGfxBuffer_destroy(gfxBuffer));
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsTextRenderBuffer* renderBuffer = (dsTextRenderBuffer*)dsAllocator_alloc(
		(dsAllocator*)&bufferAlloc, sizeof(dsTextRenderBuffer));
	DS_ASSERT(renderBuffer);
	renderBuffer->allocator = dsAllocator_keepPointer(allocator);
	renderBuffer->geometry = geometry;
	renderBuffer->glyphDataFunc = glyphDataFunc;
	renderBuffer->userData = userData;
	renderBuffer->maxGlyphs = maxGlyphs;
	renderBuffer->queuedGlyphs = 0;
	renderBuffer->tempData = dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		vertexBufferSize + indexBufferSize);
	DS_ASSERT(renderBuffer->tempData);

	return renderBuffer;
}

bool dsTextRenderBuffer_addText(dsTextRenderBuffer* renderBuffer, const dsTextLayout* text,
	uint32_t firstGlyph, uint32_t glyphCount)
{
	DS_PROFILE_FUNC_START();
	if (!renderBuffer || !text)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!DS_IS_BUFFER_RANGE_VALID(firstGlyph, glyphCount, text->text->glyphCount))
	{
		errno = EINDEX;
		DS_PROFILE_FUNC_RETURN(false);
	}

	uint32_t emptyGlyphs = 0;
	for (uint32_t i = 0; i < glyphCount; ++i)
	{
		const dsGlyphLayout* glyph = text->glyphs + firstGlyph + i;
		if (glyph->geometry.min.x == glyph->geometry.max.x ||
			glyph->geometry.min.y == glyph->geometry.max.y)
		{
			++emptyGlyphs;
		}
	}

	if (!DS_IS_BUFFER_RANGE_VALID(renderBuffer->queuedGlyphs, glyphCount - emptyGlyphs,
		renderBuffer->maxGlyphs))
	{
		errno = EINDEX;
		DS_PROFILE_FUNC_RETURN(false);
	}

	uint32_t vertexSize = renderBuffer->geometry->vertexBuffers[0].format.size;
	uint32_t indexSize = renderBuffer->geometry->indexBuffer.indexSize;
	unsigned int vertexCount = indexSize == 0 ? 1 : 4;
	DS_ASSERT(renderBuffer->queuedGlyphs <= renderBuffer->maxGlyphs);
	for (uint32_t i = 0; i < glyphCount; ++i)
	{
		// Skip empty glyphs.
		const dsGlyphLayout* glyph = text->glyphs + firstGlyph + i;
		if (glyph->geometry.min.x == glyph->geometry.max.x ||
			glyph->geometry.min.y == glyph->geometry.max.y)
		{
			continue;
		}

		uint32_t vertexOffset = vertexSize*renderBuffer->queuedGlyphs*vertexCount;
		renderBuffer->glyphDataFunc(renderBuffer->userData, text, firstGlyph + i,
			(uint8_t*)renderBuffer->tempData + vertexOffset,
			&renderBuffer->geometry->vertexBuffers[0].format, vertexCount);

		uint32_t indexOffset = renderBuffer->geometry->indexBuffer.offset +
			indexSize*renderBuffer->queuedGlyphs*6;
		if (indexSize == sizeof(uint32_t))
		{
			uint32_t* indices = (uint32_t*)((uint8_t*)renderBuffer->tempData + indexOffset);
			*(indices++) = renderBuffer->queuedGlyphs*4;
			*(indices++) = renderBuffer->queuedGlyphs*4 + 1;
			*(indices++) = renderBuffer->queuedGlyphs*4 + 2;

			*(indices++) = renderBuffer->queuedGlyphs*4 + 2;
			*(indices++) = renderBuffer->queuedGlyphs*4 + 3;
			*(indices++) = renderBuffer->queuedGlyphs*4;
		}
		else if (indexSize == sizeof(uint16_t))
		{
			uint16_t* indices = (uint16_t*)((uint8_t*)renderBuffer->tempData + indexOffset);
			*(indices++) = (uint16_t)(renderBuffer->queuedGlyphs*4);
			*(indices++) = (uint16_t)(renderBuffer->queuedGlyphs*4 + 1);
			*(indices++) = (uint16_t)(renderBuffer->queuedGlyphs*4 + 2);

			*(indices++) = (uint16_t)(renderBuffer->queuedGlyphs*4 + 2);
			*(indices++) = (uint16_t)(renderBuffer->queuedGlyphs*4 + 3);
			*(indices++) = (uint16_t)(renderBuffer->queuedGlyphs*4);
		}
		++renderBuffer->queuedGlyphs;
	}

	DS_PROFILE_FUNC_RETURN(true);
}

bool dsTextRenderBuffer_commit(dsTextRenderBuffer* renderBuffer, dsCommandBuffer* commandBuffer)
{
	if (!renderBuffer || !commandBuffer)
	{
		errno = EINVAL;
		return false;
	}

	if (renderBuffer->queuedGlyphs == 0)
		return true;

	// If close to the full size, do one copy operation.
	uint32_t vertexSize = renderBuffer->geometry->vertexBuffers[0].format.size;
	uint32_t indexSize = renderBuffer->geometry->indexBuffer.indexSize;
	uint32_t vertexCount = indexSize == 0 ? 1 : 4;
	dsGfxBuffer* gfxBuffer = renderBuffer->geometry->vertexBuffers[0].buffer;
	if (renderBuffer->queuedGlyphs >= renderBuffer->maxGlyphs/4*3)
	{
		if (!dsGfxBuffer_copyData(gfxBuffer, commandBuffer, 0, renderBuffer->tempData,
			renderBuffer->geometry->vertexBuffers[0].count*vertexSize +
			renderBuffer->geometry->indexBuffer.count*indexSize))
		{
			return false;
		}
	}
	else
	{
		if (!dsGfxBuffer_copyData(gfxBuffer, commandBuffer, 0, renderBuffer->tempData,
			renderBuffer->queuedGlyphs*vertexSize*vertexCount))
		{
			return false;
		}

		if (indexSize > 0)
		{
			uint32_t offset = renderBuffer->geometry->indexBuffer.offset;
			if (!dsGfxBuffer_copyData(gfxBuffer, commandBuffer, offset,
				(uint8_t*)renderBuffer->tempData + offset, renderBuffer->queuedGlyphs*indexSize*6))
			{
				return false;
			}
		}
	}

	return true;
}

bool dsTextRenderBuffer_clear(dsTextRenderBuffer* renderBuffer)
{
	if (!renderBuffer)
	{
		errno = EINVAL;
		return false;
	}

	renderBuffer->queuedGlyphs = 0;
	return true;
}

bool dsTextRenderBuffer_draw(dsTextRenderBuffer* renderBuffer, dsCommandBuffer* commandBuffer)
{
	if (!renderBuffer || !commandBuffer)
	{
		errno = EINVAL;
		return false;
	}

	if (renderBuffer->queuedGlyphs == 0)
		return true;

	uint32_t indexSize = renderBuffer->geometry->indexBuffer.indexSize;
	if (indexSize == 0)
	{
		dsDrawRange drawRange = {renderBuffer->queuedGlyphs, 1, 0, 0};
		if (!dsRenderer_draw(commandBuffer->renderer, commandBuffer, renderBuffer->geometry,
			&drawRange))
		{
			return false;
		}
	}
	else
	{
		dsDrawIndexedRange drawRange = {renderBuffer->queuedGlyphs*6, 1, 0, 0, 0};
		if (!dsRenderer_drawIndexed(commandBuffer->renderer, commandBuffer, renderBuffer->geometry,
			&drawRange))
		{
			return false;
		}
	}

	return true;
}

bool dsTextRenderBuffer_destroy(dsTextRenderBuffer* renderBuffer)
{
	if (!renderBuffer)
	{
		errno = EINVAL;
		return false;
	}

	if (!dsGfxBuffer_destroy(renderBuffer->geometry->vertexBuffers[0].buffer))
		return false;

	DS_VERIFY(dsDrawGeometry_destroy(renderBuffer->geometry));
	if (renderBuffer->allocator)
		DS_VERIFY(dsAllocator_free(renderBuffer->allocator, renderBuffer));
	return true;
}

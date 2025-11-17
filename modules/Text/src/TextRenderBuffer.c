/*
 * Copyright 2016-2025 Aaron Barany
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

#include "TextInternal.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Geometry/AlignedBox2.h>

#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Renderer.h>

#include <DeepSea/Text/TextIcons.h>

uint32_t dsTextRenderBuffer_countStandardGlyphs(
	const dsTextLayout* layout, uint32_t firstChar, uint32_t charCount)
{
	if (!layout || !DS_IS_BUFFER_RANGE_VALID(firstChar, charCount, layout->text->characterCount))
		return 0;

	uint32_t glyphCount = 0;
	for (uint32_t i = 0; i < charCount; ++i)
	{
		const dsCharMapping* charMapping = layout->text->charMappings + firstChar + i;
		for (uint32_t j = 0; j < charMapping->glyphCount; ++j)
		{
			const dsGlyphLayout* glyph = layout->glyphs + charMapping->firstGlyph + j;
			if (glyph->geometry.min.x < glyph->geometry.max.x &&
				glyph->geometry.min.y < glyph->geometry.max.y &&
				dsAlignedBox2_isValid(glyph->texCoords))
			{
				++glyphCount;
			}
		}
	}

	return glyphCount;
}

uint32_t dsTextRenderBuffer_countIconGlyphs(
	const dsTextLayout* layout, uint32_t firstChar, uint32_t charCount)
{
	if (!layout || !DS_IS_BUFFER_RANGE_VALID(firstChar, charCount, layout->text->characterCount))
		return 0;

	uint32_t glyphCount = 0;
	for (uint32_t i = 0; i < charCount; ++i)
	{
		const dsCharMapping* charMapping = layout->text->charMappings + firstChar + i;
		for (uint32_t j = 0; j < charMapping->glyphCount; ++j)
		{
			const dsGlyphLayout* glyph = layout->glyphs + charMapping->firstGlyph + j;
			if (glyph->geometry.min.x < glyph->geometry.max.x &&
				glyph->geometry.min.y < glyph->geometry.max.y &&
				!dsAlignedBox2_isValid(glyph->texCoords))
			{
				++glyphCount;
			}
		}
	}

	return glyphCount;
}

bool dsTextRenderBuffer_countStandardIconGlyphs(uint32_t* outStandardCount, uint32_t* outIconCount,
	const dsTextLayout* layout, uint32_t firstChar, uint32_t charCount)
{
	if (!outStandardCount || !outIconCount || !layout ||
		!DS_IS_BUFFER_RANGE_VALID(firstChar, charCount, layout->text->characterCount))
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < charCount; ++i)
	{
		const dsCharMapping* charMapping = layout->text->charMappings + firstChar + i;
		for (uint32_t j = 0; j < charMapping->glyphCount; ++j)
		{
			const dsGlyphLayout* glyph = layout->glyphs + charMapping->firstGlyph + j;
			if (glyph->geometry.min.x < glyph->geometry.max.x &&
				glyph->geometry.min.y < glyph->geometry.max.y)
			{
				bool standard = dsAlignedBox2_isValid(glyph->texCoords);
				*outStandardCount += standard;
				*outIconCount += !standard;
			}
		}
	}
	return true;
}

dsTextRenderBuffer* dsTextRenderBuffer_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, uint32_t maxStandardGlyphs, uint32_t maxIconGlyphs,
	const dsVertexFormat* vertexFormat, bool tessellationShader, dsGlyphDataFunction glyphDataFunc,
	void* userData)
{
	if (!allocator || !resourceManager || (maxStandardGlyphs == 0 && maxIconGlyphs == 0) ||
		!vertexFormat || !glyphDataFunc)
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
	uint32_t indexSize = (uint32_t)(maxStandardGlyphs*4 < max16BitIndices ? sizeof(uint16_t) :
		sizeof(uint32_t));
	uint32_t vertexCount = tessellationShader ? 1 : 4;
	uint32_t vertexBufferSize = vertexFormat->size*maxStandardGlyphs*vertexCount;
	uint32_t indexBufferSize = tessellationShader ? 0 : indexSize*maxStandardGlyphs*6;
	dsGfxBufferUsage usage = dsGfxBufferUsage_Vertex | dsGfxBufferUsage_CopyTo;
	if (!tessellationShader)
		usage |= dsGfxBufferUsage_Index;
	uint32_t bufferSize = vertexBufferSize + indexBufferSize;
	dsGfxBuffer* gfxBuffer = dsGfxBuffer_create(resourceManager, allocator, usage,
		dsGfxMemory_Stream | dsGfxMemory_Draw | dsGfxMemory_GPUOnly, NULL, bufferSize);
	if (!gfxBuffer)
		return NULL;

	dsVertexBuffer vertexBuffer = {gfxBuffer, 0, maxStandardGlyphs*vertexCount, *vertexFormat};
	dsVertexBuffer* vertexBufferPtrs[DS_MAX_GEOMETRY_VERTEX_BUFFERS] =
		{&vertexBuffer, NULL, NULL, NULL};
	dsIndexBuffer indexBuffer = {gfxBuffer, vertexBufferSize, maxStandardGlyphs*6, indexSize};
	dsDrawGeometry* geometry = dsDrawGeometry_create(resourceManager, allocator, vertexBufferPtrs,
		tessellationShader ? NULL : &indexBuffer);
	if (!geometry)
	{
		DS_VERIFY(dsGfxBuffer_destroy(gfxBuffer));
		return NULL;
	}

	uint32_t tempDataSize = bufferSize + (uint32_t)sizeof(dsIconGlyph)*maxIconGlyphs +
		(uint32_t)sizeof(dsTextIcons*)*maxIconGlyphs;

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsTextRenderBuffer)) +
		DS_ALIGNED_SIZE(tempDataSize);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		DS_VERIFY(dsDrawGeometry_destroy(geometry));
		DS_VERIFY(dsGfxBuffer_destroy(gfxBuffer));
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsTextRenderBuffer* renderBuffer = DS_ALLOCATE_OBJECT(&bufferAlloc, dsTextRenderBuffer);
	DS_ASSERT(renderBuffer);
	renderBuffer->allocator = dsAllocator_keepPointer(allocator);
	renderBuffer->geometry = geometry;
	renderBuffer->glyphDataFunc = glyphDataFunc;
	renderBuffer->userData = userData;
	renderBuffer->maxStandardGlyphs = maxStandardGlyphs;
	renderBuffer->maxIconGlyphs = maxIconGlyphs;
	renderBuffer->queuedStandardGlyphs = 0;
	renderBuffer->queuedIconGlyphs = 0;
	renderBuffer->tempIconOffset = bufferSize;
	renderBuffer->tempData = dsAllocator_alloc((dsAllocator*)&bufferAlloc, tempDataSize);
	DS_ASSERT(renderBuffer->tempData);

	return renderBuffer;
}

bool dsTextRenderBuffer_addText(
	dsTextRenderBuffer* renderBuffer, const dsTextLayout* layout, void* layoutUserData)
{
	if (!renderBuffer || !layout)
	{
		errno = EINVAL;
		return false;
	}

	return dsTextRenderBuffer_addTextRange(renderBuffer, layout, layoutUserData, 0,
		layout->text->characterCount);
}

bool dsTextRenderBuffer_addTextRange(dsTextRenderBuffer* renderBuffer,
	const dsTextLayout* layout, void* layoutUserData, uint32_t firstChar, uint32_t charCount)
{
	DS_PROFILE_FUNC_START();
	if (!renderBuffer || !layout)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!DS_IS_BUFFER_RANGE_VALID(firstChar, charCount, layout->text->characterCount))
	{
		errno = EINDEX;
		DS_PROFILE_FUNC_RETURN(false);
	}

	uint32_t standardGlyphCount = dsTextRenderBuffer_countStandardGlyphs(
		layout, firstChar, charCount);
	if (!DS_IS_BUFFER_RANGE_VALID(renderBuffer->queuedStandardGlyphs, standardGlyphCount,
			renderBuffer->maxStandardGlyphs))
	{
		errno = EINDEX;
		DS_PROFILE_FUNC_RETURN(false);
	}

	uint32_t iconGlyphCount = dsTextRenderBuffer_countIconGlyphs(layout, firstChar, charCount);
	if (!DS_IS_BUFFER_RANGE_VALID(
			renderBuffer->queuedIconGlyphs, iconGlyphCount, renderBuffer->maxIconGlyphs))
	{
		errno = EINDEX;
		DS_PROFILE_FUNC_RETURN(false);
	}

	uint32_t vertexSize = renderBuffer->geometry->vertexBuffers[0].format.size;
	uint32_t indexSize = renderBuffer->geometry->indexBuffer.indexSize;
	unsigned int vertexCount = indexSize == 0 ? 1 : 4;
	uint8_t* standardGlyphData = (uint8_t*)renderBuffer->tempData;
	dsIconGlyph* iconGlyphData = (dsIconGlyph*)(standardGlyphData + renderBuffer->tempIconOffset);
	const dsTextIcons** textIconPtrs =
		(const dsTextIcons**)(iconGlyphData + renderBuffer->maxIconGlyphs);
	const dsTextIcons* icons = layout->text->font->icons;
	for (uint32_t i = 0; i < charCount; ++i)
	{
		const dsCharMapping* charMapping = layout->text->charMappings + firstChar + i;
		for (uint32_t j = 0; j < charMapping->glyphCount; ++j)
		{
			const dsGlyphLayout* glyph = layout->glyphs + charMapping->firstGlyph + j;

			// Skip empty glyphs.
			if (glyph->geometry.min.x >= glyph->geometry.max.x ||
				glyph->geometry.min.y >= glyph->geometry.max.y)
			{
				continue;
			}

			if (dsAlignedBox2_isValid(glyph->texCoords))
			{
				uint32_t vertexOffset = vertexSize*renderBuffer->queuedStandardGlyphs*vertexCount;
				renderBuffer->glyphDataFunc(renderBuffer->userData, layout, layoutUserData,
					charMapping->firstGlyph + j, standardGlyphData + vertexOffset,
					&renderBuffer->geometry->vertexBuffers[0].format, vertexCount);

				size_t indexOffset = renderBuffer->geometry->indexBuffer.offset +
					indexSize*renderBuffer->queuedStandardGlyphs*6;
				if (indexSize == sizeof(uint32_t))
				{
					uint32_t* indices = (uint32_t*)(standardGlyphData + indexOffset);
					*(indices++) = renderBuffer->queuedStandardGlyphs*4;
					*(indices++) = renderBuffer->queuedStandardGlyphs*4 + 1;
					*(indices++) = renderBuffer->queuedStandardGlyphs*4 + 2;

					*(indices++) = renderBuffer->queuedStandardGlyphs*4 + 2;
					*(indices++) = renderBuffer->queuedStandardGlyphs*4 + 3;
					*(indices++) = renderBuffer->queuedStandardGlyphs*4;
				}
				else if (indexSize == sizeof(uint16_t))
				{
					uint16_t* indices = (uint16_t*)(standardGlyphData + indexOffset);
					*(indices++) = (uint16_t)(renderBuffer->queuedStandardGlyphs*4);
					*(indices++) = (uint16_t)(renderBuffer->queuedStandardGlyphs*4 + 1);
					*(indices++) = (uint16_t)(renderBuffer->queuedStandardGlyphs*4 + 2);

					*(indices++) = (uint16_t)(renderBuffer->queuedStandardGlyphs*4 + 2);
					*(indices++) = (uint16_t)(renderBuffer->queuedStandardGlyphs*4 + 3);
					*(indices++) = (uint16_t)(renderBuffer->queuedStandardGlyphs*4);
				}
				++renderBuffer->queuedStandardGlyphs;
			}
			else
			{
				uint32_t codepoint = glyph->mipLevel;
				const dsIconGlyph* baseIcon = dsTextIcons_findIcon(icons, codepoint);
				DS_ASSERT(baseIcon);
				dsIconGlyph* tempIcon = iconGlyphData + renderBuffer->queuedStandardGlyphs;
				tempIcon->codepoint = codepoint;
				tempIcon->advance = baseIcon->advance;
				tempIcon->bounds = glyph->geometry;
				tempIcon->userData = baseIcon->userData;
				textIconPtrs[renderBuffer->queuedStandardGlyphs] = icons;
				++renderBuffer->queuedStandardGlyphs;
			}
		}
	}

	DS_PROFILE_FUNC_RETURN(true);
}

uint32_t dsTextRenderBuffer_getRemainingStandardGlyphs(const dsTextRenderBuffer* renderBuffer)
{
	if (!renderBuffer)
		return 0;

	return renderBuffer->maxStandardGlyphs - renderBuffer->queuedStandardGlyphs;
}

uint32_t dsTextRenderBuffer_getRemainingIconGlyphs(const dsTextRenderBuffer* renderBuffer)
{
	if (!renderBuffer)
		return 0;

	return renderBuffer->maxIconGlyphs - renderBuffer->queuedIconGlyphs;
}

bool dsTextRenderBuffer_commit(dsTextRenderBuffer* renderBuffer, dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	if (!renderBuffer || !commandBuffer)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (renderBuffer->queuedStandardGlyphs > 0)
	{
		// If close to the full size, do one copy operation.
		uint32_t vertexSize = renderBuffer->geometry->vertexBuffers[0].format.size;
		uint32_t indexSize = renderBuffer->geometry->indexBuffer.indexSize;
		uint32_t vertexCount = indexSize == 0 ? 1 : 4;
		dsGfxBuffer* gfxBuffer = renderBuffer->geometry->vertexBuffers[0].buffer;
		if (renderBuffer->queuedStandardGlyphs >= renderBuffer->maxStandardGlyphs/4*3)
		{
			if (!dsGfxBuffer_copyData(
					gfxBuffer, commandBuffer, 0, renderBuffer->tempData, gfxBuffer->size))
			{
				DS_PROFILE_FUNC_RETURN(false);
			}
		}
		else
		{
			if (!dsGfxBuffer_copyData(gfxBuffer, commandBuffer, 0, renderBuffer->tempData,
					renderBuffer->queuedStandardGlyphs*vertexSize*vertexCount))
			{
				DS_PROFILE_FUNC_RETURN(false);
			}

			if (indexSize > 0)
			{
				size_t offset = renderBuffer->geometry->indexBuffer.offset;
				if (!dsGfxBuffer_copyData(gfxBuffer, commandBuffer, offset,
						(uint8_t*)renderBuffer->tempData + offset,
						renderBuffer->queuedStandardGlyphs*indexSize*6))
				{
					DS_PROFILE_FUNC_RETURN(false);
				}
			}
		}
	}

	if (renderBuffer->queuedIconGlyphs > 0)
	{
		dsIconGlyph* iconGlyphs = (dsIconGlyph*)((uint8_t*)renderBuffer->tempData +
			renderBuffer->tempIconOffset);
		const dsTextIcons** icons = (const dsTextIcons**)(iconGlyphs + renderBuffer->maxIconGlyphs);
		const dsTextIcons* curIcons = icons[0];
		uint32_t start = 0;
		uint32_t count = 0;
		for (uint32_t i = 1; i < renderBuffer->queuedIconGlyphs; ++i, ++count)
		{
			const dsTextIcons* thisIcons = icons[i];
			if (thisIcons != curIcons)
			{
				if (curIcons->prepareFunc && !curIcons->prepareFunc(
						curIcons, curIcons->userData, iconGlyphs + start, count))
				{
					DS_PROFILE_FUNC_RETURN(false);
				}

				start = i;
				count = 0;
				curIcons = thisIcons;
			}
		}

		if (curIcons->prepareFunc && !curIcons->prepareFunc(
				curIcons, curIcons->userData, iconGlyphs + start, count))
		{
			DS_PROFILE_FUNC_RETURN(false);
		}
	}

	DS_PROFILE_FUNC_RETURN(true);
}

bool dsTextRenderBuffer_clear(dsTextRenderBuffer* renderBuffer)
{
	if (!renderBuffer)
	{
		errno = EINVAL;
		return false;
	}

	renderBuffer->queuedStandardGlyphs = 0;
	renderBuffer->queuedIconGlyphs = 0;
	return true;
}

bool dsTextRenderBuffer_drawStandardGlyphs(
	dsTextRenderBuffer* renderBuffer, dsCommandBuffer* commandBuffer)
{
	if (!renderBuffer || !commandBuffer)
	{
		errno = EINVAL;
		return false;
	}

	return dsTextRenderBuffer_drawStandardGlyphRange(
		renderBuffer, commandBuffer, 0, renderBuffer->queuedStandardGlyphs);
}

bool dsTextRenderBuffer_drawStandardGlyphRange(dsTextRenderBuffer* renderBuffer,
	dsCommandBuffer* commandBuffer, uint32_t firstGlyph, uint32_t glyphCount)
{
	DS_PROFILE_FUNC_START();

	if (!renderBuffer || !commandBuffer)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!DS_IS_BUFFER_RANGE_VALID(firstGlyph, glyphCount, renderBuffer->queuedStandardGlyphs))
	{
		errno = ESIZE;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (glyphCount == 0)
		DS_PROFILE_FUNC_RETURN(true);

	uint32_t indexSize = renderBuffer->geometry->indexBuffer.indexSize;
	bool result;
	if (indexSize == 0)
	{
		dsDrawRange drawRange = {glyphCount, 1, firstGlyph, 0};
		result = dsRenderer_draw(commandBuffer->renderer, commandBuffer, renderBuffer->geometry,
			&drawRange, dsPrimitiveType_PatchList);
	}
	else
	{
		dsDrawIndexedRange drawRange = {glyphCount*6, 1, firstGlyph*6, 0, 0};
		result = dsRenderer_drawIndexed(commandBuffer->renderer, commandBuffer,
			renderBuffer->geometry, &drawRange, dsPrimitiveType_TriangleList);
	}

	DS_PROFILE_FUNC_RETURN(result);
}

bool dsTextRenderBuffer_drawIconGlyphs(
	dsTextRenderBuffer* renderBuffer, dsCommandBuffer* commandBuffer)
{
	if (!renderBuffer || !commandBuffer)
	{
		errno = EINVAL;
		return false;
	}

	return dsTextRenderBuffer_drawIconGlyphRange(
		renderBuffer, commandBuffer, 0, renderBuffer->queuedIconGlyphs);
}

bool dsTextRenderBuffer_drawIconGlyphRange(dsTextRenderBuffer* renderBuffer,
	dsCommandBuffer* commandBuffer, uint32_t firstGlyph, uint32_t glyphCount)
{
	DS_PROFILE_FUNC_START();

	if (!renderBuffer || !commandBuffer)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!DS_IS_BUFFER_RANGE_VALID(firstGlyph, glyphCount, renderBuffer->queuedIconGlyphs))
	{
		errno = ESIZE;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (glyphCount == 0)
		DS_PROFILE_FUNC_RETURN(true);

	dsIconGlyph* iconGlyphs = (dsIconGlyph*)((uint8_t*)renderBuffer->tempData +
		renderBuffer->tempIconOffset);
	const dsTextIcons** icons = (const dsTextIcons**)(iconGlyphs + renderBuffer->maxIconGlyphs);
	const dsTextIcons* curIcons = icons[0];
	uint32_t start = firstGlyph;
	uint32_t count = 0;
	for (uint32_t i = 1; i < glyphCount; ++i, ++count)
	{
		const dsTextIcons* thisIcons = icons[firstGlyph + i];
		if (thisIcons != curIcons)
		{
			if (!curIcons->drawFunc(curIcons, curIcons->userData, iconGlyphs + start, count))
			{
				DS_PROFILE_FUNC_RETURN(false);
			}

			start = firstGlyph + i;
			count = 0;
			curIcons = thisIcons;
		}
	}

	bool result = curIcons->drawFunc(curIcons, curIcons->userData, iconGlyphs + start, count);
	DS_PROFILE_FUNC_RETURN(result);
}

bool dsTextRenderBuffer_destroy(dsTextRenderBuffer* renderBuffer)
{
	if (!renderBuffer)
		return true;

	if (!dsGfxBuffer_destroy(renderBuffer->geometry->vertexBuffers[0].buffer))
		return false;

	DS_VERIFY(dsDrawGeometry_destroy(renderBuffer->geometry));
	if (renderBuffer->allocator)
		DS_VERIFY(dsAllocator_free(renderBuffer->allocator, renderBuffer));
	return true;
}

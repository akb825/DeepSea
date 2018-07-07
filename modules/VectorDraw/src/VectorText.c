/*
 * Copyright 2018 Aaron Barany
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

#include "VectorText.h"
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Text/TextLayout.h>
#include <DeepSea/Text/TextRenderBuffer.h>
#include <DeepSea/VectorDraw/VectorMaterialSet.h>

typedef struct TextVertex
{
	dsVector2f position;
	dsVector2f offset;
	dsVector2f texCoords;
	uint16_t mipLevel;
	uint16_t infoIndex;
	uint16_t fillMaterialIndex;
	uint16_t outlineMaterialIndex;
} TextVertex;

typedef struct TessTextVertex
{
	dsVector2f position;
	dsAlignedBox2f geometry;
	dsAlignedBox2f texCoords;
	uint16_t mipLevel;
	uint16_t infoIndex;
	uint16_t fillMaterialIndex;
	uint16_t outlineMaterialIndex;
} TessTextVertex;

static void getRangeOffset(dsVector2f* outOffset, const dsTextLayout* layout,
	const dsVectorCommandTextRange* range)
{
	if (range->positionType == dsVectorTextPosition_Absolute)
	{
		const dsCharMapping* charMapping = layout->text->charMappings + range->start;
		dsVector2_sub(*outOffset, range->position,
			layout->glyphs[charMapping->firstGlyph].position);
	}
	else
		*outOffset = range->position;
}

static uint32_t countGlyphs(const dsDrawIndexedRange* range, const TextDrawInfo* drawInfos,
	uint32_t infoCount)
{
	DS_UNUSED(infoCount);
	uint32_t count = 0;
	for (uint32_t i = 0; i < range->indexCount; ++i)
	{
		DS_ASSERT(range->firstIndex + i < infoCount);
		const TextDrawInfo* drawInfo = drawInfos + range->firstIndex + i;
		for (uint32_t j = 0; j < drawInfo->characterCount; ++j)
		{
			const dsCharMapping* charMapping = drawInfo->layout->text->charMappings +
				drawInfo->firstCharacter + j;
			count += charMapping->glyphCount;
		}
	}

	return count;
}

static void textVertexData(void* userData, const dsTextLayout* layout, void* userLayerData,
	uint32_t glyphIndex, void* vertexData, const dsVertexFormat* format, uint32_t vertexCount)
{
	DS_UNUSED(userData);
	DS_UNUSED(format);
	DS_UNUSED(vertexCount);
	DS_ASSERT(vertexCount == 4);
	DS_ASSERT(format->size == sizeof(TextVertex));

	const TextDrawInfo* drawInfo = (const TextDrawInfo*)userLayerData;
	const dsGlyphLayout* glyph = layout->glyphs + glyphIndex;

	TextVertex* vertex = (TextVertex*)vertexData;
	dsVector2_add(vertex->position, glyph->position, drawInfo->offset);
	vertex->offset.x = glyph->geometry.min.x;
	vertex->offset.y = glyph->geometry.min.y;
	vertex->texCoords.x = glyph->texCoords.min.x;
	vertex->texCoords.y = glyph->texCoords.min.y;
	vertex->mipLevel = (uint16_t)glyph->mipLevel;
	vertex->infoIndex = (uint16_t)drawInfo->infoIndex;
	vertex->fillMaterialIndex = (uint16_t)drawInfo->fillMaterial;
	vertex->outlineMaterialIndex = (uint16_t)drawInfo->outlineMaterial;

	++vertex;
	dsVector2_add(vertex->position, glyph->position, drawInfo->offset);
	vertex->offset.x = glyph->geometry.min.x;
	vertex->offset.y = glyph->geometry.max.y;
	vertex->texCoords.x = glyph->texCoords.min.x;
	vertex->texCoords.y = glyph->texCoords.max.y;
	vertex->mipLevel = (uint16_t)glyph->mipLevel;
	vertex->infoIndex = (uint16_t)drawInfo->infoIndex;
	vertex->fillMaterialIndex = (uint16_t)drawInfo->fillMaterial;
	vertex->outlineMaterialIndex = (uint16_t)drawInfo->outlineMaterial;

	++vertex;
	dsVector2_add(vertex->position, glyph->position, drawInfo->offset);
	vertex->offset.x = glyph->geometry.max.x;
	vertex->offset.y = glyph->geometry.max.y;
	vertex->texCoords.x = glyph->texCoords.max.x;
	vertex->texCoords.y = glyph->texCoords.max.y;
	vertex->mipLevel = (uint16_t)glyph->mipLevel;
	vertex->infoIndex = (uint16_t)drawInfo->infoIndex;
	vertex->fillMaterialIndex = (uint16_t)drawInfo->fillMaterial;
	vertex->outlineMaterialIndex = (uint16_t)drawInfo->outlineMaterial;

	++vertex;
	dsVector2_add(vertex->position, glyph->position, drawInfo->offset);
	vertex->offset.x = glyph->geometry.max.x;
	vertex->offset.y = glyph->geometry.min.y;
	vertex->texCoords.x = glyph->texCoords.max.x;
	vertex->texCoords.y = glyph->texCoords.min.y;
	vertex->mipLevel = (uint16_t)glyph->mipLevel;
	vertex->infoIndex = (uint16_t)drawInfo->infoIndex;
	vertex->fillMaterialIndex = (uint16_t)drawInfo->fillMaterial;
	vertex->outlineMaterialIndex = (uint16_t)drawInfo->outlineMaterial;
}

static void tessTextVertexData(void* userData, const dsTextLayout* layout, void* userLayerData,
	uint32_t glyphIndex, void* vertexData, const dsVertexFormat* format, uint32_t vertexCount)
{
	DS_UNUSED(userData);
	DS_UNUSED(format);
	DS_UNUSED(vertexCount);
	DS_ASSERT(vertexCount == 1);
	DS_ASSERT(format->size == sizeof(TessTextVertex));

	const TextDrawInfo* drawInfo = (const TextDrawInfo*)userLayerData;
	const dsGlyphLayout* glyph = layout->glyphs + glyphIndex;
	TessTextVertex* vertex = (TessTextVertex*)vertexData;
	dsVector2_add(vertex->position, glyph->position, drawInfo->offset);
	vertex->geometry = glyph->geometry;
	vertex->texCoords = glyph->texCoords;
	vertex->mipLevel = (uint16_t)glyph->mipLevel;
	vertex->infoIndex = (uint16_t)drawInfo->infoIndex;
	vertex->fillMaterialIndex = (uint16_t)drawInfo->fillMaterial;
	vertex->outlineMaterialIndex = (uint16_t)drawInfo->outlineMaterial;
}

bool dsVectorText_addText(dsVectorScratchData* scratchData, dsCommandBuffer* commandBuffer,
	const dsVectorMaterialSet* sharedMaterials, const dsVectorMaterialSet* localMaterials,
	const dsVectorCommandText* text, const dsVectorCommand* rangeCommands, float pixelSize)
{
	dsTextLayout* layout = dsVectorScratchData_shapeText(scratchData, commandBuffer, text->string,
		text->stringType, text->font, text->justification, text->maxLength, text->lineHeight,
		rangeCommands, text->rangeCount, pixelSize);
	if (!layout)
		return false;

	dsAlignedBox2f bounds;
	dsAlignedBox2f_makeInvalid(&bounds);
	dsVector2f offset = {{0.0f, 0.0f}};
	for (uint32_t i = 0; i < text->rangeCount; ++i)
	{
		DS_ASSERT(rangeCommands[i].commandType == dsVectorCommandType_TextRange);
		const dsVectorCommandTextRange* range = &rangeCommands[i].textRange;
		getRangeOffset(&offset, layout, range);

		for (uint32_t j = 0; j < range->count; ++j)
		{
			const dsCharMapping*charMapping = layout->text->charMappings + range->start + j;
			for (uint32_t k = 0; k < charMapping->glyphCount; ++k)
			{
				const dsGlyphLayout* glyph = layout->glyphs + charMapping->firstGlyph + k;
				dsAlignedBox2f glyphBounds;
				DS_VERIFY(dsTextLayout_applySlantToBounds(&glyphBounds, &glyph->geometry,
					range->slant));
				dsVector2f position;
				dsVector2_add(position, glyph->position, offset);
				dsVector2_add(glyphBounds.min, position, glyph->geometry.min);
				dsVector2_add(glyphBounds.max, position, glyph->geometry.max);
				dsAlignedBox2_addBox(bounds, glyphBounds);
			}
		}
	}

	for (uint32_t i = 0; i < text->rangeCount; ++i)
	{
		DS_ASSERT(rangeCommands[i].commandType == dsVectorCommandType_TextRange);
		const dsVectorCommandTextRange* range = &rangeCommands[i].textRange;
		getRangeOffset(&offset, layout, range);

		uint32_t fillMaterial = DS_VECTOR_MATERIAL_NOT_FOUND;
		if (range->fillMaterial)
		{
			fillMaterial = dsVectorMaterialSet_findMaterialIndex(sharedMaterials,
				range->fillMaterial);
			if (fillMaterial == DS_VECTOR_MATERIAL_NOT_FOUND)
			{
				fillMaterial = dsVectorMaterialSet_findMaterialIndex(localMaterials,
					range->fillMaterial);
				if (fillMaterial != DS_VECTOR_MATERIAL_NOT_FOUND)
				{
					errno = ENOTFOUND;
					DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Material '%s' not found.",
						range->fillMaterial);
					return false;
				}
				fillMaterial += DS_VECTOR_LOCAL_MATERIAL_OFFSET;
			}
		}

		uint32_t outlineMaterial = DS_VECTOR_MATERIAL_NOT_FOUND;
		if (range->outlineMaterial)
		{
			outlineMaterial = dsVectorMaterialSet_findMaterialIndex(sharedMaterials,
				range->outlineMaterial);
			if (outlineMaterial == DS_VECTOR_MATERIAL_NOT_FOUND)
			{
				outlineMaterial = dsVectorMaterialSet_findMaterialIndex(localMaterials,
					range->outlineMaterial);
				if (outlineMaterial != DS_VECTOR_MATERIAL_NOT_FOUND)
				{
					errno = ENOTFOUND;
					DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Material '%s' not found.",
						range->outlineMaterial);
					return false;
				}
				outlineMaterial += DS_VECTOR_LOCAL_MATERIAL_OFFSET;
			}
		}

		if (fillMaterial == DS_VECTOR_MATERIAL_NOT_FOUND &&
			outlineMaterial == DS_VECTOR_MATERIAL_NOT_FOUND)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG, "Vector image text doesn't have any materials.");
			return false;
		}

		DS_ASSERT(range->start == scratchData->textStyles[i].start &&
			range->count == scratchData->textStyles[i].count);
		if (i == 0)
		{
			if (!dsVectorScratchData_addTextPiece(scratchData, &bounds, &text->transform, &offset,
				text->font, range->fillOpacity, range->outlineOpacity, layout,
				scratchData->textStyles + i, fillMaterial, outlineMaterial))
			{
				return false;
			}
		}
		else if (!dsVectorScratchData_addTextRange(scratchData, &offset, range->fillOpacity,
			range->outlineOpacity, layout, scratchData->textStyles + i, fillMaterial,
			outlineMaterial))
		{
			return false;
		}
	}

	return true;
}

bool dsVectorText_createVertexFormat(dsVertexFormat* outVertexFormat,
	const dsVectorImageInitResources* initResources)
{
	uint32_t textShaderIndex = initResources->shaderModule->textShaderIndex;
	if (initResources->textShaderName)
	{
		textShaderIndex = dsShaderModule_shaderIndex(initResources->shaderModule->shaderModule,
			initResources->textShaderName);
		if (textShaderIndex == DS_MATERIAL_UNKNOWN)
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
				"Vector image text shader '%s' not found.", initResources->textShaderName);
			return false;
		}
	}
	else if (textShaderIndex == DS_MATERIAL_UNKNOWN)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG, "No vector image text shader provided.");
		return false;
	}

	bool tessText = dsShaderModule_shaderIndexHasStage(initResources->shaderModule->shaderModule,
		textShaderIndex, dsShaderStage_TessellationEvaluation);

	if (tessText)
	{
		DS_VERIFY(dsVertexFormat_initialize(outVertexFormat));
		outVertexFormat->elements[dsVertexAttrib_Position0].format =
			dsGfxFormat_decorate(dsGfxFormat_X32Y32, dsGfxFormat_Float);
		outVertexFormat->elements[dsVertexAttrib_Position1].format =
			dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);
		outVertexFormat->elements[dsVertexAttrib_TexCoord0].format =
			dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);
		outVertexFormat->elements[dsVertexAttrib_TexCoord1].format =
			dsGfxFormat_decorate(dsGfxFormat_X16Y16Z16W16, dsGfxFormat_UInt);
		DS_VERIFY(dsVertexFormat_setAttribEnabled(outVertexFormat, dsVertexAttrib_Position0, true));
		DS_VERIFY(dsVertexFormat_setAttribEnabled(outVertexFormat, dsVertexAttrib_Position1, true));
		DS_VERIFY(dsVertexFormat_setAttribEnabled(outVertexFormat, dsVertexAttrib_TexCoord0, true));
		DS_VERIFY(dsVertexFormat_setAttribEnabled(outVertexFormat, dsVertexAttrib_TexCoord1, true));
		DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(outVertexFormat));
		DS_ASSERT(outVertexFormat->elements[dsVertexAttrib_Position0].offset ==
			offsetof(TessTextVertex, position));
		DS_ASSERT(outVertexFormat->elements[dsVertexAttrib_Position1].offset ==
			offsetof(TessTextVertex, geometry));
		DS_ASSERT(outVertexFormat->elements[dsVertexAttrib_TexCoord0].offset ==
			offsetof(TessTextVertex, texCoords));
		DS_ASSERT(outVertexFormat->elements[dsVertexAttrib_TexCoord1].offset ==
			offsetof(TessTextVertex, mipLevel));
		DS_ASSERT(outVertexFormat->size == sizeof(TessTextVertex));
	}
	else
	{
		DS_VERIFY(dsVertexFormat_initialize(outVertexFormat));
		outVertexFormat->elements[dsVertexAttrib_Position].format =
			dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);
		outVertexFormat->elements[dsVertexAttrib_TexCoord0].format =
			dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
		outVertexFormat->elements[dsVertexAttrib_TexCoord1].format =
			dsGfxFormat_decorate(dsGfxFormat_X16Y16Z16W16, dsGfxFormat_UInt);
		DS_VERIFY(dsVertexFormat_setAttribEnabled(outVertexFormat, dsVertexAttrib_Position, true));
		DS_VERIFY(dsVertexFormat_setAttribEnabled(outVertexFormat, dsVertexAttrib_TexCoord0, true));
		DS_VERIFY(dsVertexFormat_setAttribEnabled(outVertexFormat, dsVertexAttrib_TexCoord1, true));
		DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(outVertexFormat));
		DS_ASSERT(outVertexFormat->elements[dsVertexAttrib_Position].offset ==
			offsetof(TextVertex, position));
		DS_ASSERT(outVertexFormat->elements[dsVertexAttrib_TexCoord0].offset ==
			offsetof(TextVertex, texCoords));
		DS_ASSERT(outVertexFormat->elements[dsVertexAttrib_TexCoord1].offset ==
			offsetof(TextVertex, mipLevel));
		DS_ASSERT(outVertexFormat->size == sizeof(TextVertex));
	}

	return true;
}

dsTextRenderBuffer* dsVectorText_createRenderBuffer(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsVertexFormat* vertexFormat,
	const dsDrawIndexedRange* range, const TextDrawInfo* drawInfos, uint32_t infoCount)
{
	uint32_t glyphCount = countGlyphs(range, drawInfos, infoCount);
	DS_ASSERT(glyphCount > 0);
	DS_ASSERT(vertexFormat->size == sizeof(TextVertex) ||
		vertexFormat->size == sizeof(TessTextVertex));
	bool tessText = vertexFormat->size == sizeof(TessTextVertex);
	return dsTextRenderBuffer_create(allocator, resourceManager, glyphCount, vertexFormat,
		tessText, tessText ? &tessTextVertexData : &textVertexData, NULL);
}

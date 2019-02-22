/*
 * Copyright 2018-2019 Aaron Barany
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
#include <DeepSea/Core/Profile.h>
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
	int16_t mipLevel;
	int16_t infoIndex;
	int16_t fillMaterialIndex;
	int16_t outlineMaterialIndex;
} TextVertex;

typedef struct TessTextVertex
{
	dsVector2f position;
	dsAlignedBox2f geometry;
	dsAlignedBox2f texCoords;
	int16_t mipLevel;
	int16_t infoIndex;
	int16_t fillMaterialIndex;
	int16_t outlineMaterialIndex;
} TessTextVertex;

static bool glyphRightToLeft(const dsTextLayout* layout, uint32_t glyphIndex)
{
	for (uint32_t i = 0; i < layout->text->rangeCount; ++i)
	{
		const dsTextRange* range = layout->text->ranges + i;
		if (glyphIndex >= range->firstGlyph &&
			glyphIndex < range->firstGlyph + range->glyphCount)
		{
			return range->backward;
		}
	}

	DS_ASSERT(false);
	return false;
}

static void getRangeOffset(dsVector2f* outOffset, const dsTextLayout* layout,
	const dsVectorCommandTextRange* range)
{
	if (range->positionType == dsVectorTextPosition_Absolute)
	{
		outOffset->x = 0.0f;
		outOffset->y = 0.0f;
		// Need to find the first character on the first line that has a valid position.
		for (uint32_t i = 0; i < range->count; ++i)
		{
			const dsCharMapping* charMapping = layout->text->charMappings + range->start + i;
			const dsGlyphLayout* glyph = layout->glyphs + charMapping->firstGlyph;
			if (glyph->position.x != FLT_MAX && glyph->position.y == 0.0f)
			{
				dsVector2_sub(*outOffset, range->position, glyph->position);
				if (glyphRightToLeft(layout, charMapping->firstGlyph))
				{
					outOffset->x -= layout->text->glyphs[i].advance*
						layout->styles[glyph->styleIndex].scale;
				}
				break;
			}
		}
	}
	else
		dsVector2_add(*outOffset, *outOffset, range->position);
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
			for (uint32_t k = 0; k < charMapping->glyphCount; ++k)
			{
				const dsGlyphLayout* glyph = drawInfo->layout->glyphs + charMapping->firstGlyph + k;
				if (glyph->geometry.min.x < glyph->geometry.max.y && glyph->geometry.min.y <
					glyph->geometry.max.y)
				{
					++count;
				}
			}
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
	vertex->mipLevel = (int16_t)glyph->mipLevel;
	vertex->infoIndex = (int16_t)drawInfo->infoIndex;
	vertex->fillMaterialIndex = (int16_t)drawInfo->fillMaterial;
	vertex->outlineMaterialIndex = (int16_t)drawInfo->outlineMaterial;

	++vertex;
	dsVector2_add(vertex->position, glyph->position, drawInfo->offset);
	vertex->offset.x = glyph->geometry.min.x;
	vertex->offset.y = glyph->geometry.max.y;
	vertex->texCoords.x = glyph->texCoords.min.x;
	vertex->texCoords.y = glyph->texCoords.max.y;
	vertex->mipLevel = (int16_t)glyph->mipLevel;
	vertex->infoIndex = (int16_t)drawInfo->infoIndex;
	vertex->fillMaterialIndex = (int16_t)drawInfo->fillMaterial;
	vertex->outlineMaterialIndex = (int16_t)drawInfo->outlineMaterial;

	++vertex;
	dsVector2_add(vertex->position, glyph->position, drawInfo->offset);
	vertex->offset.x = glyph->geometry.max.x;
	vertex->offset.y = glyph->geometry.max.y;
	vertex->texCoords.x = glyph->texCoords.max.x;
	vertex->texCoords.y = glyph->texCoords.max.y;
	vertex->mipLevel = (int16_t)glyph->mipLevel;
	vertex->infoIndex = (int16_t)drawInfo->infoIndex;
	vertex->fillMaterialIndex = (int16_t)drawInfo->fillMaterial;
	vertex->outlineMaterialIndex = (int16_t)drawInfo->outlineMaterial;

	++vertex;
	dsVector2_add(vertex->position, glyph->position, drawInfo->offset);
	vertex->offset.x = glyph->geometry.max.x;
	vertex->offset.y = glyph->geometry.min.y;
	vertex->texCoords.x = glyph->texCoords.max.x;
	vertex->texCoords.y = glyph->texCoords.min.y;
	vertex->mipLevel = (int16_t)glyph->mipLevel;
	vertex->infoIndex = (int16_t)drawInfo->infoIndex;
	vertex->fillMaterialIndex = (int16_t)drawInfo->fillMaterial;
	vertex->outlineMaterialIndex = (int16_t)drawInfo->outlineMaterial;
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
	vertex->mipLevel = (int16_t)glyph->mipLevel;
	vertex->infoIndex = (int16_t)drawInfo->infoIndex;
	vertex->fillMaterialIndex = (int16_t)drawInfo->fillMaterial;
	vertex->outlineMaterialIndex = (int16_t)drawInfo->outlineMaterial;
}

bool dsVectorText_addText(dsVectorScratchData* scratchData, dsCommandBuffer* commandBuffer,
	const dsVectorMaterialSet* sharedMaterials, const dsVectorMaterialSet* localMaterials,
	const dsVectorCommandText* text, const dsVectorCommand* rangeCommands, float pixelSize)
{
	DS_PROFILE_FUNC_START();

	dsTextLayout* layout = dsVectorScratchData_shapeText(scratchData, commandBuffer, text->string,
		text->stringType, text->font, text->alignment, text->maxLength, text->lineHeight,
		rangeCommands, text->rangeCount, pixelSize);
	if (!layout)
		DS_PROFILE_FUNC_RETURN(false);

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
				if (glyph->position.x == FLT_MAX || glyph->position.y == FLT_MAX)
					continue;

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

	MaterialSource fillMaterialSource = MaterialSource_Local;
	MaterialSource outlineMaterialSource = MaterialSource_Local;

	offset.x = 0.0f;
	offset.y = 0.0f;
	for (uint32_t i = 0; i < text->rangeCount; ++i)
	{
		DS_ASSERT(rangeCommands[i].commandType == dsVectorCommandType_TextRange);
		const dsVectorCommandTextRange* range = &rangeCommands[i].textRange;
		getRangeOffset(&offset, layout, range);

		uint32_t fillMaterial = DS_VECTOR_MATERIAL_NOT_FOUND;
		dsVectorMaterialType fillMaterialType = dsVectorMaterialType_Color;
		if (range->fillMaterial)
		{
			fillMaterial = dsVectorMaterialSet_findMaterialIndex(sharedMaterials,
				range->fillMaterial);
			if (fillMaterial == DS_VECTOR_MATERIAL_NOT_FOUND)
			{
				fillMaterial = dsVectorMaterialSet_findMaterialIndex(localMaterials,
					range->fillMaterial);
				if (fillMaterial == DS_VECTOR_MATERIAL_NOT_FOUND)
				{
					errno = ENOTFOUND;
					DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Material '%s' not found.",
						range->fillMaterial);
					DS_PROFILE_FUNC_RETURN(false);
				}
				fillMaterialType = dsVectorMaterialSet_getMaterialType(localMaterials,
					range->fillMaterial);
				fillMaterialSource = MaterialSource_Local;
			}
			else
			{
				fillMaterialType = dsVectorMaterialSet_getMaterialType(sharedMaterials,
					range->fillMaterial);
				fillMaterialSource = MaterialSource_Shared;
			}
		}

		uint32_t outlineMaterial = DS_VECTOR_MATERIAL_NOT_FOUND;
		dsVectorMaterialType outlineMaterialType = dsVectorMaterialType_Color;
		if (range->outlineMaterial)
		{
			outlineMaterial = dsVectorMaterialSet_findMaterialIndex(sharedMaterials,
				range->outlineMaterial);
			if (outlineMaterial == DS_VECTOR_MATERIAL_NOT_FOUND)
			{
				outlineMaterial = dsVectorMaterialSet_findMaterialIndex(localMaterials,
					range->outlineMaterial);
				if (outlineMaterial == DS_VECTOR_MATERIAL_NOT_FOUND)
				{
					errno = ENOTFOUND;
					DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Material '%s' not found.",
						range->outlineMaterial);
					DS_PROFILE_FUNC_RETURN(false);
				}
				outlineMaterialType = dsVectorMaterialSet_getMaterialType(localMaterials,
					range->outlineMaterial);
				outlineMaterialSource = MaterialSource_Local;
			}
			else
			{
				outlineMaterialType = dsVectorMaterialSet_getMaterialType(sharedMaterials,
					range->outlineMaterial);
				outlineMaterialSource = MaterialSource_Shared;
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
				scratchData->textStyles + i, fillMaterial, outlineMaterial, fillMaterialType,
				outlineMaterialType, fillMaterialSource, outlineMaterialSource))
			{
				DS_PROFILE_FUNC_RETURN(false);
			}
		}
		else if (!dsVectorScratchData_addTextRange(scratchData, &offset, range->fillOpacity,
			range->outlineOpacity, layout, scratchData->textStyles + i, fillMaterial,
			outlineMaterial, fillMaterialType, outlineMaterialType, fillMaterialSource,
			outlineMaterialSource))
		{
			DS_PROFILE_FUNC_RETURN(false);
		}
	}

	DS_PROFILE_FUNC_RETURN(true);
}

bool dsVectorText_createVertexFormat(dsVertexFormat* outVertexFormat,
	const dsVectorImageInitResources* initResources)
{
	uint32_t textShaderIndex =
		initResources->shaderModule->shaderIndices[dsVectorShaderType_TextColor];
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
			dsGfxFormat_decorate(dsGfxFormat_X16Y16Z16W16, dsGfxFormat_SInt);
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
			dsGfxFormat_decorate(dsGfxFormat_X32Y32, dsGfxFormat_Float);
		outVertexFormat->elements[dsVertexAttrib_TexCoord1].format =
			dsGfxFormat_decorate(dsGfxFormat_X16Y16Z16W16, dsGfxFormat_SInt);
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

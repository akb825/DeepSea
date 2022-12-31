/*
 * Copyright 2020-2022 Aaron Barany
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

#include <DeepSea/SceneVectorDraw/SceneTextNode.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/SceneVectorDraw/SceneVectorNode.h>
#include <DeepSea/Text/TextLayout.h>
#include <DeepSea/Text/TextRenderBuffer.h>

#include <string.h>

typedef struct TextVertex
{
	dsVector2f position;
	dsColor textColor;
	dsColor outlineColor;
	dsVector3f texCoords;
	float embolden;
	float outlinePosition;
	float outlineThickness;
	float antiAlias;
} TextVertex;

typedef struct TessTextVertexx
{
	dsVector2f position;
	dsVector2f mipAntiAlias;
	dsAlignedBox2f geometry;
	dsColor textColor;
	dsColor outlineColor;
	dsAlignedBox2f texCoords;
	float slant;
	float embolden;
	float outlinePosition;
	float outlineThickness;
} TessTextVertex;

static void glyphPosition(dsVector2f* outPos, const dsVector2f* basePos,
	const dsVector2f* geometryPos, float slant)
{
	dsVector2_add(*outPos, *basePos, *geometryPos);
	outPos->x -= geometryPos->y*slant;
}

bool dsSceneTextNode_defaultTextVertexFormat(dsVertexFormat* outFormat)
{
	if (!dsVertexFormat_initialize(outFormat))
		return false;

	outFormat->elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32, dsGfxFormat_Float);
	outFormat->elements[dsVertexAttrib_Color0].format =
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	outFormat->elements[dsVertexAttrib_Color1].format =
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	outFormat->elements[dsVertexAttrib_TexCoord0].format = dsGfxFormat_decorate(
		dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
	outFormat->elements[dsVertexAttrib_TexCoord1].format = dsGfxFormat_decorate(
		dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);

	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Position, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Color0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Color1, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_TexCoord0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_TexCoord1, true));
	DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(outFormat));
	return true;
}

bool dsSceneTextNode_defaultTessVertexTextFormat(dsVertexFormat* outFormat)
{
	if (!dsVertexFormat_initialize(outFormat))
		return false;

	outFormat->elements[dsVertexAttrib_Position0].format = dsGfxFormat_decorate(
		dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);
	outFormat->elements[dsVertexAttrib_Position1].format = dsGfxFormat_decorate(
		dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);
	outFormat->elements[dsVertexAttrib_Color0].format = dsGfxFormat_decorate(
		dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	outFormat->elements[dsVertexAttrib_Color1].format = dsGfxFormat_decorate(
		dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	outFormat->elements[dsVertexAttrib_TexCoord0].format = dsGfxFormat_decorate(
		dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);
	outFormat->elements[dsVertexAttrib_TexCoord1].format = dsGfxFormat_decorate(
		dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);

	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Position0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Position1, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Color0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_Color1, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_TexCoord0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(outFormat, dsVertexAttrib_TexCoord1, true));
	DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(outFormat));
	return true;
}

void dsSceneTextNode_defaultGlyphDataFunc(void* userData, const dsTextLayout* layout,
	void* layoutUserData, uint32_t glyphIndex, void* vertexData, const dsVertexFormat* format,
	uint32_t vertexCount)
{
	DS_UNUSED(userData);
	DS_UNUSED(layoutUserData);
	DS_UNUSED(format);
	DS_UNUSED(vertexCount);
	DS_ASSERT(format->elements[dsVertexAttrib_Position].offset == offsetof(TextVertex, position));
	DS_ASSERT(format->elements[dsVertexAttrib_Color0].offset == offsetof(TextVertex, textColor));
	DS_ASSERT(format->elements[dsVertexAttrib_Color1].offset == offsetof(TextVertex, outlineColor));
	DS_ASSERT(format->elements[dsVertexAttrib_TexCoord0].offset == offsetof(TextVertex, texCoords));
	DS_ASSERT(format->elements[dsVertexAttrib_TexCoord1].offset == offsetof(TextVertex, embolden));
	DS_ASSERT(format->size == sizeof(TextVertex));
	DS_ASSERT(vertexCount == 4);

	const dsTextStyle* style = layout->styles + layout->glyphs[glyphIndex].styleIndex;
	const dsGlyphLayout* glyph = layout->glyphs + glyphIndex;
	dsVector2f position = layout->glyphs[glyphIndex].position;

	dsVector2f geometryPos;
	TextVertex* vertices = (TextVertex*)vertexData;
	geometryPos.x = glyph->geometry.min.x;
	geometryPos.y = glyph->geometry.min.y;
	glyphPosition(&vertices[0].position, &position, &geometryPos, style->slant);
	vertices[0].textColor = style->color;
	vertices[0].outlineColor = style->outlineColor;
	vertices[0].texCoords.x = glyph->texCoords.min.x;
	vertices[0].texCoords.y = glyph->texCoords.min.y;
	vertices[0].texCoords.z = (float)glyph->mipLevel;
	vertices[0].embolden = style->embolden;
	vertices[0].outlinePosition = style->outlinePosition;
	vertices[0].outlineThickness = style->outlineThickness;
	vertices[0].antiAlias = style->antiAlias;

	geometryPos.x = glyph->geometry.min.x;
	geometryPos.y = glyph->geometry.max.y;
	glyphPosition(&vertices[1].position, &position, &geometryPos, style->slant);
	vertices[1].textColor = style->color;
	vertices[1].outlineColor = style->outlineColor;
	vertices[1].texCoords.x = glyph->texCoords.min.x;
	vertices[1].texCoords.y = glyph->texCoords.max.y;
	vertices[1].texCoords.z = (float)glyph->mipLevel;
	vertices[1].embolden = style->embolden;
	vertices[1].outlinePosition = style->outlinePosition;
	vertices[1].outlineThickness = style->outlineThickness;
	vertices[1].antiAlias = style->antiAlias;

	geometryPos.x = glyph->geometry.max.x;
	geometryPos.y = glyph->geometry.max.y;
	glyphPosition(&vertices[2].position, &position, &geometryPos, style->slant);
	vertices[2].textColor = style->color;
	vertices[2].outlineColor = style->outlineColor;
	vertices[2].texCoords.x = glyph->texCoords.max.x;
	vertices[2].texCoords.y = glyph->texCoords.max.y;
	vertices[2].texCoords.z = (float)glyph->mipLevel;
	vertices[2].embolden = style->embolden;
	vertices[2].outlinePosition = style->outlinePosition;
	vertices[2].outlineThickness = style->outlineThickness;
	vertices[2].antiAlias = style->antiAlias;

	geometryPos.x = glyph->geometry.max.x;
	geometryPos.y = glyph->geometry.min.y;
	glyphPosition(&vertices[3].position, &position, &geometryPos, style->slant);
	vertices[3].textColor = style->color;
	vertices[3].outlineColor = style->outlineColor;
	vertices[3].texCoords.x = glyph->texCoords.max.x;
	vertices[3].texCoords.y = glyph->texCoords.min.y;
	vertices[3].texCoords.z = (float)glyph->mipLevel;
	vertices[3].embolden = style->embolden;
	vertices[3].outlinePosition = style->outlinePosition;
	vertices[3].outlineThickness = style->outlineThickness;
	vertices[3].antiAlias = style->antiAlias;
}

void dsSceneTextNode_defaultTessGlyphDataFunc(void* userData,
	const dsTextLayout* layout, void* layoutUserData, uint32_t glyphIndex, void* vertexData,
	const dsVertexFormat* format, uint32_t vertexCount)
{
	DS_UNUSED(userData);
	DS_UNUSED(layoutUserData);
	DS_UNUSED(format);
	DS_UNUSED(vertexCount);
	DS_ASSERT(format->elements[dsVertexAttrib_Position0].offset ==
		offsetof(TessTextVertex, position));
	DS_ASSERT(format->elements[dsVertexAttrib_Position1].offset ==
		offsetof(TessTextVertex, geometry));
	DS_ASSERT(format->elements[dsVertexAttrib_Color0].offset ==
		offsetof(TessTextVertex, textColor));
	DS_ASSERT(format->elements[dsVertexAttrib_Color1].offset ==
		offsetof(TessTextVertex, outlineColor));
	DS_ASSERT(format->elements[dsVertexAttrib_TexCoord0].offset ==
		offsetof(TessTextVertex, texCoords));
	DS_ASSERT(format->elements[dsVertexAttrib_TexCoord1].offset == offsetof(TessTextVertex, slant));
	DS_ASSERT(format->size == sizeof(TessTextVertex));
	DS_ASSERT(vertexCount == 1);

	TessTextVertex* vertex = (TessTextVertex*)vertexData;
	const dsTextStyle* style = layout->styles + layout->glyphs[glyphIndex].styleIndex;
	const dsGlyphLayout* glyph = layout->glyphs + glyphIndex;
	vertex->position.x = layout->glyphs[glyphIndex].position.x;
	vertex->position.y = layout->glyphs[glyphIndex].position.y;
	vertex->mipAntiAlias.x = (float)layout->glyphs[glyphIndex].mipLevel;
	vertex->mipAntiAlias.y = style->antiAlias;
	vertex->geometry = glyph->geometry;
	vertex->texCoords = glyph->texCoords;
	vertex->textColor = style->color;
	vertex->outlineColor = style->outlineColor;
	vertex->slant = style->slant;
	vertex->embolden = style->embolden;
	vertex->outlinePosition = style->outlinePosition;
	vertex->outlineThickness = style->outlineThickness;
}

const char* const dsSceneTextNode_typeName = "TextNode";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsSceneTextNode_type(void)
{
	return &nodeType;
}

const dsSceneNodeType* dsSceneTextNode_setupParentType(dsSceneNodeType* type)
{
	dsSceneNode_setupParentType(&nodeType, dsSceneVectorNode_type());
	return dsSceneNode_setupParentType(type, &nodeType);
}

dsSceneTextNode* dsSceneTextNode_create(dsAllocator* allocator, const dsText* text,
	void* textUserData, const dsTextStyle* styles, uint32_t styleCount, dsTextAlign alignment,
	float maxWidth, float lineScale, int32_t z, uint32_t firstChar, uint32_t charCount,
	dsShader* shader, dsMaterial* material, uint32_t fontTextureElement,
	const dsSceneTextRenderBufferInfo* textRenderBufferInfo, const char* const* itemLists,
	uint32_t itemListCount, dsSceneResources** resources, uint32_t resourceCount)
{
	return dsSceneTextNode_createBase(allocator, sizeof(dsSceneTextNode), text,
		textUserData, styles, styleCount, alignment, maxWidth, lineScale, z, firstChar, charCount,
		shader, material, fontTextureElement, textRenderBufferInfo, itemLists, itemListCount,
		resources, resourceCount);
}

dsSceneTextNode* dsSceneTextNode_createBase(dsAllocator* allocator, size_t structSize,
	const dsText* text, void* textUserData, const dsTextStyle* styles, uint32_t styleCount,
	dsTextAlign alignment, float maxWidth, float lineScale, int32_t z, uint32_t firstChar,
	uint32_t charCount, dsShader* shader, dsMaterial* material, uint32_t fontTextureElement,
	const dsSceneTextRenderBufferInfo* textRenderBufferInfo,
	const char* const* itemLists, uint32_t itemListCount, dsSceneResources** resources,
	uint32_t resourceCount)
{
	if (!allocator || !text || !styles || styleCount == 0 || !shader || !material ||
		!textRenderBufferInfo || !textRenderBufferInfo->vertexFormat ||
		!textRenderBufferInfo->glyphDataFunc || (!itemLists && itemListCount > 0) ||
		(!resources && resourceCount == 0))
	{
		errno = EINVAL;
		return NULL;
	}

	const dsMaterialDesc* materialDesc = dsMaterial_getDescription(material);
	DS_ASSERT(materialDesc);
	const dsMaterialElement* fontTextureMatElement = materialDesc->elements + fontTextureElement;
	if (fontTextureElement >= materialDesc->elementCount ||
		fontTextureMatElement->type != dsMaterialType_Texture ||
		fontTextureMatElement->binding != dsMaterialBinding_Instance)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR(DS_VECTOR_DRAW_SCENE_LOG_TAG,
			"Font texture element must be a texture with instance binding.");
		return NULL;
	}

	dsTextLayout* layout = dsTextLayout_create(allocator, text, styles, styleCount);
	if (!layout)
		return NULL;

	dsTextRenderBuffer* renderBuffer = dsTextRenderBuffer_create(allocator, shader->resourceManager,
		text->glyphCount, textRenderBufferInfo->vertexFormat,
		dsShader_hasStage(shader, dsShaderStage_TessellationEvaluation),
		textRenderBufferInfo->glyphDataFunc, textRenderBufferInfo->userData);
	if (!renderBuffer)
	{
		dsTextLayout_destroy(layout);
		return NULL;
	}

	// Add the style array to the struct size to pool allocations.
	size_t styleOffset = DS_ALIGNED_SIZE(structSize);
	size_t finalStructSize = styleOffset + DS_ALIGNED_SIZE(sizeof(dsTextStyle)*styleCount);

	dsSceneTextNode* node = (dsSceneTextNode*)dsSceneVectorNode_create(
		allocator, finalStructSize, z, itemLists, itemListCount, resources, resourceCount);
	if (!node)
	{
		dsTextLayout_destroy(layout);
		DS_VERIFY(dsTextRenderBuffer_destroy(renderBuffer));
		return NULL;
	}

	dsSceneNode* baseNode = (dsSceneNode*)node;
	baseNode->type = dsSceneTextNode_setupParentType(NULL);
	baseNode->destroyFunc = &dsSceneTextNode_destroy;

	node->layout = layout;
	node->renderBuffer = renderBuffer;
	node->textUserData = textUserData;
	node->shader = shader;
	node->material = material;
	node->styles = (dsTextStyle*)((uint8_t*)node + styleOffset);
	memcpy(node->styles, styles, sizeof(dsTextStyle)*styleCount);
	node->styleCount = styleCount;
	node->fontTextureID = fontTextureMatElement->nameID;
	node->alignment = alignment;
	node->maxWidth = maxWidth;
	node->lineScale = lineScale;
	node->firstChar = firstChar;
	node->charCount = charCount;
	node->layoutVersion = 0;

	return node;
}

void dsSceneTextNode_updateLayout(dsSceneTextNode* node)
{
	if (node)
		++node->layoutVersion;
}

void dsSceneTextNode_destroy(dsSceneNode* node)
{
	DS_ASSERT(dsSceneNode_isOfType(node, dsSceneTextNode_type()));
	dsSceneTextNode* textNode = (dsSceneTextNode*)node;
	dsTextLayout_destroy(textNode->layout);
	dsTextRenderBuffer_destroy(textNode->renderBuffer);
	dsSceneVectorNode_destroy(node);
}

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

#include "SetupOpenGL.h"
#include <DeepSea/Application/Application.h>
#include <DeepSea/Application/Window.h>
#include <DeepSea/ApplicationSDL/SDLApplication.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Render/Resources/Framebuffer.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Render/RenderPass.h>
#include <DeepSea/Text/FaceGroup.h>
#include <DeepSea/Text/Font.h>
#include <DeepSea/Text/Text.h>
#include <DeepSea/Text/TextLayout.h>
#include <DeepSea/Text/TextRenderBuffer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum dsRenderType
{
	dsRenderType_OpenGL,
	dsRenderType_Count
} dsRenderType;

typedef struct TestText
{
	dsAllocator* allocator;
	dsRenderer* renderer;
	dsWindow* window;
	dsFramebuffer* framebuffer;
	dsRenderPass* renderPass;
	dsShaderModule* shaderModule;
	dsShaderVariableGroupDesc* sharedInfoDesc;
	dsShaderVariableGroup* sharedInfoGroup;
	dsMaterialDesc* materialDesc;
	dsMaterial* material;
	dsMaterial* tessMaterial;
	dsShader* shader;
	dsShader* tessShader;
	dsFaceGroup* faceGroup;
	dsFont* font;
	dsTextLayout* text;
	dsTextRenderBuffer* textRender;
	dsTextLayout* tessText;
	dsTextRenderBuffer* tessTextRender;

	uint32_t screenSizeElement;
	uint32_t positionElement;
	uint32_t curString;
} TestText;

static const char* renderTypeNames[] =
{
	"OpenGL"
};

DS_STATIC_ASSERT(DS_ARRAY_SIZE(renderTypeNames) == dsRenderType_Count, renderer_type_mismatch);

#if DS_HAS_OPENGL
static dsRenderType defaultRenderType = dsRenderType_OpenGL;
#else
#error No renderer type available
#endif

static char assetsDir[DS_PATH_MAX];
static const char* shaderDir;

typedef struct StandardVertex
{
	dsVector2f position;
	dsColor textColor;
	dsColor outlineColor;
	dsVector3f texCoords;
	dsVector4f style;
} StandardVertex;

typedef struct TessVertexx
{
	dsVector4f position;
	dsAlignedBox2f geometry;
	dsColor textColor;
	dsColor outlineColor;
	dsAlignedBox2f texCoords;
	dsVector4f style;
} TessVertex;

typedef struct TextInfo
{
	const char* standardText;
	const char* tesselatedText;
	dsTextJustification justification;
	float maxWidth;
	float lineScale;
	dsTextStyle styles[3];
} TextInfo;

#define NO_STYLE {UINT_MAX, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, {{0, 0, 0, 0}}, {{0, 0, 0, 0}}}

static TextInfo textStrings[] =
{
	{"Top text is standard quads.\nUse arrow keys or touch to cycle text.",
		"Bottom text, if visible, is tessellated.",
		dsTextJustification_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}},
		NO_STYLE, NO_STYLE}},
	{"This text has been emboldened.", NULL,
		dsTextJustification_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.2f, 0.0f, 0.0f, 0.0f, 0.1f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}},
		NO_STYLE, NO_STYLE}},
	{"This text is slanted forward.", NULL,
		dsTextJustification_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.1f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}},
		NO_STYLE, NO_STYLE}},
	{"This text is slanted backward.", NULL,
		dsTextJustification_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, -0.3f, 0.0f, 0.0f, 0.1f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}},
		NO_STYLE, NO_STYLE}},
	{"This text has outlines.", NULL,
		dsTextJustification_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.6f, 0.15f, 0.1f, {{255, 0, 0, 255}},
			{{255, 255, 0, 255}}},
		NO_STYLE, NO_STYLE}},
	{"Embolded, slanted, and outlined.", NULL,
		dsTextJustification_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, 10, 24.0f, 0.2f, 0.0f, 0.6f, 0.0f, 0.1f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}},
		{10, 9, 24.0f, 0.0f, 0.3f, 0.6f, 0.0f, 0.1f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}},
		{19, UINT_MAX - 19, 24.0f, 0.0f, 0.0f, 0.6f, 0.15f, 0.1f, {{255, 0, 0, 255}},
			{{255, 255, 0, 255}}}}},
	{"After this line\nhas larger text in the middle.\nAnd another line for good measure.", NULL,
		dsTextJustification_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, 20, 24.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}},
		{20, 6, 36.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.07f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}},
		{26, UINT_MAX - 26, 24.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}}}},
	{"This text mixes wrapping based on max distance\nas well as explicit newlines."
		"\n\nEmpty line.\nTessellated section only has newlines.",
		"\n\n\n", dsTextJustification_Left, 200.0f, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.1f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}},
		NO_STYLE, NO_STYLE}},
	{"Centered text that wraps\nand explicit newlines.", NULL,
		dsTextJustification_Center, 200.0f, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.1f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}},
		NO_STYLE, NO_STYLE}},
	{"Right-justified text that wraps\nand explicit newlines.", NULL,
		dsTextJustification_Right, 200.0f, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.1f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}},
		NO_STYLE, NO_STYLE}},
	{"The text \"\xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9\" is Arabic.\nThe text \"\xE0\xB8\x89\xE0"
		"\xB8\xB1\xE0\xB8\x99\xE0\xB8\x81\xE0\xB8\xB4\xE0\xB8\x99\xE0\xB8\x97\xE0\xB8\xB5\xE0\xB9"
		"\x88\xE0\xB8\x99\xE0\xB8\xB1\xE0\xB9\x88\xE0\xB8\x99\xE0\xB8\x9E\xE0\xB8\xA3\xE0\xB8\xB8"
		"\xE0\xB9\x88\xE0\xB8\x87\xE0\xB8\x99\xE0\xB8\xB5\xE0\xB9\x89\" is Thai.",
		NULL, dsTextJustification_Left, DS_TEXT_NO_WRAP, 1.3f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.1f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}},
		NO_STYLE, NO_STYLE}},
	// Should show as "جزيرة لازورد" and "جزيرة!? لازورد"
	{"Arabic words without punctuation: \"\xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9 "
		"\xD9\x84\xD8\xA7\xD8\xB2\xD9\x88\xD8\xB1\xD8\xAF\"\n"
		"Arabic words with punctuation: \"\xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9!? "
		"\xD9\x84\xD8\xA7\xD8\xB2\xD9\x88\xD8\xB1\xD8\xAF\"\n",
		NULL, dsTextJustification_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.1f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}},
		NO_STYLE, NO_STYLE}},
	{"Arabic words with wrapping: \"\xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9 "
		"\xD9\x84\xD8\xA7\xD8\xB2\xD9\x88\xD8\xB1\xD8\xAF\"\n"
		"Wrapping with punctuation: \"\xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9!? "
		"\xD9\x84\xD8\xA7\xD8\xB2\xD9\x88\xD8\xB1\xD8\xAF\"",
		NULL, dsTextJustification_Left, 350.0f, 1.3f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.1f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}},
		NO_STYLE, NO_STYLE}},
	{"Arabic words explicit newline: \"\xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9\n"
		"\xD9\x84\xD8\xA7\xD8\xB2\xD9\x88\xD8\xB1\xD8\xAF\"\n"
		"Explicit newline with punctuation: \"\xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9!?\n"
		"\xD9\x84\xD8\xA7\xD8\xB2\xD9\x88\xD8\xB1\xD8\xAF\"",
		NULL, dsTextJustification_Left, DS_TEXT_NO_WRAP, 1.3f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.1f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}},
		NO_STYLE, NO_STYLE}},
	{"Wrapping on script transition: \xE0\xB8\x89\xE0\xB8\xB1\xE0\xB8\x99\xE0\xB8\x81\xE0\xB8\xB4"
		"\xE0\xB8\x99\xE0\xB8\x97\xE0\xB8\xB5\xE0\xB9\x88\xE0\xB8\x99\xE0\xB8\xB1\xE0\xB9\x88\xE0"
		"\xB8\x99\xE0\xB9\x80\xE0\xB8\xA1\xE0\xB8\xB7\xE0\xB9\x88\xE0\xB8\xAD\xE0\xB8\xA7\xE0\xB8"
		"\xB2\xE0\xB8\x99",
		NULL, dsTextJustification_Left, 350.0f, 1.3f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.1f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}},
		NO_STYLE, NO_STYLE}},
	{"first is left-to-right \xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9\n"
		"\xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9 first is right-to-left",
		NULL, dsTextJustification_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.1f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}},
		NO_STYLE, NO_STYLE}}
};

typedef dsRenderer* (*CreateRendererFunction)(dsAllocator* allocator);
typedef void (*DestroyRendererFunction)(dsRenderer* renderer);
typedef const char* (*GetShaderDirFunction)(dsRenderer* renderer);

static void glyphPosition(dsVector2f* outPos, const dsVector2f* basePos,
	const dsVector2f* geometryPos, float slant)
{
	dsVector2_add(*outPos, *basePos, *geometryPos);
	outPos->x -= geometryPos->y*slant;
}

static void addTextVertex(void* userData, const dsTextLayout* layout, uint32_t glyphIndex,
	void* vertexData, const dsVertexFormat* format, uint32_t vertexCount)
{
	DS_UNUSED(userData);
	DS_ASSERT(format->elements[dsVertexAttrib_Position].offset ==
		offsetof(StandardVertex, position));
	DS_ASSERT(format->elements[dsVertexAttrib_Color0].offset ==
		offsetof(StandardVertex, textColor));
	DS_ASSERT(format->elements[dsVertexAttrib_Color1].offset ==
		offsetof(StandardVertex, outlineColor));
	DS_ASSERT(format->elements[dsVertexAttrib_TexCoord0].offset ==
		offsetof(StandardVertex, texCoords));
	DS_ASSERT(format->elements[dsVertexAttrib_TexCoord1].offset ==
		offsetof(StandardVertex, style));
	DS_ASSERT(format->size == sizeof(StandardVertex));
	DS_ASSERT(vertexCount == 4);

	const dsTextStyle* style = layout->styles + layout->glyphs[glyphIndex].styleIndex;
	const dsGlyphLayout* glyph = layout->glyphs + glyphIndex;
	dsVector2f position = layout->glyphs[glyphIndex].position;

	dsVector2f geometryPos;
	StandardVertex* vertices = (StandardVertex*)vertexData;
	geometryPos.x = glyph->geometry.min.x;
	geometryPos.y = glyph->geometry.min.y;
	glyphPosition(&vertices[0].position, &position, &geometryPos, style->slant);
	vertices[0].textColor = style->color;
	vertices[0].outlineColor = style->outlineColor;
	vertices[0].texCoords.x = glyph->texCoords.min.x;
	vertices[0].texCoords.y = glyph->texCoords.min.y;
	vertices[0].texCoords.z = (float)glyph->mipLevel;
	vertices[0].style.x = style->embolden;
	vertices[0].style.y = style->outlinePosition;
	vertices[0].style.z = style->outlineThickness;
	vertices[0].style.w = style->antiAlias;

	geometryPos.x = glyph->geometry.min.x;
	geometryPos.y = glyph->geometry.max.y;
	glyphPosition(&vertices[1].position, &position, &geometryPos, style->slant);
	vertices[1].textColor = style->color;
	vertices[1].outlineColor = style->outlineColor;
	vertices[1].texCoords.x = glyph->texCoords.min.x;
	vertices[1].texCoords.y = glyph->texCoords.max.y;
	vertices[1].texCoords.z = (float)glyph->mipLevel;
	vertices[1].style.x = style->embolden;
	vertices[1].style.y = style->outlinePosition;
	vertices[1].style.z = style->outlineThickness;
	vertices[1].style.w = style->antiAlias;

	geometryPos.x = glyph->geometry.max.x;
	geometryPos.y = glyph->geometry.max.y;
	glyphPosition(&vertices[2].position, &position, &geometryPos, style->slant);
	vertices[2].textColor = style->color;
	vertices[2].outlineColor = style->outlineColor;
	vertices[2].texCoords.x = glyph->texCoords.max.x;
	vertices[2].texCoords.y = glyph->texCoords.max.y;
	vertices[2].texCoords.z = (float)glyph->mipLevel;
	vertices[2].style.x = style->embolden;
	vertices[2].style.y = style->outlinePosition;
	vertices[2].style.z = style->outlineThickness;
	vertices[2].style.w = style->antiAlias;

	geometryPos.x = glyph->geometry.max.x;
	geometryPos.y = glyph->geometry.min.y;
	glyphPosition(&vertices[3].position, &position, &geometryPos, style->slant);
	vertices[3].textColor = style->color;
	vertices[3].outlineColor = style->outlineColor;
	vertices[3].texCoords.x = glyph->texCoords.max.x;
	vertices[3].texCoords.y = glyph->texCoords.min.y;
	vertices[3].texCoords.z = (float)glyph->mipLevel;
	vertices[3].style.x = style->embolden;
	vertices[3].style.y = style->outlinePosition;
	vertices[3].style.z = style->outlineThickness;
	vertices[3].style.w = style->antiAlias;
}

static void addTessTextVertex(void* userData, const dsTextLayout* layout, uint32_t glyphIndex,
	void* vertexData, const dsVertexFormat* format, uint32_t vertexCount)
{
	DS_UNUSED(userData);
	DS_ASSERT(format->elements[dsVertexAttrib_Position0].offset ==
		offsetof(TessVertex, position));
	DS_ASSERT(format->elements[dsVertexAttrib_Position1].offset ==
		offsetof(TessVertex, geometry));
	DS_ASSERT(format->elements[dsVertexAttrib_Color0].offset ==
		offsetof(TessVertex, textColor));
	DS_ASSERT(format->elements[dsVertexAttrib_Color1].offset ==
		offsetof(TessVertex, outlineColor));
	DS_ASSERT(format->elements[dsVertexAttrib_TexCoord0].offset ==
		offsetof(TessVertex, texCoords));
	DS_ASSERT(format->elements[dsVertexAttrib_TexCoord1].offset ==
		offsetof(TessVertex, style));
	DS_ASSERT(format->size == sizeof(TessVertex));
	DS_ASSERT(vertexCount == 1);

	TessVertex* vertex = (TessVertex*)vertexData;
	const dsTextStyle* style = layout->styles + layout->glyphs[glyphIndex].styleIndex;
	const dsGlyphLayout* glyph = layout->glyphs + glyphIndex;
	vertex->position.x = layout->glyphs[glyphIndex].position.x;
	vertex->position.y = layout->glyphs[glyphIndex].position.y;
	vertex->position.z = (float)layout->glyphs[glyphIndex].mipLevel;
	vertex->position.w = style->antiAlias;
	vertex->geometry = glyph->geometry;
	vertex->texCoords = glyph->texCoords;
	vertex->textColor = style->color;
	vertex->outlineColor = style->outlineColor;
	vertex->style.x = style->slant;
	vertex->style.y = style->embolden;
	vertex->style.z = style->outlinePosition;
	vertex->style.w = style->outlineThickness;
}

static void printHelp(const char* programPath)
{
	printf("usage: %s [OPTIONS]\n", dsPath_getFileName(programPath));
	printf("options:\n");
	printf("  -h, --help      print this help message and exit\n");
#if DS_HAS_OPENGL
	printf("      --opengl    render using OpenGL\n");
#endif
	printf("default renderer: %s\n", renderTypeNames[defaultRenderType]);
}

static bool validateAllocator(dsAllocator* allocator, const char* name)
{
	if (allocator->size == 0)
		return true;

	DS_LOG_ERROR_F("TestText", "Allocator '%s' has %llu bytes allocated with %u allocations.",
		name, (unsigned long long)allocator->size, allocator->currentAllocations);
	return false;
}

static void setPositions(TestText* testText)
{
	uint32_t width, height;
	DS_VERIFY(dsWindow_getPixelSize(&width, &height, testText->window));
	dsVector2f margin = {{10.0f, 10.0f}};

	if (testText->text)
	{
		dsVector2f position = margin;
		dsVector2_sub(position, position, testText->text->bounds.min);
		dsMaterial_setElementData(testText->material, testText->positionElement, &position,
			dsMaterialType_Vec2, 0, 1);
	}

	if (testText->tessText)
	{
		dsVector2f position = margin;
		position.y += (float)height*0.5f;
		dsVector2_sub(position, position, testText->text->bounds.min);
		dsMaterial_setElementData(testText->tessMaterial, testText->positionElement, &position,
			dsMaterialType_Vec2, 0, 1);
	}
}

static bool createFramebuffer(TestText* testText)
{
	uint32_t width, height;
	if (!dsWindow_getPixelSize(&width, &height, testText->window))
	{
		DS_LOG_ERROR_F("TestText", "Couldn't get window size: %s", dsErrorString(errno));
		return false;
	}

	if (testText->framebuffer)
		dsFramebuffer_destroy(testText->framebuffer);

	dsRenderSurface* surface = testText->window->surface;
	dsFramebufferSurface surfaces[] =
	{
		{dsGfxSurfaceType_ColorRenderSurface, dsCubeFace_None, 0, 0, surface}
	};
	testText->framebuffer = dsFramebuffer_create(testText->renderer->resourceManager,
		testText->allocator, surfaces, 1, width, height, 1);

	if (!testText->framebuffer)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create framebuffer: %s", dsErrorString(errno));
		return false;
	}

	unsigned int screenSize[] = {width, height};
	DS_VERIFY(dsShaderVariableGroup_setElementData(testText->sharedInfoGroup,
		testText->screenSizeElement, &screenSize, dsMaterialType_UVec2, 0, 1));
	DS_VERIFY(dsShaderVariableGroup_commit(testText->sharedInfoGroup,
		testText->renderer->mainCommandBuffer));

	setPositions(testText);
	return true;
}

static void createText(TestText* testText)
{
	uint32_t index = testText->curString;
	DS_ASSERT(index < DS_ARRAY_SIZE(textStrings));

	if (testText->text)
	{
		// Avoid having to store an extra member, but we still need to separately destroy the text.
		dsText_destroy((dsText*)testText->text->text);
		dsTextLayout_destroy(testText->text);
		testText->text = NULL;
	}

	if (testText->tessText)
	{
		// Avoid having to store an extra member, but we still need to separately destroy the text.
		dsText_destroy((dsText*)testText->tessText->text);
		dsTextLayout_destroy(testText->tessText);
		testText->tessText = NULL;
	}

	dsText* text = dsText_createUTF8(testText->font, testText->allocator,
		textStrings[index].standardText, false);
	if (!text)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create text: %s", dsErrorString(errno));
		return;
	}
	testText->text = dsTextLayout_create(testText->allocator, text, textStrings[index].styles,
		(uint32_t)DS_ARRAY_SIZE(textStrings[index].styles));
	if (!testText->text)
	{
		dsText_destroy(text);
		DS_LOG_ERROR_F("TestText", "Couldn't create text: %s", dsErrorString(errno));
		return;
	}
	if (!dsTextLayout_layout(testText->text, testText->renderer->mainCommandBuffer,
		textStrings[index].justification, textStrings[index].maxWidth,
		textStrings[index].lineScale))
	{
		DS_LOG_ERROR_F("TestText", "Couldn't layout text: %s", dsErrorString(errno));
		return;
	}

	DS_VERIFY(dsTextRenderBuffer_clear(testText->textRender));
	if (!dsTextRenderBuffer_addText(testText->textRender, testText->text, 0,
		testText->text->text->glyphCount))
	{
		DS_LOG_ERROR_F("TestText", "Couldn't add text: %s", dsErrorString(errno));
		return;
	}
	DS_VERIFY(dsTextRenderBuffer_commit(testText->textRender,
		testText->renderer->mainCommandBuffer));

	const char* tessString = textStrings[index].tesselatedText;
	if (!tessString)
		tessString = textStrings[index].standardText;
	DS_ASSERT(tessString);
	if (testText->tessMaterial && *tessString)
	{
		text = dsText_createUTF8(testText->font, testText->allocator, tessString, false);
		if (!text)
		{
			DS_LOG_ERROR_F("TestText", "Couldn't create text: %s", dsErrorString(errno));
			return;
		}
		testText->tessText = dsTextLayout_create(testText->allocator, text,
			textStrings[index].styles, (uint32_t)DS_ARRAY_SIZE(textStrings[index].styles));
		if (!testText->text)
		{
			dsText_destroy(text);
			DS_LOG_ERROR_F("TestText", "Couldn't create text: %s", dsErrorString(errno));
			return;
		}
		if (!dsTextLayout_layout(testText->tessText, testText->renderer->mainCommandBuffer,
			textStrings[index].justification, textStrings[index].maxWidth,
			textStrings[index].lineScale))
		{
			DS_LOG_ERROR_F("TestText", "Couldn't layout text: %s", dsErrorString(errno));
			return;
		}

		DS_VERIFY(dsTextRenderBuffer_clear(testText->tessTextRender));
		if (!dsTextRenderBuffer_addText(testText->tessTextRender, testText->tessText, 0,
			testText->tessText->text->glyphCount))
		{
			DS_LOG_ERROR_F("TestText", "Couldn't add text: %s", dsErrorString(errno));
			return;
		}
		DS_VERIFY(dsTextRenderBuffer_commit(testText->tessTextRender,
			testText->renderer->mainCommandBuffer));
	}

	setPositions(testText);
}

static void nextText(TestText* testText)
{
	++testText->curString;
	if (testText->curString >= DS_ARRAY_SIZE(textStrings))
		testText->curString = 0;
	createText(testText);
}

static void prevText(TestText* testText)
{
	if (testText->curString == 0)
		testText->curString = DS_ARRAY_SIZE(textStrings) - 1;
	else
		--testText->curString;
	createText(testText);
}

static bool processEvent(dsApplication* application, dsWindow* window, const dsEvent* event,
	void* userData)
{
	DS_UNUSED(application);

	TestText* testText = (TestText*)userData;
	DS_ASSERT(!window || window == testText->window);
	switch (event->type)
	{
		case dsEventType_WindowClosed:
			DS_VERIFY(dsWindow_destroy(window));
			testText->window = NULL;
			return false;
		case dsEventType_WindowResized:
			if (!createFramebuffer(testText))
				abort();
			return true;
		case dsEventType_KeyDown:
			switch (event->key.key)
			{
				case dsKeyCode_Right:
					nextText(testText);
					return false;
				case dsKeyCode_Left:
					prevText(testText);
					return false;
				default:
					return true;
			}
		case dsEventType_TouchFingerDown:
			nextText(testText);
			return true;
		default:
			return true;
	}
}

static void draw(dsApplication* application, dsWindow* window, void* userData)
{
	DS_UNUSED(application);
	DS_UNUSED(window);
	TestText* testText = (TestText*)userData;
	DS_ASSERT(testText->window == window);
	dsRenderer* renderer = testText->renderer;
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsSurfaceClearValue clearValue;
	clearValue.colorValue.floatValue.r = 0.0f;
	clearValue.colorValue.floatValue.g = 0.1f;
	clearValue.colorValue.floatValue.b = 0.2f;
	clearValue.colorValue.floatValue.a = 1.0f;
	DS_VERIFY(dsRenderPass_begin(testText->renderPass, commandBuffer, testText->framebuffer, NULL,
		&clearValue, 1, false));

	if (testText->text)
	{
		DS_VERIFY(dsShader_bind(testText->shader, commandBuffer, testText->material, NULL, NULL));
		DS_VERIFY(dsTextRenderBuffer_draw(testText->textRender, commandBuffer));
		DS_VERIFY(dsShader_unbind(testText->shader, commandBuffer));
	}

	if (testText->tessText)
	{
		DS_VERIFY(dsShader_bind(testText->tessShader, commandBuffer, testText->tessMaterial, NULL,
			NULL));
		DS_VERIFY(dsTextRenderBuffer_draw(testText->tessTextRender, commandBuffer));
		DS_VERIFY(dsShader_unbind(testText->tessShader, commandBuffer));
	}

	DS_VERIFY(dsRenderPass_end(testText->renderPass, commandBuffer));
}

static bool setup(TestText* testText, dsApplication* application, dsAllocator* allocator)
{
	dsRenderer* renderer = application->renderer;
	dsResourceManager* resourceManager = renderer->resourceManager;
	testText->allocator = allocator;
	testText->renderer = renderer;

	dsEventResponder responder = {&processEvent, testText, 0, 0};
	DS_VERIFY(dsApplication_addEventResponder(application, &responder));

	testText->window = dsWindow_create(application, allocator, "Test Text",
		NULL, 800, 600, dsWindowFlags_Resizeable);
	if (!testText->window)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create window: %s", dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsWindow_setDrawFunction(testText->window, &draw, testText));

	dsAttachmentInfo attachment = {dsAttachmentUsage_Clear, renderer->surfaceColorFormat,
		DS_DEFAULT_ANTIALIAS_SAMPLES};

	dsColorAttachmentRef colorAttachment = {0, false};
	uint32_t depthStencilAttachment = DS_NO_ATTACHMENT;
	dsRenderSubpassInfo subpass =
	{
		"TestText", NULL, &colorAttachment, 0, 1, depthStencilAttachment
	};
	testText->renderPass = dsRenderPass_create(renderer, allocator, &attachment, 1, &subpass, 1,
		NULL, 0);
	if (!testText->renderPass)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create render pass: %s", dsErrorString(errno));
		return false;
	}

	DS_ASSERT(shaderDir);
	char path[DS_PATH_MAX];
	if (!dsPath_combine(path, sizeof(path), assetsDir, shaderDir) ||
		!dsPath_combine(path, sizeof(path), path, "TestText.mslb"))
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create shader path: %s", dsErrorString(errno));
		return false;
	}

	testText->shaderModule = dsShaderModule_loadFile(resourceManager, allocator, path, "TestText");
	if (!testText->shaderModule)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't load shader: %s", dsErrorString(errno));
		return false;
	}

	dsShaderVariableElement sharedInfoElems[] =
	{
		{"screenSize", dsMaterialType_UVec2, 0}
	};
	testText->sharedInfoDesc = dsShaderVariableGroupDesc_create(resourceManager, allocator,
		sharedInfoElems, (uint32_t)DS_ARRAY_SIZE(sharedInfoElems));
	if (!testText->sharedInfoDesc)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create shader variable group description: %s",
			dsErrorString(errno));
		return false;
	}

	testText->screenSizeElement = dsShaderVariableGroupDesc_findElement(testText->sharedInfoDesc,
		"screenSize");
	DS_ASSERT(testText->screenSizeElement != DS_MATERIAL_UNKNOWN);

	testText->sharedInfoGroup = dsShaderVariableGroup_create(resourceManager, allocator, allocator,
		testText->sharedInfoDesc);
	if (!testText->sharedInfoGroup)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create shader variable group: %s",
			dsErrorString(errno));
		return false;
	}

	dsMaterialElement materialElems[] =
	{
		{"SharedInfo", dsMaterialType_VariableGroup, 0, testText->sharedInfoDesc, false, 0},
		{"position", dsMaterialType_Vec2, 0, NULL, false, 0},
		{"fontTex", dsMaterialType_Texture, 0, NULL, false, 0}
	};
	testText->materialDesc = dsMaterialDesc_create(resourceManager, allocator, materialElems,
		(uint32_t)DS_ARRAY_SIZE(materialElems));
	if (!testText->materialDesc)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create material description: %s",
			dsErrorString(errno));
		return false;
	}

	uint32_t sharedInfoElement = dsMaterialDesc_findElement(testText->materialDesc, "SharedInfo");
	DS_ASSERT(sharedInfoElement != DS_MATERIAL_UNKNOWN);
	testText->positionElement = dsMaterialDesc_findElement(testText->materialDesc, "position");
	DS_ASSERT(testText->positionElement != DS_MATERIAL_UNKNOWN);

	testText->material = dsMaterial_create(allocator, testText->materialDesc);
	if (!testText->material)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create material: %s", dsErrorString(errno));
		return false;
	}
	DS_VERIFY(dsMaterial_setVariableGroup(testText->material, sharedInfoElement,
		testText->sharedInfoGroup));

	testText->shader = dsShader_createName(resourceManager, allocator, testText->shaderModule,
		"Font", testText->materialDesc, dsPrimitiveType_TriangleList,
		DS_DEFAULT_ANTIALIAS_SAMPLES);
	if (!testText->shader)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create shader: %s", dsErrorString(errno));
		return false;
	}

	dsVertexFormat vertexFormat;
	DS_VERIFY(dsVertexFormat_initialize(&vertexFormat));
	vertexFormat.elements[dsVertexAttrib_Position].format = dsGfxFormat_decorate(
		dsGfxFormat_X32Y32, dsGfxFormat_Float);
	vertexFormat.elements[dsVertexAttrib_Color0].format = dsGfxFormat_decorate(
		dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	vertexFormat.elements[dsVertexAttrib_Color1].format = dsGfxFormat_decorate(
		dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	vertexFormat.elements[dsVertexAttrib_TexCoord0].format = dsGfxFormat_decorate(
		dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
	vertexFormat.elements[dsVertexAttrib_TexCoord1].format = dsGfxFormat_decorate(
		dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Position, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Color0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Color1, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_TexCoord0, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_TexCoord1, true));
	DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(&vertexFormat));
	testText->textRender = dsTextRenderBuffer_create(allocator, resourceManager, 1024,
		&vertexFormat, false, &addTextVertex, NULL);
	if (!testText->textRender)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create text render: %s", dsErrorString(errno));
		return false;
	}

	if (renderer->hasTessellationShaders)
	{
		testText->tessMaterial = dsMaterial_create(allocator, testText->materialDesc);
		if (!testText->tessMaterial)
		{
			DS_LOG_ERROR_F("TestText", "Couldn't create material: %s", dsErrorString(errno));
			return false;
		}
		DS_VERIFY(dsMaterial_setVariableGroup(testText->tessMaterial, sharedInfoElement,
			testText->sharedInfoGroup));

		testText->tessShader = dsShader_createName(resourceManager, allocator,
			testText->shaderModule, "FontTess", testText->materialDesc, dsPrimitiveType_PatchList,
			DS_DEFAULT_ANTIALIAS_SAMPLES);
		if (!testText->tessShader)
		{
			DS_LOG_ERROR_F("TestText", "Couldn't create shader: %s", dsErrorString(errno));
			return false;
		}

		DS_VERIFY(dsVertexFormat_initialize(&vertexFormat));
		vertexFormat.elements[dsVertexAttrib_Position0].format = dsGfxFormat_decorate(
			dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);
		vertexFormat.elements[dsVertexAttrib_Position1].format = dsGfxFormat_decorate(
			dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);
		vertexFormat.elements[dsVertexAttrib_Color0].format = dsGfxFormat_decorate(
			dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
		vertexFormat.elements[dsVertexAttrib_Color1].format = dsGfxFormat_decorate(
			dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
		vertexFormat.elements[dsVertexAttrib_TexCoord0].format = dsGfxFormat_decorate(
			dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);
		vertexFormat.elements[dsVertexAttrib_TexCoord1].format = dsGfxFormat_decorate(
			dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float);
		DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Position0, true));
		DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Position1, true));
		DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Color0, true));
		DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Color1, true));
		DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_TexCoord0, true));
		DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_TexCoord1, true));
		DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(&vertexFormat));
		testText->tessTextRender = dsTextRenderBuffer_create(allocator, resourceManager, 1024,
			&vertexFormat, true, &addTessTextVertex, NULL);
		if (!testText->tessTextRender)
		{
			DS_LOG_ERROR_F("TestText", "Couldn't create text render: %s", dsErrorString(errno));
			return false;
		}
	}

	testText->faceGroup = dsFaceGroup_create(allocator, NULL, DS_DEFAULT_MAX_FACES,
		dsTextQuality_High);
	if (!testText->faceGroup)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create face group: %s", dsErrorString(errno));
		return false;
	}

	if (!dsPath_combine(path, sizeof(path), assetsDir, "Fonts") ||
		!dsPath_combine(path, sizeof(path), path, "NotoSans-Regular.ttc") ||
		!dsFaceGroup_loadFaceFile(testText->faceGroup, path, "Latin"))
	{
		DS_LOG_ERROR_F("TestText", "Couldn't load font face: %s", dsErrorString(errno));
		return false;
	}

	if (!dsPath_combine(path, sizeof(path), assetsDir, "Fonts") ||
		!dsPath_combine(path, sizeof(path), path, "NotoSansArabic-Regular.ttf") ||
		!dsFaceGroup_loadFaceFile(testText->faceGroup, path, "Arabic"))
	{
		DS_LOG_ERROR_F("TestText", "Couldn't load font face: %s", dsErrorString(errno));
		return false;
	}

	if (!dsPath_combine(path, sizeof(path), assetsDir, "Fonts") ||
		!dsPath_combine(path, sizeof(path), path, "NotoSansThai-Regular.ttf") ||
		!dsFaceGroup_loadFaceFile(testText->faceGroup, path, "Thai"))
	{
		DS_LOG_ERROR_F("TestText", "Couldn't load font face: %s", dsErrorString(errno));
		return false;
	}

	const char* faceNames[] = {"Latin", "Arabic", "Thai"};
	testText->font = dsFont_create(testText->faceGroup, resourceManager, allocator, faceNames,
		(uint32_t)DS_ARRAY_SIZE(faceNames));
	if (!testText->font)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create font: %s", dsErrorString(errno));
		return false;
	}

	uint32_t textureElement = dsMaterialDesc_findElement(testText->materialDesc, "fontTex");
	DS_ASSERT(textureElement != DS_MATERIAL_UNKNOWN);
	dsMaterial_setTexture(testText->material, textureElement, dsFont_getTexture(testText->font));
	if (testText->tessMaterial)
	{
		dsMaterial_setTexture(testText->tessMaterial, textureElement,
			dsFont_getTexture(testText->font));
	}

	if (!createFramebuffer(testText))
		return false;

	testText->curString = 0;
	createText(testText);
	return true;
}

static void shutdown(TestText* testText)
{
	if (testText->tessTextRender)
		dsTextRenderBuffer_destroy(testText->tessTextRender);
	if (testText->tessText)
	{
		// Avoid having to store an extra member, but we still need to separately destroy the text.
		dsText_destroy((dsText*)testText->tessText->text);
		dsTextLayout_destroy(testText->tessText);
	}
	if (testText->textRender)
		dsTextRenderBuffer_destroy(testText->textRender);
	if (testText->text)
	{
		// Avoid having to store an extra member, but we still need to separately destroy the text.
		dsText_destroy((dsText*)testText->text->text);
		dsTextLayout_destroy(testText->text);
	}
	if (testText->font)
		dsFont_destroy(testText->font);
	if (testText->faceGroup)
		dsFaceGroup_destroy(testText->faceGroup);
	if (testText->tessShader)
		dsShader_destroy(testText->tessShader);
	if (testText->shader)
		dsShader_destroy(testText->shader);
	if (testText->tessMaterial)
		dsMaterial_destroy(testText->tessMaterial);
	if (testText->material)
		dsMaterial_destroy(testText->material);
	if (testText->materialDesc)
		dsMaterialDesc_destroy(testText->materialDesc);
	if (testText->sharedInfoGroup)
		dsShaderVariableGroup_destroy(testText->sharedInfoGroup);
	if (testText->sharedInfoDesc)
		dsShaderVariableGroupDesc_destroy(testText->sharedInfoDesc);
	if (testText->shaderModule)
		dsShaderModule_destroy(testText->shaderModule);
	if (testText->renderPass)
		dsRenderPass_destroy(testText->renderPass);
	if (testText->framebuffer)
		dsFramebuffer_destroy(testText->framebuffer);
	if (testText->window)
		dsWindow_destroy(testText->window);
}

int dsMain(int argc, const char** argv)
{
	dsRenderType renderType = defaultRenderType;
	for (int i = 1; i < argc; ++argv)
	{
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			printHelp(argv[0]);
			return 0;
		}
#if DS_HAS_OPENGL
		else if (strcmp(argv[i], "--opengl") == 0)
			renderType = dsRenderType_OpenGL;
#endif
		else
		{
			printf("Unknown option: %s\n", argv[i]);
			printHelp(argv[0]);
			return 1;
		}
	}

	DS_VERIFY(dsPath_getDirectoryName(assetsDir, sizeof(assetsDir), argv[0]));
	DS_VERIFY(dsPath_combine(assetsDir, sizeof(assetsDir), assetsDir, "TestText-assets"));

	DS_LOG_INFO_F("TestText", "Render using %s", renderTypeNames[renderType]);

	CreateRendererFunction createRendererFunc = NULL;
	DestroyRendererFunction destroyRendererFunc = NULL;
	GetShaderDirFunction getShaderDirFunc = NULL;
	switch (renderType)
	{
#if DS_HAS_OPENGL
		case dsRenderType_OpenGL:
			createRendererFunc = &dsTestText_createGLRenderer;
			destroyRendererFunc = &dsTestText_destroyGLRenderer;
			getShaderDirFunc = &dsTestText_getGLShaderDir;
			break;
#endif
		default:
			DS_ASSERT(false);
			break;
	}

	dsSystemAllocator renderAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&renderAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator applicationAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&applicationAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator testTextAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&testTextAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsRenderer* renderer = createRendererFunc((dsAllocator*)&renderAllocator);
	if (!renderer)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create renderer: %s", dsErrorString(errno));
		return 2;
	}
	dsRenderer_setVsync(renderer, true);
	dsRenderer_setDefaultAnisotropy(renderer, renderer->maxAnisotropy);

	shaderDir = getShaderDirFunc(renderer);

	dsApplication* application = dsSDLApplication_create((dsAllocator*)&applicationAllocator,
		renderer);
	if (!application)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create application: %s", dsErrorString(errno));
		destroyRendererFunc(renderer);
		return 2;
	}

	TestText testText;
	memset(&testText, 0, sizeof(testText));
	if (!setup(&testText, application, (dsAllocator*)&testTextAllocator))
	{
		shutdown(&testText);
		return 3;
	}

	int exitCode = dsApplication_run(application);

	shutdown(&testText);
	dsSDLApplication_destroy(application);
	destroyRendererFunc(renderer);

	if (!validateAllocator((dsAllocator*)&renderAllocator, "render"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&applicationAllocator, "application"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&testTextAllocator, "TestText"))
		exitCode = 4;

	return exitCode;
}

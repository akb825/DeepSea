/*
 * Copyright 2017-2019 Aaron Barany
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

#include <DeepSea/Application/Application.h>
#include <DeepSea/Application/Window.h>
#include <DeepSea/ApplicationSDL/SDLApplication.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Core/Timer.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Render/Resources/DrawGeometry.h>
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
#include <DeepSea/Render/CommandBuffer.h>
#include <DeepSea/Render/CommandBufferPool.h>
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Render/RenderPass.h>
#include <DeepSea/RenderBootstrap/RenderBootstrap.h>
#include <DeepSea/Text/FaceGroup.h>
#include <DeepSea/Text/Font.h>
#include <DeepSea/Text/Text.h>
#include <DeepSea/Text/TextLayout.h>
#include <DeepSea/Text/TextRenderBuffer.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if DS_HAS_EASY_PROFILER
#include <DeepSea/EasyProfiler/EasyProfiler.h>
#endif

// Set to a valid font path to test Chinese text
//#define CHINESE_FONT_PATH "/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc"

typedef struct TestText
{
	dsAllocator* allocator;
	dsRenderer* renderer;
	dsCommandBufferPool* setupCommands;
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
	dsShader* limitShader;
	dsFaceGroup* faceGroup;
	dsFont* font;
	dsTextLayout* text;
	dsTextRenderBuffer* textRender;
	dsTextLayout* tessText;
	dsTextRenderBuffer* tessTextRender;
	dsGfxBuffer* limitBuffer;
	dsDrawGeometry* limitGeometry;

	uint32_t screenSizeElement;
	uint32_t positionElement;
	uint32_t limitBoundsElement;
	uint32_t curString;
	uint32_t fingerCount;
	uint32_t maxFingers;
} TestText;

static const char* assetsDir = "TestText-assets";
static char shaderDir[100];

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
	bool uniform;
	dsTextAlign alignment;
	float maxWidth;
	float lineScale;
	dsTextStyle styles[3];
} TextInfo;

#define NO_STYLE {UINT_MAX, 0, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, {{0, 0, 0, 0}}, {{0, 0, 0, 0}}, \
	0.0f}

// NOTE: Uses explicit bytes for UTF-8 for compilers that don't support Unicode string constants.
// (e.g. Visual Studio) Helpful site to encode and decode: https://mothereff.in/utf-8
static TextInfo textStrings[] =
{
	{"Top text is standard quads.\nUse arrow keys or touch to cycle text.",
		"Bottom text, if visible, is tessellated.", false,
		dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
	{"All ASCII characters:\n"
		"!\"#$%&'()*+,-./0123456789:;<=>?@\n"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ\n"
		"[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", NULL, false,
		dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.2f,
		{{0, UINT_MAX, 42.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
	{"This text has been emboldened.", NULL, false,
		dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.15f, 0.0f, 0.0f, 0.0f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
	{"This text is slanted forward.", NULL, false,
		dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
	{"This text is slanted backward.", NULL, false,
		dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, -0.3f, 0.0f, 0.0f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
	{"This text has outlines.", NULL, false,
		dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.55f, 0.1f, 0.0f, {{255, 0, 0, 255}},
			{{255, 255, 0, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
	{"Embolded, slanted, and outlined.", NULL, false,
		dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, 10, 24.0f, 0.15f, 0.0f, 0.6f, 0.0f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		{10, 9, 24.0f, 0.0f, 0.3f, 0.6f, 0.0f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		{19, UINT_MAX - 19, 24.0f, 0.0f, 0.0f, 0.55f, 0.1f, 0.0f, {{255, 0, 0, 255}},
			{{255, 255, 0, 255}}, 0.0f}}},
	{"Tiny text.\nSmall text.\nHuge text.", NULL, false,
		dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, 11, 9.0f, 0.0f, 0.0f, 0.6f, 0.0f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		{11, 12, 16.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		{23, UINT_MAX - 23, 128.0f, 0.0f, 0.0f, 0.0, 0.0f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f}}},
	{"Tiny text.\nSmall text.\nHuge text.", NULL, false,
		dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, 11, 9.0f, 0.0f, 0.0f, 0.55f, 0.1f, 0.0f, {{255, 0, 0, 255}},
			{{255, 255, 0, 255}}, 0.0f},
		{11, 12, 16.0f, 0.0f, 0.0f, 0.55f, 0.1f, 0.0f, {{255, 0, 0, 255}},
			{{255, 255, 0, 255}}, 0.0f},
		{23, UINT_MAX - 23, 128.0f, 0.0f, 0.0f, 0.55f, 0.1f, 0.0f, {{255, 0, 0, 255}},
			{{255, 255, 0, 255}}, 0.0f}}},
	{"After this line\nhas larger text in the middle.\nAnd another line for good measure.", NULL,
		false, dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, 20, 24.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		{20, 6, 36.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		{26, UINT_MAX - 26, 24.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f}}},
	{"  This text mixes               wrapping based on max distance as well as explicit newlines."
		"\n\n  Empty line.\n\nThiswordislongerthanlimit. This isn't.\n\nTessellated section only "
			"has newlines.",
		"\n\n\n", false,
		dsTextAlign_Left, 200.0f, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
	{"All words are too long.", NULL, false,
		dsTextAlign_Left, 10.0f, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
	{"Centered text                that wraps\nand explicit newlines.", NULL, false,
		dsTextAlign_Center, 162.0f, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
	{"Right-justified              text that wraps\nand explicit newlines.", NULL, false,
		dsTextAlign_Right, 180.0f, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
	{"The text \"\xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9\" is Arabic.\nThe text \"\xE0\xB8\x89\xE0"
		"\xB8\xB1\xE0\xB8\x99\xE0\xB8\x81\xE0\xB8\xB4\xE0\xB8\x99\xE0\xB8\x97\xE0\xB8\xB5\xE0\xB9"
		"\x88\xE0\xB8\x99\xE0\xB8\xB1\xE0\xB9\x88\xE0\xB8\x99\xE0\xB8\x9E\xE0\xB8\xA3\xE0\xB8\xB8"
		"\xE0\xB9\x88\xE0\xB8\x87\xE0\xB8\x99\xE0\xB8\xB5\xE0\xB9\x89\" is Thai.", NULL, false,
		dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.3f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
	// Should show as "جزيرة لازورد" and "جزيرة!? لازورد"
	{"Arabic words without punctuation: \"\xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9 "
		"\xD9\x84\xD8\xA7\xD8\xB2\xD9\x88\xD8\xB1\xD8\xAF\"\n"
		"Arabic words with punctuation: \"\xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9!? "
		"\xD9\x84\xD8\xA7\xD8\xB2\xD9\x88\xD8\xB1\xD8\xAF\"\n", NULL, false,
		dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.3f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
	{"Arabic words with wrapping: \"\xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9                    "
		"\xD9\x84\xD8\xA7\xD8\xB2\xD9\x88\xD8\xB1\xD8\xAF\"\n"
		"Wrapping with punctuation: \"\xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9!?                "
		"\xD9\x84\xD8\xA7\xD8\xB2\xD9\x88\xD8\xB1\xD8\xAF\"", NULL, false,
		dsTextAlign_Left, 410.0f, 1.3f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
	// "!?" will show on the end of the line that contains English and Arabic.
	{"Arabic words explicit newline: \"\xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9\n"
		"\xD9\x84\xD8\xA7\xD8\xB2\xD9\x88\xD8\xB1\xD8\xAF\"\n"
		"Explicit newline with punctuation: \"\xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9!?\n"
		"\xD9\x84\xD8\xA7\xD8\xB2\xD9\x88\xD8\xB1\xD8\xAF\"\n"
		"Explicit newline with punctuation without any English:\n"
		"\"\xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9!?\n"
		"\xD9\x84\xD8\xA7\xD8\xB2\xD9\x88\xD8\xB1\xD8\xAF\"", NULL, false,
		dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.3f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
	{"Wrapping on script transition: \xE0\xB8\x89\xE0\xB8\xB1\xE0\xB8\x99\xE0\xB8\x81\xE0\xB8\xB4"
		"\xE0\xB8\x99\xE0\xB8\x97\xE0\xB8\xB5\xE0\xB9\x88\xE0\xB8\x99\xE0\xB8\xB1\xE0\xB9\x88\xE0"
		"\xB8\x99\xE0\xB9\x80\xE0\xB8\xA1\xE0\xB8\xB7\xE0\xB9\x88\xE0\xB8\xAD\xE0\xB8\xA7\xE0\xB8"
		"\xB2\xE0\xB8\x99", NULL, false,
		dsTextAlign_Left, 350.0f, 1.3f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
	{"first is left-to-right \xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9\n"
		"\xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9 first is right-to-left", NULL, false,
		dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.3f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
	{"Without direction mark:\n\xD9\x82\xD8\xB1\xD8\xA3\x20\x57\x69\x6B\x69\x70\x65\x64\x69\x61"
		"\xE2\x84\xA2\x20\xD8\xB7\xD9\x88\xD8\xA7\xD9\x84\x20\xD8\xA7\xD9\x84\xD9\x8A\xD9\x88\xD9"
		"\x85\x2E\n"
		"With direction mark:\n\xD9\x82\xD8\xB1\xD8\xA3\x20\x57\x69\x6B\x69\x70\x65\x64\x69\x61"
		"\xE2\x84\xA2\xE2\x80\x8E\x20\xD8\xB7\xD9\x88\xD8\xA7\xD9\x84\x20\xD8\xA7\xD9\x84\xD9\x8A"
		"\xD9\x88\xD9\x85\x2E", NULL, false,
		dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.3f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
	{"Inherited script transitions: \xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9\x65\xE2\x80\x8E\xCC"
		"\x88\xD9\x84\xD8\xA7\xD8\xB2\xD9\x88\xD8\xB1\xD8\xAF", NULL, false,
		dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.3f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
	{"Text with a negative offset in Y.", NULL, false,
		dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, 12, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		{12, 8, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, -20.0f},
		{20, UINT_MAX - 20, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f}}},
	{"Text with a positive offset in Y.", NULL, false,
		dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.0f,
		{{0, 12, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		{12, 8, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 20.0f},
		{20, UINT_MAX - 20, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f}}},
	{"  Uniform script                with a mixture of wrapping\n\n  and\nexplicit\nnewlines.\n\n"
		"Next uses Arabic only", NULL, true,
		dsTextAlign_Start, 200, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f}, NO_STYLE, NO_STYLE}},
	{"  ! \xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9 "
		"\xD9\x84\xD8\xA7\xD8\xB2\xD9\x88\xD8\xB1\xD8\xAF\n"
		"\xD8\xAC\xD8\xB2\xD9\x8A\xD8\xB1\xD8\xA9 "
		"\xD9\x84\xD8\xA7\xD8\xB2\xD9\x88\xD8\xB1\xD8\xAF!\n\n"
		"\xD9\x83\xD9\x86\xD8\xAA\x20\xD8\xA3\xD8\xB1\xD9\x8A\xD8\xAF\x20\xD8\xA3\xD9\x86\x20\xD8"
		"\xA3\xD9\x82\xD8\xB1\xD8\xA3\x20                       "
		"\xD9\x83\xD8\xAA\xD8\xA7\xD8\xA8\xD8\xA7\x20\xD8\xB9\xD9\x86\x20\xD8\xAA\xD8\xA7\xD8"
		"\xB1\xD9\x8A\xD8\xAE\x20\xD8\xA7\xD9\x84\xD9\x85\xD8\xB1\xD8\xA3\xD8\xA9\x20\xD9\x81\xD9"
		"\x8A\x20\xD9\x81\xD8\xB1\xD9\x86\xD8\xB3\xD8\xA7\xE2\x80\xAC",
		NULL, true,
		dsTextAlign_Start, 200, 1.0f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.15f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f}, NO_STYLE, NO_STYLE}},
#ifdef CHINESE_FONT_PATH
	{"Chinese text: \xE5\x9C\xB0\xE7\x82\xB9\xE6\x96\xB9\xE8\xA8\x80 "
		"\xE5\x9C\xB0\xE9\xBB\x9E\xE6\x96\xB9\xE8\xA8\x80 "
		"\xE8\xAA\x8D\xE8\xAD\x98 \xE6\x8B\x96\xE6\x8B\x89\xE6\xA9\x9F", NULL, false,
		dsTextAlign_Left, DS_TEXT_NO_WRAP, 1.2f,
		{{0, UINT_MAX, 24.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, {{255, 255, 255, 255}},
			{{255, 255, 255, 255}}, 0.0f},
		NO_STYLE, NO_STYLE}},
#endif
};

static bool textInitialized;

static void glyphPosition(dsVector2f* outPos, const dsVector2f* basePos,
	const dsVector2f* geometryPos, float slant)
{
	dsVector2_add(*outPos, *basePos, *geometryPos);
	outPos->x -= geometryPos->y*slant;
}

static void addTextVertex(void* userData, const dsTextLayout* layout, void* layoutUserData,
	uint32_t glyphIndex, void* vertexData, const dsVertexFormat* format, uint32_t vertexCount)
{
	DS_UNUSED(userData);
	DS_UNUSED(layoutUserData);
	DS_UNUSED(format);
	DS_UNUSED(vertexCount);
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

static void addTessTextVertex(void* userData, const dsTextLayout* layout, void* layoutUserData,
	uint32_t glyphIndex, void* vertexData, const dsVertexFormat* format, uint32_t vertexCount)
{
	DS_UNUSED(userData);
	DS_UNUSED(layoutUserData);
	DS_UNUSED(format);
	DS_UNUSED(vertexCount);
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
	printf("Use left/right arrows or tap on touchscreen to cyle text.\n\n");
	printf("options:\n");
	printf("  -h, --help                   print this help message and exit\n");
	printf("  -f, --font <path>            path to a custom font file for Latin glyphs\n");
	printf("  -h, --help                   print this help message and exit\n");
	printf("  -l, --low                    use low quality text\n");
	printf("  -m, --medium                 use medium quality text (default)\n");
	printf("  -H, --high                   use high quality text\n");
	printf("  -v, --very-high              use very high quality text\n");
	printf("  -r, --renderer <renderer>    explicitly use a renderer; options are:\n");
	for (int i = 0; i < dsRendererType_Default; ++i)
	{
		printf("                                 %s\n",
			dsRenderBootstrap_rendererName((dsRendererType)i));
	}
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

	float wrapWidth = textStrings[testText->curString].maxWidth;
	if (wrapWidth != DS_TEXT_NO_WRAP)
	{
		dsAlignedBox2f bounds = {{{margin.x + wrapWidth, 0.0f}},
			{{margin.x + wrapWidth + 2.0f, (float)height}}};
		DS_VERIFY(dsMaterial_setElementData(testText->material, testText->limitBoundsElement,
			&bounds, dsMaterialType_Vec4, 0, 1));
	}
}

static bool createFramebuffer(TestText* testText, dsCommandBuffer* commandBuffer)
{
	uint32_t width = testText->window->surface->width;
	uint32_t height = testText->window->surface->height;

	dsFramebuffer_destroy(testText->framebuffer);

	dsRenderSurface* surface = testText->window->surface;
	dsFramebufferSurface surfaces[] =
	{
		{dsGfxSurfaceType_ColorRenderSurface, dsCubeFace_None, 0, 0, surface}
	};
	testText->framebuffer = dsFramebuffer_create(testText->renderer->resourceManager,
		testText->allocator, "Main", surfaces, 1, width, height, 1);

	if (!testText->framebuffer)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create framebuffer: %s", dsErrorString(errno));
		return false;
	}

	unsigned int screenSize[] = {width, height};
	DS_VERIFY(dsShaderVariableGroup_setElementData(testText->sharedInfoGroup,
		testText->screenSizeElement, &screenSize, dsMaterialType_IVec2, 0, 1));
	DS_VERIFY(dsShaderVariableGroup_commit(testText->sharedInfoGroup, commandBuffer));

	setPositions(testText);
	return true;
}

static void createText(TestText* testText, dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	uint32_t index = testText->curString;
	DS_ASSERT(index < DS_ARRAY_SIZE(textStrings));

	dsTextLayout_destroyLayoutAndText(testText->text);
	testText->text = NULL;
	dsTextLayout_destroyLayoutAndText(testText->tessText);
	testText->tessText = NULL;

	dsText* text = dsText_createUTF8(testText->font, testText->allocator,
		textStrings[index].standardText, textStrings[index].uniform);
	if (!text)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create text: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN_VOID();
	}
	testText->text = dsTextLayout_create(testText->allocator, text, textStrings[index].styles,
		DS_ARRAY_SIZE(textStrings[index].styles));
	if (!testText->text)
	{
		dsText_destroy(text);
		DS_LOG_ERROR_F("TestText", "Couldn't create text: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN_VOID();
	}
	if (!dsTextLayout_layout(testText->text, commandBuffer, textStrings[index].alignment,
		textStrings[index].maxWidth, textStrings[index].lineScale))
	{
		DS_LOG_ERROR_F("TestText", "Couldn't layout text: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN_VOID();
	}

	DS_VERIFY(dsTextRenderBuffer_clear(testText->textRender));
	if (!dsTextRenderBuffer_addText(testText->textRender, testText->text, NULL))
	{
		DS_LOG_ERROR_F("TestText", "Couldn't add text: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN_VOID();
	}
	DS_VERIFY(dsTextRenderBuffer_commit(testText->textRender, commandBuffer));

	const char* tessString = textStrings[index].tesselatedText;
	if (!tessString)
		tessString = textStrings[index].standardText;
	DS_ASSERT(tessString);
	if (testText->tessMaterial && *tessString)
	{
		text = dsText_createUTF8(testText->font, testText->allocator, tessString,
			textStrings[index].uniform);
		if (!text)
		{
			DS_LOG_ERROR_F("TestText", "Couldn't create text: %s", dsErrorString(errno));
			DS_PROFILE_FUNC_RETURN_VOID();
		}
		testText->tessText = dsTextLayout_create(testText->allocator, text,
			textStrings[index].styles, DS_ARRAY_SIZE(textStrings[index].styles));
		if (!testText->text)
		{
			dsText_destroy(text);
			DS_LOG_ERROR_F("TestText", "Couldn't create text: %s", dsErrorString(errno));
			DS_PROFILE_FUNC_RETURN_VOID();
		}
		if (!dsTextLayout_layout(testText->tessText, commandBuffer, textStrings[index].alignment,
			textStrings[index].maxWidth, textStrings[index].lineScale))
		{
			DS_LOG_ERROR_F("TestText", "Couldn't layout text: %s", dsErrorString(errno));
			DS_PROFILE_FUNC_RETURN_VOID();
		}

		DS_VERIFY(dsTextRenderBuffer_clear(testText->tessTextRender));
		if (!dsTextRenderBuffer_addText(testText->tessTextRender, testText->tessText, NULL))
		{
			DS_LOG_ERROR_F("TestText", "Couldn't add text: %s", dsErrorString(errno));
			DS_PROFILE_FUNC_RETURN_VOID();
		}
		DS_VERIFY(dsTextRenderBuffer_commit(testText->tessTextRender, commandBuffer));
	}

	setPositions(testText);
	DS_PROFILE_FUNC_RETURN_VOID();
}

static void nextText(TestText* testText)
{
	++testText->curString;
	if (testText->curString >= DS_ARRAY_SIZE(textStrings))
		testText->curString = 0;
	createText(testText, testText->renderer->mainCommandBuffer);
}

static void prevText(TestText* testText)
{
	if (testText->curString == 0)
		testText->curString = DS_ARRAY_SIZE(textStrings) - 1;
	else
		--testText->curString;
	createText(testText, testText->renderer->mainCommandBuffer);
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
		case dsEventType_SurfaceInvalidated:
			if (!createFramebuffer(testText, testText->renderer->mainCommandBuffer))
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
				case dsKeyCode_ACBack:
					dsApplication_quit(application, 0);
					return false;
				default:
					return true;
			}
		case dsEventType_TouchFingerDown:
			++testText->fingerCount;
			testText->maxFingers = dsMax(testText->fingerCount, testText->maxFingers);
			return true;
		case dsEventType_TouchFingerUp:
			if (testText->fingerCount == 0)
				return true;

			--testText->fingerCount;
			if (testText->fingerCount == 0)
			{
				switch (testText->maxFingers)
				{
					case 1:
						nextText(testText);
						break;
					case 2:
						prevText(testText);
						break;
					default:
						break;
				}
				testText->maxFingers = 0;
			}
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

	if (testText->setupCommands)
	{
		dsCommandBuffer* setupCommands = testText->setupCommands->currentBuffers[0];
		DS_VERIFY(dsCommandBuffer_submit(commandBuffer, setupCommands));
		DS_VERIFY(dsCommandBufferPool_destroy(testText->setupCommands));
		testText->setupCommands = NULL;
	}

	dsSurfaceClearValue clearValue;
	clearValue.colorValue.floatValue.r = 0.0f;
	clearValue.colorValue.floatValue.g = 0.1f;
	clearValue.colorValue.floatValue.b = 0.2f;
	clearValue.colorValue.floatValue.a = 1.0f;
	DS_VERIFY(dsRenderPass_begin(testText->renderPass, commandBuffer, testText->framebuffer, NULL,
		&clearValue, 1));

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

	if (textStrings[testText->curString].maxWidth != DS_TEXT_NO_WRAP)
	{
		DS_VERIFY(dsShader_bind(testText->limitShader, commandBuffer, testText->material, NULL,
			NULL));
		dsDrawRange drawRange = {6, 1, 0, 0};
		DS_VERIFY(dsRenderer_draw(renderer, commandBuffer, testText->limitGeometry, &drawRange,
			dsPrimitiveType_TriangleList));
		DS_VERIFY(dsShader_unbind(testText->limitShader, commandBuffer));
	}

	DS_VERIFY(dsRenderPass_end(testText->renderPass, commandBuffer));
}

static bool setupShaders(TestText* testText)
{
	DS_PROFILE_FUNC_START();

	dsRenderer* renderer = testText->renderer;
	dsResourceManager* resourceManager = renderer->resourceManager;
	dsAllocator* allocator = testText->allocator;

	char path[DS_PATH_MAX];
	if (!dsPath_combine(path, sizeof(path), assetsDir, shaderDir) ||
		!dsPath_combine(path, sizeof(path), path, "TestText.mslb"))
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create shader path: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	testText->shaderModule = dsShaderModule_loadResource(resourceManager, allocator,
		dsFileResourceType_Embedded, path, "TestText");
	if (!testText->shaderModule)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't load shader: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsShaderVariableElement sharedInfoElems[] =
	{
		{"screenSize", dsMaterialType_IVec2, 0}
	};
	testText->sharedInfoDesc = dsShaderVariableGroupDesc_create(resourceManager, allocator,
		sharedInfoElems, DS_ARRAY_SIZE(sharedInfoElems));
	if (!testText->sharedInfoDesc)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create shader variable group description: %s",
			dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
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
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsMaterialElement materialElems[] =
	{
		{"SharedInfo", dsMaterialType_VariableGroup, 0, testText->sharedInfoDesc,
			dsMaterialBinding_Material, 0},
		{"position", dsMaterialType_Vec2, 0, NULL, dsMaterialBinding_Material, 0},
		{"yMult", dsMaterialType_Float, 0, NULL, dsMaterialBinding_Material, 0},
		{"fontTex", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0},
		{"bounds", dsMaterialType_Vec4, 0, NULL, dsMaterialBinding_Material, 0}
	};
	testText->materialDesc = dsMaterialDesc_create(resourceManager, allocator, materialElems,
		DS_ARRAY_SIZE(materialElems));
	if (!testText->materialDesc)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create material description: %s",
			dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	uint32_t sharedInfoElement = dsMaterialDesc_findElement(testText->materialDesc, "SharedInfo");
	DS_ASSERT(sharedInfoElement != DS_MATERIAL_UNKNOWN);
	testText->positionElement = dsMaterialDesc_findElement(testText->materialDesc, "position");
	DS_ASSERT(testText->positionElement != DS_MATERIAL_UNKNOWN);
	uint32_t yMultElement = dsMaterialDesc_findElement(testText->materialDesc, "yMult");
	DS_ASSERT(yMultElement != DS_MATERIAL_UNKNOWN);
	testText->limitBoundsElement = dsMaterialDesc_findElement(testText->materialDesc, "bounds");
	DS_ASSERT(testText->limitBoundsElement != DS_MATERIAL_UNKNOWN);

	testText->material = dsMaterial_create(resourceManager, allocator, testText->materialDesc);
	if (!testText->material)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create material: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}
	DS_VERIFY(dsMaterial_setVariableGroup(testText->material, sharedInfoElement,
		testText->sharedInfoGroup));
	float yMult = renderer->clipInvertY ? 1.0f : -1.0f;
	DS_VERIFY(dsMaterial_setElementData(testText->material, yMultElement, &yMult,
		dsMaterialType_Float, 0, 1));

	testText->shader = dsShader_createName(resourceManager, allocator, testText->shaderModule,
		"Font", testText->materialDesc);
	if (!testText->shader)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create shader: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (renderer->hasTessellationShaders)
	{
		testText->tessMaterial = dsMaterial_create(resourceManager, allocator,
			testText->materialDesc);
		if (!testText->tessMaterial)
		{
			DS_LOG_ERROR_F("TestText", "Couldn't create material: %s", dsErrorString(errno));
			DS_PROFILE_FUNC_RETURN(false);
		}
		DS_VERIFY(dsMaterial_setVariableGroup(testText->tessMaterial, sharedInfoElement,
			testText->sharedInfoGroup));
		DS_VERIFY(dsMaterial_setElementData(testText->tessMaterial, yMultElement, &yMult,
			dsMaterialType_Float, 0, 1));

		testText->tessShader = dsShader_createName(resourceManager, allocator,
			testText->shaderModule, "FontTess", testText->materialDesc);
		if (!testText->tessShader)
		{
			DS_LOG_ERROR_F("TestText", "Couldn't create shader: %s", dsErrorString(errno));
			DS_PROFILE_FUNC_RETURN(false);
		}
	}

	testText->limitShader = dsShader_createName(resourceManager, allocator,
		testText->shaderModule, "Box", testText->materialDesc);
	if (!testText->limitShader)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create shader: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	DS_PROFILE_FUNC_RETURN(true);
}

static bool setupText(TestText* testText, dsTextQuality quality, const char* fontPath)
{
	DS_PROFILE_FUNC_START();

	dsRenderer* renderer = testText->renderer;
	dsResourceManager* resourceManager = renderer->resourceManager;
	dsAllocator* allocator = testText->allocator;

	char path[DS_PATH_MAX];
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
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (renderer->hasTessellationShaders)
	{

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
			DS_PROFILE_FUNC_RETURN(false);
		}
	}

	testText->faceGroup = dsFaceGroup_create(allocator, NULL, DS_DEFAULT_MAX_FACES);
	if (!testText->faceGroup)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create face group: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!fontPath)
	{
		if (!dsPath_combine(path, sizeof(path), assetsDir, "Fonts") ||
			!dsPath_combine(path, sizeof(path), path, "NotoSans-Regular.ttc"))
		{
			DS_LOG_ERROR_F("TestText", "Couldn't create font path: %s", dsErrorString(errno));
		}
		fontPath = path;
	}
	if (!dsFaceGroup_loadFaceResource(testText->faceGroup, allocator, dsFileResourceType_Embedded,
		fontPath, "Latin"))
	{
		DS_LOG_ERROR_F("TestText", "Couldn't load font face: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!dsPath_combine(path, sizeof(path), assetsDir, "Fonts") ||
		!dsPath_combine(path, sizeof(path), path, "NotoSansArabic-Regular.ttf") ||
		!dsFaceGroup_loadFaceResource(testText->faceGroup, allocator, dsFileResourceType_Embedded,
			path, "Arabic"))
	{
		DS_LOG_ERROR_F("TestText", "Couldn't load font face: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!dsPath_combine(path, sizeof(path), assetsDir, "Fonts") ||
		!dsPath_combine(path, sizeof(path), path, "NotoSansThai-Regular.ttf") ||
		!dsFaceGroup_loadFaceResource(testText->faceGroup, allocator, dsFileResourceType_Embedded,
			path, "Thai"))
	{
		DS_LOG_ERROR_F("TestText", "Couldn't load font face: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

#ifdef CHINESE_FONT_PATH
	if (!dsFaceGroup_loadFaceResource(testText->faceGroup, allocator, dsFileResourceType_External,
		CHINESE_FONT_PATH, "Chinese"))
	{
		DS_LOG_ERROR_F("TestText", "Couldn't load font face: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}
#endif

	const char* faceNames[] = {"Latin", "Arabic", "Thai",
#ifdef CHINESE_FONT_PATH
		"Chinese"
#endif
	};
	testText->font = dsFont_create(testText->faceGroup, resourceManager, allocator, faceNames,
		DS_ARRAY_SIZE(faceNames), quality, dsTextCache_Large);
	if (!testText->font)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create font: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsTimer timer = dsTimer_create();
	double startTime = dsTimer_time(timer);
	if (!dsFont_preloadASCII(testText->font, testText->setupCommands->currentBuffers[0]))
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create preload ASCII characters: %s",
			dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}
	DS_LOG_INFO_F("TestText", "Loading ASCII characters took %f s.",
		dsTimer_time(timer) - startTime);

	uint32_t textureElement = dsMaterialDesc_findElement(testText->materialDesc, "fontTex");
	DS_ASSERT(textureElement != DS_MATERIAL_UNKNOWN);
	dsMaterial_setTexture(testText->material, textureElement, dsFont_getTexture(testText->font));
	if (testText->tessMaterial)
	{
		dsMaterial_setTexture(testText->tessMaterial, textureElement,
			dsFont_getTexture(testText->font));
	}

	DS_PROFILE_FUNC_RETURN(true);
}

static bool setupLimit(TestText* testText)
{
	dsRenderer* renderer = testText->renderer;
	dsResourceManager* resourceManager = renderer->resourceManager;
	dsAllocator* allocator = testText->allocator;

	dsVector2f positions[] = {{{0.0f, 0.0f}}, {{0.0f, 1.0f}}, {{1.0f, 1.0f}},
		{{1.0f, 1.0f}}, {{1.0f, 0.0f}}, {{0.0f, 0.0f}}};
	testText->limitBuffer = dsGfxBuffer_create(resourceManager, allocator, dsGfxBufferUsage_Vertex,
		dsGfxMemory_Draw | dsGfxMemory_Static | dsGfxMemory_GPUOnly, positions, sizeof(positions));
	if (!testText->limitBuffer)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create graphics buffer: %s", dsErrorString(errno));
		return false;
	}

	dsVertexFormat vertexFormat;
	DS_VERIFY(dsVertexFormat_initialize(&vertexFormat));
	vertexFormat.elements[dsVertexAttrib_Position].format = dsGfxFormat_decorate(dsGfxFormat_X32Y32,
		dsGfxFormat_Float);
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Position, true));
	DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(&vertexFormat));
	dsVertexBuffer vertexBuffer = {testText->limitBuffer, 0, 6, vertexFormat};
	dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS] =
		{&vertexBuffer, NULL, NULL, NULL};
	testText->limitGeometry =
		dsDrawGeometry_create(resourceManager, allocator, vertexBuffers, NULL);
	if (!testText->limitBuffer)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create draw geometry: %s", dsErrorString(errno));
		return false;
	}

	return true;
}

static bool setup(TestText* testText, dsApplication* application, dsAllocator* allocator,
	dsTextQuality quality, const char* fontPath)
{
	DS_PROFILE_FUNC_START();

	dsRenderer* renderer = application->renderer;
	testText->allocator = allocator;
	testText->renderer = renderer;

	testText->setupCommands = dsCommandBufferPool_create(renderer, allocator,
		dsCommandBufferUsage_Standard, 1);
	if (!testText->setupCommands)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create setup command buffer: %s",
			dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsCommandBuffer* setupCommands = testText->setupCommands->currentBuffers[0];
	if (!dsCommandBuffer_begin(setupCommands))
	{
		DS_LOG_ERROR_F("TestText", "Couldn't begin setup command buffer: %s",
			dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsEventResponder responder = {&processEvent, testText, 0, 0};
	DS_VERIFY(dsApplication_addEventResponder(application, &responder));

	uint32_t width = dsApplication_adjustWindowSize(application, 0, 800);
	uint32_t height = dsApplication_adjustWindowSize(application, 0, 600);
	testText->window = dsWindow_create(application, allocator, "Test Text", NULL,
		NULL, width, height, dsWindowFlags_Resizeable | dsWindowFlags_DelaySurfaceCreate);
	if (!testText->window)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create window: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (DS_ANDROID || DS_IOS)
		dsWindow_setStyle(testText->window, dsWindowStyle_FullScreen);

	if (!dsWindow_createSurface(testText->window))
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create window surface: %s", dsErrorString(errno));
		return false;
	}

	// Adjust the text size based on the DPI.
	// NOTE: On Android starting a new activity will use the same address space, so avoid applying
	// multiple times.
	if (!textInitialized)
	{
		float dpiScale = application->displays[0].dpi/DS_DEFAULT_DPI;
#if DS_ANDROID || DS_IOS
		// This is too large for smaller screens.
		dpiScale *= 0.5f;
#endif
		for (uint32_t i = 0; i < DS_ARRAY_SIZE(textStrings); ++i)
		{
			TextInfo* text = textStrings + i;
			if (text->maxWidth != DS_TEXT_NO_WRAP)
				text->maxWidth *= dpiScale;
			for (uint32_t j = 0; j < DS_ARRAY_SIZE(textStrings[i].styles); ++j)
			{
				text->styles[j].scale *= dpiScale;
				text->styles[j].verticalOffset *= dpiScale;
			}
		}

		textInitialized = true;
	}

	DS_VERIFY(dsWindow_setDrawFunction(testText->window, &draw, testText));

	dsAttachmentInfo attachment = {dsAttachmentUsage_Clear | dsAttachmentUsage_KeepAfter,
		renderer->surfaceColorFormat, DS_DEFAULT_ANTIALIAS_SAMPLES};

	dsAttachmentRef colorAttachment = {0, true};
	uint32_t depthStencilAttachment = DS_NO_ATTACHMENT;
	dsRenderSubpassInfo subpass =
	{
		"TestText", NULL, &colorAttachment, {depthStencilAttachment, false}, 0, 1
	};
	testText->renderPass = dsRenderPass_create(renderer, allocator, &attachment, 1, &subpass, 1,
		NULL, DS_DEFAULT_SUBPASS_DEPENDENCIES);
	if (!testText->renderPass)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create render pass: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!setupShaders(testText))
		DS_PROFILE_FUNC_RETURN(false);

	if (!setupText(testText, quality, fontPath))
		DS_PROFILE_FUNC_RETURN(false);

	if (!setupLimit(testText))
		DS_PROFILE_FUNC_RETURN(false);

	if (!createFramebuffer(testText, setupCommands))
		DS_PROFILE_FUNC_RETURN(false);

	for (uint32_t i = 0; i < DS_ARRAY_SIZE(textStrings); ++i)
	{
		for (unsigned int j = 0; j < 3; ++j)
		{
			DS_VERIFY(dsFont_applyHintingAndAntiAliasing(testText->font,
				textStrings[i].styles + j, 1.0f, 1.0f));
		}
	}

	testText->curString = 0;
	createText(testText, setupCommands);

	if (!dsCommandBuffer_end(setupCommands))
	{
		DS_LOG_ERROR_F("TestText", "Couldn't end setup command buffer: %s",
			dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}
	DS_PROFILE_FUNC_RETURN(true);
}

static void shutdown(TestText* testText)
{
	DS_VERIFY(dsTextRenderBuffer_destroy(testText->tessTextRender));
	dsTextLayout_destroyLayoutAndText(testText->tessText);
	DS_VERIFY(dsTextRenderBuffer_destroy(testText->textRender));
	dsTextLayout_destroyLayoutAndText(testText->text);
	DS_VERIFY(dsFont_destroy(testText->font));
	dsFaceGroup_destroy(testText->faceGroup);
	DS_VERIFY(dsShader_destroy(testText->tessShader));
	DS_VERIFY(dsShader_destroy(testText->shader));
	DS_VERIFY(dsShader_destroy(testText->limitShader));
	dsMaterial_destroy(testText->tessMaterial);
	dsMaterial_destroy(testText->material);
	DS_VERIFY(dsDrawGeometry_destroy(testText->limitGeometry));
	DS_VERIFY(dsGfxBuffer_destroy(testText->limitBuffer));
	DS_VERIFY(dsMaterialDesc_destroy(testText->materialDesc));
	DS_VERIFY(dsShaderVariableGroup_destroy(testText->sharedInfoGroup));
	DS_VERIFY(dsShaderVariableGroupDesc_destroy(testText->sharedInfoDesc));
	DS_VERIFY(dsShaderModule_destroy(testText->shaderModule));
	DS_VERIFY(dsRenderPass_destroy(testText->renderPass));
	DS_VERIFY(dsFramebuffer_destroy(testText->framebuffer));
	DS_VERIFY(dsWindow_destroy(testText->window));
	DS_VERIFY(dsCommandBufferPool_destroy(testText->setupCommands));
}

int dsMain(int argc, const char** argv)
{
#if DS_HAS_EASY_PROFILER
	dsEasyProfiler_start(true);
	dsEasyProfiler_startListening(DS_DEFAULT_EASY_PROFILER_PORT);
#endif

	dsRendererType rendererType = dsRendererType_Default;
	dsTextQuality quality = dsTextQuality_Medium;
	const char* fontPath = NULL;
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			printHelp(argv[0]);
			return 0;
		}
		else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--font") == 0)
		{
			if (i == argc - 1)
			{
				printf("-f/--font requires an extra argument\n");
				printHelp(argv[0]);
				return 1;
			}
			fontPath = argv[++i];
		}
		else if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--low") == 0)
			quality = dsTextQuality_Low;
		else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--medium") == 0)
			quality = dsTextQuality_Medium;
		else if (strcmp(argv[i], "-H") == 0 || strcmp(argv[i], "--high") == 0)
			quality = dsTextQuality_High;
		else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--very-high") == 0)
			quality = dsTextQuality_VeryHigh;
		else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--renderer") == 0)
		{
			if (i == argc - 1)
			{
				printf("--renderer option requires an argument\n");
				printHelp(argv[0]);
				return 1;
			}
			rendererType = dsRenderBootstrap_rendererTypeFromName(argv[++i]);
			if (rendererType == dsRendererType_Default)
			{
				printf("Unknown renderer type: %s\n", argv[i]);
				printHelp(argv[0]);
				return 1;
			}
		}
		else if (*argv[i])
		{
			printf("Unknown option: %s\n", argv[i]);
			printHelp(argv[0]);
			return 1;
		}
	}

	DS_LOG_INFO_F("TestText", "Render using %s", dsRenderBootstrap_rendererName(rendererType));

	dsSystemAllocator renderAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&renderAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator applicationAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&applicationAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator testTextAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&testTextAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsRendererOptions rendererOptions;
	dsRenderer_defaultOptions(&rendererOptions, "TestText", 0);
	rendererOptions.depthBits = 0;
	rendererOptions.stencilBits = 0;
	dsRenderer* renderer = dsRenderBootstrap_createRenderer(rendererType,
		(dsAllocator*)&renderAllocator, &rendererOptions);
	if (!renderer)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create renderer: %s", dsErrorString(errno));
		return 2;
	}

	dsRenderer_setVsync(renderer, true);
	dsRenderer_setDefaultAnisotropy(renderer, renderer->maxAnisotropy);
#if DS_DEBUG
	dsRenderer_setExtraDebugging(renderer, true);
#endif

	dsShaderVersion shaderVersions[] =
	{
		{DS_VK_RENDERER_ID, DS_ENCODE_VERSION(1, 0, 0)},
		{DS_MTL_RENDERER_ID, DS_ENCODE_VERSION(1, 1, 0)},
		{DS_GL_RENDERER_ID, DS_ENCODE_VERSION(1, 1, 0)},
		{DS_GL_RENDERER_ID, DS_ENCODE_VERSION(1, 5, 0)},
		{DS_GL_RENDERER_ID, DS_ENCODE_VERSION(4, 0, 0)},
		{DS_GLES_RENDERER_ID, DS_ENCODE_VERSION(1, 0, 0)},
		{DS_GLES_RENDERER_ID, DS_ENCODE_VERSION(3, 0, 0)},
		{DS_GLES_RENDERER_ID, DS_ENCODE_VERSION(3, 2, 0)},
	};
	DS_VERIFY(dsRenderer_shaderVersionToString(shaderDir, DS_ARRAY_SIZE(shaderDir), renderer,
		dsRenderer_chooseShaderVersion(renderer, shaderVersions, DS_ARRAY_SIZE(shaderVersions))));

	dsApplication* application = dsSDLApplication_create((dsAllocator*)&applicationAllocator,
		renderer, argc, argv, "DeepSea", "TestText");
	if (!application)
	{
		DS_LOG_ERROR_F("TestText", "Couldn't create application: %s", dsErrorString(errno));
		dsRenderer_destroy(renderer);
		return 2;
	}

	TestText testText;
	memset(&testText, 0, sizeof(testText));
	if (!setup(&testText, application, (dsAllocator*)&testTextAllocator, quality, fontPath))
	{
		shutdown(&testText);
		return 3;
	}

	int exitCode = dsApplication_run(application);

	shutdown(&testText);
	dsSDLApplication_destroy(application);
	dsRenderer_destroy(renderer);

	if (!validateAllocator((dsAllocator*)&renderAllocator, "render"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&applicationAllocator, "application"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&testTextAllocator, "TestText"))
		exitCode = 4;

	return exitCode;
}

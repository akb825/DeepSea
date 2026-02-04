/*
 * Copyright 2017-2025 Aaron Barany
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

#pragma once

#include <DeepSea/Core/Config.h>
#include "TextInternal.h"

bool dsIsSpace(uint32_t charcode);
const char* dsFontFace_getName(const dsFontFace* face);
uint32_t dsFontFace_getCodepointGlyph(const dsFontFace* face, uint32_t codepoint);
bool dsFontFace_cacheGlyph(dsAlignedBox2f* outBounds, dsFontFace* face,
	dsCommandBuffer* commandBuffer, dsTexture* texture, uint32_t glyph, uint32_t glyphIndex,
	dsFont* font);

void dsFaceGroup_lock(const dsFaceGroup* group);
void dsFaceGroup_unlock(const dsFaceGroup* group);
dsAllocator* dsFaceGroup_getScratchAllocator(const dsFaceGroup* group);
dsFontFace* dsFaceGroup_findFace(const dsFaceGroup* group, const char* name);
// Runs are in characters rather than codepoints.
dsRunInfo* dsFaceGroup_findBidiRuns(
	uint32_t* outCount, dsFaceGroup* group, const void* string, dsUnicodeType type);
dsText* dsFaceGroup_scratchText(dsFaceGroup* group, uint32_t length);
bool dsFaceGroup_scratchRanges(dsFaceGroup* group, uint32_t rangeCount);
bool dsFaceGroup_scratchGlyphs(dsFaceGroup* group, uint32_t length);

bool dsFaceGroup_isScriptUnique(uint32_t script);
bool dsFaceGroup_isScriptCommon(uint32_t script);
bool dsFaceGroup_areScriptsEqual(uint32_t script1, uint32_t script2);
bool dsFaceGroup_isScriptBoundary(
	uint32_t script, bool scriptUnique, bool hasLastScript, uint32_t lastScript);
dsTextDirection dsFaceGroup_textDirection(uint32_t script);

// Locking not needed for this function.
uint32_t dsFont_codepointScript(const dsFont* font, uint32_t codepoint);

const dsGlyphInfo* dsFont_getGlyphInfo(
	dsFont* font, dsCommandBuffer* commandBuffer, uint32_t face, uint32_t glyph);
uint32_t dsFont_getGlyphIndex(dsFont* font, const dsGlyphInfo* glyph);
uint32_t dsFont_findFaceForCodepoint(const dsFont* font, uint32_t codepoint);
bool dsFont_shapeRange(const dsFont* font, dsText* text, uint32_t rangeIndex,
	uint32_t firstCodepoint, uint32_t start, uint32_t count, uint32_t newlineCount,
	dsTextDirection direction);

// Pixel values are 0 or 1, +Y points down.
bool dsFont_writeGlyphToTexture(dsCommandBuffer* commandBuffer, dsTexture* texture,
	uint32_t glyphIndex, uint32_t glyphSize, uint32_t texMultiplier,
	const dsGlyphGeometry* geometry);
void dsFont_getGlyphTexturePos(dsTexturePosition* outPos, uint32_t glyphIndex,
	uint32_t glyphSize, uint32_t texMultiplier);
void dsFont_getGlyphTextureBounds(dsAlignedBox2f* outBounds, const dsTexturePosition* texturePos,
	const dsVector2f* glyphBoundsSize, uint32_t glyphSize, uint32_t texMultiplier);

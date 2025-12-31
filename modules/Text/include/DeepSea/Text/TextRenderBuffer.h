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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Text/Export.h>
#include <DeepSea/Text/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and working with text render buffers.
 */

/**
 * @brief Counts the number of standard (non-icon) glyphs.
 *
 * This will skip any whitespace or icons.
 *
 * @param layout The text layout that will be rendered.
 * @param firstChar The index of the first character.
 * @param charCount The number of characters.
 * @return The number of glyphs.
 */
DS_TEXT_EXPORT uint32_t dsTextRenderBuffer_countStandardGlyphs(
	const dsTextLayout* layout, uint32_t firstChar, uint32_t charCount);

/**
 * @brief Counts the number of icon glyphs.
 * @param layout The text layout that will be rendered.
 * @param firstChar The index of the first character.
 * @param charCount The number of characters.
 * @return The number of glyphs.
 */
DS_TEXT_EXPORT uint32_t dsTextRenderBuffer_countIconGlyphs(
	const dsTextLayout* layout, uint32_t firstChar, uint32_t charCount);

/**
 * @brief Counts the number of standard and icon glyphs separately.
 * @remark errno will be set on failure.
 * @param[inout] outStandardCount The number of standard glyphs. This will add to the existing
 *     value.
 * @param[inout] outIconCount The number of icon glyphs. This will add to the existing value.
 * @param layout The text layout that will be rendered.
 * @param firstChar The index of the first character.
 * @param charCount The number of characters.
 * @return False if the parameters are invalid.
 */
DS_TEXT_EXPORT bool dsTextRenderBuffer_countStandardIconGlyphs(uint32_t* outStandardCount,
	uint32_t* outIconCount, const dsTextLayout* layout, uint32_t firstChar, uint32_t charCount);

/**
 * @brief Creates a text render buffer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the render buffr with.
 * @param resourceManager The resource manager to create the graphics resources with.
 * @param maxStandardGlyphs The maximum number of standard glyphs to draw at once.
 * @param maxIconGlyphs The maximum number of icon glyphs to draw at once.
 * @param vertexFormat The vertex format to use.
 * @param tessellationShader True if the tessellation shader will create the quad instead of storing
 *     quads in the vertex buffer.
 * @param glyphDataFunc The function to get the data for a glyph.
 * @param userData The user data for the glyph data.
 * @return The render buffer, or NULL if it couldn't be created.
 */
DS_TEXT_EXPORT dsTextRenderBuffer* dsTextRenderBuffer_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, uint32_t maxStandardGlyphs, uint32_t maxIconGlyphs,
	const dsVertexFormat* vertexFormat, bool tessellationShader, dsGlyphDataFunction glyphDataFunc,
	void* userData);

/**
 * @brief Adds text to the render buffer.
 * @remark errno will be set on failure.
 * @param renderBuffer The text render buffer.
 * @param layout The lain out text to draw.
 * @param layoutUserData User data associated with the layout.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsTextRenderBuffer_addText(dsTextRenderBuffer* renderBuffer,
	const dsTextLayout* layout, void* layoutUserData);

/**
 * @brief Adds a text range to the render buffer.
 * @remark errno will be set on failure.
 * @param renderBuffer The text render buffer.
 * @param layout The lain out text to draw.
 * @param layoutUserData User data associated with the layout.
 * @param firstChar The first character to draw.
 * @param charCount The number of characters to draw.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsTextRenderBuffer_addTextRange(dsTextRenderBuffer* renderBuffer,
	const dsTextLayout* layout, void* layoutUserData, uint32_t firstChar, uint32_t charCount);

/**
 * @brief Gets the number of remaining standard glyphs that can be rendered.
 * @param renderBuffer The render buffer.
 * @return The number of remaining standard glyphs.
 */
DS_TEXT_EXPORT uint32_t dsTextRenderBuffer_getRemainingStandardGlyphs(
	const dsTextRenderBuffer* renderBuffer);

/**
 * @brief Gets the number of remaining icon glyphs that can be rendered.
 * @param renderBuffer The render buffer.
 * @return The number of icon standard glyphs.
 */
DS_TEXT_EXPORT uint32_t dsTextRenderBuffer_getRemainingIconGlyphs(
	const dsTextRenderBuffer* renderBuffer);

/**
 * @brief Commits added text to be rendered.
 * @remark This must be called outside of a render pass.
 * @remark errno will be set on failure.
 * @param renderBuffer The text render buffer.
 * @param commandBuffer The command buffer to queue commands onto.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsTextRenderBuffer_commit(dsTextRenderBuffer* renderBuffer,
	dsCommandBuffer* commandBuffer);

/**
 * @brief Clears the text currently pushed onto the buffer.
 * @remark errno will be set on failure.
 * @param renderBuffer The text render buffer.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsTextRenderBuffer_clear(dsTextRenderBuffer* renderBuffer);

/**
 * @brief Draws the standard glyphs that have been placed onto the buffer.
 * @remark errno will be set on failure.
 * @param renderBuffer The text render buffer.
 * @param commandBuffer The command buffer to queue commands onto.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsTextRenderBuffer_drawStandardGlyphs(
	dsTextRenderBuffer* renderBuffer, dsCommandBuffer* commandBuffer);

/**
 * @brief Draws a range of standard glyphs that have been placed onto the buffer.
 *
 * Note that glyphs that have a bounds of size 0 will be skipped.
 *
 * @remark errno will be set on failure.
 * @param renderBuffer The text render buffer.
 * @param commandBuffer The command buffer to queue commands onto.
 * @param firstGlyph The first glyph to draw.
 * @param glyphCount The number of glyphs to draw.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsTextRenderBuffer_drawStandardGlyphRange(dsTextRenderBuffer* renderBuffer,
	dsCommandBuffer* commandBuffer, uint32_t firstGlyph, uint32_t glyphCount);

/**
 * @brief Draws the icon glyphs that have been placed onto the buffer.
 * @remark errno will be set on failure.
 * @param renderBuffer The text render buffer.
 * @param commandBuffer The command buffer to queue commands onto.
 * @param globalValues The global values to use with icon shaders.
 * @param renderStates The dynamic render states to use with icon shaders.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsTextRenderBuffer_drawIconGlyphs(dsTextRenderBuffer* renderBuffer,
	dsCommandBuffer* commandBuffer, const dsSharedMaterialValues* globalValues,
	const dsDynamicRenderStates* renderStates);

/**
 * @brief Draws a range of icon glyphs that have been placed onto the buffer.
 *
 * Note that glyphs that have a bounds of size 0 will be skipped.
 *
 * @remark errno will be set on failure.
 * @param renderBuffer The text render buffer.
 * @param commandBuffer The command buffer to queue commands onto.
 * @param firstGlyph The first glyph to draw.
 * @param glyphCount The number of glyphs to draw.
 * @param globalValues The global values to use with icon shaders.
 * @param renderStates The dynamic render states to use with icon shaders.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsTextRenderBuffer_drawIconGlyphRange(dsTextRenderBuffer* renderBuffer,
	dsCommandBuffer* commandBuffer, uint32_t firstGlyph, uint32_t glyphCount,
	const dsSharedMaterialValues* globalValues, const dsDynamicRenderStates* renderStates);

/**
 * @brief Destroy a text render buffer.
 * @remark errno will be set on failure.
 * @param renderBuffer The render buffer.
 * @return False if the render buffer can't be destroyed.
 */
DS_TEXT_EXPORT bool dsTextRenderBuffer_destroy(dsTextRenderBuffer* renderBuffer);

#ifdef __cplusplus
}
#endif

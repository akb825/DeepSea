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
 * @brief Creates a text render buffer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the render buffr with.
 * @param resourceManager The resource manager to create the graphics resources with.
 * @param maxGlyphs The maximum number of glyphs to draw at once.
 * @param vertexFormat The vertex format to use.
 * @param tessellationShader True if the tessellation shader will create the quad instead of storing
 *     quads in the vertex buffer.
 * @param glyphDataFunc The function to get the data for a glyph.
 * @param userData The user data for the glyph data.
 * @return The render buffer, or NULL if it couldn't be created.
 */
DS_TEXT_EXPORT dsTextRenderBuffer* dsTextRenderBuffer_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, uint32_t maxGlyphs, const dsVertexFormat* vertexFormat,
	bool tessellationShader, dsGlyphDataFunction glyphDataFunc, void* userData);

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
 * @brief Gets the number of remaining glyphs that can be rendered.
 * @param renderBuffer The render buffer.
 * @return The number of remaining glyphs.
 */
DS_TEXT_EXPORT uint32_t dsTextRenderBuffer_getRemainingGlyphs(
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
 * @brief Draws the text that has been placed onto the buffer.
 * @remark errno will be set on failure.
 * @param renderBuffer The text render buffer.
 * @param commandBuffer The command buffer to queue commands onto.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsTextRenderBuffer_draw(dsTextRenderBuffer* renderBuffer,
	dsCommandBuffer* commandBuffer);

/**
 * @brief Draws a range of text that has been placed onto the buffer.
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
DS_TEXT_EXPORT bool dsTextRenderBuffer_drawRange(dsTextRenderBuffer* renderBuffer,
	dsCommandBuffer* commandBuffer, uint32_t firstGlyph, uint32_t glyphCount);

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

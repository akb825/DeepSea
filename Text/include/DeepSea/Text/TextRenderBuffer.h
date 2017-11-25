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
 * @param glyphDataFunc The function to get the data for a glyph.
 * @param userData The user data for the glyph data.
 * @return The render buffer, or NULL if it couldn't be created.
 */
DS_TEXT_EXPORT dsTextRenderBuffer* dsTextRenderBuffer_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, uint32_t maxGlyphs, const dsVertexFormat* vertexFormat,
	dsGlyphDataFunction glyphDataFunc, void* userData);

/**
 * @brief Queues text onto the render buffer.
 * @remark errno will be set on failure.
 * @param renderBuffer The text render buffer to draw with.
 * @param commandBuffer The command buffer to queue commands onto.
 * @param text The lain out text to draw.
 * @param firstGlyph The first glyph to draw.
 * @param glyphCount The number of glyphs to draw.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsTextRenderBuffer_queueText(dsTextRenderBuffer* renderBuffer,
	dsCommandBuffer* commandBuffer, const dsTextLayout* text, uint32_t firstGlyph,
	uint32_t glyphCount);

/**
 * @brief Flushes any remaining queued text.
 * @remark errno will be set on failure.
 * @param commandBuffer The command buffer to queue commands onto.
 * @param renderer The renderer to draw with.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsTextRenderBuffer_flush(dsTextRenderBuffer* renderBuffer,
	dsCommandBuffer* commandBuffer);

/**
 * @brief Destroy a text render buffer.
 * @remark errno will be set on failure.
 * @param renderBuffer The render buffer to destroy.
 * @return False if the render buffer can't be destroyed.
 */
DS_TEXT_EXPORT bool dsTextBuffer_destroy(dsTextRenderBuffer* renderBuffer);

#ifdef __cplusplus
}
#endif

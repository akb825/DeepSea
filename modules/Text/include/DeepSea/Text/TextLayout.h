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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Render/Types.h>
#include <DeepSea/Text/Export.h>
#include <DeepSea/Text/Types.h>
#include <float.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for laying out text to prepare it for rendering.
 *
 * The reuslts from text layout can be re-used so long DS_GLYPH_SLOTS unique glyphs aren't used.
 * After that point, glyphs will be replaced with their new values.
 *
 * In order to guarantee that the glyphs are correct, either dsTextLayout_layout() or
 * dsTextLayout_refresh() will need to be called. In the extremely * unlikely event that a single
 * piece of text exceeds this limit, it will need to be split. In the still very unlikely, but
 * slightly more plausable event that multiple pieces of text exceed this limit in a single frame,
 * some text will need to be drawn before laying out or refreshing more text to avoid glyphs from
 * being evicted before they are drawn.
 *
 * In more realistic situations, dsTextLayout_refresh() should be called when there are major
 * changes in text being drawn. This will account for cases where enough new glyphs are added that
 * could evict existing glyphs.
 */

/**
 * @brief Constant to not wrap text.
 */
#define DS_TEXT_NO_WRAP FLT_MAX

/**
 * @brief Gets the full allocation size for dsTextLayout.
 *
 * This won't include any memory for computing the lines.
 *
 * @param text The text that will be used to create the layout.
 * @param styleCount The number of styles.
 * @return The full allocation size or 0 if invalid.
 */
DS_TEXT_EXPORT size_t dsTextLayout_fullAllocSize(const dsText* text, uint32_t styleCount);

/**
 * @brief Applies slant to the bounds for a glyph.
 * @remark errno will be set on failure.
 * @param[out] outBounds The adjusted bounds.
 * @param glyphBounds The bounds of the glyph.
 * @param slant The slant to apply.
 * @return False if the parameters are invalid.
 */
DS_TEXT_EXPORT bool dsTextLayout_applySlantToBounds(dsAlignedBox2f* outBounds,
	const dsAlignedBox2f* glyphBounds, float slant);

/**
 * @brief Creates a layout for a piece fo text.
 *
 * The text will not be lain out until dsTextLayout_layout() is called.
 *
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the layout for.
 * @param text The text to create the layout for.
 * @param styles The styles to use for the layout. The character ranges must be monotomically
 *     increasing, non-overlapping, and cover the full range of text.
 * @param styleCount The number of styles.
 */
DS_TEXT_EXPORT dsTextLayout* dsTextLayout_create(dsAllocator* allocator, const dsText* text,
	const dsTextStyle* styles, uint32_t styleCount);

/**
 * @brief Resolves the alignment based on the text.
 *
 * This will resolve dsTextAlign_Start and dsTextAlign_End to
 * dsTextAlign_Left or dsTextAlign_Right.
 *
 * @remark This is automatically called as part of dsTextLayout_layout().
 * @param layout The text layout.
 * @param alignment The initial alignment.
 * @return The resolved alignment.
 */
DS_TEXT_EXPORT dsTextAlign dsTextLayout_resolveAlign(const dsTextLayout* layout,
	dsTextAlign alignment);

/**
 * @brief Performs layout on the text, preparing it to be rendered.
 * @remark errno will be set on failure.
 * @param layout The layout to process.
 * @param commandBuffer The command buffer to queue any texture operations on.
 * @param alignment The alignment to follow when laying out the text.
 * @param maxWidth The maximum width before breaking text. Set to DS_TEXT_NO_WRAP to avoid wrapping.
 * @param lineScale The scale to apply to the distance between each line. Set to 1 to use the base
 *     font height directly.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsTextLayout_layout(dsTextLayout* layout, dsCommandBuffer* commandBuffer,
	dsTextAlign alignment, float maxWidth, float lineScale);

/**
 * @brief Refreshes the glyphs in the cache, ensuring they are available to be rendered.
 * @remark errno will be set on failure.
 * @param layout The layout to refresh.
 * @param commandBuffer The command buffer to queue any texture operations on.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsTextLayout_refresh(dsTextLayout* layout, dsCommandBuffer* commandBuffer);

/**
 * @brief Refreshes the glyphs in the cache for a range of characters, ensuring they are available
 *     to be rendered.
 * @remark errno will be set on failure.
 * @param layout The layout to refresh.
 * @param commandBuffer The command buffer to queue any texture operations on.
 * @param firstChar The first character to refresh.
 * @param charCount The number of characters to refresh.
 * @return False if an error occurred.
 */
DS_TEXT_EXPORT bool dsTextLayout_refreshRange(dsTextLayout* layout, dsCommandBuffer* commandBuffer,
	uint32_t firstChar, uint32_t charCount);

/**
 * @brief Destroys a text layout object.
 * @param layout The layout to destroy.
 */
DS_TEXT_EXPORT void dsTextLayout_destroy(dsTextLayout* layout);

/**
 * @brief Destroys a text layout object and its containing text.
 * @param layout The layout to destroy.
 */
DS_TEXT_EXPORT void dsTextLayout_destroyLayoutAndText(dsTextLayout* layout);

#ifdef __cplusplus
}
#endif

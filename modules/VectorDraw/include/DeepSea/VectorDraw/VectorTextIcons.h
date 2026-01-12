/*
 * Copyright 2026 Aaron Barany
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
#include <DeepSea/VectorDraw/Export.h>
#include <DeepSea/VectorDraw//Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating vector text icons.
 *
 * This provides the functions to dsTextIcons to be able to use vector images to display the icons.
 *
 * @see dsTextIcons
 */

/**
 * @brief Creates a container for text icons using vector images.
 * @remark errno will be set on failure.
 * @param allocator The allocator for the text icons. This must support freeing memory.
 * @param resourceManager The resource manager to create graphics resources with.
 * @param shaders The vector shaders to draw icons with. This must remain alive at least as long as
 *     the vector text icons.
 * @param codepointRanges The ranges of codepoints this will cover. These codepoints will be
 *     reserved for icons, but not every codepont must be added as an icon. At least one range must
 *     be provided.
 * @param codepointRangeCount The number of codepoint ranges.
 * @param maxIcons The maximum number of icons that will be stored.
 * @return The text icons or NULL if an error occurred.
 */
DS_VECTORDRAW_EXPORT dsTextIcons* dsVectorTextIcons_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsVectorShaders* shaders,
	const dsIndexRange* codepointRanges, uint32_t codepointRangeCount, uint32_t maxIcons);

/**
 * @brief Adds an icon to vector text icons.
 * @remark errno will be set on failure.
 * @param icons The text icons to add the icon to.
 * @param codepoint The character code used to assign the icon to.
 * @param advance The amount to advance to the next glyph, typically normalized in the range (0, 1].
 * @param bounds The normalized bounds for the icon, typically in the range [0, 1].
 * @param image The vector image for the icon.
 * @param takeOwnership Whether to take ownership of the image. If true, the image will be
 *     destroyed when the text icons is destroyed or if adding the icon fails.
 * @return False if the icon couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorTextIcons_addIcon(dsTextIcons* icons, uint32_t codepoint,
	float advance, const dsAlignedBox2f* bounds, dsVectorImage* image, bool takeOwnership);

/**
 * @brief Replaces an icon on vector text icons.
 *
 * The advance and bounds may not be changed to keep layouts compatible.
 *
 * @remark errno will be set on failure.
 * @param icons The text icons to add the icon to.
 * @param codepoint The character code used to assign the icon to.
 * @param image The image for the icon.
 * @param takeOwnership Whether to take ownership of the image. If true, the image will be
 *     destroyed when the text icons is destroyed or if adding the icon fails.
 * @return False if the icon couldn't be added.
 */
DS_VECTORDRAW_EXPORT bool dsVectorTextIcons_replaceIcon(dsTextIcons* icons, uint32_t codepoint,
	dsVectorImage* image, bool takeOwnership);

/**
 * @brief Gets the image for an icon glyph used within vector text icons.
 *
 * This will properly remove the extra bit added to the user data to track whether this has
 * ownership.
 *
 * @remark errno will be set on failure.
 * @param icon The icon returned from dsTextIcons_findIcon().
 * @return The image for the icon.
 */
DS_VECTORDRAW_EXPORT dsVectorImage* dsVectorTextIcons_getIconImage(const dsIconGlyph* icon);

#ifdef __cplusplus
}
#endif

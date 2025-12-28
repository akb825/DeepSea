/*
 * Copyright 2025 Aaron Barany
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
 * @brief Functions for creating and manipulating text icons.
 * @see dsTextIcons
 */

/**
 * @brief Gets base size of the text icons structure.
 * @return The base size of the structure.
 */
DS_TEXT_EXPORT size_t dsTextIcons_sizeof(void);

/**
 * @brief Calculates the full allocation size of text icons.
 * @param codepointRangeCount The number of codepoint ranges.
 * @param maxIcons The maximum icons to store.
 * @return The full allocation size for the text icons.
 */
DS_TEXT_EXPORT size_t dsTextIcons_fullAllocSize(uint32_t codepointRangeCount, uint32_t maxIcons);

/**
 * @brief Creates a container for text icons.
 * @remark errno will be set on failure.
 * @param allocator The allocator for the text icons.
 * @param codepointRanges The ranges of codepoints this will cover. These codepoints will be
 *     reserved for icons, but not every codepont must be added as an icon. At least one range must
 *     be provided.
 * @param codepointRangeCount The number of codepoint ranges.
 * @param maxIcons The maximum number of icons that will be stored.
 * @param userData Common user data for text icons. This will be destroyed with destroyUserDataFunc
 *     if creation fails.
 * @param destroyUserDataFunc Function to destroy the common user data.
 * @param prepareFunc The function to prepare glyphs for drawing.
 * @param drawFunc The function to draw icon glyphs. This function is required.
 * @param destroyGlyphUserDataFunc Function to destroy glyph user data.
 * @return The text icons or NULL if an error occurred.
 */
DS_TEXT_EXPORT dsTextIcons* dsTextIcons_create(dsAllocator* allocator,
	const dsIndexRange* codepointRanges, uint32_t codepointRangeCount, uint32_t maxIcons,
	void* userData, dsDestroyUserDataFunction destroyUserDataFunc,
	dsPrepareDrawTextIconsFunction prepareFunc, dsPrepareDrawTextIconsFunction drawFunc,
	dsDestroyUserDataFunction destroyGlyphUserDataFunc);

/**
 * @brief Gets the allocator for text icons.
 * @remark errno will be set on failure.
 * @param icons The text icons.
 * @return The allocator for the icons or NULL if icons is NULL.
 */
DS_TEXT_EXPORT dsAllocator* dsTextIcons_getAllocator(const dsTextIcons* icons);

/**
 * @brief Checks whether a codepoint is vallid to be used with the icons.
 * @param icons The text icons.
 * @param codepoint The codepoint to check.
 * @return Whether the codepoint is valid for use with the icons.
 */
DS_TEXT_EXPORT bool dsTextIcons_isCodepointValid(const dsTextIcons* icons, uint32_t codepoint);

/**
 * @brief Adds an icon to text icons.
 * @remark errno will be set on failure.
 * @param icons The text icons to add the icon to.
 * @param codepoint The character code used to assign the icon to.
 * @param advance The amount to advance to the next glyph, typically normalized in the range (0, 1].
 * @param bounds The normalized bounds for the icon, typically in the range [0, 1].
 * @param userData The user data associated with the icon. This will be destroyed when icons is
 *     destroyed according to destroyGlyphUserDataFunc, or immediately if adding the icon fails.
 * @return False if the icon couldn't be added.
 */
DS_TEXT_EXPORT bool dsTextIcons_addIcon(dsTextIcons* icons, uint32_t codepoint, float advance,
	const dsAlignedBox2f* bounds, void* userData);

/**
 * @brief Finds the icon for a character.
 * @remark errno will be set on failure.
 * @param icons The text icons to find the icon in.
 * @param codepoint The character code to search for.
 * @return The found icon glyph or NULL if the icon couldn't be found.
 */
const dsIconGlyph* dsTextIcons_findIcon(const dsTextIcons* icons, uint32_t codepoint);

/**
 * @brief Destroys text icons.
 * @param icons The text icons to destroy.
 */
DS_TEXT_EXPORT void dsTextIcons_destroy(dsTextIcons* icons);

#ifdef __cplusplus
}
#endif

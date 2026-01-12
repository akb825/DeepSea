/*
 * Copyright 2025-2026 Aaron Barany
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
 * @brief Functions for creating and manipulating texture text icons.
 *
 * This provides the functions to dsTextIcons to be able to use textures to display the icons.
 *
 * @see dsTextIcons
 */

/**
 * @brief Name for the texture shader variable for texture text icons.
 */
DS_TEXT_EXPORT extern const char* const dsTextureTextIcons_textureName;

/**
 * @brief Name for the icon data shader variable for texture text icons.
 */
DS_TEXT_EXPORT extern const char* const dsTextureTextIcons_iconDataName;

/**
 * @brief Creates the shader variable group description used to describe the variables for text
 * icons.
 * @remark This should be shared among all dsTextureTextIconData instances.
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager.
 * @param allocator The allocator to create the shader variable group with. If NULL, the allocator
 *     from resourceManager.
 * @return The shader variable group description or NULL if an error occurred.
 */
DS_TEXT_EXPORT dsShaderVariableGroupDesc* dsTextureTextIcons_createShaderVariableGroupDesc(
	dsResourceManager* resourceManager, dsAllocator* allocator);

/**
 * @brief Checks whether or not a shader variable group is compatible with dsTextureTextIconData.
 * @param iconDataDesc The shader variable group for the icon data.
 * @return Whether or not iconData is compatible.
 */
DS_TEXT_EXPORT bool dsTextureTextIcons_isShaderVariableGroupCompatible(
	const dsShaderVariableGroupDesc* iconDataDesc);

/**
 * @brief Creates a container for text icons using textures.
 * @remark errno will be set on failure.
 * @param allocator The allocator for the text icons. This must support freeing memory.
 * @param resourceManager The resource manager to create graphics resources with.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, allocator
 *     will be used.
 * @param shader The shader to draw the icons with. The input vertex data should have 2D positions
 *     and 2D texture coordinates. This must remain alive at least as long as the texture text
 *     icons.
 * @param material The material to draw the icons with. This must remain alive at least as long as
 *     the texture text icons.
 * @param iconDataDesc The shader variable group description created from
 *     dsTextureTextIcons_createShaderVariableGroupDesc(). This must remain alive at least as
 *     long as the texture text icons.
 * @param codepointRanges The ranges of codepoints this will cover. These codepoints will be
 *     reserved for icons, but not every codepont must be added as an icon. At least one range must
 *     be provided.
 * @param codepointRangeCount The number of codepoint ranges.
 * @param maxIcons The maximum number of icons that will be stored.
 * @return The text icons or NULL if an error occurred.
 */
DS_TEXT_EXPORT dsTextIcons* dsTextureTextIcons_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, dsAllocator* resourceAllocator, dsShader* shader,
	dsMaterial* material, const dsShaderVariableGroupDesc* iconDataDesc,
	const dsIndexRange* codepointRanges, uint32_t codepointRangeCount, uint32_t maxIcons);

/**
 * @brief Adds an icon to texture text icons.
 * @remark errno will be set on failure.
 * @param icons The text icons to add the icon to.
 * @param codepoint The character code used to assign the icon to.
 * @param advance The amount to advance to the next glyph, typically normalized in the range (0, 1].
 * @param bounds The normalized bounds for the icon, typically in the range [0, 1].
 * @param texture The texture for the icon.
 * @param takeOwnership Whether to take ownership of the texture. If true, the texture will be
 *     destroyed when the text icons is destroyed or if adding the icon fails.
 * @return False if the icon couldn't be added.
 */
DS_TEXT_EXPORT bool dsTextureTextIcons_addIcon(dsTextIcons* icons, uint32_t codepoint,
	float advance, const dsAlignedBox2f* bounds, dsTexture* texture, bool takeOwnership);

/**
 * @brief Replaces an icon on texture text icons.
 *
 * The advance and bounds may not be changed to keep layouts compatible.
 *
 * @remark errno will be set on failure.
 * @param icons The text icons to add the icon to.
 * @param codepoint The character code used to assign the icon to.
 * @param texture The texture for the icon.
 * @param takeOwnership Whether to take ownership of the texture. If true, the texture will be
 *     destroyed when the text icons is destroyed or if adding the icon fails.
 * @return False if the icon couldn't be added.
 */
DS_TEXT_EXPORT bool dsTextureTextIcons_replaceIcon(dsTextIcons* icons, uint32_t codepoint,
	dsTexture* texture, bool takeOwnership);

/**
 * @brief Gets the texture for an icon glyph used within texture text icons.
 *
 * This will properly remove the extra bit added to the user data to track whether this has
 * ownership.
 *
 * @remark errno will be set on failure.
 * @param icon The icon returned from dsTextIcons_findIcon().
 * @return The texture for the icon.
 */
DS_TEXT_EXPORT dsTexture* dsTextureTextIcons_getIconTexture(const dsIconGlyph* icon);

#ifdef __cplusplus
}
#endif

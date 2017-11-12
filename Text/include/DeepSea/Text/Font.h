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

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating fonts.
 *
 * @see dsFont
 */

/**
 * @brief Creates a font.
 * @remark errno will be set on failure.
 * @param group The face group to get the faces from.
 * @param resourceManager The resource manager to create the graphics resources.
 * @param allocator The allocator for the font. If NULL, it will use the same allocator as the font
 *     group.
 * @param faceNames The names of the faces to use from the face group. When a glyph is requested, it
 *     will use the first face that contains that glyph. This way multiple faces may be used
 *     for different language sets.
 * @param faceCount The number of faces in faceNames.
 * @return The created font, or NULL if the font couldn't be created.
 */
DS_TEXT_EXPORT dsFont* dsFont_create(dsFaceGroup* group, dsResourceManager* resourceManager,
	dsAllocator* allocator, const char** faceNames, uint32_t faceCount);

/**
 * @brief Gets the face group for a font.
 * @param font The font.
 * @return The face group.
 */
DS_TEXT_EXPORT const dsFaceGroup* dsFont_getFaceGroup(const dsFont* font);

/**
 * @brief Gets the number of faces in the font.
 * @param font The font.
 * @return The number of faces.
 */
DS_TEXT_EXPORT uint32_t dsFont_getFaceCount(const dsFont* font);

/**
 * @brief Gets the name for a face within the font.
 * @param font The font.
 * @param face The index of the font.
 * @return The name of the face.
 */
DS_TEXT_EXPORT const char* dsFont_getFaceName(const dsFont* font, uint32_t face);

/**
 * @brief Gets the texture for the font.
 * @param font The font.
 * @return The texture.
 */
DS_TEXT_EXPORT dsTexture* dsFont_getTexture(dsFont* font);

/**
 * @brief Destroyes the font.
 * @remark errno will be set on failure.
 * @param font The font o destroy.
 * @return False if the font couldn't be destroyed.
 */
DS_TEXT_EXPORT bool dsFont_destroy(dsFont* font);

#ifdef __cplusplus
}
#endif

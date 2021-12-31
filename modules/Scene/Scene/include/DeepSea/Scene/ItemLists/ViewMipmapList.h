/*
 * Copyright 2021 Aaron Barany
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
#include <DeepSea/Scene/ItemLists/Types.h>
#include <DeepSea/Scene/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating view mipmap lists.
 *
 * View mipmap lists are created with a list of names of textures stored in the view's globalValues.
 * Each texture, if found, will have mipmaps generated by calling dsTexture_generateMipmaps(). This
 * is an item list type to fit into the scene layout, though it doesn't use any items from the
 * scene.
 */

/**
 * @brief The view mipmap list type name.
 */
DS_SCENE_EXPORT extern const char* const dsViewMipmapList_typeName;

/**
 * @brief Gets the type of a view mipmap list.
 * @return The type of a view mipmap list.
 */
DS_SCENE_EXPORT dsSceneItemListType dsViewMipmapList_type(void);

/**
 * @brief Creates a view mipmap list.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the view mipmap list with.
 * @param name The name of the view mipmap list. This will be copied.
 * @param textureNames The names of the textures to create mipmaps for. The necessary parts will be
 *     copied.
 * @param textureCount The number of names in textureNames.
 * @return The view mipmap list or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneItemList* dsViewMipmapList_create(dsAllocator* allocator,
	const char* name, const char* const* textureNames, uint32_t textureCount);

#ifdef __cplusplus
}
#endif
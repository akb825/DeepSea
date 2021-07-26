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
#include <DeepSea/Scene/Types.h>
#include <DeepSea/SceneLighting/Types.h>
#include <DeepSea/SceneLighting/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating shadow cull lists.
 *
 * This will cull for a surface within a scene light shadows instance. The item data is treated as a
 * bool value for whether or not the item is out of view. In other words, check if the void* value
 * is zero if it's in view or non-zero for out of view.
 */

/**
 * @brief The shadow cull list type name.
 */
DS_SCENELIGHTING_EXPORT extern const char* const dsShadowCullList_typeName;

/**
 * @brief Creates a shadow cull list.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the list with. This must support freeing memory.
 * @param name The name of the shadow cull list. This will be copied.
 * @param shadows The scene light shadows to cull for.
 * @param surface The surface index to cull for. This must be less than
 *     DS_MAX_SCENE_LIGHT_SHADOWS_SURFACES.
 * @return The shadow cull list or NULL if the parameters are invalid.
 */
DS_SCENELIGHTING_EXPORT dsSceneItemList* dsShadowCullList_create(dsAllocator* allocator,
	const char* name, dsSceneLightShadows* shadows, uint32_t surface);

#ifdef __cplusplus
}
#endif

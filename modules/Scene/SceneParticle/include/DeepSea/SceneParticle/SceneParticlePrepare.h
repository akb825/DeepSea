/*
 * Copyright 2022-2025 Aaron Barany
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
#include <DeepSea/SceneParticle/Export.h>
#include <DeepSea/SceneParticle/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene particle prepares.
 *
 * This is responsible for creating the particle emitters for each unique location a particle node
 * is in the scene graph and prepares it for rendering.
 */

/**
 * @brief The scene particle prepare list type name.
 */
DS_SCENEPARTICLE_EXPORT extern const char* const dsSceneParticlePrepare_typeName;

/**
 * @brief Gets the type of a scene particle prepare list.
 * @return The type of a scene particle prepare list.
 */
DS_SCENEPARTICLE_EXPORT const dsSceneItemListType* dsSceneParticlePrepare_type(void);

/**
 * @brief Creates a scene particle prepare list.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the list with. This must support freeing memory.
 * @param name The name of the particle prepare list. This will be copied.
 * @return The particle prepare list or NULL if an error occurred.
 */
DS_SCENEPARTICLE_EXPORT dsSceneItemList* dsSceneParticlePrepare_create(dsAllocator* allocator,
	const char* name);

#ifdef __cplusplus
}
#endif

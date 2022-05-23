/*
 * Copyright 2022 Aaron Barany
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
 * @brief Functions for creating and manipulating scene particle prepare lists.
 *
 * This will prepare dsSceneParticleNode node types for drawing.
 */

/**
 * @brief The scene particle prepare list type name.
 */
DS_SCENEPARTICLE_EXPORT extern const char* const dsSceneParticlePrepareList_typeName;

/**
 * @brief Gets the type of a scene particle prepare list.
 * @return The type of a scene particle prepare list.
 */
DS_SCENEPARTICLE_EXPORT dsSceneItemListType dsSceneParticlePrepareList_type(void);

/**
 * @brief Creates a scene particle prepare list.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the list with. This must support freeing memory.
 * @param name The name of the particle prepare list. This will be copied.
 * @param particleDraws The particle draws that will be used to draw the particle emitters managed
 *    by the particle prepare list. This array will be copied.
 * @param particleDrawCount The number of particle draws.
 * @return The particle prepare list or NULL if an error occurred.
 */
DS_SCENEPARTICLE_EXPORT dsSceneItemList* dsSceneParticlePrepareList_create(dsAllocator* allocator,
	const char* name, dsParticleDraw* const* particleDraws, uint32_t particleDrawCount);

#ifdef __cplusplus
}
#endif

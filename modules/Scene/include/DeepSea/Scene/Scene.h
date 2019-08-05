/*
 * Copyright 2019 Aaron Barany
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
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scenes.
 * @see dsScene
 */

/**
 * @brief Creates a scene.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene with. This must support freeing memory.
 * @param sharedItems The item lists to run before the rest of the scene. (e.g. cull item lists)
 *     This will copy the array itself and take ownership of the objects. If creation fails, this
 *     means it will immediately destroy all objects in this list.
 * @param sharedItemCount The number of shared items.
 * @param pipeline The pipeline to perform when rendering the scene. This will copy the array itself
 *     and take ownership of the objects, i.e. render passes and draw item lists. If creation fails,
 *     this means it will immediately destroy all objects in this list.
 * @param pipelineCount The number of pipeline items.
 * @param globalData The list of global data isntances used within the scene. This will copy the
 *     array itself and take ownership of the objects. If creation fails, this means it will
 *     immediately destroy all objects in this list.
 * @param globalDataCount The number of global data instances.
 * @return The scene or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsScene* dsScene_create(dsAllocator* allocator,
	dsSceneItemList* const* sharedItems, uint32_t sharedItemCount,
	const dsScenePipelineItem* pipeline, uint32_t pipelineCount,
	dsSceneGlobalData* const* globalData, uint32_t globalDataCount);

/**
 * @brief Updates dirty nodes within the scene.
 * @remark errno will be set on failure.
 * @param scene The scene to update.
 * @return False if the parameters are invalid.
 */
DS_SCENE_EXPORT bool dsScene_update(dsScene* scene);

/**
 * @brief Destroys a scene.
 * @param scene The scene to destroy.
 */
DS_SCENE_EXPORT void dsScene_destroy(dsScene* scene);

#ifdef __cplusplus
}
#endif

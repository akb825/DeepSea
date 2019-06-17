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
#include <DeepSea/Core/Types.h>
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
 * @param allocator The allocator to create the scene with.
 * @param pipeline The pipeline to perform when rendering the scene. This will perform a deep copy
 *     of the arrays and will take ownership of the objects, i.e. render passes and draw item lists.
 *     If creation fails, this means it will immediately destroy all objects in this list.
 * @param pipelineCount The number of pipeline items.
 * @param stringPool An optional string pool for pooled string allocation. This will take
 *     ownership of the strings pointer within stringPool, which will also be freed if creation
 *     failed. This may be NULL if no string pool is used.
 * @return The scene or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsScene* dsScene_create(dsAllocator* allocator, const dsScenePipelineItem* pipeline,
	uint32_t pipelineCount, const dsStringPool* stringPool);

#ifdef __cplusplus
}
#endif

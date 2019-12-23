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
 * @brief Functions for creating and manipulating scene load scratch data.
 * @see dsSceneLoadScratchData
 */

/**
 * @brief Creates load scratch data.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scratch data with. This must support freeing memory.
 * @return The load scratch data.
 */
DS_SCENE_EXPORT dsSceneLoadScratchData* dsSceneLoadScratchData_create(dsAllocator* allocator);

/**
 * @brief Allocates scratch data.
 * @remark errno will be set on failure.
 * @param scratchData The scratch data to allocate from.
 * @param size The size to allocator.
 * @return The allocated data, or NULL if allocation failed.
 */
DS_SCENE_EXPORT void* dsSceneLoadScratchData_allocate(dsSceneLoadScratchData* scratchData,
	uint32_t size);

/**
 * @brief Pops allocated data from the scratch data.
 *
 * This deallocates the last allocated memory.
 *
 * @remark errno will be set on failure.
 * @param scratchData The scratch data to pop from.
 * @param size The number of bytes from the previous allocation to remove.
 * @return False if the parameters are invalide.
 */
DS_SCENE_EXPORT bool dsSceneLoadScratchData_popData(dsSceneLoadScratchData* scratchData,
	uint32_t size);

/**
 * @brief Destroys load scratch data.
 * @param scratchData The scratch data to destroy.
 */
DS_SCENE_EXPORT void dsSceneLoadScratchData_destroy(dsSceneLoadScratchData* scratchData);

#ifdef __cplusplus
}
#endif

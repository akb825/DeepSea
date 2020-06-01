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
 * @brief Functions for creating and manipulating scene global data.
 * @see dsSceneGlobalData
 */

/**
 * @brief Loads a scene global data from a flatbuffer data buffer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene nodes.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the global data allocator.
 * @param loadContext The scene load context.
 * @param scratchData The scene scratch data.
 * @param type The type name of the node to load.
 * @param data The data for the global data. The data isn't used after this call.
 * @param size The size of the data buffer.
 * @return The global data, or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneGlobalData* dsSceneGlobalData_load(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, const char* type, const void* data, size_t size);

/**
 * @brief Destroys a scene global data object.
 * @remark errno will be set on failure.
 * @param globalData The global data to destroy.
 * @return False if the global data couldn't be destroyed.
 */
DS_SCENE_EXPORT bool dsSceneGlobalData_destroy(dsSceneGlobalData* globalData);

#ifdef __cplusplus
}
#endif


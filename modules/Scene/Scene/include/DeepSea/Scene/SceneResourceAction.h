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
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function for loading a scene resource action.
 */


/**
 * @brief Loads a scene resource action from a flatbuffer data buffer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create data with.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the data allocator.
 * @param loadContext The scene load context.
 * @param scratchData The scene scratch data.
 * @param type The type name of the resource action to load.
 * @param data The data for the resource action. The data isn't used after this call.
 * @param size The size of the data buffer.
 * @param relativePathUserData User data to manage opening of relative paths.
 * @param openRelativePathStreamFunc Function to open streams for relative paths.
 * @param closeRelativePathStreamFunc Function to close streams for relative paths.
 * @return False if loading failed.
 */
DS_SCENE_EXPORT bool dsSceneResourceAction_load(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, const char* type, const void* data, size_t size,
	void* relativePathUserData,
	dsOpenSceneResourcesRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseSceneResourcesRelativePathStreamFunction closeRelativePathStreamFunc);

#ifdef __cplusplus
}
#endif

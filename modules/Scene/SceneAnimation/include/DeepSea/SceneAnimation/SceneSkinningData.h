/*
 * Copyright 2023 Aaron Barany
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
#include <DeepSea/SceneAnimation/Export.h>
#include <DeepSea/SceneAnimation/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create scene skinning data.
 */

/**
 * @brief The scene skinning data type name.
 */
DS_SCENEANIMATION_EXPORT extern const char* const dsSceneSkinningData_typeName;

/**
 * @brief Checks whether buffers will be used for skinning.
 * @param resourceManager The resource manager.
 * @return True if buffers will be used, false if textures will be used as a fallback.s
 */
DS_SCENEANIMATION_EXPORT bool dsSceneSkinningData_useBuffers(dsResourceManager* resourceManager);

/**
 * @brief Creates a scene skinning data to use with a dsSceneItemList.
 * @param allocator The allocator to create the skinning data with. This must support freeing
 *     memory.
 * @param resourceAllocator The allocator for graphics resources. Defaults to allocator if NULL.
 * @param resourceManager The resource manager.
 * @return The scene skinning data or NULL if an error occurred.
 */
DS_SCENEANIMATION_EXPORT dsSceneInstanceData* dsSceneSkinningData_create(dsAllocator* allocator,
	dsAllocator* resourceAllocator, dsResourceManager* resourceManager);

#ifdef __cplusplus
}
#endif

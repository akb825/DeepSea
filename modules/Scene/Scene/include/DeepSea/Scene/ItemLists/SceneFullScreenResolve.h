/*
 * Copyright 2020 Aaron Barany
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
 * @brief Functions for creating and manipulating full screen resolves.
 * @see dsSceneFullScreenResolve
 */

/**
 * @brief The scene full screen resolve type name.
 */
DS_SCENE_EXPORT extern const char* const dsSceneFullScreenResolve_typeName;

/**
 * @brief Creates a full screen resolve.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the list with.
 * @param name The name of the full screen resolve. This will be copied.
 * @param resourceManager The resource manager to create graphics resources from.
 * @param resourceAllocator The allocator to create graphics resources with. If NULL, it will use
 *     the full screen resolve allocator.
 * @param shader The shader to draw with.
 * @param material The material to draw with.
 * @param renderStates The render states to use, or NULL if no special render states are needed.
 * @return The full screen resolve or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneItemList* dsSceneFullScreenResolve_create(dsAllocator* allocator,
	const char* name, dsResourceManager* resourceManager, dsAllocator* resourceAllocator,
	dsShader* shader, dsMaterial* material, const dsDynamicRenderStates* renderStates);

/**
 * @brief Gets the render states for a full screen resolve.
 * @param resolve The full screen resolve.
 * @return The render states or NULL if no special render states are used.
 */
DS_SCENE_EXPORT const dsDynamicRenderStates* dsSceneFullScreenResolve_getRenderStates(
	const dsSceneFullScreenResolve* resolve);

/**
 * @brief Sets the render states for a full screen resolve.
 * @param resolve The full screen resolve.
 * @param renderStates The render states or NULL if no special render states are needed.
 */
DS_SCENE_EXPORT void dsSceneFullScreenResolve_setRenderStates(dsSceneFullScreenResolve* resolve,
	const dsDynamicRenderStates* renderStates);

/**
 * @brief Destroys a full screen resolve.
 * @param resolve The full screen resolve.
 */
DS_SCENE_EXPORT void dsSceneFullScreenResolve_destroy(dsSceneFullScreenResolve* resolve);

#ifdef __cplusplus
}
#endif

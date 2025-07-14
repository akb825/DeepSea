/*
 * Copyright 2020-2025 Aaron Barany
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
 * @brief The full screen resolve type name.
 */
DS_SCENE_EXPORT extern const char* const dsSceneFullScreenResolve_typeName;

/**
 * @brief Gets the type of a full screen resolve.
 * @return The type of a full screen resolve.
 */
DS_SCENE_EXPORT const dsSceneItemListType* dsSceneFullScreenResolve_type(void);

/**
 * @brief Creates the draw geometry for a full screen resolve.
 *
 * This contains 4 vertices in the range [-1, 1] o be drawn with a triangle strip. This will be
 * created with the first call to dsSceneFullScreenResolve_createDrawGeometry() and destroyed with
 * the last matching call to dsSceneFullScreenResolve_destroyGeometry().
 *
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager to create the draw geometry with.
 * @return The draw geometry or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsDrawGeometry* dsSceneFullScreenResolve_createGeometry(
	dsResourceManager* resourceManager);

/**
 * @brief Destroys the draw geometry for a matching call to
 * dsSceneFullScreenResolve_createGeometry().
 */
DS_SCENE_EXPORT void dsSceneFullScreenResolve_destroyGeometry(void);

/**
 * @brief Creates a full screen resolve.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the list with.
 * @param name The name of the full screen resolve. This will be copied.
 * @param resourceManager The resource manager to create graphics resources from.
 * @param shader The shader to draw with.
 * @param material The material to draw with.
 * @param renderStates The render states to use, or NULL if no special render states are needed.
 * @return The full screen resolve or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneFullScreenResolve* dsSceneFullScreenResolve_create(dsAllocator* allocator,
	const char* name, dsResourceManager* resourceManager, dsShader* shader, dsMaterial* material,
	const dsDynamicRenderStates* renderStates);

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

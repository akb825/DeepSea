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
 * @brief Functions for creating and manipulating scene animation lists.
 *
 * This is responsible for creating the per-instance data for dsAnimationNode and updating and
 * applying animations.
 *
 * @see dsSceneAnimationList
 */

/**
 * @brief The scene animation list type name.
 */
DS_SCENEANIMATION_EXPORT extern const char* const dsSceneAnimationList_typeName;

/**
 * @brief Gets the type of a scene animation list.
 * @return The type of a scene animation list.
 */
DS_SCENEANIMATION_EXPORT dsSceneItemListType dsSceneAnimationList_type(void);

/**
 * @brief Creates a scene animation list.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the list with. This must support freeing memory.
 * @param name The name of the scene animation list. This will be copied.
 * @return The scene animation list or NULL if an error occurred.
 */
DS_SCENEANIMATION_EXPORT dsSceneAnimationList* dsSceneAnimationList_create(
	dsAllocator* allocator, const char* name);

/**
 * @brief Updates the ragdolls within a scene animation list.
 *
 * This should typically be hooked up to run after the system that updates the ragdoll nodes,
 * typically a physics system. This is usually done after the animation list itself has been updated
 * so non-ragdoll animations can contribute to the ragdoll updates.
 *
 * @remark errno will be set on failure.
 * @param animationList The scene animation list.
 * @return False if the animation list ragdolls couldn't be updated.
 */
DS_SCENEANIMATION_EXPORT bool dsSceneAnimationList_updateRagdolls(
	dsSceneAnimationList* animationList);

#ifdef __cplusplus
}
#endif

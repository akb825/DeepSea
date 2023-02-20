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
 * @brief Functions for creating and manipulating scene animations.
 * @see dsSceneAnimation
 */

/**
 * @brief Creates a scene animation.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene animation with.
 * @param sceneAnimationTree The scene animation tree.
 * @return The scene animation or NULL if an error occurred.
 */
DS_SCENEANIMATION_EXPORT dsSceneAnimation* dsSceneAnimation_create(dsAllocator* allocator,
	dsSceneAnimationTree* sceneAnimationTree);

/**
 * @brief Adds a keyframe animation to the animation.
 * @remark errno will be set on failure.
 * @param animation The animation.
 * @param keyframeAnimation They keyframe animation to add.
 * @param weight The weight of the keyframe animation.
 * @param time The initial time of the entry.
 * @param timeScale The scale to apply to the time when incrementing the entry time.
 * @param wrap Whether to wrap or clamp the time when evaluating the animation.
 * @return False if an error occurred.
 */
DS_SCENEANIMATION_EXPORT bool dsSceneAnimation_addKeyframeAnimation(dsSceneAnimation* animation,
	const dsKeyframeAnimation* keyframeAnimation, float weight, double time, double timeScale,
	bool wrap);

/**
 * @brief Finds a keyframe animation entry in an animation.
 * @param animation The animation.
 * @param keyframeAnimation The keyframe animation to find the entry for.
 * @return The entry for the keyframe animation or NULL if not found. This may be invalidated if a
 *     keyframe animation is added or removed.
 */
DS_SCENEANIMATION_EXPORT dsKeyframeAnimationEntry* dsSceneAnimation_findKeyframeAnimationEntry(
	dsSceneAnimation* animation, const dsKeyframeAnimation* keyframeAnimation);

/**
 * @brief Removes a keyframe animation from the animation.
 * @param animation The animation.
 * @param keyframeAnimation The keyframe animation.
 * @return False if the animation wasn't found.
 */
DS_SCENEANIMATION_EXPORT bool dsSceneAnimation_removeKeyframeAnimation(dsSceneAnimation* animation,
	const dsKeyframeAnimation* keyframeAnimation);

/**
 * @brief Adds a direct animation to the animation.
 * @remark errno will be set on failure.
 * @param animation The animation.
 * @param directAnimation They direct animation to add.
 * @param weight The weight of the direct animation.
 * @return False if an error occurred.
 */
DS_SCENEANIMATION_EXPORT bool dsSceneAnimation_addDirectAnimation(dsSceneAnimation* animation,
	const dsDirectAnimation* directAnimation, float weight);

/**
 * @brief Finds a direct animation entry in an animation.
 * @param animation The animation.
 * @param directAnimation The direct animation to find the entry for.
 * @return The entry for the direct animation or NULL if not found. This may be invalidated if a
 *     direct animation is added or removed.
 */
DS_SCENEANIMATION_EXPORT dsDirectAnimationEntry* dsSceneAnimation_findDirectAnimationEntry(
	dsSceneAnimation* animation, const dsDirectAnimation* directAnimation);

/**
 * @brief Removes a direct animation from the animation.
 * @param animation The animation.
 * @param directAnimation The direct animation.
 * @return False if the animation wasn't found.
 */
DS_SCENEANIMATION_EXPORT bool dsSceneAnimation_removeDirectAnimation(dsSceneAnimation* animation,
	const dsDirectAnimation* directAnimation);

/**
 * @brief Destroys a direct animation.
 * @param animation The animation to destroy.
 */
DS_SCENEANIMATION_EXPORT void dsSceneAnimation_destroy(dsSceneAnimation* animation);


#ifdef __cplusplus
}
#endif

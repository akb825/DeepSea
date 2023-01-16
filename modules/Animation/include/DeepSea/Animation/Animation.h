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
#include <DeepSea/Animation/Export.h>
#include <DeepSea/Animation/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating animations.
 * @see dsKeyframeAnimationEntry
 * @see dsDirectionAnimationEntry
 * @see dsAnimation
 */

/**
 * @brief Creates an animation.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the animation with. The allocator must support freeing
 *     memory.
 * @param treeID The ID of the animation trees the animation is compatible with.
 * @return The animation or NULL if an error occurred.
 */
DS_ANIMATION_EXPORT dsAnimation* dsAnimation_create(dsAllocator* allocator, uint32_t treeID);

/**
 * @brief Adds a keyframe animation to the animation.
 * @remark errno will be set on failure.
 * @param animation The animation.
 * @param keyframeAnimation They keyframe animation to add.
 * @param map The mapping from the keyframe animation to animation tree nodes.
 * @param weight The weight of the keyframe animation.
 * @param time The initial time of the entry.
 * @param timeScale The scale to apply to the time when incrementing the entry time.
 * @param wrap Whether to wrap or clamp the time when evaluating the animation.
 * @return False if an error occurred.
 */
DS_ANIMATION_EXPORT bool dsAnimation_addKeyframeAnimation(dsAnimation* animation,
	const dsKeyframeAnimation* keyframeAnimation, const dsKeyframeAnimationNodeMap* map,
	float weight, double time, double timeScale, bool wrap);

/**
 * @brief Finds a keyframe animation entry in an animation.
 * @param animation The animation.
 * @param keyframeAnimation The keyframe animation to find the entry for.
 * @return The entry for the keyframe animation or NULL if not found. This may be invalidated if a
 *     keyframe animation is added or removed.
 */
DS_ANIMATION_EXPORT dsKeyframeAnimationEntry* dsAnimation_findKeyframeAnimationEntry(
	dsAnimation* animation, const dsKeyframeAnimation* keyframeAnimation);

/**
 * @brief Removes a keyframe animation from the animation.
 * @param animation The animation.
 * @param keyframeAnimation The keyframe animation.
 * @return False if the animation wasn't found.
 */
DS_ANIMATION_EXPORT bool dsAnimation_removeKeyframeAnimation(dsAnimation* animation,
	const dsKeyframeAnimation* keyframeAnimation);

/**
 * @brief Adds a direct animation to the animation.
 * @remark errno will be set on failure.
 * @param animation The animation.
 * @param directAnimation They direct animation to add.
 * @param map The mapping from the direct animation to animation tree nodes.
 * @param weight The weight of the direct animation.
 * @return False if an error occurred.
 */
DS_ANIMATION_EXPORT bool dsAnimation_addDirectAnimation(dsAnimation* animation,
	const dsDirectAnimation* directAnimation, const dsDirectAnimationNodeMap* map, float weight);

/**
 * @brief Finds a direct animation entry in an animation.
 * @param animation The animation.
 * @param directAnimation The direct animation to find the entry for.
 * @return The entry for the direct animation or NULL if not found. This may be invalidated if a
 *     direct animation is added or removed.
 */
DS_ANIMATION_EXPORT dsDirectAnimationEntry* dsAnimation_findDirectAnimationEntry(
	dsAnimation* animation, const dsDirectAnimation* directAnimation);

/**
 * @brief Removes a direct animation from the animation.
 * @param animation The animation.
 * @param directAnimation The direct animation.
 * @return False if the animation wasn't found.
 */
DS_ANIMATION_EXPORT bool dsAnimation_removeDirectAnimation(dsAnimation* animation,
	const dsDirectAnimation* directAnimation);

/**
 * @brief Updates the animation.
 * @remark errno will be set on failure.
 * @param animation The animation.
 * @param time The time to advance keyframe animations.
 * @return False if the parameters are invalid.
 */
DS_ANIMATION_EXPORT bool dsAnimation_update(dsAnimation* animation, double time);

/**
 * @brief Applies an animation to an animation tree.
 * @remark errno will be set on failure.
 * @param animation The animation.
 * @param tree The animation tree to apply the animation to.
 * @return False if the animation couldn't be applied.
 */
DS_ANIMATION_EXPORT bool dsAnimation_apply(const dsAnimation* animation, dsAnimationTree* tree);

/**
 * @brief Destroys a direct animation.
 * @param animation The animation to destroy.
 */
DS_ANIMATION_EXPORT void dsAnimation_destroy(dsAnimation* animation);

#ifdef __cplusplus
}
#endif

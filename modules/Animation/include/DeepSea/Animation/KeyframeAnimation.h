/*
 * Copyright 2022 Aaron Barany
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
 * @brief Functions for creating and manipulating keyframe animations.
 * @see dsAnimationChannel
 * @see dsAnimationKeyframes
 * @see dsKeyframeAnimation
 */

/**
 * @brief Creates a keyframe animation.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the keyframe animation with.
 * @param keyframes The keyframes for the animation. The contents will be copied. It isn't valid
 *     to create a keyframe animation without any keyframes.
 * @param keyframesCount The number of animation keyframes objects.
 * @return The keyframe animation or NULL if an error occurred.
 */
DS_ANIMATION_EXPORT dsKeyframeAnimation* dsKeyframeAnimation_create(dsAllocator* allocator,
	const dsAnimationKeyframes* keyframes, uint32_t keyframesCount);

/**
 * @brief Loads an keyframe animation from a file.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the keyframe animation.
 * @param scratchAllocator The allocator for temporary data. If NULL, it will use the keyframe
 *     animation allocator.
 * @param filePath The file path for the keyframe animation to load.
 * @return The loaded keyframe animation or NULL if it couldn't be loaded.
 */
DS_ANIMATION_EXPORT dsKeyframeAnimation* dsKeyframeAnimation_loadFile(dsAllocator* allocator,
	dsAllocator* scratchAllocator, const char* filePath);

/**
 * @brief Loads an keyframe animation from a file.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the keyframe animation.
 * @param scratchAllocator The allocator for temporary data. If NULL, it will use the keyframe
 *     animation allocator.
 * @param type The resource type.
 * @param filePath The file path for the keyframe animation to load.
 * @return The loaded keyframe animation or NULL if it couldn't be loaded.
 */
DS_ANIMATION_EXPORT dsKeyframeAnimation* dsKeyframeAnimation_loadResource(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsFileResourceType type, const char* filePath);

/**
 * @brief Loads an keyframe animation from a stream.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the keyframe animation.
 * @param scratchAllocator The allocator for temporary data. If NULL, it will use the keyframe
 *     animation allocator.
 * @param stream The stream to load the keyframe animation from. This stream will be read from the
 *     current position until the end.
 * @return The loaded keyframe animation or NULL if it couldn't be loaded.
 */
DS_ANIMATION_EXPORT dsKeyframeAnimation* dsKeyframeAnimation_loadStream(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsStream* stream);

/**
 * @brief Loads an keyframe animation from a data buffer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the keyframe animation.
 * @param scratchAllocator The allocator for temporary data. If NULL, it will use the keyframe
 *     animation allocator.
 * @param data The data for the keyframe animation. The data isn't used after this call.
 * @param size The size of the data buffer.
 * @return The loaded keyframe animation or NULL if it couldn't be loaded.
 */
DS_ANIMATION_EXPORT dsKeyframeAnimation* dsKeyframeAnimation_loadData(dsAllocator* allocator,
	dsAllocator* scratchAllocator, const void* data, size_t size);

/**
 * @brief Destroys a keyframe animation.
 * @param animation The keyframe animation to destroy.
 */
DS_ANIMATION_EXPORT void dsKeyframeAnimation_destroy(dsKeyframeAnimation* animation);

#ifdef __cplusplus
}
#endif

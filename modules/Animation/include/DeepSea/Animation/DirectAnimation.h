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
#include <DeepSea/Animation/Export.h>
#include <DeepSea/Animation/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating direct animations.
 * @see dsAnimationChannel
 * @see dsAnimationKeyframes
 * @see dsKeyframeAnimation
 */

/**
 * @brief Creates a direct animation.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the keyframe animation with.
 * @param channels The channels for the animation. The contents will be copied. It isn't valid
 *     to create a direct animation without any channels.
 * @param channelCount The number of animation channels.
 * @return The direct animation or NULL if an error occurred.
 */
DS_ANIMATION_EXPORT dsDirectAnimation* dsDirectAnimation_create(dsAllocator* allocator,
	const dsDirectAnimationChannel* channels, uint32_t channelCount);

/**
 * @brief Loads an direct animation from a file.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the direct animation with.
 * @param scratchAllocator The allocator for temporary data. If NULL, it will use the direct
 *     animation allocator.
 * @param filePath The file path for the direct animation to load.
 * @return The loaded direct animation or NULL if it couldn't be loaded.
 */
DS_ANIMATION_EXPORT dsDirectAnimation* dsDirectAnimation_loadFile(dsAllocator* allocator,
	dsAllocator* scratchAllocator, const char* filePath);

/**
 * @brief Loads an direct animation from a resource file.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the direct animation with.
 * @param scratchAllocator The allocator for temporary data. If NULL, it will use the direct
 *     animation allocator.
 * @param type The resource type.
 * @param filePath The file path for the direct animation to load.
 * @return The loaded direct animation or NULL if it couldn't be loaded.
 */
DS_ANIMATION_EXPORT dsDirectAnimation* dsDirectAnimation_loadResource(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsFileResourceType type, const char* filePath);

/**
 * @brief Loads an direct animation from a file within an archive.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the direct animation with.
 * @param scratchAllocator The allocator for temporary data. If NULL, it will use the direct
 *     animation allocator.
 * @param archive The archive to load the direct animation from.
 * @param filePath The file path for the direct animation to load.
 * @return The loaded direct animation or NULL if it couldn't be loaded.
 */
DS_ANIMATION_EXPORT dsDirectAnimation* dsDirectAnimation_loadArchive(dsAllocator* allocator,
	dsAllocator* scratchAllocator, const dsFileArchive* archive, const char* filePath);

/**
 * @brief Loads an direct animation from a stream.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the direct animation with.
 * @param scratchAllocator The allocator for temporary data. If NULL, it will use the direct
 *     animation allocator.
 * @param stream The stream to load the direct animation from. This stream will be read from the
 *     current position until the end.
 * @return The loaded direct animation or NULL if it couldn't be loaded.
 */
DS_ANIMATION_EXPORT dsDirectAnimation* dsDirectAnimation_loadStream(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsStream* stream);

/**
 * @brief Loads an direct animation from a data buffer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the direct animation with.
 * @param scratchAllocator The allocator for temporary data. If NULL, it will use the direct
 *     animation allocator.
 * @param data The data for the direct animation. The data isn't used after this call.
 * @param size The size of the data buffer.
 * @return The loaded direct animation or NULL if it couldn't be loaded.
 */
DS_ANIMATION_EXPORT dsDirectAnimation* dsDirectAnimation_loadData(dsAllocator* allocator,
	dsAllocator* scratchAllocator, const void* data, size_t size);

/**
 * @brief Destroys a direct animation.
 * @param animation The direct animation to destroy.
 */
DS_ANIMATION_EXPORT void dsDirectAnimation_destroy(dsDirectAnimation* animation);

#ifdef __cplusplus
}
#endif

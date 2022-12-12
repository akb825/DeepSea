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
 * @brief Functions for creating and manipulating direct animation node maps.
 * @see dsDirectAnimationNodeMap
 */

/**
 * @brief Creates a direct animation node map.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the keyframe animation node map with.
 * @param animation The keyframe animation to create the mapping for.
 * @param tree The animation tree to create the mapping for. The mapping will be calid for any
 *     animation tree with the same ID.
 * @return The keyframe animation node map or NULL if an error occurred.
 */
DS_ANIMATION_EXPORT dsDirectAnimationNodeMap* dsDirectAnimationNodeMap_create(
	dsAllocator* allocator, const dsDirectAnimation* animation, const dsAnimationTree* tree);

/**
 * @brief Destroys a keyframe animation node map.
 * @param map The keyframe animation node map to destroy.
 */
DS_ANIMATION_EXPORT void dsDirectAnimationNodeMap_destroy(dsDirectAnimationNodeMap* map);

#ifdef __cplusplus
}
#endif

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
 * @brief Functions for creating and manipulating animation node map caches.
 *
 * Usage of dsAnimationNodeMapCache is thread-safe.
 *
 * @see dsAnimationNodeMapCache
 */

/**
 * @brief Creates an animation node map cache.
 * @remark errno will be set on failure.
 * @param allocator The allocator for the node map cache. This must support freeing memory.
 * @return The animation node map cache or NULL if an error occurred.
 */
DS_ANIMATION_EXPORT dsAnimationNodeMapCache* dsAnimationNodeMapCache_create(dsAllocator* allocator);

/**
 * @brief Adds an animation tree to the node map cache.
 *
 * Duplicate calls for the same animation tree may be made, the entry will be removed for the last
 * matching call to dsAnimationNodeMapCache_removeAnimationTree().
 *
 * @remark errno will be set on failure.
 * @param cache The animation node map cache to add the animation tree to.
 * @param tree The animation tree to add to the cache. This will be cloned on the first insertion
 *     of a compatible animation tree.
 */
DS_ANIMATION_EXPORT bool dsAnimationNodeMapCache_addAnimationTree(dsAnimationNodeMapCache* cache,
	dsAnimationTree* tree);

/**
 * @brief Removes an animation tree from the node map cache.
 *
 * The entry will be fully removed once all calls to dsAnimationNodeMapCache_addAnimationTree() have
 * been matched with a call to dsAnimationNodeMapCache_removeAnimationTree().
 *
 * @remark errno will be set on failure.
 * @param cache The animation node map cache to add the animation tree to.
 * @param tree The animation tree to remove from the cache.
 */
DS_ANIMATION_EXPORT bool dsAnimationNodeMapCache_removeAnimationTree(dsAnimationNodeMapCache* cache,
	dsAnimationTree* tree);

/**
 * @brief Destroys an animation node map cache.
 * @param cache The animation node map cache to destroy.
 */
DS_ANIMATION_EXPORT void dsAnimationNodeMapCache_destroy(dsAnimationNodeMapCache* cache);

#ifdef __cplusplus
}
#endif

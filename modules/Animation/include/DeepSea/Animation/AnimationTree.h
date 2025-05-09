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
 * @brief Functions for creating and manipulating animation trees.
 * @see dsAnimationBuildNode
 * @see dsAnimationJointBuildNode
 * @see dsAnimationNode
 * @see dsAnimationTree
 */

/**
 * @brief Creates an animation tree.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the animation tree with.
 * @param rootNodes The root nodes for the tree. These nodes will be flattened into the final
 *     structure. It is the responsibility of the caller to free any memory for the build nodes.
 * @param rootNodeCount The number of root nodes. It is not valid to have an animation tree with no
 *     nodes.
 * @return The animation tree or NULL if an error occurred.
 */
DS_ANIMATION_EXPORT dsAnimationTree* dsAnimationTree_create(dsAllocator* allocator,
	const dsAnimationBuildNode* const* rootNodes, uint32_t rootNodeCount);

/**
 * @brief Creates an animation tree of joints.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the animation tree with.
 * @param nodes The nodes for the tree. The indices for the nodes will be preserved.
 * @param nodeCount The number of nodes. It is not valid to have an animation tree with no nodes.
 * @return The animation tree or NULL if an error occurred.
 */
DS_ANIMATION_EXPORT dsAnimationTree* dsAnimationTree_createJoints(dsAllocator* allocator,
	const dsAnimationJointBuildNode* nodes, uint32_t nodeCount);

/**
 * @brief Loads an animation tree from a file.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the animation tree with.
 * @param scratchAllocator The allocator for temporary data. If NULL, it will use the animation tree
 *     allocator.
 * @param filePath The file path for the animation tree to load.
 * @return The loaded animation tree or NULL if it couldn't be loaded.
 */
DS_ANIMATION_EXPORT dsAnimationTree* dsAnimationTree_loadFile(dsAllocator* allocator,
	dsAllocator* scratchAllocator, const char* filePath);

/**
 * @brief Loads an animation tree from a resource file.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the animation tree with.
 * @param scratchAllocator The allocator for temporary data. If NULL, it will use the animation tree
 *     allocator.
 * @param type The resource type.
 * @param filePath The file path for the animation tree to load.
 * @return The loaded animation tree or NULL if it couldn't be loaded.
 */
DS_ANIMATION_EXPORT dsAnimationTree* dsAnimationTree_loadResource(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsFileResourceType type, const char* filePath);

/**
 * @brief Loads an animation tree from a file within an archive.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the animation tree with.
 * @param scratchAllocator The allocator for temporary data. If NULL, it will use the animation tree
 *     allocator.
 * @param archive The archive to load the animation tree from.
 * @param filePath The file path for the animation tree to load.
 * @return The loaded animation tree or NULL if it couldn't be loaded.
 */
DS_ANIMATION_EXPORT dsAnimationTree* dsAnimationTree_loadArchive(dsAllocator* allocator,
	dsAllocator* scratchAllocator, const dsFileArchive* archive, const char* filePath);

/**
 * @brief Loads an animation tree from a stream.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the animation tree with.
 * @param scratchAllocator The allocator for temporary data. If NULL, it will use the animation tree
 *     allocator.
 * @param stream The stream to load the animation tree from. This stream will be read from the
 *     current position until the end.
 * @return The loaded animation tree or NULL if it couldn't be loaded.
 */
DS_ANIMATION_EXPORT dsAnimationTree* dsAnimationTree_loadStream(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsStream* stream);

/**
 * @brief Loads an animation tree from a data buffer.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the animation tree with.
 * @param scratchAllocator The allocator for temporary data. If NULL, it will use the animation tree
 *     allocator.
 * @param data The data for the animation tree. The data isn't used after this call.
 * @param size The size of the data buffer.
 * @return The loaded animation tree or NULL if it couldn't be loaded.
 */
DS_ANIMATION_EXPORT dsAnimationTree* dsAnimationTree_loadData(dsAllocator* allocator,
	dsAllocator* scratchAllocator, const void* data, size_t size);

/**
 * @brief Clones an animation tree.
 *
 * This can create a separate animation tree that's compatible with the original tree, allowing them
 * to be animated separately.
 *
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the animation tree with.
 * @param tree The animation tree.
 * @return The cloned animation tree or NULL if an error occurred.
 */
DS_ANIMATION_EXPORT dsAnimationTree* dsAnimationTree_clone(dsAllocator* allocator,
	const dsAnimationTree* tree);

/**
 * @brief Finds an animation node by name.
 * @remark errno will be set on failure.
 * @param tree The animation tree.
 * @param name The name of the node.
 * @return The found node or NULL if not found.
 */
DS_ANIMATION_EXPORT const dsAnimationNode* dsAnimationTree_findNodeName(const dsAnimationTree* tree,
	const char* name);

/**
 * @brief Finds an animation node by name ID.
 * @remark errno will be set on failure.
 * @param tree The animation tree.
 * @param nameID The hash of the name of the node.
 * @return The found node or NULL if not found.
 */
DS_ANIMATION_EXPORT const dsAnimationNode* dsAnimationTree_findNodeID(const dsAnimationTree* tree,
	uint32_t nameID);

/**
 * @brief Finds an animation node index by name.
 * @remark errno will be set on failure.
 * @param tree The animation tree.
 * @param name The name of the node.
 * @return The found node index or DS_NO_ANIMATION_NODE if not found.
 */
DS_ANIMATION_EXPORT uint32_t dsAnimationTree_findNodeIndexName(const dsAnimationTree* tree,
	const char* name);

/**
 * @brief Finds an animation node index by name ID.
 * @remark errno will be set on failure.
 * @param tree The animation tree.
 * @param nameID The hash of the name of the node.
 * @return The found node index or DS_NO_ANIMATION_NODE if not found.
 */
DS_ANIMATION_EXPORT uint32_t dsAnimationTree_findNodeIndexID(const dsAnimationTree* tree,
	uint32_t nameID);

/**
 * @brief Updates the transforms for an animation tree.
 * @remark errno will be set on failure.
 * @param tree The animation tree.
 * @return False if tree is NULL.
 */
DS_ANIMATION_EXPORT bool dsAnimationTree_updateTransforms(dsAnimationTree* tree);

/**
 * @brief Destroys an animation tree.
 * @param tree The animation tree to destroy.
 */
DS_ANIMATION_EXPORT void dsAnimationTree_destroy(dsAnimationTree* tree);

#ifdef __cplusplus
}
#endif

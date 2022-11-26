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
#include <DeepSea/Core/Types.h>
#include <DeepSea/Math/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/Animation library.
 */

/**
 * @brief Log tag used by the animation library.
 */
#define DS_ANIMATION_LOG_TAG "animation"

/**
 * @brief Constant for an index representing no node.
 */
#define DS_NO_ANIMATION_NODE ((uint32_t)-1)

/// @cond
typedef struct dsAnimationBuildNode dsAnimationBuildNode;
/// @endcond

/**
 * @brief Struct describing a node used for building an animation tree.
 *
 * The animation created with build nodes cannot be used for skinning. This will be flattened when
 * building a dsAnimationTree.
 *
 * @see dsAnimationTree
 * @see AnimationTree.h
 */
typedef struct dsAnimationBuildNode
{
	/**
	 * @brief The name of the node.
	 */
	const char* name;

	/**
	 * @brief The scale of the node.
	 */
	dsVector3f scale;

	/**
	 * @brief The rotation of the node.
	 */
	dsQuaternion4f rotation;

	/**
	 * @brief The translation of the node.
	 */
	dsVector3f translation;

	/**
	 * @brief The number of children for the node.
	 */
	uint32_t childCount;

	/**
	 * @brief The child nodes.
	 */
	const dsAnimationBuildNode* const* children;
} dsAnimationBuildNode;

/**
 * @brief Struct describing a node used for building an animation tree with joints used for
 * skinning.
 *
 * The order and indices will be preserved when building the tree.
 *
 * @see dsAnimationTree
 * @see AnimationTree.h
 */
typedef struct dsAnimationJointBuildNode
{
	/**
	 * @brief The name of the node.
	 */
	const char* name;

	/**
	 * @brief The scale of the node.
	 */
	dsVector3f scale;

	/**
	 * @brief The rotation of the node.
	 */
	dsQuaternion4f rotation;

	/**
	 * @brief The translation of the node.
	 */
	dsVector3f translation;

	/**
	 * @brief Transform to the local space of the node.
	 */
	dsMatrix44f toNodeLocalSpace;

	/**
	 * @brief The number of children for the node.
	 */
	uint32_t childCount;

	/**
	 * @brief The indices of the child nodes.
	 */
	const uint32_t* children;
} dsAnimationJointBuildNode;

/**
 * @brief Struct describing a node transformed for animatins within a dsAnimationTree.
 *
 * The final transform will be in the order of scale, rotation, translation.
 *
 * @see dsAnimationTree
 * @see AnimationTree.h
 */
typedef struct dsAnimationNode
{
	/**
	 * @brief The ID for the name of the node.
	 */
	uint32_t nameID;

	/**
	 * @brief The scale of the node.
	 */
	dsVector3f scale;

	/**
	 * @brief The rotation of the node.
	 */
	dsQuaternion4f rotation;

	/**
	 * @brief The translation of the node.
	 */
	dsVector3f translation;

	/**
	 * @brief The cached transform for this node within the animation tree.
	 */
	dsMatrix44f transform;

	/**
	 * @brief The index of the parent.
	 */
	uint32_t parent;

	/**
	 * @brief The number of children.
	 */
	uint32_t childCount;

	/**
	 * @brief The indices of the child nodes.
	 */
	uint32_t children[];
} dsAnimationNode;

/**
 * @brief Struct describing a tree of nodes transformed for animations.
 * @see dsAnimationBuildNode
 * @see dsAnimationJointBuildNode
 * @see dsAnimationNode
 * @see AnimationTree.h
 */
typedef struct dsAnimationTree
{
	/**
	 * @brief The allocator the animation tree was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief ID for the animation tree.
	 *
	 * The ID is generated for every new dsAnimationTree instance that is created, though it will
	 * be copied when cloned. This can be used to verify that the mapping for node indices is valid
	 * when connecting to an animation.
	 */
	uint32_t id;

	/**
	 * @brief The total number of nodes.
	 */
	uint32_t nodeCount;

	/**
	 * @brief The number of root nodes.
	 */
	uint32_t rootNodeCount;

	/**
	 * @brief The animation nodes in the animation tree.
	 */
	dsAnimationNode** nodes;

	/**
	 * @brief the indices of the root nodes in the tree.
	 */
	const uint32_t* rootNodes;

	/**
	 * @brief List of transforms to the node local space.
	 *
	 * This is a parallel array to nodes. It will only be set when provided, such as for skinning.
	 */
	const dsMatrix44f* toNodeLocalSpace;

	/**
	 * @brief Hash table from name ID to node index.
	 */
	const dsHashTable* nodeTable;
} dsAnimationTree;

#ifdef __cplusplus
}
#endif

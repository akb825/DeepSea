/*
 * Copyright 2023-2025 Aaron Barany
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

#include <DeepSea/Animation/Types.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/SceneAnimation library.
 */

/**
 * @brief Log tag used by the scene animation library.
 */
#define DS_SCENE_ANIMATION_LOG_TAG "scene-animation"

/**
 * @brief Enum for the type of ragdoll used with a scene animation.
 */
typedef enum dsSceneAnimationRagdollType
{
	/**
	 * Replacement for the main skeleton that is normally driven by direct animations, where the
	 * ragdoll is selectively enabled to drive the skeleton through physics.
	 */
	dsSceneAnimationRagdollType_Skeleton,

	/**
	 * Ragdoll for an addition that will always be driven by physics on top of the main skeleton.
	 */
	dsSceneAnimationRagdollType_Addition
} dsSceneAnimationRagdollType;

/**
 * @brief Struct describing a node that manages an animation.
 *
 * Any child node of the animation node may reference the animation. Typically one or more
 * dsSceneAnimationTreeNode will be under the dsSceneAnimationNode to apply the animation.
 *
 * @see SceneAnimationNode.h
 */
typedef struct dsSceneAnimationNode
{
	/**
	 * @brief The base node.
	 */
	dsSceneNode node;

	/**
	 * @brief The cache for animation node maps.
	 */
	dsAnimationNodeMapCache* nodeMapCache;
} dsSceneAnimationNode;

/**
 * @brief Struct describing a node that manages an animation tree.
 *
 * It's expected this will be under a dsSceneAnimationNode to manage the animation. Any child node
 * of the animation node may reference the transformed animation tree, such as to apply a transform
 * from a node of an animation node or skin a model.
 *
 * @see SceneAnimationNode.h
 */
typedef struct dsSceneAnimationTreeNode
{
	/**
	 * @brief The base node.
	 */
	dsSceneNode node;

	/**
	 * @brief The scene animation tree.
	 *
	 * The animation tree will be cloned for each dsSceneTreeNode instance associated with a
	 * dsSceneAnimationList. The node maps may be shared between all dsSceneTreeNode instances and
	 * other dsSceneAnimationNodes that use the same animation tree.
	 */
	dsAnimationTree* animationTree;

	/**
	 * @brief The cache for animation node maps.
	 */
	dsAnimationNodeMapCache* nodeMapCache;
} dsSceneAnimationTreeNode;

/**
 * @brief Struct describing a node that takes a transform from a node in an animation tree.
 *
 * It's expected this will be under a dsSceneAnimationTreeNode to manage the transform.
 *
 * @see SceneAnimationTransformNode.h
 */
typedef struct dsSceneAnimationTransformNode
{
	/**
	 * @brief The base node.
	 */
	dsSceneNode node;

	/**
	 * @brief The name of the animation node to take the transform from.
	 */
	const char* animationNodeName;

	/**
	 * @brief The ID of the animation node to take the transform from.
	 */
	uint32_t animationNodeID;
} dsSceneAnimationTransformNode;

/**
 * @brief Struct describing a node that takes reads the transform from a node and applies it to
 * the ragdoll animation.
 *
 * It's expected this will be under a dsSceneAnimationNode to manage the animation.
 *
 * @see SceneAnimationRagdollNode.h
 */
typedef struct dsSceneAnimationRagdollNode
{
	/**
	 * @brief The base node.
	 */
	dsSceneNode node;

	/**
	 * @brief The type of ragdoll this drives.
	 */
	dsSceneAnimationRagdollType ragdollType;

	/**
	 * @brief Bitmask of the animation components to apply.
	 */
	uint32_t animationComponents;

	/**
	 * @brief The number of nodes to go up for the relative transform.
	 */
	unsigned int relativeAncestor;

	/**
	 * @brief The name of the animation node to take the transform from.
	 */
	const char* animationNodeName;
} dsSceneAnimationRagdollNode;

/**
 * @brief Scene item list implementation for managing animations.
 *
 * This will hold information for the various scene animation node types.
 *
 * @see SceneAnimationList.h
 */
typedef struct dsSceneAnimationList dsSceneAnimationList;

#ifdef __cplusplus
}
#endif

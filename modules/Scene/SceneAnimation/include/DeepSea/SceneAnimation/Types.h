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

#ifdef __cplusplus
}
#endif

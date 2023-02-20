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
 * @brief Struct describing an animaton tree used within a scene.
 *
 * This brings together an animation tree and shared cache of direct and keyframe animation node
 * mappings. The animation tree stored within this is intended to not be modified, but should be
 * cloned for each instance that uses it to tie together with an actual animation.
 *
 * @see SceneAnimationTree.h
 */
typedef struct dsSceneAnimationTree dsSceneAnimationTree;

/**
 * @brief Struct describing a node that manages an animation with an animation tree.
 *
 * Any child node of the animation node may reference the transformed animation tree, such as to
 * apply a transform from a node of an animation node or skin a model.
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
	 * @brief The scene animation tree.
	 *
	 * The animation tree will be cloned for each dsSceneTreeNode instance associated with a
	 * dsSceneAnimationList. The node maps may be shared between all dsSceneTreeNode instances and
	 * other dsSceneAnimationNodes that use the same animation tree.
	 */
	dsSceneAnimationTree* animationTree;
} dsSceneAnimationNode;

/**
 * @brief Struct defining an animation used in a scene.
 *
 * This is typically created in a dsSceneAnimationList to be stored with the dsSceneTreeNode for a
 * dsSceneAnimationNode. This ties together a dsAnimation and dsAnimationTree to animate.
 *
 * The dsSceneAnimation interface should be used to add and remove animations to ensure that the
 * shared animation node maps are properly used.
 *
 * @see SceneAnimation.h
 */
typedef struct dsSceneAnimation
{
	/**
	 * @brief The allocator the animation was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The animation.
	 */
	dsAnimation* animation;

	/**
	 * @brief The animation tree to animate.
	 */
	dsAnimationTree* animationTree;

	/**
	 * @brief The scene animation tree.
	 *
	 * This is also used to have shared animation node maps.
	 */
	dsSceneAnimationTree* sceneAnimationTree;
} dsSceneAnimation;

#ifdef __cplusplus
}
#endif

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
 * @brief Enum for the component to animate.
 */
typedef enum dsAnimationComponent
{
	dsAnimationComponent_Translation, ///< Animates the translation of the node.
	dsAnimationComponent_Rotation,    ///< Animates the rotation of the node.
	dsAnimationComponent_Scale        ///< Animates the scale of the node.
} dsAnimationComponent;

/**
 * @brief Enum for how to interpolate an animation keyframe.
 */
typedef enum dsAnimationInterpolation
{
	dsAnimationInterpolation_Step,   ///< Instantly switch to the new value.
	dsAnimationInterpolation_Linear, ///< Linearly interpolate between values.
	dsAnimationInterpolation_Cubic   ///< Interpolate using cubic splines.
} dsAnimationInterpolation;

/**
 * @brief Struct describing the transform for an animation joint used for skinning.
 */
typedef struct dsAnimationJointTransform
{
	/**
	 * @brief The transform for the joint.
	 */
	dsMatrix44f transform;

	/**
	 * @brief The inverse transpose of the transform.
	 */
	dsMatrix33f inverseTranspose;
} dsAnimationJointTransform;

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
	 *
	 * All child indices must be after the current node's index.
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
	const uint32_t* children;
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
	dsAnimationNode* nodes;

	/**
	 * @brief the indices of the root nodes in the tree.
	 */
	const uint32_t* rootNodes;

	/**
	 * @brief List of transforms to the node local space.
	 *
	 * This is a parallel array to nodes. This will only be set when built from joints.
	 */
	const dsMatrix44f* toNodeLocalSpace;

	/**
	 * @brief List of transforms for joints when performing skinning.
	 *
	 * This is a parallel array to nodes. This will only be set when built from joints.
	 */
	dsAnimationJointTransform* jointTransforms;

	/**
	 * @brief Hash table from name ID to node index.
	 */
	const dsHashTable* nodeTable;
} dsAnimationTree;

/**
 * @brief Struct describing a channel of an animation.
 *
 * A channel applies a transform to a component of a transform of an animation node.
 *
 * @see dsAnimationKeyframes
 * @see dsKeyframeAnimation
 * @see KeyframeAnimation.h
 */
typedef struct dsAnimationChannel
{
	/**
	 * @brief The name of the node to animate.
	 */
	const char* node;

	/**
	 * @brief The component of the node transform to animate.
	 */
	dsAnimationComponent component;

	/**
	 * @brief How to interpolate the values from one keyframe to the next.
	 */
	dsAnimationInterpolation interpolation;

	/**
	 * @brief The number of total values.
	 *
	 * The number of values depend on the component, interpolation, and parent dsAnimationKeyframes
	 * structure:
	 *
	 * Base values based on the component:
	 * - Translation: 3.
	 * - Rotation: 4.
	 * - Scale: 4.
	 *
	 * Multiplier for the number of base values based on interpolation type:
	 * - Step and linear: 1
	 * - Cubic: 3
	 *
	 * The total number of values is the number of keyframes multiplied the number of base values
	 * and interpolation multiplier.
	 */
	uint32_t valueCount;

	/**
	 * @brief The values for the animation component.
	 */
	const float* values;
} dsAnimationChannel;

/**
 * @brief Struct describing keyframes wihin an animation with shared timestamps.
 * @see dsAnimationChannel
 * @see dsKeyframeAnimation
 * @see KeyframeAnimation.h
 */
typedef struct dsAnimationKeyframes
{
	/**
	 * @brief The number of keyframes.
	 */
	uint32_t keyframeCount;

	/**
	 * @brief The number of channels the keyframes apply to.
	 */
	uint32_t channelCount;

	/**
	 * @brief The time value for each keyframe.
	 */
	const float* keyframeTimes;

	/**
	 * @brief The channels that apply to the keyframe.
	 */
	const dsAnimationChannel* channels;
} dsAnimationKeyframes;

/**
 * @brief Struct describing an animation built on keyframes.
 * @see dsAnimationChannel
 * @see dsAnimationKeyframes
 * @see KeyframeAnimation.h
 */
typedef struct dsKeyframeAnimation
{
	/**
	 * @brief The allocator the keyframe animation was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief Unique ID for the keyframe animation.
	 *
	 * The ID is generated for every new dsKeyframeAnimation instance that is created. This can be
	 * used to verify that the mapping for node indices is valid when connecting to an animation
	 * tree.
	 */
	uint32_t id;

	/**
	 * @brief The minimum time for any keyframe.
	 */
	float minTime;

	/**
	 * @brief The maximum time for any keyframe.
	 */
	float maxTime;

	/**
	 * @brief The number of dsAnimationKeyframes instances.
	 */
	uint32_t keyframesCount;

	/**
	 * @brief The keyframes for the animation.
	 */
	const dsAnimationKeyframes* keyframes;
} dsKeyframeAnimation;

#ifdef __cplusplus
}
#endif

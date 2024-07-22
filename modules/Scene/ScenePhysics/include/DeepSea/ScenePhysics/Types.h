/*
 * Copyright 2024 Aaron Barany
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
#include <DeepSea/Physics/Types.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/ScenePhysics library.
 */

/**
 * @brief Log tag used by the scene physics library.
 */
#define DS_SCENE_PHYSICS_LOG_TAG "scene-physics"

/**
 * @brief Struct describing a scene node that instantiates physics objects as needed.
 *
 * This can store rigid body templates and constraints to instantiate whenever the node is
 * instantiated within the scene. Sub-nodes of this, such as with dsSceneRigidBodyTransformNode, may
 * look up the instances by name.
 *
 * @see ScenePhysicsInstanceNode.h
 */
typedef struct dsScenePhysicsInstanceNode dsScenePhysicsInstanceNode;

/**
 * @brief Struct defining a physics constraint in a scene.
 *
 * This may have instance names for actors and connected constraints to instantiate when under a
 * dsScenePhysicsInstanceNode.
 *
 * @see ScenePhysicsConstraint.h
 */
typedef struct dsScenePhysicsConstraint
{
	/**
	 * @brief The allocator the constraint was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The base physics constraint.
	 */
	dsPhysicsConstraint* constraint;

	/**
	 * @brief The instance name for the first rigid body.
	 */
	const char* firstRigidBodyInstanceName;

	/**
	 * @brief The instance name for the constraint for the first actor that is related to this
	 *     constraint.
	 */
	const char* firstConnectedConstraintInstanceName;

	/**
	 * @brief The instance name for the second rigid body.
	 */
	const char* secondRigidBodyInstanceName;

	/**
	 * @brief The instance name for the constraint for the second actor that is related to this
	 *     constraint.
	 */
	const char* secondConnectedConstraintInstanceName;
} dsScenePhysicsConstraint;

/**
 * @brief Struct describing a node that synchronizes the transform with a rigid body.
 *
 * @see SceneRigidBodyNode.h
 */
typedef struct dsSceneRigidBodyNode
{
	/**
	 * @brief The base node.
	 */
	dsSceneNode node;

	/**
	 * @brief The name of the rigid body to manage.
	 *
	 * This will be set when dynamically getting the rigid body from a parent
	 * dsScenePhysicsInstanceNode.
	 */
	const char* rigidBodyName;

	/**
	 * @brief The ID of the rigid body to manage.
	 */
	uint32_t rigidBodyID;

	/**
	 * @brief The rigid body to manage.
	 *
	 * This will be set when the node can only be instantiated once from a rigid body.
	 */
	dsRigidBody* rigidBody;
} dsSceneRigidBodyNode;

#ifdef __cplusplus
}
#endif

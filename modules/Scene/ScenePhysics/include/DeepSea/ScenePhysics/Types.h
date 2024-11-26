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

/// @cond
typedef struct dsScenePhysicsConstraintNode dsScenePhysicsConstraintNode;
/// @endcond

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
	const char* firstRigidBodyInstance;

	/**
	 * @brief The instance name for the constraint for the first actor that is related to this
	 * constraint.
	 */
	const char* firstConnectedConstraintInstance;

	/**
	 * @brief The instance name for the second rigid body.
	 */
	const char* secondRigidBodyInstance;

	/**
	 * @brief The instance name for the constraint for the second actor that is related to this
	 * constraint.
	 */
	const char* secondConnectedConstraintInstance;
} dsScenePhysicsConstraint;

/**
 * @brief Struct describing a node that synchronizes the transform with a rigid body.
 *
 * None of the members should be modified directly.
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
	 * dsSceneRigidBodyGroupNode.
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

/**
 * @brief Struct describiing a rigid body template with a name.
 *
 * This is used when initializing a dsSceneRigidBodyGroupNode with its component rigid bodies.
 *
 * @see dsSceneRigidBodyGroupNode
 * @see SceneRigidBodyGroupNode.h
 */
typedef struct dsNamedSceneRigidBodyTemplate
{
	/**
	 * @brief The name of the rigid body.
	 */
	const char* name;

	/**
	 * @brief The rigid body template.
	 *
	 * This will be used to create the rigid bodies when instantiated in the scene graph.
	 */
	dsRigidBodyTemplate* rigidBodyTemplate;

	/**
	 * @brief Whether to transfer ownership to the node.
	 *
	 * If true the rigid body template will be deleted even if node creation failed.
	 */
	bool transferOwnership;
} dsNamedSceneRigidBodyTemplate;

/**
 * @brief Struct describiing a physics constraint with a name.
 *
 * This is used when initializing a dsSceneRigidBodyGroupNode with its component constraints.
 *
 * @see dsSceneRigidBodyGroupNode
 * @see SceneRigidBodyGroupNode.h
 */
typedef struct dsNamedScenePhysicsConstraint
{
	/**
	 * @brief The name of the rigid body.
	 */
	const char* name;

	/**
	 * @brief The constraint.
	 *
	 * This will be cloned when instantiated in the scene graph.
	 */
	dsPhysicsConstraint* constraint;

	/**
	 * @brief The name of the first rigid body on the constraint.
	 *
	 * If NULL the rigid body originally set on the constraint will be used.
	 */
	const char* firstRigidBody;

	/**
	 * @brief The name of the first connected constraint.
	 *
	 * If NULL the connected constraint originally set on the constraint will be used.
	 */
	const char* firstConnectedConstraint;

	/**
	 * @brief The name of the second rigid body on the constraint.
	 *
	 * If NULL the rigid body originally set on the constraint will be used.
	 */
	const char* secondRigidBody;

	/**
	 * @brief The name of the second connected constraint.
	 *
	 * If NULL the connected constraint originally set on the constraint will be used.
	 */
	const char* secondConnectedConstraint;

	/**
	 * @brief Whether to transfer ownership to the node.
	 *
	 * If true the constraint will be deleted even if node creation failed.
	 */
	bool transferOwnership;
} dsNamedScenePhysicsConstraint;

/**
 * @brief Struct describing a ndoe that holds rigid bodies and constraints for a sub-graph.
 *
 * dsSceneRigidBodyNode instances below this in the scene graph hierarchy may reference rigid bodies
 * by name.
 *
 * @see SceneRigidBodyGroupNode.h
 */
typedef struct dsSceneRigidBodyGroupNode dsSceneRigidBodyGroupNode;

/**
 * @brief Struct describing a reference to a physics actor in a scene, typically used for a physics
 * constraint node.
 *
 * One of rigidBodyInstance or actor must be set. If rigidBodyInstance is set, rigidBodyGroupNode
 * must also be set.
 *
 * @see dsScenePhysicsConstraintNode
 */
typedef struct dsScenePhysicsActorReference
{
	/**
	 * @brief The root node used to search for instances for the rigid body group node.
	 *
	 * If NULL, searches will be done directy from rigidBodyGroupNode.
	 */
	const dsSceneNode* rootNode;

	/**
	 * @brief The rigid body group node to search for the rigid body by name.
	 *
	 * This may be NULL when the actor is provided by pointer.
	 */
	const dsSceneRigidBodyGroupNode* rigidBodyGroupNode;

	/**
	 * @brief The instance name for the rigid body.
	 *
	 * If set, this will be searched for within rigidBodyGroupNode.
	 */
	const char* instanceName;

	/**
	 * @brief A direct pointer to the actor.
	 *
	 * This should be set when not searching by instance name.
	 */
	const dsPhysicsActor* actor;
} dsScenePhysicsActorReference;

/**
 * @brief Struct describing a reference to a physics constraint in a scene, typically used for a
 * physics constraint node.
 *
 * One of constraintInstance, constraintNode, or constraint must be set. If constraintInstance is
 * set, rigidBodyGroupNode must also be set.
 *
 * @see dsScenePhysicsConstraintNode
 */
typedef struct dsScenePhysicsConstraintReference
{
	/**
	 * @brief The root node used to search for instances for the rigid body group node or physics
	 * constraint node.
	 *
	 * If NULL, searches will be done directy from rigidBodyGroupNode or constraintNode.
	 */
	const dsSceneNode* rootNode;

	/**
	 * @brief The rigid body group node to search for the constraint by name.
	 *
	 * This may be NULL when the constraint is provided by pointer.
	 */
	const dsSceneRigidBodyGroupNode* rigidBodyGroupNode;

	/**
	 * @brief The instance name for the constraint.
	 *
	 * If set, this will be searched for within rigidBodyGroupNode.
	 */
	const char* instanceName;

	/**
	 * @brief The constraint node to get the constraint from.
	 *
	 * This will use the instance within the scene graph relative to rootNode. If rootNode is not
	 * set, constraintNode must only exist once in the scene graph.
	 */
	const dsScenePhysicsConstraintNode* constraintNode;

	/**
	 * @brief A direct pointer to the constraint.
	 *
	 * This should be set when not searching by instance name or from a constraint node.
	 */
	const dsPhysicsConstraint* constraint;
} dsScenePhysicsConstraintReference;

/**
 * @brief Struct describing a node that maintains a constraint between two rigid bodies.
 *
 * This allows for connecting constraints between actors (typically rigid bodies) between different
 * sub-trees in the scene graph. While a dsRigidBodyGroupNode typically contains both rigid bodies
 * and constraints that are part of a single logical group of ojbects, this allows for constraints
 * between two such groups.
 *
 * This will not keep a reference count to any of the node pointers for the actor and constraint
 * references, as it may introduce circular references. It is the responsibility of the owner of a
 * constraint node to destroy it before any nodes, actors, or other constraints it depends on.
 *
 * None of the members should be modified directly.
 *
 * @see ScenePhysicsConstraintNode.h
 */
struct dsScenePhysicsConstraintNode
{
	/**
	 * @brief The base node.
	 */
	dsSceneNode node;

	/**
	 * @brief The base physics constraint.
	 */
	dsPhysicsConstraint* constraint;

	/**
	 * @brief Reference to the first actor in the constraint.
	 */
	dsScenePhysicsActorReference firstActor;

	/**
	 * @brief Reference to the first connected constraint.
	 */
	dsScenePhysicsConstraintReference firstConnectedConstraint;

	/**
	 * @brief Reference to the second actor in the constraint.
	 */
	dsScenePhysicsActorReference secondActor;

	/**
	 * @brief Reference to the second connected constraint.
	 */
	dsScenePhysicsConstraintReference secondConnectedConstraint;

	/**
	 * @brief Whether this node owns the base constraint.
	 */
	bool ownsConstraint;

	/**
	 * @brief Instance ID for the first actor.
	 */
	uint32_t firstActorInstanceID;

	/**
	 * @brief Instance ID for the first connected constraint.
	 */
	uint32_t firstConnectedConstraintInstanceID;

	/**
	 * @brief Instance ID for the second actor.
	 */
	uint32_t secondActorInstanceID;

	/**
	 * @brief Instance ID for the second connected constraint.
	 */
	uint32_t secondConnectedConstraintInstanceID;
};

#ifdef __cplusplus
}
#endif

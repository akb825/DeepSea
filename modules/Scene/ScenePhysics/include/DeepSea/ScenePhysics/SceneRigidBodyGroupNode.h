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
#include <DeepSea/ScenePhysics/Export.h>
#include <DeepSea/ScenePhysics/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene rigid body group nodes.
 * @see dsSceneRigidBodyGroupNode
 */

/**
 * @brief The type name for an rigid body group node.
 */
DS_SCENEPHYSICS_EXPORT extern const char* const dsSceneRigidBodyGroupNode_typeName;

/**
 * @brief Gets the type of an rigid body group node.
 * @return The type of an rigid body group node.
 */
DS_SCENEPHYSICS_EXPORT const dsSceneNodeType* dsSceneRigidBodyGroupNode_type(void);

/**
 * @brief Creates a rigid body group node.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the node with. This must support freeing memory.
 * @param motionType The motion type for the rigid body group, or dsPhysicsMotionType_Unknown if
 *     any motion type may be used for the component rigid bodies. Setting to an explicit motion
 *     type may have better performance for some implementations.
 * @param rigidBodies The rigid bodies for the group.
 * @param rigidBodyCount The number of rigid bodies.
 * @param constraints The constraints used with the rigid bodies.
 * @param constraintCount The number of constraints.
 * @param itemLists The list of item list names that will be used to process the node. These will be
 *     copied.
 * @param itemListCount The number of item lists.
 * @return The rigid body group node or NULL if an error occurred.
 */
DS_SCENEPHYSICS_EXPORT dsSceneRigidBodyGroupNode* dsSceneRigidBodyGroupNode_create(
	dsAllocator* allocator, dsPhysicsMotionType motionType,
	const dsNamedSceneRigidBodyTemplate* rigidBodies, uint32_t rigidBodyCount,
	const dsNamedScenePhysicsConstraint* constraints, uint32_t constraintCount,
	const char* const* itemLists, uint32_t itemListCount);

/**
 * @brief Finds a rigid body by name for a tree node.
 *
 * This will check starting with the tree node passed in, then go up for each successive parent
 * until a dsSceneRigidBodyGroupNode is found. This assumes that the animation was created from a
 * dsScenePhysicsList.
 *
 * @param treeNode The tree node to get the rigid body for.
 * @param name The name of the rigid body.
 * @return The rigid body or NULL if there isn't one present.
 */
DS_SCENEPHYSICS_EXPORT dsRigidBody* dsSceneRigidBodyGroupNode_findRigidBodyForInstanceName(
	const dsSceneTreeNode* treeNode, const char* name);

/**
 * @brief Finds a rigid body by name ID for a tree node.
 *
 * This will check starting with the tree node passed in, then go up for each successive parent
 * until a dsSceneRigidBodyGroupNode is found. This assumes that the animation was created from a
 * dsScenePhysicsList.
 *
 * @param treeNode The tree node to get the rigid body for.
 * @param nameID The ID for the name of the rigid body.
 * @return The rigid body or NULL if there isn't one present.
 */
DS_SCENEPHYSICS_EXPORT dsRigidBody* dsSceneRigidBodyGroupNode_findRigidBodyForInstanceID(
	const dsSceneTreeNode* treeNode, uint32_t nameID);

/**
 * @brief Finds a constraint by name for a tree node.
 *
 * This will check starting with the tree node passed in, then go up for each successive parent
 * until a dsSceneRigidBodyGroupNode is found. This assumes that the animation was created from a
 * dsScenePhysicsList.
 *
 * @param treeNode The tree node to get the rigid body for.
 * @param name The name of the rigid body.
 * @return The constraint or NULL if there isn't one present.
 */
DS_SCENEPHYSICS_EXPORT dsPhysicsConstraint* dsSceneRigidBodyGroupNode_findConstraintForInstanceName(
	const dsSceneTreeNode* treeNode, const char* name);

/**
 * @brief Finds a constraint by name ID for a tree node.
 *
 * This will check starting with the tree node passed in, then go up for each successive parent
 * until a dsSceneRigidBodyGroupNode is found. This assumes that the animation was created from a
 * dsScenePhysicsList.
 *
 * @param treeNode The tree node to get the rigid body for.
 * @param nameID The ID for the name of the rigid body.
 * @return The constraint or NULL if there isn't one present.
 */
DS_SCENEPHYSICS_EXPORT dsPhysicsConstraint* dsSceneRigidBodyGroupNode_findConstraintForInstanceID(
	const dsSceneTreeNode* treeNode, uint32_t nameID);

#ifdef __cplusplus
}
#endif

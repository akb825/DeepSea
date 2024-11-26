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
 * @brief Functions for creating and manipulating scene physics constraint nodes.
 * @see dsScenePhysicsConstraintNode
 */

/**
 * @brief The type name for a physics constraint node.
 */
DS_SCENEPHYSICS_EXPORT extern const char* const dsScenePhysicsConstraintNode_typeName;

/**
 * @brief Gets the type of a physics constraint node.
 * @return The type of a physics constraint node.
 */
DS_SCENEPHYSICS_EXPORT const dsSceneNodeType* dsScenePhysicsConstraintNode_type(void);

/**
 * @brief Creates a physics constraint node.
 *
 * Any names provided within actor and constraint references will be copied.
 *
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the node with. This must support freeing memory.
 * @param constraint The base physics constraint.
 * @param takeOwnership Whether to take ownership of the base constraint. If true, the constraint
 *     will be destroyed immediately if creation fails.
 * @param firstActor A reference to the first actor. If NULL, the first actor on the base constraint
 *     will be used.
 * @param firstConnectedConstraint A reference to the first connected constraint. If NULL, the first
 *     connected constraint on the base constraint will be used if it is present. The connected
 *     constraint is optional, so it is valid to neither pass a reference here nor have one set on
 *     the base constraint.
 * @param secondActor A reference to the second actor. If NULL, the second actor on the base
 *     constraint will be used.
 * @param secondConnectedConstraint A reference to the first connected constraint. If NULL, the
 *     second connected constraint on the base constraint will be used if it is present. The
 *     connected constraint is optional, so it is valid to neither pass a reference here nor have
 *     one set on the base constraint.
 * @param itemLists The list of item list names that will be used to process the node. These will be
 *     copied.
 * @param itemListCount The number of item lists.
 * @return The rigid body node or NULL if an error occurred.
 */
DS_SCENEPHYSICS_EXPORT dsScenePhysicsConstraintNode* dsScenePhysicsConstraintNode_create(
	dsAllocator* allocator, dsPhysicsConstraint* constraint, bool takeOwnership,
	const dsScenePhysicsActorReference* firstActor,
	const dsScenePhysicsConstraintReference* firstConnectedConstraint,
	const dsScenePhysicsActorReference* secondActor,
	const dsScenePhysicsConstraintReference* secondConnectedConstraint,
	const char* const* itemLists, uint32_t itemListCount);

/**
 * @brief Gets the physics constraint for a tree node.
 *
 * This will check starting with the tree node passed in, then go up for each successive parent
 * until a dsScenePhysicsConstraintNode is found. This assumes that the physics constraint was
 * created from a dsScenePhysicsList.
 *
 * @param treeNode The tree node to get the physics constraint for.
 * @return The physics constraint or NULL if there isn't one present.
 */
DS_SCENEPHYSICS_EXPORT dsPhysicsConstraint* dsScenePhysicsConstraintNode_getConstraintForInstance(
	const dsSceneTreeNode* treeNode);

#ifdef __cplusplus
}
#endif

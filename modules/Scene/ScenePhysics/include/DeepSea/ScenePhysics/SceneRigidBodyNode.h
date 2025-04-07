/*
 * Copyright 2024-2025 Aaron Barany
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
 * @brief Functions for creating and manipulating scene rigid body nodes.
 * @see dsSceneRigidBodyNode
 */

/**
 * @brief The type name for a rigid body node.
 */
DS_SCENEPHYSICS_EXPORT extern const char* const dsSceneRigidBodyNode_typeName;

/**
 * @brief Gets the type of a rigid body node.
 * @return The type of a rigid body node.
 */
DS_SCENEPHYSICS_EXPORT const dsSceneNodeType* dsSceneRigidBodyNode_type(void);

/**
 * @brief Creates a rigid body node.
 *
 * Only one of rigidBodyName, rigidBody, or rigidBodyTemplate should be set.
 *
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the node with. This must support freeing memory.
 * @param rigidBodyName The name of the rigid body to take the transform from. This will be set when
 *     dynamically getting the rigid body from a parent dsScenePhysicsInstanceNode.
 * @param rigidBody The rigid body to get the transform from. This will be set when the node can
 *     only be instantiated once from a rigid body.
 * @param rigidBodyTemplate The rigid body template to instantiate and take the transform from. This
 *     will be set when the node can be instantiated multiple times from a rigid body template
 *     independent of a dsScenePhysicsInstanceNode.
 * @param transferOwnership Whether ownership of rigidBody or rigidBodyTemplate should be
 *     transferred to the node. If this is true and an error occurs during creation, the rigidBody
 *     or rigidBodyTemplate pointers will be destroyed immediately.
 * @param itemLists The list of item list names that will be used to process the node. These will be
 *     copied.
 * @param itemListCount The number of item lists.
 * @return The rigid body node or NULL if an error occurred.
 */
DS_SCENEPHYSICS_EXPORT dsSceneRigidBodyNode* dsSceneRigidBodyNode_create(dsAllocator* allocator,
	const char* rigidBodyName, dsRigidBody* rigidBody, dsRigidBodyTemplate* rigidBodyTemplate,
	bool transferOwnership, const char* const* itemLists, uint32_t itemListCount);

/**
 * @brief Gets the rigid body for a tree node.
 *
 * This will check starting with the tree node passed in, then go up for each successive parent
 * until a dsSceneRigidBodyNode is found. This assumes that the rigid body was created from a
 * dsScenePhysicsList.
 *
 * @param treeNode The tree node to get the rigid body for.
 * @return The rigid body or NULL if there isn't one present.
 */
DS_SCENEPHYSICS_EXPORT dsRigidBody* dsSceneRigidBodyNode_getRigidBodyForInstance(
	const dsSceneTreeNode* treeNode);

#ifdef __cplusplus
}
#endif

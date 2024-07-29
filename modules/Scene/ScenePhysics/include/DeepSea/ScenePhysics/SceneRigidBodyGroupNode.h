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
	dsAllocator* allocator, const dsNamedSceneRigidBodyTemplate* rigidBodies,
	uint32_t rigidBodyCount, const dsNamedScenePhysicsConstraint* constraints,
	uint32_t constraintCount, const char* const* itemLists, uint32_t itemListCount);

#ifdef __cplusplus
}
#endif

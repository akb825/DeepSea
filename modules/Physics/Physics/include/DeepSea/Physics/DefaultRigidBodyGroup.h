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
#include <DeepSea/Physics/Export.h>
#include <DeepSea/Physics/RigidBodyTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manipulate a default rigid body group implementation.
 *
 * These are provided as a convenience for physics implementations where the underlying library
 * doesn't support native rigid body groups. These functions assume that parameter validation (e.g.
 * not passing NULL) has been done ahead of time.
 *
 * @see dsRigidBodyGroup
 */

/**
 * @brief Creates a default rigid body group.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the rigid body group with.
 * @param allocator The allocator to create the rigid body group with. This must support freeing
 *     memory.
 * @param motionType The motion type for the rigid bodies created with the group.
 * @return The rigid body group or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsRigidBodyGroup* dsDefaultRigidBodyGroup_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, dsPhysicsMotionType motionType);

/**
 * @brief Adds a rigid body to a default rigid body group.
 * @remark errno will be set on failure.
 * @param group The rigid body group.
 * @param rigidBody The rigid body to add.
 * @return False if the rigid body couldn't be added.
 */
DS_PHYSICS_EXPORT bool dsDefaultRigidBodyGroup_addRigidBody(dsRigidBodyGroup* group,
	dsRigidBody* rigidBody);

/**
 * @brief Removes a rigid body from a default rigid body group.
 * @remark The caller is responsible for removing the rigid body from the scene if needed.
 * @remark errno will be set on failure.
 * @param group The rigid body group.
 * @param rigidBody The rigid body to remove.
 * @return False if the rigid body couldn't be removed.
 */
DS_PHYSICS_EXPORT bool dsDefaultRigidBodyGroup_removeRigidBody(dsRigidBodyGroup* group,
	dsRigidBody* rigidBody);

/**
 * @brief Adds a default rigid body group to a scene.
 * @remark errno will be set on failure.
 * @param engine The physics engine the rigid body group was created with. This is mainly provided
 *     to assign the function pointer on dsPhysicsEngine.
 * @param scene The scene to add the rigid body group to.
 * @param group The rigid body group.
 * @param activate Whether the rigid bodies should be activated on insertion.
 * @return False if the rigid body group couldn't be added.
 */
DS_PHYSICS_EXPORT bool dsDefaultRigidBodyGroup_addToScene(dsPhysicsEngine* engine,
	dsPhysicsScene* scene, dsRigidBodyGroup* group, bool activate);

/**
 * @brief Removes a default rigid body group from a scene.
 * @remark errno will be set on failure.
 * @param engine The physics engine the rigid body group was created with. This is mainly provided
 *     to assign the function pointer on dsPhysicsEngine.
 * @param scene The scene to remove the rigid body group from.
 * @param group The rigid body group.
 * @return False if the rigid body group couldn't be removed.
 */
DS_PHYSICS_EXPORT bool dsDefaultRigidBodyGroup_removeFromScene(dsPhysicsEngine* engine,
	dsPhysicsScene* scene, dsRigidBodyGroup* group);

/**
 * @brief Destroys a default rigid body group.
 * @remark errno will be set on failure.
 * @param engine The physics engine the rigid body group was created with. This is unused and only
 *     provided to assign the function pointer on dsPhysicsEngine.
 * @param group The rigid body group to destroy.
 * @return False if the rigid body group couldn't be destroyed.
 */
DS_PHYSICS_EXPORT bool dsDefaultRigidBodyGroup_destroy(dsPhysicsEngine* engine,
	dsRigidBodyGroup* group);

#ifdef __cplusplus
}
#endif

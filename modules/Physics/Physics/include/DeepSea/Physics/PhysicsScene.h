/*
 * Copyright 2023-2024 Aaron Barany
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
#include <DeepSea/Physics/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manipulate physics scenes.
 * @see dsPhysicsScene
 */

/**
 * @brief Creates a physics scene.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the physics scene with.
 * @param allocator The allocator to create the physics scene with. If NULL, it will use the same
 *     allocator as the physics engine.
 * @param limits The limits for the physics scene.
 * @param threadPool The thread pool to use for multithreaded processing, or NULL for
 *     single-threaded processing.
 * @return The physics scene or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsPhysicsScene* dsPhysicsScene_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsSceneLimits* limits, dsThreadPool* threadPool);

/**
 * @brief Adds rigid bodies to a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to add the rigid body to.
 * @param rigidBodies The rigid bodies to add. These must not be part of a rigid body group.
 * @param rigidBodyCount The number of rigid bodies to add.
 * @param activate Whether the rigid bodies should be activated on insertion.
 * @return False if the rigid body couldn't be added.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_addRigidBodies(dsPhysicsScene* scene,
	dsRigidBody* const* rigidBodies, uint32_t rigidBodyCount, bool activate);

/**
 * @brief Removes a rigid body from a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to remove the rigid body from.
 * @param rigidBodies The rigid bodies to remove. These must not be part of a rigid body group.
 * @param rigidBodyCount The number of rigid bodies to remove.
 * @return False if the rigid body couldn't be removed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_removeRigidBodies(dsPhysicsScene* scene,
	dsRigidBody* const* rigidBodies, uint32_t rigidBodyCount);

/**
 * @brief Adds a rigid body group to a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to add the rigid body group to.
 * @param group The rigid body group to add.
 * @param activate Whether the rigid bodies should be activated on insertion.
 * @return False if the rigid body group couldn't be added.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_addRigidBodyGroup(dsPhysicsScene* scene,
	dsRigidBodyGroup* group, bool activate);

/**
 * @brief Removes a rigid body group from a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to remove the rigid body group from.
 * @param group The rigid body group to remove.
 * @return False if the rigid body group couldn't be removed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_removeRigidBodyGroup(dsPhysicsScene* scene,
	dsRigidBodyGroup* group);

/**
 * @brief Destroys a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to destroy.
 * @return False if the scene couldn't be destroyed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_destroy(dsPhysicsScene* scene);

#ifdef __cplusplus
}
#endif

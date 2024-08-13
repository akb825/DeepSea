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
 * @brief Functions to create and manipulate rigid body groups.
 * @see dsRigidBodyGroup
 */

/**
 * @brief Creates a rigid body group.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the rigid body group with.
 * @param allocator The allocator to create the rigid body group with. This must support freeing
 *     memory.
 * @param motionType The motion type for the rigid bodies created with the group. This may be set to
 *     dsPhysicsMotionType_Unknown to allow for any motion type for component rigid bodies, but
 *     having a consistent motion type may be more efficient for some implementations.
 * @return The rigid body group or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsRigidBodyGroup* dsRigidBodyGroup_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, dsPhysicsMotionType motionType);

/**
 * @brief Destroys a rigid body group.
 *
 * All component rigidi bodies must have already been destroyed before destroying the rigid body
 * group.
 *
 * @remark errno will be set on failure.
 * @param group The rigid body group to destroy.
 * @return False if the rigid body group couldn't be destroyed.
 */
DS_PHYSICS_EXPORT bool dsRigidBodyGroup_destroy(dsRigidBodyGroup* group);

#ifdef __cplusplus
}
#endif

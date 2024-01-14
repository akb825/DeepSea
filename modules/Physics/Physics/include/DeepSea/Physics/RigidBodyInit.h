/*
 * Copyright 2023 Aaron Barany
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
 * @brief Functions to create and manipulate rigid body initialization objects.
 * @see dsRigidBodyInit
 */

/**
 * @brief Initializes a rigid body initialization object.
 *
 * This takes the most commonly changed attributes and sets the default values for the remaining
 * members.
 *
 * @remark errno will be set on failure.
 * @param rigidBodyInit The rigid body initialization object to initialize.
 * @param flags The flags for the rigid body.
 * @param motionType The initial motion type for the rigid body.
 * @param layer The layer the rigid body will be associated with.
 * @param position The position of the rigid body or NULL if at the origin.
 * @param orientation The orientation of the rigid body or NULL if not rotated.
 * @param scale The scale of the rigid body or NULL if not scaled.
 * @param friction The coefficient of friction, with 0 meaning no friction and increasing values
 *     having higher friction.
 * @param restitution The restitution of the rigid body, where 0 is fully inelastic and 1 is fully
 *     elastic.
 * @return False if the parameters are invalid.
 */
DS_PHYSICS_EXPORT bool dsRigidBodyInit_initialize(dsRigidBodyInit* rigidBodyInit,
	dsRigidBodyFlags flags, dsPhysicsMotionType motionType, dsPhysicsLayer layer,
	const dsVector3f* position, dsQuaternion4f* orientation, const dsVector3f* scale,
	float friction, float restitution);

/**
 * @brief Initializes a rigid body initialization for a rigid body that will be a part of a group.
 *
 * This takes the most commonly changed attributes and sets the default values for the remaining
 * members.
 *
 * @remark errno will be set on failure.
 * @param rigidBodyInit The rigid body initialization object to initialize.
 * @param group The group the rigid body will be a member of. The rigid body group must live longer
 *     than the rigid body.
 * @param flags The flags for the rigid body.
 * @param layer The layer the rigid body will be associated with.
 * @param position The position of the rigid body or NULL if at the origin.
 * @param orientation The orientation of the rigid body or NULL if not rotated.
 * @param scale The scale of the rigid body or NULL if not scaled.
 * @param friction The coefficient of friction, with 0 meaning no friction and increasing values
 *     having higher friction.
 * @param restitution The restitution of the rigid body, where 0 is fully inelastic and 1 is fully
 *     elastic.
 * @return False if the parameters are invalid.
 */
DS_PHYSICS_EXPORT bool dsRigidBodyInit_initializeGroup(dsRigidBodyInit* rigidBodyInit,
	dsRigidBodyGroup* group, dsRigidBodyFlags flags, dsPhysicsLayer layer,
	const dsVector3f* position, dsQuaternion4f* orientation, const dsVector3f* scale,
	float friction, float restitution);

/**
 * @brief Checks whether a rigid body initialization object is valid.
 * @return False if the members on rigidBodyInit are incompatible.
 */
DS_PHYSICS_EXPORT bool dsRigidBodyInit_isValid(const dsRigidBodyInit* rigidBodyInit);

#ifdef __cplusplus
}
#endif

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
#include <DeepSea/Physics/Constraints/Types.h>
#include <DeepSea/Physics/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for manipulating physics constraints.
 * @see dsPhysicsConstraint
 */

/**
 * @brief Initializes a physics constraint.
 *
 * This is called by the specific shape's initialize functions.
 *
 * @remark errno will be set on failure.
 * @param constraint The constraint to initialize.
 * @param engine The physics engine the constraint was created with.
 * @param allocator The allocator the constraint was created with.
 * @param type The type of the constraint.
 * @param firstActor The first actor for the constraint.
 * @param secondActor The second actor for the constraint.
 * @param enabled Whether the constraint is enabled.
 * @param impl The underlying implementation of the constraint.
 * @param getForceFunc Function to get the last applied force for the constraint.
 * @param getTorqueFunc Function to get the last applied torque for the constraint.
 * @param destroyFunc The destroy function of the constraint.
 * @return False if the parameters are invalid.
 */
DS_PHYSICS_EXPORT bool dsPhysicsConstraint_initialize(dsPhysicsConstraint* constraint,
	dsPhysicsEngine* engine, dsAllocator* allocator, dsPhysicsConstraintType type,
	const dsPhysicsActor* firstActor, const dsPhysicsActor* secondActor, bool enabled,
	void* impl, dsGetPhysicsConstraintForceFunction getForceFunc,
	dsGetPhysicsConstraintForceFunction getTorqueFunc,
	dsDestroyPhysicsConstraintFunction destroyFunc);

/**
 * @brief Sets whether a physics constraint is enabled.
 *
 * When a constraint is disabled it will not be enforced.
 *
 * @remark errno will be set on failure.
 * @param constraint The constraint to set the enabled state on.
 * @param enabled Whether the constraint is enabled.
 * @return False if the enabled state couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsPhysicsConstraint_setEnabled(
	dsPhysicsConstraint* constraint, bool enabled);

/**
 * @brief Gets the force applied in the previous step to enforce a physics constraint.
 *
 * It is only valid to query the last applied force for an enabled constraint.
 *
 * @remark errno will be set on failure.
 * @param[out] outForce The last applied force for the constraint. This will be relative to the
 *     first body of the constraint.
 * @param constraint The physics constraint to get the force for.
 * @return False if the force couldn't be queried.
 */
DS_PHYSICS_EXPORT bool dsPhysicsConstraint_getLastAppliedForce(dsVector3f* outForce,
	const dsPhysicsConstraint* constraint);

/**
 * @brief Gets the torque applied in the previous step to enforce a physics constraint.
 *
 * It is only valid to query the last applied torque for an enabled constraint.
 *
 * @remark errno will be set on failure.
 * @param[out] outTorque The last applied torque for the constraint. This will be relative to the
 *     first body of the constraint.
 * @param constraint The physics constraint to get the torque for.
 * @return False if the force couldn't be queried.
 */
DS_PHYSICS_EXPORT bool dsPhysicsConstraint_getLastAppliedTorque(dsVector3f* outTorque,
	const dsPhysicsConstraint* constraint);

/**
 * @brief Destroys a physics constraint.
 * @remark errno will be set on failure.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsConstraint_destroy(dsPhysicsConstraint* constraint);

#ifdef __cplusplus
}
#endif

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
 * @brief Functions for creating and manipulating revolute physics constraints.
 * @see dsRevolutePhysicsConstraint
 */

/**
 * @brief Gets the type for a revolute physics constraint.
 * @return The type for a revolute physics constraint.
 */
DS_PHYSICS_EXPORT const dsPhysicsConstraintType* dsRevolutePhysicsConstraint_type(void);

/**
 * @brief Creates a revolute physics constraint.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with. If NULL the engine's allocator will
 *     be used.
 * @param firstActor The first physics actor the constraint is attached to. This may be NULL to set
 *     later by cloning.
 * @param firstPosition The position of the constraint on the first actor.
 * @param firstRotation The rotation of the constraint on the first actor. The axis of the
 *     quaternion represents the axis that will be rotated around, while the rotation will be used
 *     relative to any rotation limits.
 * @param secondActor The second physics actor the constraint is attached to. This may be NULL to
 *     set later by cloning.
 * @param secondPosition The position of the constraint on the second actor.
 * @param secondRotation The rotation of the constraint on the second actor. The axis of the
 *     quaternion represents the axis that will be rotated around, while the rotation will be used
 *     relative to any rotation limits.
 * @param limitEnabled Whether the rotation limit is enabled.
 * @param minAngle The minimum angle for the rotation in the range [-pi, 0].
 * @param maxAngle The maximum angle for the rotation in the range [0, pi].
 * @param limitStiffness The spring stiffness applied when limiting the angle.
 * @param limitDamping The spring damping applied when limiting the angle in the range [0, 1].
 * @param motorType The type of motor to use.
 * @param motorTarget The target of the motor, either as an angle or an angular velocity.
 * @param maxMotorTorque The maximum torque to apply for the motor. When the motor is disabled, the
 *     torque will be applied to stop motion.
 * @return The revolute constraint or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsRevolutePhysicsConstraint* dsRevolutePhysicsConstraint_create(
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstRotation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondRotation, bool limitEnabled, float minAngle, float maxAngle,
	float limitStiffness, float limitDamping, dsPhysicsConstraintMotorType motorType,
	float motorTarget, float maxMotorTorque);

/**
 * @brief Enables the angle limit and sets the limit parameters for a revolute physics constraint.
 * @remark errno will be set on failure.
 * @param constraint The constraint to set the angle limits on.
 * @param minAngle The minimum angle for the rotation in the range [-pi, 0].
 * @param maxAngle The maximum angle for the rotation in the range [0, pi].
 * @param limitStiffness The spring stiffness applied when limiting the angle.
 * @param limitDamping The spring damping applied when limiting the angle in the range [0, 1].
 * @return False if the limit couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRevolutePhysicsConstraint_setLimit(dsRevolutePhysicsConstraint* constraint,
	float minAngle, float maxAngle, float limitStiffness, float limitDamping);

/**
 * @brief Disables the angle limit for a revolute physics constraint.
 * @remark errno will be set on failure.
 * @param constraint The constraint to disable the angle limits on.
 * @return False if the limit couldn't be disabled.
 */
DS_PHYSICS_EXPORT bool dsRevolutePhysicsConstraint_disableLimit(
	dsRevolutePhysicsConstraint* constraint);

/**
 * @brief Sets the motor parameters for a revolute physics constraint.
 * @remark errno will be set on failure.
 * @param constraint The constraint to set the motor parameters on.
 * @param motorType The type of motor to use.
 * @param target The target angle if motorType is dsPhysicsConstraintMotorType_Position or target
 *    rotational velocity if motorType is dsPhysicsConstraintMotorType_Velocity.
 * @param maxTorque The maximum torque to apply for the motor. When the motor is disabled, the
 *     torque will be applied to stop motion.
 */
DS_PHYSICS_EXPORT bool dsRevolutePhysicsConstraint_setMotor(dsRevolutePhysicsConstraint* constraint,
	dsPhysicsConstraintMotorType motorType, float target, float maxTorque);

/**
 * @brief Initializes a revolute physics constraint.
 *
 * This is called by the physics implementation to initialize the common members.
 *
 * @param[out] constraint The constraint to initialize.
 * @param engine The physics engine the constraint was created with.
 * @param allocator The allocator the constraint was created with.
 * @param firstActor The first physics actor the constraint is attached to.
 * @param firstPosition The position of the constraint on the first actor.
 * @param firstRotation The rotation of the constraint on the first actor. The axis of the
 *     quaternion represents the axis that will be rotated around, while the rotation will be used
 *     relative to any rotation limits.
 * @param secondActor The second physics actor the constraint is attached to.
 * @param secondPosition The position of the constraint on the second actor.
 * @param secondRotation The rotation of the constraint on the second actor. The axis of the
 *     quaternion represents the axis that will be rotated around, while the rotation will be used
 *     relative to any rotation limits.
 * @param limitEnabled Whether the rotation limit is enabled.
 * @param minAngle The minimum angle for the rotation in the range [-pi, 0].
 * @param maxAngle The maximum angle for the rotation in the range [0, pi].
 * @param limitStiffness The spring stiffness applied when limiting the angle.
 * @param limitDamping The spring damping applied when limiting the angle in the range [0, 1].
 * @param motorType The type of motor to use.
 * @param motorTarget The target of the motor, either as an angle or an angular velocity.
 * @param maxMotorTorque The maximum torque to apply for the motor. When the motor is disabled, the
 *     torque will be applied to stop motion.
 * @param impl The underlying implementation for the constraint.
 */
DS_PHYSICS_EXPORT void dsRevolutePhysicsConstraint_initialize(
	dsRevolutePhysicsConstraint* constraint, dsPhysicsEngine* engine, dsAllocator* allocator,
	const dsPhysicsActor* firstActor, const dsVector3f* firstPosition,
	const dsQuaternion4f* firstRotation, const dsPhysicsActor* secondActor,
	const dsVector3f* secondPosition, const dsQuaternion4f* secondRotation, bool limitEnabled,
	float minAngle, float maxAngle, float limitStiffness, float limitDamping,
	dsPhysicsConstraintMotorType motorType, float motorTarget, float maxMotorTorque, void* impl);

#ifdef __cplusplus
}
#endif

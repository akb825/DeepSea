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
 * @brief Functions for creating and manipulating slider physics constraints.
 * @see dsSliderPhysicsConstraint
 */

/**
 * @brief Gets the type for a slider physics constraint.
 * @return The type for a slider physics constraint.
 */
DS_PHYSICS_EXPORT const dsPhysicsConstraintType* dsSliderPhysicsConstraint_type(void);

/**
 * @brief Creates a slider physics constraint.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with. If NULL the engine's allocator will
 *     be used.
 * @param firstActor The first physics actor the constraint is attached to. This may be NULL to set
 *     later by cloning.
 * @param firstPosition The position of the constraint on the first actor.
 * @param firstOrientation The rotation of the constraint on the first actor. The slider will be
 *     limited to the axis of the quaternion.
 * @param secondActor The second physics actor the constraint is attached to. This may be NULL to
 *     set later by cloning.
 * @param secondPosition The position of the constraint on the second actor.
 * @param secondOrientation The rotation of the constraint on the second actor. The slider will be
 *     limited to the axis of the quaternion.
 * @param limitEnabled Whether the distance limit is enabled.
 * @param minDistance The minimum distance between the reference points in the range [-FLT_MAX, 0].
 * @param maxDistance The maximum distance between the reference points in the range [0, FLT_MAX].
 * @param limitStiffness The spring stiffness applied when limiting the distance.
 * @param limitDamping The spring damping applied when limiting the distance in the range [0, 1].
 * @param motorType The type of motor to use.
 * @param motorTarget The target of the motor, either as an angle or an angular velocity.
 * @param maxMotorForce The maximum force to apply for the motor. When the motor is disabled, the
 *     force will be applied to stop motion.
 * @return The slider constraint or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsSliderPhysicsConstraint* dsSliderPhysicsConstraint_create(
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstOrientation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondOrientation, bool limitEnabled, float minDistance,
	float maxDistance, float limitStiffness, float limitDamping,
	dsPhysicsConstraintMotorType motorType, float motorTarget, float maxMotorForce);

/**
 * @brief Enables the angle limit and sets the limit parameters for a slider physics constraint.
 * @remark errno will be set on failure.
 * @param constraint The constraint to set the angle limits on.
 * @param minDistance The minimum distance between the reference points in the range [-FLT_MAX, 0].
 * @param maxDistance The maximum distance between the reference points in the range [0, FLT_MAX].
 * @param limitStiffness The spring stiffness applied when limiting the distance.
 * @param limitDamping The spring damping applied when limiting the distance in the range [0, 1].
 * @return False if the limit couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsSliderPhysicsConstraint_setLimit(dsSliderPhysicsConstraint* constraint,
	float minDistance, float maxDistance, float limitStiffness, float limitDamping);

/**
 * @brief Disables the angle limit for a slider physics constraint.
 * @remark errno will be set on failure.
 * @param constraint The constraint to disable the angle limits on.
 * @return False if the limit couldn't be disabled.
 */
DS_PHYSICS_EXPORT bool dsSliderPhysicsConstraint_disableLimit(
	dsSliderPhysicsConstraint* constraint);

/**
 * @brief Sets the motor parameters for a slider physics constraint.
 * @remark errno will be set on failure.
 * @param constraint The constraint to set the motor parameters on.
 * @param motorType The type of motor to use.
 * @param target The target distance if motorType is dsPhysicsConstraintMotorType_Position or target
 *    velocity if motorType is dsPhysicsConstraintMotorType_Velocity.
 * @param maxForce The maximum force to apply for the motor. When the motor is disabled, the
 *     force will be applied to stop motion.
 */
DS_PHYSICS_EXPORT bool dsSliderPhysicsConstraint_setMotor(dsSliderPhysicsConstraint* constraint,
	dsPhysicsConstraintMotorType motorType, float target, float maxForce);

/**
 * @brief Initializes a slider physics constraint.
 *
 * This is called by the physics implementation to initialize the common members.
 *
 * @param[out] constraint The constraint to initialize.
 * @param engine The physics engine the constraint was created with.
 * @param allocator The allocator the constraint was created with.
 * @param firstActor The first physics actor the constraint is attached to.
 * @param firstPosition The position of the constraint on the first actor.
 * @param firstOrientation The rotation of the constraint on the first actor. The slider will be
 *     limited to the axis of the quaternion.
 * @param secondActor The second physics actor the constraint is attached to.
 * @param secondPosition The position of the constraint on the second actor.
 * @param secondOrientation The rotation of the constraint on the second actor. The slider will be
 *     limited to the axis of the quaternion.
 * @param limitEnabled Whether the distance limit is enabled.
 * @param minDistance The minimum distance between the reference points in the range [-FLT_MAX, 0].
 * @param maxDistance The maximum distance between the reference points in the range [0, FLT_MAX].
 * @param limitStiffness The spring stiffness applied when limiting the distance.
 * @param limitDamping The spring damping applied when limiting the distance in the range [0, 1].
 * @param motorType The type of motor to use.
 * @param motorTarget The target of the motor, either as an angle or an angular velocity.
 * @param maxMotorForce The maximum force to apply for the motor. When the motor is disabled, the
 *     force will be applied to stop motion.
 * @param impl The underlying implementation for the constraint.
 */
DS_PHYSICS_EXPORT void dsSliderPhysicsConstraint_initialize(
	dsSliderPhysicsConstraint* constraint, dsPhysicsEngine* engine, dsAllocator* allocator,
	const dsPhysicsActor* firstActor, const dsVector3f* firstPosition,
	const dsQuaternion4f* firstOrientation, const dsPhysicsActor* secondActor,
	const dsVector3f* secondPosition, const dsQuaternion4f* secondOrientation, bool limitEnabled,
	float minDistance, float maxDistance, float limitStiffness, float limitDamping,
	dsPhysicsConstraintMotorType motorType, float motorTarget, float maxMotorForce, void* impl);

#ifdef __cplusplus
}
#endif

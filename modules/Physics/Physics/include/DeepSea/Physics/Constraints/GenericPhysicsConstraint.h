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
 * @brief Functions for creating and manipulating generic physics constraints.
 * @see dsGenericPhysicsConstraint
 */

/**
 * @brief Gets the type for a generic physics constraint.
 * @return The type for a generic physics constraint.
 */
DS_PHYSICS_EXPORT const dsPhysicsConstraintType* dsGenericPhysicsConstraint_type(void);

/**
 * @brief Creates a generic physics constraint.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with. If NULL the engine's allocator will
 *     be used.
 * @param firstActor The first physics actor the constraint is attached to. This may be NULL to set
 *     later by cloning.
 * @param firstPosition The position of the constraint on the first actor.
 * @param firstOrientation The orientation of the constraint on the first actor.
 * @param secondActor The second physics actor the constraint is attached to. This may be NULL to
 *     set later by cloning.
 * @param secondPosition The position of the constraint on the second actor.
 * @param secondOrientation The orientation of the constraint on the second actor.
 * @param limits The limits for each degree of freedom.
 * @param motors The motors for each degree of freedom.
 * @param combineSwingTwistMotors Whether the swing and twist motors are combined.
 * @return The generic constraint or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsGenericPhysicsConstraint* dsGenericPhysicsConstraint_create(
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstOrientation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondOrientation,
	const dsGenericPhysicsConstraintLimit limits[DS_PHYSICS_CONSTRAINT_DOF_COUNT],
	const dsGenericPhysicsConstraintMotor motors[DS_PHYSICS_CONSTRAINT_DOF_COUNT],
	bool combineSwingTwistMotors);

/**
 * @brief Sets the limits for a degree of freedom of a generic physics constraint.
 * @remark errno will be set on failure.
 * @param constraint The constraint to set the limit on.
 * @param dof The degree of freedom to set the limit for.
 * @param limitType The type of the limit.
 * @param minValue The minimum value of the limit. For angles this must be in the range [-pi, pi].
 * @param maxValue The maximum value of the limit. For angles this must be in the range [-pi, pi].
 * @param stiffness The stiffness when the limited by range.
 * @param damping The damping when the limited by range in the range [0, 1].
 * @return False if the limit couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsGenericPhysicsConstraint_setLimit(dsGenericPhysicsConstraint* constraint,
	dsPhysicsConstraintDOF dof, dsPhysicsConstraintLimitType limitType, float minValue,
	float maxValue, float stiffness, float damping);

/**
 * @brief Sets the motor parameters for a degree of freedom of a generic physics constraint.
 * @remark errno will be set on failure.
 * @param constraint The constraint to set the limit on.
 * @param dof The degree of freedom to set the motor for.
 * @param motorType The type of the motor.
 * @param target The target of the motor, either a position or velocity.
 * @param maxForce The maximum force to apply for the motor. If the motor is disabled this is the
 *     maximum amount of force to apply to stop motion.
 */
DS_PHYSICS_EXPORT bool dsGenericPhysicsConstraint_setMotor(dsGenericPhysicsConstraint* constraint,
	dsPhysicsConstraintDOF dof, dsPhysicsConstraintMotorType motorType, float target,
	float maxForce);

/**
 * @brief Sets whether the swing and twist motors are combined for a generic physics constraint.
 * @remark errno will be set on failure.
 * @param constraint The constraint to set the combine swing twist state on.
 * @param combineSwingTwist True to combine the swing and twist motors, false to keep them separate.
 * @return False if the combine swing twist state couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsGenericPhysicsConstraint_setCombineSwingTwistMotor(
	dsGenericPhysicsConstraint* constraint, bool combineSwingTwist);

/**
 * @brief Initializes a generic physics constraint.
 *
 * This is called by the physics implementation to initialize the common members.
 *
 * @param[out] constraint The constraint to initialize.
 * @param engine The physics engine the constraint was created with.
 * @param allocator The allocator the constraint was created with.
 * @param firstActor The first physics actor the constraint is attached to.
 * @param firstPosition The position of the constraint on the first actor.
 * @param firstOrientation The orientation of the constraint on the first actor.
 * @param secondActor The second physics actor the constraint is attached to.
 * @param secondPosition The position of the constraint on the second actor.
 * @param secondOrientation The orientation of the constraint on the second actor.
 * @param limits The limits for each degree of freedom.
 * @param motors The motors for each degree of freedom.
 * @param combineSwingTwistMotors Whether the swing and twist motors are combined.
 * @param impl The underlying implementation for the constraint.
 */
DS_PHYSICS_EXPORT void dsGenericPhysicsConstraint_initialize(dsGenericPhysicsConstraint* constraint,
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstOrientation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondOrientation,
	const dsGenericPhysicsConstraintLimit limits[DS_PHYSICS_CONSTRAINT_DOF_COUNT],
	const dsGenericPhysicsConstraintMotor motors[DS_PHYSICS_CONSTRAINT_DOF_COUNT],
	bool combineSwingTwistMotors, void* impl);

#ifdef __cplusplus
}
#endif

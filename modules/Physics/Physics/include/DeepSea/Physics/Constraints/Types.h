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
#include <DeepSea/Core/Types.h>
#include <DeepSea/Math/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @cond
typedef struct dsPhysicsActor dsPhysicsActor;
typedef struct dsPhysicsConstraint dsPhysicsConstraint;
typedef struct dsPhysicsEngine dsPhysicsEngine;
/// @endcond

/**
 * @file
 * @brief Includes all of the types for constraints in the DeepSea/Physics library.
 */

/**
 * @brief Enum for the type of motor to apply to a physicsconstraint.
 */
typedef enum dsPhysicsConstraintMotorType
{
	dsPhysicsConstraintMotorType_Disabled, ///< The motor is disabled and doesn't apply force.
	/** Forces are applied to reach a target position or orientation. */
	dsPhysicsConstraintMotorType_Position,
	dsPhysicsConstraintMotorType_Velocity  ///< Forces are applied to reach a target velocity.
} dsPhysicsConstraintMotorType;

/**
 * @brief Value that denotes the type of a physics constraint.
 */
typedef const int* dsPhysicsConstraintType;

/**
 * @brief Function to destroy a physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
typedef bool (*dsDestroyPhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsPhysicsConstraint* constraint);

/**
 * @brief Function to get the applied force for a physics constraint.
 * @param[out] outForce The force applied to the constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constraint to get the force for.
 * @return False if the force couldn't be queried.
 */
typedef bool (*dsGetPhysicsConstraintForceFunction)(dsVector3f* outForce,
	dsPhysicsEngine* engine, const dsPhysicsConstraint* constraint);

/**
 * @brief Base type for a physics constraint.
 *
 * Constraints place requirements of the positions of physics bodies relative to each-other. Forces
 * will be applied to the bodies to ensure the constraints are satisfied to the best of the physics
 * engine's capabilities.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see PhysicsConstraint.h
 */
typedef struct dsPhysicsConstraint
{
	/**
	 * @brief The physics engine the shape was created with.
	 */
	dsPhysicsEngine* engine;

	/**
	 * @brief The allocator the shape was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The type of the constraint.
	 */
	dsPhysicsConstraintType type;

	/**
	 * @brief Whether the constraint is enabled.
	 */
	bool enabled;

	/**
	 * @brief The first actor for the constraint.
	 */
	const dsPhysicsActor* firstActor;

	/**
	 * @brief The second actor for the constraint.
	 */
	const dsPhysicsActor* secondActor;

	/**
	 * @brief Pointer to the constraint implementation.
	 *
	 * This is a convenience to avoid needing to check the type to get the underlying constraint for
	 * the physics implementation.
	 */
	void* impl;

	/**
	 * @brief Function to get the applied force for the constraint.
	 */
	dsGetPhysicsConstraintForceFunction getForceFunc;

	/**
	 * @brief Function to get the applied torque for the constraint.
	 */
	dsGetPhysicsConstraintForceFunction getTorqueFunc;

	/**
	 * @brief Function to destroy the constraint.
	 */
	dsDestroyPhysicsConstraintFunction destroyFunc;
} dsPhysicsConstraint;

/**
 * @brief Struct describing a physics constraint that has zero degrees of freedom.
 *
 * This effectively glues two actors together so they move as one rigid object.
 *
 * Transforms are relative to the local coordinate space of each actor. The transforms are
 * immutable, so changing the attachment location and orientation requires creating a new
 * constraint.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see FixedPhysicsConstraint.h
 */
typedef struct dsFixedPhysicsConstraint
{
	/**
	 * @brief The base constraint type.
	 */
	dsPhysicsConstraint constraint;

	/**
	 * @brief The position of the constraint relative to the first actor.
	 */
	dsVector3f firstPosition;

	/**
	 * @brief The position of the constraint relative to the second actor.
	 */
	dsVector3f secondPosition;

	/**
	 * @brief The rotation of the constraint relative to the first actor.
	 */
	dsQuaternion4f firstRotation;

	/**
	 * @brief The rotation of the constraint relative to the second actor.
	 */
	dsQuaternion4f secondRotation;
} dsFixedPhysicsConstraint;

/**
 * @brief Struct describing a physics constraint that has free rotation around a point.
 *
 * This is akin to a ball-socket and has no limits to the rotation.
 *
 * Points are relative to the local coordinate space of each actor and are immutable, so changing
 * the attachment location requires creating a new constraint.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see PointPhysicsConstraint.h
 */
typedef struct dsPointPhysicsConstraint
{
	/**
	 * @brief The base constraint type.
	 */
	dsPhysicsConstraint constraint;

	/**
	 * @brief The position of the constraint relative to the first actor.
	 */
	dsVector3f firstPosition;

	/**
	 * @brief The position of the constraint relative to the second actor.
	 */
	dsVector3f secondPosition;
} dsPointPhysicsConstraint;

/**
 * @brief Struct describing a physics constraint that has limited rotation at a point.
 *
 * This is akin to a ball-socket that has a hard limit to a cone.
 *
 * Transforms are relative to the local coordinate space of each actor. The transforms are
 * immutable, so changing the attachment location and orientation requires creating a new
 * constraint. The limiting angle may be adjusted after creation.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see ConePhysicsConstraint.h
 */
typedef struct dsConePhysicsConstraint
{
	/**
	 * @brief The base constraint type.
	 */
	dsPhysicsConstraint constraint;

	/**
	 * @brief The position of the constraint relative to the first actor.
	 */
	dsVector3f firstPosition;

	/**
	 * @brief The position of the constraint relative to the second actor.
	 */
	dsVector3f secondPosition;

	/**
	 * @brief The rotation of the constraint relative to the first actor.
	 */
	dsQuaternion4f firstRotation;

	/**
	 * @brief The rotation of the constraint relative to the second actor.
	 */
	dsQuaternion4f secondRotation;

	/**
	 * @brief The maximum angle of the constraint relative to the attachment rotation axes.
	 */
	float maxAngle;
} dsConePhysicsConstraint;

/**
 * @brief Struct describing a physics constraint that has limited rotation at a point.
 *
 * This is akin to a ball-socket that has a hard limit to the swing and the twist. Each axis may
 * have an independnent angle limit, allowing for a non-symmiterical cone. The XY plane is normal
 * to the connecting point, meaning the X and Y axis angles are along the swing and the Z axis is
 * the twist. A motor may be applied to rotate towards a goal rotation.
 *
 * This joint is suitable for ragdolls applied to a skeleton.
 *
 * Transforms are relative to the local coordinate space of each actor. The transforms are
 * immutable, so changing the attachment location and orientation requires creating a new
 * constraint. The limiting angles and motor may be adjusted after creation.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see SwingTwistPhysicsConstraint.h
 */
typedef struct dsSwingTwistPhysicsConstraint
{
	/**
	 * @brief The base constraint type.
	 */
	dsPhysicsConstraint constraint;

	/**
	 * @brief The position of the constraint relative to the first actor.
	 */
	dsVector3f firstPosition;

	/**
	 * @brief The position of the constraint relative to the second actor.
	 */
	dsVector3f secondPosition;

	/**
	 * @brief The rotation of the constraint relative to the first actor.
	 */
	dsQuaternion4f firstRotation;

	/**
	 * @brief The rotation of the constraint relative to the second actor.
	 */
	dsQuaternion4f secondRotation;

	/**
	 * @brief The maximum angle of the constraint along the X axis.
	 */
	float maxSwingXAngle;

	/**
	 * @brief The maximum angle of the constraint along the Y axis.
	 */
	float maxSwingYAngle;

	/**
	 * @brief The maximum angle of the constraint along the Z axis.
	 */
	float maxTwistZAngle;

	/**
	 * @brief The type of motor to use for the constraint.
	 *
	 * dsPhysicsConstraintMotorType_Velocity is not supported.
	 */
	dsPhysicsConstraintMotorType motorType;

	/**
	 * @brief The target rotation for the motor relative to the first actor.
	 */
	dsQuaternion4f targetRotation;

	/**
	 * @brief The maximum torque for the motor.
	 *
	 * If the motor is disabled this is the maximum amount of torque to applied to stop motion.
	 */
	float maxTorque;
} dsSwingTwistPhysicsConstraint;

/**
 * @brief Struct describing a revolute physics constraint, or constraint that can rotate around an
 *     arbitrary axis.
 *
 * This may be used to represent a hinge or axle.
 *
 * Transforms are relative to the local coordinate space of each actor. The transforms are
 * immutable, so changing the attachment location and orientation requires creating a new
 * constraint. The limiting angles and motor may be adjusted after creation.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see RevolutePhysicsConstraint.h
 */
typedef struct dsRevolutePhysicsConstraint
{
	/**
	 * @brief The base constraint type.
	 */
	dsPhysicsConstraint constraint;

	/**
	 * @brief The position of the constraint relative to the first actor.
	 */
	dsVector3f firstPosition;

	/**
	 * @brief The position of the constraint relative to the second actor.
	 */
	dsVector3f secondPosition;

	/**
	 * @brief The rotation of the constraint relative to the first actor.
	 *
	 * The axis of the quaternion represents the axis that will be rotated around, while the
	 * rotation will be used relative to any rotation limits.
	 */
	dsQuaternion4f firstRotation;

	/**
	 * @brief The rotation of the constraint relative to the second actor.
	 *
	 * The axis of the quaternion represents the axis that will be rotated around, while the
	 * rotation will be used relative to any rotation limits.
	 */
	dsQuaternion4f secondRotation;

	/**
	 * @brief Whether the angle is enabled.
	 */
	bool limitEnabled;

	/**
	 * @brief The minimum angle when the limit is enabled.
	 */
	float minAngle;

	/**
	 * @brief The maximum angle when the limit is enabled.
	 */
	float maxAngle;

	/**
	 * @brief The spring stiffness applied when limiting the angle.
	 */
	float limitStiffness;

	/**
	 * @brief The spring damping applied when limiting the angle.
	 */
	float limitDamping;

	/**
	 * @brief The type of the motor to apply to the constraint.
	 */
	dsPhysicsConstraintMotorType motorType;

	/**
	 * @brief The target for the motor.
	 *
	 * This will be an angle if motorType is dsPhysicsConstraintMotorType_Position or an angular
	 * velocity if motorType is dsPhysicsConstraintMotorType_Velocity.
	 */
	float motorTarget;

	/**
	 * @brief The maximum torque for the motor.
	 *
	 * If the motor is disabled this is the maximum amount of torque to applied to stop motion.
	 */
	float maxTorque;
} dsRevolutePhysicsConstraint;

/**
 * @brief Function to set whether a physics constraint is enabled.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constrant to set the enabled state on.
 * @param enabled Whether the constraint is enabled and will be enforced.
 * @return False if the enabled state couldn't be set.
 */
typedef bool (*dsSetPhysicsConstraintEnabledFunction)(dsPhysicsEngine* engine,
	dsPhysicsConstraint* constraint, bool enabled);

/**
 * @brief Function to create a fixed physics constraint.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with.
 * @param enabled Whether the constraint is enabled after creation.
 * @param firstActor The first physics actor the constraint is attached to.
 * @param firstPosition The position of the constraint on the first actor.
 * @param firstRotation The rotation of the constraint on the first actor.
 * @param secondActor The second physics actor the constraint is attached to.
 * @param secondPosition The position of the constraint on the second actor.
 * @param secondRotation The rotation of the constraint on the second actor.
 * @return The fixed constraint or NULL if it couldn't be created.
 */
typedef dsFixedPhysicsConstraint* (*dsCreateFixedPhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsAllocator* allocator, bool enabled, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstRotation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondRotation);

/**
 * @brief Function to destroy a fixed physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
typedef bool (*dsDestroyFixedPhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsFixedPhysicsConstraint* constraint);

/**
 * @brief Function to create a point physics constraint.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with.
 * @param enabled Whether the constraint is enabled after creation.
 * @param firstActor The first physics actor the constraint is attached to.
 * @param firstPosition The position of the constraint on the first actor.
 * @param secondActor The second physics actor the constraint is attached to.
 * @param secondPosition The position of the constraint on the second actor.
 * @return The point constraint or NULL if it couldn't be created.
 */
typedef dsPointPhysicsConstraint* (*dsCreatePointPhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsAllocator* allocator, bool enabled, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsPhysicsActor* secondActor,
	const dsVector3f* secondPosition);

/**
 * @brief Function to destroy a point physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
typedef bool (*dsDestroyPointPhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsPointPhysicsConstraint* constraint);

/**
 * @brief Function to create a cone physics constraint.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with.
 * @param enabled Whether the constraint is enabled after creation.
 * @param firstActor The first physics actor the constraint is attached to.
 * @param firstPosition The position of the constraint on the first actor.
 * @param firstRotation The rotation of the constraint on the first actor.
 * @param secondActor The second physics actor the constraint is attached to.
 * @param secondPosition The position of the constraint on the second actor.
 * @param secondRotation The rotation of the constraint on the second actor.
 * @param maxAngle The maximum angle of the constraint relative to the attachment rotation axes.
 * @return The fixed constraint or NULL if it couldn't be created.
 */
typedef dsConePhysicsConstraint* (*dsCreateConePhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsAllocator* allocator, bool enabled, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstRotation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondRotation, float maxAngle);

/**
 * @brief Function to destroy a cone physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
typedef bool (*dsDestroyConePhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsConePhysicsConstraint* constraint);

/**
 * @brief Function to set the max angle for a cone physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to set the max angle on.
 * @param maxAngle The maximum angle of the constraint relative to the attachment rotation axes.
 * @return False if the angle couldn't be set.
 */
typedef bool (*dsSetConePhysicsConstraintMaxAngleFunction)(dsPhysicsEngine* engine,
	dsConePhysicsConstraint* constraint, float maxAngle);

/**
 * @brief Function to create a swing twist physics constraint.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with.
 * @param enabled Whether the constraint is enabled after creation.
 * @param firstActor The first physics actor the constraint is attached to.
 * @param firstPosition The position of the constraint on the first actor.
 * @param firstRotation The rotation of the constraint on the first actor.
 * @param secondActor The second physics actor the constraint is attached to.
 * @param secondPosition The position of the constraint on the second actor.
 * @param secondRotation The rotation of the constraint on the second actor.
 * @param maxSwingXAngle The maximum angle of the constraint along the X axis.
 * @param maxSwingYAngle The maximum angle of the constraint along the Y axis.
 * @param maxTwistZAngle The maximum angle of the constraint along the Z axis.
 * @param motorType The type of motor to use.
 * @param targetRotation The target rotation to reach when the motor is enabled.
 * @param maxTorque The maximum torque to apply for the motor.
 * @return The swing twist constraint or NULL if it couldn't be created.
 */
typedef dsSwingTwistPhysicsConstraint* (*dsCreateSwingTwistPhysicsConstraintFunction)(
	dsPhysicsEngine* engine, dsAllocator* allocator, bool enabled,
	const dsPhysicsActor* firstActor, const dsVector3f* firstPosition,
	const dsQuaternion4f* firstRotation, const dsPhysicsActor* secondActor,
	const dsVector3f* secondPosition, const dsQuaternion4f* secondRotation, float maxSwingXAngle,
	float maxSwingYAngle, float maxTwistZAngle, dsPhysicsConstraintMotorType motorType,
	const dsQuaternion4f* targetRotation, float maxTorque);

/**
 * @brief Function to destroy a swing twist physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
typedef bool (*dsDestroySwingTwistPhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsSwingTwistPhysicsConstraint* constraint);

/**
 * @brief Function to set the max angles for a swing twist physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to set the max angle on.
 * @param maxSwingXAngle The maximum angle of the constraint along the X axis.
 * @param maxSwingYAngle The maximum angle of the constraint along the Y axis.
 * @param maxTwistZAngle The maximum angle of the constraint along the Z axis.
 * @return False if the angles couldn't be set.
 */
typedef bool (*dsSetSwingTwistPhysicsConstraintMaxAnglesFunction)(dsPhysicsEngine* engine,
	dsSwingTwistPhysicsConstraint* constraint, float maxSwingXAngle, float maxSwingYAngle,
	float maxTwistZAngle);

/**
 * @brief Function to set the motor parameters for a swing twist physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to set the max angle on.
 * @param motorType The type of motor to use.
 * @param targetRotation The target rotation of the joint or NULL to leave unchanged.
 * @param maxTorque The maximum torque to apply for the motor.
 * @return False if the motor parameters couldn't be set.
 */
typedef bool (*dsSetSwingTwistPhysicsConstraintMotorFunction)(dsPhysicsEngine* engine,
	dsSwingTwistPhysicsConstraint* constraint, dsPhysicsConstraintMotorType motorType,
	const dsQuaternion4f* targetRotation, float maxTorque);

/**
 * @brief Function to create a revolute physics constraint.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with.
 * @param enabled Whether the constraint is enabled after creation.
 * @param firstActor The first physics actor the constraint is attached to.
 * @param firstPosition The position of the constraint on the first actor.
 * @param firstRotation The rotation of the constraint on the first actor.
 * @param secondActor The second physics actor the constraint is attached to.
 * @param secondPosition The position of the constraint on the second actor.
 * @param secondRotation The rotation of the constraint on the second actor.
 * @param limitEnabled Whether the rotation limit is enabled.
 * @param minAngle The minimum angle for the rotation.
 * @param maxAngle The maximum angle for the rotation.
 * @param limitStiffness The spring stiffness applied when limiting the angle.
 * @param limitDamping The spring damping applied when limiting the angle.
 * @param motorType The type of motor to use.
 * @param motorTarget The target of the motor, either as an angle or an angular velocity.
 * @param maxTorque The maximum torque to apply for the motor.
 * @return The revolute constraint or NULL if it couldn't be created.
 */
typedef dsRevolutePhysicsConstraint* (*dsCreateRevolutePhysicsConstraintFunction)(
	dsPhysicsEngine* engine, dsAllocator* allocator, bool enabled,
	const dsPhysicsActor* firstActor, const dsVector3f* firstPosition,
	const dsQuaternion4f* firstRotation, const dsPhysicsActor* secondActor,
	const dsVector3f* secondPosition, const dsQuaternion4f* secondRotation, bool limitEnabled,
	float minAngle, float maxAngle, float limitStiffness, float limitDamping,
	dsPhysicsConstraintMotorType motorType, float motorTarget, float maxTorque);

/**
 * @brief Function to destroy a revolute physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
typedef bool (*dsDestroyRevolutePhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsRevolutePhysicsConstraint* constraint);

/**
 * @brief Function to set the angle limits on a revolute physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to set the angle limits on.
 * @param minAngle The minimum angle for the rotation.
 * @param maxAngle The maximum angle for the rotation.
 * @param limitStiffness The spring stiffness applied when limiting the angle.
 * @param limitDamping The spring damping applied when limiting the angle.
 * @return False if the angle limits couldn't be set.
 */
typedef bool (*dsSetRevolutePhysicsConstraintLimitFunction)(dsPhysicsEngine* engine,
	dsRevolutePhysicsConstraint* constraint, float minAngle, float maxAngle, float limitStiffness,
	float limitDamping);

/**
 * @brief Function to disable the angle limits on a revolute physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to disable the angle limits.
 * @return False if the angle limits couldn't be disabled.
 */
typedef bool (*dsDisableRevolutePhysicsConstraintLimitFunction)(dsPhysicsEngine* engine,
	dsRevolutePhysicsConstraint* constraint);

/**
 * @brief Function to set the motor parameters for a revolute  physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to set the max angle on.
 * @param motorType The type of motor to use.
 * @param target The target angle or rotational velocity for the motor.
 * @param maxTorque The maximum torque to apply for the motor.
 * @return False if the motor parameters couldn't be set.
 */
typedef bool (*dsSetRevolutePhysicsConstraintMotorFunction)(dsPhysicsEngine* engine,
	dsRevolutePhysicsConstraint* constraint, dsPhysicsConstraintMotorType motorType, float target,
	float maxTorque);

#ifdef __cplusplus
}
#endif

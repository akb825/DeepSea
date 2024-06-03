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
 * @brief Constant for the number of degrees of freedom for a physics constraint.
 */
#define DS_PHYSICS_CONSTRAINT_DOF_COUNT 6

/**
 * @brief Enum for the type of motor to apply to a physics constraint.
 */
typedef enum dsPhysicsConstraintMotorType
{
	dsPhysicsConstraintMotorType_Disabled, ///< The motor is disabled and doesn't apply force.
	/** Forces are applied to reach a target position or orientation. */
	dsPhysicsConstraintMotorType_Position,
	dsPhysicsConstraintMotorType_Velocity  ///< Forces are applied to reach a target velocity.
} dsPhysicsConstraintMotorType;

/**
 * @brief Enum for the type of a physics constraint limit.
 */
typedef enum dsPhysicsConstraintLimitType
{
	dsPhysicsConstraintLimitType_Fixed, ///< The limit is fixed and unmoving.
	dsPhysicsConstraintLimitType_Free,  ///< The limit is free and unchecked.
	dsPhysicsConstraintLimitType_Range  ///< The limit is checked within a range.
} dsPhysicsConstraintLimitType;

/**
 * @brief Enum for a degree of freedom of a physics constraint.
 */
typedef enum dsPhysicsConstraintDOF
{
	dsPhysicsConstraintDOF_TranslateX, ///< Translation along the X axis.
	dsPhysicsConstraintDOF_TranslateY, ///< Translation along the Y axis.
	dsPhysicsConstraintDOF_TranslateZ, ///< Translation along the Z axis.
	dsPhysicsConstraintDOF_RotateX,    ///< Rotation along the X axis, or part of the swing.
	dsPhysicsConstraintDOF_RotateY,    ///< Rotation along the Y axis, or part of the swing.
	dsPhysicsConstraintDOF_RotateZ     ///< Rotation along the Z axis, or the twist.
} dsPhysicsConstraintDOF;

/// @cond
typedef struct dsPhysicsScene dsPhysicsScene;
/// @endcond

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
 * @brief Function to set whether a physics constraint is enabled.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constrant to set the enabled state on.
 * @param enabled Whether the constraint is enabled and will be enforced.
 * @return False if the enabled state couldn't be set.
 */
typedef bool (*dsSetPhysicsConstraintEnabledFunction)(dsPhysicsEngine* engine,
	dsPhysicsConstraint* constraint, bool enabled);

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
 * Constraints place requirements of the positions and orientations of physics actors relative to
 * each-other. Forces will be applied to the actors to ensure the constraints are satisfied to the
 * best of the physics engine's capabilities.
 *
 * Actors do not track what constraints they are used with. Callers are responsible for ensuring
 * that all constraints that reference an actor are removed before that actor is removed.
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
	 * @brief The physics scene the constraint is a member of, or NULL if not associated with a
	 *     scene.
	 *
	 * The constraint may only be associated at most one scene at a time
	 */
	dsPhysicsScene* scene;

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
	 * @brief Function to set whether the constraint is enabled.
	 */
	dsSetPhysicsConstraintEnabledFunction setEnabledFunc;

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
 * the twist. A motor may be applied to rotate towards a goal rotation, and a force may be applied
 * to stop movement when the motor is disabled.
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
	dsQuaternion4f motorTargetRotation;

	/**
	 * @brief The maximum torque for the motor.
	 *
	 * If the motor is disabled this is the maximum amount of torque to apply to stop motion.
	 */
	float maxMotorTorque;
} dsSwingTwistPhysicsConstraint;

/**
 * @brief Struct describing a revolute physics constraint, or constraint that can rotate around an
 *     arbitrary axis.
 *
 * This may be used to represent a hinge or axle depending on whether an angle limit is enabled.
 * Spring parameters may be used to determine how soft the limit is when enabled. A motor may be
 * used to reach a target location or velocity, and a torque may be applied to stop rotation when
 * the motor is disabled.
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
	 * If the motor is disabled this is the maximum amount of torque to apply to stop motion.
	 */
	float maxMotorTorque;
} dsRevolutePhysicsConstraint;

/**
 * @brief Struct describing a distance physics constraint, which keeps two actors within a distance
 *     range of each-other.
 *
 * This generally models a spring connecting both objects with no limits on rotation.
 *
 * Positions are relative to the local coordinate space of each actor. The positions are
 * immutable, so changing the attachment location and orientation requires creating a new
 * constraint. The limiting angles and spring parameters may be adjusted after creation.
 *
 * @see DistancePhysicsConstraint.h
 */
typedef struct dsDistancePhysicsConstraint
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
	 * @brief The minimum distance between reference points.
	 */
	float minDistance;

	/**
	 * @brief The maximum distance between reference points.
	 */
	float maxDistance;

	/**
	 * @brief The stiffness for the spring to keep within the distance range.
	 */
	float limitStiffness;

	/**
	 * @brief The damping for the spring to keep within the distance range.
	 */
	float limitDamping;
} dsDistancePhysicsConstraint;

/**
 * @brief Struct describing a slider physics constraint, which limits movement along a single axis
 *     with no rotation.
 *
 * The distance between points may optionally be limited with spring parameters to adjust the
 * limit's softness. A motor may also optionally be enabled to reach a target distance or velocity,
 * and a force may be applied to stop motion when the motor is disabled.
 *
 * Transforms are relative to the local coordinate space of each actor. The transforms are
 * immutable, so changing the attachment location and orientation requires creating a new
 * constraint. The limits, spring parameters, and motor may be adjusted after creation.
 *
 * @see SliderPhysicsConstraint.h
 */
typedef struct dsSliderPhysicsConstraint
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
	 * The slider will be limited to the axis of the quaternion.
	 */
	dsQuaternion4f firstRotation;

	/**
	 * @brief The rotation of the constraint relative to the second actor.
	 *
	 * The slider will be limited to the axis of the quaternion.
	 */
	dsQuaternion4f secondRotation;

	/**
	 * @brief Whether the distance limit is enabled.
	 */
	bool limitEnabled;

	/**
	 * @brief The minimum distance between reference points.
	 *
	 * This may be negative to have the two reference points pass beyond each-other.
	 */
	float minDistance;

	/**
	 * @brief The maximum distance between reference points.
	 */
	float maxDistance;

	/**
	 * @brief The stiffness for the spring to keep within the distance range.
	 */
	float limitStiffness;

	/**
	 * @brief The damping for the spring to keep within the distance range.
	 */
	float limitDamping;

	/**
	 * @brief The type of the motor to apply to the constraint.
	 */
	dsPhysicsConstraintMotorType motorType;

	/**
	 * @brief The target for the motor.
	 *
	 * This will be an distance if motorType is dsPhysicsConstraintMotorType_Position or velocity if
	 * motorType is dsPhysicsConstraintMotorType_Velocity.
	 */
	float motorTarget;

	/**
	 * @brief The maximum force for the motor.
	 *
	 * If the motor is disabled this is the maximum amount of force to apply to stop motion.
	 */
	float maxMotorForce;
} dsSliderPhysicsConstraint;

/**
 * @brief Struct describing a limit for a single degree of freedom of a generic physics constraint.
 *
 * @see dsGenericPhysicsConstraint
 * @see GenericPhysicsConstraint.h
 */
typedef struct dsGenericPhysicsConstraintLimit
{
	/**
	 * @brief The type of the limit.
	 */
	dsPhysicsConstraintLimitType limitType;

	/**
	 * @brief The minimum value of the limit.
	 *
	 * For angles, this can be in the range [-pi, pi].
	 */
	float minValue;

	/**
	 * @brief The maximum value of the limit.
	 *
	 * For angles, this can be in the range [-pi, pi].
	 */
	float maxValue;

	/**
	 * @brief The stiffness for the spring when the range is limited.
	 */
	float stiffness;

	/**
	 * @brief The damping for the spring in the range [0, 1] when the range is limited.
	 */
	float damping;
} dsGenericPhysicsConstraintLimit;

/**
 * @brief Struct describing a motor for a single degree of freedom of a generic physics constraint.
 *
 * @see dsGenericPhysicsConstraint
 * @see GenericPhysicsConstraint.h
 */
typedef struct dsGenericPhysicsConstraintMotor
{
	/**
	 * @brief The type of the motor to apply to the degree of freedom.
	 */
	dsPhysicsConstraintMotorType motorType;

	/**
	 * @brief The target of the motor, either as a position or a velocity.
	 */
	float target;

	/**
	 * @brief The maximum force or torque of the motor.
	 *
	 * If the motor is disabled this is the maximum amount of force to apply to stop motion.
	 */
	float maxForce;
} dsGenericPhysicsConstraintMotor;

/**
 * @brief Struct describing a generic physics constraint, which provides control over all 6 degrees
 *     of freedom. (3 translation axes and 3 rotation axes)
 *
 * Most physics constraints may be modeled using a dsGenericPhysicsConstraint, though the
 * specialized constraints are typically faster and may be more stable. Each degree may be fixed,
 * free without limits, or limited within a range with spring parameters for the limits. A motor may
 * also optionally be enabled to reach a target position or velocity for each degree, and a force
 * may be applied to stop motion when the motor is disabled. The motor may be set individually for
 * each translational axis, and either for the swing and twist separately or for all angles together
 * for the rotational axes.
 *
 * Transforms are relative to the local coordinate space of each actor. The transforms are
 * immutable, so changing the attachment location and orientation requires creating a new
 * constraint. The limits, spring parameters, and motors may be adjusted after creation.
 *
 * @see GenericPhysicsConstraint.h
 */
typedef struct dsGenericPhysicsConstraint
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
	 * @brief The limits for each degree of freedom for the constraint.
	 */
	dsGenericPhysicsConstraintLimit limits[DS_PHYSICS_CONSTRAINT_DOF_COUNT];

	/**
	 * @brief The motors for each degree of freedom of the constraint.
	 *
	 * The motor type and maximum torque for RotationX will apply for RotationY as well. If
	 * combineSwingTwistMotors is true, the motor type and maximum torque of RotationX will also
	 * apply to RotationZ.
	 */
	dsGenericPhysicsConstraintMotor motors[DS_PHYSICS_CONSTRAINT_DOF_COUNT];

	/**
	 * @brief Whether the swing and twist motors are combined.
	 */
	bool combineSwingTwistMotors;
} dsGenericPhysicsConstraint;

/**
 * @brief Struct describing a gear physics constraint, ensuring the rotation of two actors are
 *     locked based on the gear ratio.
 *
 * This expects that each actor has revolute constraint to limit movement to a single axis. The
 * revolute constraints may optionally be provided to improve precision and avoid the relative
 * rotations drifting over time.
 *
 * Axes are relative to the local coordinate space of each actor. The axes are immutable, so
 * changing the rotation axes requires creating a new constraint. The ratio may be adjusted after
 * creation.
 *
 * @see GearPhysicsConstraint.h
 */
typedef struct dsGearPhysicsConstraint
{
	/**
	 * @brief The base constraint type.
	 */
	dsPhysicsConstraint constraint;

	/**
	 * @brief The axis of rotation for the first actor.
	 */
	dsVector3f firstAxis;

	/**
	 * @brief The axis of rotation for the second actor.
	 */
	dsVector3f secondAxis;

	/**
	 * @brief The slider constraint for the first actor.
	 */
	const dsRevolutePhysicsConstraint* firstConstraint;

	/**
	 * @brief The revolute constraint for the second actor.
	 */
	const dsRevolutePhysicsConstraint* secondConstraint;

	/**
	 * @brief The gear ratio.
	 *
	 * The ratio is defined as firstActorToothCount/secondActorToothCount. The ratio may be negative
	 * if the axes are flipped.
	 */
	float ratio;
} dsGearPhysicsConstraint;

/**
 * @brief Struct describing a rack and pinion physics constraint, ensuring the translation of a rack
 *     and rotation of a pinion are locked based on the gear ratio.
 *
 * The first actor will correspond to the rack, while the second actor will correspond to the
 * pinion.
 *
 * This expects that the rack has a slider constraint to limit translation along a single axis and
 * the pinion has a revolute constraint to limit the rotation along a single axis. The onstraints
 * may optionally be provided to improve precision and avoid the relative position and rotation
 * drifting over time.
 *
 * Axes are relative to the local coordinate space of each actor. The axes are immutable, so
 * changing the translation and rotation axes requires creating a new constraint. The ratio may be
 * adjusted after creation.
 *
 * @see RackAndPinionPhysicsConstraint.h
 */
typedef struct dsRackAndPinionPhysicsConstraint
{
	/**
	 * @brief The base constraint type.
	 */
	dsPhysicsConstraint constraint;

	/**
	 * @brief The axis of translation for the first actor.
	 */
	dsVector3f firstAxis;

	/**
	 * @brief The axis of rotation for the second actor.
	 */
	dsVector3f secondAxis;

	/**
	 * @brief The revolute constraint for the first actor.
	 */
	const dsSliderPhysicsConstraint* firstConstraint;

	/**
	 * @brief The revolute constraint for the second actor.
	 */
	const dsRevolutePhysicsConstraint* secondConstraint;

	/**
	 * @brief The gear ratio.
	 *
	 * The ratio is defined as 2*PI*rackToothCount/(rackLength*pinionToothCount). The ratio may be
	 * negative if the axes are flipped.
	 */
	float ratio;
} dsRackAndPinionPhysicsConstraint;

/**
 * @brief Function to create a fixed physics constraint.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with.
 * @param firstActor The first physics actor the constraint is attached to.
 * @param firstPosition The position of the constraint on the first actor.
 * @param firstRotation The rotation of the constraint on the first actor.
 * @param secondActor The second physics actor the constraint is attached to.
 * @param secondPosition The position of the constraint on the second actor.
 * @param secondRotation The rotation of the constraint on the second actor.
 * @return The fixed constraint or NULL if it couldn't be created.
 */
typedef dsFixedPhysicsConstraint* (*dsCreateFixedPhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsActor* firstActor,
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
 * @brief Function to set whether a fixed physics constraint is enabled.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constrant to set the enabled state on.
 * @param enabled Whether the constraint is enabled and will be enforced.
 * @return False if the enabled state couldn't be set.
 */
typedef bool (*dsSetFixedPhysicsConstraintEnabledFunction)(dsPhysicsEngine* engine,
	dsFixedPhysicsConstraint* constraint, bool enabled);

/**
 * @brief Function to get the applied force for a fixed physics constraint.
 * @param[out] outForce The force applied to the constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constraint to get the force for.
 * @return False if the force couldn't be queried.
 */
typedef bool (*dsGetFixedPhysicsConstraintForceFunction)(dsVector3f* outForce,
	dsPhysicsEngine* engine, const dsFixedPhysicsConstraint* constraint);

/**
 * @brief Function to create a point physics constraint.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with.
 * @param firstActor The first physics actor the constraint is attached to.
 * @param firstPosition The position of the constraint on the first actor.
 * @param secondActor The second physics actor the constraint is attached to.
 * @param secondPosition The position of the constraint on the second actor.
 * @return The point constraint or NULL if it couldn't be created.
 */
typedef dsPointPhysicsConstraint* (*dsCreatePointPhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsActor* firstActor,
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
 * @brief Function to set whether a point physics constraint is enabled.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constrant to set the enabled state on.
 * @param enabled Whether the constraint is enabled and will be enforced.
 * @return False if the enabled state couldn't be set.
 */
typedef bool (*dsSetPointPhysicsConstraintEnabledFunction)(dsPhysicsEngine* engine,
	dsPointPhysicsConstraint* constraint, bool enabled);

/**
 * @brief Function to get the applied force for a point physics constraint.
 * @param[out] outForce The force applied to the constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constraint to get the force for.
 * @return False if the force couldn't be queried.
 */
typedef bool (*dsGetPointPhysicsConstraintForceFunction)(dsVector3f* outForce,
	dsPhysicsEngine* engine, const dsPointPhysicsConstraint* constraint);

/**
 * @brief Function to create a cone physics constraint.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with.
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
	dsAllocator* allocator, const dsPhysicsActor* firstActor, const dsVector3f* firstPosition,
	const dsQuaternion4f* firstRotation, const dsPhysicsActor* secondActor,
	const dsVector3f* secondPosition, const dsQuaternion4f* secondRotation, float maxAngle);

/**
 * @brief Function to destroy a cone physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
typedef bool (*dsDestroyConePhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsConePhysicsConstraint* constraint);

/**
 * @brief Function to set whether a cone physics constraint is enabled.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constrant to set the enabled state on.
 * @param enabled Whether the constraint is enabled and will be enforced.
 * @return False if the enabled state couldn't be set.
 */
typedef bool (*dsSetConePhysicsConstraintEnabledFunction)(dsPhysicsEngine* engine,
	dsConePhysicsConstraint* constraint, bool enabled);

/**
 * @brief Function to get the applied force for a cone physics constraint.
 * @param[out] outForce The force applied to the constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constraint to get the force for.
 * @return False if the force couldn't be queried.
 */
typedef bool (*dsGetConePhysicsConstraintForceFunction)(dsVector3f* outForce,
	dsPhysicsEngine* engine, const dsConePhysicsConstraint* constraint);

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
 * @param motorTargetRotation The target rotation to reach when the motor is enabled.
 * @param maxMotorTorque The maximum torque to apply for the motor.
 * @return The swing twist constraint or NULL if it couldn't be created.
 */
typedef dsSwingTwistPhysicsConstraint* (*dsCreateSwingTwistPhysicsConstraintFunction)(
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstRotation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondRotation, float maxSwingXAngle, float maxSwingYAngle,
	float maxTwistZAngle, dsPhysicsConstraintMotorType motorType,
	const dsQuaternion4f* motorTargetRotation, float maxMotorTorque);

/**
 * @brief Function to destroy a swing twist physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
typedef bool (*dsDestroySwingTwistPhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsSwingTwistPhysicsConstraint* constraint);

/**
 * @brief Function to set whether a swing twist physics constraint is enabled.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constrant to set the enabled state on.
 * @param enabled Whether the constraint is enabled and will be enforced.
 * @return False if the enabled state couldn't be set.
 */
typedef bool (*dsSetSwingTwistPhysicsConstraintEnabledFunction)(dsPhysicsEngine* engine,
	dsSwingTwistPhysicsConstraint* constraint, bool enabled);

/**
 * @brief Function to get the applied force for a swing twist physics constraint.
 * @param[out] outForce The force applied to the constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constraint to get the force for.
 * @return False if the force couldn't be queried.
 */
typedef bool (*dsGetSwingTwistPhysicsConstraintForceFunction)(dsVector3f* outForce,
	dsPhysicsEngine* engine, const dsSwingTwistPhysicsConstraint* constraint);

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
 * @param maxMotorTorque The maximum torque to apply for the motor.
 * @return The revolute constraint or NULL if it couldn't be created.
 */
typedef dsRevolutePhysicsConstraint* (*dsCreateRevolutePhysicsConstraintFunction)(
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstRotation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondRotation, bool limitEnabled, float minAngle, float maxAngle,
	float limitStiffness, float limitDamping, dsPhysicsConstraintMotorType motorType,
	float motorTarget, float maxMotorTorque);

/**
 * @brief Function to destroy a revolute physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
typedef bool (*dsDestroyRevolutePhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsRevolutePhysicsConstraint* constraint);

/**
 * @brief Function to set whether a revolute physics constraint is enabled.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constrant to set the enabled state on.
 * @param enabled Whether the constraint is enabled and will be enforced.
 * @return False if the enabled state couldn't be set.
 */
typedef bool (*dsSetRevolutePhysicsConstraintEnabledFunction)(dsPhysicsEngine* engine,
	dsRevolutePhysicsConstraint* constraint, bool enabled);

/**
 * @brief Function to get the applied force for a revolute physics constraint.
 * @param[out] outForce The force applied to the constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constraint to get the force for.
 * @return False if the force couldn't be queried.
 */
typedef bool (*dsGetRevolutePhysicsConstraintForceFunction)(dsVector3f* outForce,
	dsPhysicsEngine* engine, const dsRevolutePhysicsConstraint* constraint);

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
 * @brief Function to set the motor parameters for a revolute physics constraint.
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

/**
 * @brief Function to create a distance physics constraint.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with.
 * @param firstActor The first physics actor the constraint is attached to.
 * @param firstPosition The position of the constraint on the first actor.
 * @param secondActor The second physics actor the constraint is attached to.
 * @param secondPosition The position of the constraint on the second actor.
 * @param minDistance The minimum distance to keep between reference points.
 * @param maxDistance The maximum distance to keep between reference points.
 * @param limitStiffness The stiffness for the spring for the distance limit.
 * @param limitDamping The damping for the spring for the distance limit.
 * @return The distance constraint or NULL if it couldn't be created.
 */
typedef dsDistancePhysicsConstraint* (*dsCreateDistancePhysicsConstraintFunction)(
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsPhysicsActor* secondActor,
	const dsVector3f* secondPosition, float minDistance, float maxDistance, float limitStiffness,
	float limitDamping);

/**
 * @brief Function to destroy a distance physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
typedef bool (*dsDestroyDistancePhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsDistancePhysicsConstraint* constraint);

/**
 * @brief Function to set whether a distance physics constraint is enabled.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constrant to set the enabled state on.
 * @param enabled Whether the constraint is enabled and will be enforced.
 * @return False if the enabled state couldn't be set.
 */
typedef bool (*dsSetDistancePhysicsConstraintEnabledFunction)(dsPhysicsEngine* engine,
	dsDistancePhysicsConstraint* constraint, bool enabled);

/**
 * @brief Function to get the applied force for a distance physics constraint.
 * @param[out] outForce The force applied to the constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constraint to get the force for.
 * @return False if the force couldn't be queried.
 */
typedef bool (*dsGetDistancePhysicsConstraintForceFunction)(dsVector3f* outForce,
	dsPhysicsEngine* engine, const dsDistancePhysicsConstraint* constraint);

/**
 * @brief Function to set the limits for a distance physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to set the limits on.
 * @param minDistance The minimum distance to keep between reference points.
 * @param maxDistance The maximum distance to keep between reference points.
 * @param limitStiffness The stiffness for the spring.
 * @param limitDamping The damping for the spring.
 * @return False if the limits couldn't be set.
 */
typedef bool (*dsSetDistancePhysicsConstraintLimitFunction)(dsPhysicsEngine* engine,
	dsDistancePhysicsConstraint* constraint, float minDistance, float maxDistance,
	float limitStiffness, float limitDamping);

/**
 * @brief Function to create a slider physics constraint.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with.
 * @param firstActor The first physics actor the constraint is attached to.
 * @param firstPosition The position of the constraint on the first actor.
 * @param firstRotation The rotation of the constraint on the first actor.
 * @param secondActor The second physics actor the constraint is attached to.
 * @param secondPosition The position of the constraint on the second actor.
 * @param secondRotation The rotation of the constraint on the second actor.
 * @param limitEnabled Whether the distance limit is enabled.
 * @param minDistance The minimum distance between reference points.
 * @param maxDistance The maximum distance between reference points.
 * @param limitStiffness The spring stiffness applied when limiting the distance.
 * @param limitDamping The spring damping applied when limiting the distance.
 * @param motorType The type of motor to use.
 * @param motorTarget The target of the motor, either as a distance or velocity.
 * @param maxMotorForce The maximum force to apply for the motor.
 * @return The slider constraint or NULL if it couldn't be created.
 */
typedef dsSliderPhysicsConstraint* (*dsCreateSliderPhysicsConstraintFunction)(
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstRotation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondRotation, bool limitEnabled, float minDistance, float maxDistance,
	float limitStiffness, float limitDamping, dsPhysicsConstraintMotorType motorType,
	float motorTarget, float maxMotorForce);

/**
 * @brief Function to destroy a slider physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
typedef bool (*dsDestroySliderPhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsSliderPhysicsConstraint* constraint);

/**
 * @brief Function to set whether a slider physics constraint is enabled.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constrant to set the enabled state on.
 * @param enabled Whether the constraint is enabled and will be enforced.
 * @return False if the enabled state couldn't be set.
 */
typedef bool (*dsSetSliderPhysicsConstraintEnabledFunction)(dsPhysicsEngine* engine,
	dsSliderPhysicsConstraint* constraint, bool enabled);

/**
 * @brief Function to get the applied force for a slider physics constraint.
 * @param[out] outForce The force applied to the constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constraint to get the force for.
 * @return False if the force couldn't be queried.
 */
typedef bool (*dsGetSliderPhysicsConstraintForceFunction)(dsVector3f* outForce,
	dsPhysicsEngine* engine, const dsSliderPhysicsConstraint* constraint);

/**
 * @brief Function to set the limits for a slider physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to set the limits on.
 * @param minDistance The minimum distance to keep between reference points.
 * @param maxDistance The maximum distance to keep between reference points.
 * @param limitStiffness The stiffness for the spring.
 * @param limitDamping The damping for the spring.
 * @return False if the limits couldn't be set.
 */
typedef bool (*dsSetSliderPhysicsConstraintLimitFunction)(dsPhysicsEngine* engine,
	dsSliderPhysicsConstraint* constraint, float minDistance, float maxDistance,
	float limitStiffness, float limitDamping);

/**
 * @brief Function to disable the distance limits on a slider physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to disable the distance limits.
 * @return False if the distance limits couldn't be disabled.
 */
typedef bool (*dsDisableSliderPhysicsConstraintLimitFunction)(dsPhysicsEngine* engine,
	dsSliderPhysicsConstraint* constraint);

/**
 * @brief Function to set the motor parameters for a slider physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to set the max angle on.
 * @param motorType The type of motor to use.
 * @param target The target distance or velocity for the motor.
 * @param maxForce The maximum force to apply for the motor.
 * @return False if the motor parameters couldn't be set.
 */
typedef bool (*dsSetSliderPhysicsConstraintMotorFunction)(dsPhysicsEngine* engine,
	dsSliderPhysicsConstraint* constraint, dsPhysicsConstraintMotorType motorType, float target,
	float maxForce);

/**
 * @brief Function to create a generic physics constraint.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with.
 * @param firstActor The first physics actor the constraint is attached to.
 * @param firstPosition The position of the constraint on the first actor.
 * @param firstRotation The rotation of the constraint on the first actor.
 * @param secondActor The second physics actor the constraint is attached to.
 * @param secondPosition The position of the constraint on the second actor.
 * @param secondRotation The rotation of the constraint on the second actor.
 * @param limits The limits for each degree of freedom.
 * @param motors The motors for each degree of freedom.
 * @param combineSwingTwistMotors Whether the swing and twist motors are combined.
 * @return The generic constraint or NULL if it couldn't be created.
 */
typedef dsGenericPhysicsConstraint* (*dsCreateGenericPhysicsConstraintFunction)(
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstRotation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondRotation,
	const dsGenericPhysicsConstraintLimit limits[DS_PHYSICS_CONSTRAINT_DOF_COUNT],
	const dsGenericPhysicsConstraintMotor motors[DS_PHYSICS_CONSTRAINT_DOF_COUNT],
	bool combineSwingTwistMotors);

/**
 * @brief Function to destroy a generic physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
typedef bool (*dsDestroyGenericPhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsGenericPhysicsConstraint* constraint);

/**
 * @brief Function to set whether a generic physics constraint is enabled.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constrant to set the enabled state on.
 * @param enabled Whether the constraint is enabled and will be enforced.
 * @return False if the enabled state couldn't be set.
 */
typedef bool (*dsSetGenericPhysicsConstraintEnabledFunction)(dsPhysicsEngine* engine,
	dsGenericPhysicsConstraint* constraint, bool enabled);

/**
 * @brief Function to get the applied force for a generic physics constraint.
 * @param[out] outForce The force applied to the constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constraint to get the force for.
 * @return False if the force couldn't be queried.
 */
typedef bool (*dsGetGenericPhysicsConstraintForceFunction)(dsVector3f* outForce,
	dsPhysicsEngine* engine, const dsGenericPhysicsConstraint* constraint);

/**
 * @brief Function to set the limit for a degree of freedon of a generic physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to set the limit on.
 * @param dof The degree of freedom to set the limit for.
 * @param limitType The type of the limit.
 * @param minValue The minimum value of the limit.
 * @param maxValue The maximum value of the limit.
 * @param stiffness The stiffness when the limited by range.
 * @param damping The damping when the limited by range.
 * @return False if the limit couldn't be set.
 */
typedef bool (*dsSetGenericPhysicsConstraintLimitFunction)(dsPhysicsEngine* engine,
	dsGenericPhysicsConstraint* constraint, dsPhysicsConstraintDOF dof,
	dsPhysicsConstraintLimitType limitType, float minValue, float maxValue, float stiffness,
	float damping);

/**
 * @brief Function to set the motor for a degree of freedom of a generic physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to set the motor on.
 * @param dof The degree of freedom to set the motor for.
 * @param motorType The type of the motor.
 * @param target The target of the motor, either a position or velocity.
 * @param maxForce The maximum force to apply for the motor.
 * @return False if the motor couldn't be set.
 */
typedef bool (*dsSetGenericPhysicsConstraintMotorFunction)(dsPhysicsEngine* engine,
	dsGenericPhysicsConstraint* constraint, dsPhysicsConstraintDOF dof,
	dsPhysicsConstraintMotorType motorType, float target, float maxForce);

/**
 * @brief Function to set whether the swing and twist motors are combined for a generic physics
 *     constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to set the combine swing twist state on.
 * @param combineSwingTwist Whether the swing and twist motors should be combined.
 * @return False if the combine swing twist state couldn't be set.
 */
typedef bool (*dsSetGenericPhysicsConstraintCombineSwingTwistMotorFunction)(dsPhysicsEngine* engine,
	dsGenericPhysicsConstraint* constraint, bool combineSwingTwist);

/**
 * @brief Function to create a gear constraint.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with.
 * @param firstActor The first physics actor the constraint is attached to.
 * @param firstAxis The axis of rotation for the first actor.
 * @param firstConstraint The revolute constraint for the first actor.
 * @param secondActor The second physics actor the constraint is attached to.
 * @param secondAxis The axis of rotation for the second actor.
 * @param secondConstraint The revolute constraint for the second actor.
 * @param ratio The gear ratio between the two actors.
 * @return The gear constraint or NULL if it couldn't be created.
 */
typedef dsGearPhysicsConstraint* (*dsCreateGearPhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsActor* firstActor, const dsVector3f* firstAxis,
	const dsRevolutePhysicsConstraint* firstConstraint, const dsPhysicsActor* secondActor,
	const dsVector3f* secondAxis, const dsRevolutePhysicsConstraint* secondConstraint, float ratio);

/**
 * @brief Function to destroy a gear physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
typedef bool (*dsDestroyGearPhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsGearPhysicsConstraint* constraint);

/**
 * @brief Function to set whether a gear physics constraint is enabled.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constrant to set the enabled state on.
 * @param enabled Whether the constraint is enabled and will be enforced.
 * @return False if the enabled state couldn't be set.
 */
typedef bool (*dsSetGearPhysicsConstraintEnabledFunction)(dsPhysicsEngine* engine,
	dsGearPhysicsConstraint* constraint, bool enabled);

/**
 * @brief Function to get the applied force for a gear physics constraint.
 * @param[out] outForce The force applied to the constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constraint to get the force for.
 * @return False if the force couldn't be queried.
 */
typedef bool (*dsGetGearPhysicsConstraintForceFunction)(dsVector3f* outForce,
	dsPhysicsEngine* engine, const dsGearPhysicsConstraint* constraint);

/**
 * @brief Function to set the gear ratio for a gear physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to set the gear ratio on.
 * @param ratio The gear ratio.
 * @return False if the gear ratio couldn't be set.
 */
typedef bool (*dsSetGearPhysicsConstraintRatioFunction)(dsPhysicsEngine* engine,
	dsGearPhysicsConstraint* constraint, float ratio);

/**
 * @brief Function to create a rack and pinion constraint.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with.
 * @param rackActor The physics actor for the rack the constraint is attached to.
 * @param rackAxis The axis of translation for the rack actor.
 * @param rackConstraint The slider constraint for the rack actor.
 * @param pinionActor The physics actor for the pinion the constraint is attached to.
 * @param pinionAxis The axis of rotation for the pinion actor.
 * @param pinionConstraint The revolute constraint for the pinion actor.
 * @param ratio The gear ratio between the two actors.
 * @return The rack and pinion constraint or NULL if it couldn't be created.
 */
typedef dsRackAndPinionPhysicsConstraint* (*dsCreateRackAndPinionPhysicsConstraintFunction)(
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* rackActor,
	const dsVector3f* rackAxis, const dsSliderPhysicsConstraint* rackConstraint,
	const dsPhysicsActor* pinionActor, const dsVector3f* pinionAxis,
	const dsRevolutePhysicsConstraint* pinionConstraint, float ratio);

/**
 * @brief Function to destroy a rack and pinion physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
typedef bool (*dsDestroyRackAndPinionPhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsRackAndPinionPhysicsConstraint* constraint);

/**
 * @brief Function to set whether a rack and pinion physics constraint is enabled.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constrant to set the enabled state on.
 * @param enabled Whether the constraint is enabled and will be enforced.
 * @return False if the enabled state couldn't be set.
 */
typedef bool (*dsSetRackAndPinionPhysicsConstraintEnabledFunction)(dsPhysicsEngine* engine,
	dsRackAndPinionPhysicsConstraint* constraint, bool enabled);

/**
 * @brief Function to get the applied force for a rack and pinion physics constraint.
 * @param[out] outForce The force applied to the constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constraint to get the force for.
 * @return False if the force couldn't be queried.
 */
typedef bool (*dsGetRackAndPinionPhysicsConstraintForceFunction)(dsVector3f* outForce,
	dsPhysicsEngine* engine, const dsRackAndPinionPhysicsConstraint* constraint);

/**
 * @brief Function to set the rack and pinion ratio for a gear physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to set the gear ratio on.
 * @param ratio The gear ratio.
 * @return False if the gear ratio couldn't be set.
 */
typedef bool (*dsSetRackAndPinionPhysicsConstraintRatioFunction)(dsPhysicsEngine* engine,
	dsRackAndPinionPhysicsConstraint* constraint, float ratio);

#ifdef __cplusplus
}
#endif

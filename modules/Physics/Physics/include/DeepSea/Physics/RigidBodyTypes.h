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
#include <DeepSea/Core/Types.h>
#include <DeepSea/Math/Types.h>
#include <DeepSea/Physics/Shapes/Types.h>
#include <DeepSea/Physics/SharedTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes types used for rigid bodies and functions pointers for the implementation.
 */

/**
 * @brief Enum for flags to control the behavior of rigid bodies.
 *
 * Flags may be toggled after creation unless otherwise specified.
 */
typedef enum dsRigidBodyFlags
{
	dsRigidBodyFlags_Default = 0,  ///< Default behavior.
	/** Can change the motion type. This flag can't be changed after creation. */
	dsRigidBodyFlags_MutableMotionType = 0x1,
	/**
	 * Shapes may be added, removed, or transformed after creation. This flag can't be changed after
	 * creation.
	 */
	dsRigidBodyFlags_MutableShape = 0x2,
	/** Allow the body to be scaled. This flag can't be changed after creation. */
	dsRigidBodyFlags_Scalable = 0x4,
	/** Use linear collision to avoid fast-moving objects missing collisions. */
	dsRigidBodyFlags_LinearCollision = 0x8,
	dsRigidBodyFlags_Sensor = 0x10,             ///< Detect collisions but don't interact.
	dsRigidBodyFlags_SensorDetectStatic = 0x20, ///< Allow detecting static objects as a sensor.
	/** Always consider the body to be active, not allowing it to go to sleep. */
	dsRigidBodyFlags_AlwaysActive = 0x40,
	dsRigidBodyFlags_DisableGravity = 0x80,    ///< Disable gravity for the body.
	dsRigidBodyFlags_GyroscopicForces = 0x100, ///< Apply gyroscopic forces to the body.
	/** Avoid combining similar contact points from the same collision pair. */
	dsRigidBodyFlags_AllContacts = 0x200,
	dsRigidBodyFlags_CustomContactProperties = 0x400 ///< Contact properties may be overridden.
} dsRigidBodyFlags;

/**
 * @brief Function to check whether two collision groups may collide.
 * @param firstGroup The first collision group.
 * @param secondGroup The second collision group.
 * @return True if the groups may collide.
 */
typedef bool (*dsCanCollisionGroupsCollideFunction)(uint64_t firstGroup, uint64_t secondGroup);

/**
 * @brief Struct describing the initialization parameters for a rigid body.
 *
 * This groups together the body-specific parameters for creation for easier creation. Convenience
 * functions may be used to set commonly changed values while leaving others at default.
 *
 * @see RigidBodyInit.h
 */
typedef struct dsRigidBodyInit
{
	/**
	 * @brief User data associated with the rigid body.
	 */
	void* userData;

	/**
	 * @brief Function to destroy the user data.
	 *
	 * This will be called even if the creation of the body fails.
	 */
	dsDestroyUserDataFunction destroyUserDataFunc;

	/**
	 * @brief Flags to control the behavior of the rigid body.
	 */
	dsRigidBodyFlags flags;

	/**
	 * @brief The type of motion for the rigid body.
	 */
	dsPhysicsMotionType motionType;

	/**
	 * @brief The mask of degrees of freedom the simulation may modify.
	 */
	dsPhysicsDOFMask dofMask;

	/**
	 * @brief The layer the rigid body is associated with.
	 * @remark The layer cannot be changed after creation.
	 */
	dsPhysicsLayer layer;

	/**
	 * @brief Collision group ID that the body belongs to.
	 */
	uint64_t collisionGroup;

	/**
	 * @brief Function to check whether two collision groups can collide.
	 *
	 * When checking a pair of intersecting bodies, they will collide if both set this function
	 * to NULL or the function returns true. Behavior is undefined if the function is set on both
	 * bodies and would return true for one body but false the other.
	 */
	dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc;

	/**
	 * @brief The position of the body in world space.
	 */
	dsVector3f position;

	/**
	 * @brief The orientation of the body in world space.
	 */
	dsQuaternion4f orientation;

	/**
	 * @brief The scale factor of the body.
	 *
	 * This will only be used if dsRigidBodyFlags_Scalable is set.
	 */
	dsVector3f scale;

	/**
	 * @brief The initial linear velocity of the body.
	 */
	dsVector3f linearVelocity;

	/**
	 * @brief The initial angular velocity of the body.
	 */
	dsVector3f angularVelocity;

	/**
	 * @brief The coefficient of friction, with 0 meaning no friction and increasing values having
	 * higher friction.
	 */
	float friction;

	/**
	 * @brief The restitution value, where 0 is fully inelastic and 1 is fully elastic.
	 */
	float restitution;

	/**
	 * @brief Linear damping factor in the range [0, 1] to reduce the velocity over time.
	 *
	 * Defaults to a small value to avoid moving forever.
	 */
	float linearDamping;

	/**
	 * @brief Angular damping factor in the range [0, 1] to reduce the angular velocity over time.
	 *
	 * Defaults to a small value to avoid moving forever.
	 */
	float angularDamping;

	/**
	 * @brief The maximum linear velocity.
	 *
	 * Defaults to a large value to avoid simulation instability.
	 */
	float maxLinearVelocity;

	/**
	 * @brief The maximum angular velocity in radians/s.
	 *
	 * Defaults to a large value to avoid simulation instability.
	 */
	float maxAngularVelocity;

	/**
	 * @brief The expected number of shapes.
	 *
	 * If the number of shapes is known ahead of time, this may be set to a non-zero number to
	 * allocate space for those shapes ahead of time. This doesn't limit the number of final shapes
	 * that may be added.
	 */
	uint32_t shapeCount;
} dsRigidBodyInit;

/**
 * @brief Struct describing a rigid body for use by physics simulations.
 *
 * Rigid bodies may not be deformed, with the shape remaining the same as they are simulated, though
 * a limited form of deformation may be achieved by connecting multiple rigid bodies with
 * constraints.
 *
 * Physics implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsRigidBody and the true internal type.
 *
 * After creation, one or more shapes must be added with dsRigidBody_addShape(). After all component
 * shapes have been added, they must be finalized by calling dsRigidBody_finalizeShapes() or
 * dsRigidBody_finalizeShapesCustomMassProperties().
 *
 * Members that won't be modified during simulation are stored by value for easy access. Members
 * that may be updated on a per-frame basis, such as the velocity, must be queried to avoid
 * unnecessary copies. The exception to this is the position and rotation, since these will almost
 * always be used.
 *
 * THe underlying implementations are responsible for setting and maintaining all members as they
 * are updated. The base interface defined in RigidBody.h makes no assumptions apart from general
 * error checking performed beforehand that changes are valid.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see RigidBody.h
 */
typedef struct dsRigidBody
{
	/**
	 * @brief The physics engine the rigid body was created with.
	 */
	dsPhysicsEngine* engine;

	/**
	 * @brief The allocator the rigid body was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief User data associated with the rigid body.
	 */
	void* userData;

	/**
	 * @brief Function to destroy the user data.
	 */
	dsDestroyUserDataFunction destroyUserDataFunc;

	/**
	 * @brief Flags to control the behavior of the rigid body.
	 */
	dsRigidBodyFlags flags;

	/**
	 * @brief The type of motion for the rigid body.
	 */
	dsPhysicsMotionType motionType;

	/**
	 * @brief The mask of degrees of freedom the simulation may modify.
	 */
	dsPhysicsDOFMask dofMask;

	/**
	 * @brief The layer the rigid body is associated with.
	 */
	dsPhysicsLayer layer;

	/**
	 * @brief Collision group ID that the body belongs to.
	 */
	uint64_t collisionGroup;

	/**
	 * @brief Function to check whether two collision groups can collide.
	 *
	 * When checking a pair of intersecting bodies, they will collide if both set this function
	 * to NULL or the function returns true. Behavior is undefined if the function is set on both
	 * bodies and would return true for one body but false the other.
	 */
	dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc;

	/**
	 * @brief The position of the body in world space.
	 */
	dsVector3f position;

	/**
	 * @brief The orientation of the body in world space.
	 */
	dsQuaternion4f orientation;

	/**
	 * @brief The scale factor of the body.
	 *
	 * This will only be used if dsRigidBodyFlags_Scalable is set.
	 */
	dsVector3f scale;

	/**
	 * @brief The mass properties of the rigid body.
	 *
	 * This isn't modified by the scale, though implementations will internally scale the mass
	 * properties when interfacing with the underlying physics library.
	 *
	 * Implementations may want to initialize this to empty, but otherwise it will be managed by the
	 * base functions in the Physics library.
	 */
	dsPhysicsMassProperties massProperties;

	/**
	 * @brief The coefficient of friction, with 0 meaning no friction and increasing values having
	 * higher friction.
	 */
	float friction;

	/**
	 * @brief The restitution value, where 0 is fully inelastic and 1 is fully elastic.
	 */
	float restitution;

	/**
	 * @brief Linear damping factor in the range [0, 1] to reduce the velocity over time.
	 */
	float linearDamping;

	/**
	 * @brief Angular damping factor in the range [0, 1] to reduce the angular velocity over time.
	 */
	float angularDamping;

	/**
	 * @brief The maximum linear velocity.
	 */
	float maxLinearVelocity;

	/**
	 * @brief The maximum angular velocity in radians/s.
	 */
	float maxAngularVelocity;

	/**
	 * @brief The shapes associated with the body.
	 */
	dsPhysicsShapeInstance* shapes;

	/**
	 * @brief The number of shapes in the body.
	 */
	uint32_t shapeCount;

	/**
	 * @brief The maximum number of shapes before re-allocation is needed.
	 */
	uint32_t maxShapes;

	/**
	 * @brief Whether the shapes have been finalized.
	 *
	 * If false, the rigid body is in an intermediate state that can't be used. Once finalized, the
	 * shapes may only be modified if the dsRigidBodyFlags_MutableShape flag is set.
	 *
	 * Implementations should initialize this to false, but further changes should be left to the
	 * base Physics library.
	 */
	bool shapesFinalized;
} dsRigidBody;

/**
 * @brief Function to create a rigid body.
 * @param engine The physics engine to create the rigid body with.
 * @param allocator The allocator to create the rigid body with.
 * @param initParams The initialization parameters.
 * @return The rigid body or NULL if it couldn't be created.
 */
typedef dsRigidBody* (*dsCreateRigidBodyFunction)(dsPhysicsEngine* engine, dsAllocator* allocator,
	const dsRigidBodyInit* initParams);

/**
 * @brief Function to destroy a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to destroy.
 * @return False if the rigid body couldn't be destroyed.
 */
typedef bool (*dsDestroyRigidBodyFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody);

/**
 * @brief Function to add a shape to a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to add the shape to.
 * @param shape The shape to add.
 * @param translate The translation for the shape or NULL to leave at origin.
 * @param rotate The rotation for the shape or NULL to leave unrotated.
 * @param scale The scale of the shape or NULL to leave unscaled.
 * @param density The density of the shape.
 * @return The ID for the added shape instance or DS_NO_PHYSICS_SHAPE_ID if it couldn't be added.
 */
typedef uint32_t (*dsAddRigidBodyShapeFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	dsPhysicsShape* shape, const dsVector3f* translate, const dsQuaternion4f* rotate,
	const dsVector3f* scale, float density);

/**
 * @brief Function to set the transform for a shape within a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to shape set the shape transform on.
 * @param index The index of the shape.
 * @param translate The new translation or NULL to leave unchanged.
 * @param rotate The new rotation or NULL to leave unchanged.
 * @param scale The new scale or NULL to leave unchanged.
 * @return False if the transform couldn't be changed.
 */
typedef bool (*dsSetRigidBodyShapeTransformFunction)(dsPhysicsEngine* engine,
	dsRigidBody* rigidBody, uint32_t index, const dsVector3f* translate,
	const dsQuaternion4f* rotate, const dsVector3f* scale);

/**
 * @brief Function to remove a shape from a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to remove the shape from.
 * @param index The index of the shape to remove.
 * @return False if the shape couldn't be removed.
 */
typedef bool (*dsRemoveRigidBodyShapeFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	uint32_t index);

/**
 * @brief Function to finalize the shapes on a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to finalize the shapes on.
 * @param massProperties The mass properties for the rigid body.
 * @return False if the shapes couldn't be finalized.
 */
typedef bool (*dsFinalizeRigidBodyShapesFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	const dsPhysicsMassProperties* massProperties);

/**
 * @brief Function to set the flags for a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to set the flags on.
 * @param flags The new flags.
 * @return False if the flags couldn't be applied.
 */
typedef bool (*dsSetRigidBodyFlagsFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	dsRigidBodyFlags flags);

/**
 * @brief Function to set the motion type on a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to change the motion type on.
 * @param motionType The new motion type.
 * @return False if the motion type couldn't be changed.
 */
typedef bool (*dsSetRigidBodyMotionTypeFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	dsPhysicsMotionType motionType);

/**
 * @brief Function to set the degree of freedom mask on a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to change the degrees of freedom on.
 * @param dofMask The new degree of freedom mask.
 * @return False if the degree of freedom mask couldn't be set.
 */
typedef bool (*dsSetRigidBodyDOFMaskFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	dsPhysicsDOFMask dofMask);

/**
 * @brief Function to set the collision group on a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to change the collision group on.
 * @param collisionGroup The new collision group.
 * @return False if the collision group couldn't be set.
 */
typedef bool (*dsSetRigidBodyCollisionGroupFunction)(dsPhysicsEngine* engine,
	dsRigidBody* rigidBody, uint64_t collisionGroup);

/**
 * @brief Function to set the can collision groups collide function on a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to change the can collision groups collide function on.
 * @param canCollideFunc The new can collision groups collide function.
 * @return False if the can collision groups collide function couldn't be set.
 */
typedef bool (*dsSetRigidBodyCanCollisionGroupsCollideFunction)(dsPhysicsEngine* engine,
	dsRigidBody* rigidBody, dsCanCollisionGroupsCollideFunction canCollideFunc);

/**
 * @brief Function to set the transform on a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to change the position on.
 * @param position The new position or NULL to leave unchanged.
 * @param orientation The new orientation or NULL to leave unchanged.
 * @param scale The new scale or NULL to leave unchanged.
 * @return False if the transform couldn't be set.
 */
typedef bool (*dsSetRigidBodyTransformFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	const dsVector3f* position, const dsQuaternion4f* orientation, const dsVector3f* scale);

/**
 * @brief Function to set the mass of a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to change the mass on.
 * @param mass The new mass.
 * @return False if the mass couldn't be set.
 */
typedef bool (*dsSetRigidBodyMassFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	float mass);

/**
 * @brief Function to set the friction of a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to change the friction on.
 * @param friction The new friction.
 * @return False if the friction couldn't be set.
 */
typedef bool (*dsSetRigidBodyFrictionFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	float friction);

/**
 * @brief Function to set the restitution of a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to change the restitution on.
 * @param restitution The new restitution.
 * @return False if the restitution couldn't be set.
 */
typedef bool (*dsSetRigidBodyRestitutionFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	float restitution);

/**
 * @brief Function to set the linear or angular damping of a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to change the linear damping on.
 * @param damping The new damping.
 * @return False if the damping couldn't be set.
 */
typedef bool (*dsSetRigidBodyDampingFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	float damping);

/**
 * @brief Function to set the max linear or angular velocity of a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to change the max linear velocity on.
 * @param maxVelocity The new max velocity.
 * @return False if the max velocity couldn't be set.
 */
typedef bool (*dsSetRigidBodyMaxVelocityFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	float maxVelocity);

/**
 * @brief Function to get the linear or angular velocity from a rigid body.
 * @param[out] outVelocity The velocity to set.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to change the velocity on.
 * @return False if the velocity couldn't be queried.
 */
typedef bool (*dsGetRigidBodyVelocityFunction)(dsVector3f* outVelocity, dsPhysicsEngine* engine,
	const dsRigidBody* rigidBody);

/**
 * @brief Function to set the linear or angular velocity from a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to change the velocity on.
 * @param velocity The new velocity
 * @return False if the velocity couldn't be set.
 */
typedef bool (*dsSetRigidBodyVelocityFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	const dsVector3f* velocity);

/**
 * @brief Function to add a force, torque, or impulse on a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to add the force on.
 * @param force The force to add.
 * @return False if the force couldn't be added.
 */
typedef bool (*dsAddRigidBodyForceFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	const dsVector3f* force);

/**
 * @brief Function to clear the accumulated force, torque, or impulse on a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to clear the force on.
 * @return False if the force couldn't be cleared.
 */
typedef bool (*dsClearRigidBodyForceFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody);

/**
 * @brief Function to get whether a rigid body is active.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to get the active state from.
 * @return Whether the rigid body is active.
 */
typedef bool (*dsGetRigidBodyActiveFunction)(dsPhysicsEngine* engine, const dsRigidBody* rigidBody);

/**
 * @brief Function to set whether a rigid body is active.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to set the active state on.
 * @param active Whether the rigid body is active.
 * @return False if the active state couldn't be set.
 */
typedef bool (*dsSetRigidBodyActiveFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	bool active);

#ifdef __cplusplus
}
#endif

// Needs to be after the extern "C" block.
/// @cond
DS_ENUM_BITMASK_OPERATORS(dsRigidBodyFlags);
/// @endcond

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
 * @brief Default damping value for physics actors.
 */
#define DS_DEFAULT_PHYSICS_DAMPING 0.05f

/**
 * @brief Default maximum linear velocity for physics actors.
 */
#define DS_DEFAULT_PHYSICS_MAX_LINEAR_VELOCITY 500.0f

/**
 * @brief Default maximum angular velocity for physics actors.
 */
#define DS_DEFAULT_PHYSICS_MAX_ANGULAR_VELOCITY 47.123890f

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
	dsRigidBodyFlags_CustomContactProperties = 0x400, ///< Contact properties may be overridden.
	/** Invoke callbacks on the physics scene when it comes into contact with other bodies. */
	dsRigidBodyFlags_ContactCallbacks = 800
} dsRigidBodyFlags;

/// @cond
typedef struct dsRigidBody dsRigidBody;
/// @endcond

/**
 * @brief Struct to group together multiple associated rigid bodies.
 *
 * Rigid bodies may optionally be created as part of a group, in which case they will be added and
 * removed from physics scenes together and may improve the speed of collision checks on some
 * implementations. The rigid bodies are expected to be near each-other, such as connected by
 * constraints.
 *
 * Rigid bodies that are part of a group must share the same motion type, and may not have the
 * dsRigidBodyFlags_MutableMotionType flag set.
 *
 * Physics implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsRigidBodyGroup and the true internal type.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @remark Implementations must make managing of rigid body groups thread-safe.
 * @remark Implementations may use the functions in DefaultRigidBodyGroup.h for the physics engine
 *    functon pointers when the underlying physics library doesn't natively support rigid body
 *    groups.
 * @see RigidBodyGroup.h
 */
typedef struct dsRigidBodyGroup
{
	/**
	 * @brief The physics engine the rigid body group was created with.
	 */
	dsPhysicsEngine* engine;

	/**
	 * @brief The allocator the rigid body group was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The physics scene the rigid body group is a member of, or NULL if not associated with
	 *     a scene.
	 *
	 * The rigid body group may only be associated at most one scene at a time.
	 *
	 * Implementations should assign this with atomics to avoid contention when checking during
	 * rigid body creation that the group isn't part of a scene.
	 */
	dsPhysicsScene* scene;

	/**
	 * @brief The motion type for all rigid bodies.
	 */
	dsPhysicsMotionType motionType;

	/**
	 * @brief The number of rigid bodies in the group.
	 */
	uint32_t rigidBodyCount;
} dsRigidBodyGroup;

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
	 * This will be called even if the creation of the rigid body fails.
	 */
	dsDestroyUserDataFunction destroyUserDataFunc;

	/**
	 * @brief The group the rigid body will be associated with.
	 *
	 * This may be NULL to have it not be associated with a group.
	 */
	dsRigidBodyGroup* group;

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
	 * @brief The hardness value, where 0 indicates to use this body's restitution on collision and
	 *     1 indicates to use the other body's restitution.
	 */
	float hardness;

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
 * @remark The transform members are at the top and closest to the user data pointer from
 *     dsPhysicsActor to improve cache locality as they will be the most commonly accessed
 *     members.
 * @see RigidBody.h
 */
typedef struct dsRigidBody
{
	/**
	 * @brief The actor base fields.
	 */
	dsPhysicsActor actor;

	/**
	 * @brief Whether the rigid body is active.
	 *
	 * When not active, the rigid body will not be in motion and the position and orientation will
	 * not be changed by the physics simulation.
	 */
	bool active;

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
	 * This will only be used if dsRigidBodyFlags_Scalable is set, and will not be updated by the
	 * physics simulation.
	 */
	dsVector3f scale;

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
	 * @brief The hardness value, where 0 indicates to use this body's restitution on collision and
	 *     1 indicates to use the other body's restitution.
	 */
	float hardness;

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
	 * @brief The group the rigid body is associated with, or NULL if not associated with a group.
	 */
	dsRigidBodyGroup* group;

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
 * @brief Struct defining a template to create similar rigid body instances.
 *
 * This is a factory object to create rigid bodies that are similar, typically instances of the same
 * object. This will store the shape information and common attributes, while the per-instance
 * information is provided when creating the rigid body. This is more convenient than going through
 * dsRigidBodyInit to create a dsRigidBody directly when multiple similar rigid bodies are created.
 *
 * All members apart from the shape members may be modified directly as needed.
 *
 * @see RigidBodyDemplate.h
 */
typedef struct dsRigidBodyTemplate
{
	/**
	 * @brief The physics engine the rigid body template was created with.
	 */
	dsPhysicsEngine* engine;

	/**
	 * @brief The allocator the rigid body template was created with.
	 */
	dsAllocator* allocator;

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
	 * @brief The layer the rigid body will be associated with.
	 */
	dsPhysicsLayer layer;

	/**
	 * @brief Collision group ID that the rigid body will belong to.
	 */
	uint64_t collisionGroup;

	/**
	 * @brief Function to check whether two collision groups can collide.
	 *
	 * When checking a pair of intersecting actors, they will collide if both set this function
	 * to NULL or the function returns true. Behavior is undefined if the function is set on both
	 * bodies and would return true for one body but false the other.
	 */
	dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc;

	/**
	 * @brief The mass properties of the rigid body.
	 *
	 * This may be assigned directly if custom mass properties are used.
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
	 * @brief The hardness value, where 0 indicates to use this body's restitution on collision and
	 *     1 indicates to use the other body's restitution.
	 */
	float hardness;

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
	 * @remark The instance IDs aren't set and won't be guaranteed to match the rigid bodies created.
	 *     They will be added in the same order as listed here on creation.
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
} dsRigidBodyTemplate;

/**
 * @brief Function to create a rigid body group.
 * @param engine The physics engine to create the rigid body group with.
 * @param allocator The allocator to create the rigid body group with.
 * @param motionType The motion type for the rigid bodies that are part of the group.
 */
typedef dsRigidBodyGroup* (*dsCreateRigidBodyGroupFunction)(dsPhysicsEngine* engine,
	dsAllocator* allocator, dsPhysicsMotionType motionType);

/**
 * @brief Function to destroy a rigid body group.
 *
 * If the rigid body is a member of a group or scene, the implementation should remove it during
 * destruction.
 *
 * @param engine The physics engine the rigid body group was created with.
 * @param group The rigid body group to destroy.
 * @return False if the rigid body group couldn't be destroyed.
 */
typedef bool (*dsDestroyRigidBodyGroupFunction)(dsPhysicsEngine* engine, dsRigidBodyGroup* group);

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
 * @param material The material of the shape or NULL to use the material of the rigid body.
 * @return The ID for the added shape instance or DS_INVALID_PHYSICS_ID if it couldn't be added.
 */
typedef uint32_t (*dsAddRigidBodyShapeFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	dsPhysicsShape* shape, const dsVector3f* translate, const dsQuaternion4f* rotate,
	const dsVector3f* scale, float density, const dsPhysicsShapePartMaterial* material);

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
 * @brief Function to set the material for a shape within a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to shape set the shape material on.
 * @param index The index of the shape.
 * @param material The new material for the shape or NULL to use the rigid body's material.
 * @return False if the material couldn't be changed.
 */
typedef bool (*dsSetRigidBodyShapeMaterialFunction)(dsPhysicsEngine* engine,
	dsRigidBody* rigidBody, uint32_t index, const dsPhysicsShapePartMaterial* material);

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
 * @param rigidBody The rigid body to set the transform on.
 * @param position The new position or NULL to leave unchanged.
 * @param orientation The new orientation or NULL to leave unchanged.
 * @param scale The new scale or NULL to leave unchanged.
 * @param activate Whether to activate the rigid body if it's currently inactive.
 * @return False if the transform couldn't be set.
 */
typedef bool (*dsSetRigidBodyTransformFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	const dsVector3f* position, const dsQuaternion4f* orientation, const dsVector3f* scale,
	bool activate);

/**
 * @brief Function to set the transform target for moving a kinematic rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to set the kinmatic target transform on.
 * @param time The time over which the kinematic transform occurs.
 * @param position The new position or NULL to leave unchanged.
 * @param orientation The new orientation or NULL to leave unchanged.
 * @return False if the kinematic target couldn't be set.
 */
typedef bool (*dsSetRigidBodyKinematicTargetFunction)(dsPhysicsEngine* engine,
	dsRigidBody* rigidBody, float time, const dsVector3f* position,
	const dsQuaternion4f* orientation);

/**
 * @brief Function to set a float value on a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to change the value on.
 * @param value The new value.
 * @return False if the value couldn't be set.
 */
typedef bool (*dsSetRigidBodyFloatValueFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	float value);

/**
 * @brief Function to get a vector value from a rigid body.
 * @param[out] outValue Storage for the value to get.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to get the value from.
 * @return False if the value couldn't be queried.
 */
typedef bool (*dsGetRigidBodyVectorValueFunction)(dsVector3f* outValue, dsPhysicsEngine* engine,
	const dsRigidBody* rigidBody);

/**
 * @brief Function to set a vector value on a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to change the value on.
 * @param value The new value.
 * @return False if the value couldn't be set.
 */
typedef bool (*dsSetRigidBodyVectorValueFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody,
	const dsVector3f* value);

/**
 * @brief Function to clear the accumulated force, torque, or impulse on a rigid body.
 * @param engine The physics engine the rigid body was created with.
 * @param rigidBody The rigid body to clear the force on.
 * @return False if the force couldn't be cleared.
 */
typedef bool (*dsClearRigidBodyForceFunction)(dsPhysicsEngine* engine, dsRigidBody* rigidBody);

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

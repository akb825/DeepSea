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

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Shared physics types used across multiple Types.h files that are spread out to avoid
 * getting too long.
 */

/**
 * @brief Constant for an invalid ID for a physics object.
 */
#define DS_INVALID_PHYSICS_ID (uint32_t)-1

/**
 * @brief Enum describing a layer of physics objects.
 */
typedef enum dsPhysicsLayer
{
	dsPhsyicsLayer_StaticWorld, ///< Static world collision that cannot collide with itself.
	dsPhysicsLayer_Objects,     ///< Standard physics objects that can collide with anything.
	/** Projectiles that can collide with everything but other projectiles. */
	dsPhysicsLayer_Projectiles
} dsPhysicsLayer;

/**
 * @brief Enum for the type of a physics actor.
 */
typedef enum dsPhysicsActorType
{
	dsPhysicsActorType_RigidBody ///< Non-deformable object represented with dsRigidBody.
} dsPhysicsActorType;

/**
 * @brief Enum for a mask of degrees of freedom for physics actors.
 */
typedef enum dsPhysicsDOFMask
{
	dsPhysicsDOFMask_None = 0,     ///< No degrees of freedom.
	dsPhysicsDOFMask_TransX = 0x1, ///< Translation along the X axis.
	dsPhysicsDOFMask_TransY = 0x2, ///< Translation along the Y axis.
	dsPhysicsDOFMask_TransZ = 0x4, ///< Translation along the Z axis.
	dsPhysicsDOFMask_RotX = 0x8,   ///< Rotation along the X axis.
	dsPhysicsDOFMask_RotY = 0x10,  ///< Rotation along the Y axis.
	dsPhysicsDOFMask_RotZ = 0x20,  ///< Rotationalong the Z axis.

	/** Translation along all axes. */
	dsPhysicsDOFMask_TransAll = dsPhysicsDOFMask_TransX | dsPhysicsDOFMask_TransY |
		dsPhysicsDOFMask_TransZ,
	/** Rotation along all axes. */
	dsPhysicsDOFMask_RotAll = dsPhysicsDOFMask_RotX | dsPhysicsDOFMask_RotY |
		dsPhysicsDOFMask_RotZ,
	/** Translation and rotation along all axes. */
	dsPhysicsDOFMask_All = dsPhysicsDOFMask_TransAll | dsPhysicsDOFMask_RotAll
} dsPhysicsDOFMask;

/**
 * @brief Enum for how a physics actor does, or doesn't, move.
 */
typedef enum dsPhysicsMotionType
{
	/**
	 * Object that that won't be moved by the physics simulation. While static objects may be moved
	 * manually, they may not properly interact with other objects.
	 */
	dsPhysicsMotionType_Static,

	/**
	 * Object that may be moved directly, but won't be affected by forces. When moved, it will be
	 * treated as an object with infinite mass and always move dynamic objects away.
	 */
	dsPhysicsMotionType_Kinematic,

	/**
	 * Object that will be moved based on the physics simulation with the various forces applied.
	 */
	dsPhysicsMotionType_Dynamic
} dsPhysicsMotionType;

/// @cond
typedef struct dsPhysicsEngine dsPhysicsEngine;
typedef struct dsPhysicsScene dsPhysicsScene;
/// @endcond

/**
 * @brief Function to check whether two collision groups may collide.
 * @param firstGroup The first collision group.
 * @param secondGroup The second collision group.
 * @return True if the groups may collide.
 */
typedef bool (*dsCanCollisionGroupsCollideFunction)(uint64_t firstGroup, uint64_t secondGroup);

/**
 * @brief Base type of a physics actor.
 *
 * This shares the common fields across the concrete physics actor types, allowing them to be shared
 * for uses such as managing contact points. The most commonly used concrete actor type is
 * dsRigidBody.
 */
typedef struct dsPhysicsActor
{
	/**
	 * @brief The physics engine the actor was created with.
	 */
	dsPhysicsEngine* engine;

	/**
	 * @brief The allocator the actor was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief Function to destroy the user data.
	 */
	dsDestroyUserDataFunction destroyUserDataFunc;

	/**
	 * @brief The physics scene the actor is a member of, or NULL if not associated with a scene.
	 *
	 * The actor may only be associated at most one scene at a time
	 */
	dsPhysicsScene* scene;

	/**
	 * @brief The type of the actor.
	 *
	 * This will denote which concrete type the actor is.
	 */
	dsPhysicsActorType type;

	/**
	 * @brief The layer the actor is associated with.
	 */
	dsPhysicsLayer layer;

	/**
	 * @brief Collision group ID that the actor belongs to.
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
	 * @brief User data associated with the actor.
	 *
	 * This is declared last so it can be nearest the most commonly accessed members in subclasses.
	 */
	void* userData;
} dsPhysicsActor;

/**
 * @brief Struct describing the mass and moment of inertia of a physics object.
 *
 * Instances are typically created from dsShape instances, and may be modified or combined from
 * there. If the mass properties are known ahead of time, the values may be initialized explicitly.
 *
 * Default inertia for shapes will be computed using this and its accompanying functions. This
 * ensures consistent and realistic inertia across implementations.
 *
 * The inertia is represented in local shape space, allowing for more accurate application of
 * forces relative to the shape itself. inertiaRotate and inertiaTranslate, applied in that order,
 * would transform the inertia (and shape it represents) relative the coordinate space for the
 * overall object. For example, if you were to have a box where the local origin is at the base of
 * the box, inertiaTranslate would be shifted up to the center of box.
 *
 * @see PhysicsMassProperties.h
 */
typedef struct dsPhysicsMassProperties
{
	/**
	 * @brief The tensor matrix for the moment of inertia around the center of mass.
	 *
	 * The final inertia should be queried with dsPhysicsMassProperties_getInertia() to apply any
	 * offset of the center of mass relative to inertiaTranslate. The translated inertia tensor
	 * isn't stored here since translating is a lossy process, where translating by a then b isn't
	 * the same result as translating once by (a + b).
	 */
	dsMatrix33f centeredInertia;

	/**
	 * @brief The center of mass.
	 *
	 * This will usually equal inertiaTranslate, but may differ if shifted.
	 */
	dsVector3f centerOfMass;

	/**
	 * @brief The total mass for the object.
	 *
	 * @remark This is the unscaled mass. To get the final mass, call
	 * dsPhysicsMassProperties_getScaledMass().
	 */
	float mass;

	/**
	 * @brief Translation for the frame of reference of the inertia tensor.
	 *
	 * This will be the point around which the object will rotate when in free-fall and is usually
	 * the center of mass.
	 */
	dsVector3f inertiaTranslate;

	/**
	 * @brief Rotation for the frame of reference of the inertia tensor.
	 */
	dsQuaternion4f inertiaRotate;
} dsPhysicsMassProperties;

/**
 * @brief Function to find a physics actor by name.
 * @param engine The physics engine the physics actor was created with.
 * @param userData User data to find the physics actor with.
 * @param name The name of the physics actor.
 * @return The physics actor or NULL if it couldn't be found.
 */
typedef dsPhysicsActor* (*dsFindPhysicsActorFunction)(
	dsPhysicsEngine* engine, void* userData, const char* name);

#ifdef __cplusplus
}
#endif

// Needs to be after the extern "C" block.
/// @cond
DS_ENUM_BITMASK_OPERATORS(dsPhysicsDOFMask);
/// @endcond

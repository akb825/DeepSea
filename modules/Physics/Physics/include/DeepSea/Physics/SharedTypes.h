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

/// @cond
typedef struct dsPhysicsEngine dsPhysicsEngine;
/// @endcond

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
 * @brief Enum for a mask of degrees of freedom for physics objects.
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
 * @brief Enum for how a physics option does, or doesn't, move.
 */
typedef enum dsPhysicsMotionType
{
	/**
	 * Object that that won't be moved by the physics simulation. While static objects may be moved
	 * manually, they may not properly interact with other objects.
	 */
	dsPhysicsMotionType_Static,

	/**
	 * Object that may be moved directly or by setting the velocities, but won't be affected by
	 * forces. When moved, it will be treated as an object with infinite mass and always move
	 * dynamic objects away.
	 */
	dsPhysicsMotionType_Kinematic,

	/**
	 * Object that will be moved based on the physics simulation with the various forces applied.
	 */
	dsPhysicsMotionType_Dynamic
} dsPhysicsMotionType;

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
	 * @brief The tensor matrix for the moment of inertia.
	 */
	dsMatrix33f inertia;
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

	/**
	 * @brief The total mass for the object.
	 *
	 * @remark This is the unscaled mass. To get the final mass, call
	 * dsPhysicsMassProperties_getScaledMass().
	 */
	float mass;
} dsPhysicsMassProperties;

#ifdef __cplusplus
}
#endif

// Needs to be after the extern "C" block.
/// @cond
DS_ENUM_BITMASK_OPERATORS(dsPhysicsDOFMask);
/// @endcond

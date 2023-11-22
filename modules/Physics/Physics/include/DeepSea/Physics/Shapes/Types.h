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
 * @brief Includes all of the types for shapes in the DeepSea/Physics library.
 *
 * The shape types are roughly listed in order of cheapest to most expensive to evaluate.
 */

/**
 * @brief Default convex radius for physics shapes.
 *
 * This offsers a good tradeoff between precision and performance for typical objects with
 * coordinates in meters.
 */
#define DS_DEFAULT_PHYSICS_SHAPE_CONVEX_RADIUS 0.05f

/**
 * @brief Enum for the axis to align a physics shape to.
 */
typedef enum dsPhysicsAxis
{
	dsPhysicsAxis_X, ///< X axis.
	dsPhysicsAxis_Y, ///< Y axis.
	dsPhysicsAxis_Z  ///< Z axis.
} dsPhysicsAxis;

/// @cond
typedef struct dsPhysicsEngine dsPhysicsEngine;
typedef struct dsPhysicsShape dsPhysicsShape;
/// @endcond

/**
 * @brief Struct defining the type of a physics shape.
 *
 * The struct contains type-specific information, while the pointer to the type can be used to
 * compare types from the base shape.
 */
typedef struct dsPhysicsShapeType
{
	/**
	 * @brief Whether shapes of this type may only be used with static bodies.
	 */
	bool staticBodiesOnly;
} dsPhysicsShapeType;

/**
 * @brief Function to destroy a physics shape.
 * @param shape The shape to destroy.
 */
typedef void (*dsDestroyPhysicsShapeFunction)(dsPhysicsShape* shape);

/**
 * @brief Base type for a physics shape.
 *
 * Shapes are the individual pieces of geometry that may be colided. Individual types of shapes may
 * subclass this by having this be the first member of the struct, allowing pointers to be freely
 * cast between dsPhysicsShape and its concrete type.
 *
 * Shapes are typically defined around the origin and fixed orientation and are immutable, though a
 * transform bay be set and modified when applying them to bodies.
 *
 * Shapes may be shared across bodies and are reference counted to allow shared ownership, starting
 * at a reference count of 1 and destroyed once it reaches 0. Objects that use a shape should call
 * dsPhysicsShape_addRef() to increment the reference count and dsPhysicsShape_freeRef() to
 * decrement it once it is no longer needed.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see PhysicsShape.h
 */
struct dsPhysicsShape
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
	 * @brief The type of the shape.
	 */
	const dsPhysicsShapeType* type;

	/**
	 * @brief Pointer to the shape implementation.
	 *
	 * This is a convenience to avoid needing to check the type to get the underlying shape for the
	 * physics implementation.
	 */
	void* impl;

	/**
	 * @brief Reference count for the shape.
	 */
	uint32_t refCount;

	/**
	 * @brief Function to destroy the shape.
	 *
	 * This will be automatically called in dsPhysicsShape_freeRef() once refCount hits 0.
	 */
	dsDestroyPhysicsShapeFunction destroyFunc;
};

/**
 * @brief Physics shape implementation for a sphere.
 * @remark None of the members should be modified outside of the implementation.
 * @see PhysicsSphere.h
 */
typedef struct dsPhysicsSphere
{
	/**
	 * @brief The base shape information.
	 */
	dsPhysicsShape shape;

	/**
	 * @brief The radius of the sphere.
	 */
	float radius;
} dsPhysicsSphere;

/**
 * @brief Physics shape implementation for a box.
 * @remark None of the members should be modified outside of the implementation.
 * @see PhysicsBox.h
 */
typedef struct dsPhysicsBox
{
	/**
	 * @brief The base shape information.
	 */
	dsPhysicsShape shape;

	/**
	 * @brief The half extents for each axis.
	 *
	 * The full box geometry ranges from -halfExtents to +halfExtents.
	 */
	dsVector3f halfExtents;

	/**
	 * @brief The convex radius for collision checks.
	 *
	 * Larger values will improve performance at the expense of precision by rounding the corners
	 * of the shape.
	 */
	float convexRadius;
} dsPhysicsBox;

/**
 * @brief Physics shape implementation for a capsule.
 *
 * A capsule is a cylinder with hemisphere caps, and is faster and more accurate than a standard
 * cylinder.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see PhysicsCapsule.h
 */
typedef struct dsPhysicsCapsule
{
	/**
	 * @brief The base shape information.
	 */
	dsPhysicsShape shape;

	/**
	 * @brief Half the height of the cylinder portion of the capsule.
	 *
	 * The full height will be 2*(halfHeight + radius).
	 */
	float halfHeight;

	/**
	 * @brief The radius of the capsule.
	 */
	float radius;

	/**
	 * @brief The axis the capsule is aligned with.
	 */
	dsPhysicsAxis axis;
} dsPhysicsCapsule;

/**
 * @brief Physics shape implementation for a cylinder.
 *
 * Some implementations may approximate the cylinder with a convex hull.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see PhysicsCylinder.h
 */
typedef struct dsPhysicsCylinder
{
	/**
	 * @brief The base shape information.
	 */
	dsPhysicsShape shape;

	/**
	 * @brief Half the height of the cylinder.
	 */
	float halfHeight;

	/**
	 * @brief The radius of the cylinder.
	 */
	float radius;

	/**
	 * @brief The axis the cylinder is aligned with.
	 */
	dsPhysicsAxis axis;

	/**
	 * @brief The convex radius for collision checks.
	 *
	 * Larger values will improve performance at the expense of precision by rounding the corners
	 * of the shape.
	 */
	float convexRadius;
} dsPhysicsCylinder;

/**
 * @brief Physics shape implementation for a cone.
 *
 * Some implementations may approximate the cone with a convex hull.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see PhysicsCone.h
 */
typedef struct dsPhysicsCone
{
	/**
	 * @brief The base shape information.
	 */
	dsPhysicsShape shape;

	/**
	 * @brief Half the height of the cone.
	 */
	float height;

	/**
	 * @brief The radius of the cone.
	 */
	float radius;

	/**
	 * @brief The axis the cone is aligned with.
	 */
	dsPhysicsAxis axis;

	/**
	 * @brief The convex radius for collision checks.
	 *
	 * Larger values will improve performance at the expense of precision by rounding the corners
	 * of the shape.
	 */
	float convexRadius;
} dsPhysicsCone;

#ifdef __cplusplus
}
#endif

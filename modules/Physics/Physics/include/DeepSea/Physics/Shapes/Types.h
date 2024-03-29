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
#include <DeepSea/Geometry/Types.h>
#include <DeepSea/Math/Types.h>
#include <DeepSea/Physics/SharedTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types for shapes in the DeepSea/Physics library.
 *
 * The shape types are roughly listed in order of cheapest to most expensive to evaluate.
 *
 * Heightfield types are omitted as they are quite inconsistent across various physics library
 * implementations. For example, Jolt only supports square heightfields, and can't differentiate
 * between the two triangles for each sample for material information. Only PhysX gives full control
 * for the splitting edge for each sample's square. Jolt only supports float inputs, PhysX only
 * signed 16-bit integers mixed with material indices, and Bullet supports many different input
 * formats.
 *
 * dsPhysicsMesh should be used in place of a heightfield with the explicitly triangulated result,
 * and can have consistent features across all implementations.
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
typedef struct dsPhysicsShapePartMaterial dsPhysicsShapePartMaterial;
typedef struct dsPhysicsMassProperties dsPhysicsMassProperties;
/// @endcond

/**
 * @brief Function to get the mass properties from a shape.
 * @param[out] outMassProperties The mass properties to populate.
 * @param shape The shape to get the mass properties for
 * @param density The density of the shape.
 * @return False if the shape is invalid for mass properties.
 */
typedef bool (*dsGetPhysicsShapeMassPropertiesFunction)(dsPhysicsMassProperties* outMassProperties,
	const dsPhysicsShape* shape, float density);

/**
 * @brief Function to get the physics material for a face of a shape.
 * @param[out] outMaterial The material to populate.
 * @param shape the shape to get the material for.
 * @param faceIndex The index of the face to get the material for.
 * @return False if the material couldn't be queried.
 */
typedef bool (*dsGetPhysicsShapeMaterialFunction)(dsPhysicsShapePartMaterial* outMaterial,
	const dsPhysicsShape* shape, uint32_t faceIndex);

/**
 * @brief Function to destroy a physics shape.
 * @param engine The physics engine the shape was created with.
 * @param shape The shape to destroy.
 */
typedef void (*dsDestroyPhysicsShapeFunction)(dsPhysicsEngine* engine, dsPhysicsShape* shape);

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

	/**
	 * @brief Whether shapes of this type may only be scaled with a uniform scale across all three
	 * axes.
	 *
	 * Typically shapes that have a radius as part of their parameters may only be uniformly scaled.
	 */
	bool uniformScaleOnly;

	/**
	 * @brief Function to get the mass properties for the shape.
	 */
	dsGetPhysicsShapeMassPropertiesFunction getMassPropertiesFunc;

	/**
	 * @brief Function to get the material for the shape.
	 */
	dsGetPhysicsShapeMaterialFunction getMaterialFunc;
} dsPhysicsShapeType;

/**
 * @brief Material to apply to an individual part of a shape, such as a triangle.
 */
typedef struct dsPhysicsShapePartMaterial
{
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
} dsPhysicsShapePartMaterial;

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
	 * @brief Bounds for the shape.
	 *
	 * This will be populated by the base implementations in the Physics library.
	 */
	dsAlignedBox3f bounds;

	/**
	 * @brief Pointer to the shape implementation.
	 *
	 * This is a convenience to avoid needing to check the type to get the underlying shape for the
	 * physics implementation.
	 */
	void* impl;

	/**
	 * @brief Data used for debugging.
	 *
	 * When used in a graphical application, this may be the model used to draw with.
	 *
	 * @remark This may be assigned as needed outside of the implementation.
	 */
	void* debugData;

	/**
	 * @brief Function used to destroy debugData.
	 *
	 * This may be NULL if debugData doesn't need to be destroyed.
	 *
	 * @remark This may be assigned as needed outside of the implementation.
	 */
	dsDestroyUserDataFunction destroyDebugDataFunc;

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
 * @brief Struct describing an instance of a  physics shape with a transform.
 */
typedef struct dsPhysicsShapeInstance
{
	/**
	 * @brief The physics shape.
	 */
	dsPhysicsShape* shape;

	/**
	 * @brief The ID for the shape.
	 *
	 * This will be unique within a rigid body, but may be shared across multiple rigid bodies.
	 */
	uint32_t id;

	/**
	 * @brief The density of the shape.
	 */
	float density;

	/**
	 * @brief Whether the translate portion of the transform should be used.
	 */
	bool hasTranslate;

	/**
	 * @brief Whether the rotate portion of the transform should be used.
	 */
	bool hasRotate;

	/**
	 * @brief Whether the scale portion of the transform should be used.
	 */
	bool hasScale;

	/**
	 * @brief Whether the material should be used.
	 */
	bool hasMaterial;

	/**
	 * @brief The translation for the shape.
	 */
	dsVector3f translate;

	/**
	 * @brief The scale for the shape.
	 */
	dsVector3f scale;

	/**
	 * @brief The rotation for the shape.
	 */
	dsQuaternion4f rotate;

	/**
	 * @brief The material to use for the shape.
	 */
	dsPhysicsShapePartMaterial material;
} dsPhysicsShapeInstance;

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
 * The origin of the cone is at the tip, while the center of mass is at 3/4 the height along the
 * given axis.
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

/**
 * @brief Physics shape implementation for a convex hull.
 * @remark None of the members should be modified outside of the implementation.
 * @see PhysicsCone.h
 */
typedef struct dsPhysicsConvexHull
{
	/**
	 * @brief The base shape information.
	 */
	dsPhysicsShape shape;

	/**
	 * @brief The number of vertices in the convex hull.
	 */
	uint32_t vertexCount;

	/**
	 * @brief The number of faces in the convex hull.
	 */
	uint32_t faceCount;

	/**
	 * @brief Cached base mass properties for the convex hull.
	 *
	 * This will be populated by the base dsPhysicsConvexHull in the Physics library.
	 */
	dsPhysicsMassProperties baseMassProperties;
} dsPhysicsConvexHull;

/**
 * @brief Physics shape implementation for a triangle mesh.
 * @remark Meshes may not be used for dynamic bodies. They are intended for static objects such as
 *     terrain.
 * @remark None of the members should be modified outside of the implementation.
 * @see PhysicsCone.h
 */
typedef struct dsPhysicsMesh
{
	/**
	 * @brief The base shape information.
	 */
	dsPhysicsShape shape;

	/**
	 * @brief The number of triangles in the mesh.
	 */
	uint32_t triangleCount;

	/**
	 * @brief The number of materials in the mesh.
	 */
	uint32_t materialCount;

	/**
	 * @brief The size of a material index, either sizeof(uint16_t) or sizeof(uint32_t).
	 */
	uint32_t materialIndexSize;

	/**
	 * @brief The mapping from triangle to material index.
	 */
	const void* materialIndices;

	/**
	 * @brief The materials for the mesh.
	 */
	const dsPhysicsShapePartMaterial* materials;
} dsPhysicsMesh;

#ifdef __cplusplus
}
#endif

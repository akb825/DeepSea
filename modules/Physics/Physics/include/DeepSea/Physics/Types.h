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
#include <DeepSea/Physics/Shapes/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/Physics library.
 */

/**
 * @brief Log tag used by the physics library.
 */
#define DS_PHYSICS_LOG_TAG "physics"

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
typedef bool (*dsCanCollisionGroupsCollidFunction)(uint64_t firstGroup, uint64_t secondGroup);

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
	dsCanCollisionGroupsCollidFunction canCollisionGroupsCollideFunc;

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
	 * @brief Explicit center of mass for the body.
	 */
	dsVector3f centerOfMass;

	/**
	 * @brief True if centerOfMass is set, false to compute the center of mass.
	 */
	bool centerOfMassSet;

	/**
	 * @brief The mass of the body.
	 */
	float mass;

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
 * Members that won't be modified during simulation are stored by value for easy access. Members
 * that may be updated on a per-frame basis, such as the velocity, must be queried to avoid
 * unnecessary copies. The exception to this is the position and rotation, since these will almost
 * always be used.
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
	dsCanCollisionGroupsCollidFunction canCollisionGroupsCollideFunc;

	/**
	 * @brief The position of the body in world space.
	 */
	dsVector3f position;

	/**
	 * @brief The orientation of the body in world space.
	 */
	dsVector4f orientation;

	/**
	 * @brief The scale factor of the body.
	 *
	 * This will only be used if dsRigidBodyFlags_Scalable is set.
	 */
	dsVector3f scale;

	/**
	 * @brief The mass of the body.
	 */
	float mass;

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
	dsTransformedPhysicsShape* shapes;

	/**
	 * @brief The number of shapes in the body.
	 */
	uint32_t shapeCount;

	/**
	 * @brief The maximum number of shapes before re-allocation is needed.
	 */
	uint32_t maxShapes;
} dsRigidBody;

/**
 * @brief Struct describing limits for objects within a scene.
 *
 * Some implementations may view these as strict upper limits, others may use them as hints to
 * pre-allocate, while others may ignore them completely.
 *
 * @param dsPhysicsScene
 * @see PhysicsScene.h
 */
typedef struct dsPhysicsSceneLimits
{
	/**
	 * @brief The maximum number of bodies that are only used for collision and not affected by
	 * physics.
	 */
	uint32_t maxStaticBodies;

	/**
	 * @brief The maximum number of bodies that are affected by physics.
	 */
	uint32_t maxDynamicBodies;

	/**
	 * @brief The maximum number of groups of bodies that are connected through constraints.
	 */
	uint32_t maxConstrainedBodyGroups;

	/**
	 * @brief The maximum number of shapes used by static bodies.
	 *
	 * If 0 maxStaticBodies will be used.
	 */
	uint32_t maxStaticShapes;

	/**
	 * @brief The maximum number of shapes used by dynamic bodies.
	 *
	 * If 0 maxDynamicBodies will be used.
	 */
	uint32_t maxDynamicShapes;

	/**
	 * @brief The maximum number of constraints.
	 */
	uint32_t maxConstraints;

	/**
	 * @brief The maximum number of pairs of bodies that may collide.
	 *
	 * The implementation is only guaranteed to process this many pairs of potentially colliding
	 * bodies. If it is exceeded, further collisions may be ignored.
	 *
	 * This should be much larger than the maximum number of contact points as the collision pairs
	 * may not actually touch.
	 */
	uint32_t maxBodyCollisionPairs;

	/**
	 * @brief The maximum number of contact points between colliding bodies.
	 *
	 * The implementation is only guaranteed to process this many contacts between bodies. If it is
	 * exceeded, further contacts may be discarded.
	 */
	uint32_t maxContactPoints;
} dsPhysicsSceneLimits;

/**
 * @brief Struct defining a scene of objects in a physics simulation.
 * @remark None of the members should be modified outside of the implementation.
 * @see dsPhysicsSceneLimits
 * @see PhysicsScene.h
 */
typedef struct dsPhysicsScene
{
	/**
	 * @brief The physics engine the scene was created with.
	 */
	dsPhysicsEngine* engine;

	/**
	 * @brief The allocator the scene was created with.
	 */
	dsAllocator* allocator;
} dsPhysicsScene;

/**
 * @brief Function to destroy a physics engine.
 * @param engine The physics engine to destroy.
 * @return False if the physics engine couldn't be destroyed.
 */
typedef bool (*dsDestroyPhysicsEngineFunction)(dsPhysicsEngine* engine);

/**
 * @brief Function to create a physics scene.
 * @param engine The physics engine to create the scene with.
 * @param allocator The allocator to create the scene with.
 * @param limits The limits for the physics scene.
 * @param threadPool The thread pool to use for multithreaded processing, or NULL for
 *     single-threaded processing.
 * @return The created physics scene or NULL if it couldn't be created.
 */
typedef dsPhysicsScene* (*dsCreatePhysicsSceneFunction)(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsSceneLimits* limits, dsThreadPool* threadPool);

/**
 * @brief Function to destroy a physics scene.
 * @param engine The physics engine the scene was created with.
 * @param scene The physics scene to destroy.
 * @return False if the physics scene couldn't be destroyed.
 */
typedef bool (*dsDestroyPhysicsSceneFunction)(dsPhysicsEngine* engine, dsPhysicsScene* scene);

/**
 * @brief Function to create a physics sphere.
 * @param engine The physics engine to create the sphere with.
 * @param allocator The allocator to create the sphere with.
 * @param radius The radius of the sphere.
 * @return The sphere or NULL if it couldn't be created.
 */
typedef dsPhysicsSphere* (*dsCreatePhysicsSphereFunction)(dsPhysicsEngine* engine,
	dsAllocator* allocator, float radius);

/**
 * @brief Function to create a physics box.
 * @param engine The physics engine to create the box with.
 * @param allocator The allocator to create the box with.
 * @param halfExtents The half extents for each axis.
 * @param convexRadius The convex radius used for collision checks.
 * @return The box or NULL if it couldn't be created.
 */
typedef dsPhysicsBox* (*dsCreatePhysicsBoxFunction)(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsVector3f* halfExtents, float convexRadius);

/**
 * @brief Function to create a physics capsule.
 * @param engine The physics engine to create the capsule with.
 * @param allocator The allocator to create the capsule with.
 * @param halfHeight The half height of the cylinder portion of the capsule.
 * @param radius The radius of the capsule.
 * @param axis The axis the capsule is aligned with.
 * @return The capsule or NULL if it couldn't be created.
 */
typedef dsPhysicsCapsule* (*dsCreatePhysicsCapsuleFunction)(dsPhysicsEngine* engine,
	dsAllocator* allocator, float halfHeight, float radius, dsPhysicsAxis axis);

/**
 * @brief Function to create a physics cylinder.
 * @param engine The physics engine to create the cylinder with.
 * @param allocator The allocator to create the cylinder with.
 * @param halfHeight The half height of the cylinder.
 * @param radius The radius of the cylinder.
 * @param axis The axis the cylinder is aligned with.
 * @param convexRadius The convex radius used for collision checks.
 * @return The cylinder or NULL if it couldn't be created.
 */
typedef dsPhysicsCylinder* (*dsCreatePhysicsCylinderFunction)(dsPhysicsEngine* engine,
	dsAllocator* allocator, float halfHeight, float radius, dsPhysicsAxis axis,
	float convexRadius);

/**
 * @brief Function to create a physics cone.
 * @param engine The physics engine to create the cone with.
 * @param allocator The allocator to create the cone with.
 * @param height The height of the cone.
 * @param radius The radius of the cone.
 * @param axis The axis the cone is aligned with.
 * @param convexRadius The convex radius used for collision checks.
 * @return The cone or NULL if it couldn't be created.
 */
typedef dsPhysicsCone* (*dsCreatePhysicsConeFunction)(dsPhysicsEngine* engine,
	dsAllocator* allocator, float height, float radius, dsPhysicsAxis axis,
	float convexRadius);

/**
 * @brief Function to create a physics convex hull.
 * @param engine The physics engine to create the convex hull with.
 * @param allocator The allocator to create the convex hull with.
 * @param vertices Pointer to the vertices.
 * @param vertexCount The number of vertices.
 * @param vertexStride The stride in bytes between each vertex.
 * @param convexRadius The convex radius used for collision checks.
 * @param cacheName Unique name used to cache the result.
 * @return The conex hull or NULL if it couldn't be created.
 */
typedef dsPhysicsConvexHull* (*dsCreatePhysicsConvexHullFunction)(dsPhysicsEngine* engine,
	dsAllocator* allocator, const void* vertices, uint32_t vertexCount, size_t vertexStride,
	float convexRadius, const char* cacheName);

/**
 * @brief Function to get a vertex from the convex hull.
 * @param[out] outVertex The value to set for the vertex.
 * @param engine The physics engine that created the convex hull.
 * @param convexHull The convex hull to get the vertex from.
 * @param vertexIndex The index to the vertex to get.
 */
typedef void (*dsGetPhysicsConvexHullVertexFunction)(dsVector3f* outVertex, dsPhysicsEngine* engine,
	const dsPhysicsConvexHull* convexHull, uint32_t vertexIndex);

/**
 * @brief Function to get the number of vertices for a face in the convex hull.
 * @remark This may not provide any data if debug is false in the physics engine.
 * @param engine The physics engine that created the convex hull.
 * @param convexHull The convex hull to get the face vertex from.
 * @param faceIndex The index of the face to get the index count from.
 * @return The number of vertex indices for the face.
 */
typedef uint32_t (*dsGetPhysicsConvexHullFaceVertexCountFunction)(dsPhysicsEngine* engine,
	const dsPhysicsConvexHull* convexHull, uint32_t faceIndex);

/**
 * @brief Function to get the face for a convex hull.
 * @remark This may not provide any data if debug is false in the physics engine.
 * @remark errno should be set to ESIZE and return 0 if outIndexCapacity is too small.
 * @param[out] outIndices The indices for the face vertices. This will only be populated if there is
 *     enough capacity.
 * @param outIndexCapacity The capacity of outIndices.
 * @param[out] outNormal The normal for the face. This may be NULL if no normal is needed.
 * @param convexHull The convex hull to get the face vertex from.
 * @param faceIndex The index of the face to get.
 * @return The number of vertex indices for the face.
 */
typedef uint32_t (*dsGetPhysicsConvexHullFaceFunction)(uint32_t* outIndices,
	uint32_t outIndexCapacity, dsVector3f* outNormal, dsPhysicsEngine* engine,
	const dsPhysicsConvexHull* convexHull, uint32_t faceIndex);

/**
 * @brief Function to create a physics mesh.
 * @param engine The physics engine to create the mesh with.
 * @param allocator The allocator to create the mesh with.
 * @param vertices Pointer to the first vertex. Each vertex is defined as 3 floats.
 * @param vertexCount The number of vertices. At least 3 vertices must be provided.
 * @param vertexStride The stride in bytes between each vertex.
 * @param indices The pointer to the first index. Three indices are expected for each triangle.
 * @param triangleCount The number of triangles in the mesh.
 * @param indexSize The size of each index.
 * @param triangleMaterialIndices Material indices for each triangle, which index into the
 *     triangleMaterials array. May be NULL if per-triangle materials aren't used.
 * @param triangleMaterialIndexSize The size of each triangle material index.
 * @param triangleMaterials The per-triangle materials, or NULL if per-triangle materials aren't
 *     used.
 * @param triangleMaterialCount The number of per-triangle materials.
 * @param cacheName Unique name used to cache the result.
 * @return The mesh or NULL if it couldn't be created.
 */
typedef dsPhysicsMesh* (*dsCreatePhysicsMeshFunction)(dsPhysicsEngine* engine,
	dsAllocator* allocator, const void* vertices, uint32_t vertexCount, size_t vertexStride,
	const void* indices, uint32_t triangleCount, size_t indexSize,
	const void* triangleMaterialIndices, size_t triangleMaterialIndexSize,
	const dsPhysicsShapePartMaterial* triangleMaterials, uint32_t triangleMaterialCount,
	const char* cacheName);

/**
 * @brief Struct describing the core engine for managing physics.
 *
 * This is a base type for the physics engine, which is implemented to either integrate to a 3rd
 * party physics engine or with a custom engine. It contains function pointers to create and destroy
 * the various physics objects and any other central management.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see PhysicsEngine.h
 */
struct dsPhysicsEngine
{
	/**
	 * @brief Allocator for the physics engine.
	 *
	 * When possible, this will be used for global allocations in the physics allocations. Depending
	 * on the level of control for the underlying implementation, this may also be used for some
	 * internal allocations for individual objects.
	 */
	dsAllocator* allocator;

	/**
	 * @brief True to enable debugging.
	 *
	 * Internally this may compute extra data for use for debugging. Externally this may be used to
	 * populate debugData on shapes, such as for geometry to visualize the physics geometry.
	 */
	bool debug;

	/**
	 * @brief The maximum number of vertices allowed for a convex hull.
	 */
	uint32_t maxConvexHullVertices;

	/**
	 * @brief Directory to cache pre-computed physics data.
	 */
	const char* cacheDir;

	// ------------------------------------------ Shape creation -----------------------------------

	/**
	 * @brief Function to destroy the physics engine.
	 */
	dsDestroyPhysicsEngineFunction destroyFunc;

	/**
	 * @brief Function to create a physics scene.
	 */
	dsCreatePhysicsSceneFunction createSceneFunc;

	/**
	 * @brief Function to destroy a physics scene.
	 */
	dsDestroyPhysicsSceneFunction destroySceneFunc;

	/**
	 * @brief Function to create a physics sphere.
	 */
	dsCreatePhysicsSphereFunction createSphereFunc;

	/**
	 * @brief Function to create a physics box.
	 */
	dsCreatePhysicsBoxFunction createBoxFunc;

	/**
	 * @brief Function to create a physics capsule.
	 */
	dsCreatePhysicsCapsuleFunction createCapsuleFunc;

	/**
	 * @brief Function to create a physics cylinder.
	 */
	dsCreatePhysicsCylinderFunction createCylinderFunc;

	/**
	 * @brief Function to create a physics cone.
	 */
	dsCreatePhysicsConeFunction createConeFunc;

	/**
	 * @brief Function to create a physics convex hull.
	 */
	dsCreatePhysicsConvexHullFunction createConvexHullFunc;

	/**
	 * @brief Function to get the vertex of a convex hull.
	 */
	dsGetPhysicsConvexHullVertexFunction getConvexHullVertexFunc;

	/**
	 * @brief Function to get the number of vertices for the face of a convex hull.
	 */
	dsGetPhysicsConvexHullFaceVertexCountFunction getConvexHullFaceVertexCountFunc;

	/**
	 * @brief Function to get the face of a convex hull.
	 */
	dsGetPhysicsConvexHullFaceFunction getConvexHullFaceFunc;

	/**
	 * @brief Function to create a physics mesh.
	 */
	dsCreatePhysicsMeshFunction createMeshFunc;
};

#ifdef __cplusplus
}
#endif

// Needs to be after the extern "C" block.
/// @cond
DS_ENUM_BITMASK_OPERATORS(dsPhysicsDOFMask);
DS_ENUM_BITMASK_OPERATORS(dsRigidBodyFlags);
/// @endcond

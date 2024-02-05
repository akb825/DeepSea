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
#include <DeepSea/Physics/Shapes/Types.h>
#include <DeepSea/Physics/RigidBodyTypes.h>

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

/**
 * @brief Struct containing the information for a point of contact between two physics actors.
 * @see dsPhysicsActorContactManifold
 * @see PhysicsActorContactManifold.h
 */
typedef struct dsPhysicsActorContactPoint
{
	/**
	 * @brief The index of the shape on the first actor.
	 */
	uint32_t shapeIndexA;

	/**
	 * @brief The index of the face on the shape of the first actor.
	 */
	uint32_t faceIndexA;

	/**
	 * @brief The index of the shape on the second actor.
	 */
	uint32_t shapeIndexB;

	/**
	 * @brief The index of the face on the shape of the second actor.
	 */
	uint32_t faceIndexB;

	/**
	 * @brief The point on the first actor.
	 */
	dsVector3f pointA;

	/**
	 * @brief The point on the second actor.
	 */
	dsVector3f pointB;

	/**
	 * @brief The normal relative to the first actor.
	 *
	 * Negate to have the normal relative to the second actor.
	 */
	dsVector3f normal;

	/**
	 * @brief The signed distance between the points.
	 *
	 * A negative value indicates that the actors inter-penetrate.
	 */
	float distance;
} dsPhysicsActorContactPoint;

/**
 * @brief Struct holding the contacts for a pair of physics actors.
 *
 * Depending on the implementation, there may either be a single contact manifold between a pair of
 * actors or multiple. Therefore, it should not be assumed that when a contact manifold is added or
 * removed that it is the first or last contact between the actor pair.
 *
 * When modifying contact points, contactSettingCount will be non-zero, in which case the properties
 * such as combined friction and restitution may be set. The manifold may not necessarily be the
 * same between modifying the contact properties and responding to contact events.
 *
 * Physics implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsPhysiscActorContactProperties and the true internal type.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see PhysicsActorContactManifold.h
 */
typedef struct dsPhysicsActorContactManifold
{
	/**
	 * @brief The physics scene the contact manifold was created with.
	 */
	dsPhysicsScene* scene;

	/**
	 * @brief The first actor for the contact.
	 */
	const dsPhysicsActor* actorA;

	/**
	 * @brief The second actor for the contact.
	 */
	const dsPhysicsActor* actorB;

	/**
	 * @brief The number of contact points.
	 */
	uint32_t pointCount;

	/**
	 * @brief The number of contact properties that may be modified.
	 *
	 * The contact properties include the combined friction and restitution values.
	 *
	 * This will be one of the following values:
	 * - 0: the contact properties may not be set, this is for responding to events only.
	 * - 1: Only a single set of contact properties is maintained for all the contact points in this
	 *   manifold. All of the points in the manifold will have the same shape and face indices. In
	 *   this case, the callback need only set the contact properties once for all points.
	 * - pointCount: the contact properties are maintained separately for each point. In this case,
	 *   the shape and face indices may differ for each point and the callback should set the
	 *   contact properties for all points.
	 */
	uint32_t contactPropertiesCount;
} dsPhysicsActorContactManifold;

/**
 * @brief Struct describing the contact properties between two physics actors.
 * @see PhysicsActorContactManifold.h
 */
typedef struct dsPhysicsActorContactProperties
{
	/**
	 * @brief The friction between both actors at the contact.
	 */
	float combinedFriction;

	/**
	 * @brief The restitution between both actors at the contact.
	 */
	float combinedRestitution;

	/**
	 * @brief The target velocity relative to the first actor.
	 */
	dsVector3f targetVelocity;
} dsPhysicsActorContactProperties;

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
 * @brief Function to combine friction values.
 * @param frictionA The first friction value.
 * @param frictionB The second friction value.
 * @return The combined friction value.
 */
typedef float (*dsCombineFrictionFunction)(float frictionA, float frictionB);

/**
 * @brief Function to combine restitution values.
 * @param restitutionA The first restitution value.
 * @param hardnessA The first hardness value.
 * @param restitutionB The second restitution value.
 * @param hardnessB The second hardness value.
 * @return The combined restitution value.
 */
typedef float (*dsCombineRestitutionFunction)(float restitutionA, float hardnessA,
	float restitutionB, float hardnessB);

/**
 * @brief Function to respond to physics actor contact manifold events.
 * @param scene The physics scene the event came from.
 * @param manifold The contact manifold for the event.
 * @param userData User data supplied for the event.
 */
typedef void (*dsPhysicsActorContactManifoldFunction)(dsPhysicsScene* scene,
	const dsPhysicsActorContactManifold* manifold, void* userData);

/**
 * @brief Function to update to physics actor contact properties.
 * @param scene The physics scene the event came from.
 * @param manifold The contact manifold to update the properties on.
 * @param userData User data supplied for the event.
 * @return True if the properties were updated, in which case the implementation will assume all
 *     properties were set on the manifold, or false if the properties were left unchanged, in which
 *     case the implementation will assume the defaults should be used for all properties.
 */
typedef bool (*dsUpdatePhysicsActorContactPropertiesFunction)(dsPhysicsScene* scene,
	dsPhysicsActorContactManifold* manifold, void* userData);

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

	/**
	 * @brief The function to combine friction values.
	 *
	 * This defaults to dsPhysicsScene_defaultCombineFriction.
	 */
	dsCombineFrictionFunction combineFrictionFunc;

	/**
	 * @brief The function to combine restitution values.
	 *
	 * This defaults to dsPhysicsScene_defaultCombineRestitution.
	 */
	dsCombineRestitutionFunction combineRestitutionFunc;

	/**
	 * @brief Function to update contact properties betnween physics actors.
	 */
	dsUpdatePhysicsActorContactPropertiesFunction updatePhysicsActorContactPropertiesFunc;

	/**
	 * @brief User data provided to updatePhysicsActorContactPropertiesFunc.
	 */
	void* updatePhysicsActorContactPropertiesUserData;

	/**
	 * @brief Function to destroy the update physics actor contact properties user data.
	 */
	dsDestroyUserDataFunction destroyUpdatePhysicsActionContactPropertiesUserDataFunc;

	/**
	 * @brief Function to respond to a physics actor contact manifold being added..
	 */
	dsPhysicsActorContactManifoldFunction physicsActorContactManifoldAddedFunc;

	/**
	 * @brief User data provided to physicsActorContactManifoldAddedFunc.
	 */
	void* physicsActorContactManifoldAddedUserData;

	/**
	 * @brief Function to destroy the physics actor contact manifold added user data.
	 */
	dsDestroyUserDataFunction destroyPhysicsActorContactManifoldAddedUserDataFunc;

	/**
	 * @brief Function to respond to a physics actor contact manifold being updated..
	 */
	dsPhysicsActorContactManifoldFunction physicsActorContactManifoldUpdatedFunc;

	/**
	 * @brief User data provided to physicsActorContactManifoldUpdatedFunc.
	 */
	void* physicsActorContactManifoldUpdatedUserData;

	/**
	 * @brief Function to destroy the physics actor contact manifold updated user data.
	 */
	dsDestroyUserDataFunction destroyPhysicsActorContactManifoldUpdatedUserDataFunc;

	/**
	 * @brief Function to respond to a physics actor contact manifold being removed..
	 */
	dsPhysicsActorContactManifoldFunction physicsActorContactManifoldRemovedFunc;

	/**
	 * @brief User data provided to physicsActorContactManifoldRemovedFunc.
	 */
	void* physicsActorContactManifoldRemovedUserData;

	/**
	 * @brief Function to destroy the physics actor contact manifold removed user data.
	 */
	dsDestroyUserDataFunction destroyPhysicsActorContactManifoldRemovedUserDataFunc;
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
 * @brief Function to set the combine friction function on a physics scene.
 * @param engine The physics engine the scene was created with.
 * @param scene The physics scene to set the combine function on.
 * @param combineFunc The friction combine function.
 * @return False if the friction combine function couldn't be set.
 */
typedef bool (*dsSetPhysicsSceneCombineFrictionFunction)(dsPhysicsEngine* engine,
	dsPhysicsScene* scene, dsCombineFrictionFunction combineFunc);

/**
 * @brief Function to set the combine restitution function on a physics scene.
 * @param engine The physics engine the scene was created with.
 * @param scene The physics scene to set the combine function on.
 * @param combineFunc The restitution combine function.
 * @return False if the restitution combine function couldn't be set.
 */
typedef bool (*dsSetPhysicsSceneCombineRestitutionFunction)(dsPhysicsEngine* engine,
	dsPhysicsScene* scene, dsCombineRestitutionFunction combineFunc);

/**
 * @brief Function to add rigid bodies to a physics scene.
 * @param engine The physics engine the scene was created with.
 * @param scene The scene to add the rigid body to.
 * @param rigidBodies The rigid bodies to add.
 * @param rigidBodyCount The number of rigid bodies to add.
 * @param activate Whether the rigid bodies should be activated on insertion.
 * @return False if the rigid body couldn't be added.
 */
typedef bool (*dsPhysicsSceneAddRigidBodiesFunction)(dsPhysicsEngine* engine, dsPhysicsScene* scene,
	dsRigidBody* const* rigidBodies, uint32_t rigidBodyCount, bool activate);

/**
 * @brief Function to remove rigid bodies from a physics scene.
 * @param engine The physics engine the scene was created with.
 * @param scene The scene to remove the rigid bodies from.
 * @param rigidBodies The rigid bodies to remove.
 * @param rigidBodyCount The number of rigid bodies to remove.
 * @return False if the rigid body couldn't be removed.
 */
typedef bool (*dsPhysicsSceneRemoveRigidBodiesFunction)(dsPhysicsEngine* engine,
	dsPhysicsScene* scene, dsRigidBody* const* rigidBodies, uint32_t rigidBodyCount);

/**
 * @brief Function to add a rigid body group to a physics scene.
 * @param engine The physics engine the scene was created with.
 * @param scene The scene to add the rigid body group to.
 * @param group The rigid body group to add.
 * @param activate Whether the rigid bodies should be activated on insertion.
 * @return False if the rigid group body group couldn't be added.
 */
typedef bool (*dsPhysicsSceneAddRigidBodyGroupFunction)(dsPhysicsEngine* engine,
	dsPhysicsScene* scene, dsRigidBodyGroup* group, bool activate);

/**
 * @brief Function to remove a rigid body group from a physics scene.
 * @param engine The physics engine the scene was created with.
 * @param scene The scene to remove the rigid body group from.
 * @param group The rigid body group to remove.
 * @return False if the rigid body group couldn't be removed.
 */
typedef bool (*dsPhysicsSceneRemoveRigidBodyGroupFunction)(dsPhysicsEngine* engine,
	dsPhysicsScene* scene, dsRigidBodyGroup* group);

/**
 * @brief Function to set a physics actor contact manifold callback on a physics scene.
 * @param engine The physics engine the scene was created with.
 * @param scene The physics scene to set the callback on.
 * @param function The callback to respond to contact manifold events.
 * @param userData The user data to forward to the callback.
 * @param destroyUserDataFunc Function to destroy the user data.
 * @return False if the callback couldn't be set.
 */
typedef bool (*dsSetPhysicsSceneContactManifoldFunction)(dsPhysicsEngine* engine,
	dsPhysicsScene* scene, dsPhysicsActorContactManifoldFunction function, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc);

/**
 * @brief Function to set a callback to update physics actor contact properties on a physics scene.
 * @param engine The physics engine the scene was created with.
 * @param scene The physics scene to set the callback on.
 * @param function The callback to respond to update contact properties.
 * @param userData The user data to forward to the callback.
 * @param destroyUserDataFunc Function to destroy the user data.
 * @return False if the callback couldn't be set.
 */
typedef bool (*dsSetPhysicsSceneUpdateContactPropertiesFunction)(dsPhysicsEngine* engine,
	dsPhysicsScene* scene, dsUpdatePhysicsActorContactPropertiesFunction function, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc);

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
 * @brief Function to destroy a physics sphere.
 * @param engine The physics engine the sphere was created with.
 * @param sphere The sphere to destroy.
 * @return False if the sphere couldn't be destroyed.
 */
typedef bool (*dsDestroyPhysicsSphereFunction)(dsPhysicsEngine* engine, dsPhysicsSphere* sphere);

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
 * @brief Function to destroy a physics box.
 * @param engine The physics engine the box was created with.
 * @param box The sphere to destroy.
 * @return False if the box couldn't be destroyed.
 */
typedef bool (*dsDestroyPhysicsBoxFunction)(dsPhysicsEngine* engine, dsPhysicsBox* box);

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
 * @brief Function to destroy a physics capsule.
 * @param engine The physics engine the capsule was created with.
 * @param capsule The capsule to destroy.
 * @return False if the capsule couldn't be destroyed.
 */
typedef bool (*dsDestroyPhysicsCapsuleFunction)(dsPhysicsEngine* engine, dsPhysicsCapsule* capsule);

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
 * @brief Function to destroy a physics cylinder.
 * @param engine The physics engine the cylinder was created with.
 * @param cylinder The cylinder to destroy.
 * @return False if the cylinder couldn't be destroyed.
 */
typedef bool (*dsDestroyPhysicsCylinderFunction)(dsPhysicsEngine* engine,
	dsPhysicsCylinder* cylinder);

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
 * @brief Function to destroy a physics cone.
 * @param engine The physics engine the cone was created with.
 * @param cone The cone to destroy.
 * @return False if the cone couldn't be destroyed.
 */
typedef bool (*dsDestroyPhysicsConeFunction)(dsPhysicsEngine* engine, dsPhysicsCone* cone);

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
 * @brief Function to destroy a physics convex hull.
 * @param engine The physics engine the convex hull was created with.
 * @param convexHull The convex hull to destroy.
 * @return False if the convex hull couldn't be destroyed.
 */
typedef bool (*dsDestroyPhysicsConvexHullFunction)(dsPhysicsEngine* engine,
	dsPhysicsConvexHull* convexHull);

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
 * @brief Function to destroy a physics mesh.
 * @param engine The physics engine the mesh was created with.
 * @param mesh The mesh to destroy.
 * @return False if the mesh couldn't be destroyed.
 */
typedef bool (*dsDestroyPhysicsMeshFunction)(dsPhysicsEngine* engine, dsPhysicsMesh* mesh);

/**
 * @brief Function to get a contact point within a contact manifold.
 * @param[out] outPoint Storage for the contact point.
 * @param engine The physics engine the contact manifold was created with.
 * @param manifold The contact manifold to get the contact point from.
 * @param index The index of the contact point.
 * @return False if the contact point couldn't be queried.
 */
typedef bool (*dsGetPhysicsActorContactPointFunction)(dsPhysicsActorContactPoint* outPoint,
	dsPhysicsEngine* engine, const dsPhysicsActorContactManifold* manifold, uint32_t index);

/**
 * @brief Function to set contact properties within a contact manifold.
 * @param engine The physics engine the contact manifold was created with.
 * @param manifold The contact manifold to set the contact properties on.
 * @param index The index of the contact properties.
 * @param properties The contact properties to set.
 * @return False if the contact properties couldn't be set.
 */
typedef bool (*dsSetPhysicsActorContactPropertiesFunction)(dsPhysicsEngine* engine,
	dsPhysicsActorContactManifold* manifold, uint32_t index,
	const dsPhysicsActorContactProperties* properties);

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

	/**
	 * @brief Function to destroy the physics engine.
	 */
	dsDestroyPhysicsEngineFunction destroyFunc;

	// ---------------------------------------------- Scenes ---------------------------------------

	/**
	 * @brief Function to create a physics scene.
	 */
	dsCreatePhysicsSceneFunction createSceneFunc;

	/**
	 * @brief Function to destroy a physics scene.
	 */
	dsDestroyPhysicsSceneFunction destroySceneFunc;

	/**
	 * @brief Function to set the combine friction function on a physics scene.
	 */
	dsSetPhysicsSceneCombineFrictionFunction setSceneCombineFrictionFunc;

	/**
	 * @brief Function to set the combine restitution function on a physics scene.
	 */
	dsSetPhysicsSceneCombineRestitutionFunction setSceneCombineRestitutionFunc;

	/**
	 * @brief Function to add rigid bodies to a physics scene.
	 */
	dsPhysicsSceneAddRigidBodiesFunction addSceneRigidBodiesFunc;

	/**
	 * @brief Function to remove rigid bodies from a physics scene.
	 */
	dsPhysicsSceneRemoveRigidBodiesFunction removeSceneRigidBodiesFunc;

	/**
	 * @brief Function to add a rigid body group to a physics scene.
	 */
	dsPhysicsSceneAddRigidBodyGroupFunction addSceneRigidBodyGroupFunc;

	/**
	 * @brief Function to remove a rigid body group from a physics scene.
	 */
	dsPhysicsSceneRemoveRigidBodyGroupFunction removeSceneRigidBodyGroupFunc;

	/**
	 * @brief Function to set the physics actor contact properties update callback on a physics scene.
	 */
	dsSetPhysicsSceneUpdateContactPropertiesFunction setSceneUpdateContactPropertiesFunc;

	/**
	 * @brief Function to set the physics actor contact manifold added callback on a physics scene.
	 */
	dsSetPhysicsSceneContactManifoldFunction setSceneContactManifoldAddedFunc;

	/**
	 * @brief Function to set the physics actor contact manifold updated callback on a physics
	 *     scene.
	 */
	dsSetPhysicsSceneContactManifoldFunction setSceneContactManifoldUpdatedFunc;

	/**
	 * @brief Function to set the physics actor contact manifold removed callback on a physics
	 *     scene.
	 */
	dsSetPhysicsSceneContactManifoldFunction setSceneContactManifoldRemovedFunc;

	// ---------------------------------------- Contact manifolds ----------------------------------

	/**
	 * @brief Function to get a contact point from a contact manifold.
	 */
	dsGetPhysicsActorContactPointFunction getPhysicsActorContactPointFunc;

	/**
	 * @brief Function to set contact properties on a contact manifold.
	 */
	dsSetPhysicsActorContactPropertiesFunction setPhysicsActorContactPropertiesFunc;

	// ------------------------------------------ Shape creation -----------------------------------

	/**
	 * @brief Function to create a physics sphere.
	 */
	dsCreatePhysicsSphereFunction createSphereFunc;

	/**
	 * @brief Function to destroy a physics sphere.
	 */
	dsDestroyPhysicsSphereFunction destroySphereFunc;

	/**
	 * @brief Function to create a physics box.
	 */
	dsCreatePhysicsBoxFunction createBoxFunc;

	/**
	 * @brief Function to destroy a physics box.
	 */
	dsDestroyPhysicsBoxFunction destroyBoxFunc;

	/**
	 * @brief Function to create a physics capsule.
	 */
	dsCreatePhysicsCapsuleFunction createCapsuleFunc;

	/**
	 * @brief Function to destroy a physics capsule.
	 */
	dsDestroyPhysicsCapsuleFunction destroyCapsuleFunc;

	/**
	 * @brief Function to create a physics cylinder.
	 */
	dsCreatePhysicsCylinderFunction createCylinderFunc;

	/**
	 * @brief Function to destroy a physics cylinder.
	 */
	dsDestroyPhysicsCylinderFunction destroyCylinderFunc;

	/**
	 * @brief Function to create a physics cone.
	 */
	dsCreatePhysicsConeFunction createConeFunc;

	/**
	 * @brief Function to destroy a physics cone.
	 */
	dsDestroyPhysicsConeFunction destroyConeFunc;

	/**
	 * @brief Function to create a physics convex hull.
	 */
	dsCreatePhysicsConvexHullFunction createConvexHullFunc;

	/**
	 * @brief Function to destroy a physics convex hull.
	 */
	dsDestroyPhysicsConvexHullFunction destroyConvexHullFunc;

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

	/**
	 * @brief Function to destroy a physics mesh.
	 */
	dsDestroyPhysicsMeshFunction destroyMeshFunc;

	// ------------------------------------------- Rigid bodies ------------------------------------

	/**
	 * @brief Function to create a rigid body group.
	 */
	dsCreateRigidBodyGroupFunction createRigidBodyGroupFunc;

	/**
	 * @brief Function to destroy a rigid body group.
	 */
	dsDestroyRigidBodyGroupFunction destroyRigidBodyGroupFunc;

	/**
	 * @brief Function to create a rigid body.
	 */
	dsCreateRigidBodyFunction createRigidBodyFunc;

	/**
	 * @brief Function to destroy a rigid body.
	 */
	dsDestroyRigidBodyFunction destroyRigidBodyFunc;

	/**
	 * @brief Function to add a shape to a rigid body.
	 */
	dsAddRigidBodyShapeFunction addRigidBodyShapeFunc;

	/**
	 * @brief Function to set the transform of a shape on a rigid body.
	 */
	dsSetRigidBodyShapeTransformFunction setRigidBodyShapeTransformFunc;

	/**
	 * @brief Function to set the material of a shape on a rigid body.
	 */
	dsSetRigidBodyShapeMaterialFunction setRigidBodyShapeMaterialFunc;

	/**
	 * @brief Function to remove a shape from a rigid body.
	 */
	dsRemoveRigidBodyShapeFunction removeRigidBodyShapeFunc;

	/**
	 * @brief Function to finalize the shapes on a rigid body.
	 */
	dsFinalizeRigidBodyShapesFunction finalizeRigidBodyShapesFunc;

	/**
	 * @brief Function to set flags on a rigid body.
	 */
	dsSetRigidBodyFlagsFunction setRigidBodyFlagsFunc;

	/**
	 * @brief Function to set the motion type on a rigid body.
	 */
	dsSetRigidBodyMotionTypeFunction setRigidBodyMotionTypeFunc;

	/**
	 * @brief Function to set the degree of freedom mask on a rigid body.
	 */
	dsSetRigidBodyDOFMaskFunction setRigidBodyDOFMaskFunc;

	/**
	 * @brief Function to set the collision group on a rigid body.
	 */
	dsSetRigidBodyCollisionGroupFunction setRigidBodyCollisionGroupFunc;

	/**
	 * @brief Function to set the can collision groups collide function on a rigid body.
	 */
	dsSetRigidBodyCanCollisionGroupsCollideFunction setRigidBodyCanCollisionGroupsCollideFunc;

	/**
	 * @brief Function to set the transform on a rigid body.
	 */
	dsSetRigidBodyTransformFunction setRigidBodyTransformFunc;

	/**
	 * @brief Function to set the mass on a rigid body.
	 */
	dsSetRigidBodyFloatValueFunction setRigidBodyMassFunc;

	/**
	 * @brief Function to set the friction on a rigid body.
	 */
	dsSetRigidBodyFloatValueFunction setRigidBodyFrictionFunc;

	/**
	 * @brief Function to set the restitution on a rigid body.
	 */
	dsSetRigidBodyFloatValueFunction setRigidBodyRestitutionFunc;

	/**
	 * @brief Function to set the restitution on a hardness body.
	 */
	dsSetRigidBodyFloatValueFunction setRigidBodyHardnessFunc;

	/**
	 * @brief Function to set the linear damping on a rigid body.
	 */
	dsSetRigidBodyFloatValueFunction setRigidBodyLinearDampingFunc;

	/**
	 * @brief Function to set the angular damping on a rigid body.
	 */
	dsSetRigidBodyFloatValueFunction setRigidBodyAngularDampingFunc;

	/**
	 * @brief Function to set the max linear velocity on a rigid body.
	 */
	dsSetRigidBodyFloatValueFunction setRigidBodyMaxLinearVelocityFunc;

	/**
	 * @brief Function to set the max angular velocity on a rigid body.
	 */
	dsSetRigidBodyFloatValueFunction setRigidBodyMaxAngularVelocityFunc;

	/**
	 * @brief Function to get the linear velocity on a rigid body.
	 */
	dsGetRigidBodyVectorValueFunction getRigidBodyLinearVelocityFunc;

	/**
	 * @brief Function to set the linear velocity on a rigid body.
	 */
	dsSetRigidBodyVectorValueFunction setRigidBodyLinearVelocityFunc;

	/**
	 * @brief Function to get the angular velocity on a rigid body.
	 */
	dsGetRigidBodyVectorValueFunction getRigidBodyAngularVelocityFunc;

	/**
	 * @brief Function to set the angular velocity on a rigid body.
	 */
	dsSetRigidBodyVectorValueFunction setRigidBodyAngularVelocityFunc;

	/**
	 * @brief Function to add force to a rigid body.
	 */
	dsSetRigidBodyVectorValueFunction addRigidBodyForceFunc;

	/**
	 * @brief Function to clear the accumulated forces on a rigid body.
	 */
	dsClearRigidBodyForceFunction clearRigidBodyForceFunc;

	/**
	 * @brief Function to add torque to a rigid body.
	 */
	dsSetRigidBodyVectorValueFunction addRigidBodyTorqueFunc;

	/**
	 * @brief Function to clear the accumulated torque on a rigid body.
	 */
	dsClearRigidBodyForceFunction clearRigidBodyTorqueFunc;

	/**
	 * @brief Function to add linear impulse to a rigid body.
	 */
	dsSetRigidBodyVectorValueFunction addRigidBodyLinearImpulseFunc;

	/**
	 * @brief Function to clear the accumulated linear impulses on a rigid body.
	 */
	dsClearRigidBodyForceFunction clearRigidBodyLinearImpulseFunc;

	/**
	 * @brief Function to add angular impulse to a rigid body.
	 */
	dsSetRigidBodyVectorValueFunction addRigidBodyAngularImpulseFunc;

	/**
	 * @brief Function to clear the accumulated angular impulses on a rigid body.
	 */
	dsClearRigidBodyForceFunction clearRigidBodyAngularImpulseFunc;

	/**
	 * @brief Function to get whether a rigid body is active.
	 */
	dsGetRigidBodyActiveFunction getRigidBodyActiveFunc;

	/**
	 * @brief Function to set whether a rigid body is active.
	 */
	dsSetRigidBodyActiveFunction setRigidBodyActiveFunc;
};

#ifdef __cplusplus
}
#endif

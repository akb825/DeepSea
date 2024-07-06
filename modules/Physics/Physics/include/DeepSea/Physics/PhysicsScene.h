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

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Physics/Export.h>
#include <DeepSea/Physics/Types.h>
#include <math.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manipulate physics scenes.
 * @see dsPhysicsScene
 */

/**
 * @brief Default combine friction function used by the physics scene.
 * @param frictionA The first friction value.
 * @param frictionB The second friction value.
 * @return The combined friction value as a geometric mean, or sqrt(frictionA*frictionB).
 */
DS_PHYSICS_EXPORT inline float dsPhysicsScene_defaultCombineFriction(
	float frictionA, float frictionB);

/**
 * @brief Default combine friction restitution used by the physics scene.
 *
 * The hardness values are used to weigh between the restitution values. A harder surface (hardness
 * closer to 1) will bias more to the other restitution value, while a softer surface (hardness
 * closer to 0) will bias more to its restitution value. For example, concrete is a hard surface
 * that isn't very bouncy (low restitution, high hardness) but a rubber ball will bounce very well
 * off of it. On the other hand, a cushion (low restitution, low hardness) will have nothing bounce
 * well off of it.
 *
 * @param restitutionA The first restitution value.
 * @param hardnessA The first hardness value.
 * @param restitutionB The secon restitution value.
 * @param hardnessB The second hardness value.
 * @return The combined restitution value as a weighted average based on the hardness. The weight
 *     of restitutionA is sqrt(hardnessA*(1 - hardnessB)), while the weight of restitutionB is
 *     sqrt(hardnessB*(1 - hardnessA)).
 */
DS_PHYSICS_EXPORT inline float dsPhysicsScene_defaultCombineRestitution(
	float restitutionA, float hardnessA, float restitutionB, float hardnessB);

/**
 * @brief Creates a physics scene.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the physics scene with.
 * @param allocator The allocator to create the physics scene with. If NULL, it will use the same
 *     allocator as the physics engine.This must support freeing memory.
 * @param settings The settings for the physics scene.
 * @param threadPool The thread pool to use for multithreaded processing, or NULL for
 *     single-threaded processing.
 * @return The physics scene or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsPhysicsScene* dsPhysicsScene_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsSceneSettings* settings, dsThreadPool* threadPool);

/**
 * @brief Sets the combine friction function for a scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to set the combine function on.
 * @param combineFunc The combine friction function.
 * @return False if the combine friction function couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_setCombineFrictionFunction(dsPhysicsScene* scene,
	dsCombineFrictionFunction combineFunc);

/**
 * @brief Sets the combine restitution function for a scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to set the combine function on.
 * @param combineFunc The combine friction function.
 * @return False if the combine friction restitution couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_setCombineRestitutionFunction(dsPhysicsScene* scene,
	dsCombineRestitutionFunction combineFunc);

/**
 * @brief Combines two friction values.
 * @remark For performance this won't perform any error checks apart from asserts.
 * @param scene The physics scene.
 * @param frictionA The first friction value.
 * @param frictionB The second friction value.
 * @return The combined friction value.
 */
DS_PHYSICS_EXPORT inline float dsPhysicsScene_combineFriction(const dsPhysicsScene* scene,
	float frictionA, float frictionB);

/**
 * @brief Combines two restitution values.
 * @remark For performance this won't perform any error checks apart from asserts.
 * @param scene The physics scene.
 * @param restitutionA The first restitution value.
 * @param hardnessA The first hardness value.
 * @param restitutionB The second restitution value.
 * @param hardnessB The second hardness value.
 * @return The combined friction value.
 */
DS_PHYSICS_EXPORT inline float dsPhysicsScene_combineRestitution(const dsPhysicsScene* scene,
	float restitutionA, float hardnessA, float restitutionB, float hardnessB);

/**
 * @brief Sets the function to update a physics actor contact properties.
 * @remark errno will be set on failure.
 * @param scene The physics scene to set the function on.
 * @param function The function to call to update the physics actor contact properties.
 * @param userData The user data to provide to the callback function.
 * @param destroyUserDataFunc The function called to destroy the user data when the scene is
 *     destroyed, the update contact properties function is changed, or setting the function fails.
 * @return False if the function couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_setUpdateContactPropertiesFunction(dsPhysicsScene* scene,
	dsUpdatePhysicsActorContactPropertiesFunction function, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc);

/**
 * @brief Sets the function to respond a physics actor contact manifold being added.
 * @remark errno will be set on failure.
 * @param scene The physics scene to set the function on.
 * @param function The function to call when a physics actor contact manifold is added.
 * @param userData The user data to provide to the callback function.
 * @param destroyUserDataFunc The function called to destroy the user data when the scene is
 *     destroyed, the contact manifold added function is changed, or setting the function fails.
 * @return False if the function couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_setContactManifoldAddedFunction(dsPhysicsScene* scene,
	dsPhysicsActorContactManifoldFunction function, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc);

/**
 * @brief Sets the function to respond a physics actor contact manifold being updated.
 * @remark errno will be set on failure.
 * @param scene The physics scene to set the function on.
 * @param function The function to call when a physics actor contact manifold is updated.
 * @param userData The user data to provide to the callback function.
 * @param destroyUserDataFunc The function called to destroy the user data when the scene is
 *     destroyed, the contact manifold updated function is changed, or setting the function fails.
 * @return False if the function couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_setContactManifoldUpdatedFunction(dsPhysicsScene* scene,
	dsPhysicsActorContactManifoldFunction function, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc);

/**
 * @brief Sets the function to respond a physics actor contact manifold being removed.
 * @remark errno will be set on failure.
 * @param scene The physics scene to set the function on.
 * @param function The function to call when a physics actor contact manifold is removed.
 * @param userData The user data to provide to the callback function.
 * @param destroyUserDataFunc The function called to destroy the user data when the scene is
 *     destroyed, the contact manifold removed function is changed, or setting the function fails.
 * @return False if the function couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_setContactManifoldRemovedFunction(dsPhysicsScene* scene,
	dsPhysicsActorContactManifoldFunction function, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc);

/**
 * @brief Adds a callback function to before a physics scene is stepped.
 *
 * Multiple callbacks may be executed in parallel, allowing for effecient bulk updates. Components
 * of the physic scene may not be added or removed in the callback, and care should be made to not
 * modify the same objects from multiple callbacks to avoid potential thread contention.

 * @remark errno will be set on failure.
 * @param scene The physics scene to add the listener to.
 * @param function The function to call before a physics scene step.
 * @param userData The user data to provide to the listener.
 * @param destroyUserDataFunc The function called to destroy the user data when the scene is
 *     destroyed, the function is removed, or adding the function fails.
 * @return The ID for the added step listener or DS_INVALID_PHYSICS_ID if the listener couldn't be
 *     added.
 */
DS_PHYSICS_EXPORT uint32_t dsPhysicsScene_addPreStepListener(dsPhysicsScene* scene,
	dsOnPhysicsSceneStepFunction function, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc);

/**
 * @brief Removes a previously added pre-step listener.
 * @remark errno will be set on failure.
 * @param scene The physics scene to remove the listener from.
 * @param listenerID The ID for the listener returned from dsPhysicsScene_addPreStepListener().
 * @return False if the step listener couldn't be removed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_removePreStepListener(
	dsPhysicsScene* scene, uint32_t listenerID);

/**
 * @brief Adds a callback function to after a physics scene is stepped.
 *
 * Multiple callbacks may be executed in parallel, allowing for effecient bulk updates. Components
 * of the physic scene may not be added or removed in the callback, and care should be made to not
 * modify the same objects from multiple callbacks to avoid potential thread contention.

 * @remark errno will be set on failure.
 * @param scene The physics scene to add the listener to.
 * @param function The function to call after a physics scene step.
 * @param userData The user data to provide to the listener.
 * @param destroyUserDataFunc The function called to destroy the user data when the scene is
 *     destroyed, the function is removed, or adding the function fails.
 * @return The ID for the added step listener or DS_INVALID_PHYSICS_ID if the listener couldn't be
 *     added.
 */
DS_PHYSICS_EXPORT uint32_t dsPhysicsScene_addPostStepListener(dsPhysicsScene* scene,
	dsOnPhysicsSceneStepFunction function, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc);

/**
 * @brief Removes a previously added post-step listener.
 * @remark errno will be set on failure.
 * @param scene The physics scene to remove the listener from.
 * @param listenerID The ID for the listener returned from dsPhysicsScene_addPostStepListener().
 * @return False if the step listener couldn't be removed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_removePostStepListener(
	dsPhysicsScene* scene, uint32_t listenerID);

/**
 * @brief Sets the gravity on a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to set the gravity on.
 * @param gravity The new gravity for the physics scene.
 * @return False if the gravity couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_setGravity(dsPhysicsScene* scene, const dsVector3f* gravity);

/**
 * @brief Locks the physics scene for reading.
 * @remark errno will be set on failure.
 * @param[out] outLock The lock to populate.
 * @param scene The physics scene to lock.
 * @return False if the physics scene couldn't be locked.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_lockRead(dsPhysicsSceneLock* outLock, dsPhysicsScene* scene);

/**
 * @brief Unlocks the physics scene for reading.
 * @remark errno will be set on failure.
 * @param[inout] outLock The lock to update.
 * @param scene The physics scene to lock.
 * @return False if the physics scene couldn't be unlocked.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_unlockRead(
	dsPhysicsSceneLock* outLock, dsPhysicsScene* scene);

/**
 * @brief Locks the physics scene for writing.
 * @remark errno will be set on failure.
 * @param[out] outLock The lock to populate.
 * @param scene The physics scene to lock.
 * @return False if the physics scene couldn't be locked.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_lockWrite(dsPhysicsSceneLock* outLock, dsPhysicsScene* scene);

/**
 * @brief Unlocks the physics scene for writing.
 * @remark errno will be set on failure.
 * @param[inout] outLock The lock to update.
 * @param scene The physics scene to lock.
 * @return False if the physics scene couldn't be unlocked.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_unlockWrite(
	dsPhysicsSceneLock* outLock, dsPhysicsScene* scene);

/**
 * @brief Adds rigid bodies to a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to add the rigid bodies to.
 * @param rigidBodies The rigid bodies to add. These must not be part of a rigid body group.
 * @param rigidBodyCount The number of rigid bodies to add.
 * @param activate Whether the rigid bodies should be activated on insertion.
 * @param lock The previously acquired lock. This must have been locked for writing.
 * @return False if the rigid body couldn't be added.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_addRigidBodies(dsPhysicsScene* scene,
	dsRigidBody* const* rigidBodies, uint32_t rigidBodyCount, bool activate,
	const dsPhysicsSceneLock* lock);

/**
 * @brief Removes rigid bodies from a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to remove the rigid bodies from.
 * @param rigidBodies The rigid bodies to remove. These must not be part of a rigid body group.
 * @param rigidBodyCount The number of rigid bodies to remove.
 * @param lock The previously acquired lock. This must have been locked for writing.
 * @return False if the rigid body couldn't be removed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_removeRigidBodies(dsPhysicsScene* scene,
	dsRigidBody* const* rigidBodies, uint32_t rigidBodyCount, const dsPhysicsSceneLock* lock);

/**
 * @brief Adds a rigid body group to a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to add the rigid body group to.
 * @param group The rigid body group to add.
 * @param activate Whether the rigid bodies should be activated on insertion.
 * @param lock The previously acquired lock. This must have been locked for writing.
 * @return False if the rigid body group couldn't be added.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_addRigidBodyGroup(dsPhysicsScene* scene,
	dsRigidBodyGroup* group, bool activate, const dsPhysicsSceneLock* lock);

/**
 * @brief Removes a rigid body group from a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to remove the rigid body group from.
 * @param group The rigid body group to remove.
 * @param lock The previously acquired lock. This must have been locked for writing.
 * @return False if the rigid body group couldn't be removed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_removeRigidBodyGroup(dsPhysicsScene* scene,
	dsRigidBodyGroup* group, const dsPhysicsSceneLock* lock);

/**
 * @brief Gets actors from a physics scene.
 *
 * The ordering may change when actors are added or removed.
 *
 * @remark errno will be set on failure.
 * @param[out] outActors Storage for the actor pointers. This must have space for at least count
 *     pointers.
 * @param scene The physics scene to get the actors from.
 * @param firstIndex The first index to get actors from.
 * @param count The number of actors to get.
 * @param lock The previously acquired lock. This must have been locked for reading or writing.
 * @return The number of actors that were populated or DS_INVALID_PHYSICS_ID if the actors couldn't
 *     be queried.
 */
DS_PHYSICS_EXPORT uint32_t dsPhysicsScene_getActors(dsPhysicsActor** outActors,
	const dsPhysicsScene* scene, uint32_t firstIndex, uint32_t count,
	const dsPhysicsSceneLock* lock);

/**
 * @brief Adds constraints to a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to add the constraints to.
 * @param constraints The constraints to add. All constraints must be valid, and the actors the
 *     constraints reference must have already been added to this scene.
 * @param constraintCount The number of constraints to add.
 * @param enable Whether the constraints should be enabled on insertion.
 * @param lock The previously acquired lock. This must have been locked for writing.
 * @return False if the constraints couldn't be added.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_addConstraints(dsPhysicsScene* scene,
	dsPhysicsConstraint* const* constraints, uint32_t constraintCount, bool enable,
	const dsPhysicsSceneLock* lock);

/**
 * @brief Removes constraints from a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to remove the constraints from.
 * @param constraints The constraints to remove.
 * @param constraintCount The number of constraints to remove.
 * @param lock The previously acquired lock. This must have been locked for writing.
 * @return False if the constraints couldn't be removed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_removeConstraints(dsPhysicsScene* scene,
	dsPhysicsConstraint* const* constraints, uint32_t constraintCount,
	const dsPhysicsSceneLock* lock);

/**
 * @brief Gets constraints from a physics scene.
 *
 * The ordering may change when constraints are added or removed.
 *
 * @remark errno will be set on failure.
 * @param[out] outConstraints Storage for the constraint pointers. This must have space for at least
 *     count pointers.
 * @param scene The physics scene to get the constraints from.
 * @param firstIndex The first index to get constraints from.
 * @param count The number of constraints to get.
 * @param lock The previously acquired lock. This must have been locked for reading or writing.
 * @return The number of constraints that were populated or DS_INVALID_PHYSICS_ID if the actors
 *     couldn't be queried.
 */
DS_PHYSICS_EXPORT uint32_t dsPhysicsScene_getConstraints(dsPhysicsConstraint** outConstraints,
	const dsPhysicsScene* scene, uint32_t firstIndex, uint32_t count,
	const dsPhysicsSceneLock* lock);

/**
 * @brief Casts a ray with a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to cast the ray with.
 * @param ray The ray to intersect with the scene. The direction should be scaled by the maximum
 *     distance of the intersection.
 * @param queryType The query type to perform.
 * @param userData The user data to forward to the callback functions.
 * @param layer The physics layer to perform the query on. This will follow the same rules as any
 *     physics actor in the same layer.
 * @param collisionGroup The collision group of the ray.
 * @param canCollisionGroupsCollideFunc The function to call for whether collision groups can
 *     collide. This will be called in place of any actor's canCollisionGroupsCollideFunc if set,
 *     otherwise the actor's function will be called if present.
 * @param canCollidePhysicsActorFunc The function to call for whether a specific physics actor and
 *     shape may collide with the query. If NULL all actors that pass the layer and collision group
 *     tests will be intersected.
 * @param addResultFunc Function to call to add a collision result. This may be NULL to simply count
 *     the number of intersections.
 * @param lock The previously acquired lock. This must have been locked for reading or writing.
 * @return The number of collisions or DS_INVALID_PHYSICS_ID if an error occurred.
 */
DS_PHYSICS_EXPORT uint32_t dsPhysicsScene_castRay(const dsPhysicsScene* scene,
	const dsRay3f* ray, dsPhysicsQueryType queryType, void* userData, dsPhysicsLayer layer,
	uint64_t collisionGroup, dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc,
	dsCanIntersectPhysicsActorFunction canCollidePhysicsActorFunc,
	dsAddPhysicsRayIntersectionResult addResultFunc, const dsPhysicsSceneLock* lock);

/**
 * @brief Intersects shapes with a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to cast the shapes with.
 * @param shapes The shapes to intersect with the scene.
 * @param shapeCount The number of shapes to intersect.
 * @param queryType The query type to perform.
 * @param userData The user data to forward to the callback functions.
 * @param layer The physics layer to perform the query on. This will follow the same rules as any
 *     physics actor in the same layer.
 * @param collisionGroup The collision group of the shapes.
 * @param canCollisionGroupsCollideFunc The function to call for whether collision groups can
 *     collide. This will be called in place of any actor's canCollisionGroupsCollideFunc if set,
 *     otherwise the actor's function will be called if present.
 * @param canCollidePhysicsActorFunc The function to call for whether a specific physics actor and
 *     shape may collide with the query. If NULL all actors that pass the layer and collision group
 *     tests will be intersected.
 * @param addResultFunc Function to call to add a collision result. This may be NULL to simply count
 *     the number of intersections.
 * @param lock The previously acquired lock. This must have been locked for reading or writing.
 * @return The number of collisions or DS_INVALID_PHYSICS_ID if an error occurred.
 */
DS_PHYSICS_EXPORT uint32_t dsPhysicsScene_intersectShapes(const dsPhysicsScene* scene,
	const dsPhysicsShapeInstance* shapes, uint32_t shapeCount, dsPhysicsQueryType queryType,
	void* userData, dsPhysicsLayer layer, uint64_t collisionGroup,
	dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc,
	dsCanIntersectPhysicsActorFunction canCollidePhysicsActorFunc,
	dsAddPhysicsShapeIntersectionResult addResultFunc,
	const dsPhysicsSceneLock* lock);

/**
 * @brief updates the simulation for the physics scene.
 *
 * This will implicilty lock the scene for writing for the duration of the update.
 *
 * @remark errno will be set on failure.
 * @param scene The physics scene to update.
 * @param time The total amount of time to advance the physics simulation. This must be >= 0.
 * @param stepCount The number of steps to perform to update the simulation. This must be at least
 *     1.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_update(
	dsPhysicsScene* scene, float time, unsigned int stepCount);

/**
 * @brief Destroys a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to destroy.
 * @return False if the scene couldn't be destroyed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_destroy(dsPhysicsScene* scene);

/**
 * @brief Initializes a physics scene.
 *
 * This is called by the physics implementation to initialize the common members.
 *
 * @remark errno will be set on failure.
 * @param[out] scene The physics scene to initialize.
 * @param engine The physics engine the scene was created with.
 * @param allocator The allocator the scene was created with.
 * @param settings The settings for the physics scene.
 * @return False if internal allocations failed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_initialize(dsPhysicsScene* scene, dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsSceneSettings* settings);

/**
 * @brief Shuts down a physics scene.
 *
 * This is called by the physics implementation to shut down the common members.
 *
 * @param scene The scene to shut down.
 */
DS_PHYSICS_EXPORT void dsPhysicsScene_shutdown(dsPhysicsScene* scene);

inline float dsPhysicsScene_defaultCombineFriction(float frictionA, float frictionB)
{
	DS_ASSERT(frictionA >= 0);
	DS_ASSERT(frictionB >= 0);
	return sqrtf(frictionA*frictionB);
}

inline float dsPhysicsScene_defaultCombineRestitution(
	float restitutionA, float hardnessA, float restitutionB, float hardnessB)
{
	DS_ASSERT(hardnessA >= 0);
	DS_ASSERT(hardnessB >= 0);

	// Use average if both hardness values are 0 to avoid divide by zero.
	if (hardnessA == 0 && hardnessB == 0)
		return (restitutionA + restitutionB)*0.5f;

	float weightA = sqrtf(hardnessA*(1 - hardnessB));
	float weightB = sqrtf(hardnessB*(1 - hardnessA));
	return (restitutionA*weightA + restitutionB*weightB)/(weightA + weightB);
}

inline float dsPhysicsScene_combineFriction(const dsPhysicsScene* scene,
	float frictionA, float frictionB)
{
	DS_ASSERT(scene);
	DS_ASSERT(scene->combineFrictionFunc);
	return scene->combineFrictionFunc(frictionA, frictionB);
}

inline float dsPhysicsScene_combineRestitution(const dsPhysicsScene* scene,
	float restitutionA, float hardnessA, float restitutionB, float hardnessB)
{
	DS_ASSERT(scene);
	DS_ASSERT(scene->combineRestitutionFunc);
	return scene->combineRestitutionFunc(restitutionA, hardnessA, restitutionB, hardnessB);
}

#ifdef __cplusplus
}
#endif

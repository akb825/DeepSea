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
 *     allocator as the physics engine.
 * @param limits The limits for the physics scene.
 * @param threadPool The thread pool to use for multithreaded processing, or NULL for
 *     single-threaded processing.
 * @return The physics scene or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsPhysicsScene* dsPhysicsScene_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsSceneLimits* limits, dsThreadPool* threadPool);

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
 * @brief Adds rigid bodies to a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to add the rigid body to.
 * @param rigidBodies The rigid bodies to add. These must not be part of a rigid body group.
 * @param rigidBodyCount The number of rigid bodies to add.
 * @param activate Whether the rigid bodies should be activated on insertion.
 * @return False if the rigid body couldn't be added.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_addRigidBodies(dsPhysicsScene* scene,
	dsRigidBody* const* rigidBodies, uint32_t rigidBodyCount, bool activate);

/**
 * @brief Removes a rigid body from a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to remove the rigid body from.
 * @param rigidBodies The rigid bodies to remove. These must not be part of a rigid body group.
 * @param rigidBodyCount The number of rigid bodies to remove.
 * @return False if the rigid body couldn't be removed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_removeRigidBodies(dsPhysicsScene* scene,
	dsRigidBody* const* rigidBodies, uint32_t rigidBodyCount);

/**
 * @brief Adds a rigid body group to a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to add the rigid body group to.
 * @param group The rigid body group to add.
 * @param activate Whether the rigid bodies should be activated on insertion.
 * @return False if the rigid body group couldn't be added.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_addRigidBodyGroup(dsPhysicsScene* scene,
	dsRigidBodyGroup* group, bool activate);

/**
 * @brief Removes a rigid body group from a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to remove the rigid body group from.
 * @param group The rigid body group to remove.
 * @return False if the rigid body group couldn't be removed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_removeRigidBodyGroup(dsPhysicsScene* scene,
	dsRigidBodyGroup* group);

/**
 * @brief Sets the function to update a physics actor contact properties.
 * @remark errno will be set on failure.
 * @param scene The scene to set the function on.
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
 * @param scene The scene to set the function on.
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
 * @param scene The scene to set the function on.
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
 * @param scene The scene to set the function on.
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
 * @brief Destroys a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to destroy.
 * @return False if the scene couldn't be destroyed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_destroy(dsPhysicsScene* scene);

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

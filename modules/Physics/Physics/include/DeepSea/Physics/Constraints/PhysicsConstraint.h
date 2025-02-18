/*
 * Copyright 2024-2025 Aaron Barany
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
#include <DeepSea/Physics/Types.h>
#include <DeepSea/Physics/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for manipulating physics constraints.
 * @see dsPhysicsConstraint
 */

/**
 * @brief Initializes a physics constraint.
 *
 * This is called by the specific shape's initialize functions.
 *
 * @remark errno will be set on failure.
 * @param constraint The constraint to initialize.
 * @param engine The physics engine the constraint was created with.
 * @param allocator The allocator the constraint was created with.
 * @param type The type of the constraint.
 * @param firstActor The first actor for the constraint.
 * @param secondActor The second actor for the constraint.
 * @param impl The underlying implementation of the constraint.
 * @param setEnabledFunc Function to set whether the constraint is enabled.
 * @param getForceFunc Function to get the last applied force for the constraint. This may be NULL
 *     for constraints that have no limits on position.
 * @param getTorqueFunc Function to get the last applied torque for the constraint. This may be NULL
 *     for constraints that have no limits on orientation.
 * @param destroyFunc The destroy function of the constraint.
 * @return False if the parameters are invalid.
 */
DS_PHYSICS_EXPORT bool dsPhysicsConstraint_initialize(dsPhysicsConstraint* constraint,
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsConstraintType* type,
	const dsPhysicsActor* firstActor, const dsPhysicsActor* secondActor, void* impl,
	dsSetPhysicsConstraintEnabledFunction setEnabledFunc,
	dsGetPhysicsConstraintForceFunction getForceFunc,
	dsGetPhysicsConstraintForceFunction getTorqueFunc,
	dsDestroyPhysicsConstraintFunction destroyFunc);

/**
 * @brief Loads a physics constraint from a file.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with. If NULL the engine's allocator will
 *     be used.
 * @param findActorFunc Function to find an actor by name. This is required.
 * @param findActorUserData User data to pass to findActorFunc.
 * @param findConstraintFunc Function to find a constraint by name. This will be used if a
 *     constraint reference is used. All lookups will fail if this function is NULL.
 * @param findConstraintUserData User data to pass to findConstraintFunc.
 * @param filePath The file path for the physics constraint to load.
 * @return The loaded physics constraint or NULL if it couldn't be loaded.
 */
DS_PHYSICS_EXPORT dsPhysicsConstraint* dsPhysicsConstraint_loadFile(dsPhysicsEngine* engine,
	dsAllocator* allocator, dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	dsFindPhysicsConstraintFunction findConstraintFunc, void* findConstraintUserData,
	const char* filePath);

/**
 * @brief Loads a physics constraint from a resource file.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with. If NULL the engine's allocator will
 *     be used.
 * @param findActorFunc Function to find an actor by name. This is required.
 * @param findActorUserData User data to pass to findActorFunc.
 * @param findConstraintFunc Function to find a constraint by name. This will be used if a
 *     constraint reference is used. All lookups will fail if this function is NULL.
 * @param findConstraintUserData User data to pass to findConstraintFunc.
 * @param type The type of resource to load.
 * @param filePath The file path for the physics constraint to load.
 * @return The loaded physics constraint or NULL if it couldn't be loaded.
 */
DS_PHYSICS_EXPORT dsPhysicsConstraint* dsPhysicsConstraint_loadResource(dsPhysicsEngine* engine,
	dsAllocator* allocator, dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	dsFindPhysicsConstraintFunction findConstraintFunc, void* findConstraintUserData,
	dsFileResourceType type, const char* filePath);

/**
 * @brief Loads a physics constraint from a file within an archive.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with. If NULL the engine's allocator will
 *     be used.
 * @param findActorFunc Function to find an actor by name. This is required.
 * @param findActorUserData User data to pass to findActorFunc.
 * @param findConstraintFunc Function to find a constraint by name. This will be used if a
 *     constraint reference is used. All lookups will fail if this function is NULL.
 * @param findConstraintUserData User data to pass to findConstraintFunc.
 * @param archive The archive to load the physics constraint from.
 * @param filePath The file path for the physics constraint to load.
 * @return The loaded physics constraint or NULL if it couldn't be loaded.
 */
DS_PHYSICS_EXPORT dsPhysicsConstraint* dsPhysicsConstraint_loadArchive(dsPhysicsEngine* engine,
	dsAllocator* allocator, dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	dsFindPhysicsConstraintFunction findConstraintFunc, void* findConstraintUserData,
	const dsFileArchive* archive, const char* filePath);

/**
 * @brief Loads a physics constraint from a stream.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with. If NULL the engine's allocator will
 *     be used.
 * @param findActorFunc Function to find an actor by name. This is required.
 * @param findActorUserData User data to pass to findActorFunc.
 * @param findConstraintFunc Function to find a constraint by name. This will be used if a
 *     constraint reference is used. All lookups will fail if this function is NULL.
 * @param findConstraintUserData User data to pass to findConstraintFunc.
 * @param stream The stream to load from.
 * @return The loaded physics constraint or NULL if it couldn't be loaded.
 */
DS_PHYSICS_EXPORT dsPhysicsConstraint* dsPhysicsConstraint_loadStream(dsPhysicsEngine* engine,
	dsAllocator* allocator, dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	dsFindPhysicsConstraintFunction findConstraintFunc, void* findConstraintUserData,
	dsStream* stream);

/**
 * @brief Loads a physics constraint from a data buffer.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with. If NULL the engine's allocator will
 *     be used.
 * @param findActorFunc Function to find an actor by name. This is required.
 * @param findActorUserData User data to pass to findActorFunc.
 * @param findConstraintFunc Function to find a constraint by name. This will be used if a
 *     constraint reference is used. All lookups will fail if this function is NULL.
 * @param findConstraintUserData User data to pass to findConstraintFunc.
 * @param data The data buffer to load from.
 * @param size The size of the data buffer.
 * @return The loaded physics constraint or NULL if it couldn't be loaded.
 */
DS_PHYSICS_EXPORT dsPhysicsConstraint* dsPhysicsConstraint_loadData(dsPhysicsEngine* engine,
	dsAllocator* allocator, dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	dsFindPhysicsConstraintFunction findConstraintFunc, void* findConstraintUserData,
	const void* data, size_t size);

/**
 * @brief Clones a physics constraint.
 * @remark errno will be set on failure.
 * @param constraint The constraint to clone.
 * @param allocator The allocator to create the cloned constraint with. If NULL the original
 *     constraint's allocator will be used.
 * @param firstActor The first actor for the cloned constraint. If NULL the original constraint's
 *     first actor will be used.
 * @param firstConnectedConstraint Optional constraint for the first actor that is related to
 *     this constraint.
 * @param secondActor The second actor for the cloned constraint. If NULL the original constraint's
 *     second actor will be used.
 * @param secondConnectedConstraint Optional constraint for the second actor that is related to
 *     this constraint.
 * @return The constraint or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsPhysicsConstraint* dsPhysicsConstraint_clone(
	const dsPhysicsConstraint* constraint, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsPhysicsConstraint* firstConnectedConstraint, const dsPhysicsActor* secondActor,
	const dsPhysicsConstraint* secondConnectedConstraint);

/**
 * @brief Function to check whether a physics constraint is valid for use.
 *
 * A constraint is valid if both actors are set. An invalid constraint may be used as a template to
 * clone.
 *
 * @param constraint The constraing to check.
 * @return Whether the constraint is valid.
 */
DS_PHYSICS_EXPORT inline bool dsPhysicsConstraint_isValid(const dsPhysicsConstraint* constraint);

/**
 * @brief Sets whether a physics constraint is enabled.
 *
 * The constraint must be a part of a scene before it can be enabled. When a constraint is disabled
 * it will not be enforced.
 *
 * @remark errno will be set on failure.
 * @param constraint The constraint to set the enabled state on.
 * @param enabled Whether the constraint is enabled.
 * @return False if the enabled state couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsPhysicsConstraint_setEnabled(
	dsPhysicsConstraint* constraint, bool enabled);

/**
 * @brief Gets the force applied in the previous step to enforce a physics constraint.
 *
 * It is only valid to query the last applied force for an enabled constraint.
 *
 * @remark errno will be set on failure.
 * @param[out] outForce The last applied force for the constraint. This will be relative to the
 *     first body of the constraint.
 * @param constraint The physics constraint to get the force for.
 * @return False if the force couldn't be queried.
 */
DS_PHYSICS_EXPORT bool dsPhysicsConstraint_getLastAppliedForce(dsVector3f* outForce,
	const dsPhysicsConstraint* constraint);

/**
 * @brief Gets the torque applied in the previous step to enforce a physics constraint.
 *
 * It is only valid to query the last applied torque for an enabled constraint.
 *
 * @remark errno will be set on failure.
 * @param[out] outTorque The last applied torque for the constraint. This will be relative to the
 *     first body of the constraint.
 * @param constraint The physics constraint to get the torque for.
 * @return False if the force couldn't be queried.
 */
DS_PHYSICS_EXPORT bool dsPhysicsConstraint_getLastAppliedTorque(dsVector3f* outTorque,
	const dsPhysicsConstraint* constraint);

/**
 * @brief Destroys a physics constraint.
 *
 * If the constraint is part of a physics scene it will be implicitly removed.
 *
 * @remark errno will be set on failure.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsConstraint_destroy(dsPhysicsConstraint* constraint);

inline bool dsPhysicsConstraint_isValid(const dsPhysicsConstraint* constraint)
{
	return constraint && constraint->firstActor && constraint->secondActor;
}

#ifdef __cplusplus
}
#endif

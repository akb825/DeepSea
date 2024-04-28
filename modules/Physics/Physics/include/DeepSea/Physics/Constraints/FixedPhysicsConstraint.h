/*
 * Copyright 2024 Aaron Barany
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
#include <DeepSea/Physics/Constraints/Types.h>
#include <DeepSea/Physics/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating fixed physics constraints.
 * @see dsFixedPhysicsConstraint
 */

/**
 * @brief Gets the type for a fixed physics constraint.
 * @return The type for a fixed physics constraint.
 */
DS_PHYSICS_EXPORT dsPhysicsConstraintType dsFixedPhysicsConstraint_type(void);

/**
 * @brief Creates a fixed physics constraint.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with. If NULL the engine's allocator will
 *     be used.
 * @param enabled Whether the constraint is enabled after creation.
 * @param firstActor The first physics actor the constraint is attached to.
 * @param firstPosition The position of the constraint on the first actor.
 * @param firstRotation The rotation of the constraint on the first actor.
 * @param secondActor The second physics actor the constraint is attached to.
 * @param secondPosition The position of the constraint on the second actor.
 * @param secondRotation The rotation of the constraint on the second actor.
 * @return The fixed constraint or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsFixedPhysicsConstraint* dsFixedPhysicsConstraint_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, bool enabled, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstRotation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondRotation);

/**
 * @brief Initializes a fixed physics constraint.
 *
 * This is called by the physics implementation to initialize the common members.
 *
 * @param[out] constraint The constraint to initialize.
 * @param engine The physics engine the constraint was created with.
 * @param allocator The allocator the constraint was created with.
 * @param enabled Whether the constraint is enabled after creation.
 * @param firstActor The first physics actor the constraint is attached to.
 * @param firstPosition The position of the constraint on the first actor.
 * @param firstRotation The rotation of the constraint on the first actor.
 * @param secondActor The second physics actor the constraint is attached to.
 * @param secondPosition The position of the constraint on the second actor.
 * @param secondRotation The rotation of the constraint on the second actor.
 * @param impl The underlying implementation for the constraint.
 * @param getForceFunc Function to get the last applied force for the constraint.
 * @param getTorqueFunc Function to get the last applied torque for the constraint.
 */
DS_PHYSICS_EXPORT void dsFixedPhysicsConstraint_initialize(dsFixedPhysicsConstraint* constraint,
	dsPhysicsEngine* engine, dsAllocator* allocator, bool enabled, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstRotation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondRotation, void* impl,
	dsGetPhysicsConstraintForceFunction getForceFunc,
	dsGetPhysicsConstraintForceFunction getTorqueFunc);

#ifdef __cplusplus
}
#endif

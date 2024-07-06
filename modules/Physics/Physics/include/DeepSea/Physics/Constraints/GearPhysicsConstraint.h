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
 * @brief Functions for creating and manipulating gear physics constraints.
 * @see dsGearPhysicsConstraint
 */

/**
 * @brief Gets the type for a gear physics constraint.
 * @return The type for a gear physics constraint.
 */
DS_PHYSICS_EXPORT const dsPhysicsConstraintType* dsGearPhysicsConstraint_type(void);

/**
 * @brief Computes the ratio for a gear physics constraint.
 * @remark errno will be set on failure.
 * @param firstActorToothCount The number of teeth for the first actor.
 * @param secondActorToothCount The number of teeth for the second actor.
 * @return The ratio or 0 if the parameters are invalid.
 */
DS_PHYSICS_EXPORT float dsGearPhysicsConstraint_computeRatio(unsigned int firstActorToothCount,
	unsigned int secondActorToothCount);

/**
 * @brief Creates a gear physics constraint.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with. If NULL the engine's allocator will
 *     be used.
 * @param firstActor The first physics actor the constraint is attached to. This may be NULL to set
 *     later by cloning.
 * @param firstAxis The axis of rotation for the first actor.
 * @param firstConstraint The revolute constraint for the first actor. This may be NULL, but
 *     providing it can avoid angle drift over time.
 * @param secondActor The second physics actor the constraint is attached to. This may be NULL to
 *     set later by cloning.
 * @param secondAxis The axis of rotation for the second actor.
 * @param secondConstraint The revolute constraint for the second actor. This may be NULL, but
 *     providing it can avoid angle drift over time.
 * @param ratio The gear ratio between the two actors. The ratio is defined as
 *     firstActorToothCount/secondActorToothCount. The ratio may be negative if the axes are
 *     flipped.
 * @return The gear constraint or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsGearPhysicsConstraint* dsGearPhysicsConstraint_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsActor* firstActor, const dsVector3f* firstAxis,
	const dsRevolutePhysicsConstraint* firstConstraint, const dsPhysicsActor* secondActor,
	const dsVector3f* secondAxis, const dsRevolutePhysicsConstraint* secondConstraint, float ratio);

/**
 * @brief Sets the gear ratio for a gear physics constraint.
 * @remark errno will be set on failure.
 * @param constraint The gear constraint to set the gear ratio on.
 * @param ratio The gear ratio between the two actors. The ratio is defined of
 *     firstActorToothCount/secondActorToothCount. The ratio may be negative if the axes aren't
 *     aligned.
 * @return False if the ratio couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsGearPhysicsConstraint_setRatio(dsGearPhysicsConstraint* constraint,
	float ratio);

/**
 * @brief Initializes a gear physics constraint.
 *
 * This is called by the physics implementation to initialize the common members.
 *
 * @param[out] constraint The constraint to initialize.
 * @param engine The physics engine the constraint was created with.
 * @param allocator The allocator the constraint was created with.
 * @param firstActor The first physics actor the constraint is attached to.
 * @param firstAxis The axis of rotation for the first actor.
 * @param firstConstraint The revolute constraint for the first actor. This may be NULL, but
 *     providing it can avoid angle drift over time.
 * @param secondActor The second physics actor the constraint is attached to.
 * @param secondAxis The axis of rotation for the second actor.
 * @param secondConstraint The revolute constraint for the second actor. This may be NULL, but
 *     providing it can avoid angle drift over time.
 * @param ratio The gear ratio between the two actors. The ratio is defined as
 *     firstActorToothCount/secondActorToothCount. The ratio may be negative if the axes are
 *     flipped.
 * @param impl The underlying implementation for the constraint.
 */
DS_PHYSICS_EXPORT void dsGearPhysicsConstraint_initialize(dsGearPhysicsConstraint* constraint,
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstAxis, const dsRevolutePhysicsConstraint* firstConstraint,
	const dsPhysicsActor* secondActor, const dsVector3f* secondAxis,
	const dsRevolutePhysicsConstraint* secondConstraint, float ratio, void* impl);

#ifdef __cplusplus
}
#endif

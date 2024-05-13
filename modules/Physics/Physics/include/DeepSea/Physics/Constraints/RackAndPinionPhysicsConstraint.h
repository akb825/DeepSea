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
 * @brief Functions for creating and manipulating rack and pinion physics constraints.
 * @see dsRackAndPinionPhysicsConstraint
 */

/**
 * @brief Gets the type for a rack and pinion physics constraint.
 * @return The type for a rack and pinion physics constraint.
 */
DS_PHYSICS_EXPORT dsPhysicsConstraintType dsRackAndPinionPhysicsConstraint_type(void);

/**
 * @brief Computes the ratio for a rack and pinion physics constraint.
 * @remark errno will be set on failure.
 * @param rackToothCount The number of teeth for the rack actor.
 * @param rackLength The length of the rack.
 * @param pinionToothCount The number of teeth for the pinion actor.
 * @return The ratio or 0 if the parameters are invalid.
 */
DS_PHYSICS_EXPORT float dsRackAndPinionPhysicsConstraint_computeRatio(unsigned int rackToothCount,
	float rackLength, unsigned int pinionToothCount);

/**
 * @brief Creates a rack and pinion physics constraint.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with. If NULL the engine's allocator will
 *     be used.
 * @param enabled Whether the constraint is enabled after creation.
 * @param rackActor The physics actor for the rack the constraint is attached to.
 * @param rackAxis The axis of translation for the rack actor.
 * @param rackConstraint The slider constraint for the rack actor.This may be NULL, but providing it
 *     can avoid positional drift over time.
 * @param pinionActor The physics actor for the pinion the constraint is attached to.
 * @param pinionAxis The axis of rotation for the pinion actor.
 * @param pinionConstraint The revolute constraint for the pinion actor. This may be NULL, but
 *     providing it can avoid angle drift over time.
 * @param ratio The rack and pinion ratio between the two actors. The ratio is defined as
 *     2*PI*rackToothCount/(rackLength*pinionToothCount). The ratio may be negative if the axes are
 *     flipped.
 * @return The rack and pinion constraint or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsRackAndPinionPhysicsConstraint* dsRackAndPinionPhyiscsConstraint_create(
	dsPhysicsEngine* engine, dsAllocator* allocator, bool enabled, const dsPhysicsActor* rackActor,
	const dsVector3f* rackAxis, const dsSliderPhysicsConstraint* rackConstraint,
	const dsPhysicsActor* pinionActor, const dsVector3f* pinionAxis,
	const dsRevolutePhysicsConstraint* pinionConstraint, float ratio);

/**
 * @brief Sets the rack and pinion ratio for a rack and pinion physics constraint.
 * @remark errno will be set on failure.
 * @param constraint The rack and pinion constraint to set the rack and pinion ratio on.
 * @param ratio The rack and pinion ratio between the two actors. The ratio is defined of
 *     firstActorToothCount/secondActorToothCount. The ratio may be negative if the axes aren't
 *     aligned.
 * @return False if the ratio couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRackAndPinionPhysicsConstraint_setRatio(dsRackAndPinionPhysicsConstraint* constraint,
	float ratio);

/**
 * @brief Initializes a rack and pinion physics constraint.
 *
 * This is called by the physics implementation to initialize the common members.
 *
 * @param[out] constraint The constraint to initialize.
 * @param engine The physics engine the constraint was created with.
 * @param allocator The allocator the constraint was created with.
 * @param enabled Whether the constraint is enabled after creation.
 * @param rackActor The physics actor for the rack the constraint is attached to.
 * @param rackAxis The axis of translation for the rack actor.
 * @param rackConstraint The slider constraint for the rack actor.This may be NULL, but providing it
 *     can avoid positional drift over time.
 * @param pinionActor The physics actor for the pinion the constraint is attached to.
 * @param pinionAxis The axis of rotation for the pinion actor.
 * @param pinionConstraint The revolute constraint for the pinion actor. This may be NULL, but
 *     providing it can avoid angle drift over time.
 * @param ratio The rack and pinion ratio between the two actors. The ratio is defined as
 *     2*PI*rackToothCount/(rackLength*pinionToothCount). The ratio may be negative if the axes are
 *     flipped.
 * @param impl The underlying implementation for the constraint.
 * @param getForceFunc Function to get the last applied force for the constraint.
 * @param getTorqueFunc Function to get the last applied torque for the constraint.
 */
DS_PHYSICS_EXPORT void dsRackAndPinionPhyiscsConstraint_initialize(
	dsRackAndPinionPhysicsConstraint* constraint, dsPhysicsEngine* engine, dsAllocator* allocator,
	bool enabled, const dsPhysicsActor* rackActor, const dsVector3f* rackAxis,
	const dsSliderPhysicsConstraint* rackConstraint,
	const dsPhysicsActor* pinionActor, const dsVector3f* pinionAxis,
	const dsRevolutePhysicsConstraint* pinionConstraint, float ratio, void* impl,
	dsGetPhysicsConstraintForceFunction getForceFunc,
	dsGetPhysicsConstraintForceFunction getTorqueFunc);

#ifdef __cplusplus
}
#endif

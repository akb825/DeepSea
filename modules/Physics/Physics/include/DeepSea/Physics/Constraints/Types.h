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
#include <DeepSea/Core/Types.h>
#include <DeepSea/Math/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/// @cond
typedef struct dsPhysicsActor dsPhysicsActor;
typedef struct dsPhysicsConstraint dsPhysicsConstraint;
typedef struct dsPhysicsEngine dsPhysicsEngine;
/// @endcond

/**
 * @file
 * @brief Includes all of the types for constraints in the DeepSea/Physics library.
 */

/**
 * @brief Value that denotes the type of a physics constraint.
 */
typedef const int* dsPhysicsConstraintType;

/**
 * @brief Function to destroy a physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
typedef bool (*dsDestroyPhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsPhysicsConstraint* constraint);

/**
 * @brief Function to get the applied force for a physics constraint.
 * @param[out] outForce The force applied to the constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constraint to get the force for.
 * @return False if the force couldn't be queried.
 */
typedef bool (*dsGetPhysicsConstraintForceFunction)(dsVector3f* outForce,
	dsPhysicsEngine* engine, const dsPhysicsConstraint* constraint);

/**
 * @brief Base type for a physics constraint.
 *
 * Constraints place requirements of the positions of physics bodies relative to each-other. Forces
 * will be applied to the bodies to ensure the constraints are satisfied to the best of the physics
 * engine's capabilities.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see PhysicsConstraint.h
 */
typedef struct dsPhysicsConstraint
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
	 * @brief The type of the constraint.
	 */
	dsPhysicsConstraintType type;

	/**
	 * @brief Whether the constraint is enabled.
	 */
	bool enabled;

	/**
	 * @brief The first actor for the constraint.
	 */
	const dsPhysicsActor* firstActor;

	/**
	 * @brief The second actor for the constraint.
	 */
	const dsPhysicsActor* secondActor;

	/**
	 * @brief Pointer to the constraint implementation.
	 *
	 * This is a convenience to avoid needing to check the type to get the underlying constraint for
	 * the physics implementation.
	 */
	void* impl;

	/**
	 * @brief Function to get the applied force for the constraint.
	 */
	dsGetPhysicsConstraintForceFunction getForceFunc;

	/**
	 * @brief Function to get the applied torque for the constraint.
	 */
	dsGetPhysicsConstraintForceFunction getTorqueFunc;

	/**
	 * @brief Function to destroy the constraint.
	 */
	dsDestroyPhysicsConstraintFunction destroyFunc;
} dsPhysicsConstraint;

/**
 * @brief Struct describing a physics constraint that has zero degrees of freedom.
 *
 * This effectively glues two actors together so they move as one rigid object.
 *
 * Transforms are relative to the local coordinate space of each actor. The transforms are
 * immutable, so changing the attachment location and orientation requires creating a new
 * constraint.
 *
 * @remark None of the members should be modified outside of the implementation.
 * @see PhysicsConstraint.h
 */
typedef struct dsFixedPhysicsConstraint
{
	/**
	 * @brief The base constraint type.
	 */
	dsPhysicsConstraint constraint;

	/**
	 * @brief The position of the constraint relative to the first actor.
	 */
	dsVector3f firstPosition;

	/**
	 * @brief The position of the constraint relative to the second actor.
	 */
	dsVector3f secondPosition;

	/**
	 * @brief The rotation of the constraint relative to the first actor.
	 */
	dsQuaternion4f firstRotation;

	/**
	 * @brief The rotation of the constraint relative to the second actor.
	 */
	dsQuaternion4f secondRotation;
} dsFixedPhysicsConstraint;

/**
 * @brief Function to set whether a physics constraint is enabled.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The physics constrant to set the enabled state on.
 * @param enabled Whether the constraint is enabled and will be enforced.
 * @return False if the enabled state couldn't be set.
 */
typedef bool (*dsSetPhysicsConstraintEnabledFunction)(dsPhysicsEngine* engine,
	dsPhysicsConstraint* constraint, bool enabled);

/**
 * @brief Function to create a fixed physics constraint.
 * @param engine The physics engine to create the constraint with.
 * @param allocator The allocator to create the constraint with.
 * @param enabled Whether the constraint is enabled after creation.
 * @param firstActor The first physics actor the constraint is attached to.
 * @param firstPosition The position of the constraint on the first actor.
 * @param firstRotation The rotation of the constraint on the first actor.
 * @param secondActor The second physics actor the constraint is attached to.
 * @param secondPosition The position of the constraint on the second actor.
 * @param secondRotation The rotation of the constraint on the second actor.
 * @return The fixed constraint or NULL if it couldn't be created.
 */
typedef dsFixedPhysicsConstraint* (*dsCreateFixedPhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsAllocator* allocator, bool enabled, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstRotation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondRotation);

/**
 * @brief Destroys a fixed physics constraint.
 * @param engine The physics engine the constraint was created with.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
typedef bool (*dsDestroyFixedPhysicsConstraintFunction)(dsPhysicsEngine* engine,
	dsFixedPhysicsConstraint* constraint);

#ifdef __cplusplus
}
#endif

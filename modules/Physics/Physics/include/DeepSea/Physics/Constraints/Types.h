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

#ifdef __cplusplus
extern "C"
{
#endif

/// @cond
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
	 * @brief Pointer to the constraint implementation.
	 *
	 * This is a convenience to avoid needing to check the type to get the underlying constraint for
	 * the physics implementation.
	 */
	void* impl;

	/**
	 * @brief Function to destroy the constraint.
	 */
	dsDestroyPhysicsConstraintFunction destroyFunc;
} dsPhysicsConstraint;

#ifdef __cplusplus
}
#endif

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
 * @brief Functions for manipulating physics constraints.
 * @see dsPhysicsShape
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
 * @param enabled Whether the constraint is enabled.
 * @param impl The underlying implementation of the constraint.
 * @param destroyFunc The destroy function of the constraint.
 * @return False if the parameters are invalid.
 */
DS_PHYSICS_EXPORT bool dsPhysicsConstraint_initialize(dsPhysicsConstraint* constraint,
	dsPhysicsEngine* engine, dsAllocator* allocator, dsPhysicsConstraintType type, bool enabled,
	void* impl, dsDestroyPhysicsConstraintFunction destroyFunc);

/**
 * @brief Destroys a physics constraint.
 * @remark errno will be set on failure.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsConstraint_destroy(dsPhysicsConstraint* constraint);

#ifdef __cplusplus
}
#endif

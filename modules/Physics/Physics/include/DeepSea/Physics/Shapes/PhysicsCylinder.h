/*
 * Copyright 2023 Aaron Barany
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
#include <DeepSea/Physics/Shapes/Types.h>
#include <DeepSea/Physics/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating physics cylinders.
 * @see dsPhysicsCylinder
 */

/**
 * @brief Gets the type of a physics cylinder.
 * @return The type of a cylinder.
 */
DS_PHYSICS_EXPORT const dsPhysicsShapeType* dsPhysicsCylinder_type(void);

/**
 * @brief Creates a physics cylinder.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the cylinder with.
 * @param allocator The allocator to create the cylinder with. If NULL the engine's allocator will be
 *     used.
 * @param halfHeight The half height of the cylinder. This must be > 0.
 * @param radius The radius of the cylinder. This must be > 0.
 * @param axis The axis to align the cylinder with. This must not be unaligned.
 * @param convexRadius The convex radius used for collision checks. Larger values will improve
 *     performance at the expense of precision by rounding the corners of the shape. This must be
 *     >= 0. Set to DS_DEFAULT_PHYSICS_SHAPE_CONVEX_RADIUS for typical shapes in meter space.
 * @return The cylinder or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsPhysicsCylinder* dsPhysicsCylinder_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, float halfHeight, float radius, dsPhysicsAxis axis,
	float convexRadius);

/**
 * @brief Destroys a physics cylinder.
 * @remark errno will be set on failure.
 * @param cylinder The cylinder to destroy.
 * @return False if the cylinder couldn't be destroyed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsCylinder_destroy(dsPhysicsCylinder* cylinder);

#ifdef __cplusplus
}
#endif

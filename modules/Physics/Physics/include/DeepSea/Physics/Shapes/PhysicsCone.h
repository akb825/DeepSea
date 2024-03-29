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
 * @brief Functions for creating and manipulating physics cones.
 * @see dsPhysicsCone
 */

/**
 * @brief Gets the type of a physics cone.
 * @return The type of a cone.
 */
DS_PHYSICS_EXPORT const dsPhysicsShapeType* dsPhysicsCone_type(void);

/**
 * @brief Creates a physics cone.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the cone with.
 * @param allocator The allocator to create the cone with. If NULL the engine's allocator will be
 *     used.
 * @param height The height of the cone. This must be > 0.
 * @param radius The radius of the cone. This must be > 0.
 * @param axis The axis to align the cone with.
 * @param convexRadius The convex radius used for collision checks. Larger values will improve
 *     performance at the expense of precision by rounding the corners of the shape. This must be
 *     >= 0. Set to DS_DEFAULT_PHYSICS_SHAPE_CONVEX_RADIUS for typical shapes in meter space.
 * @return The cone or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsPhysicsCone* dsPhysicsCone_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, float height, float radius, dsPhysicsAxis axis,
	float convexRadius);

/**
 * @brief Initializes a physics cone.
 *
 * This is called by the physics implementation to initialize the common members.
 *
 * @param[out] cone The cone to initialize.
 * @param engine The physics engine the cone was created with.
 * @param allocator The allocator the cone was created with.
 * @param impl The underlying implementation of the shape.
 * @param height The height of the cone.
 * @param radius The radius of the cone.
 * @param axis The axis to align the cone with.
 * @param convexRadius The convex radius used for collision checks.
 */
DS_PHYSICS_EXPORT void dsPhysicsCone_initialize(dsPhysicsCone* cone,
	dsPhysicsEngine* engine, dsAllocator* allocator, void* impl, float height, float radius,
	dsPhysicsAxis axis, float convexRadius);

#ifdef __cplusplus
}
#endif

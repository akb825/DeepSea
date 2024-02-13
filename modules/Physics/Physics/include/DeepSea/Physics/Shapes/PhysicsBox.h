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
 * @brief Functions for creating and manipulating physics boxes.
 * @see dsPhysicsBox
 */

/**
 * @brief Gets the type of a physics box.
 * @return The type of a box.
 */
DS_PHYSICS_EXPORT const dsPhysicsShapeType* dsPhysicsBox_type(void);

/**
 * @brief Creates a physics box.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the box with.
 * @param allocator The allocator to create the box with. If NULL the engine's allocator will be
 *     used.
 * @param halfExtents The half extents for each axis. Each value must be >= 0, and may be implicitly
 *     increased if < convexRadius.
 * @param convexRadius The convex radius used for collision checks. Larger values will improve
 *     performance at the expense of precision by rounding the corners of the shape. This must be
 *     >= 0. Set to DS_DEFAULT_PHYSICS_SHAPE_CONVEX_RADIUS for typical shapes in meter space.
 * @return The box or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsPhysicsBox* dsPhysicsBox_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsVector3f* halfExtents, float convexRadius);

/**
 * @brief Initializes a physics box
 *
 * This is called by the physics implementation to initialize the common members.
 *
 * @param[out] box The box to initialize.
 * @param engine The physics engine the box was created with.
 * @param allocator The allocator the box was created with.
 * @param impl The underlying implementation of the shape.
 * @param halfExtents The half extents for each axis.
 * @param convexRadius The convex radius used for collision checks.
 */
DS_PHYSICS_EXPORT void dsPhysicsBox_initialize(dsPhysicsBox* box, dsPhysicsEngine* engine,
	dsAllocator* allocator, void* impl, const dsVector3f* halfExtents, float convexRadius);

#ifdef __cplusplus
}
#endif

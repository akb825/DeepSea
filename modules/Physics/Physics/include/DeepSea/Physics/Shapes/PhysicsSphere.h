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
 * @brief Functions for creating and manipulating physics spheres.
 * @see dsPhysicsSphere
 */

/**
 * @brief Gets the type of a physics sphere.
 * @return The type of a sphere.
 */
DS_PHYSICS_EXPORT const dsPhysicsShapeType* dsPhysicsSphere_type(void);

/**
 * @brief Creates a physics sphere.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the sphere with.
 * @param allocator The allocator to create the sphere with. If NULL the engine's allocator will be
 *     used.
 * @param radius The radius of the sphere. This must be > 0.
 * @return The sphere or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsPhysicsSphere* dsPhysicsSphere_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, float radius);

/**
 * @brief Destroys a physics sphere.
 * @remark errno will be set on failure.
 * @param sphere The sphere to destroy.
 * @return False if the sphere couldn't be destroyed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsSphere_destroy(dsPhysicsSphere* sphere);

#ifdef __cplusplus
}
#endif

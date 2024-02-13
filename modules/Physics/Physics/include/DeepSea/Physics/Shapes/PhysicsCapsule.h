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
 * @brief Functions for creating and manipulating physics capsules.
 * @see dsPhysicsCapsule
 */

/**
 * @brief Gets the type of a physics capsule.
 * @return The type of a capsule.
 */
DS_PHYSICS_EXPORT const dsPhysicsShapeType* dsPhysicsCapsule_type(void);

/**
 * @brief Creates a physics capsule.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the capsule with.
 * @param allocator The allocator to create the capsule with. If NULL the engine's allocator will be
 *     used.
 * @param halfHeight The half height of the cylinder portion of the capsule. This must be > 0.
 * @param radius The radius of the capsule. This must be > 0.
 * @param axis The axis to align the capsule with.
 * @return The capsule or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsPhysicsCapsule* dsPhysicsCapsule_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, float halfHeight, float radius, dsPhysicsAxis axis);

/**
 * @brief Initializes a physics capsule.
 *
 * This is called by the physics implementation to initialize the common members.
 *
 * @param[out] capsule The capsule to initialize.
 * @param engine The physics engine the capsule was created with.
 * @param allocator The allocator the capsule was created with.
 * @param impl The underlying implementation of the shape.
 * @param halfHeight The half height of the cylinder portion of the capsule.
 * @param radius The radius of the capsule.
 * @param axis The axis to align the capsule with.
 */
DS_PHYSICS_EXPORT void dsPhysicsCapsule_initialize(dsPhysicsCapsule* capsule,
	dsPhysicsEngine* engine, dsAllocator* allocator, void* impl, float halfHeight, float radius,
	dsPhysicsAxis axis);

#ifdef __cplusplus
}
#endif

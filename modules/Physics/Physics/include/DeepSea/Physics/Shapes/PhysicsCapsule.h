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
 * @param axis The axis to align the capsule with. This must not be unaligned.
 * @return The capsule or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsPhysicsCapsule* dsPhysicsCapsule_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, float halfHeight, float radius, dsPhysicsAxis axis);

#ifdef __cplusplus
}
#endif

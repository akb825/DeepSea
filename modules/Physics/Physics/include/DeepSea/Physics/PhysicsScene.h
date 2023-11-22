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
#include <DeepSea/Physics/Export.h>
#include <DeepSea/Physics/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manipulate physics scenes.
 * @see dsPhysicsScene
 */

/**
 * @brief Creates a physics scene.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the physics scene with.
 * @param allocator The allocator to create the physics scene with. If NULL, it will use the same
 *     allocator as the physics engine.
 * @param limits The limits for the physics scene.
 * @param threadPool The thread pool to use for multithreaded processing, or NULL for
 *     single-threaded processing.
 * @return The physics scene or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsPhysicsScene* dsPhysicsScene_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsSceneLimits* limits, dsThreadPool* threadPool);

/**
 * @brief Destroys a physics scene.
 * @remark errno will be set on failure.
 * @param scene The physics scene to destroy.
 * @return False if the scene couldn't be destroyed.
 */
DS_PHYSICS_EXPORT bool dsPhysicsScene_destroy(dsPhysicsScene* scene);

#ifdef __cplusplus
}
#endif

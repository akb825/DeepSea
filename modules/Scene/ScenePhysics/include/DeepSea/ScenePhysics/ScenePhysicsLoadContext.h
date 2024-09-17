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
#include <DeepSea/Physics/Types.h>
#include <DeepSea/Scene/Types.h>
#include <DeepSea/ScenePhysics/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function for registering the ScenePhysics types with a dsSceneLoadContext.
 */

/**
 * @brief Registers the scene physics types for loading.
 * @remark errno will be set on failure.
 * @param loadContext The load context to register the types with.
 * @param allocator The allocator to use for extra data used for registration.
 * @param physicsEngine The physics engine to create physics resources with.
 * @param takeOwnership Whether to take ownership of the physics engine. If true and creation fails,
 *     the physics engine will be destroyed immediately.
 * @param threadPool The thread pool to use when processing physics scenes, or NULL to process
 *     single-threaded.
 * @return False if not all of the types could be registered.
 */
DS_SCENEPHYSICS_EXPORT bool dsScenePhysicsLoadConext_registerTypes(
	dsSceneLoadContext* loadContext, dsAllocator* allocator, dsPhysicsEngine* physicsEngine,
	bool takeOwnership, dsThreadPool* threadPool);

#ifdef __cplusplus
}
#endif

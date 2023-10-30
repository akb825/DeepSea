/*
 * Copyright 2019-2023 Aaron Barany
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
#include <DeepSea/Core/Thread/Types.h>
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene thread managers.
 * @see dsSceneThreadManager
 */

/**
 * @brief Creates a scene thread manager.
 *
 * The item lists declared within a dsScene will be executed in parallel within the thread pool.
 *
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the thread manager with. This must support freeing
 *     memory.
 * @param renderer The renderer.
 * @param threadPool The thread pool to execute commands on. This must be set up to acquire resource
 *     contexts on each thread, such as by using dsResourceManager_createThreadPool().
 * @return The scene thread manager or NULL if an error occurred. This must be destroyed before the
 *     resource manager.
 */
DS_SCENE_EXPORT dsSceneThreadManager* dsSceneThreadManager_create(dsAllocator* allocator,
	dsRenderer* renderer, dsThreadPool* threadPool);

/**
 * @brief Destroys a thread manager.
 * @remark errno will be set on failure.
 * @param threadManager The thread manager to destroy.
 * @return False if an error occurred.
 */
DS_SCENE_EXPORT bool dsSceneThreadManager_destroy(dsSceneThreadManager* threadManager);

#ifdef __cplusplus
}
#endif

/*
 * Copyright 2016 Aaron Barany
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
#include <DeepSea/Render/Resources/Types.h>
#include <DeepSea/Render/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for interacting with a resource manager.
 *
 * All manipulation of graphics resources requires a resource context to be created. There will
 * always be a resource context available on the main thread, while other threads require a resource
 * context to be created. Up to maxResourceContexts contexts may be created, which may be 0 for
 * platforms that don't allow multiple threads to access graphics resources.
 *
 * These functions are for using the resource manager in general. Functions for creating and
 * manipulating specific resource types are found in the .h files for that resource type.
 *
 * @see dsResourceManager
 */

/**
 * @brief Create a resource context for the current thread.
 *
 * This will allow resources to be created and manipulated from the current thread. It will remain
 * valid until dsResourceManager_destroyResourceContext() is called.
 *
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager
 * @return False if the resource context couldn't be created.
 */
DS_RENDER_EXPORT bool dsResourceManager_createResourceContext(dsResourceManager* resourceManager);

/**
 * @brief Flushes the resource context for the current thread.
 *
 * Use this to guarantee that any resource changes are available for rendering.
 *
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager
 * @return False if the resource context couldn't be flushed.
 */
DS_RENDER_EXPORT bool dsResourceManager_flushResourceContext(dsResourceManager* resourceManager);

/**
 * @brief Destroys the resource context for the current thread.
 *
 * Any remaining tasks are flushed.
 *
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager
 * @return False if the resource context couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsResourceManager_destroyResourceContext(dsResourceManager* resourceManager);

/**
 * @brief Checks whether or not resources can be used on this thread.
 * @param resourceManager The resource manager.
 * @return True if resources may be used.
 */
DS_RENDER_EXPORT bool dsResourceManager_canUseResources(const dsResourceManager* resourceManager);

/**
 * @brief Reports statistics about the allocated resources to the profiler.
 *
 * This will automatically be called at the end of each frame.
 *
 * @param resourceManager The resource manager.
 */
DS_RENDER_EXPORT void dsResourceManager_reportStatistics(const dsResourceManager* resourceManager);

/**
 * @brief Initializes the members of a resource manager.
 *
 * This will initiialize all members to 0 and set up any internal structures. This is called by the
 * render implementation.
 *
 * @remark errno will be set on failure.
 * @return False if an error occurred.
 */
DS_RENDER_EXPORT bool dsResourceManager_initialize(dsResourceManager* resourceManager);

/**
 * @brief Destroys the private members of a resource manager.
 *
 * This is called by the render implementation.
 */
DS_RENDER_EXPORT void dsResourceManager_shutdown(dsResourceManager* resourceManager);

#ifdef __cplusplus
}
#endif

/*
 * Copyright 2016-2026 Aaron Barany
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
 * @brief Creates a thread pool that can be used with graphics resources.
 *
 * This provides start/end functions for the thread pool that acquires and releases resource
 * contexts. If other operations must be performed on thread start and stop, a standard dsThreadPool
 * may be created with custom thread start and end functions that call
 * dsResourceManager_acquireResourceContext() on start and
 * dsResourceManager_releaseResourceContext() on end.
 *
 * In general the number of threads shouldn't be changed after creation. If it is changed, the
 * number of threads must not exceed the number of resource contexts. If acquiring a resource
 * context fails on a thread, the application will be aborted.
 *
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the thread pool with.
 * @param resourceManager The resource manager to acquire resource contexts from.
 * @param threadCount The initial number of threads. This may not exceed the maximum number of
 *     resource contexts.
 * @param stackSize The size of the stack of each thread in bytes. Set to 0 for the system default.
 * @return The thread pool or NULL if an error occurred.
 */
DS_RENDER_EXPORT dsThreadPool* dsResourceManager_createThreadPool(dsAllocator* allocator,
	dsResourceManager* resourceManager, unsigned int threadCount, size_t stackSize);

/**
 * @brief Acquires a resource context for the current thread.
 *
 * This will allow resources to be created and manipulated from the current thread. It will remain
 * valid until dsResourceManager_releaseResourceContext() is called.
 *
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager.
 * @return False if there are no remaining resource contexts or a resource context is already
 *     available for this thread.
 */
DS_RENDER_EXPORT bool dsResourceManager_acquireResourceContext(dsResourceManager* resourceManager);

/**
 * @brief Gets a command buffer for use with resource-related operations, typically copies.
 *
 * This may be queried after dsResourceManager_acquireResourceContext() has returned true or on the
 * main thread. It should not be submitted directly, and instead should be submitted either through
 * dsResourceManager_flushResourceContext() or dsResourceManager_releaseResourceContext(), in which
 * case the commands will be submitted at the start of the next frame.
 *
 * This will lazily acquire a command buffer when called, so it should only be called when a command
 * buffer is truly required to avoid the overhead of creating unneeded command buffers. Long-lived
 * pointers should not be kept, as dsResourceManager_flushResourceContext() may change the command
 * buffer.
 *
 * The main thread will also return a command buffer separate from the main command buffer of the
 * renderer, which allows for resource operations to be performed outside of a frame. This will be
 * implicitly flushed at the start of the next frame.
 *
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager.
 * @return The command buffer for any resource-related operations, or NULL if there is no active
 *     resource context.
 */
DS_RENDER_EXPORT dsCommandBuffer* dsResourceManager_getResourceCommandBuffer(
	dsResourceManager* resourceManager);

/**
 * @brief Flushes the resource context for the current thread.
 *
 * Use this to guarantee that any resource changes are available for rendering.
 *
 * @remark errno will be set on failure.
 * @remark This will invalidate any command buffer pointer acquired from
 *     dsResourceManager_getResourceCommandBuffer(). See the documentation for that function for the
 *     expected usage.
 * @param resourceManager The resource manager.
 * @return False if the resource context couldn't be flushed.
 */
DS_RENDER_EXPORT bool dsResourceManager_flushResourceContext(dsResourceManager* resourceManager);

/**
 * @brief Releases the resource context for the current thread.
 *
 * Any remaining tasks are flushed. Once released, the resource context will be free to be acquired
 * from another thread.
 *
 * @remark errno will be set on failure.
 * @param resourceManager The resource manager.
 * @return False if the resource context couldn't be destroyed.
 */
DS_RENDER_EXPORT bool dsResourceManager_releaseResourceContext(dsResourceManager* resourceManager);

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

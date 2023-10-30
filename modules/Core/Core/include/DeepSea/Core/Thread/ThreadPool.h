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
#include <DeepSea/Core/Thread/Types.h>
#include <DeepSea/Core/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manipulate a thread pool.
 * @remark All functions are thread safe unless otherwise specified.
 */

/**
 * @brief Constant for the maximum number of threads supported by the thread pool.
 *
 * This is set to be much higher than the expected amount of parallelism on a single machine while
 * being low enough to make certain assumptions for internal operations.
 */
#define DS_THREAD_POOL_MAX_THREADS 1023

/**
 * @brief Gets the full maximal thread count for a thread pool.
 *
 * This will equal the number of logical cores minus one, minimum of one, assuming that the main
 * thread will also be utilized. This is ideal if you assume nothing else will run on the system.
 * However, if other background processes run on the machine, including threads not controled by
 * the thread pool such as audio threads, delays may occur as unscheduled threads may hold onto
 * locks.
 *
 * @return The full thread count.
 */
DS_CORE_EXPORT unsigned int dsThreadPool_fullThreadCount(void);

/**
 * @brief Gets the default thread count for a thread pool.
 *
 * This will use 3/4 of the logical cores, minimum of one. This allows extra cores to remain idle to
 * account for other background processes to run on the system, including threads not controled by
 * the thread pool such as audio threads. This can avoid potential delays caused by unscheduled
 * threads holding onto locks.
 *
 * @return The default thread count.
 */
DS_CORE_EXPORT unsigned int dsThreadPool_defaultThreadCount(void);

/**
 * @brief Creates a thread pool.
 * @remark errno will be set on failure.
 * @param allocator The allocator for the thread pool. This must support freeing memory.
 * @param threadCount The number of threads to use. This should typically one fewer thread than your
 *     desired total concurrency to account for work performed on the main thread. A value of 0 is
 *     valid for task queues to perform their work serially in dsThreadTaskQueue_waitForTasks().
 *     Pass dsThreadPool_defaultThreadCount() for a default value based on the number of logical
 *     cores.
 * @param stackSize The size of the stack of each thread in bytes. Set to 0 for the system default.
 * @param startThreadFunc Function to call when a worker thread starts to set up any per-thread
 *     resources. May be NULL if no setup is needed.
 * @param endThreadFunc Function to call when a worker thread ends to tear down any per-thread
 *     resources. May be NULL if no tear down is needed.
 * @param startEndThreadUserData User data to provide to startThreadFunc and endThreadFunc.
 * @return The thread pool or NULL if it couldn't be created.
 */
DS_CORE_EXPORT dsThreadPool* dsThreadPool_create(dsAllocator* allocator, unsigned int threadCount,
	size_t stackSize, dsThreadTaskFunction startThreadFunc, dsThreadTaskFunction endThreadFunc,
	void* startEndThreadUserData);

/**
 * @brief Gets the number of threads for a thread pool.
 * @param threadPool The thread pool.
 * @return The number of threads currently running.
 */
DS_CORE_EXPORT unsigned int dsThreadPool_getThreadCount(const dsThreadPool* threadPool);

/**
 * @brief Gets the number of threads for a thread pool.
 *
 * This is unlocked, which avoids locking overhead and potential delays while threads are searching
 * for tasks to execute. However, it is only safe to use if you are very certain that
 * dsThreadPool_setThreadCount() can't be called concurrently. For example, this should be safe to
 * use when calling from the main application thread.
 *
 * @param threadPool The thread pool.
 * @return The number of threads currently running.
 */
DS_CORE_EXPORT unsigned int dsThreadPool_getThreadCountUnlocked(const dsThreadPool* threadPool);

/**
 * @brief Sets the number of threads for the thread pool.
 *
 * This may block until threads that are busy have finished executing.
 *
 * @remark This must not be called on a task thread, otherwise a deadlock may occur.
 *
 * @remark errno will be set on failure.
 * @param threadPool The thread pool.
 * @param threadCount The number of threads to use. This should typically one fewer thread than your
 *     desired total concurrency to account for work performed on the main thread. A value of 0 is
 *     valid for task queues to perform their work serially in dsThreadTaskQueue_waitForTasks().
 *     Pass dsThreadPool_defaultThreadCount() for a default value based on the number of logical
 *     cores.
 * @return False if an error occurred. On failure the number of threads may not be the same as
 *     before calling this function, call dsThreadPool_getThreadCount() to get the current state.
 */
DS_CORE_EXPORT bool dsThreadPool_setThreadCount(dsThreadPool* threadPool, unsigned int threadCount);

/**
 * @brief Destroys a thread pool.
 *
 * All task queues created with the thread pool must be destroyed before the thread pool.
 *
 * @remark errno will be set on failure.
 * @param threadPool The thread pool to destroy.
 * @return False if the thread pool couldn't be destroyed.
 */
DS_CORE_EXPORT bool dsThreadPool_destroy(dsThreadPool* threadPool);

#ifdef __cplusplus
}
#endif

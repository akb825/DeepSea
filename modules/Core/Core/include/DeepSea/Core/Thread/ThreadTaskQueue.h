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
 * @brief Functions to create and manipulate a thread task queue.
 */

/**
 * @brief Gets the size of dsThreadTaskQueue.
 * @return The size of dsThreadTaskQueue.
 */
DS_CORE_EXPORT size_t dsThreadTaskQueue_sizeof(void);

/**
 * @brief Gets the full allocation size of dsThreadTaskQueue.
 * @param maxTasks The maximum number of tasks that can be queued at once.
 * @return The full allocation size of dsThreadTaskQueue.
 */
DS_CORE_EXPORT size_t dsThreadTaskQueue_fullAllocSize(uint32_t maxTasks);

/**
 * @brief Creates a thread task queue.
 * @remark errno will be set on failure.
 * @param allocator The allocator for the task queue.
 * @param threadPool The thread pool to process tasks. This must be alive at least as long as the
 *     task queue.
 * @param maxTasks The maximum number of tasks that can be queued at once. This isn't a hard limit,
 *     but may cause dsThreadTaskQueue_addTasks() to block if it is exceeded.
 * @param maxConcurrency The maximum number of tasks to run in parallel. If 0 all threads on the
 *     thread pool may run tasks from this queue.
 * @return The task queue or NULL if it couldn't be created.
 */
DS_CORE_EXPORT dsThreadTaskQueue* dsThreadTaskQueue_create(dsAllocator* allocator,
	dsThreadPool* threadPool, uint32_t maxTasks, uint32_t maxConcurrency);

/**
 * @brief Gets the maximum parallelism of a task queue.
 * @param taskQueue The task queue.
 * @return The maximum number of tasks that can be run in parallel or 0 if there is no limit.
 */
DS_CORE_EXPORT uint32_t dsThreadTaskQueue_getMaxConcurrency(const dsThreadTaskQueue* taskQueue);

/**
 * @brief Sets the maximum parallelism of a task queue.
 * @remark errno will be set on failure.
 * @param taskQueue The task queue.
 * @param maxConcurrency The maximum number of tasks to run in parallel. If 0 all threads on the
 *     thread pool may run tasks from this queue.
 * @return False if an error occurred.
 */
DS_CORE_EXPORT bool dsThreadTaskQueue_setMaxConcurrency(dsThreadTaskQueue* taskQueue,
	uint32_t maxConcurrency);

/**
 * @brief Adds tasks to the task queue.
 *
 * It's best to queue batches of tasks to reduce overhead for locking. If there isn't enough room
 * for all tasks, this will process tasks on the current thread as needed.
 *
 * This function is safe to call across threads and within tasks. Tasks within a single call will
 * be queued together unless the max task count is exceeded, in which case it's possible for other
 * concurrent calls to add tasks when waiting for space to be available.
 *
 * @remark errno will be set on failure.
 * @param taskQueue The task queue to add tasks to.
 * @param tasks The tasks to add.
 * @param taskCount The number of tasks to add.
 * @return False if an error occurred.
 */
DS_CORE_EXPORT bool dsThreadTaskQueue_addTasks(dsThreadTaskQueue* taskQueue,
	const dsThreadTask* tasks, uint32_t taskCount);

/**
 * @brief Waits for all tasks on the queue to be completed.
 *
 * This will also process tasks on the current thread while waiting. This assumes that the
 * individual tasks themselves are short-running.
 *
 * @remark This must not be called within a task on the current task queue, otherwise a deadlock
 *     will occur.
 * @remark errno will be set on failure.
 * @param taskQueue The task queue to wait on.
 * @return False if an error occurred.
 */
DS_CORE_EXPORT bool dsThreadTaskQueue_waitForTasks(dsThreadTaskQueue* taskQueue);

/**
 * @brief Destroys a thread task queue.
 *
 * This will implicitly wait for any remaining tasks to finish executing.
 *
 * @param taskQueue The task queue to destroy.
 */
DS_CORE_EXPORT void dsThreadTaskQueue_destroy(dsThreadTaskQueue* taskQueue);

#ifdef __cplusplus
}
#endif

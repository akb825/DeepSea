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
#include <DeepSea/Core/Export.h>
#include <DeepSea/Core/Thread/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manage threads.
 * @see dsThread
 */

/**
 * @brief Gets the number of logical cores on the current device.
 * @return The number of cores, or 0 if it couldn't be determined.
 */
DS_CORE_EXPORT unsigned int dsThread_logicalCoreCount(void);

/**
 * @brief Creates a thread.
 * @remark errno will be set on failure.
 * @param[out] thread The thread to create.
 * @param function The function to call.
 * @param userData The user data to pass to the function.
 * @param stackSize The size of the thread's stack. Set to 0 for the default size.
 * @param name The name of the thread, used for profiling. The lifetime of the string should exceed
 *     the lifetime of the thread, such as with a string constant. If NULL, will be set to "Thread".
 * @return True if the thread was created.
 */
DS_CORE_EXPORT bool dsThread_create(dsThread* thread, dsThreadFunction function, void* userData,
	unsigned int stackSize, const char* name);

/**
 * @brief Sets the name of this thread.
 * @remark This is automatically called for threads created with dsThread.
 * @param name The name of the thread.
 * @return True if the name was set.
 */
DS_CORE_EXPORT bool dsThread_setThisThreadName(const char* name);

/**
 * @brief Exits the current thread.
 * @param returnVal The return value of the thread, returned from dsThread_join().
 */
DS_CORE_EXPORT void dsThread_exit(dsThreadReturnType returnVal);

/**
 * @brief Gets the ID for a thread.
 * @param thread The thread to get the ID for.
 * @return The thread ID. This will be 0 if the thread is invalid.
 */
DS_CORE_EXPORT dsThreadID dsThread_getID(dsThread thread);

/**
 * @brief Gets the ID of this thread.
 * @return The ID of this thread.
 */
DS_CORE_EXPORT dsThreadID dsThread_thisThreadID(void);

/**
 * @brief Returns an invalid thread ID.
 * @return An invalid ID.
 */
DS_CORE_EXPORT dsThreadID dsThread_invalidID(void);

/**
 * @brief Checks whether or not two threads are equal.
 * @param thread1 The first thread.
 * @param thread2 The second thread.
 * @return True if thread1 is equal to thread2.
 */
DS_CORE_EXPORT bool dsThread_equal(dsThreadID thread1, dsThreadID thread2);

/**
 * @brief Yields this thread to allow other threads to continue.
 */
DS_CORE_EXPORT void dsThread_yield(void);

/**
 * @brief Sleeps the current threads.
 *
 * This will automatically profile the time spent sleeping.
 *
 * @param milliseconds The number of milliseconds to sleep for.
 * @param name The name used for profiling the sleep. If NULL, will be set to "Sleep".
 */
DS_CORE_EXPORT void dsThread_sleep(unsigned int milliseconds, const char* name);

/**
 * @brief Detaches a thread.
 *
 * Once a thread is detached, it will continue executing in the background.
 *
 * @remark errno will be set on failure.
 * @param[inout] thread The thread to detach. The content will be cleared.
 * @return True if the thread was detached.
 */
DS_CORE_EXPORT bool dsThread_detach(dsThread* thread);

/**
 * @brief Joins a thread, waiting for it to complete.
 *
 * This will automatically profile the time spent waiting to join with the thread.
 *
 * @remark errno will be set on failure.
 * @param[inout] thread The thread to detach. The content will be cleared.
 * @param[out] returnVal Pointer to recieve the return value of the thread. This may be NULL.
 * @return True if the thread was joined.
 */
DS_CORE_EXPORT bool dsThread_join(dsThread* thread, dsThreadReturnType* returnVal);

#ifdef __cplusplus
}
#endif

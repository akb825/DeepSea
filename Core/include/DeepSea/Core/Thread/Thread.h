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
#include <stdbool.h>
#include <stdint.h>

#if !DS_WINDOWS
#include <sys/types.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manage threads.
 */

/**
 * @brief Type of a thread ID.
 */
typedef struct dsThreadId
{
	/** Internal */
#if DS_WINDOWS
	uint32_t threadId;
#else
	pthread_t threadId;
#endif
} dsThreadId;

/**
 * @brief Structure that holds the reference for a thread.
 */
typedef struct dsThread
{
	/** Internal */
#if DS_WINDOWS
	void* thread;
#else
	pthread_t thread;
#endif
} dsThread;

/**
 * @brief Type of the thread return type.
 *
 * While this will be larger on some platforms, it is only guaranteed to be 32-bits.
 */
#if DS_WINDOWS
typedef int32_t dsThreadReturnType;
#else
typedef intptr_t dsThreadReturnType;
#endif

/**
 * @brief Function that is called at the beginning of a thread.
 * @param userData User data for the thread.
 * @return The exit code for the thread.
 */
typedef dsThreadReturnType (*dsThreadFunction)(void* userData);

/**
 * @brief Creates a thread.
 * @param[out] thread The thread to create.
 * @param function The function to call.
 * @param userData The user data to pass to the function.
 * @param stackSize The size of the thread's stack. Set to 0 for the default size.
 * @return True if the thread was created.
 */
DS_CORE_EXPORT bool dsThread_create(dsThread* thread, dsThreadFunction function, void* userData,
	unsigned int stackSize);

/**
 * @brief Sets the name of this thread.
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
DS_CORE_EXPORT dsThreadId dsThread_getId(dsThread thread);

/**
 * @brief Gets the ID of this thread.
 * @return The ID of this thread.
 */
DS_CORE_EXPORT dsThreadId dsThread_thisThreadId();

/**
 * @brief Returns an invalid thread ID.
 * @return An invalid ID.
 */
DS_CORE_EXPORT dsThreadId dsThread_invalidId();

/**
 * @brief Checks whether or not two threads are equal.
 * @param thread1 The first thread.
 * @param thread2 The second thread.
 * @return True if thread1 is equal to thread2.
 */
DS_CORE_EXPORT bool dsThread_equal(dsThreadId thread1, dsThreadId thread2);

/**
 * @brief Sleeps the current threads.
 * @param milliseconds The number of milliseconds to sleep for.
 */
DS_CORE_EXPORT void dsThread_sleep(unsigned int milliseconds);

/**
 * @brief Detaches a thread.
 *
 * Once a thread is detached, it will continue executing in the background.
 * @param[inout] thread The thread to detach. The content will be cleared.
 * @return True if the thread was detached.
 */
DS_CORE_EXPORT bool dsThread_detach(dsThread* thread);

/**
 * @brief Joins a thread, waiting for it to complete.
 * @param[inout] thread The thread to detach. The content will be cleared.
 * @param[out] returnVal Pointer to recieve the return value of the thread. This may be NULL.
 * @return True if the thread was joined.
 */
DS_CORE_EXPORT bool dsThread_join(dsThread* thread, dsThreadReturnType* returnVal);

#ifdef __cplusplus
}
#endif

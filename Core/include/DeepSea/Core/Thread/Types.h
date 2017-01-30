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
#include <stdbool.h>
#include <stddef.h>
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
 * @brief Includes all of the types used by the Thread portion of the DeepSea/Coroe library.
 */

/// \{
typedef struct dsAllocator dsAllocator;
/// \}

/**
 * @brief Type for a thread condition variable.
 * @see ConditionVariable.h
 */
typedef struct dsConditionVariable dsConditionVariable;

/**
 * @brief Result of a condition variable wait.
 * @see ConditionVariable.h
 */
typedef enum dsConditionVariableResult
{
	dsConditionVariableResult_Success, ///< The conditon variable was notified.
	dsConditionVariableResult_Error,   ///< There was an error when waiting.
	dsConditionVariableResult_Timeout  ///< The wait timed out.
} dsConditionVariableResult;

/**
 * @brief Type for a mutex lock.
 * @see Mutex.h
 */
typedef struct dsMutex dsMutex;

/**
 * @brief Structure that holds the state of a spinlock.
 * @see Spinlock.h
 */
typedef struct dsSpinlock
{
	/** Internal */
#if DS_WINDOWS
	uint32_t counter;
#else
	pthread_spinlock_t spinlock;
#endif
} dsSpinlock;

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
 * @brief Type of the thread return type.
 *
 * While this will be larger on some platforms, it is only guaranteed to be 32-bits.
 *
 * @remark Since this is not guaranteed to store more than 32-bits, it is not safe to use to store
 * a pointer.
 *
 * @see Thread.h
 */
#if DS_WINDOWS
typedef int32_t dsThreadReturnType;
#else
typedef intptr_t dsThreadReturnType;
#endif

/**
 * @brief Function that is called at the beginning of a thread.
 * @see Thread.h
 * @param userData User data for the thread.
 * @return The exit code for the thread.
 */
typedef dsThreadReturnType (*dsThreadFunction)(void* userData);

/**
 * @brief Structure that holds the reference for a thread.
 * @see Thread.h
 */
typedef struct dsThread
{
	/**
	 * @brief The internal thread handle.
	 */
#if DS_WINDOWS
	void* thread;
#else
	pthread_t thread;
#endif

	/**
	 * @brief The name of the thread.
	 */
	const char* name;

	/**
	 * @brief The user data passed to the function.
	 */
	void* userData;

	/**
	 * @brief The function called for the thread.
	 */
	dsThreadFunction function;

	/**
	 * @brief Whether or not the thread has started.
	 */
	int32_t started;
} dsThread;

/**
 * @brief Structure that holds thread-local storage.
 * @see ThreadStorage.h
 */
typedef struct dsThreadStorage
{
	/** Internal */
#if DS_WINDOWS
	uint32_t storage;
#else
	pthread_key_t storage;
#endif
} dsThreadStorage;

#ifdef __cplusplus
}
#endif

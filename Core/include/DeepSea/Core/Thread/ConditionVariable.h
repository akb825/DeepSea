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
#include <DeepSea/Core/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manage condition variables.
 */

/**
 * @brief Gets the size of dsConditionVariable.
 * @return The size of dsConditionVariable.
 */
DS_CORE_EXPORT unsigned int dsConditionVariable_sizeof();

/**
 * @brief Gets the full allocated size of dsConditionVariable.
 * @return The full allocated size of dsConditionVariable.
 */
DS_CORE_EXPORT unsigned int dsConditionVariable_fullAllocSize();

/**
 * @brief Creates a condition variable.
 * @param allocator The allocator to use. If NULL, malloc() and free() will be used.
 * @return The condition variable, or NULL if it couldn't be created.
 */
DS_CORE_EXPORT dsConditionVariable* dsConditionVariable_create(dsAllocator* allocator);

/**
 * @brief Waits for a condition variable to be notified with dsConditionVariable_notifiedOne() or
 * dsConditionVariable_notifyAll()
 * @param condition The condition variable to wait on.
 * @param mutex A mutex that is locked. It will be unlocked while the condition variable waits, then
 * locked again before this function returns.
 * @return The result of waiting.
 */
DS_CORE_EXPORT dsConditionVariableResult dsConditionVariable_wait(dsConditionVariable* condition,
	dsMutex* mutex);

/**
 * @brief Waits for a condition variable to be notified with dsConditionVariable_notifiedOne(),
 * dsConditionVariable_notifyAll(), or times out.
 * @param condition The condition variable to wait on.
 * @param mutex A mutex that is locked. It will be unlocked while the condition variable waits, then
 * locked again before this function returns.
 * @param milliseconds The number of milliseconds to wait for.
 * @return The result of waiting.
 */
DS_CORE_EXPORT dsConditionVariableResult dsConditionVariable_timedWait(
	dsConditionVariable* condition, dsMutex* mutex, unsigned int milliseconds);

/**
 * @brief Notifies a single thread waiting on the condition variable to continue.
 * @param condition The condition variable to notify.
 * @return False if an error occured.
 */
DS_CORE_EXPORT bool dsConditionVariable_notifyOne(dsConditionVariable* condition);

/**
 * @brief Notifies all threads waiting on the condition variable to continue.
 * @param condition The condition variable to notify.
 * @return False if an error occured.
 */
DS_CORE_EXPORT bool dsConditionVariable_notifyAll(dsConditionVariable* condition);

/**
 * @brief Destroys a condition variable.
 * @param condition The condition variable to destroy.
 */
DS_CORE_EXPORT void dsConditionVariable_destroy(dsConditionVariable* condition);

#ifdef __cplusplus
}
#endif

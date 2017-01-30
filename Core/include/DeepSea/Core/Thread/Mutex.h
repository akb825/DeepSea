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
 * @brief Functions to create and manage mutexes.
 *
 * Mutex functions are automatically profiled, profiling the time that is spent waiting to lock
 * and when the mutex is locked.
 *
 * @see dsMutex
 */

/**
 * @brief Gets the size of dsMutex.
 * @return The size of dsMutex.
 */
DS_CORE_EXPORT unsigned int dsMutex_sizeof(void);

/**
 * @brief Gets the full allocated size of dsMutex.
 * @return The full allocated size of dsMutex.
 */
DS_CORE_EXPORT unsigned int dsMutex_fullAllocSize(void);

/**
 * @brief Creates a mutex.
 * @param allocator The allocator to use. If NULL, malloc() and free() will be used.
 * @param name The name of the mutex, used for profiling. The lifetime of the string should exceed
 * the lifetime of the mutex, such as with a string constant. If NULL, will be set to "Mutex".
 * @return The mutex, or NULL if it couldn't be created.
 */
DS_CORE_EXPORT dsMutex* dsMutex_create(dsAllocator* allocator, const char* name);

/**
 * @brief Locks the mutex if it isn't already locked.
 * @param mutex The mutex to lock.
 * @return True if the mutex could be locked.
 */
DS_CORE_EXPORT bool dsMutex_tryLock(dsMutex* mutex);

/**
 * @brief Locks the mutex.
 * @param mutex The mutex to lock.
 * @return True if the mutex could be locked.
 */
DS_CORE_EXPORT bool dsMutex_lock(dsMutex* mutex);

/**
 * @brief Unlocks the mutex.
 * @param mutex The mutex to unlock.
 * @return True if the mutex could be unlocked.
 */
DS_CORE_EXPORT bool dsMutex_unlock(dsMutex* mutex);

/**
 * @brief Destroys a mutex.
 * @param mutex The mutex to destroy.
 */
DS_CORE_EXPORT void dsMutex_destroy(dsMutex* mutex);

#ifdef __cplusplus
}
#endif

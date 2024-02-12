/*
 * Copyright 2024 Aaron Barany
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
 * @brief Functions to create and manage read/write locks.
 *
 * Read/write lock functions are automatically profiled, profiling the time that is spent waiting to
 * lock and when the read/write lock is locked.
 */

/**
 * @brief Gets the size of dsReadWriteLock.
 * @return The size of dsReadWriteLock.
 */
DS_CORE_EXPORT size_t dsReadWriteLock_sizeof(void);

/**
 * @brief Gets the full allocated size of dsMutex.
 * @return The full allocated size of dsMutex.
 */
DS_CORE_EXPORT size_t dsReadWriteLock_fullAllocSize(void);

/**
 * @brief Creates a read/write lock.
 * @remark errno will be set on failure.
 * @param allocator The allocator to use.
 * @param readName The name used to profile for read locks. The lifetime of the string should exceed
 *     the lifetime of the lock, such as with a string constant. If NULL, will be set to
 *     "Read Lock".
 * @param writeName The name used to profile for write locks. The lifetime of the string should
 *     exceed the lifetime of the lock, such as with a string constant. If NULL, will be set to
 *     "Write Lock".
 * @return The read/write lock or NULL if it couldn't be created.
 */
DS_CORE_EXPORT dsReadWriteLock* dsReadWriteLock_create(
	dsAllocator* allocator, const char* readName, const char* writeName);

/**
 * @brief Locks a read/write lock for reading if not already locked for writing.
 *
 * Multiple read locks may be acquired concurrently.
 *
 * @remark errno will be set on failure.
 * @param lock The lock to acquire the read lock for.
 * @return True if the read lock was acquired.
 */
DS_CORE_EXPORT bool dsReadWriteLock_tryLockRead(dsReadWriteLock* lock);

/**
 * @brief Locks a read/write lock for reading.
 *
 * Multiple read locks may be acquired concurrently, though it will block with write locks.
 *
 * @remark errno will be set on failure.
 * @param lock The lock to acquire the read lock for.
 * @return False if the lock is NULL.
 */
DS_CORE_EXPORT bool dsReadWriteLock_lockRead(dsReadWriteLock* lock);

/**
 * @brief Unlocks a read/write lock for reading.
 * @remark errno will be set on failure.
 * @param lock The lock to release the read lock for.
 * @return False if the lock is NULL or it wasn't previusly locked for reading.
 */
DS_CORE_EXPORT bool dsReadWriteLock_unlockRead(dsReadWriteLock* lock);

/**
 * @brief Locks a read/write lock for writing if not already locked for reading or writing.
 * @remark errno will be set on failure.
 * @param lock The lock to acquire the write lock for.
 * @return False if the lock is NULL.
 */
DS_CORE_EXPORT bool dsReadWriteLock_tryLockWrite(dsReadWriteLock* lock);

/**
 * @brief Locks a read/write lock for writing.
 *
 * This will block if any read or write locks are currently active.
 *
 * @remark errno will be set on failure.
 * @param lock The lock to acquire the write lock for.
 * @return False if the lock is NULL.
 */
DS_CORE_EXPORT bool dsReadWriteLock_lockWrite(dsReadWriteLock* lock);

/**
 * @brief Unlocks a read/write lock for writing.
 * @remark errno will be set on failure.
 * @param lock The lock to release the write lock for.
 * @return False if the lock is NULL or it wasn't previusly locked for writing.
 */
DS_CORE_EXPORT bool dsReadWriteLock_unlockWrite(dsReadWriteLock* lock);

/**
 * @brief Destroys a read/write lock.
 * @param lock The lock to destroy.
 */
DS_CORE_EXPORT void dsReadWriteLock_destroy(dsReadWriteLock* lock);

#ifdef __cplusplus
}
#endif

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
 * @brief Functions to create and manage read/write spinlocks.
 */

/**
 * @brief Initializes a read/write spinlock.
 * @remark errno will be set on failure.
 * @param[out] lock The lock to initialize.
 * @return False if the lock is NULL.
 */
DS_CORE_EXPORT bool dsReadWriteSpinlock_initialize(dsReadWriteSpinlock* lock);

/**
 * @brief Locks a read/write spinlock for reading.
 *
 * Multiple read locks may be acquired concurrently, though it will block with write locks.
 *
 * @remark errno will be set on failure.
 * @param lock The lock to acquire the read lock for.
 * @return False if the lock is NULL.
 */
DS_CORE_EXPORT bool dsReadWriteSpinlock_lockRead(dsReadWriteSpinlock* lock);

/**
 * @brief Unlocks a read/write spinlock for reading.
 * @remark errno will be set on failure.
 * @param lock The lock to release the read lock for.
 * @return False if the lock is NULL or it wasn't previusly locked for reading.
 */
DS_CORE_EXPORT bool dsReadWriteSpinlock_unlockRead(dsReadWriteSpinlock* lock);

/**
 * @brief Locks a read/write spinlock for writing.
 *
 * This will block if any read or write locks are currently active.
 *
 * @remark errno will be set on failure.
 * @param lock The lock to acquire the write lock for.
 * @return False if the lock is NULL.
 */
DS_CORE_EXPORT bool dsReadWriteSpinlock_lockWrite(dsReadWriteSpinlock* lock);

/**
 * @brief Unlocks a read/write spinlock for writing.
 * @remark errno will be set on failure.
 * @param lock The lock to release the write lock for.
 * @return False if the lock is NULL or it wasn't previusly locked for writing.
 */
DS_CORE_EXPORT bool dsReadWriteSpinlock_unlockWrite(dsReadWriteSpinlock* lock);

/**
 * @brief Destroys a read/write spinlock.
 * @param[inout] lock The lock to destroy.
 */
DS_CORE_EXPORT void dsReadWriteSpinlock_shutdown(dsReadWriteSpinlock* lock);

#ifdef __cplusplus
}
#endif

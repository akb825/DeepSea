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
 * @brief Spinlock for a high-performance mutex for small operations.
 *
 * A spinlock is useful for synchronizationo of very small operations. If an operation may take a
 * long time to complete, or if a ConditionVariable is needed, use Mutex instead.
 *
 * @see dsSpinlock
 */

/**
 * @brief Initializes a spinlock.
 * @param[out] spinlock The spinlock to intialize.
 * @return False if spinlock is NULL.
 */
DS_CORE_EXPORT bool dsSpinlock_initialize(dsSpinlock* spinlock);

/**
 * @brief Locks the spinlock if it isn't already locked.
 * @param spinlock The spinlock to lock.
 * @return False if the spinlock is NULL.
 */
DS_CORE_EXPORT bool dsSpinlock_tryLock(dsSpinlock* spinlock);

/**
 * @brief Locks a spinlock, blocking until it can be aquired.
 * @param spinlock The spinlock to lock.
 * @return False if spinlock is NULL.
 */
DS_CORE_EXPORT bool dsSpinlock_lock(dsSpinlock* spinlock);

/**
 * @brief Unlocks a spinlock.
 * @param spinlock The spinlock to unlock.
 * @return False if spinlock wasn't previously locked or is NULL.
 */
DS_CORE_EXPORT bool dsSpinlock_unlock(dsSpinlock* spinlock);

/**
 * @brief Destroys a spinlock.
 * @param[inout] spinlock The spinlock to destroy.
 */
DS_CORE_EXPORT void dsSpinlock_destroy(dsSpinlock* spinlock);

#ifdef __cplusplus
}
#endif

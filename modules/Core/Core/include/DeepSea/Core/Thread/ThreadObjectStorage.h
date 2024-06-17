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
#include <DeepSea/Core/Export.h>
#include <DeepSea/Core/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating thread object storage.
 * @see dsThreadObjectStorage
 */

/**
 * @brief Gets the size of a dsThreadObjectStorage instance.
 * @return The size of dsThreadObjectStorage.
 */
size_t dsThreadObjectStorage_sizeof(void);

/**
 * @brief Creates thread object storage.
 * @remark errno will be set on failure.
 * @param allocator The allocator for the thread object storage and internal state management. This
 *     must support freeing memory.
 * @param cleanupFunc The function to clean up stored objects. This will not be called for NULL
 *     values. The callback is required to enforce that this isn't used for trivial cases where
 *     dsThreadStorage would be more appropriate.
 * @return The thread object storage or NULL if it couldn't be created.
 */
DS_CORE_EXPORT dsThreadObjectStorage* dsThreadObjectStorage_create(dsAllocator* allocator,
	dsDestroyUserDataFunction cleanupFunc);

/**
 * @brief Gets the thread-specific object for the current thread.
 * @param storage The thread object storage.
 * @return The thread-specific object or NULL if no object was previously set.
 */
DS_CORE_EXPORT void* dsThreadObjectStorage_get(const dsThreadObjectStorage* storage);

/**
 * @brief Takes ownership of the thread-specific object for the current thread.
 *
 * The value for the current thread will be reset to NULL after taking it. This can be used to
 * prevent the object from being cleaned up.
 *
 * @param storage The thread object storage.
 * @return The thread-specific object or NULL if no object was previously set.
 */
DS_CORE_EXPORT void* dsThreadObjectStorage_take(const dsThreadObjectStorage* storage);

/**
 * @brief Sets the thread-specific object for the current thread.
 * @remark errno will be set on failure.
 * @remark If an object was previously set for this thread it will be cleaned up.
 * @param storage The thread object storage.
 * @param object
 */
DS_CORE_EXPORT bool dsThreadObjectStorage_set(dsThreadObjectStorage* storage, void* object);

/**
 * @brief Destroys a thread object storage.
 *
 * All remaining non-NULL objects will have the cleanup function called on them. Care should be
 * taken to ensure that any remaining threads that have set objects on storage don't exit
 * concurrently.
 *
 * @param storage The thread object storage to destroy.
 */
DS_CORE_EXPORT void dsThreadObjectStorage_destroy(dsThreadObjectStorage* storage);

#ifdef __cplusplus
}
#endif

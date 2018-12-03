/*
 * Copyright 2018 Aaron Barany
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
#include <DeepSea/Core/Memory/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manage lifetime objects.
 *
 * A lifetime object allows you to check if an object is still alive. You can acquire the pointer to
 * an object, in which case it will be guaranteed to remain alive until you release it. Aquiring the
 * object will fail if it's been destroyed. This is typically useful for ensuring proper object
 * usage if it may be deleted on another thread, and as such dsLifetime is thread-safe to use.
 *
 * The lifetime object itself will remain alive as long as a reference is active. Call
 * dsLifetime_addRef() to keep a reference alive, and dsLifetime_freeRef() once you don't need it
 * anymore.
 *
 * @see dsLifetime
 */

/**
 * @brief Gets the size of dsLifetime.
 * @return The size of dsLifetime.
 */
DS_CORE_EXPORT size_t dsLifetime_sizeof(void);

/**
 * @brief Gets the full allocated size of dsLifetime.
 * @return The full allocated size of dsLifetime.
 */
DS_CORE_EXPORT size_t dsLifetime_fullAllocSize(void);

/**
 * @brief Creates a lifetime object.
 *
 * This implicitly starts with a ref count of one, which will be freed when dsLifetime_destroy() is
 * called.
 *
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the lifetime instance with.
 * @param object The object to track. This must not be NULL.
 * @return The lifetime instance, or NULL if it failed.
 */
DS_CORE_EXPORT dsLifetime* dsLifetime_create(dsAllocator* allocator, void* object);

/**
 * @brief Adds a reference to a lifetime instance.
 * @param lifetime The lifetime instance to add ref.
 * @return The lifetime instance passed in.
 */
DS_CORE_EXPORT dsLifetime* dsLifetime_addRef(dsLifetime* lifetime);

/**
 * @brief Adds a reference to a lifetime instance.
 * @param lifetime The lifetime instance to free ref.
 */
DS_CORE_EXPORT void dsLifetime_freeRef(dsLifetime* lifetime);

/**
 * @brief Acquires the object the lifetime instance was creqted with.
 * @remark An object should only be acquired for short period of times, such as a function scope.
 * @param lifetime The lifetime instance to acquire the object from.
 * @return The object or NULL if it was destroyed. If non-NULL, then dsLifetime_release() should be
 *     called to allow the object to be destroyed.
 */
DS_CORE_EXPORT void* dsLifetime_acquire(dsLifetime* lifetime);

/**
 * @brief Releases the previously acquired object, unblocking it from being freed.
 * @param lifetime The lifetime instance to release the object.
 */
DS_CORE_EXPORT void dsLifetime_release(dsLifetime* lifetime);

/**
 * @brief Gets the object pointer for the lifetime instance.
 *
 * Unlike dsLifetime_acquire(), this won't guarantee that the object remains alive after this calll.
 *
 * @param lifetime The lifetime instance to get the object from.
 * @return The object or NULL if it was destroyed.
 */
DS_CORE_EXPORT void* dsLifetime_getObject(dsLifetime* lifetime);

/**
 * @brief Destroys the lifetime object.
 *
 * This will wait until any call to dsLifetime_acquire() is matched by a corresponding call to
 * dsLifetime_release(). After the object is destroyed, all calls to dsLifetime_acquire() and
 * dsLifetime_getObject() will return NULL.
 *
 * The memory for the lifetime instance will remain valid until all calls to dsLifetime_addRef() are
 * matched with a corresponding call to dsLifetime_freeRef().
 *
 * @param lifetime The lifetime instance to destroy.
 */
DS_CORE_EXPORT void dsLifetime_destroy(dsLifetime* lifetime);

#ifdef __cplusplus
}
#endif

/*
 * Copyright 2016-2024 Aaron Barany
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
 * @brief Macros and functions for managing thread-local storage.
 * @see dsThreadStorage
 */

/**
 * @brief Macro to declare a static or global variable to use thread-local storage.
 */
#if DS_GCC || DS_CLANG
#	define DS_THREAD_LOCAL __thread
#elif DS_MSC
#	define DS_THREAD_LOCAL __declspec(thread)
#else
#error Need to provide thread local implementation for this compiler.
#endif

/**
 * @brief Creates thread-local storage.
 * @remark errno will be set on failure.
 * @param storage The storage to create.
 * @return False if the storage couldn't be created.
 */
DS_CORE_EXPORT bool dsThreadStorage_initialize(dsThreadStorage* storage);

/**
 * @brief Gets the thread-specific data for the current thread.
 * @param storage The thread-local storage.
 * @return The thread-specific data or NULL if no data was previously set.
 */
DS_CORE_EXPORT void* dsThreadStorage_get(dsThreadStorage storage);

/**
 * @brief Sets the thread-specific data for the current thread.
 * @remark Any resources associated with the value should be destroyed before the thread exits.
 * @remark errno will be set on failure.
 * @param storage The thread-local storage.
 * @param value The value to set.
 * @return True if the value was set.
 */
DS_CORE_EXPORT bool dsThreadStorage_set(dsThreadStorage storage, void* value);

/**
 * @brief Destroys the thread-specific data.
 * @remark Any resources associated with the storage should be destroyed before the storage is
 * destoryed.
 * @param[inout] storage The storage to destroy. The contents will be cleared.
 */
DS_CORE_EXPORT void dsThreadStorage_shutdown(dsThreadStorage* storage);

#ifdef __cplusplus
}
#endif

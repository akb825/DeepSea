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
 * @brief Macros and functions for managing thread-local storage.
 */

/**
 * @brief Macro to declare a static or global variable to use thread-local storage.
 */
#ifdef _MSC_VER
#define DS_THREAD_LOCAL __declspec(thread)
#else
#define DS_THREAD_LOCAL __thread
#endif

/**
 * @brief Structure that holds thread-local storage.
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

/**
 * @brief Creates thread-local storage.
 * @param storage The storage to create.
 * @return False if the storage couldn't be created.
 */
DS_CORE_EXPORT bool dsThreadStorage_initialize(dsThreadStorage* storage);

/**
 * @brief Gets the thread-specific data.
 * @param storage The thread-local storage.
 * @return The thread-specific data or NULL if no data is specified.
 */
DS_CORE_EXPORT void* dsThreadStorage_get(dsThreadStorage storage);

/**
 * @brief Sets the thread-specific data.
 * @remark Any resources associated with the value should be destroyed before the thread exits.
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
DS_CORE_EXPORT void dsThreadStorage_destroy(dsThreadStorage* storage);

#ifdef __cplusplus
}
#endif

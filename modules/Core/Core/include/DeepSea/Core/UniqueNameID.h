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
 * @brief Functions for creating unique IDs for string names.
 *
 * Unique name IDs may be used in place of strings to use integer operations rather than string
 * operations for improved performance. This guarantees that each ID value is unique for each unique
 * name within the application.
 *
 * The value of 0 is reserved for an invalid name ID. These IDs are NOT guaranteed to be stable
 * across application runs, so the string rather than the ID should be used for persistent data such
 * as files.
 *
 * Creating and getting unique name IDs is thread-safe, though initialization and shutdown is not.
 * It is expected that initialization and shutdown should only be done once at the start and end of
 * the application.
 */

/**
 * @brief Constant for the default number of initial name IDs.
 */
#define DS_DEFAULT_INITIAL_UNIQUE_NAME_ID_LIMIT 1024

/**
 * @brief Initializes the global unique name ID state.
 * @remark errno will be set on failure.
 * @param allocator The allocator to use for managing the name IDs. This must support freeing
 *     memory.
 * @param initialNameLimit The initial number of names to allocate space for. This limit may be
 *     exceeded, but will require re-allocation of the internal storage.
 * @return False if the parameters are invalid or it has already been initialized.
 */
DS_CORE_EXPORT bool dsUniqueNameID_initialize(dsAllocator* allocator, uint32_t initialNameLimit);

/**
 * @brief Checks whether the global unique name ID state is initialized.
 * @return Whether the global unique name ID state is initialized.
 */
DS_CORE_EXPORT bool dsUniqueNameID_isInitialized(void);

/**
 * @brief Creates a unique name ID.
 *
 * This will return the previously created name ID if dsUniqueNameID_create() has been called with
 * the same name.
 *
 * @remark errno will be set on failure.
 * @param name The name to get the ID for.
 * @return The unique name ID or 0 if the unique name ID stat hasn't been initialized or the name is
 *     NULL.
 */
DS_CORE_EXPORT uint32_t dsUniqueNameID_create(const char* name);

/**
 * @brief Gets a previously createda unique name ID.
 * @param name The name to get the ID for.
 * @return The unique name ID or 0 if the unique name ID stat hasn't been initialized, the name is
 *     NULL, or the name has not had an ID previously created.
 */
DS_CORE_EXPORT uint32_t dsUniqueNameID_get(const char* name);

/**
 * @brief Shuts down the global unique name ID state.
 * @remark errno will be set on failure.
 * @return False if the unique name ID state is not currently initialized.
 */
DS_CORE_EXPORT bool dsUniqueNameID_shutdown(void);

#ifdef __cplusplus
}
#endif

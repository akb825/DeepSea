/*
 * Copyright 2019 Aaron Barany
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
#include <DeepSea/Core/Containers/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for manipulating string pools.
 *
 * Usage is typically as follows:
 * 1. Initialize the string pool with dsStringPool_initialize().
 * 2. Reserve space for each string with dsStringPool_reserve().
 * 3. Allocate the reserved memory with dsStringPool_allocate().
 * 4. Insert each string with dsStringPool_insert().
 * 5. When the memory is no longer needed, call dsStringPool_shutdown().
 *
 * @see dsStringPool
 */

/**
 * @brief Initializes a string pool.
 * @remark errno will be set on failure.
 * @param[out] stringPool The string pool to intialize.
 * @return False if stringPool is NULL.
 */
DS_CORE_EXPORT bool dsStringPool_initialize(dsStringPool* stringPool);

/**
 * @brief Reserves a string in the table.
 * @remark errno will be set on failure.
 * @param stringPool The string pool.
 * @param string The string to reserve.
 * @return False if an error occurred.
 */
DS_CORE_EXPORT bool dsStringPool_reserve(dsStringPool* stringPool, const char* string);

/**
 * @brief Allocates the reserved memory to store the strings.
 * @remark errno will be set on failure.
 * @param stringPool The string pool.
 * @param allocator The allocator to create the memory with.
 * @return False if an error occurred.
 */
DS_CORE_EXPORT bool dsStringPool_allocate(dsStringPool* stringPool, dsAllocator* allocator);

/**
 * @brief Inserts a string into the string pool.
 * @remark errno will be set on failure.
 * @param stringPool The string pool.
 * @param string The string to insert.
 * @return The inserted string, or NULL if no space was available.
 */
DS_CORE_EXPORT const char* dsStringPool_insert(dsStringPool* stringPool, const char* string);

/**
 * @brief Frees any memory in a string pool.
 * @param stringPool The string pool to shutdown.
 */
DS_CORE_EXPORT void dsStringPool_shutdown(dsStringPool* stringPool);

#ifdef __cplusplus
}
#endif

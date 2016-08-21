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
#include <DeepSea/Core/Containers/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Common hashing and key comparisson functions.
 *
 * These can be passed directly into dsHashTable_initialize(). The exception is dsHashCombine(),
 * which is used to combine two hash values.
 */

/**
 * @brief Combines two hash values.
 *
 * This can be chained to combine as many hash values together as you want. This is preferable to a
 * simple addition or XOR of the values since dsHashCombine(a, b) != dsHashCombine(b, a).
 *
 * @param first The first hash value.
 * @param second The second hash value.
 * @return The combination of first and second.
 *
 */
DS_CORE_EXPORT uint32_t dsHashCombine(uint32_t first, uint32_t second);

/**
 * @brief Hashes a C string.
 * @param string The string to hash.
 */
DS_CORE_EXPORT uint32_t dsHashString(const void* string);

/**
 * @brief Checks if two C strings are equal.
 * @param first The first string.
 * @param second The second string.
 * @return True if first and second are equal.
 */
DS_CORE_EXPORT bool dsHashStringEqual(const void* first, const void* second);

/**
 * @brief Hashes an 8-bit value.
 * @param ptr A pointer to the value to hash.
 * @return The hashed value.
 */
DS_CORE_EXPORT uint32_t dsHash8(const void* ptr);

/**
 * @brief Checks if two 8-bit values are equal.
 * @param first A pointer to the first value.
 * @param second A pointer to the second value.
 * @return The hashed value.
 */
DS_CORE_EXPORT bool dsHash8Equal(const void* first, const void* second);

/**
 * @brief Hashes an 16-bit value.
 * @param ptr A pointer to the value to hash.
 * @return The hashed value.
 */
DS_CORE_EXPORT uint32_t dsHash16(const void* ptr);

/**
 * @brief Checks if two 16-bit values are equal.
 * @param first A pointer to the first value.
 * @param second A pointer to the second value.
 * @return The hashed value.
 */
DS_CORE_EXPORT bool dsHash16Equal(const void* first, const void* second);

/**
 * @brief Hashes a 32-bit value.
 * @param ptr A pointer to the value to hash.
 * @return The hashed value.
 */
DS_CORE_EXPORT uint32_t dsHash32(const void* ptr);

/**
 * @brief Checks if two 32-bit values are equal.
 * @param first A pointer to the first value.
 * @param second A pointer to the second value.
 * @return The hashed value.
 */
DS_CORE_EXPORT bool dsHash32Equal(const void* first, const void* second);

/**
 * @brief Hashes a 64-bit value.
 * @param ptr A pointer to the value to hash.
 * @return The hashed value.
 */
DS_CORE_EXPORT uint32_t dsHash64(const void* ptr);

/**
 * @brief Checks if two 64-bit values are equal.
 * @param first A pointer to the first value.
 * @param second A pointer to the second value.
 * @return The hashed value.
 */
DS_CORE_EXPORT bool dsHash64Equal(const void* first, const void* second);

/**
 * @brief Hashes a uint32_t value.
 * @param ptr A pointer to the value to hash.
 * @return The hashed value.
 */
DS_CORE_EXPORT uint32_t dsHashSizeT(const void* ptr);

/**
 * @brief Checks if two uint32_t values are equal.
 * @param first A pointer to the first value.
 * @param second A pointer to the second value.
 * @return The hashed value.
 */
DS_CORE_EXPORT bool dsHashSizeTEqual(const void* first, const void* second);

/**
 * @brief Hashes a pointer.
 *
 * This hashes the value passed in directly rather than dereferencing it.
 *
 * @param ptr The pointer to hash.
 */
DS_CORE_EXPORT uint32_t dsHashPointer(const void* ptr);

/**
 * @brief Checks if two pointers are equal.
 *
 * This is the same as first == second.
 *
 * @param first The first pointer.
 * @param second The second pointer.
 * @return True if first and second are equal.
 */
DS_CORE_EXPORT bool dsHashPointerEqual(const void* first, const void* second);

#ifdef __cplusplus
}
#endif

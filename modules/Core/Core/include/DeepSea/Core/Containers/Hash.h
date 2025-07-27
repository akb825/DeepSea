/*
 * Copyright 2016-2025 Aaron Barany
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
 * @brief Default seed value when computing a hash.
 *
 * This may be used with a hash combine function to get the same result as the non-combine version.
 */
#define DS_DEFAULT_HASH_SEED 0xc70f6907U

/**
 * @brief Generic hash generation for a list of bytes.
 * @param buffer The bytes to hash.
 * @param size The size of the buffer.
 * @return The hash.
 */
DS_CORE_EXPORT uint32_t dsHashBytes(const void* buffer, size_t size);

/**
 * @brief Generic hash generation for a list of bytes, combining with a previous hash.
 * @param seed The previous hash value.
 * @param buffer The bytes to hash.
 * @param size The size of the buffer.
 * @return The hash.
 */
DS_CORE_EXPORT uint32_t dsHashCombineBytes(uint32_t seed, const void* buffer, size_t size);

/**
 * @brief Generic 128-bit hash generation for a list of bytes, combining with a previous hash.
 *
 * This is useful for tasks such as comparing file hashes, where more than the typical 32 bits is
 * desired.
 *
 * @param[out] outResult The result. This must be 128 bits.
 * @param seed The previous hash value. This must be 128 bits.
 * @param buffer The bytes to hash.
 * @param size The size of the buffer.
 */
DS_CORE_EXPORT void dsHashCombineBytes128(void* outResult, const void* seed, const void* buffer,
	size_t size);

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
 * @brief Returns the value directly as a uint32_t value.
 *
 * This is typically used when the value is pre-hashed for performance reasons.
 *
 * @param value A pointer to a uint32_t for the hash value.
 * @return The hash value.
 */
DS_CORE_EXPORT uint32_t dsHashIdentity(const void* value);

/**
 * @brief Hashes a C string.
 * @param string The string to hash.
 */
DS_CORE_EXPORT uint32_t dsHashString(const void* string);

/**
 * @brief Hashes a C string.
 * @param string The string to hash.
 */
DS_CORE_EXPORT uint32_t dsHashString(const void* string);

/**
 * @brief Hashes a C string, combining with a previous hash.
 * @param seed The previous hash value.
 * @param string The string to hash.
 */
DS_CORE_EXPORT uint32_t dsHashCombineString(uint32_t seed, const void* string);

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
 * @brief Hashes an 8-bit value, combining with a previous hash.
 * @param seed The previous hash value.
 * @param ptr A pointer to the value to hash.
 * @return The hashed value.
 */
DS_CORE_EXPORT uint32_t dsHashCombine8(uint32_t seed, const void* ptr);

/**
 * @brief Checks if two 8-bit values are equal.
 * @param first A pointer to the first value.
 * @param second A pointer to the second value.
 * @return The hashed value.
 */
DS_CORE_EXPORT bool dsHash8Equal(const void* first, const void* second);

/**
 * @brief Hashes a 16-bit value.
 * @param ptr A pointer to the value to hash.
 * @return The hashed value.
 */
DS_CORE_EXPORT uint32_t dsHash16(const void* ptr);

/**
 * @brief Hashes a 16-bit value, combining with a previous hash.
 * @param seed The previous hash value.
 * @param ptr A pointer to the value to hash.
 * @return The hashed value.
 */
DS_CORE_EXPORT uint32_t dsHashCombine16(uint32_t seed, const void* ptr);

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
 * @brief Hashes a 32-bit value, combining with a previous hash.
 * @param seed The previous hash value.
 * @param ptr A pointer to the value to hash.
 * @return The hashed value.
 */
DS_CORE_EXPORT uint32_t dsHashCombine32(uint32_t seed, const void* ptr);

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
 * @brief Hashes a 64-bit value, combining with a previous hash.
 * @param seed The previous hash value.
 * @param ptr A pointer to the value to hash.
 * @return The hashed value.
 */
DS_CORE_EXPORT uint32_t dsHashCombine64(uint32_t seed, const void* ptr);

/**
 * @brief Checks if two 64-bit values are equal.
 * @param first A pointer to the first value.
 * @param second A pointer to the second value.
 * @return The hashed value.
 */
DS_CORE_EXPORT bool dsHash64Equal(const void* first, const void* second);

/**
 * @brief Hashes a size_t value.
 * @param ptr A pointer to the value to hash.
 * @return The hashed value.
 */
DS_CORE_EXPORT uint32_t dsHashSizeT(const void* ptr);

/**
 * @brief Hashes a size_t value, combining with a previous hash.
 * @param seed The previous hash value.
 * @param ptr A pointer to the value to hash.
 * @return The hashed value.
 */
DS_CORE_EXPORT uint32_t dsHashCombineSizeT(uint32_t seed, const void* ptr);

/**
 * @brief Checks if two size_t values are equal.
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
 * @brief Hashes a pointer, combining with a previous hash.
 *
 * This hashes the value passed in directly rather than dereferencing it.
 *
 * @param seed The previous hash value.
 * @param ptr A pointer to the value to hash.
 * @return The hashed value.
 */
DS_CORE_EXPORT uint32_t dsHashCombinePointer(uint32_t seed, const void* ptr);

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

/**
 * @brief Hashes a float value.
 *
 * This is similar to dsHash32(), except it treats 0 and -0 as the same.
 *
 * @param ptr A pointer to the value to hash.
 * @return The hashed value.
 */
DS_CORE_EXPORT uint32_t dsHashFloat(const void* ptr);

/**
 * @brief Hashes a float value, combining with a previous hash.
 *
 * This is similar to dsHashCombine32(), except it treats 0 and -0 as the same.
 *
 * @param seed The previous hash value.
 * @param ptr A pointer to the value to hash.
 * @return The hashed value.
 */
DS_CORE_EXPORT uint32_t dsHashCombineFloat(uint32_t seed, const void* ptr);

/**
 * @brief Checks if two float values are equal.
 *
 * This is similar to dsHash32Equal(), except it treats 0 and -0 as the same.
 *
 * @param first A pointer to the first value.
 * @param second A pointer to the second value.
 * @return The hashed value.
 */
DS_CORE_EXPORT bool dsHashFloatEqual(const void* first, const void* second);

/**
 * @brief Hashes a double value.
 *
 * This is similar to dsHash64(), except it treats 0 and -0 as the same.
 *
 * @param ptr A pointer to the value to hash.
 * @return The hashed value.
 */
DS_CORE_EXPORT uint32_t dsHashDouble(const void* ptr);

/**
 * @brief Hashes a double value, combining with a previous hash.
 *
 * This is similar to dsHashCombine64(), except it treats 0 and -0 as the same.
 *
 * @param seed The previous hash value.
 * @param ptr A pointer to the value to hash.
 * @return The hashed value.
 */
DS_CORE_EXPORT uint32_t dsHashCombineDouble(uint32_t seed, const void* ptr);

/**
 * @brief Checks if two double values are equal.
 *
 * This is similar to dsHash64Equal(), except it treats 0 and -0 as the same.
 *
 * @param first A pointer to the first value.
 * @param second A pointer to the second value.
 * @return The hashed value.
 */
DS_CORE_EXPORT bool dsHashDoubleEqual(const void* first, const void* second);

#ifdef __cplusplus
}
#endif

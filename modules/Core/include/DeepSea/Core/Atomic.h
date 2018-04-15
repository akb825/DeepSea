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
#include <stdint.h>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Macros for atomic operations.
 */

#if DS_GCC || DS_CLANG

/**
 * @brief Atomically loads a 32-bit value.
 * @param[in] xPtr A pointer to the atomic value to load.
 * @param[out] returnPtr A pointer to the value to load to.
 */
#define DS_ATOMIC_LOAD32(xPtr, returnPtr) \
	__atomic_load((int32_t*)(xPtr), (int32_t*)(returnPtr), __ATOMIC_SEQ_CST)

/**
 * @brief Atomically loads a 64-bit value.
 * @param[in] xPtr A pointer to the atomic value to load.
 * @param[out] returnPtr A pointer to the value to load to.
 */
#define DS_ATOMIC_LOAD64(xPtr, returnPtr) \
	__atomic_load((int64_t*)(xPtr), (int64_t*)(returnPtr), __ATOMIC_SEQ_CST)

/**
 * @brief Atomically stores a 32-bit value.
 * @param[out] xPtr A pointer to the atomic value to store to.
 * @param[in] valuePtr A pointer to the value to store.
 */
#define DS_ATOMIC_STORE32(xPtr, valuePtr) \
	__atomic_store((int32_t*)(xPtr), (int32_t*)(valuePtr), __ATOMIC_SEQ_CST)

/**
 * @brief Atomically stores a 64-bit value.
 * @param[out] xPtr A pointer to the atomic value to store to.
 * @param[in] valuePtr A pointer to the value to store.
 */
#define DS_ATOMIC_STORE64(xPtr, valuePtr) \
	__atomic_store((int64_t*)(xPtr), (int64_t*)(valuePtr), __ATOMIC_SEQ_CST)

/**
 * @brief Atomically exchanges a 32-bit value.
 * @param[out] xPtr A pointer to the atomic value to exchange with.
 * @param[in] valuePtr A pointer to the new value to set.
 * @param[out] returnPtr A pointer to store the original value.
 */
#define DS_ATOMIC_EXCHANGE32(xPtr, valuePtr, returnPtr) \
	__atomic_exchange((int32_t*)(xPtr), (int32_t*)(valuePtr),(int32_t*)(returnPtr), __ATOMIC_SEQ_CST)

/**
 * @brief Atomically exchanges a 64-bit value.
 * @param[out] xPtr A pointer to the atomic value to exchange with.
 * @param[in] valuePtr A pointer to the new value to set.
 * @param[out] returnPtr A pointer to store the original value.
 */
#define DS_ATOMIC_EXCHANGE64(xPtr, valuePtr, returnPtr) \
	__atomic_exchange((int64_t*)(xPtr), (int64_t*)(valuePtr),(int64_t*)(returnPtr), __ATOMIC_SEQ_CST)

/**
 * @brief Atomically exchanges a 32-bit value if atomic value matches the expected value.
 * @param[inout] xPtr A pointer to the atomic value to exchange with.
 * @param[inout] expectedPtr A pointer to the expected value. This will be populated with the
 *     current value of xPtr if the comparison fails.
 * @param[in] valuePtr A pointer to the new value to set.
 * @param weak True if the comparison is allowed to fail even if would normally succeed. This can
 *     improve performance, but the call should be done in a loop.
 * @return True if the exchange took place.
 */
#define DS_ATOMIC_COMPARE_EXCHANGE32(xPtr, expectedPtr, valuePtr, weak) \
	__atomic_compare_exchange((int32_t*)(xPtr), (int32_t*)(expectedPtr), (int32_t*)(valuePtr), \
		weak, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)

/**
 * @brief Atomically exchanges a 64-bit value if atomic value matches the expected value.
 * @param[inout] xPtr A pointer to the atomic value to exchange with.
 * @param[inout] expectedPtr A pointer to the expected value. This will be populated with the
 *     current value of xPtr if the comparison fails.
 * @param[in] valuePtr A pointer to the new value to set.
 * @param weak True if the comparison is allowed to fail even if would normally succeed. This can
 *     improve performance, but the call should be done in a loop.
 * @return True if the exchange took place.
 */
#define DS_ATOMIC_COMPARE_EXCHANGE64(xPtr, expectedPtr, valuePtr, weak) \
	__atomic_compare_exchange((int64_t*)(xPtr), (int64_t*)(expectedPtr), (int64_t*)(valuePtr), \
		weak, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)

/**
 * @brief Atomically fetches a 32-bit value and adds to it.
 * @param[inout] xPtr A pointer to the atomic value to add to.
 * @param value The value to add to the atomic value.
 * @return The value of the atomic value before the add.
 */
#define DS_ATOMIC_FETCH_ADD32(xPtr, value) \
	__atomic_fetch_add((int32_t*)(xPtr), (int32_t)(value), __ATOMIC_SEQ_CST)

/**
 * @brief Atomically fetches a v64-bit alue and adds to it.
 * @param[inout] xPtr A pointer to the atomic value to add to.
 * @param value The value to add to the atomic value.
 * @return The value of the atomic value before the add.
 */
#define DS_ATOMIC_FETCH_ADD64(xPtr, value) \
	__atomic_fetch_add((int64_t*)(xPtr), (int64_t)(value), __ATOMIC_SEQ_CST)

#elif DS_MSC

#if DS_64BIT
#define DS_INTERLOCKED_OR64_IMPL _InterlockedOr64
#define DS_INTERLOCKED_EXCHANGE64_IMPL _InterlockedExchange64
#define DS_INTERLOCKED_COMPARE_EXCHANGE64_IMPL _InterlockedCompareExchange64
#define DS_INTERLOCKED_EXCHANGE_ADD64_IMPL _InterlockedExchangeAdd64
#else
#define DS_INTERLOCKED_OR64_IMPL dsAtomic_interlockedOr64Impl
#define DS_INTERLOCKED_EXCHANGE64_IMPL dsAtomic_interlockedExchange64Impl
#define DS_INTERLOCKED_COMPARE_EXCHANGE64_IMPL dsAtomic_interlockedCompareExchange64Impl
#define DS_INTERLOCKED_EXCHANGE_ADD64_IMPL dsAtomic_interlockedExchangeAdd64Impl

DS_CORE_EXPORT __int64 dsAtomic_interlockedOr64Impl(__int64* xPtr, __int64 value);
DS_CORE_EXPORT __int64 dsAtomic_interlockedExchange64Impl(__int64* xPtr, __int64 value);
DS_CORE_EXPORT __int64 dsAtomic_interlockedCompareExchange64Impl(__int64* xPtr, __int64 value,
	__int64 expected);
DS_CORE_EXPORT __int64 dsAtomic_interlockedExchangeAdd64Impl(__int64* xPtr, __int64 value);
#endif

#define DS_ATOMIC_LOAD32(xPtr, returnPtr) \
	(void)(*(long*)(returnPtr) = _InterlockedOr((long*)(xPtr), 0))

#define DS_ATOMIC_LOAD64(xPtr, returnPtr) \
	(void)(*(__int64*)(returnPtr) = DS_INTERLOCKED_OR64_IMPL((__int64*)(xPtr), 0))

#define DS_ATOMIC_STORE32(xPtr, valuePtr) \
	(void)_InterlockedExchange((long*)(xPtr), *(long*)(valuePtr))

#define DS_ATOMIC_STORE64(xPtr, valuePtr) \
	(void)DS_INTERLOCKED_EXCHANGE64_IMPL((__int64*)(xPtr), *(__int64*)(valuePtr))

#define DS_ATOMIC_EXCHANGE32(xPtr, valuePtr, returnPtr) \
	(void)(*(long*)(returnPtr) = _InterlockedExchange((long*)(xPtr), *(long*)(valuePtr)))

#define DS_ATOMIC_EXCHANGE64(xPtr, valuePtr, returnPtr) \
	(void)(*(__int64*)(returnPtr) = DS_INTERLOCKED_EXCHANGE64_IMPL((__int64*)(xPtr), \
		*(__int64*)(valuePtr)))

inline int dsAtomic_compareExchange32Impl(long* xPtr, long* expectedPtr, long* valuePtr)
{
	long expected = *expectedPtr;
	*expectedPtr = _InterlockedCompareExchange(xPtr, *valuePtr, *expectedPtr);
	return *expectedPtr == expected;
}

#define DS_ATOMIC_COMPARE_EXCHANGE32(xPtr, expectedPtr, valuePtr, weak) \
	dsAtomic_compareExchange32Impl((long*)(xPtr), (long*)(expectedPtr), (long*)(valuePtr))

inline int dsAtomic_compareExchange64Impl(__int64* xPtr, __int64* expectedPtr, __int64* valuePtr)
{
	__int64 expected = *expectedPtr;
	*expectedPtr = DS_INTERLOCKED_COMPARE_EXCHANGE64_IMPL(xPtr, *valuePtr, *expectedPtr);
	return *expectedPtr == expected;
}

#define DS_ATOMIC_COMPARE_EXCHANGE64(xPtr, expectedPtr, valuePtr, weak) \
	dsAtomic_compareExchange64Impl((__int64*)(xPtr), (__int64*)(expectedPtr), (__int64*)(valuePtr))

#define DS_ATOMIC_FETCH_ADD32(xPtr, value) \
	_InterlockedExchangeAdd((long*)(xPtr), (long)(value))

#define DS_ATOMIC_FETCH_ADD64(xPtr, value) \
	DS_INTERLOCKED_EXCHANGE_ADD64_IMPL((__int64*)(xPtr), (__int64)(value))

#else

#error Need to provide atomic operations for this compiler.

#endif

#if DS_64BIT

/**
 * @brief Atomically loads a size_t value.
 * @param[in] xPtr A pointer to the atomic value to load.
 * @param[out] returnPtr A pointer to the value to load to.
 */
#define DS_ATOMIC_LOAD_SIZE(xPtr, returnPtr) DS_ATOMIC_LOAD64(xPtr, returnPtr)

/**
 * @brief Atomically stores a size_t value.
 * @param[out] xPtr A pointer to the atomic value to store to.
 * @param[in] valuePtr A pointer to the value to store.
 */
#define DS_ATOMIC_STORE_SIZE(xPtr, valuePtr) DS_ATOMIC_STORE64(xPtr, valuePtr)

/**
 * @brief Atomically exchanges a size value.
 * @param[out] xPtr A pointer to the atomic value to exchange with.
 * @param[in] valuePtr A pointer to the new value to set.
 * @param[out] returnPtr A pointer to store the original value.
 */
#define DS_ATOMIC_EXCHANGE_SIZE(xPtr, valuePtr, returnPtr) \
	DS_ATOMIC_EXCHANGE64(xPtr, valuePtr, returnPtr)

/**
 * @brief Atomically exchanges a size_t value if atomic value matches the expected value.
 * @param[inout] xPtr A pointer to the atomic value to exchange with.
 * @param[inout] expectedPtr A pointer to the expected value. This will be populated with the
 *     current value of xPtr if the comparison fails.
 * @param[in] valuePtr A pointer to the new value to set.
 * @param weak True if the comparison is allowed to fail even if would normally succeed. This can
 *     improve performance, but the call should be done in a loop.
 * @return True if the exchange took place.
 */
#define DS_ATOMIC_COMPARE_EXCHANGE_SIZE(xPtr, expectedPtr, valuePtr, weak) \
	DS_ATOMIC_COMPARE_EXCHANGE64(xPtr, expectedPtr, valuePtr, weak)

/**
 * @brief Atomically fetches a size_t value and adds to it.
 * @param[inout] xPtr A pointer to the atomic value to add to.
 * @param value The value to add to the atomic value.
 * @return The value of the atomic value before the add.
 */
#define DS_ATOMIC_FETCH_ADD_SIZE(xPtr, value) DS_ATOMIC_FETCH_ADD64(xPtr, value)

/**
 * @brief Atomically loads a pointer value.
 * @param[in] xPtr A pointer to the atomic value to load.
 * @param[out] returnPtr A pointer to the value to load to.
 */
#define DS_ATOMIC_LOAD_PTR(xPtr, returnPtr) DS_ATOMIC_LOAD64(xPtr, returnPtr)

/**
 * @brief Atomically stores a pointer value.
 * @param[out] xPtr A pointer to the atomic value to store to.
 * @param[in] valuePtr A pointer to the value to store.
 */
#define DS_ATOMIC_STORE_PTR(xPtr, valuePtr) DS_ATOMIC_STORE64(xPtr, valuePtr)

/**
 * @brief Atomically exchanges a pointer value.
 * @param[out] xPtr A pointer to the atomic value to exchange with.
 * @param[in] valuePtr A pointer to the new value to set.
 * @param[out] returnPtr A pointer to store the original value.
 */
#define DS_ATOMIC_EXCHANGE_PTR(xPtr, valuePtr, returnPtr) \
	DS_ATOMIC_EXCHANGE64(xPtr, valuePtr, returnPtr)

/**
 * @brief Atomically exchanges a pointer value if atomic value matches the expected value.
 * @param[inout] xPtr A pointer to the atomic value to exchange with.
 * @param[inout] expectedPtr A pointer to the expected value. This will be populated with the
 *     current value of xPtr if the comparison fails.
 * @param[in] valuePtr A pointer to the new value to set.
 * @param weak True if the comparison is allowed to fail even if would normally succeed. This can
 *     improve performance, but the call should be done in a loop.
 * @return True if the exchange took place.
 */
#define DS_ATOMIC_COMPARE_EXCHANGE_PTR(xPtr, expectedPtr, valuePtr, weak) \
	DS_ATOMIC_COMPARE_EXCHANGE64(xPtr, expectedPtr, valuePtr, weak)

/**
 * @brief Atomically fetches a pointer value and adds to it.
 * @param[inout] xPtr A pointer to the atomic value to add to.
 * @param value The value to add to the atomic value.
 * @return The value of the atomic value before the add. This will be returned as a void*.
 */
#define DS_ATOMIC_FETCH_ADD_PTR(xPtr, value) \
	(void*)DS_ATOMIC_FETCH_ADD64(xPtr, (value)*(int64_t)sizeof(**(xPtr)))

#else

#define DS_ATOMIC_LOAD_SIZE(xPtr, valuePtr) DS_ATOMIC_LOAD32(xPtr, valuePtr)

#define DS_ATOMIC_STORE_SIZE(xPtr, valuePtr) DS_ATOMIC_STORE32(xPtr, valuePtr)

#define DS_ATOMIC_EXCHANGE_SIZE(xPtr, valuePtr, returnPtr) \
	DS_ATOMIC_EXCHANGE32(xPtr, valuePtr, returnPtr)

#define DS_ATOMIC_COMPARE_EXCHANGE_SIZE(xPtr, expectedPtr, valuePtr, weak) \
	DS_ATOMIC_COMPARE_EXCHANGE32(xPtr, expectedPtr, valuePtr, weak)

#define DS_ATOMIC_FETCH_ADD_SIZE(xPtr, value)  DS_ATOMIC_FETCH_ADD32(xPtr, value)

#define DS_ATOMIC_LOAD_PTR(xPtr, valuePtr) DS_ATOMIC_LOAD32(xPtr, valuePtr)

#define DS_ATOMIC_STORE_PTR(xPtr, valuePtr) DS_ATOMIC_STORE32(xPtr, valuePtr)

#define DS_ATOMIC_EXCHANGE_PTR(xPtr, valuePtr, returnPtr) \
	DS_ATOMIC_EXCHANGE32(xPtr, valuePtr, returnPtr)

#define DS_ATOMIC_COMPARE_EXCHANGE_PTR(xPtr, expectedPtr, valuePtr, weak) \
	DS_ATOMIC_COMPARE_EXCHANGE32(xPtr, expectedPtr, valuePtr, weak)

#define DS_ATOMIC_FETCH_ADD_PTR(xPtr, value) \
	(void*)DS_ATOMIC_FETCH_ADD32(xPtr, (value)*(int32_t)sizeof(**(xPtr)))

#endif

#ifdef __cplusplus
}
#endif

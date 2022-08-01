/*
 * Copyright 2016-2022 Aaron Barany
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

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Export.h>
#include <DeepSea/Math/Types.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to generate random numbers.
 *
 * Random numbers are generated with the xoshiro256** algorithm. This has a large period (2^256 - 1)
 * and is statistically indistinguishable from true random numbers for all known tests.
 *
 * See https://prng.di.unimi.it for more informtion.
 *
 * @remark These random numbers aren't suitable for cryptographic purposes, as an attacker can
 * reconstruct the state with relatively little data.
 */

/**
 * @brief Creates a random seed.
 *
 * This uses a combination of a high frequency time and a counter to guarantee that two calls in
 * rapid succession will have different values.
 *
 * @return A random seed.
 */
DS_MATH_EXPORT uint64_t dsRandom_createSeed(void);

/**
 * @brief Seeds a random number generator.
 * @param[out] random The random number generator to seed.
 * @param seed The seed for the generator.
 */
DS_MATH_EXPORT void dsRandom_seed(dsRandom* random, uint64_t seed);

/**
 * @brief Gets the next raw random number.
 * @param[inout] random The random number generator.
 * @return A random 64-bit number. All bits should be of high quality randomness.
 */
DS_MATH_EXPORT uint64_t dsRandom_next(dsRandom* random);

/**
 * @brief Gets the next random value as a bool.
 * @param[inout] random The random number generator.
 * @return A random bool value of true or false.
 */
DS_MATH_EXPORT inline bool dsRandom_nextBool(dsRandom* random);

/**
 * @brief Gets the next random value as a uint32_t value up to a maximum.
 * @remark This avoids bias that would be encountered when taking the naive approach of
 *     dsRandom_next() % maxBound.
 * @remark This will reliably return up to 24 bits of randomness.
 * @param[inout] random The random number generator.
 * @param maxBound The maximum bound that all values will be less than.
 * @return A random value in the range of [0, maxBound).
 */
DS_MATH_EXPORT inline uint32_t dsRandom_nextUInt32(dsRandom* random, uint32_t maxBound);

/**
 * @brief Gets the next random value as a uint32_t value in a custom range.
 * @remark This avoids bias that would be encountered when taking the naive approach of
 *     dsRandom_next() % (maxBound - minVal) + minVal.
 * @remark This will reliably return up to 24 bits of randomness.
 * @param[inout] random The random number generator.
 * @param minVal The minimum value.
 * @param maxBound The maximum bound that all values will be less than.
 * @return A random value in the range of [minVal, maxBound).
 */
DS_MATH_EXPORT inline uint32_t dsRandom_nextUInt32Range(dsRandom* random, uint32_t minVal,
	uint32_t maxBound);

/**
 * @brief Gets the next random value as an int32_t value in a custom range.
 * @remark This avoids bias that would be encountered when taking the naive approach of
 *     dsRandom_next() % (maxBound - minVal) + minVal.
 * @remark This will reliably return up to 24 bits of randomness.
 * @param[inout] random The random number generator.
 * @param minVal The minimum value.
 * @param maxBound The maximum bound that all values will be less than.
 * @return A random value in the range of [minVal, maxBound).
 */
DS_MATH_EXPORT inline int32_t dsRandom_nextInt32Range(dsRandom* random, int32_t minVal,
	int32_t maxBound);

/**
 * @brief Gets the next random value as a uint64_t value up to a maximum.
 * @remark This avoids bias that would be encountered when taking the naive approach of
 *     dsRandom_next() % maxBound.
 * @remark This will reliably return up to 53 bits of randomness.
 * @param[inout] random The random number generator.
 * @param maxBound The maximum bound that all values will be less than.
 * @return A random value in the range of [0, maxBound).
 */
DS_MATH_EXPORT inline uint64_t dsRandom_nextUInt64(dsRandom* random, uint64_t maxBound);

/**
 * @brief Gets the next random value as a uint64_t value in a custom range.
 * @remark This avoids bias that would be encountered when taking the naive approach of
 *     dsRandom_next() % (maxBound - minVal) + minVal.
 * @remark This will reliably return up to 53 bits of randomness.
 * @param[inout] random The random number generator.
 * @param minVal The minimum value.
 * @param maxBound The maximum bound that all values will be less than.
 * @return A random value in the range of [minVal, maxBound).
 */
DS_MATH_EXPORT inline uint64_t dsRandom_nextUInt64Range(dsRandom* random, uint64_t minVal,
	uint64_t maxBound);

/**
 * @brief Gets the next random value as an int64_t value in a custom range.
 * @remark This avoids bias that would be encountered when taking the naive approach of
 *     dsRandom_next() % (maxBound - minVal) + minVal.
 * @remark This will reliably return up to 53 bits of randomness.
 * @param[inout] random The random number generator.
 * @param minVal The minimum value.
 * @param maxBound The maximum bound that all values will be less than.
 * @return A random value in the range of [minVal, maxBound).
 */
DS_MATH_EXPORT inline int64_t dsRandom_nextInt64Range(dsRandom* random, int64_t minVal,
	int64_t maxBound);

/**
 * @brief Gets the next random value as a float in the range [0, 1).
 * @param[inout] random The random number generator.
 * @return A random float value in the range [0, 1).
 */
DS_MATH_EXPORT inline float dsRandom_nextFloat(dsRandom* random);

/**
 * @brief Gets the next random value as a float in a custom range.
 * @remark The final result may suffer from floating point roundoff error, possibly taking the
 *     value out of the desired range.
 * @param[inout] random The random number generator.
 * @param minVal The minimum value.
 * @param maxBound The maximum bounds that all values will be less than.
 * @return A random float value in the range [minVal, maxBound).
 */
DS_MATH_EXPORT inline float dsRandom_nextFloatRange(dsRandom* random, float minVal,
	float maxBound);

/**
 * @brief Gets the next random value as a double in the range [0, 1).
 * @param[inout] random The random number generator.
 * @return A random double value in the range [0, 1).
 */
DS_MATH_EXPORT inline double dsRandom_nextDouble(dsRandom* random);

/**
 * @brief Gets the next random value as a double in a custom range.
 * @remark The final result may suffer from floating point roundoff error, possibly taking the
 *     value out of the desired range.
 * @param[inout] random The random number generator.
 * @param minVal The minimum value.
 * @param maxBound The maximum bounds that all values will be less than.
 * @return A random double value in the range [minVal, maxBound).
 */
DS_MATH_EXPORT inline double dsRandom_nextDoubleRange(dsRandom* random, double minVal,
	double maxBound);

DS_MATH_EXPORT inline bool dsRandom_nextBool(dsRandom* random)
{
	return (bool)(dsRandom_next(random) & 1);
}

DS_MATH_EXPORT inline uint32_t dsRandom_nextUInt32(dsRandom* random, uint32_t maxBound)
{
	DS_ASSERT(maxBound < 0x1000000);
	return (uint32_t)(dsRandom_nextFloat(random)*(float)maxBound);
}

DS_MATH_EXPORT inline uint32_t dsRandom_nextUInt32Range(dsRandom* random, uint32_t minVal,
	uint32_t maxBound)
{
	DS_ASSERT(minVal <= maxBound);
	return dsRandom_nextUInt32(random, maxBound - minVal) + minVal;
}

DS_MATH_EXPORT inline int32_t dsRandom_nextInt32Range(dsRandom* random, int32_t minVal,
	int32_t maxBound)
{
	DS_ASSERT(minVal <= maxBound);
	return dsRandom_nextUInt32(random, maxBound - minVal) + minVal;
}

DS_MATH_EXPORT inline uint64_t dsRandom_nextUInt64(dsRandom* random, uint64_t maxBound)
{
	DS_ASSERT(maxBound < 0x20000000000000ULL);
	return (uint64_t)(dsRandom_nextDouble(random)*(double)maxBound);
}

DS_MATH_EXPORT inline uint64_t dsRandom_nextUInt64Range(dsRandom* random, uint64_t minVal,
	uint64_t maxBound)
{
	DS_ASSERT(minVal <= maxBound);
	return dsRandom_nextUInt64(random, maxBound - minVal) + minVal;
}

DS_MATH_EXPORT inline int64_t dsRandom_nextInt64Range(dsRandom* random, int64_t minVal,
	int64_t maxBound)
{
	DS_ASSERT(minVal <= maxBound);
	return dsRandom_nextUInt64(random, maxBound - minVal) + minVal;
}

DS_MATH_EXPORT inline float dsRandom_nextFloat(dsRandom* random)
{
	return (float)(dsRandom_next(random) >> 40)*0x1.0p-24f;
}

DS_MATH_EXPORT inline float dsRandom_nextFloatRange(dsRandom* random, float minVal,
	float maxBound)
{
	DS_ASSERT(minVal <= maxBound);
	float range = maxBound - minVal;
	return dsRandom_nextFloat(random)*range + minVal;
}

DS_MATH_EXPORT inline double dsRandom_nextDouble(dsRandom* random)
{
	return (double)(dsRandom_next(random) >> 11)*0x1.0p-53;
}

DS_MATH_EXPORT inline double dsRandom_nextDoubleRange(dsRandom* random, double minVal,
	double maxBound)
{
	DS_ASSERT(minVal <= maxBound);
	double range = maxBound - minVal;
	return dsRandom_nextDouble(random)*range + minVal;
}

#ifdef __cplusplus
}
#endif

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
 * Random numbers are generated with the xoshiro256++ algorithm. This has a large period (2^256 - 1)
 * and is statistically indistinguishable from true random numbers for all known tests. The ++
 * scrambler is chosen over the ** scrambler as it is harder to accidentally reduce the quality of
 * scrambling, such as by multiplying by certain factors.
 *
 * See https://prng.di.unimi.it for more informtion.
 *
 * @remark These random numbers aren't suitable for cryptographic purposes, as an attacker can
 * reconstruct the state with relatively little data.
 *
 * @see dsRandom
 */

/**
 * @brief Creates a random seed.
 *
 * This creates a fully random seed that can be re-used or shared for consistent behavior. This can
 * be used for dsRandom_seed() to initialize a dsRandom instance with the seed, and may also be used
 * with dsRandom_nextSeed() to advance the seed to initialize mutliple dsRandom instances.
 *
 * @return A random seed.
 */
DS_MATH_EXPORT uint64_t dsRandom_createSeed(void);

/**
 * @brief Gets the next seed based on a state.
 *
 * This can be useful to very rapidly and deterministically seed multiple dsRandom instances with
 * different states.
 *
 * @param[inout] state The current state to get the next seed from. The initial value can be any
 *      64-bit number.
 * @return The next seed.
 */
DS_MATH_EXPORT inline uint64_t dsRandom_nextSeed(uint64_t* state);

/**
 * @brief Seeds a random number generator.
 * @param[out] random The random number generator to seed.
 * @param seed The seed for the generator.
 */
DS_MATH_EXPORT void dsRandom_seed(dsRandom* random, uint64_t seed);

/**
 * @brief Initializes a random number generator with a fully random state.
 *
 * This provides more initial states than what dsRandom_seed() can provide. This is the preferred
 * way to initialize dsRandom when you don't want a repeatable initial state.
 *
 * @param[out] random The random number generator to initialize.
 */
DS_MATH_EXPORT void dsRandom_initialize(dsRandom* random);

/**
 * @brief Gets the next raw random number.
 * @param[inout] random The random number generator.
 * @return A random 64-bit number. All bits should be of high quality randomness.
 */
DS_MATH_EXPORT inline uint64_t dsRandom_next(dsRandom* random);

/**
 * @brief Gets the next random value as a bool.
 * @param[inout] random The random number generator.
 * @return A random bool value of true or false.
 */
DS_MATH_EXPORT inline bool dsRandom_nextBool(dsRandom* random);

/**
 * @brief Gets the next random value as a uint32_t value up to a maximum.
 * @remark This avoids bias that would be encountered when taking the naive approach of
 *     dsRandom_next() % (maxValue + 1).
 * @param[inout] random The random number generator.
 * @param maxValue The maximum value.
 * @return A random value in the range of [0, maxValue].
 */
DS_MATH_EXPORT inline uint32_t dsRandom_nextUInt32(dsRandom* random, uint32_t maxValue);

/**
 * @brief Gets the next random value as a uint32_t value in a custom range.
 * @remark This avoids bias that would be encountered when taking the naive approach of
 *     dsRandom_next() % (maxValue - minValue + 1) + minValue.
 * @remark This will reliably return up to 24 bits of randomness.
 * @param[inout] random The random number generator.
 * @param minValue The minimum value.
 * @param maxValue The maximum value.
 * @return A random value in the range of [minValue, maxValue].
 */
DS_MATH_EXPORT inline uint32_t dsRandom_nextUInt32Range(dsRandom* random, uint32_t minValue,
	uint32_t maxValue);

/**
 * @brief Gets the next random value as an int32_t value in a custom range.
 * @remark This avoids bias that would be encountered when taking the naive approach of
 *     dsRandom_next() % (maxValue - minValue + 1) + minValue.
 * @param[inout] random The random number generator.
 * @param minValue The minimum value.
 * @param maxValue The maximum value.
 * @return A random value in the range of [minValue, maxValue].
 */
DS_MATH_EXPORT inline int32_t dsRandom_nextInt32Range(dsRandom* random, int32_t minValue,
	int32_t maxValue);

/**
 * @brief Gets the next random value as a uint64_t value up to a maximum.
 * @remark This avoids bias that would be encountered when taking the naive approach of
 *     dsRandom_next() % (maxValue + 1).
 * @param[inout] random The random number generator.
 * @param maxValue The maximum value.
 * @return A random value in the range of [0, maxValue].
 */
DS_MATH_EXPORT inline uint64_t dsRandom_nextUInt64(dsRandom* random, uint64_t maxValue);

/**
 * @brief Gets the next random value as a uint64_t value in a custom range.
 * @remark This avoids bias that would be encountered when taking the naive approach of
 *     dsRandom_next() % (maxValue - minValue + 1) + minVal.
 * @param[inout] random The random number generator.
 * @param minValue The minimum value.
 * @param maxValue The maximum value.
 * @return A random value in the range of [minValue, maxValue].
 */
DS_MATH_EXPORT inline uint64_t dsRandom_nextUInt64Range(dsRandom* random, uint64_t minValue,
	uint64_t maxValue);

/**
 * @brief Gets the next random value as an int64_t value in a custom range.
 * @remark This avoids bias that would be encountered when taking the naive approach of
 *     dsRandom_next() % (maxBound - minVal) + minVal.
 * @param[inout] random The random number generator.
 * @param minValue The minimum value.
 * @param maxValue The maximum value.
 * @return A random value in the range of [minValue, maxValue].
 */
DS_MATH_EXPORT inline int64_t dsRandom_nextInt64Range(dsRandom* random, int64_t minValue,
	int64_t maxValue);

/**
 * @brief Gets the next random value as a float in the range [0, 1).
 * @param[inout] random The random number generator.
 * @return A random float value in the range [0, 1).
 */
DS_MATH_EXPORT inline float dsRandom_nextFloat(dsRandom* random);

/**
 * @brief Gets the next random value as a float in the range (-1, 1).
 * @param[inout] random The random number generator.
 * @return A random float value in the range (-1, 1).
 */
DS_MATH_EXPORT inline float dsRandom_nextSignedFloat(dsRandom* random);

/**
 * @brief Gets the next random value as a float in a custom range.
 * @remark The final result may suffer from floating point roundoff error, possibly taking the
 *     value out of the desired range.
 * @param[inout] random The random number generator.
 * @param minValue The minimum value.
 * @param maxBound The maximum bounds that all values will be less than.
 * @return A random float value in the range [minValue, maxBound).
 */
DS_MATH_EXPORT inline float dsRandom_nextFloatRange(dsRandom* random, float minValue,
	float maxBound);

/**
 * @brief Gets the next random value as a float in a custom centered range.
 * @remark The final result may suffer from floating point roundoff error, possibly taking the
 *     value out of the desired range.
 * @param[inout] random The random number generator.
 * @param centerValue The value in the center of the range.
 * @param range The maximum bound distance to the left and right of centerValue.
 * @return A random float value in the range (centerValue - range, centerValue + range).
 */
DS_MATH_EXPORT inline float dsRandom_nextFloatCenteredRange(dsRandom* random, float centerValue,
	float range);

/**
 * @brief Gets the next random value as a double in the range [0, 1).
 * @param[inout] random The random number generator.
 * @return A random double value in the range [0, 1).
 */
DS_MATH_EXPORT inline double dsRandom_nextDouble(dsRandom* random);

/**
 * @brief Gets the next random value as a double in the range (-1, 1).
 * @param[inout] random The random number generator.
 * @return A random double value in the range (-1, 1).
 */
DS_MATH_EXPORT inline double dsRandom_nextSignedDouble(dsRandom* random);

/**
 * @brief Gets the next random value as a double in a custom range.
 * @remark The final result may suffer from floating point roundoff error, possibly taking the
 *     value out of the desired range.
 * @param[inout] random The random number generator.
 * @param minValue The minimum value.
 * @param maxBound The maximum bounds that all values will be less than.
 * @return A random double value in the range [minValue, maxBound).
 */
DS_MATH_EXPORT inline double dsRandom_nextDoubleRange(dsRandom* random, double minValue,
	double maxBound);

/**
 * @brief Gets the next random value as a double in a custom centered range.
 * @remark The final result may suffer from floating point roundoff error, possibly taking the
 *     value out of the desired range.
 * @param[inout] random The random number generator.
 * @param centerValue The value in the center of the range.
 * @param range The maximum bound distance to the left and right of centerValue.
 * @return A random double value in the range (centerValue - range, centerValue + range).
 */
DS_MATH_EXPORT inline double dsRandom_nextDoubleCenteredRange(dsRandom* random, double centerValue,
	double range);

DS_MATH_EXPORT inline uint64_t dsRandom_nextSeed(uint64_t* state)
{
	DS_ASSERT(state);
	*state += 0x9e3779b97f4a7c15ULL;
	uint64_t value = *state;
	value = (value ^ (value >> 30))*0xbf58476d1ce4e5b9ULL;
	value = (value ^ (value >> 27))*0x94d049bb133111ebULL;
	return value ^ (value >> 31);
}

DS_MATH_EXPORT inline uint64_t dsRandom_next(dsRandom* random)
{
/// @cond
#define DS_RANDOM_ROTL(x, k) (((x) << (k)) | ((x) >> (64 - (k))))
/// @endcond

	DS_ASSERT(random);
	uint64_t temp = random->state[0] + random->state[3];
	uint64_t next = DS_RANDOM_ROTL(temp, 23) + random->state[0];
	temp = random->state[1] << 17;

	random->state[2] ^= random->state[0];
	random->state[3] ^= random->state[1];
	random->state[1] ^= random->state[2];
	random->state[0] ^= random->state[3];

	random->state[2] ^= temp;
	random->state[3] = DS_RANDOM_ROTL(random->state[3], 45);

	return next;

#undef DS_RANDOM_ROTL
}

DS_MATH_EXPORT inline bool dsRandom_nextBool(dsRandom* random)
{
	return (bool)(dsRandom_next(random) & 1);
}

DS_MATH_EXPORT inline uint32_t dsRandom_nextUInt32(dsRandom* random, uint32_t maxValue)
{
	if (maxValue == 0)
		return 0;
	else if (maxValue == (uint32_t)-1)
		return (uint32_t)dsRandom_next(random);

	// Single debiased mod method from: https://www.pcg-random.org/posts/bounded-rands.html
	uint32_t maxBound = maxValue + 1;
	uint32_t difThreshold = -maxBound;
	uint32_t baseValue, result;
	do
	{
		baseValue = (uint32_t)dsRandom_next(random);
		result = baseValue % maxBound;
	} while (baseValue - result > difThreshold);
	return result;
}

DS_MATH_EXPORT inline uint32_t dsRandom_nextUInt32Range(dsRandom* random, uint32_t minValue,
	uint32_t maxValue)
{
	DS_ASSERT(minValue <= maxValue);
	return dsRandom_nextUInt32(random, maxValue - minValue) + minValue;
}

DS_MATH_EXPORT inline int32_t dsRandom_nextInt32Range(dsRandom* random, int32_t minValue,
	int32_t maxValue)
{
	DS_ASSERT(minValue <= maxValue);
	return dsRandom_nextUInt32(random, maxValue - minValue) + minValue;
}

DS_MATH_EXPORT inline uint64_t dsRandom_nextUInt64(dsRandom* random, uint64_t maxValue)
{
	if (maxValue == 0)
		return 0;
	else if (maxValue == (uint64_t)-1)
		return dsRandom_next(random);

	// Single debiased mod method from: https://www.pcg-random.org/posts/bounded-rands.html
	uint64_t maxBound = maxValue + 1;
	uint64_t difThreshold = -maxBound;
	uint64_t baseValue, result;
	do
	{
		baseValue = dsRandom_next(random);
		result = baseValue % maxBound;
	} while (baseValue - result > difThreshold);
	return result;
}

DS_MATH_EXPORT inline uint64_t dsRandom_nextUInt64Range(dsRandom* random, uint64_t minValue,
	uint64_t maxValue)
{
	DS_ASSERT(minValue <= maxValue);
	return dsRandom_nextUInt64(random, maxValue - minValue) + minValue;
}

DS_MATH_EXPORT inline int64_t dsRandom_nextInt64Range(dsRandom* random, int64_t minValue,
	int64_t maxValue)
{
	DS_ASSERT(minValue <= maxValue);
	return dsRandom_nextUInt64(random, maxValue - minValue) + minValue;
}

DS_MATH_EXPORT inline float dsRandom_nextFloat(dsRandom* random)
{
	return (float)(dsRandom_next(random) >> 40)*0x1.0p-24f;
}

DS_MATH_EXPORT inline float dsRandom_nextSignedFloat(dsRandom* random)
{
	uint64_t bits = dsRandom_next(random);
	float unsignedFloat = (float)(bits >> 40)*0x1.0p-24f;
	return bits & 1 ? -unsignedFloat : unsignedFloat;
}

DS_MATH_EXPORT inline float dsRandom_nextFloatRange(dsRandom* random, float minValue,
	float maxBound)
{
	DS_ASSERT(minValue <= maxBound);
	float range = maxBound - minValue;
	return dsRandom_nextFloat(random)*range + minValue;
}

DS_MATH_EXPORT inline float dsRandom_nextFloatCenteredRange(dsRandom* random, float centerValue,
	float range)
{
	DS_ASSERT(range >= 0);
	return dsRandom_nextSignedFloat(random)*range + centerValue;
}

DS_MATH_EXPORT inline double dsRandom_nextDouble(dsRandom* random)
{
	return (double)(dsRandom_next(random) >> 11)*0x1.0p-53;
}

DS_MATH_EXPORT inline double dsRandom_nextSignedDouble(dsRandom* random)
{
	uint64_t bits = dsRandom_next(random);
	double unsignedDouble = (double)(bits >> 11)*0x1.0p-53f;
	return bits & 1 ? -unsignedDouble : unsignedDouble;
}

DS_MATH_EXPORT inline double dsRandom_nextDoubleRange(dsRandom* random, double minValue,
	double maxBound)
{
	DS_ASSERT(minValue <= maxBound);
	double range = maxBound - minValue;
	return dsRandom_nextDouble(random)*range + minValue;
}

DS_MATH_EXPORT inline double dsRandom_nextDoubleCenteredRange(dsRandom* random, double centerValue,
	double range)
{
	DS_ASSERT(range >= 0);
	return dsRandom_nextSignedDouble(random)*range + centerValue;
}

#ifdef __cplusplus
}
#endif

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
#include <DeepSea/Math/Export.h>
#include <DeepSea/Math/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function to generate random numbers.
 * @remark These functions are designed to give fast results and good distribution, but aren't
 *     suitable for cryptographic uses.
 */

/**
 * @brief Creates a random seed.
 *
 * This uses a combination of a high frequency time and a counter to guarantee that two calls in
 * rapid succession will have different values.
 *
 * @return A random seed.
 */
DS_MATH_EXPORT uint32_t dsRandomSeed(void);

/**
 * @brief Calculates a random number between 1 and DS_RANDOM_MAX.
 *
 * This is equivalent to the minstd_rand type in the C++ random library.
 *
 * @param seed The seed for the random number generator. This will be updated to allow the next.
 * number to be different.
 */
DS_MATH_EXPORT uint32_t dsRandom(uint32_t* seed);

/**
 * @brief Returns a random double value.
 * @param seed The seed for the random number generator. This will be updated to allow the next.
 * @param minVal The minimum double value.
 * @param maxVal The maximum double value.
 * @return A random double value.
 */
DS_MATH_EXPORT double dsRandomDouble(uint32_t* seed, double minVal, double maxVal);

/**
 * @brief Returns a random float value.
 * @param seed The seed for the random number generator. This will be updated to allow the next.
 * @param minVal The minimum float value.
 * @param maxVal The maximum float value.
 * @return A random float value.
 */
DS_MATH_EXPORT float dsRandomFloat(uint32_t* seed, float minVal, float maxVal);

/**
 * @brief Returns a random int value.
 *
 * This tries to use the full range of random bits rather than just the bottom few.
 *
 * @param seed The seed for the random number generator. This will be updated to allow the next.
 * @param minVal The minimum int value.
 * @param maxVal The maximum int value.
 * @return A random int value.
 */
DS_MATH_EXPORT int dsRandomInt(uint32_t* seed, int minVal, int maxVal);

#ifdef __cplusplus
}
#endif

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
#include <DeepSea/Math/Export.h>
#include <DeepSea/Math/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function to generate random numbers.
 */

/**
 * @brief Calculates a random number between 1 and DS_RANDOM_MAX.
 *
 * This is equivalent to the minstd_rand type in the C++ random library.
 *
 * @param seed The seed for the random number generator. This will be updated to allow the next.
 * number to be different.
 */
DS_MATH_EXPORT uint32_t dsRandom(uint32_t* seed);

#ifdef __cplusplus
}
#endif

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
#include <math.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Core math functions and macros.
 */

/**
 * @brief Returns the minimum between x and y.
 * @param x The first value.
 * @param y The second value.
 * @return The minimum between x and y.
 */
#define dsMin(x, y) ((x) < (y) ? (x) : (y))

/**
 * @brief Returns the maximum between x and y.
 * @param x The first value.
 * @param y The second value.
 * @return The maximum between x and y.
 */
#define dsMax(x, y) ((x) > (y) ? (x) : (y))

#ifdef __cplusplus
}
#endif

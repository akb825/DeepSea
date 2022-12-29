/*
 * Copyright 2022 Aaron Barany
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
#include <DeepSea/Math/SIMD/SIMD.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the SIMD types used in the DeepSea/Math library.
 *
 * The types will only be defined if SIMD is possible on the current target.
 */

#if DS_HAS_SIMD

/**
 * @brief Struct defining four 4x4 matrices to be operated on simultaneously.
 */
typedef struct dsMatrix44x4f
{
	/**
	 * @brief The values of the matrix.
	 */
	dsSIMD4f values[4][4];
} dsMatrix44x4f;

#endif // DS_HAS_SIMD

#ifdef __cplusplus
}
#endif

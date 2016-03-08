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

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Macros and functions for core memory constants and operations.
 */

/**
 * @brief The alignment of allocated memory.
 *
 * All allocators should ensure that memory is aligned to this amount.
 */
#define DS_ALLOC_ALIGNMENT 16

/**
 * @brief Attributes a type to use a specific alignment.
 * @param x The alignment to use. This will most commonly be 16.
 */
#ifdef _MSC_VER
#define DS_ALIGN(x) __declspec(align(x))
#else
#define DS_ALIGN(x) __attribute__((aligned(x)))
#endif

/**
 * @brief Gets the aligned size of an object.
 *
 * This can be used to calculate the amount of space to allocate to preserve alignment according to
 * DS_ALLOC_ALIGNMENT. For example, when pre-allocating a pool of memory for a group of objects
 * and advancing the memory pointer for each object.
 *
 * @param x The original size.
 * @return The aligned size.
 */
#define DS_ALIGNED_SIZE(x) ((((x) + DS_ALLOC_ALIGNMENT - 1)/DS_ALLOC_ALIGNMENT)*DS_ALLOC_ALIGNMENT)

#ifdef __cplusplus
}
#endif

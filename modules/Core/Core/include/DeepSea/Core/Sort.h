/*
 * Copyright 2018-2023 Aaron Barany
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
#include <DeepSea/Core/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for sorting and working with sortad array.
 */

/**
 * @brief Compares two values.
 * @param a The first value to comapre.
 * @param b The second value to compare.
 * @return -1 if a < b, 1 if a > b, or 0 if a == b.
 */
#define DS_CMP(a, b) (((a) > (b)) - ((a) < (b)))

/**
 * @brief Combines two compare values.
 * @param a The first comparison value.
 * @param b The second comparison value.
 * @return a if a != 0, otherwise b.
 */
DS_CORE_EXPORT inline int dsCombineCmp(int a, int b);

/**
 * @brief Performs a qsort on an array with a context pointer.
 *
 * This is equivalent to qsort_r() on Linux, but is generalized to work properly with other
 * platforms that have equivalent but slihgtly different functions.
 *
 * @param array The array to sort.
 * @param memberCount The number of members.
 * @param memberSize The size of each member.
 * @param compareFunc The comparison function.
 * @param context The context to provide with the comapre function.
 */
DS_CORE_EXPORT void dsSort(void* array, size_t memberCount, size_t memberSize,
	dsSortCompareFunction compareFunc, void* context);

/**
 * @brief Performs a binary search on a sorted array.
 * @param key The key to search for.
 * @param array The array to search in.
 * @param memberCount The number of members.
 * @param memberSize The size of each member.
 * @param compareFunc The comparison function. The "left" parameter is the key.
 * @param context The context to provide with the comapre function.
 * @return A pointer to a member that equals key or NULL if not found.
 */
DS_CORE_EXPORT const void* dsBinarySearch(const void* key, const void* array, size_t memberCount,
	size_t memberSize, dsSortCompareFunction compareFunc, void* context);

/**
 * @brief Performs a binary search to find a lower bound on a sorted array.
 * @param key The key to search for.
 * @param array The array to search in.
 * @param memberCount The number of members.
 * @param memberSize The size of each member.
 * @param compareFunc The comparison function. The "left" parameter is the key.
 * @param context The context to provide with the comapre function.
 * @return A pointer to the first element that is not less than the key, or NULL if all elelments
 *     are less than the key.
 */
DS_CORE_EXPORT const void* dsBinarySearchLowerBound(const void* key, const void* array,
	size_t memberCount, size_t memberSize, dsSortCompareFunction compareFunc, void* context);

/**
 * @brief Performs a binary search to find an upper bound on a sorted array.
 * @param key The key to search for.
 * @param array The array to search in.
 * @param memberCount The number of members.
 * @param memberSize The size of each member.
 * @param compareFunc The comparison function. The "left" parameter is the key.
 * @param context The context to provide with the comapre function.
 * @return The last element that's not greater than the key, or NULL if all elements are greater
 *     than the key.
 */
DS_CORE_EXPORT const void* dsBinarySearchUpperBound(const void* key, const void* array,
	size_t memberCount, size_t memberSize, dsSortCompareFunction compareFunc, void* context);

DS_CORE_EXPORT inline int dsCombineCmp(int a, int b)
{
	return a | (((a != 0) - 1) & b);
}

#ifdef __cplusplus
}
#endif

/*
 * Copyright 2018 Aaron Barany
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
 * @return A pointer to a member that equals key, or NULL if not found.
 */
DS_CORE_EXPORT void* dsBinarySearch(const void* key, const void* array, size_t memberCount,
	size_t memberSize, dsSortCompareFunction compareFunc, void* context);

/**
 * @brief Performs a binary search to find a lower bound on a sorted array.
 * @param key The key to search for.
 * @param array The array to search in.
 * @param memberCount The number of members.
 * @param memberSize The size of each member.
 * @param compareFunc The comparison function. The "left" parameter is the key.
 * @param context The context to provide with the comapre function.
 * @return A pointer to the first element that is not less than the key, or NULL if there is no such
 *     element.
 */
DS_CORE_EXPORT void* dsBinarySearchLowerBound(const void* key, const void* array,
	size_t memberCount, size_t memberSize, dsSortCompareFunction compareFunc, void* context);

/**
 * @brief Performs a binary search to find an upper bound on a sorted array.
 * @param key The key to search for.
 * @param array The array to search in.
 * @param memberCount The number of members.
 * @param memberSize The size of each member.
 * @param compareFunc The comparison function. The "left" parameter is the key.
 * @param context The context to provide with the comapre function.
 * @return The first element that's > the key, or NULL if no element is > the key.
 */
DS_CORE_EXPORT void* dsBinarySearchUpperBound(const void* key, const void* array,
	size_t memberCount, size_t memberSize, dsSortCompareFunction compareFunc, void* context);

#ifdef __cplusplus
}
#endif

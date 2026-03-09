/*
 * Copyright 2026 Aaron Barany
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

#include <DeepSea/Core/Streams/Types.h>
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating view filters.
 * @see dsViewFilter
 */

/**
 * @brief Gets the size of dsViewFilter.
 * @return The size of dsViewFilter.
 */
DS_SCENE_EXPORT size_t dsViewFilter_sizeof(void);

/**
 * @brief Gets the full allocation size of dsViewFilter.
 * @param viewNameCount The number of view names used in the filter.
 * @return The full allocation size of dsViewFilter.
 */
DS_SCENE_EXPORT size_t dsViewFilter_fullAllocSize(uint32_t viewNameCount);

/**
 * @brief Creates a view filter.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the view filter with.
 * @param viewNames The names of the views to use for the filter.
 * @param viewNameCount The number of view names.
 * @param invert Whether to invert the filter, such that all views except those within viewNames
 *     will pass the filter.
 */
DS_SCENE_EXPORT dsViewFilter* dsViewFilter_create(
	dsAllocator* allocator, const char* const* viewNames, uint32_t viewNameCount, bool invert);

/**
 * @brief Checks whether a filter contains a view by name.
 * @param filter The view filter. This may be NULL, in which case all non-NULL names are accepted.
 * @param name The view name.
 * @return Whether the view is accepted by the filter.
 */
DS_SCENE_EXPORT bool dsViewFilter_containsName(const dsViewFilter* filter, const char* name);

/**
 * @brief Checks whether a filter contains a view by ID.
 * @param filter The view filter. This may be NULL, in which case all non-0 IDs are accepted.
 * @param nameID The view name ID.
 * @return Whether the view is accepted by the filter.
 */
DS_SCENE_EXPORT bool dsViewFilter_containsID(const dsViewFilter* filter, uint32_t nameID);

/**
 * @brief Gets the hash for a view filter.
 * @param filter The filter to hash.
 * @param commonHash The hash to combine with the hash of the filter.
 * @return The hash value.
 */
DS_SCENE_EXPORT uint32_t dsViewFilter_hash(const dsViewFilter* filter, uint32_t commonHash);

/**
 * @brief Checks whether two view filters are equal.
 * @param left The left hand side.
 * @param right The right hand side.
 * @return Whether left and right are equal.
 */
DS_SCENE_EXPORT bool dsViewFilter_equal(const dsViewFilter* left, const dsViewFilter* right);

/**
 * @brief Destroys a view filter.
 * @param filter The filter to destroy.
 */
DS_SCENE_EXPORT void dsViewFilter_destroy(dsViewFilter* filter);

#ifdef __cplusplus
}
#endif

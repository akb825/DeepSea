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
#include <DeepSea/Core/Types.h>
#include <DeepSea/Geometry/Export.h>
#include <DeepSea/Geometry/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating bounding volume hierarchies.
 *
 * A BVH will use a hierarchy of axis-aligned bounding boxes. This can be used with 2 or 3
 * dimentional bounds of float, double, or int. (i.e. dsAlignedBox[23][fdi]) This allows a spacial
 * lookup of objects in O(log(n)) time in the average case.
 *
 * Balancing may optionally be performed at build time to create more optimal intermediate bounds
 * and improve lookup times. However, this will increased the time to build the BVH from O(log(n))
 * to roughly O(n^2*log(n)). In cases where data is randomly distributed, not balancing may cause
 * lookup performance to degrade to O(n), in which case the extra time spent balancing may be
 * quickly made up with better lookup times.
 *
 * @see dsBVH
 */

/**
 * @brief Creates a BVH.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the BVH with. This must support freeing memory.
 * @param axisCount The number of axes for the bounding boxes. This must be 2 or 3.
 * @param element The type for each bounds element.
 * @param userData User data associated with the BVH.
 * @return The created BVH, or NULL if it couldn't be created.
 */
DS_GEOMETRY_EXPORT dsBVH* dsBVH_create(dsAllocator* allocator, uint8_t axisCount,
	dsGeometryElement element, void* userData);

/**
 * @brief Gets the number of axes for the bounds within a BVH.
 * @param bvh The BVH.
 * @return The number of axes.
 */
DS_GEOMETRY_EXPORT uint8_t dsBVH_getAxisCount(const dsBVH* bvh);

/**
 * @brief Gets the type for the bounds elements within a BVH.
 * @param bvh The BVH.
 * @return The type of each bounds element.
 */
DS_GEOMETRY_EXPORT dsGeometryElement dsBVH_getElement(const dsBVH* bvh);

/**
 * @brief Gets the user data for the BVH.
 * @param bvh The BVH.
 * @return The user data.
 */
DS_GEOMETRY_EXPORT void* dsBVH_getUserData(const dsBVH* bvh);

/**
 * @brief Sets the user data for the BVH.
 * @param bvh The BVH.
 * @param userData The user data.
 */
DS_GEOMETRY_EXPORT void dsBVH_setUserData(dsBVH* bvh, void* userData);

/**
 * @brief Builds the hierarchy for a BVH.
 *
 * This will replace the contents of the BVH.
 *
 * @remark errno will be set on failure.
 * @param bvh The BVH to build.
 * @param objects An array of objects to build the BVH for. This must remain alive as long as the
 *     BVH remains built with this data. If DS_GEOMETRY_OBJECT_POINTERS is used, then this is
 *     instead an array of pointers to objects, and the objects must remain alive as opposed to the
 *     array.
 * @param objectCount The number of objects in the array.
 * @param objectSize The size of each object inside of the object array. Two special values can be
 *     used to adjust behavior of the structure:
 *     - DS_GEOMETRY_OBJECT_POINTERS: indicates that the objects array is an array of pointers to
 *       the objects. This will store the pointer at each index rather than the offset into the
 *       array.
 *     - DS_GEOMETRY_OBJECT_INDICES: indicates to store an index to the object instead of a pointer
 *       to the object. The void* provided for each object should be cast to size_t. When this is
 *       used, it is valid for the objects array to be NULL.
 * @param objectBoundsFunc The function to query the bounds from each object.
 * @param balance True to balance the nodes within the BVH. This will improve lookup times, but will
 *     increase the cost of building the BVH.
 * @return False if an error occurred. The current contents will be cleared on error.
 */
DS_GEOMETRY_EXPORT bool dsBVH_build(dsBVH* bvh, const void* objects, uint32_t objectCount,
	size_t objectSize, dsBVHObjectBoundsFunction objectBoundsFunc, bool balance);

/**
 * @brief Updates a BVH, querying updated bounds from the objects and updating the bounds
 * accordingly.
 *
 * This will keep the topology of the BVH the same while updating the internal bounds. If the
 * objects move around enough, the tree may become unbalanced, causing lookups to become unbalanced.
 * Consider re-building the BVH if the objects move significantly with respect to each-other if you
 * want to maintain a balanced tree.
 *
 * @remark errno will be set on failure.
 * @param bvh The BVH to balance.
 * @return False if an error occurred.
 */
DS_GEOMETRY_EXPORT bool dsBVH_update(dsBVH* bvh);

/**
 * @brief Intersects a bounding box with the BVH.
 * @param bvh The BVH to intersect.
 * @param bounds The bounds to intersect. The type should be a dsAlignedBox* type appropriate for
 *     the axis count and precision.
 * @param visitor A visitor function to call for each intersecting object. This may be NULL if you
 *     only want to know how many objects intersect.
 * @param userData User data to pass to the visitor function.
 * @return The number of objects that intersected.
 */
DS_GEOMETRY_EXPORT uint32_t dsBVH_intersect(const dsBVH* bvh, const void* bounds,
	dsBVHVisitFunction visitor, void* userData);

/**
 * @brief Gets the bounds of the BVH.
 * @param outBounds The bounds. The type should be a dsAlignedBox* type appropriate for the axis
 *     count and precision.
 * @param bvh The BVH to get the bounds from.
 * @return True if the bounds are set, false if the BVH is empty or invalid.
 */
DS_GEOMETRY_EXPORT bool dsBVH_getBounds(void* outBounds, const dsBVH* bvh);

/**
 * @brief Clears the contents of the BVH.
 *
 * Internal memory will remain allocated to re-use for future calls to dsBVH_build().
 *
 * @param bvh The BVH to clear.
 */
DS_GEOMETRY_EXPORT void dsBVH_clear(dsBVH* bvh);

/**
 * @brief Destroyes a BVH.
 * @param bvh The BVH to destroy.
 */
DS_GEOMETRY_EXPORT void dsBVH_destroy(dsBVH* bvh);

#ifdef __cplusplus
}
#endif

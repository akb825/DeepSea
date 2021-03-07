/*
 * Copyright 2018-2021 Aaron Barany
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
 * @brief Functions for creating and manipulating Kd trees.
 *
 * A Kd tree will create separating planes between the provided data points. This can be used with 2
 * or 3 dimensional points of float, double, or int. (i.e. dsVector[23][fdi]) This allows a spacial
 * lookup of objects in O(log(n)) time in the average case.
 *
 * @see dsKdTree
 */

/**
 * @brief Creates a Kd tree.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the Kd tree with. This must support freeing memory.
 * @param axisCount The number of axes for the points. This must be 2 or 3.
 * @param element The type for each vector element.
 * @param userData User data associated with the Kd tree.
 * @return The created Kd tree, or NULL if it couldn't be created.
 */
DS_GEOMETRY_EXPORT dsKdTree* dsKdTree_create(dsAllocator* allocator, uint8_t axisCount,
	dsGeometryElement element, void* userData);

/**
 * @brief Gets the number of axes for the bounds within a Kd tree.
 * @param kdTree The Kd tree.
 * @return The number of axes.
 */
DS_GEOMETRY_EXPORT uint8_t dsKdTree_getAxisCount(const dsKdTree* kdTree);

/**
 * @brief Gets the type for the bounds elements within a Kd tree.
 * @param kdTree The Kd tree.
 * @return The type of each bounds element.
 */
DS_GEOMETRY_EXPORT dsGeometryElement dsKdTree_getElement(const dsKdTree* kdTree);

/**
 * @brief Gets the user data for the Kd tree.
 * @param kdTree The Kd tree.
 * @return The user data.
 */
DS_GEOMETRY_EXPORT void* dsKdTree_getUserData(const dsKdTree* kdTree);

/**
 * @brief Sets the user data for the Kd tree.
 * @param kdTree The Kd tree.
 * @param userData The user data.
 */
DS_GEOMETRY_EXPORT void dsKdTree_setUserData(dsKdTree* kdTree, void* userData);

/**
 * @brief Builds the tree for the Kd tree.
 *
 * This will replace the contents of the Kd tree.
 *
 * @remark errno will be set on failure.
 * @param kdTree The Kd tree to build.
 * @param objects An array of objects to build the Kd tree for. This must remain alive as long as
 *     the Kd tree remains built with this data. If DS_GEOMETRY_OBJECT_POINTERS is used, then this
 *     is instead an array of pointers to objects, and the objects must remain alive as opposed to
 *     the array.
 * @param objectCount The number of objects in the array.
 * @param objectSize The size of each object inside of the object array. Two special values can be
 *     used to adjust behavior of the structure:
 *     - DS_GEOMETRY_OBJECT_POINTERS: indicates that the objects array is an array of pointers to
 *       the objects. This will store the pointer at each index rather than the offset into the
 *       array.
 *     - DS_GEOMETRY_OBJECT_INDICES: indicates to store an index to the object instead of a pointer
 *       to the object. The void* provided for each object should be cast to size_t. When this is
 *       used, it is valid for the objects array to be NULL.
 * @param objectPointFunc The function to query the point from each object.
 * @param balance True to balance the nodes within the Kd tree. This will improve lookup times, but
 *     will increase the cost of building the Kd tree.
 * @return False if an error occurred. The current contents will be cleared on error.
 */
DS_GEOMETRY_EXPORT bool dsKdTree_build(dsKdTree* kdTree, const void* objects, uint32_t objectCount,
	size_t objectSize, dsKdTreeObjectPointFunction objectPointFunc, bool balance);

/**
 * @brief Finds the object nearest to a point.
 * @param kdTree The Kd tree to search in.
 * @param point The point to find the nearest neighbor to.
 * @return The nearest neighbor, or NULL if the Kd tree is empty or the parameters are invalid.
 */
DS_GEOMETRY_EXPORT const void*  dsKdTree_nearestNeighbor(const dsKdTree* kdTree, const void* point);

/**
 * @brief Traverses a Kd tree.
 * @remark errno will be set on failure.
 * @param kdTree The Kd tree to traverse.
 * @param traverseFunc The traversal function.
 * @param userData User data to pass to the traversal function. This will typically contain a point
 *     to compare with.
 * @return False if an error occurred.
 */
DS_GEOMETRY_EXPORT bool dsKdTree_traverse(const dsKdTree* kdTree,
	dsKdTreeTraverseFunction traverseFunc, void* userData);

/**
 * @brief Clears the contents of the Kd tree.
 *
 * Internal memory will remain allocated to re-use for future calls to dsKdTree_build().
 *
 * @param kdTree The Kd tree to clear.
 */
DS_GEOMETRY_EXPORT void dsKdTree_clear(dsKdTree* kdTree);

/**
 * @brief Destroyes a Kd tree.
 * @param kdTree The Kd tree to destroy.
 */
DS_GEOMETRY_EXPORT void dsKdTree_destroy(dsKdTree* kdTree);

#ifdef __cplusplus
}
#endif

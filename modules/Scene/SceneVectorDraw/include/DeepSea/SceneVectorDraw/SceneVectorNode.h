/*
 * Copyright 2020-2022 Aaron Barany
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
#include <DeepSea/SceneVectorDraw/Export.h>
#include <DeepSea/SceneVectorDraw/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manipulate vector nodes.
 * @see dsSceneVectorNode
 */

/**
 * @brief Gets the type of a vector node.
 * @return The type of a vector node.
 */
DS_SCENEVECTORDRAW_EXPORT const dsSceneNodeType* dsSceneVectorNode_type(void);

/**
 * @brief Creates a vector node as a base class of another node type.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the vector node with.
 * @param structSize The size of the struct.
 * @param z The Z value used for sorting vector nodes.
 * @param itemLists List of item list names to add the node to. The array will be copied.
 * @param itemListCount The number of item lists.
 * @param resources The resources to keep a reference to.
 * @param resourceCount The number of resources.
 * @return The vector node or NULL if an error occurred.
 */
DS_SCENEVECTORDRAW_EXPORT dsSceneVectorNode* dsSceneVectorNode_create(dsAllocator* allocator,
	size_t structSize, int32_t z, const char* const* itemLists, uint32_t itemListCount,
	dsSceneResources** resources, uint32_t resourceCount);

/**
 * @brief Destroys a vector node.
 * @remark This should only be called as part of a subclass' destroy function.
 * @param node The node to destroy.
 */
DS_SCENEVECTORDRAW_EXPORT void dsSceneVectorNode_destroy(dsSceneNode* node);

#ifdef __cplusplus
}
#endif

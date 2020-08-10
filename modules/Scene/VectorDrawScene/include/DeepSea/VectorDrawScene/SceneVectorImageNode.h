/*
 * Copyright 2020 Aaron Barany
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
#include <DeepSea/VectorDrawScene/Export.h>
#include <DeepSea/VectorDrawScene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manipulate vector image nodes.
 * @see dsSceneVectorImageNode
 */

/**
 * @brief The type name for a vector image node.
 */
DS_VECTORDRAWSCENE_EXPORT extern const char* const dsSceneVectorImageNode_typeName;

/**
 * @brief Gets the type of a vector image node.
 * @return The type of a vector image node.
 */
DS_VECTORDRAWSCENE_EXPORT const dsSceneNodeType* dsSceneVectorImageNode_type(void);

/**
 * @brief Sets up the parent type for a node type subclassing from dsSceneVectorImageNode.
 * @param type The subclass type for dsSceneVectorImageNode.
 * @return The type parameter or the type for dsSceneVectorImageNode if type is NULL.
 */
DS_VECTORDRAWSCENE_EXPORT const dsSceneNodeType* dsSceneVectorImageNode_setupParentType(
	dsSceneNodeType* type);

/**
 * @brief Creates a vector image node.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the vector image node with.
 * @param vectorImage The vector image to draw.
 * @param z The Z value used for sorting vector nodes.
 * @param shaders The vector shaders to draw with.
 * @param material The material to draw with.
 * @param itemLists List of item list names to add the node to. The array will be copied.
 * @param itemListCount The number of item lists.
 * @param resources The resources to keep a reference to.
 * @param resourceCount The number of resources.
 * @return The vector image node or NULL if an error occurred.
 */
DS_VECTORDRAWSCENE_EXPORT dsSceneVectorImageNode* dsSceneVectorImageNode_create(
	dsAllocator* allocator, dsVectorImage* vectorImage, int32_t z, const dsVectorShaders* shaders,
	dsMaterial* material, const char** itemLists, uint32_t itemListCount,
	dsSceneResources** resources, uint32_t resourceCount);

/**
 * @brief Creates a vector image node as a base class of another node type.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the model node with.
 * @param structSize The size of the struct.
 * @param z The Z value used for sorting vector nodes.
 * @param vectorImage The vector image to draw.
 * @param shaders The vector shaders to draw with.
 * @param material The material to draw with.
 * @param itemLists List of item list names to add the node to. The array will be copied.
 * @param itemListCount The number of item lists.
 * @param resources The resources to keep a reference to.
 * @param resourceCount The number of resources.
 * @return The vector image node or NULL if an error occurred.
 */
DS_VECTORDRAWSCENE_EXPORT dsSceneVectorImageNode* dsSceneVectorImageNode_createBase(
	dsAllocator* allocator, size_t structSize, dsVectorImage* vectorImage, int32_t z,
	const dsVectorShaders* shaders, dsMaterial* material, const char** itemLists,
	uint32_t itemListCount, dsSceneResources** resources, uint32_t resourceCount);

#ifdef __cplusplus
}
#endif

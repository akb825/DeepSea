/*
 * Copyright 2019 Aaron Barany
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
#include <DeepSea/Scene/Nodes/Types.h>
#include <DeepSea/Scene/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating model nodes.
 * @see dsSceneModelNode
 */

/**
 * @brief The type name for a model node.
 */
DS_SCENE_EXPORT extern const char* const dsSceneModelNode_typeName;

/**
 * @brief The type name for a model node when performing a clone with remapping materials.
 *
 * This is used to determine which loader implementation to use when loading from file.
 */
DS_SCENE_EXPORT extern const char* const dsSceneModelNode_remapTypeName;

/**
 * @brief The type name for a model node when performing a clone with re-configuring the layout.
 *
 * This is used to determine which loader implementation to use when loading from file.
 */
DS_SCENE_EXPORT extern const char* const dsSceneModelNode_reconfigTypeName;

/**
 * @brief Gets the type of a model node.
 * @return The type of a model node.
 */
DS_SCENE_EXPORT const dsSceneNodeType* dsSceneModelNode_type(void);

/**
 * @brief Creates a scene model node.
 *
 * @remark The same item list name may appear multiple times in the model list, but the extra item
 * list is expected to contain unique names.
 *
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the model node with.
 * @param models The models to draw within the node. The array will be copied. It is expected that
 *     at least one model is provided.
 * @param modelCount The number of models.
 * @param extraItemLists List of item list names to add the node to. This is in addition to the list
 *     of draw lists from the models array, such as for cull lists. The array will be copied.
 * @param extraItemListCount The number of item lists.
 * @param resources The resources to keep a reference to.
 * @param resourceCount The number of resources.
 * @param bounds The bounding box for the model. If NULL, the model will never be culled.
 * @return The model node or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneModelNode* dsSceneModelNode_create(dsAllocator* allocator,
	const dsSceneModelInitInfo* models, uint32_t modelCount, const char* const* extraItemLists,
	uint32_t extraItemListCount, dsSceneResources** resources, uint32_t resourceCount,
	const dsOrientedBox3f* bounds);

/**
 * @brief Creates a scene node as a base class of another node type.
 *
 * @remark The same item list name may appear multiple times in the model list, but the extra item
 * list is expected to contain unique names.
 *
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the model node with.
 * @param structSize The size of the struct.
 * @param models The models to draw within the node. The array will be copied. It is expected that
 *     at least one model is provided.
 * @param modelCount The number of models.
 * @param extraItemLists List of item list names to add the node to. This is in addition to the list
 *     of draw lists from the models array, such as for cull lists. The array will be copied.
 * @param extraItemListCount The number of item lists.
 * @param resources The resources to keep a reference to.
 * @param resourceCount The number of resources.
 * @param bounds The bounding box for the model. If NULL, the model will never be culled.
 * @return The model node or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneModelNode* dsSceneModelNode_createBase(dsAllocator* allocator,
	size_t structSize, const dsSceneModelInitInfo* models, uint32_t modelCount,
	const char* const* extraItemLists, uint32_t extraItemListCount, dsSceneResources** resources,
	uint32_t resourceCount, const dsOrientedBox3f* bounds);

/**
 * @brief Clones a model node, optionally remapping the materials.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the model node with.
 * @param origModel The existing model to clone.
 * @param remaps The materials to remap.
 * @param remapCount The number of material remaps.
 * @return The cloned model or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneModelNode* dsSceneModelNode_cloneRemap(dsAllocator* allocator,
	const dsSceneModelNode* origModel, const dsSceneMaterialRemap* remaps, uint32_t remapCount);

/**
 * @brief Clones a model node as the base class for another model type, optionally remapping the
 *     materials.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the model node with.
 * @param structSize The size of the struct.
 * @param origModel The existing model to clone.
 * @param remaps The materials to remap.
 * @param remapCount The number of material remaps.
 * @return The cloned model or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneModelNode* dsSceneModelNode_cloneRemapBase(dsAllocator* allocator,
	size_t structSize, const dsSceneModelNode* origModel, const dsSceneMaterialRemap* remaps,
	uint32_t remapCount);

/**
 * @brief Clones a model node, re-configuring the layout based on the original geometry.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the model node with.
 * @param origModel The existing model to clone.
 * @param models The new configuration of models. All model names in the reconfig list must be
 *     available in origModel, re-using the geometry and draw ranges while using the new shader,
 *     material, item list, and draw range.
 * @param modelCount The number of models.
 * @param extraItemLists List of item list names to add the node to. This is in addition to the list
 *     of draw lists from the models array, such as for cull lists. The array will be copied.
 * @param extraItemListCount The number of item lists.
 * @return The cloned model or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneModelNode* dsSceneModelNode_cloneReconfig(dsAllocator* allocator,
	const dsSceneModelNode* origModel, const dsSceneModelReconfig* models, uint32_t modelCount,
	const char* const* extraItemLists, uint32_t extraItemListCount);

/**
 * @brief Clones a model node as the base class for another model type, re-configuring the layout
 *    based on the original geometry.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the model node with.
 * @param structSize The size of the struct.
 * @param origModel The existing model to clone.
 * @param models The new configuration of models. All model names in the reconfig list must be
 *     available in origModel, re-using the geometry and draw ranges while using the new shader,
 *     material, item list, and draw range.
 * @param modelCount The number of models.
 * @param extraItemLists List of item list names to add the node to. This is in addition to the list
 *     of draw lists from the models array, such as for cull lists. The array will be copied.
 * @param extraItemListCount The number of item lists.
 * @return The cloned model or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneModelNode* dsSceneModelNode_cloneReconfigBase(dsAllocator* allocator,
	size_t structSize, const dsSceneModelNode* origModel, const dsSceneModelReconfig* models,
	uint32_t modelCount, const char* const* extraItemLists, uint32_t extraItemListCount);

/**
 * @brief Remaps the materials for a model.
 * @remark errno will be set on failure.
 * @param node The node to remap the materials on.
 * @param remaps The materials to remap.
 * @param remapCount The number of material remaps.
 * @return False if the parameters are invalid.
 */
DS_SCENE_EXPORT bool dsSceneModelNode_remapMaterials(dsSceneModelNode* node,
	const dsSceneMaterialRemap* remaps, uint32_t remapCount);

/**
 * @brief Destroys a model node.
 * @remark This should only be called as part of a subclass' destroy function, never to explicitly
 *     a model node instance since nodes are reference counted.
 * @param node The node to destroy.
 */
DS_SCENE_EXPORT void dsSceneModelNode_destroy(dsSceneNode* node);

#ifdef __cplusplus
}
#endif

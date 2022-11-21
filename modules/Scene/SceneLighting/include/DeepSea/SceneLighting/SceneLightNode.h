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
#include <DeepSea/Scene/Types.h>
#include <DeepSea/SceneLighting/Export.h>
#include <DeepSea/SceneLighting/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating manipulating scene light nodes.
 * @see dsSceneLightNode
 */

/**
 * @brief The type name for a scene light node.
 */
DS_SCENELIGHTING_EXPORT extern const char* const dsSceneLightNode_typeName;

/**
 * @brief Gets the type of a scene light node.
 * @return The type of a scene light node.
 */
DS_SCENELIGHTING_EXPORT const dsSceneNodeType* dsSceneLightNode_type(void);

/**
 * @brief Creates a scene light node.
 * @remark errno will be set on failure.
 * @param allocator The allocator for the node.
 * @param templateLight The light used as a template for all the lights created in the scene. The
 *     nameID field will be ignored. This will be copied.
 * @param lightBaseName The base name for the lights added to the scene. The lights will have ".n"
 *     appended to the name, where n is an index incremented for new instances. This will be copied.
 * @param singleInstance Whether or not one instance is allowed. If true, lightBaseName will be used
 *     as-is without any suffix.
 * @param itemLists The list of item list names that will be used to process the node. These will be
 *     copied.
 * @param itemListCount The number of item lists.
 */
DS_SCENELIGHTING_EXPORT dsSceneLightNode* dsSceneLightNode_create(dsAllocator* allocator,
	const dsSceneLight* templateLight, const char* lightBaseName, bool singleInstance,
	const char* const* itemLists, uint32_t itemListCount);

/**
 * @brief Gets the template light for the light node.
 *
 * This may be modified, though this will only affect lights that are created afterward when
 * changing the scene graph structure.
 *
 * @remark errno will be set on failure.
 * @param lightNode The scene light node.
 * @return The template light or NULL if lightNode is NULL.
 */
DS_SCENELIGHTING_EXPORT const dsSceneLight* dsSceneLightNode_getTemplateLight(
	const dsSceneLightNode* lightNode);

/**
 * @brief Gets the template light for the light node.
 *
 * This may be modified, though this will only affect lights that are created afterward when
 * changing the scene graph structure.
 *
 * @remark errno will be set on failure.
 * @param lightNode The scene light node.
 * @return The template light or NULL if lightNode is NULL.
 */
DS_SCENELIGHTING_EXPORT dsSceneLight* dsSceneLightNode_getMutableTemplateLight(
	dsSceneLightNode* lightNode);

/**
 * @brief Gets the base name for the lights added to the light set.
 *
 * Unless created with singleInstance to true, the lights will have ".n" appended to the name, where
 * n is an index incremented for new instances.
 *
 * @remark errno will be set on failure.
 * @param lightNode The scene light node.
 * @return The base name for the lights or NULL if lightNode is NULL.
 */
DS_SCENELIGHTING_EXPORT const char* dsSceneLightNode_getLightBaseName(
	const dsSceneLightNode* lightNode);

/**
 * @brief Gets whether or not the light node only allows a single instance.
 *
 * If true, the light base name will be used as-is without any suffix. Attempting to have more
 * instances in the scene graph will result in an error when adding the light.
 *
 * @param lightNode The scene light node.
 * @return Whether or not only a single instance is allowed.
 */
DS_SCENELIGHTING_EXPORT bool dsSceneLightNode_getSingleInstance(const dsSceneLightNode* lightNode);

/**
 * @brief Gets the light for a scene light node.
 * @remark This assumes that the light was created from a dsSceneLightSetPrepare.
 * @param treeNode The tree node to get the light for.
 * @return The light or NULL if there isn't one present.
 */
DS_SCENELIGHTING_EXPORT dsSceneLight* dsSceneLightNode_getLightForInstance(
	const dsSceneTreeNode* treeNode);

#ifdef __cplusplus
}
#endif

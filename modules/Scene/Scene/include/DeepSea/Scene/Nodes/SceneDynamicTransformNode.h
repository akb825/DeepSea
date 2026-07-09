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
#include <DeepSea/Scene/Nodes/Types.h>
#include <DeepSea/Scene/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating dynamic transform nodes.
 * @see dsSceneDynamicTransformNode
 */

/**
 * @brief The type name for a dynamic transform node.
 */
DS_SCENE_EXPORT extern const char* const dsSceneDynamicTransformNode_typeName;

/**
 * @brief Gets the type of a dynamic transform node.
 * @return The type of a dynamic transform node.
 */
DS_SCENE_EXPORT const dsSceneNodeType* dsSceneDynamicTransformNode_type(void);

/**
 * @brief Creates a transform node.
 * @remark errno will be set on failure.
 * @param allocator The allocator for the node. This must support freeing memory.
 * @param transform The initial transform for the node. Set to NULL to use the identity transform.
 * @param itemLists The list of item list names that will be used to process the node. These will be
 *     copied.
 * @param itemListCount The number of item lists.
 * @return The transform node or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneDynamicTransformNode* dsSceneDynamicTransformNode_create(
	dsAllocator* allocator, const dsRigidTransform3f* transform, const char* const* itemLists,
	uint32_t itemListCount);

/**
 * @brief Sets the transform node's rigid transform.
 *
 * This will assign the transform and mark the node as dirty.
 *
 * @remark errno will be set on failure.
 * @param node The node to set the transform on.
 * @param transform The new transform.
 * @return False if the parameters are invalid.
 */
DS_SCENE_EXPORT bool dsSceneDynamicTransformNode_setTransform(
	dsSceneDynamicTransformNode* node, const dsRigidTransform3f* transform);

#ifdef __cplusplus
}
#endif

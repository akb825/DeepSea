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
#include <DeepSea/Scene/Export.h>
#include <DeepSea/Scene/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating transform nodes.
 * @see dsTransformNode
 */

/**
 * @brief Gets the type of a transform node.
 * @return The type of a transform node.
 */
DS_SCENE_EXPORT dsSceneNodeType dsSceneTransformNode_type(void);

/**
 * @brief Creates a transform node.
 * @remark errno will be set on failure.
 * @param allocator The allocator for the node. This must support freeing memory.
 * @param transform The initial transform for the node.
 * @return The transform node or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneTransformNode* dsSceneTransformNode_create(dsAllocator* allocator,
	const dsMatrix44f* transform);

/**
 * @brief Sets the transform node's transform matrix.
 *
 * This will assign the transform and mark the node as dirty.
 *
 * @remark errno will be set on failure.
 * @param node The node to set the transform on.
 * @param transform THe new transform.
 * @return False if the parameters are invalid.
 */
DS_SCENE_EXPORT bool dsSceneTransformNode_setTransform(dsSceneTransformNode* node,
	const dsMatrix44f* transform);

#ifdef __cplusplus
}
#endif

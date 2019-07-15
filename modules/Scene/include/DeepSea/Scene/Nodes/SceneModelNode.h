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
 * @brief Functions for creating and manipulating model nodes.
 * @see dsSceneModelNode
 */

/**
 * @brief Gets the type of a model node.
 * @return The type of a model node.
 */
DS_SCENE_EXPORT dsSceneNodeType dsSceneModelNode_type(void);

/**
 * @brief Creates a scene model node.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the model node with.
 * @param models The models to draw within the node. The array will be copied. It is expected that
 *     at least one model is provided.
 * @param modelCount The number of models.
 * @param resources The resources to keep a reference to.
 * @param resourceCount The number of resources.
 * @param bounds The bounding box for the model. If NULL, the model will never be culled.
 * @return The model node or NULL if an error occurred.
 */
DS_SCENE_EXPORT dsSceneModelNode* dsSceneModelNode_create(dsAllocator* allocator,
	const dsSceneModelInitInfo* models, uint32_t modelCount, dsSceneResources** resources,
	uint32_t resourceCount, const dsOrientedBox3f* bounds);

#ifdef __cplusplus
}
#endif

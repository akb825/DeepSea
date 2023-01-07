/*
 * Copyright 2022-2023 Aaron Barany
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
 * @brief Functions for manipulating cull nodes.
 * @see dsSceneCullNode
 */

/**
 * @brief Gets the type of a cull node.
 * @return The type of a cull node.
 */
DS_SCENE_EXPORT const dsSceneNodeType* dsSceneCullNode_type(void);

/**
 * @brief Gets the bounds for a cull node.
 *
 * If false is returned, the node should be considered always out of view. If true is returned and
 * bounds are invalid, the node should be considered always in view.
 *
 * @param[out] outBoxMatrix The bounds of the node in world space.
 * @param node The cull node to check.
 * @param treeNode The tree node for the instance to check.
 * @return False if there's no bounds available.
 */
DS_SCENE_EXPORT bool dsSceneCullNode_getBounds(dsMatrix44f* outBoxMatrix,
	const dsSceneCullNode* node, const dsSceneTreeNode* treeNode);

#ifdef __cplusplus
}
#endif

/*
 * Copyright 2019-2025 Aaron Barany
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

dsScene* dsSceneTreeNode_getScene(dsSceneTreeNode* node);
bool dsSceneTreeNode_buildSubtree(dsSceneNode* node, dsSceneNode* child);
void dsSceneTreeNode_removeSubtree(dsSceneNode* node, dsSceneNode* child);
bool dsSceneTreeNode_reparentSubtree(dsSceneNode* node, dsSceneNode* child, dsSceneNode* newParent);
bool dsSceneTreeNode_transferSceneNodes(dsSceneNode* prevRoot, dsSceneNode* newRoot,
	const dsScene* newScene, const dsHashTable* commonItemLists);
void dsSceneTreeNode_updateSubtree(dsSceneTreeNode* node);

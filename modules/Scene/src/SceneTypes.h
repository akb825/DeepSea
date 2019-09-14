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
#include <DeepSea/Core/Types.h>
#include <DeepSea/Scene/Types.h>

typedef struct dsSceneItemEntry
{
	dsSceneItemList* list;
	uint64_t entry;
} dsSceneItemEntry;

struct dsSceneTreeNode
{
	dsAllocator* allocator;
	dsSceneNode* node;
	dsSceneTreeNode* parent;
	dsSceneTreeNode** children;
	dsSceneItemEntry* itemLists;
	uint32_t childCount;
	uint32_t maxChildren;
	dsMatrix44f transform;
	bool dirty;
};

typedef struct dsSceneTreeRootNode
{
	dsSceneTreeNode node;
	dsScene* scene;
} dsSceneTreeRootNode;

typedef struct dsSceneItemListNode
{
	dsHashTableNode node;
	dsSceneItemList* list;
} dsSceneItemListNode;

struct dsScene
{
	dsAllocator* allocator;
	dsRenderer* renderer;

	void* userData;
	dsDestroySceneUserDataFunction destroyUserDataFunc;

	dsSceneNode rootNode;
	dsSceneTreeRootNode rootTreeNode;
	dsSceneTreeNode* rootTreeNodePtr;

	dsSceneItemList** sharedItems;
	dsScenePipelineItem* pipeline;
	dsSceneGlobalData** globalData;
	uint32_t sharedItemCount;
	uint32_t pipelineCount;
	uint32_t globalDataCount;
	uint32_t globalValueCount;
	dsHashTable* itemLists;

	dsSceneTreeNode** dirtyNodes;
	uint32_t dirtyNodeCount;
	uint32_t maxDirtyNodes;
};

extern dsSceneNodeType dsRootSceneNodeType;

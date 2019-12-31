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
#include <DeepSea/Core/Containers/Types.h>
#include <DeepSea/Core/Streams/Types.h>
#include <DeepSea/Scene/Types.h>

#define DS_MAX_SCENE_TYPES 128
#define DS_SCENE_TYPE_TABLE_SIZE 173

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
	dsSceneNodeItemData itemData;
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

	dsSceneItemLists* sharedItems;
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

typedef struct dsRotatedFramebuffer
{
	dsFramebuffer* framebuffer;
	bool rotated;
} dsRotatedFramebuffer;

typedef struct dsLoadSceneNodeItem
{
	dsHashTableNode node;
	char name[DS_MAX_SCENE_NAME_LENGTH];
	dsLoadSceneNodeFunction loadFunc;
	void* userData;
	dsDestroySceneUserDataFunction destroyUserDataFunc;
} dsLoadSceneNodeItem;

typedef struct dsLoadSceneItemListItem
{
	dsHashTableNode node;
	char name[DS_MAX_SCENE_NAME_LENGTH];
	dsLoadSceneItemListFunction loadFunc;
	void* userData;
	dsDestroySceneUserDataFunction destroyUserDataFunc;
} dsLoadSceneItemListItem;

typedef struct dsLoadSceneInstanceDataItem
{
	dsHashTableNode node;
	char name[DS_MAX_SCENE_NAME_LENGTH];
	dsLoadSceneInstanceDataFunction loadFunc;
	void* userData;
	dsDestroySceneUserDataFunction destroyUserDataFunc;
} dsLoadSceneInstanceDataItem;

typedef struct dsLoadSceneGlobalDataItem
{
	dsHashTableNode node;
	char name[DS_MAX_SCENE_NAME_LENGTH];
	dsLoadSceneGlobalDataFunction loadFunc;
	void* userData;
	dsDestroySceneUserDataFunction destroyUserDataFunc;
} dsLoadSceneGlobalDataItem;

struct dsSceneLoadContext
{
	dsAllocator* allocator;
	dsRenderer* renderer;

	dsLoadSceneNodeItem nodeTypes[DS_MAX_SCENE_TYPES];
	dsLoadSceneItemListItem itemListTypes[DS_MAX_SCENE_TYPES];
	dsLoadSceneInstanceDataItem instanceDataTypes[DS_MAX_SCENE_TYPES];
	dsLoadSceneGlobalDataItem globalDataTypes[DS_MAX_SCENE_TYPES];

	DS_STATIC_HASH_TABLE(DS_SCENE_TYPE_TABLE_SIZE) nodeTypeTable;
	DS_STATIC_HASH_TABLE(DS_SCENE_TYPE_TABLE_SIZE) itemListTypeTable;
	DS_STATIC_HASH_TABLE(DS_SCENE_TYPE_TABLE_SIZE) instanceDataTypeTable;
	DS_STATIC_HASH_TABLE(DS_SCENE_TYPE_TABLE_SIZE) globalDataTypeTable;
};

struct dsSceneLoadScratchData
{
	dsAllocator* allocator;
	dsCommandBuffer* commandBuffer;

	void* readBuffer;
	size_t readBufferSize;
	bool readBufferUsed;

	dsSceneResources** sceneResources;
	uint32_t sceneResourceCount;
	uint32_t maxSceneResources;
};

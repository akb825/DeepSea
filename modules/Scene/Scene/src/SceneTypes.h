/*
 * Copyright 2019-2023 Aaron Barany
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

#ifdef __cplusplus
extern "C"
{
#endif

#define DS_MAX_SCENE_TYPES 128
#define DS_SCENE_TYPE_TABLE_SIZE 173

typedef struct dsSceneTreeRootNode
{
	dsSceneTreeNode node;
	dsScene* scene;
} dsSceneTreeRootNode;

typedef struct dsSceneItemListNode
{
	dsHashTableNode node;
	dsSceneItemList* list;
	// Allows updating the original list value when transferring from one scene to another.
	dsSceneItemList** listPtr;
} dsSceneItemListNode;

struct dsScene
{
	dsAllocator* allocator;
	dsRenderer* renderer;

	void* userData;
	dsDestroyUserDataFunction destroyUserDataFunc;

	dsSceneNode rootNode;
	dsSceneTreeRootNode rootTreeNode;
	dsSceneTreeNode* rootTreeNodePtr;

	dsSceneItemLists* sharedItems;
	dsScenePipelineItem* pipeline;
	uint32_t sharedItemCount;
	uint32_t pipelineCount;
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
	dsDestroyUserDataFunction destroyUserDataFunc;
} dsLoadSceneNodeItem;

typedef struct dsLoadSceneItemListItem
{
	dsHashTableNode node;
	char name[DS_MAX_SCENE_NAME_LENGTH];
	dsLoadSceneItemListFunction loadFunc;
	void* userData;
	dsDestroyUserDataFunction destroyUserDataFunc;
} dsLoadSceneItemListItem;

typedef struct dsLoadSceneInstanceDataItem
{
	dsHashTableNode node;
	char name[DS_MAX_SCENE_NAME_LENGTH];
	dsLoadSceneInstanceDataFunction loadFunc;
	void* userData;
	dsDestroyUserDataFunction destroyUserDataFunc;
} dsLoadSceneInstanceDataItem;

typedef struct dsLoadCustomSceneResourceItem
{
	dsHashTableNode node;
	char name[DS_MAX_SCENE_NAME_LENGTH];
	const dsCustomSceneResourceType* type;
	dsLoadCustomSceneResourceFunction loadFunc;
	dsDestroyCustomSceneResourceFunction destroyResourceFunc;
	void* userData;
	dsDestroyUserDataFunction destroyUserDataFunc;
	uint32_t additionalResources;
} dsLoadCustomSceneResourceItem;

typedef struct dsLoadSceneResourceActionItem
{
	dsHashTableNode node;
	char name[DS_MAX_SCENE_NAME_LENGTH];
	dsLoadSceneResourceActionFunction loadFunc;
	void* userData;
	dsDestroyUserDataFunction destroyUserDataFunc;
	uint32_t additionalResources;
} dsLoadSceneResourceActionItem;

struct dsSceneLoadContext
{
	dsAllocator* allocator;
	dsRenderer* renderer;

	dsLoadSceneNodeItem nodeTypes[DS_MAX_SCENE_TYPES];
	dsLoadSceneItemListItem itemListTypes[DS_MAX_SCENE_TYPES];
	dsLoadSceneInstanceDataItem instanceDataTypes[DS_MAX_SCENE_TYPES];
	dsLoadCustomSceneResourceItem customResourceTypes[DS_MAX_SCENE_TYPES];
	dsLoadSceneResourceActionItem resourceActionTypes[DS_MAX_SCENE_TYPES];

	DS_STATIC_HASH_TABLE(DS_SCENE_TYPE_TABLE_SIZE) nodeTypeTable;
	DS_STATIC_HASH_TABLE(DS_SCENE_TYPE_TABLE_SIZE) itemListTypeTable;
	DS_STATIC_HASH_TABLE(DS_SCENE_TYPE_TABLE_SIZE) instanceDataTypeTable;
	DS_STATIC_HASH_TABLE(DS_SCENE_TYPE_TABLE_SIZE) customResourceTypeTable;
	DS_STATIC_HASH_TABLE(DS_SCENE_TYPE_TABLE_SIZE) resourceActionTypeTable;
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

#ifdef __cplusplus
}
#endif

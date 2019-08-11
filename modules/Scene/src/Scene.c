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

#include <DeepSea/Scene/Scene.h>

#include "Nodes/SceneTreeNode.h"
#include "SceneTypes.h"
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Scene/ItemLists/SceneItemList.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/SceneGlobalData.h>
#include <DeepSea/Scene/SceneRenderPass.h>

#include <string.h>

static dsSceneNodeType rootNodeType;

static void destroyObjects(dsSceneItemList* const* sharedItems, uint32_t sharedItemCount,
	const dsScenePipelineItem* pipeline, uint32_t pipelineCount,
	dsSceneGlobalData* const* globalData, uint32_t globalDataCount, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc)
{
	if (sharedItems)
	{
		for (uint32_t i = 0; i < sharedItemCount; ++i)
			dsSceneItemList_destroy(sharedItems[i]);
	}

	if (pipeline)
	{
		for (uint32_t i = 0; i < pipelineCount; ++i)
		{
			dsSceneRenderPass_destroy(pipeline[i].renderPass);
			dsSceneItemList_destroy(pipeline[i].computeItems);
		}
	}

	if (globalData)
	{
		for (uint32_t i = 0; i < globalDataCount; ++i)
			dsSceneGlobalData_destroy(globalData[i]);
	}

	if (destroyUserDataFunc)
		destroyUserDataFunc(userData);
}

static size_t fullAllocSize(uint32_t* outNameCount, dsSceneItemList* const* sharedItems,
	uint32_t sharedItemCount, const dsScenePipelineItem* pipeline, uint32_t pipelineCount,
	dsSceneGlobalData* const* globalData, uint32_t globalDataCount)
{
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsScene)) +
		DS_ALIGNED_SIZE(sizeof(dsSceneItemList*)*sharedItemCount) +
		DS_ALIGNED_SIZE(sizeof(dsScenePipelineItem)*pipelineCount) +
		DS_ALIGNED_SIZE(sizeof(dsSceneGlobalData*)*globalDataCount);

	*outNameCount = sharedItemCount;
	for (uint32_t i = 0; i < sharedItemCount; ++i)
	{
		if (!sharedItems[i])
			return 0;
	}

	for (uint32_t i = 0; i < pipelineCount; ++i)
	{
		const dsScenePipelineItem* item = pipeline  + i;
		if ((item->renderPass && item->computeItems) ||
			(!item->renderPass && !item->computeItems))
		{
			return 0;
		}

		if (item->renderPass)
		{
			fullSize += DS_ALIGNED_SIZE(strlen(item->renderPass->framebuffer) + 1);
			for (uint32_t j = 0; j < item->renderPass->renderPass->subpassCount; ++j)
			{
				const dsSubpassDrawLists* items = item->renderPass->drawLists + j;
				*outNameCount += items->count;
			}
		}
		else
			++*outNameCount;
	}

	for (uint32_t i = 0; i < globalDataCount; ++i)
	{
		if (!globalData[i])
			return 0;
	}

	return fullSize + dsHashTable_fullAllocSize(dsHashTable_getTableSize(*outNameCount)) +
		DS_ALIGNED_SIZE(sizeof(dsSceneItemListNode)**outNameCount);
}

static bool insertSceneList(dsHashTable* hashTable, dsSceneItemListNode* node,
	dsSceneItemList* list)
{
	node->list = list;
	if (!dsHashTable_insert(hashTable, list->name, (dsHashTableNode*)node, NULL))
	{
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Scene item list '%s' isn't unique within the scene.",
			node->list->name);
		return false;
	}

	return true;
}

dsScene* dsScene_create(dsAllocator* allocator, dsRenderer* renderer,
	dsSceneItemList* const* sharedItems, uint32_t sharedItemCount,
	const dsScenePipelineItem* pipeline, uint32_t pipelineCount,
	dsSceneGlobalData* const* globalData, uint32_t globalDataCount, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc)
{
	if (!allocator || !renderer || (!sharedItems && sharedItemCount > 0) || !pipeline ||
		pipelineCount == 0 || (!globalData && globalDataCount > 0))
	{
		errno = EINVAL;
		destroyObjects(sharedItems, sharedItemCount, pipeline, pipelineCount, globalData,
			globalDataCount, userData, destroyUserDataFunc);
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene allocator must support freeing memory.");
		destroyObjects(sharedItems, sharedItemCount, pipeline, pipelineCount, globalData,
			globalDataCount, userData, destroyUserDataFunc);
		return NULL;
	}

	uint32_t nameCount;
	size_t fullSize = fullAllocSize(&nameCount, sharedItems, sharedItemCount, pipeline,
		pipelineCount, globalData, globalDataCount);
	if (fullSize == 0)
	{
		errno = EINVAL;
		destroyObjects(sharedItems, sharedItemCount, pipeline, pipelineCount, globalData,
			globalDataCount, userData, destroyUserDataFunc);
		return NULL;
	}

	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		destroyObjects(sharedItems, sharedItemCount, pipeline, pipelineCount, globalData,
			globalDataCount, userData, destroyUserDataFunc);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsScene* scene = DS_ALLOCATE_OBJECT(&bufferAlloc, dsScene);
	DS_ASSERT(scene);

	scene->allocator = dsAllocator_keepPointer(allocator);
	scene->renderer = renderer;
	scene->userData = userData;
	scene->destroyUserDataFunc = destroyUserDataFunc;

	DS_VERIFY(dsSceneNode_initialize(&scene->rootNode, allocator, &rootNodeType, NULL, 0, NULL));

	dsSceneTreeNode* rootTreeNode = &scene->rootTreeNode.node;
	rootTreeNode->allocator = allocator;
	rootTreeNode->node = &scene->rootNode;
	rootTreeNode->parent = NULL;
	rootTreeNode->children = NULL;
	rootTreeNode->drawItems = NULL;
	rootTreeNode->childCount = 0;
	rootTreeNode->maxChildren = 0;
	dsMatrix44_identity(rootTreeNode->transform);
	rootTreeNode->dirty = false;
	scene->rootTreeNode.scene = scene;

	if (sharedItemCount > 0)
	{
		scene->sharedItems =
			DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneItemList*, sharedItemCount);
		DS_ASSERT(scene->sharedItems);
		memcpy(scene->sharedItems, sharedItems, sizeof(dsSceneItemList*)*sharedItemCount);
	}
	else
		scene->sharedItems = NULL;
	scene->sharedItemCount = sharedItemCount;

	scene->pipeline = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsScenePipelineItem, pipelineCount);
	DS_ASSERT(scene->pipeline);
	memcpy(scene->pipeline, pipeline, sizeof(dsScenePipelineItem)*pipelineCount);
	scene->pipelineCount = pipelineCount;

	for (uint32_t i = 0; i < pipelineCount; ++i)
	{
		dsScenePipelineItem* item = scene->pipeline + i;
		if (!item->renderPass)
			continue;

		size_t framebufferLen = strlen(pipeline[i].renderPass->framebuffer) + 1;
		item->renderPass->framebuffer =
			DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, framebufferLen);
		DS_ASSERT(item->renderPass->framebuffer);
		memcpy((void*)item->renderPass->framebuffer, pipeline[i].renderPass->framebuffer,
			framebufferLen);
	}

	scene->globalValueCount = 0;
	if (globalDataCount > 0)
	{
		scene->globalData =
			DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneGlobalData*, globalDataCount);
		DS_ASSERT(scene->globalData);
		memcpy(scene->globalData, globalData, sizeof(dsSceneGlobalData*)*globalDataCount);
		for (uint32_t i = 0; i < globalDataCount; ++i)
			scene->globalValueCount += globalData[i]->valueCount;
	}
	scene->globalDataCount = globalDataCount;

	uint32_t tableSize = dsHashTable_getTableSize(nameCount);
	size_t hashTableSize = dsHashTable_fullAllocSize(tableSize);
	scene->itemLists = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, hashTableSize);
	DS_ASSERT(scene->itemLists);
	DS_VERIFY(dsHashTable_initialize(scene->itemLists, tableSize, &dsHashString,
		&dsHashStringEqual));

	scene->dirtyNodes = NULL;
	scene->dirtyNodeCount = 0;
	scene->maxDirtyNodes = 0;

	dsSceneItemListNode* itemNodes = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneItemListNode,
		nameCount);
	DS_ASSERT(itemNodes);
	uint32_t curItems = 0;
	for (uint32_t i = 0; i < pipelineCount; ++i)
	{
		const dsScenePipelineItem* item = pipeline  + i;
		if (item->renderPass)
		{
			for (uint32_t j = 0; j < item->renderPass->renderPass->subpassCount; ++j)
			{
				const dsSubpassDrawLists* items = item->renderPass->drawLists + j;
				for (uint32_t k = 0; k < items->count; ++k)
				{
					dsSceneItemListNode* node = itemNodes + curItems++;
					if (!insertSceneList(scene->itemLists, node, items->drawLists[i]))
					{
						errno = EINVAL;
						dsScene_destroy(scene);
						return NULL;
					}
				}
			}
		}
		else
		{
			dsSceneItemListNode* node = itemNodes + curItems++;
			if (!insertSceneList(scene->itemLists, node, item->computeItems))
			{
				errno = EINVAL;
				dsScene_destroy(scene);
				return NULL;
			}
		}
	}

	return scene;
}

dsAllocator* dsScene_getAllocator(const dsScene* scene)
{
	if (!scene)
		return NULL;
	return scene->allocator;
}

dsRenderer* dsScene_getRenderer(const dsScene* scene)
{
	if (!scene)
		return NULL;
	return scene->renderer;
}

void* dsScene_getUserData(const dsScene* scene)
{
	if (!scene)
		return NULL;
	return scene->userData;
}

bool dsScene_update(dsScene* scene)
{
	if (!scene)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < scene->dirtyNodeCount; ++i)
		dsSceneTreeNode_updateSubtree(scene->dirtyNodes[i]);
	scene->dirtyNodeCount = 0;
	return true;
}

void dsScene_destroy(dsScene* scene)
{
	if (!scene)
		return;

	// Prevent tree teardown from removing from the dirty list, which is just a waste of cycles on
	// destruction.
	scene->dirtyNodeCount = 0;

	DS_ASSERT(scene->rootNode.refCount == 1);
	dsSceneNode_freeRef(&scene->rootNode);

	dsSceneTreeNode* rootTreeNode = &scene->rootTreeNode.node;
	DS_ASSERT(rootTreeNode->childCount == 0);
	DS_VERIFY(dsAllocator_free(rootTreeNode->allocator, rootTreeNode->children));

	destroyObjects(scene->sharedItems, scene->sharedItemCount, scene->pipeline,
		scene->pipelineCount, scene->globalData, scene->globalDataCount, scene->userData,
		scene->destroyUserDataFunc);
	DS_VERIFY(dsAllocator_free(scene->allocator, scene->dirtyNodes));

	DS_VERIFY(dsAllocator_free(scene->allocator, scene));
}

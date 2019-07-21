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
#include <DeepSea/Core/Containers/StringPool.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/SceneItemList.h>
#include <DeepSea/Scene/SceneRenderPass.h>

#include <string.h>

static dsSceneNodeType rootNodeType;

static void destroyObjects(const dsScenePipelineItem* pipeline, uint32_t pipelineCount,
	const dsStringPool* stringPool)
{
	if (pipeline)
	{
		for (uint32_t i = 0; i < pipelineCount; ++i)
		{
			dsSceneRenderPass_destroy(pipeline[i].renderPass);
			dsSceneItemList_destroy(pipeline[i].computeItems);
		}
	}

	if (stringPool->allocator)
		DS_VERIFY(dsAllocator_free(stringPool->allocator, stringPool->strings));
}

static size_t fullAllocSize(uint32_t* outNameCount, const dsScenePipelineItem* pipeline,
	uint32_t pipelineCount)
{
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsScene)) +
		DS_ALIGNED_SIZE(sizeof(dsScenePipelineItem)*pipelineCount);

	*outNameCount = 0;
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
			for (uint32_t j = 0; j < item->renderPass->renderPass->subpassCount; ++j)
			{
				const dsSubpassDrawLists* items = item->renderPass->drawLists + j;
				*outNameCount += items->count;
			}
		}
		else
			++*outNameCount;
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

dsScene* dsScene_create(dsAllocator* allocator, const dsScenePipelineItem* pipeline,
	uint32_t pipelineCount, const dsStringPool* stringPool)
{
	if (!allocator || !pipeline || pipelineCount == 0)
	{
		errno = EINVAL;
		destroyObjects(pipeline, pipelineCount, stringPool);
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene allocator must support freeing memory.");
		destroyObjects(pipeline, pipelineCount, stringPool);
		return NULL;
	}

	uint32_t nameCount;
	size_t fullSize = fullAllocSize(&nameCount, pipeline, pipelineCount);
	if (fullSize == 0)
	{
		errno = EINVAL;
		destroyObjects(pipeline, pipelineCount, stringPool);
		return NULL;
	}

	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		destroyObjects(pipeline, pipelineCount, stringPool);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsScene* scene = DS_ALLOCATE_OBJECT(&bufferAlloc, dsScene);
	DS_ASSERT(scene);

	scene->allocator = dsAllocator_keepPointer(allocator);
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

	scene->pipeline = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsScenePipelineItem, pipelineCount);
	DS_ASSERT(scene->pipeline);
	memcpy(scene->pipeline, pipeline, sizeof(dsScenePipelineItem)*pipelineCount);
	scene->pipelineCount = pipelineCount;

	uint32_t tableSize = dsHashTable_getTableSize(nameCount);
	size_t hashTableSize = dsHashTable_fullAllocSize(tableSize);
	scene->itemLists = dsAllocator_alloc((dsAllocator*)&bufferAlloc, hashTableSize);
	DS_ASSERT(scene->itemLists);
	DS_VERIFY(dsHashTable_initialize(scene->itemLists, tableSize, &dsHashString,
		&dsHashStringEqual));
	if (stringPool)
		scene->stringPool = *stringPool;
	else
		memset(&scene->stringPool, 0, sizeof(scene->stringPool));

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

	destroyObjects(scene->pipeline, scene->pipelineCount, &scene->stringPool);
	DS_VERIFY(dsAllocator_free(scene->allocator, scene->dirtyNodes));

	DS_VERIFY(dsAllocator_free(scene->allocator, scene));
}

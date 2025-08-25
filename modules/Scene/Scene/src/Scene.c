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

#include <DeepSea/Scene/Scene.h>

#include "Nodes/SceneTreeNodeInternal.h"
#include "SceneTypes.h"

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Streams/FileArchive.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Scene/ItemLists/SceneItemList.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/SceneRenderPass.h>

#include <string.h>

static void destroyObjects(const dsSceneItemLists* sharedItems, uint32_t sharedItemCount,
	const dsScenePipelineItem* pipeline, uint32_t pipelineCount, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc)
{
	if (sharedItems)
	{
		for (uint32_t i = 0; i < sharedItemCount; ++i)
		{
			const dsSceneItemLists* itemLists = sharedItems + i;
			for (uint32_t j = 0; j < itemLists->count; ++j)
				dsSceneItemList_destroy(itemLists->itemLists[j]);
		}
	}

	if (pipeline)
	{
		for (uint32_t i = 0; i < pipelineCount; ++i)
		{
			dsSceneRenderPass_destroy(pipeline[i].renderPass);
			dsSceneItemList_destroy(pipeline[i].computeItems);
		}
	}

	if (destroyUserDataFunc)
		destroyUserDataFunc(userData);
}

static size_t fullAllocSize(uint32_t* outNameCount, uint32_t* outGlobalValueCount,
	const dsSceneItemLists* sharedItems, uint32_t sharedItemCount,
	const dsScenePipelineItem* pipeline, uint32_t pipelineCount)
{
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsScene)) +
		DS_ALIGNED_SIZE(sizeof(dsSceneItemList)*sharedItemCount) +
		DS_ALIGNED_SIZE(sizeof(dsScenePipelineItem)*pipelineCount);

	*outNameCount = 0;
	*outGlobalValueCount = 0;
	for (uint32_t i = 0; i < sharedItemCount; ++i)
	{
		const dsSceneItemLists* itemLists = sharedItems + i;
		if (sharedItems[i].count > 0 && !itemLists->itemLists)
			return 0;

		for (uint32_t j = 0; j < itemLists->count; ++j)
		{
			const dsSceneItemList* itemList = itemLists->itemLists[j];
			if (!itemList || !itemList->type)
				return 0;

			*outGlobalValueCount += itemList->globalValueCount;
		}

		*outNameCount += itemLists->count;
		fullSize += DS_ALIGNED_SIZE(sizeof(dsSceneItemList*)*itemLists->count);
	}

	for (uint32_t i = 0; i < pipelineCount; ++i)
	{
		const dsScenePipelineItem* item = pipeline  + i;
		if ((item->renderPass && item->computeItems) ||
			(!item->renderPass && !item->computeItems))
		{
			DS_LOG_ERROR(DS_SCENE_LOG_TAG,
				"A scene pipeline item must contain either a render pass or a compute item.");
			return 0;
		}

		if (item->renderPass)
		{
			const dsRenderPass* baseRenderPass = item->renderPass->renderPass;
			for (uint32_t j = 0; j < baseRenderPass->subpassCount; ++j)
			{
				const dsSceneItemLists* items = item->renderPass->drawLists + j;
				for (uint32_t k = 0; k < items->count; ++k)
				{
					const dsSceneItemList* itemList = items->itemLists[k];
					if (!itemList)
						return 0;
					else if (!itemList->type || !itemList->type->commitFunc)
					{
						DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
							"Scene item list '%s' inside render subpass '%s' must have a commit "
							"function.", itemList->name, baseRenderPass->subpasses[j].name);
						return 0;
					}
					else if (itemList->globalValueCount > 0)
					{
						DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
							"Scene item list '%s' with global values must be in the sharedItems "
							"array.", itemList->name);
						return 0;
					}
				}
				*outNameCount += items->count;
			}
		}
		else
		{
			dsSceneItemList* itemList = item->computeItems;
			if (!itemList->type)
				return 0;
			else if (itemList->globalValueCount > 0)
			{
				DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
					"Scene item list '%s' with global values must be in the sharedItem array.",
					itemList->name);
				return 0;
			}
			else if (itemList->type->preRenderPassFunc)
			{
				DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
					"Compute scene item list '%s' may not have a preRenderPass function.",
					itemList->name);
				return 0;
			}
			++*outNameCount;
		}
	}

	return fullSize + dsHashTable_fullAllocSize(dsHashTable_tableSize(*outNameCount)) +
		DS_ALIGNED_SIZE(sizeof(dsSceneItemListNode)**outNameCount);
}

static bool hashPrevItemLists(void** outData, dsHashTable** outHashTable,
	dsScene* prevScene, dsAllocator* allocator)
{
	if (!prevScene)
		return true;

	size_t itemListCount = prevScene->itemLists->list.length;
	size_t tableSize = dsHashTable_tableSize(itemListCount);
	size_t hashTableSize = dsHashTable_fullAllocSize(tableSize);
	size_t fullSize = DS_ALIGNED_SIZE(itemListCount*sizeof(dsSceneItemListNode)) + hashTableSize;
	*outData = dsAllocator_alloc(allocator, fullSize);
	if (!*outData)
		return false;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, *outData, fullSize));

	dsSceneItemListNode* nodes =
		DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneItemListNode, itemListCount);
	DS_ASSERT(nodes);

	dsHashTable* hashTable = (dsHashTable*)dsAllocator_alloc(
		(dsAllocator*)&bufferAlloc, hashTableSize);
	DS_ASSERT(hashTable);
	*outHashTable = hashTable;
	DS_VERIFY(dsHashTable_initialize(hashTable, tableSize, (dsHashFunction)&dsSceneItemList_hash,
		(dsKeysEqualFunction)&dsSceneItemList_equal));

	dsSceneItemListNode* curNode = nodes;
	for (dsListNode* node = prevScene->itemLists->list.head; node; node = node->next, ++curNode)
	{
		dsSceneItemListNode* itemListNode = (dsSceneItemListNode*)node;
		curNode->list = itemListNode->list;
		curNode->listPtr = itemListNode->listPtr;
		DS_VERIFY(dsHashTable_insert(hashTable, curNode->list, (dsHashTableNode*)curNode, NULL));
	}
	return true;
}


static bool insertSceneList(dsHashTable* hashTable, dsSceneItemListNode* node,
	dsSceneItemList** listPtr, const dsHashTable* prevItemLists)
{
	dsSceneItemList* list = *listPtr;
	if (prevItemLists)
	{
		dsSceneItemListNode* prevNode = (dsSceneItemListNode*)dsHashTable_find(prevItemLists, list);
		if (prevNode)
		{
			// Swap with previous list if equivalent found. Clear out the original list pointer to
			// use later to indicate that the list was replaced.
			DS_ASSERT(prevNode->listPtr);
			*prevNode->listPtr = list;
			prevNode->listPtr = NULL;
			list = *listPtr = prevNode->list;
		}
	}

	node->list = list;
	node->listPtr = listPtr;
	if (!dsHashTable_insert(hashTable, list->name, (dsHashTableNode*)node, NULL))
	{
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Scene item list '%s' isn't unique within the scene.",
			node->list->name);
		return false;
	}

	return true;
}

static void hashPrevItemListPointers(void* data, dsHashTable* hashTable)
{
	DS_ASSERT(data);
	DS_ASSERT(hashTable);

	size_t itemListCount = hashTable->list.length;
	dsSceneItemListNode* prevNode = (dsSceneItemListNode*)data;
	dsHashTableNode* newNode = (dsHashTableNode*)data;
	DS_VERIFY(dsHashTable_initialize(
		hashTable, hashTable->tableSize, &dsHashPointer, &dsHashPointerEqual));
	for (size_t i = 0; i < itemListCount; ++i, ++prevNode)
	{
		// Used if the original list pionter was cleared.
		if (!prevNode->listPtr)
		{
			DS_VERIFY(dsHashTable_insert(hashTable, prevNode->list, newNode, NULL));
			++newNode;
		}
	}
}

dsScene* dsScene_loadImpl(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData, const void* data,
	size_t dataSize, void* userData, dsDestroyUserDataFunction destroyUserDataFunc,
	dsScene* prevScene, const char* fileName);

dsScene* dsScene_create(dsAllocator* allocator, dsRenderer* renderer,
	const dsSceneItemLists* sharedItems, uint32_t sharedItemCount,
	const dsScenePipelineItem* pipeline, uint32_t pipelineCount, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc, dsScene* prevScene)
{
	if (!allocator || !renderer || (!sharedItems && sharedItemCount > 0) || !pipeline ||
		pipelineCount == 0)
	{
		errno = EINVAL;
		destroyObjects(sharedItems, sharedItemCount, pipeline, pipelineCount, userData,
			destroyUserDataFunc);
		dsScene_destroy(prevScene);
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene allocator must support freeing memory.");
		destroyObjects(sharedItems, sharedItemCount, pipeline, pipelineCount, userData,
			destroyUserDataFunc);
		dsScene_destroy(prevScene);
		return NULL;
	}

	uint32_t nameCount, globalValueCount;
	size_t fullSize = fullAllocSize(&nameCount, &globalValueCount, sharedItems, sharedItemCount,
		pipeline, pipelineCount);
	if (fullSize == 0)
	{
		errno = EINVAL;
		destroyObjects(sharedItems, sharedItemCount, pipeline, pipelineCount, userData,
			destroyUserDataFunc);
		dsScene_destroy(prevScene);
		return NULL;
	}

	void* prevItemListData = NULL;
	dsHashTable* prevItemLists = NULL;
	if (!hashPrevItemLists(&prevItemListData, &prevItemLists, prevScene, allocator))
	{
		destroyObjects(sharedItems, sharedItemCount, pipeline, pipelineCount, userData,
			destroyUserDataFunc);
		dsScene_destroy(prevScene);
	}

	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		destroyObjects(sharedItems, sharedItemCount, pipeline, pipelineCount, userData,
			destroyUserDataFunc);
		dsScene_destroy(prevScene);
		DS_VERIFY(dsAllocator_free(allocator, prevItemListData));
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

	DS_VERIFY(dsSceneNode_initialize(&scene->rootNode, allocator, &dsRootSceneNodeType, NULL, 0));

	dsSceneTreeNode* rootTreeNode = &scene->rootTreeNode.node;
	rootTreeNode->allocator = allocator;
	rootTreeNode->node = &scene->rootNode;
	rootTreeNode->parent = NULL;
	rootTreeNode->children = NULL;
	rootTreeNode->itemLists = NULL;
	rootTreeNode->childCount = 0;
	rootTreeNode->maxChildren = 0;
	dsMatrix44_identity(rootTreeNode->transform);
	rootTreeNode->dirty = false;
	scene->rootTreeNode.scene = scene;
	scene->rootTreeNodePtr = (dsSceneTreeNode*)&scene->rootTreeNode;
	scene->rootNode.treeNodes = &scene->rootTreeNodePtr;
	scene->rootNode.treeNodeCount = 1;
	scene->rootNode.maxTreeNodes = 1;

	if (sharedItemCount > 0)
	{
		scene->sharedItems =
			DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneItemLists, sharedItemCount);
		DS_ASSERT(scene->sharedItems);
		for (uint32_t i = 0; i < sharedItemCount; ++i)
		{
			const dsSceneItemLists* origItemLists = sharedItems + i;
			dsSceneItemLists* itemLists = scene->sharedItems + i;
			itemLists->count = origItemLists->count;
			if (itemLists->count)
			{
				itemLists->itemLists = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneItemList*,
					itemLists->count);
				DS_ASSERT(itemLists->itemLists);
				memcpy(itemLists->itemLists, origItemLists->itemLists,
					sizeof(dsSceneItemLists*)*itemLists->count);
			}
			else
				itemLists->itemLists = NULL;
		}
	}
	else
		scene->sharedItems = NULL;
	scene->sharedItemCount = sharedItemCount;

	scene->pipeline = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsScenePipelineItem, pipelineCount);
	DS_ASSERT(scene->pipeline);
	memcpy(scene->pipeline, pipeline, sizeof(dsScenePipelineItem)*pipelineCount);
	scene->pipelineCount = pipelineCount;
	scene->globalValueCount = globalValueCount;

	size_t tableSize = dsHashTable_tableSize(nameCount);
	size_t hashTableSize = dsHashTable_fullAllocSize(tableSize);
	scene->itemLists = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, hashTableSize);
	DS_ASSERT(scene->itemLists);
	DS_VERIFY(dsHashTable_initialize(scene->itemLists, tableSize, &dsHashString,
		&dsHashStringEqual));

	scene->dirtyNodes = NULL;
	scene->dirtyNodeCount = 0;
	scene->maxDirtyNodes = 0;

	dsSceneItemListNode* itemNodes = DS_ALLOCATE_OBJECT_ARRAY(
		&bufferAlloc, dsSceneItemListNode, nameCount);
	DS_ASSERT(itemNodes);
	uint32_t curItems = 0;
	for (uint32_t i = 0; i < sharedItemCount; ++i)
	{
		const dsSceneItemLists* itemLists = scene->sharedItems + i;
		for (uint32_t j = 0; j < itemLists->count; ++j)
		{
			dsSceneItemListNode* node = itemNodes + curItems++;
			if (!insertSceneList(
					scene->itemLists, node, itemLists->itemLists + j, prevItemLists))
			{
				errno = EINVAL;
				dsScene_destroy(scene);
				dsScene_destroy(prevScene);
				DS_VERIFY(dsAllocator_free(allocator, prevItemListData));
				return NULL;
			}
		}
	}

	for (uint32_t i = 0; i < pipelineCount; ++i)
	{
		dsScenePipelineItem* item = scene->pipeline  + i;
		if (item->renderPass)
		{
			for (uint32_t j = 0; j < item->renderPass->renderPass->subpassCount; ++j)
			{
				const dsSceneItemLists* items = item->renderPass->drawLists + j;
				for (uint32_t k = 0; k < items->count; ++k)
				{
					dsSceneItemListNode* node = itemNodes + curItems++;
					if (!insertSceneList(
							scene->itemLists, node, items->itemLists + k, prevItemLists))
					{
						errno = EINVAL;
						dsScene_destroy(scene);
						dsScene_destroy(prevScene);
						DS_VERIFY(dsAllocator_free(allocator, prevItemListData));
						return NULL;
					}
				}
			}
		}
		else
		{
			dsSceneItemListNode* node = itemNodes + curItems++;
			if (!insertSceneList(scene->itemLists, node, &item->computeItems, prevItemLists))
			{
				errno = EINVAL;
				dsScene_destroy(scene);
				dsScene_destroy(prevScene);
				DS_VERIFY(dsAllocator_free(allocator, prevItemListData));
				return NULL;
			}
		}
	}
	DS_ASSERT(curItems == nameCount);

	if (prevScene)
	{
		// Transfer over the nodes. Avoid removing or re-adding entries for item lists that were
		// kept.
		hashPrevItemListPointers(prevItemListData, prevItemLists);
		bool success = dsSceneTreeNode_transferSceneNodes(
			&prevScene->rootNode, &scene->rootNode, scene, prevItemLists);

		dsScene_destroy(prevScene);
		DS_VERIFY(dsAllocator_free(allocator, prevItemListData));
		if (!success)
		{
			dsScene_destroy(scene);
			return false;
		}
	}
	return scene;
}

dsScene* dsScene_loadFile(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData,
	void* userData, dsDestroyUserDataFunction destroyUserDataFunc, dsScene* prevScene,
	const char* filePath)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !loadContext || !scratchData || !filePath)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	dsFileStream stream;
	if (!dsFileStream_openPath(&stream, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open scene file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsSceneLoadScratchData_readUntilEnd(&size, scratchData, (dsStream*)&stream);
	dsFileStream_close(&stream);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsScene* scene = dsScene_loadImpl(allocator, resourceAllocator, loadContext, scratchData,
		buffer, size, userData, destroyUserDataFunc, prevScene, filePath);
	DS_VERIFY(dsSceneLoadScratchData_freeReadBuffer(scratchData, buffer));
	DS_PROFILE_FUNC_RETURN(scene);
}

dsScene* dsScene_loadResource(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData,
	void* userData, dsDestroyUserDataFunction destroyUserDataFunc, dsScene* prevScene,
	dsFileResourceType type, const char* filePath)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !loadContext || !scratchData || !filePath)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsResourceStream stream;
	if (!dsResourceStream_open(&stream, type, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open scene node file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsSceneLoadScratchData_readUntilEnd(&size, scratchData, (dsStream*)&stream);
	dsResourceStream_close(&stream);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsScene* scene = dsScene_loadImpl(allocator, resourceAllocator, loadContext, scratchData,
		buffer, size, userData, destroyUserDataFunc, prevScene, filePath);
	DS_VERIFY(dsSceneLoadScratchData_freeReadBuffer(scratchData, buffer));
	DS_PROFILE_FUNC_RETURN(scene);
}

dsScene* dsScene_loadArchive(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData,
	void* userData, dsDestroyUserDataFunction destroyUserDataFunc, dsScene* prevScene,
	const dsFileArchive* archive, const char* filePath)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !loadContext || !scratchData || !archive || !filePath)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsStream* stream = dsFileArchive_openFile(archive, filePath);
	if (!stream)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open scene node file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsSceneLoadScratchData_readUntilEnd(&size, scratchData, stream);
	dsStream_close(stream);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsScene* scene = dsScene_loadImpl(allocator, resourceAllocator, loadContext, scratchData,
		buffer, size, userData, destroyUserDataFunc, prevScene, filePath);
	DS_VERIFY(dsSceneLoadScratchData_freeReadBuffer(scratchData, buffer));
	DS_PROFILE_FUNC_RETURN(scene);
}

dsScene* dsScene_loadStream(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc, dsScene* prevScene, dsStream* stream)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !loadContext || !scratchData || !stream)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsSceneLoadScratchData_readUntilEnd(&size, scratchData, stream);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsScene* scene = dsScene_loadImpl(allocator, resourceAllocator, loadContext, scratchData,
		buffer, size, userData, destroyUserDataFunc, prevScene, NULL);
	DS_VERIFY(dsSceneLoadScratchData_freeReadBuffer(scratchData, buffer));
	DS_PROFILE_FUNC_RETURN(scene);
}

dsScene* dsScene_loadData(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc, dsScene* prevScene, const void* data,
	size_t size)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !loadContext || !scratchData || !data || size == 0)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsScene* scene = dsScene_loadImpl(allocator, resourceAllocator, loadContext, scratchData, data,
		size, userData, destroyUserDataFunc, prevScene, NULL);
	DS_PROFILE_FUNC_RETURN(scene);
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

uint32_t dsScene_getNodeCount(const dsScene* scene)
{
	if (!scene)
		return 0;
	return scene->rootNode.childCount;
}

dsSceneNode* dsScene_getNode(const dsScene* scene, uint32_t index)
{
	if (!scene)
	{
		errno = EINVAL;
		return NULL;
	}

	if (index >= scene->rootNode.childCount)
	{
		errno = EINDEX;
		return NULL;
	}

	return scene->rootNode.children[index];
}

bool dsScene_addNode(dsScene* scene, dsSceneNode* node)
{
	if (!scene || !node)
	{
		errno = EINVAL;
		return false;
	}

	return dsSceneNode_addChild(&scene->rootNode, node);
}

bool dsScene_removeNodeIndex(dsScene* scene, uint32_t nodeIndex)
{
	if (!scene)
	{
		errno = EINVAL;
		return false;
	}

	return dsSceneNode_removeChildIndex(&scene->rootNode, nodeIndex);
}

bool dsScene_removeNode(dsScene* scene, dsSceneNode* node)
{
	if (!scene || !node)
	{
		errno = EINVAL;
		return false;
	}

	return dsSceneNode_removeChildNode(&scene->rootNode, node);
}

void dsScene_clearNodes(dsScene* scene)
{
	if (scene)
		dsSceneNode_clear(&scene->rootNode);
}

dsSceneItemList* dsScene_findItemList(dsScene* scene, const char* name)
{
	if (!scene || !name)
		return NULL;

	dsSceneItemListNode* foundNode = (dsSceneItemListNode*)dsHashTable_find(scene->itemLists, name);
	if (!foundNode)
		return NULL;

	return foundNode->list;
}

bool dsScene_forEachItemList(dsScene* scene, dsVisitSceneItemListsFunction visitFunc,
	void* userData)
{
	if (!scene || !visitFunc)
		return false;

	for (dsListNode* node = scene->itemLists->list.head; node; node = node->next)
	{
		if (!visitFunc(((dsSceneItemListNode*)node)->list, userData))
			break;
	}

	return true;
}

bool dsScene_update(dsScene* scene, float time)
{
	DS_PROFILE_FUNC_START();
	if (!scene)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	for (dsListNode* node = scene->itemLists->list.head; node; node = node->next)
	{
		dsSceneItemList* itemList = ((dsSceneItemListNode*)node)->list;
		dsUpdateSceneItemListFunction preTransformUpdateFunc =
			itemList->type->preTransformUpdateFunc;
		if (preTransformUpdateFunc)
		{
			DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);
			preTransformUpdateFunc(itemList, scene, time);
			DS_PROFILE_SCOPE_END();
		}
	}

	for (uint32_t i = 0; i < scene->dirtyNodeCount; ++i)
		dsSceneTreeNode_updateSubtree(scene->dirtyNodes[i]);
	scene->dirtyNodeCount = 0;

	for (dsListNode* node = scene->itemLists->list.head; node; node = node->next)
	{
		dsSceneItemList* itemList = ((dsSceneItemListNode*)node)->list;
		dsUpdateSceneItemListFunction updateFunc = itemList->type->updateFunc;
		if (updateFunc)
		{
			DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);
			updateFunc(itemList, scene, time);
			DS_PROFILE_SCOPE_END();
		}
	}

	DS_PROFILE_FUNC_RETURN(true);
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
		scene->pipelineCount, scene->userData, scene->destroyUserDataFunc);
	DS_VERIFY(dsAllocator_free(scene->allocator, scene->dirtyNodes));

	DS_VERIFY(dsAllocator_free(scene->allocator, scene));
}

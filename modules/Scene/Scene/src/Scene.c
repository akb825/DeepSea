/*
 * Copyright 2019-2021 Aaron Barany
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
#include <DeepSea/Scene/SceneGlobalData.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/SceneRenderPass.h>

#include <string.h>

static void destroyObjects(const dsSceneItemLists* sharedItems, uint32_t sharedItemCount,
	const dsScenePipelineItem* pipeline, uint32_t pipelineCount,
	dsSceneGlobalData* const* globalData, uint32_t globalDataCount, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc)
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

	if (globalData)
	{
		for (uint32_t i = 0; i < globalDataCount; ++i)
			dsSceneGlobalData_destroy(globalData[i]);
	}

	if (destroyUserDataFunc)
		destroyUserDataFunc(userData);
}

static size_t fullAllocSize(uint32_t* outNameCount, const dsSceneItemLists* sharedItems,
	uint32_t sharedItemCount, const dsScenePipelineItem* pipeline, uint32_t pipelineCount,
	dsSceneGlobalData* const* globalData, uint32_t globalDataCount)
{
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsScene)) +
		DS_ALIGNED_SIZE(sizeof(dsSceneItemList)*sharedItemCount) +
		DS_ALIGNED_SIZE(sizeof(dsScenePipelineItem)*pipelineCount) +
		DS_ALIGNED_SIZE(sizeof(dsSceneGlobalData*)*globalDataCount);

	*outNameCount = 0;
	for (uint32_t i = 0; i < sharedItemCount; ++i)
	{
		const dsSceneItemLists* itemLists = sharedItems + i;
		if (sharedItems[i].count > 0 && !itemLists->itemLists)
			return 0;

		for (uint32_t j = 0; j < itemLists->count; ++j)
		{
			if (!itemLists->itemLists[j])
				return 0;
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
					if (!items->itemLists[k])
						return 0;
				}
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

static void dummyDestroyFunc(dsSceneNode* node)
{
	DS_UNUSED(node);
}

dsScene* dsScene_loadImpl(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData, const void* data,
	size_t dataSize, void* userData, dsDestroySceneUserDataFunction destroyUserDataFunc,
	const char* fileName);

dsScene* dsScene_create(dsAllocator* allocator, dsRenderer* renderer,
	const dsSceneItemLists* sharedItems, uint32_t sharedItemCount,
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

	DS_VERIFY(dsSceneNode_initialize(&scene->rootNode, allocator, &dsRootSceneNodeType, NULL, 0,
		&dummyDestroyFunc));

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
	else
		scene->globalData = NULL;
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
	for (uint32_t i = 0; i < sharedItemCount; ++i)
	{
		const dsSceneItemLists* itemLists = sharedItems + i;
		for (uint32_t j = 0; j < itemLists->count; ++j)
		{
			dsSceneItemListNode* node = itemNodes + curItems++;
			if (!insertSceneList(scene->itemLists, node, sharedItems[i].itemLists[j]))
			{
				errno = EINVAL;
				dsScene_destroy(scene);
				return NULL;
			}
		}
	}

	for (uint32_t i = 0; i < pipelineCount; ++i)
	{
		const dsScenePipelineItem* item = pipeline  + i;
		if (item->renderPass)
		{
			for (uint32_t j = 0; j < item->renderPass->renderPass->subpassCount; ++j)
			{
				const dsSceneItemLists* items = item->renderPass->drawLists + j;
				for (uint32_t k = 0; k < items->count; ++k)
				{
					dsSceneItemListNode* node = itemNodes + curItems++;
					if (!insertSceneList(scene->itemLists, node, items->itemLists[k]))
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
	DS_ASSERT(curItems == nameCount);

	return scene;
}

dsScene* dsScene_loadFile(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData,
	void* userData, dsDestroySceneUserDataFunction destroyUserDataFunc, const char* filePath)
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
		buffer, size, userData, destroyUserDataFunc, filePath);
	DS_VERIFY(dsSceneLoadScratchData_freeReadBuffer(scratchData, buffer));
	DS_PROFILE_FUNC_RETURN(scene);
}

dsScene* dsScene_loadResource(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData,
	void* userData, dsDestroySceneUserDataFunction destroyUserDataFunc, dsFileResourceType type,
	const char* filePath)
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
		buffer, size, userData, destroyUserDataFunc, filePath);
	DS_VERIFY(dsSceneLoadScratchData_freeReadBuffer(scratchData, buffer));
	DS_PROFILE_FUNC_RETURN(scene);
}

dsScene* dsScene_loadStream(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc, dsStream* stream)
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
		buffer, size, userData, destroyUserDataFunc, NULL);
	DS_VERIFY(dsSceneLoadScratchData_freeReadBuffer(scratchData, buffer));
	DS_PROFILE_FUNC_RETURN(scene);
}

dsScene* dsScene_loadData(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc, const void* data, size_t size)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !loadContext || !scratchData || !data || size == 0)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsScene* scene = dsScene_loadImpl(allocator, resourceAllocator, loadContext, scratchData, data,
		size, userData, destroyUserDataFunc, NULL);
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

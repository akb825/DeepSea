/*
 * Copyright 2019-2026 Aaron Barany
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

#include <DeepSea/Scene/ItemLists/SceneModelList.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Core/Sort.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Geometry/OrientedBox3.h>
#include <DeepSea/Geometry/Frustum3.h>

#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/Renderer.h>

#include <DeepSea/Scene/ItemLists/SceneInstanceData.h>
#include <DeepSea/Scene/ItemLists/SceneItemListEntries.h>
#include <DeepSea/Scene/Nodes/SceneModelNode.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneNodeItemData.h>

#include <stdlib.h>
#include <string.h>

typedef struct Entry
{
	const dsSceneModelNode* node;
	const dsSceneTreeNode* treeNode;
	const dsMatrix44f* transform;
	const dsSceneNodeItemData* itemData;
	uint64_t nodeID;
} Entry;

typedef struct DrawItem
{
	dsShader* shader;
	dsMaterial* material;
	dsDrawGeometry* geometry;
	uint32_t instance;
	float flatDistance;

	const dsSceneModelDrawRange* drawRanges;
	uint32_t drawRangeCount;
	dsPrimitiveType primitiveType;
} DrawItem;

struct dsSceneModelList
{
	dsSceneItemList itemList;

	dsDynamicRenderStates renderStates;
	bool hasRenderStates;
	dsModelSortType sortType;

	dsSharedMaterialValues* instanceValues;
	dsSceneInstanceData** instanceData;
	uint32_t* cullListIDs;
	uint32_t* viewIDs;
	uint32_t instanceDataCount;
	uint32_t cullListCount;
	uint32_t viewCount;

	Entry* entries;
	uint32_t entryCount;
	uint32_t maxEntries;
	uint64_t nextNodeID;

	uint64_t* removeEntries;
	uint32_t removeEntryCount;
	uint32_t maxRemoveEntries;

	const dsSceneTreeNode** instances;
	uint32_t instanceCount;
	uint32_t maxInstances;

	DrawItem* drawItems;
	uint32_t drawItemCount;
	uint32_t maxDrawItems;
};

static bool isInView(const dsSceneModelList* modelList, const dsView* view)
{
	for (uint32_t i = 0; i < modelList->viewCount; ++i)
	{
		if (modelList->viewIDs[i] == view->nameID)
			return true;
	}
	return modelList->viewCount == 0;
}

static void addInstances(dsSceneItemList* itemList, const dsView* view)
{
	DS_PROFILE_FUNC_START();

	dsSceneModelList* modelList = (dsSceneModelList*)itemList;
	modelList->instanceCount = 0;
	modelList->drawItemCount = 0;

	for (uint32_t i = 0; i < modelList->entryCount; ++i)
	{
		const Entry* entry = modelList->entries + i;
		const dsSceneModelNode* modelNode = entry->node;
		bool culled = false;
		for (uint32_t j = 0; j < modelList->cullListCount; ++j)
		{
			// Non-zero cull result means out of view.
			if (dsSceneNodeItemData_findID(entry->itemData, modelList->cullListIDs[j]))
			{
				culled = true;
				break;
			}
		}
		if (culled)
			continue;

		// Use the dist2 macro to take the first 3 vectors of the dsVector4f columns.
		float distance = sqrtf(dsVector3_dist2(entry->transform->columns[3],
			view->cameraMatrix.columns[3]))*view->lodBias;

		dsVector3f direction;
		dsVector3_sub(direction, entry->transform->columns[3], view->cameraMatrix.columns[3]);
		float flatDistance = -dsVector3_dot(direction, view->cameraMatrix.columns[2]);

		bool hasAny = false;
		uint32_t instanceIndex = modelList->instanceCount;
		for (uint32_t j = 0; j < modelNode->modelCount; ++j)
		{
			dsSceneModelInfo* model = modelNode->models + j;
			if (model->modelListID != itemList->nameID)
				continue;

			if (model->distanceRange.x <= model->distanceRange.y &&
				(distance < model->distanceRange.x || distance >= model->distanceRange.y))
			{
				continue;
			}

			uint32_t itemIndex = modelList->drawItemCount;
			if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, modelList->drawItems,
					modelList->drawItemCount, modelList->maxDrawItems, 1))
			{
				continue;
			}

			hasAny = true;

			DrawItem* item = modelList->drawItems + itemIndex;
			item->shader = model->shader;
			item->material = model->material;
			item->geometry = model->geometry;
			item->instance = instanceIndex;
			item->flatDistance = flatDistance;

			item->drawRanges = model->drawRanges;
			item->drawRangeCount = model->drawRangeCount;
			item->primitiveType = model->primitiveType;
		}

		if (!hasAny)
			continue;

		if (!DS_CHECK(DS_SCENE_LOG_TAG, DS_RESIZEABLE_ARRAY_ADD(itemList->allocator,
				modelList->instances, modelList->instanceCount, modelList->maxInstances, 1)))
		{
			--modelList->instanceCount;
			DS_PROFILE_FUNC_RETURN_VOID();
		}

		modelList->instances[instanceIndex] = entry->treeNode;
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}

static void setupInstances(
	dsSceneModelList* modelList, const dsView* view, dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	for (uint32_t i = 0; i < modelList->instanceDataCount; ++i)
	{
		DS_CHECK(DS_SCENE_LOG_TAG, dsSceneInstanceData_populateData(modelList->instanceData[i],
			view, commandBuffer, modelList->instances, modelList->instanceCount));
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}

static inline int sortByMaterial(const void* left, const void* right)
{
	const DrawItem* leftInfo = (const DrawItem*)left;
	const DrawItem* rightInfo = (const DrawItem*)right;
	int shaderCmp = DS_CMP(leftInfo->shader, rightInfo->shader);
	int materialCmp = DS_CMP(leftInfo->material, rightInfo->material);
	int geometryCmp = DS_CMP(leftInfo->geometry, rightInfo->geometry);
	// Small enough that subtract should be safe.
	int instanceCmp = leftInfo->instance - rightInfo->instance;

	int result = dsCombineCmp(shaderCmp, materialCmp);
	result = dsCombineCmp(result, geometryCmp);
	return dsCombineCmp(result, instanceCmp);
}

static int sortBackToFront(const void* left, const void* right)
{
	const DrawItem* leftInfo = (const DrawItem*)left;
	const DrawItem* rightInfo = (const DrawItem*)right;
	return DS_CMP(rightInfo->flatDistance, leftInfo->flatDistance);
}

static int sortFrontToBack(const void* left, const void* right)
{
	const DrawItem* leftInfo = (const DrawItem*)left;
	const DrawItem* rightInfo = (const DrawItem*)right;
	return DS_CMP(leftInfo->flatDistance, rightInfo->flatDistance);
}

static void sortGeometry(dsSceneModelList* modelList)
{
	DS_PROFILE_FUNC_START();

	switch (modelList->sortType)
	{
		case dsModelSortType_Material:
			qsort(modelList->drawItems, modelList->drawItemCount, sizeof(DrawItem),
				&sortByMaterial);
			break;
		case dsModelSortType_BackToFront:
			qsort(modelList->drawItems, modelList->drawItemCount, sizeof(DrawItem),
				&sortBackToFront);
			break;
		case dsModelSortType_FrontToBack:
			qsort(modelList->drawItems, modelList->drawItemCount, sizeof(DrawItem),
				&sortFrontToBack);
			break;
		default:
			break;
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}

static void drawGeometry(
	dsSceneModelList* modelList, const dsView* view, dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	dsRenderer* renderer = commandBuffer->renderer;
	dsShader* lastShader = NULL;
	dsMaterial* lastMaterial = NULL;
	uint32_t lastInstance = (uint32_t)-1;
	dsDynamicRenderStates* renderStates =
		modelList->hasRenderStates ? &modelList->renderStates : NULL;
	bool hasInstances = modelList->instanceDataCount > 0;
	for (uint32_t i = 0; i < modelList->drawItemCount; ++i)
	{
		const DrawItem* drawItem = modelList->drawItems + i;
		bool updateInstances = false;
		if (drawItem->shader != lastShader || drawItem->material != lastMaterial)
		{
			if (lastShader)
				dsShader_unbind(lastShader, commandBuffer);

			if (!DS_CHECK(DS_SCENE_LOG_TAG, dsShader_bind(drawItem->shader, commandBuffer,
					drawItem->material, view->globalValues, renderStates)))
			{
				continue;
			}

			lastShader = drawItem->shader;
			lastMaterial = drawItem->material;
			updateInstances = hasInstances;
		}

		if (drawItem->instance != lastInstance && hasInstances)
		{
			for (uint32_t j = 0; j < modelList->instanceDataCount; ++j)
			{
				DS_CHECK(DS_SCENE_LOG_TAG, dsSceneInstanceData_bindInstance(
					modelList->instanceData[j], drawItem->instance, modelList->instanceValues));
			}

			lastInstance = drawItem->instance;
			updateInstances = true;
		}

		if (updateInstances)
		{
			DS_CHECK(DS_SCENE_LOG_TAG, dsShader_updateInstanceValues(drawItem->shader,
				commandBuffer, modelList->instanceValues));
		}

		if (drawItem->geometry->indexBuffer.buffer)
		{
			for (uint32_t j = 0; j < drawItem->drawRangeCount; ++j)
			{
				DS_CHECK(DS_SCENE_LOG_TAG, dsRenderer_drawIndexed(renderer, commandBuffer,
					drawItem->geometry, &drawItem->drawRanges[j].drawIndexedRange,
					drawItem->primitiveType));
			}
		}
		else
		{
			for (uint32_t j = 0; j < drawItem->drawRangeCount; ++j)
			{
				DS_CHECK(DS_SCENE_LOG_TAG, dsRenderer_draw(renderer, commandBuffer,
					drawItem->geometry, &drawItem->drawRanges[j].drawRange,
					drawItem->primitiveType));
			}
		}
	}

	if (lastShader)
		dsShader_unbind(lastShader, commandBuffer);

	DS_PROFILE_FUNC_RETURN_VOID();
}

static void cleanup(dsSceneModelList* modelList)
{
	for (uint32_t i = 0; i < modelList->instanceDataCount; ++i)
		dsSceneInstanceData_finish(modelList->instanceData[i]);
}

static void destroyInstanceData(
	dsSceneInstanceData* const* instanceData, uint32_t instanceDataCount)
{
	for (uint32_t i = 0; i < instanceDataCount; ++i)
		dsSceneInstanceData_destroy(instanceData[i]);
}

static uint64_t dsSceneModelList_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_ASSERT(itemList);
	DS_UNUSED(thisItemData);
	if (!dsSceneNode_isOfType(node, dsSceneModelNode_type()))
		return DS_NO_SCENE_NODE;

	const dsSceneModelNode* modelNode = (const dsSceneModelNode*)node;
	for (uint32_t i = 0; i < modelNode->modelCount; ++i)
	{
		dsSceneModelInfo* model = modelNode->models + i;
		if (!model->shader || !model->material)
			return DS_NO_SCENE_NODE;
	}

	dsSceneModelList* modelList = (dsSceneModelList*)itemList;

	uint32_t index = modelList->entryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, modelList->entries, modelList->entryCount,
			modelList->maxEntries, 1))
	{
		return DS_NO_SCENE_NODE;
	}

	Entry* entry = modelList->entries + index;
	entry->node = modelNode;
	entry->treeNode = treeNode;
	entry->transform = &treeNode->transform;
	entry->itemData = itemData;
	entry->nodeID = modelList->nextNodeID++;
	return entry->nodeID;
}

static void dsSceneModelList_removeNode(
	dsSceneItemList* itemList, dsSceneTreeNode* treeNode, uint64_t nodeID)
{
	DS_ASSERT(itemList);
	DS_UNUSED(treeNode);
	dsSceneModelList* modelList = (dsSceneModelList*)itemList;

	uint32_t index = modelList->removeEntryCount;
	if (DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, modelList->removeEntries,
			modelList->removeEntryCount, modelList->maxRemoveEntries, 1))
	{
		modelList->removeEntries[index] = nodeID;
	}
	else
	{
		dsSceneItemListEntries_removeSingle(modelList->entries, &modelList->entryCount,
			sizeof(Entry), offsetof(Entry, nodeID), nodeID);
	}
}

static void dsSceneModelList_preRenderPass(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(itemList);
	DS_ASSERT(!itemList->skipPreRenderPass);
	dsSceneModelList* modelList = (dsSceneModelList*)itemList;
	if (!isInView(modelList, view))
		return;

	dsRenderer_pushDebugGroup(commandBuffer->renderer, commandBuffer, itemList->name);

	// Lazily remove entries.
	dsSceneItemListEntries_removeMulti(modelList->entries, &modelList->entryCount, sizeof(Entry),
		offsetof(Entry, nodeID), modelList->removeEntries, modelList->removeEntryCount);
	modelList->removeEntryCount = 0;

	addInstances(itemList, view);
	setupInstances(modelList, view, commandBuffer);

	dsRenderer_popDebugGroup(commandBuffer->renderer, commandBuffer);
}

static void dsSceneModelList_commit(
	dsSceneItemList* itemList, const dsView* view, dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(itemList);
	dsSceneModelList* modelList = (dsSceneModelList*)itemList;
	if (!isInView(modelList, view))
		return;

	dsRenderer_pushDebugGroup(commandBuffer->renderer, commandBuffer, itemList->name);

	if (itemList->skipPreRenderPass)
	{
		// Lazily remove entries.
		dsSceneItemListEntries_removeMulti(modelList->entries, &modelList->entryCount, sizeof(Entry),
			offsetof(Entry, nodeID), modelList->removeEntries, modelList->removeEntryCount);
		modelList->removeEntryCount = 0;

		addInstances(itemList, view);
		setupInstances(modelList, view, NULL);
	}
	sortGeometry(modelList);
	drawGeometry(modelList, view, commandBuffer);
	cleanup(modelList);

	dsRenderer_popDebugGroup(commandBuffer->renderer, commandBuffer);
}

static uint32_t dsSceneModelList_hash(const dsSceneItemList* itemList, uint32_t commonHash)
{
	DS_ASSERT(itemList);
	const dsSceneModelList* modelList = (const dsSceneModelList*)itemList;
	uint32_t hash = commonHash;
	if (modelList->hasRenderStates)
		hash = dsHashCombineBytes(hash, &modelList->renderStates, sizeof(dsDynamicRenderStates));
	hash = dsHashCombine32(hash, &modelList->sortType);
	for (uint32_t i = 0; i < modelList->instanceDataCount; ++i)
		hash = dsSceneInstanceData_hash(modelList->instanceData[i], hash);
	hash = dsHashCombineBytes(
		hash, modelList->cullListIDs, sizeof(uint32_t)*modelList->cullListCount);
	hash = dsHashCombineBytes(hash, modelList->viewIDs, sizeof(uint32_t)*modelList->viewCount);
	return hash;
}

static bool dsSceneModelList_equal(const dsSceneItemList* left, const dsSceneItemList* right)
{
	DS_ASSERT(left);
	DS_ASSERT(left->type == dsSceneModelList_type());
	DS_ASSERT(right);
	DS_ASSERT(right->type == dsSceneModelList_type());

	const dsSceneModelList* leftModelList = (const dsSceneModelList*)left;
	const dsSceneModelList* rightModelList = (const dsSceneModelList*)right;

	if (leftModelList->hasRenderStates != rightModelList->hasRenderStates ||
		(leftModelList->hasRenderStates && memcmp(&leftModelList->renderStates,
			&rightModelList->renderStates, sizeof(dsDynamicRenderStates)) != 0) ||
		leftModelList->sortType != rightModelList->sortType ||
		leftModelList->instanceDataCount != rightModelList->instanceDataCount ||
		leftModelList->cullListCount != rightModelList->cullListCount ||
		leftModelList->viewCount != rightModelList->viewCount)
	{
		return false;
	}

	for (uint32_t i = 0; i < leftModelList->instanceDataCount; ++i)
	{
		if (!dsSceneInstanceData_equal(
				leftModelList->instanceData[i], rightModelList->instanceData[i]))
		{
			return false;
		}
	}

	for (uint32_t i = 0; i < leftModelList->cullListCount; ++i)
	{
		if (leftModelList->cullListIDs[i] != rightModelList->cullListIDs[i])
			return false;
	}

	for (uint32_t i = 0; i < leftModelList->viewCount; ++i)
	{
		if (leftModelList->viewIDs[i] != rightModelList->viewIDs[i])
			return false;
	}
	return true;
}

static void dsSceneModelList_destroy(dsSceneItemList* itemList)
{
	DS_ASSERT(itemList);
	dsSceneModelList* modelList = (dsSceneModelList*)itemList;
	destroyInstanceData(modelList->instanceData, modelList->instanceDataCount);
	dsSharedMaterialValues_destroy(modelList->instanceValues);
	DS_VERIFY(dsAllocator_free(itemList->allocator, modelList->entries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, modelList->removeEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, (void*)modelList->instances));
	DS_VERIFY(dsAllocator_free(itemList->allocator, modelList->drawItems));
	DS_VERIFY(dsAllocator_free(itemList->allocator, modelList));
}

const char* const dsSceneModelList_typeName = "ModelList";

static dsSceneItemListType itemListType =
{
	.addNodeFunc = &dsSceneModelList_addNode,
	.removeNodeFunc = &dsSceneModelList_removeNode,
	.preRenderPassFunc = &dsSceneModelList_preRenderPass,
	.commitFunc = &dsSceneModelList_commit,
	.hashFunc = &dsSceneModelList_hash,
	.equalFunc = &dsSceneModelList_equal,
	.destroyFunc = &dsSceneModelList_destroy
};

const dsSceneItemListType* dsSceneModelList_type(void)
{
	return &itemListType;
}

dsSceneModelList* dsSceneModelList_create(dsAllocator* allocator, const char* name,
	dsSceneInstanceData* const* instanceData, uint32_t instanceDataCount, dsModelSortType sortType,
	const dsDynamicRenderStates* renderStates, const char* const* cullLists, uint32_t cullListCount,
	const char* const* views, uint32_t viewCount)
{
	if (!allocator || !name || (!instanceData && instanceDataCount > 0) ||
		(!cullLists && cullListCount > 0) || (!views && viewCount > 0))
	{
		errno = EINVAL;
		if (instanceData)
			destroyInstanceData(instanceData, instanceDataCount);
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene model list allocator must support freeing memory.");
		destroyInstanceData(instanceData, instanceDataCount);
		return NULL;
	}

	uint32_t valueCount = 0;
	bool skipPreRenderPass = true;
	for (uint32_t i = 0; i < instanceDataCount; ++i)
	{
		if (!instanceData[i])
		{
			errno = EINVAL;
			destroyInstanceData(instanceData, instanceDataCount);
			return NULL;
		}
		valueCount += instanceData[i]->valueCount;
		if (instanceData[i]->needsCommandBuffer)
			skipPreRenderPass = false;
	}

	for (uint32_t i = 0; i < cullListCount; ++i)
	{
		if (!cullLists[i])
		{
			errno = EINVAL;
			destroyInstanceData(instanceData, instanceDataCount);
			return NULL;
		}
	}

	for (uint32_t i = 0; i < viewCount; ++i)
	{
		if (!views[i])
		{
			errno = EINVAL;
			destroyInstanceData(instanceData, instanceDataCount);
			return NULL;
		}
	}

	size_t nameLen = strlen(name);
	size_t instanceDataSize = valueCount > 0 ? dsSharedMaterialValues_fullAllocSize(valueCount) : 0;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneModelList)) + DS_ALIGNED_SIZE(nameLen + 1) +
		DS_ALIGNED_SIZE(sizeof(dsSceneInstanceData*)*instanceDataCount) + instanceDataSize +
		DS_ALIGNED_SIZE(sizeof(uint32_t)*cullListCount) +
		DS_ALIGNED_SIZE(sizeof(uint32_t)*viewCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		destroyInstanceData(instanceData, instanceDataCount);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsSceneModelList* modelList = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneModelList);
	DS_ASSERT(modelList);

	dsSceneItemList* itemList = (dsSceneItemList*)modelList;
	itemList->allocator = allocator;
	itemList->type = dsSceneModelList_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen + 1);
	memcpy((void*)itemList->name, name, nameLen + 1);
	itemList->nameID = dsUniqueNameID_create(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = true;
	itemList->skipPreRenderPass = skipPreRenderPass;

	if (renderStates)
	{
		modelList->renderStates = *renderStates;
		modelList->hasRenderStates = true;
	}
	else
		modelList->hasRenderStates = false;
	modelList->sortType = sortType;

	if (instanceDataCount > 0)
	{
		if (valueCount > 0)
		{
			modelList->instanceValues = dsSharedMaterialValues_create(
				(dsAllocator*)&bufferAlloc, valueCount);
			DS_ASSERT(modelList->instanceValues);
		}
		else
			modelList->instanceValues = NULL;
		modelList->instanceData = DS_ALLOCATE_OBJECT_ARRAY(
			&bufferAlloc, dsSceneInstanceData*, instanceDataCount);
		DS_ASSERT(modelList->instanceData);
		memcpy(modelList->instanceData, instanceData,
			sizeof(dsSceneInstanceData*)*instanceDataCount);
	}
	else
	{
		modelList->instanceValues = NULL;
		modelList->instanceData = NULL;
	}
	modelList->instanceDataCount = instanceDataCount;

	if (cullListCount > 0)
	{
		modelList->cullListIDs = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, uint32_t, cullListCount);
		DS_ASSERT(modelList->cullListIDs);
		for (uint32_t i = 0; i < cullListCount; ++i)
			modelList->cullListIDs[i] = dsUniqueNameID_create(cullLists[i]);
	}
	else
		modelList->cullListIDs = NULL;
	modelList->cullListCount = cullListCount;

	if (viewCount > 0)
	{
		modelList->viewIDs = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, uint32_t, viewCount);
		DS_ASSERT(modelList->viewIDs);
		for (uint32_t i = 0; i < viewCount; ++i)
			modelList->viewIDs[i] = dsUniqueNameID_create(views[i]);
	}
	else
		modelList->viewIDs = NULL;
	modelList->viewCount = viewCount;

	modelList->entries = NULL;
	modelList->entryCount = 0;
	modelList->maxEntries = 0;
	modelList->nextNodeID = 0;

	modelList->removeEntries = NULL;
	modelList->removeEntryCount = 0;
	modelList->maxRemoveEntries = 0;

	modelList->instances = NULL;
	modelList->instanceCount = 0;
	modelList->maxInstances = 0;

	modelList->drawItems = NULL;
	modelList->drawItemCount = 0;
	modelList->maxDrawItems = 0;

	return modelList;
}

dsModelSortType dsSceneModelList_getSortType(const dsSceneModelList* modelList)
{
	return modelList ? modelList->sortType : dsModelSortType_None;
}

void dsSceneModelList_setSortType(dsSceneModelList* modelList, dsModelSortType sortType)
{
	if (modelList)
		modelList->sortType = sortType;
}

const dsDynamicRenderStates* dsSceneModelList_getRenderStates(const dsSceneModelList* modelList)
{
	return modelList && modelList->hasRenderStates ? &modelList->renderStates : NULL;
}

void dsSceneModelList_setRenderStates(
	dsSceneModelList* modelList, const dsDynamicRenderStates* renderStates)
{
	if (!modelList)
		return;

	if (renderStates)
	{
		modelList->hasRenderStates = true;
		modelList->renderStates = *renderStates;
	}
	else
		modelList->hasRenderStates = false;
}

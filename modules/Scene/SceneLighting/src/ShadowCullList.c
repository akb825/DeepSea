/*
 * Copyright 2021-2025 Aaron Barany
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

#include <DeepSea/SceneLighting/ShadowCullList.h>

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Geometry/OrientedBox3.h>
#include <DeepSea/Geometry/Frustum3.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Scene/ItemLists/SceneItemListEntries.h>
#include <DeepSea/Scene/Nodes/SceneCullNode.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <DeepSea/SceneLighting/SceneLightShadows.h>

#include <limits.h>
#include <string.h>

#define MIN_DYNAMIC_ENTRY_ID LLONG_MAX

typedef struct StaticEntry
{
	dsMatrix44f localBoxMatrix;
	const dsMatrix44f* transform;
	bool* result;
	uint64_t nodeID;
} StaticEntry;

typedef struct DynamicEntry
{
	const dsSceneCullNode* node;
	const dsSceneTreeNode* treeNode;
	bool* result;
	uint64_t nodeID;
} DynamicEntry;

typedef struct dsShadowCullList
{
	dsSceneItemList itemList;

	dsSceneLightShadows* shadows;
	uint32_t surface;

	StaticEntry* staticEntries;
	uint32_t staticEntryCount;
	uint32_t maxStaticEntries;
	uint64_t nextStaticNodeID;

	uint64_t* removeStaticEntries;
	uint32_t removeStaticEntryCount;
	uint32_t maxRemoveStaticEntries;

	DynamicEntry* dynamicEntries;
	uint32_t dynamicEntryCount;
	uint32_t maxDynamicEntries;
	uint64_t nextDynamicNodeID;

	uint64_t* removeDynamicEntries;
	uint32_t removeDynamicEntryCount;
	uint32_t maxRemoveDynamicEntries;
} dsShadowCullList;

static uint64_t dsShadowCullList_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(itemData);
	if (!dsSceneNode_isOfType(node, dsSceneCullNode_type()))
		return DS_NO_SCENE_NODE;

	dsShadowCullList* cullList = (dsShadowCullList*)itemList;
	const dsSceneCullNode* cullNode = (const dsSceneCullNode*)node;
	if (!cullNode->hasBounds)
		return DS_NO_SCENE_NODE;

	if (cullNode->getBoundsFunc)
	{
		uint32_t index = cullList->dynamicEntryCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, cullList->dynamicEntries,
				cullList->dynamicEntryCount, cullList->maxDynamicEntries, 1))
		{
			return DS_NO_SCENE_NODE;
		}

		DynamicEntry* entry = cullList->dynamicEntries + index;
		entry->node = cullNode;
		entry->treeNode = treeNode;
		entry->result = (bool*)thisItemData;
		entry->nodeID = cullList->nextDynamicNodeID++;
		return entry->nodeID;
	}

	uint32_t index = cullList->staticEntryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, cullList->staticEntries,
			cullList->staticEntryCount, cullList->maxStaticEntries, 1))
	{
		return DS_NO_SCENE_NODE;
	}

	StaticEntry* entry = cullList->staticEntries + index;
	entry->localBoxMatrix = cullNode->staticLocalBoxMatrix;
	entry->transform = &treeNode->transform;
	entry->result = (bool*)thisItemData;
	entry->nodeID = cullList->nextStaticNodeID++;
	return entry->nodeID;
}

static void dsShadowCullList_removeNode(
	dsSceneItemList* itemList, dsSceneTreeNode* treeNode, uint64_t nodeID)
{
	DS_UNUSED(treeNode);
	dsShadowCullList* cullList = (dsShadowCullList*)itemList;
	if (nodeID < MIN_DYNAMIC_ENTRY_ID)
	{
		uint32_t index = cullList->removeStaticEntryCount;
		if (DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, cullList->removeStaticEntries,
				cullList->removeStaticEntryCount, cullList->maxRemoveStaticEntries, 1))
		{
			cullList->removeStaticEntries[index] = nodeID;
		}
		else
		{
			dsSceneItemListEntries_removeSingle(cullList->staticEntries,
				&cullList->staticEntryCount, sizeof(StaticEntry), offsetof(StaticEntry, nodeID),
				nodeID);
		}
	}
	else
	{
		uint32_t index = cullList->removeDynamicEntryCount;
		if (DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, cullList->removeDynamicEntries,
				cullList->removeDynamicEntryCount, cullList->maxRemoveDynamicEntries, 1))
		{
			cullList->removeDynamicEntries[index] = nodeID;
		}
		else
		{
			dsSceneItemListEntries_removeSingle(cullList->dynamicEntries,
				&cullList->dynamicEntryCount, sizeof(DynamicEntry), offsetof(DynamicEntry, nodeID),
				nodeID);
		}
	}
}

static void lazyRemoveEntries(dsShadowCullList* cullList)
{
	dsSceneItemListEntries_removeMulti(cullList->staticEntries, &cullList->staticEntryCount,
		sizeof(StaticEntry), offsetof(StaticEntry, nodeID), cullList->removeStaticEntries,
		cullList->removeStaticEntryCount);
	cullList->removeStaticEntryCount = 0;

	dsSceneItemListEntries_removeMulti(cullList->dynamicEntries, &cullList->dynamicEntryCount,
		sizeof(DynamicEntry), offsetof(DynamicEntry, nodeID), cullList->removeDynamicEntries,
		cullList->removeDynamicEntryCount);
	cullList->removeDynamicEntryCount = 0;
}

#if DS_HAS_SIMD
DS_SIMD_START(DS_SIMD_FLOAT4)
static void dsShadowCullList_commitSIMD(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(commandBuffer);
	dsShadowCullList* cullList = (dsShadowCullList*)itemList;
	lazyRemoveEntries(cullList);

	if (cullList->surface >= dsSceneLightShadows_getSurfaceCount(cullList->shadows))
	{
		for (uint32_t i = 0; i < cullList->staticEntryCount; ++i)
		{
			const StaticEntry* entry = cullList->staticEntries + i;
			*entry->result = true;
		}
		for (uint32_t i = 0; i < cullList->dynamicEntryCount; ++i)
		{
			const DynamicEntry* entry = cullList->dynamicEntries + i;
			*entry->result = true;
		}
		return;
	}

	for (uint32_t i = 0; i < cullList->staticEntryCount; ++i)
	{
		const StaticEntry* entry = cullList->staticEntries + i;
		dsMatrix44f boxMatrix;
		dsMatrix44f_affineMulSIMD(&boxMatrix, entry->transform, &entry->localBoxMatrix);
		*entry->result = dsSceneLightShadows_intersectBoxMatrixSIMD(cullList->shadows,
			cullList->surface, &boxMatrix) == dsIntersectResult_Outside;
	}

	for (uint32_t i = 0; i < cullList->dynamicEntryCount; ++i)
	{
		const DynamicEntry* entry = cullList->dynamicEntries + i;
		dsMatrix44f boxMatrix;
		if (entry->node->getBoundsFunc(&boxMatrix, entry->node, entry->treeNode))
		{
			*entry->result = dsSceneLightShadows_intersectBoxMatrixSIMD(cullList->shadows,
				cullList->surface, &boxMatrix) == dsIntersectResult_Outside;
		}
		else
			*entry->result = true;
	}

	if (!dsSceneLightShadows_computeSurfaceProjection(cullList->shadows, cullList->surface))
	{
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
			"Couldn't compute projection for shadows '%s' surface %d.",
			dsSceneLightShadows_getName(cullList->shadows), cullList->surface);
	}
}
DS_SIMD_END()

DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_FMA)
static void dsShadowCullList_commitFMA(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(commandBuffer);
	dsShadowCullList* cullList = (dsShadowCullList*)itemList;
	lazyRemoveEntries(cullList);

	if (cullList->surface >= dsSceneLightShadows_getSurfaceCount(cullList->shadows))
	{
		for (uint32_t i = 0; i < cullList->staticEntryCount; ++i)
		{
			const StaticEntry* entry = cullList->staticEntries + i;
			*entry->result = true;
		}
		for (uint32_t i = 0; i < cullList->dynamicEntryCount; ++i)
		{
			const DynamicEntry* entry = cullList->dynamicEntries + i;
			*entry->result = true;
		}
		return;
	}

	for (uint32_t i = 0; i < cullList->staticEntryCount; ++i)
	{
		const StaticEntry* entry = cullList->staticEntries + i;
		dsMatrix44f boxMatrix;
		dsMatrix44f_affineMulFMA(&boxMatrix, entry->transform, &entry->localBoxMatrix);
		*entry->result = dsSceneLightShadows_intersectBoxMatrixFMA(cullList->shadows,
			cullList->surface, &boxMatrix) == dsIntersectResult_Outside;
	}

	for (uint32_t i = 0; i < cullList->dynamicEntryCount; ++i)
	{
		const DynamicEntry* entry = cullList->dynamicEntries + i;
		dsMatrix44f boxMatrix;
		if (entry->node->getBoundsFunc(&boxMatrix, entry->node, entry->treeNode))
		{
			*entry->result = dsSceneLightShadows_intersectBoxMatrixFMA(cullList->shadows,
				cullList->surface, &boxMatrix) == dsIntersectResult_Outside;
		}
		else
			*entry->result = true;
	}

	if (!dsSceneLightShadows_computeSurfaceProjection(cullList->shadows, cullList->surface))
	{
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
			"Couldn't compute projection for shadows '%s' surface %d.",
			dsSceneLightShadows_getName(cullList->shadows), cullList->surface);
	}
}
DS_SIMD_END()
#endif

static void dsShadowCullList_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(commandBuffer);
	dsShadowCullList* cullList = (dsShadowCullList*)itemList;
	lazyRemoveEntries(cullList);

	if (cullList->surface >= dsSceneLightShadows_getSurfaceCount(cullList->shadows))
	{
		for (uint32_t i = 0; i < cullList->staticEntryCount; ++i)
		{
			const StaticEntry* entry = cullList->staticEntries + i;
			*entry->result = true;
		}
		for (uint32_t i = 0; i < cullList->dynamicEntryCount; ++i)
		{
			const DynamicEntry* entry = cullList->dynamicEntries + i;
			*entry->result = true;
		}
		return;
	}

	for (uint32_t i = 0; i < cullList->staticEntryCount; ++i)
	{
		const StaticEntry* entry = cullList->staticEntries + i;
		dsMatrix44f boxMatrix;
		dsMatrix44f_affineMul(&boxMatrix, entry->transform, &entry->localBoxMatrix);
		*entry->result = dsSceneLightShadows_intersectBoxMatrix(cullList->shadows,
			cullList->surface, &boxMatrix) == dsIntersectResult_Outside;
	}

	for (uint32_t i = 0; i < cullList->dynamicEntryCount; ++i)
	{
		const DynamicEntry* entry = cullList->dynamicEntries + i;
		dsMatrix44f boxMatrix;
		if (entry->node->getBoundsFunc(&boxMatrix, entry->node, entry->treeNode))
		{
			*entry->result = dsSceneLightShadows_intersectBoxMatrix(cullList->shadows,
				cullList->surface, &boxMatrix) == dsIntersectResult_Outside;
		}
		else
			*entry->result = true;
	}

	if (!dsSceneLightShadows_computeSurfaceProjection(cullList->shadows, cullList->surface))
	{
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
			"Couldn't compute projection for shadows '%s' surface %d.",
			dsSceneLightShadows_getName(cullList->shadows), cullList->surface);
	}
}

static void dsShadowCullList_destroy(dsSceneItemList* itemList)
{
	dsShadowCullList* cullList = (dsShadowCullList*)itemList;
	DS_VERIFY(dsAllocator_free(itemList->allocator, cullList->staticEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, cullList->removeStaticEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, cullList->dynamicEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, cullList->removeDynamicEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

const char* const dsShadowCullList_typeName = "ShadowCullList";

dsSceneItemListType dsShadowCullList_type(void)
{
	static int list;
	return &list;
}

dsSceneItemList* dsShadowCullList_create(dsAllocator* allocator, const char* name,
	dsSceneLightShadows* shadows, uint32_t surface)
{
	if (!allocator || !name || surface >= DS_MAX_SCENE_LIGHT_SHADOWS_SURFACES)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG,
			"Shadow cull list allocator must support freeing memory.");
		return NULL;
	}

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsShadowCullList)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsShadowCullList* cullList = DS_ALLOCATE_OBJECT(&bufferAlloc, dsShadowCullList);
	DS_ASSERT(cullList);

	dsSceneItemList* itemList = (dsSceneItemList*)cullList;
	itemList->allocator = allocator;
	itemList->type = dsShadowCullList_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsUniqueNameID_create(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = false;
	itemList->addNodeFunc = &dsShadowCullList_addNode;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = &dsShadowCullList_removeNode;
	itemList->reparentNodeFunc = NULL;
	itemList->preTransformUpdateFunc = NULL;
	itemList->updateFunc = NULL;
	itemList->preRenderPassFunc = NULL;
#if DS_HAS_SIMD
	if (DS_SIMD_ALWAYS_FMA || dsHostSIMDFeatures & dsSIMDFeatures_FMA)
		itemList->commitFunc = &dsShadowCullList_commitFMA;
	else if (DS_SIMD_ALWAYS_FLOAT4 || dsHostSIMDFeatures & dsSIMDFeatures_Float4)
		itemList->commitFunc = &dsShadowCullList_commitSIMD;
	else
#endif
		itemList->commitFunc = &dsShadowCullList_commit;
	itemList->destroyFunc = &dsShadowCullList_destroy;

	cullList->shadows = shadows;
	cullList->surface = surface;

	cullList->staticEntries = NULL;
	cullList->staticEntryCount = 0;
	cullList->maxStaticEntries = 0;
	cullList->nextStaticNodeID = 0;

	cullList->dynamicEntries = NULL;
	cullList->dynamicEntryCount = 0;
	cullList->maxDynamicEntries = 0;
	cullList->nextDynamicNodeID = MIN_DYNAMIC_ENTRY_ID;

	return itemList;
}

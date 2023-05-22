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

#include <DeepSea/Scene/ItemLists/ViewCullList.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Geometry/Frustum3.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Scene/Nodes/SceneCullNode.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/View.h>

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

typedef struct dsViewCullList
{
	dsSceneItemList itemList;

	StaticEntry* staticEntries;
	uint32_t staticEntryCount;
	uint32_t maxStaticEntries;
	uint64_t nextStaticNodeID;

	DynamicEntry* dynamicEntries;
	uint32_t dynamicEntryCount;
	uint32_t maxDynamicEntries;
	uint64_t nextDynamicNodeID;
} dsViewCullList;

static uint64_t dsViewCullList_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(itemData);
	if (!dsSceneNode_isOfType(node, dsSceneCullNode_type()))
		return DS_NO_SCENE_NODE;

	dsViewCullList* cullList = (dsViewCullList*)itemList;
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

static void dsViewCullList_removeNode(dsSceneItemList* itemList, uint64_t nodeID)
{
	dsViewCullList* cullList = (dsViewCullList*)itemList;
	if (nodeID < MIN_DYNAMIC_ENTRY_ID)
	{
		for (uint32_t i = 0; i < cullList->staticEntryCount; ++i)
		{
			if (cullList->staticEntries[i].nodeID != nodeID)
				continue;

			// Order shouldn't matter, so use constant-time removal.
			cullList->staticEntries[i] = cullList->staticEntries[cullList->staticEntryCount - 1];
			--cullList->staticEntryCount;
			break;
		}
	}
	else
	{
		for (uint32_t i = 0; i < cullList->dynamicEntryCount; ++i)
		{
			if (cullList->dynamicEntries[i].nodeID != nodeID)
				continue;

			// Order shouldn't matter, so use constant-time removal.
			cullList->dynamicEntries[i] = cullList->dynamicEntries[cullList->dynamicEntryCount - 1];
			--cullList->dynamicEntryCount;
			break;
		}
	}
}

#if DS_HAS_SIMD
DS_SIMD_START(DS_SIMD_FLOAT4)
static void dsViewCullList_commitSIMD(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(commandBuffer);
	DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);

	dsViewCullList* cullList = (dsViewCullList*)itemList;
	for (uint32_t i = 0; i < cullList->staticEntryCount; ++i)
	{
		const StaticEntry* entry = cullList->staticEntries + i;
		dsMatrix44f boxMatrix;
		dsMatrix44f_affineMulSIMD(&boxMatrix, entry->transform, &entry->localBoxMatrix);
		*entry->result = dsFrustum3f_intersectBoxMatrixSIMD(&view->viewFrustum, &boxMatrix) ==
			dsIntersectResult_Outside;
	}

	for (uint32_t i = 0; i < cullList->dynamicEntryCount; ++i)
	{
		const DynamicEntry* entry = cullList->dynamicEntries + i;
		dsMatrix44f boxMatrix;
		if (entry->node->getBoundsFunc(&boxMatrix, entry->node, entry->treeNode))
		{
			*entry->result = dsFrustum3f_intersectBoxMatrixSIMD(&view->viewFrustum, &boxMatrix) ==
				dsIntersectResult_Outside;
		}
		else
			*entry->result = true;
	}

	DS_PROFILE_SCOPE_END();
}
DS_SIMD_END()

DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_FMA)
static void dsViewCullList_commitFMA(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(commandBuffer);
	DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);

	dsViewCullList* cullList = (dsViewCullList*)itemList;
	for (uint32_t i = 0; i < cullList->staticEntryCount; ++i)
	{
		const StaticEntry* entry = cullList->staticEntries + i;
		dsMatrix44f boxMatrix;
		dsMatrix44f_affineMulFMA(&boxMatrix, entry->transform, &entry->localBoxMatrix);
		*entry->result = dsFrustum3f_intersectBoxMatrixFMA(&view->viewFrustum, &boxMatrix) ==
			dsIntersectResult_Outside;
	}

	for (uint32_t i = 0; i < cullList->dynamicEntryCount; ++i)
	{
		const DynamicEntry* entry = cullList->dynamicEntries + i;
		dsMatrix44f boxMatrix;
		if (entry->node->getBoundsFunc(&boxMatrix, entry->node, entry->treeNode))
		{
			*entry->result = dsFrustum3f_intersectBoxMatrixFMA(&view->viewFrustum, &boxMatrix) ==
				dsIntersectResult_Outside;
		}
		else
			*entry->result = true;
	}

	DS_PROFILE_SCOPE_END();
}
DS_SIMD_END()
#endif

static void dsViewCullList_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(commandBuffer);
	DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);

	dsViewCullList* cullList = (dsViewCullList*)itemList;
	for (uint32_t i = 0; i < cullList->staticEntryCount; ++i)
	{
		const StaticEntry* entry = cullList->staticEntries + i;
		dsMatrix44f boxMatrix;
		dsMatrix44f_affineMul(&boxMatrix, entry->transform, &entry->localBoxMatrix);
		*entry->result = dsFrustum3f_intersectBoxMatrix(&view->viewFrustum, &boxMatrix) ==
			dsIntersectResult_Outside;
	}

	for (uint32_t i = 0; i < cullList->dynamicEntryCount; ++i)
	{
		const DynamicEntry* entry = cullList->dynamicEntries + i;
		dsMatrix44f boxMatrix;
		if (entry->node->getBoundsFunc(&boxMatrix, entry->node, entry->treeNode))
		{
			*entry->result = dsFrustum3f_intersectBoxMatrix(&view->viewFrustum, &boxMatrix) ==
				dsIntersectResult_Outside;
		}
		else
			*entry->result = true;
	}

	DS_PROFILE_SCOPE_END();
}

static void dsViewCullList_destroy(dsSceneItemList* itemList)
{
	dsViewCullList* cullList = (dsViewCullList*)itemList;
	DS_VERIFY(dsAllocator_free(itemList->allocator, cullList->staticEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, cullList->dynamicEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

const char* const dsViewCullList_typeName = "ViewCullList";

dsSceneItemList* dsViewCullList_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const char* name, const uint8_t* data, size_t dataSize)
{
	DS_UNUSED(loadContext);
	DS_UNUSED(scratchData);
	DS_UNUSED(resourceAllocator);
	DS_UNUSED(userData);
	DS_UNUSED(data);
	DS_UNUSED(dataSize);
	return dsViewCullList_create(allocator, name);
}

dsSceneItemListType dsViewCullList_type(void)
{
	static int type;
	return &type;
}

dsSceneItemList* dsViewCullList_create(dsAllocator* allocator, const char* name)
{
	if (!allocator || !name)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "View cull list allocator must support freeing memory.");
		return NULL;
	}

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsViewCullList)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsViewCullList* cullList = DS_ALLOCATE_OBJECT(&bufferAlloc, dsViewCullList);
	DS_ASSERT(cullList);

	dsSceneItemList* itemList = (dsSceneItemList*)cullList;
	itemList->allocator = allocator;
	itemList->type = dsViewCullList_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsHashString(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = false;
	itemList->addNodeFunc = &dsViewCullList_addNode;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = &dsViewCullList_removeNode;
	itemList->preTransformUpdateFunc = NULL;
	itemList->updateFunc = NULL;
#if DS_HAS_SIMD
	if (DS_SIMD_ALWAYS_FMA || dsHostSIMDFeatures & dsSIMDFeatures_FMA)
		itemList->commitFunc = &dsViewCullList_commitFMA;
	else if (DS_SIMD_ALWAYS_FLOAT4 || dsHostSIMDFeatures & dsSIMDFeatures_Float4)
		itemList->commitFunc = &dsViewCullList_commitSIMD;
	else
#endif
		itemList->commitFunc = &dsViewCullList_commit;
	itemList->destroyFunc = &dsViewCullList_destroy;

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

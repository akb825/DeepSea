/*
 * Copyright 2021-2023 Aaron Barany
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

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Geometry/OrientedBox3.h>
#include <DeepSea/Geometry/Frustum3.h>

#include <DeepSea/Scene/Nodes/SceneCullNode.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <DeepSea/SceneLighting/SceneLightShadows.h>

#include <string.h>

typedef struct Entry
{
	const dsSceneCullNode* node;
	const dsSceneTreeNode* treeNode;
	bool* result;
	uint64_t nodeID;
} Entry;

typedef struct dsShadowCullList
{
	dsSceneItemList itemList;

	dsSceneLightShadows* shadows;
	uint32_t surface;

	Entry* entries;
	uint32_t entryCount;
	uint32_t maxEntries;
	uint64_t nextNodeID;
} dsShadowCullList;

static uint64_t dsShadowCullList_addNode(dsSceneItemList* itemList, const dsSceneNode* node,
	const dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(itemData);
	if (!dsSceneNode_isOfType(node, dsSceneCullNode_type()))
		return DS_NO_SCENE_NODE;

	const dsSceneCullNode* cullNode = (const dsSceneCullNode*)node;
	dsOrientedBox3f bounds;
	if (!dsSceneCullNode_getBounds(&bounds, cullNode, treeNode) || !dsOrientedBox3_isValid(bounds))
		return DS_NO_SCENE_NODE;

	dsShadowCullList* cullList = (dsShadowCullList*)itemList;

	uint32_t index = cullList->entryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, cullList->entries, cullList->entryCount,
			cullList->maxEntries, 1))
	{
		return DS_NO_SCENE_NODE;
	}

	Entry* entry = cullList->entries + index;
	entry->node = cullNode;
	entry->treeNode = treeNode;
	entry->result = (bool*)thisItemData;
	entry->nodeID = cullList->nextNodeID++;
	return entry->nodeID;
}

static void dsShadowCullList_removeNode(dsSceneItemList* itemList, uint64_t nodeID)
{
	dsShadowCullList* cullList = (dsShadowCullList*)itemList;
	for (uint32_t i = 0; i < cullList->entryCount; ++i)
	{
		if (cullList->entries[i].nodeID != nodeID)
			continue;

		// Order shouldn't matter, so use constant-time removal.
		cullList->entries[i] = cullList->entries[cullList->entryCount - 1];
		--cullList->entryCount;
		break;
	}
}

#if DS_HAS_SIMD
DS_SIMD_START_FLOAT4()
static void dsShadowCullList_commitSIMD(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(commandBuffer);
	DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);

	dsShadowCullList* cullList = (dsShadowCullList*)itemList;
	if (cullList->surface >= dsSceneLightShadows_getSurfaceCount(cullList->shadows))
	{
		for (uint32_t i = 0; i < cullList->entryCount; ++i)
		{
			const Entry* entry = cullList->entries + i;
			*entry->result = true;
		}
		DS_PROFILE_SCOPE_END();
		return;
	}

	for (uint32_t i = 0; i < cullList->entryCount; ++i)
	{
		const Entry* entry = cullList->entries + i;
		dsOrientedBox3f bounds;
		DS_VERIFY(entry->node->getBoundsFunc(&bounds, entry->node, entry->treeNode));
		DS_ASSERT(dsOrientedBox3_isValid(bounds));
		*entry->result = dsSceneLightShadows_intersectOrientedBoxSIMD(cullList->shadows,
			cullList->surface, &bounds) == dsIntersectResult_Outside;;
	}

	if (!dsSceneLightShadows_computeSurfaceProjection(cullList->shadows, cullList->surface))
	{
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
			"Couldn't compute projection for shadows '%s' surface %d.",
			dsSceneLightShadows_getName(cullList->shadows), cullList->surface);
	}

	DS_PROFILE_SCOPE_END();
}
DS_SIMD_END()

DS_SIMD_START_FLOAT4()
static void dsShadowCullList_commitFMA(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(commandBuffer);
	DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);

	dsShadowCullList* cullList = (dsShadowCullList*)itemList;
	if (cullList->surface >= dsSceneLightShadows_getSurfaceCount(cullList->shadows))
	{
		for (uint32_t i = 0; i < cullList->entryCount; ++i)
		{
			const Entry* entry = cullList->entries + i;
			*entry->result = true;
		}
		DS_PROFILE_SCOPE_END();
		return;
	}

	for (uint32_t i = 0; i < cullList->entryCount; ++i)
	{
		const Entry* entry = cullList->entries + i;
		dsOrientedBox3f bounds;
		DS_VERIFY(entry->node->getBoundsFunc(&bounds, entry->node, entry->treeNode));
		DS_ASSERT(dsOrientedBox3_isValid(bounds));
		*entry->result = dsSceneLightShadows_intersectOrientedBoxFMA(cullList->shadows,
			cullList->surface, &bounds) == dsIntersectResult_Outside;;
	}

	if (!dsSceneLightShadows_computeSurfaceProjection(cullList->shadows, cullList->surface))
	{
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
			"Couldn't compute projection for shadows '%s' surface %d.",
			dsSceneLightShadows_getName(cullList->shadows), cullList->surface);
	}

	DS_PROFILE_SCOPE_END();
}
DS_SIMD_END()
#endif

static void dsShadowCullList_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(commandBuffer);
	DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);

	dsShadowCullList* cullList = (dsShadowCullList*)itemList;
	if (cullList->surface >= dsSceneLightShadows_getSurfaceCount(cullList->shadows))
	{
		for (uint32_t i = 0; i < cullList->entryCount; ++i)
		{
			const Entry* entry = cullList->entries + i;
			*entry->result = true;
		}
		DS_PROFILE_SCOPE_END();
		return;
	}

	for (uint32_t i = 0; i < cullList->entryCount; ++i)
	{
		const Entry* entry = cullList->entries + i;
		dsOrientedBox3f bounds;
		DS_VERIFY(entry->node->getBoundsFunc(&bounds, entry->node, entry->treeNode));
		DS_ASSERT(dsOrientedBox3_isValid(bounds));
		*entry->result = dsSceneLightShadows_intersectOrientedBox(cullList->shadows,
			cullList->surface, &bounds) == dsIntersectResult_Outside;;
	}

	if (!dsSceneLightShadows_computeSurfaceProjection(cullList->shadows, cullList->surface))
	{
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
			"Couldn't compute projection for shadows '%s' surface %d.",
			dsSceneLightShadows_getName(cullList->shadows), cullList->surface);
	}

	DS_PROFILE_SCOPE_END();
}

static void dsShadowCullList_destroy(dsSceneItemList* itemList)
{
	dsShadowCullList* cullList = (dsShadowCullList*)itemList;
	DS_VERIFY(dsAllocator_free(itemList->allocator, cullList->entries));
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
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "View cull list allocator must support freeing memory.");
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
	itemList->nameID = dsHashString(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = false;
	itemList->addNodeFunc = &dsShadowCullList_addNode;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = &dsShadowCullList_removeNode;
	itemList->updateFunc = NULL;
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

	cullList->entries = NULL;
	cullList->entryCount = 0;
	cullList->maxEntries = 0;
	cullList->nextNodeID = 0;

	return itemList;
}

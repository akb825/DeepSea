/*
 * Copyright 2021 Aaron Barany
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

#include <DeepSea/Scene/Nodes/SceneModelNode.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <DeepSea/SceneLighting/SceneLightShadows.h>

#include <string.h>

typedef struct Entry
{
	dsSceneModelNode* node;
	const dsMatrix44f* transform;
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

const char* const dsShadowCullList_typeName = "ShadowCullList";

uint64_t dsShadowCullList_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	const dsMatrix44f* transform, dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(itemData);
	if (!dsSceneNode_isOfType(node, dsSceneModelNode_type()))
		return DS_NO_SCENE_NODE;

	dsShadowCullList* cullList = (dsShadowCullList*)itemList;

	uint32_t index = cullList->entryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, cullList->entries, cullList->entryCount,
			cullList->maxEntries, 1))
	{
		return DS_NO_SCENE_NODE;
	}

	Entry* entry = cullList->entries + index;
	entry->node = (dsSceneModelNode*)node;
	entry->transform = transform;
	entry->result = (bool*)thisItemData;
	entry->nodeID = cullList->nextNodeID++;
	return entry->nodeID;
}

void dsShadowCullList_removeNode(dsSceneItemList* itemList, uint64_t nodeID)
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

void dsShadowCullList_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(commandBuffer);
	DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);

	dsShadowCullList* cullList = (dsShadowCullList*)itemList;
	if (cullList->surface >= dsSceneLightShadows_getSurfaceCount(cullList->shadows))
	{
		DS_PROFILE_SCOPE_END();
		return;
	}

	for (uint32_t i = 0; i < cullList->entryCount; ++i)
	{
		const Entry* entry = cullList->entries + i;
		dsSceneModelNode* modelNode = entry->node;
		dsOrientedBox3f transformedBounds = modelNode->bounds;
		DS_VERIFY(dsOrientedBox3f_transform(&transformedBounds, entry->transform));
		*entry->result = dsSceneLightShadows_intersectOrientedBox(cullList->shadows,
			cullList->surface, &transformedBounds) == dsIntersectResult_Outside;
	}

	DS_VERIFY(dsSceneLightShadows_computeSurfaceProjection(cullList->shadows, cullList->surface));
	DS_PROFILE_SCOPE_END();
}

void dsShadowCullList_destroy(dsSceneItemList* itemList)
{
	dsShadowCullList* cullList = (dsShadowCullList*)itemList;
	DS_VERIFY(dsAllocator_free(itemList->allocator, cullList->entries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
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

	size_t nameLen = strlen(name);
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsShadowCullList)) + DS_ALIGNED_SIZE(nameLen + 1);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsShadowCullList* cullList = DS_ALLOCATE_OBJECT(&bufferAlloc, dsShadowCullList);
	DS_ASSERT(cullList);

	dsSceneItemList* itemList = (dsSceneItemList*)cullList;
	itemList->allocator = allocator;
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen + 1);
	memcpy((void*)itemList->name, name, nameLen + 1);
	itemList->nameID = dsHashString(name);
	itemList->needsCommandBuffer = false;
	itemList->addNodeFunc = &dsShadowCullList_addNode;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = &dsShadowCullList_removeNode;
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

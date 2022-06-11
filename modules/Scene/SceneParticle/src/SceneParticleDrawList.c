/*
 * Copyright 2022 Aaron Barany
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

#include <DeepSea/SceneParticle/SceneParticleDrawList.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Particle/ParticleDraw.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneTreeNode.h>

#include <DeepSea/SceneParticle/SceneParticleNode.h>
#include <DeepSea/SceneParticle/SceneParticlePrepareList.h>

#include <string.h>

typedef struct Entry
{
	const dsSceneTreeNode* treeNode;
	dsParticleEmitter* emitter;
	uint64_t nodeID;
} Entry;

typedef struct dsSceneParticleDrawList
{
	dsSceneItemList itemList;

	dsParticleDraw* drawer;

	Entry* entries;
	uint32_t entryCount;
	uint32_t maxEntries;
	uint64_t nextNodeID;
} dsSceneParticleDrawList;

static uint64_t dsSceneParticleDrawList_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	const dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(treeNode);
	DS_UNUSED(thisItemData);
	if (!dsSceneNode_isOfType(node, dsSceneParticleNode_type()))
		return DS_NO_SCENE_NODE;

	dsSceneParticleDrawList* drawList = (dsSceneParticleDrawList*)itemList;

	uint32_t prepareListIndex = 0;
	for (; prepareListIndex < node->itemListCount; ++prepareListIndex)
	{
		const dsSceneItemList* itemList = dsSceneTreeNode_getItemList(treeNode, prepareListIndex);
		if (itemList && itemList->type == dsSceneParticlePrepareList_type())
			break;
	}

	if (prepareListIndex == node->itemListCount)
	{
		DS_LOG_WARNING(DS_SCENE_PARTICLE_LOG_TAG,
			"Particle node must be registered with a scene particle prepare list to be drawn.");
		return DS_NO_SCENE_NODE;
	}

	DS_ASSERT(prepareListIndex < itemData->count);
	dsParticleEmitter* emitter = (dsParticleEmitter*)itemData->itemData[prepareListIndex].data;
	if (!emitter)
	{
		DS_LOG_WARNING(DS_SCENE_PARTICLE_LOG_TAG, "Particle node must have the scene particle "
			"prepare list before the scene particle draw list.");
		return DS_NO_SCENE_NODE;
	}

	uint32_t index = drawList->entryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, drawList->entries, drawList->entryCount,
			drawList->maxEntries, 1))
	{
		return DS_NO_SCENE_NODE;
	}

	if (!dsParticleDraw_addEmitter(drawList->drawer, emitter))
	{
		--drawList->entryCount;
		return DS_NO_SCENE_NODE;
	}

	Entry* entry = drawList->entries + index;
	entry->treeNode = treeNode;
	entry->emitter = emitter;
	entry->nodeID = drawList->nextNodeID++;
	return entry->nodeID;
}

static void dsSceneParticleDrawList_removeNode(dsSceneItemList* itemList, uint64_t nodeID)
{
	dsSceneParticleDrawList* drawList = (dsSceneParticleDrawList*)itemList;
	for (uint32_t i = 0; i < drawList->entryCount; ++i)
	{
		Entry* entry = drawList->entries + i;
		if (entry->nodeID != nodeID)
			continue;

		dsParticleDraw_removeEmitter(drawList->drawer, entry->emitter);

		// Order shouldn't matter, so use constant-time removal.
		drawList->entries[i] = drawList->entries[drawList->entryCount - 1];
		--drawList->entryCount;
		break;
	}
}

static void dsSceneParticleDrawList_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);

	dsSceneParticleDrawList* drawList = (dsSceneParticleDrawList*)itemList;
	DS_CHECK(DS_SCENE_PARTICLE_LOG_TAG, dsParticleDraw_draw(drawList->drawer, commandBuffer,
		view->globalValues, &view->viewMatrix, &view->viewFrustum));

	DS_PROFILE_SCOPE_END();
}

static void dsSceneParticleDrawList_destroy(dsSceneItemList* itemList)
{
	dsSceneParticleDrawList* drawList = (dsSceneParticleDrawList*)itemList;
	DS_VERIFY(dsAllocator_free(itemList->allocator, drawList->entries));
	dsParticleDraw_destroy(drawList->drawer);
	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

const char* const dsSceneParticleDrawList_typeName = "ParticleDrawList";

dsSceneItemListType dsSceneParticleDrawList_type(void)
{
	static int type;
	return &type;
}

dsSceneItemList* dsSceneParticleDrawList_create(dsAllocator* allocator, const char* name,
	dsResourceManager* resourceManager, dsAllocator* resourceAllocator)
{
	if (!allocator || !name || !resourceAllocator)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_PARTICLE_LOG_TAG,
			"Particle drfaw list allocator must support freeing memory.");
		return NULL;
	}

	size_t nameLen = strlen(name);
	size_t fullSize =
		DS_ALIGNED_SIZE(sizeof(dsSceneParticleDrawList)) + DS_ALIGNED_SIZE(nameLen + 1);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsSceneParticleDrawList* drawList =
		DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneParticleDrawList);
	DS_ASSERT(drawList);

	dsSceneItemList* itemList = (dsSceneItemList*)drawList;
	itemList->allocator = allocator;
	itemList->type = dsSceneParticleDrawList_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen + 1);
	memcpy((void*)itemList->name, name, nameLen + 1);
	itemList->nameID = dsHashString(name);
	itemList->needsCommandBuffer = true;
	itemList->addNodeFunc = &dsSceneParticleDrawList_addNode;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = dsSceneParticleDrawList_removeNode;
	itemList->updateFunc = NULL;
	itemList->commitFunc = &dsSceneParticleDrawList_commit;
	itemList->destroyFunc = &dsSceneParticleDrawList_destroy;

	drawList->entries = NULL;
	drawList->entryCount = 0;
	drawList->maxEntries = 0;
	drawList->nextNodeID = 0;

	drawList->drawer = dsParticleDraw_create(allocator, resourceManager, resourceAllocator);
	if (!drawList->drawer)
	{
		dsSceneParticleDrawList_destroy(itemList);
		return NULL;
	}

	return itemList;
}

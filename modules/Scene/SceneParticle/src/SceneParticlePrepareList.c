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

#include <DeepSea/SceneParticle/SceneParticlePrepareList.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Particle/ParticleEmitter.h>
#include <DeepSea/Particle/ParticleDraw.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneTreeNode.h>
#include <DeepSea/SceneParticle/SceneParticleNode.h>

#include <string.h>

typedef struct Entry
{
	dsSceneParticleNode* node;
	const dsSceneTreeNode* treeNode;
	dsParticleEmitter* emitter;
	uint64_t nodeID;
} Entry;

typedef struct dsSceneParticlePrepareList
{
	dsSceneItemList itemList;

	dsParticleDraw** particleDraws;
	uint32_t particleDrawCount;

	Entry* entries;
	uint32_t entryCount;
	uint32_t maxEntries;
	uint64_t nextNodeID;
} dsSceneParticlePrepareList;

static uint64_t dsSceneParticlePrepareList_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	const dsSceneTreeNode* treeNode, dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(itemData);
	DS_UNUSED(treeNode);
	DS_UNUSED(thisItemData);
	if (!dsSceneNode_isOfType(node, dsSceneParticleNode_type()))
		return DS_NO_SCENE_NODE;

	dsSceneParticlePrepareList* prepareList = (dsSceneParticlePrepareList*)itemList;

	uint32_t index = prepareList->entryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, prepareList->entries, prepareList->entryCount,
			prepareList->maxEntries, 1))
	{
		return DS_NO_SCENE_NODE;
	}

	Entry* entry = prepareList->entries + index;
	entry->node = (dsSceneParticleNode*)node;
	entry->treeNode = treeNode;
	entry->emitter = dsSceneParticleNode_createEmitter(entry->node, treeNode);
	if (!DS_CHECK_MESSAGE(DS_SCENE_PARTICLE_LOG_TAG, entry->emitter != NULL,
			"dsSceneParticleNode_createEmitter(entry->node)"))
	{
		--prepareList->entryCount;
		return DS_NO_SCENE_NODE;
	}

	for (uint32_t i = 0; i < prepareList->particleDrawCount; ++i)
	{
		if (!DS_CHECK(DS_SCENE_PARTICLE_LOG_TAG,
				dsParticleDraw_addEmitter(prepareList->particleDraws[i], entry->emitter)))
		{
			dsParticleEmitter_destroy(entry->emitter);
			--prepareList->entryCount;
		}
	}

	entry->nodeID = prepareList->nextNodeID++;
	return entry->nodeID;
}

static void dsSceneParticlePrepareList_removeNode(dsSceneItemList* itemList, uint64_t nodeID)
{
	dsSceneParticlePrepareList* prepareList = (dsSceneParticlePrepareList*)itemList;
	for (uint32_t i = 0; i < prepareList->entryCount; ++i)
	{
		if (prepareList->entries[i].nodeID != nodeID)
			continue;

		dsParticleEmitter_destroy(prepareList->entries[i].emitter);

		// Order shouldn't matter, so use constant-time removal.
		prepareList->entries[i] = prepareList->entries[prepareList->entryCount - 1];
		--prepareList->entryCount;
		break;
	}
}

static void dsSceneParticlePrepareList_update(dsSceneItemList* itemList, const dsScene* scene,
	float time)
{
	DS_UNUSED(scene);
	DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);

	dsSceneParticlePrepareList* prepareList = (dsSceneParticlePrepareList*)itemList;
	for (uint32_t i = 0; i < prepareList->entryCount; ++i)
	{
		Entry* entry = prepareList->entries + i;
		DS_CHECK(DS_SCENE_PARTICLE_LOG_TAG,
			dsSceneParticleNode_updateEmitter(entry->node, entry->emitter, entry->treeNode, time));
	}

	DS_PROFILE_SCOPE_END();
}

void dsSceneParticlePrepareList_destroy(dsSceneItemList* itemList)
{
	dsSceneParticlePrepareList* prepareList = (dsSceneParticlePrepareList*)itemList;
	DS_VERIFY(dsAllocator_free(itemList->allocator, prepareList->entries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

const char* const dsSceneParticlePrepareList_typeName = "ParticlePrepareList";

dsSceneItemListType dsSceneParticlePrepareList_type(void)
{
	static int type;
	return &type;
}

dsSceneItemList* dsSceneParticlePrepareList_create(dsAllocator* allocator, const char* name,
	dsParticleDraw* const* particleDraws, uint32_t particleDrawCount)
{
	if (!allocator || !name || !particleDraws || particleDrawCount == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_PARTICLE_LOG_TAG,
			"Particle prepare list allocator must support freeing memory.");
		return NULL;
	}

	for (uint32_t i = 0; i < particleDrawCount; ++i)
	{
		if (!particleDraws[i])
		{
			errno = EINVAL;
			return false;
		}
	}

	size_t nameLen = strlen(name);
	size_t fullSize =
		DS_ALIGNED_SIZE(sizeof(dsSceneParticlePrepareList)) + DS_ALIGNED_SIZE(nameLen + 1) +
		DS_ALIGNED_SIZE(sizeof(dsParticleDraw*)*particleDrawCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsSceneParticlePrepareList* prepareList =
		DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneParticlePrepareList);
	DS_ASSERT(prepareList);

	dsSceneItemList* itemList = (dsSceneItemList*)prepareList;
	itemList->allocator = allocator;
	itemList->type = dsSceneParticlePrepareList_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen + 1);
	memcpy((void*)itemList->name, name, nameLen + 1);
	itemList->nameID = dsHashString(name);
	itemList->needsCommandBuffer = true;
	itemList->addNodeFunc = &dsSceneParticlePrepareList_addNode;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = &dsSceneParticlePrepareList_removeNode;
	itemList->updateFunc = &dsSceneParticlePrepareList_update;
	itemList->commitFunc = NULL;
	itemList->destroyFunc = &dsSceneParticlePrepareList_destroy;

	prepareList->particleDraws = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsParticleDraw*,
		particleDrawCount);
	DS_ASSERT(prepareList->particleDraws);
	prepareList->particleDrawCount = particleDrawCount;

	prepareList->entries = NULL;
	prepareList->entryCount = 0;
	prepareList->maxEntries = 0;
	prepareList->nextNodeID = 0;

	return itemList;
}

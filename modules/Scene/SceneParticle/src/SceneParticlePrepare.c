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

#include <DeepSea/SceneParticle/SceneParticlePrepare.h>

#include "SceneParticlePrepareLoad.h"

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
	const dsSceneParticleNode* node;
	const dsSceneTreeNode* treeNode;
	dsParticleEmitter* emitter;
	uint64_t nodeID;
} Entry;

typedef struct dsSceneParticlePrepare
{
	dsSceneItemList itemList;

	Entry* entries;
	uint32_t entryCount;
	uint32_t maxEntries;
	uint64_t nextNodeID;
} dsSceneParticlePrepare;

static uint64_t dsSceneParticlePrepare_addNode(dsSceneItemList* itemList, const dsSceneNode* node,
	const dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(itemData);
	DS_UNUSED(treeNode);
	if (!dsSceneNode_isOfType(node, dsSceneParticleNode_type()))
		return DS_NO_SCENE_NODE;

	dsSceneParticlePrepare* prepareList = (dsSceneParticlePrepare*)itemList;

	uint32_t index = prepareList->entryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, prepareList->entries, prepareList->entryCount,
			prepareList->maxEntries, 1))
	{
		return DS_NO_SCENE_NODE;
	}

	Entry* entry = prepareList->entries + index;
	entry->node = (const dsSceneParticleNode*)node;
	entry->treeNode = treeNode;
	entry->emitter = dsSceneParticleNode_createEmitter(entry->node, treeNode);
	if (!DS_CHECK_MESSAGE(DS_SCENE_PARTICLE_LOG_TAG, entry->emitter != NULL,
			"dsSceneParticleNode_createEmitter(entry->node)"))
	{
		--prepareList->entryCount;
		return DS_NO_SCENE_NODE;
	}

	*thisItemData = entry->emitter;
	entry->nodeID = prepareList->nextNodeID++;
	return entry->nodeID;
}

static void dsSceneParticlePrepare_removeNode(dsSceneItemList* itemList, uint64_t nodeID)
{
	dsSceneParticlePrepare* prepareList = (dsSceneParticlePrepare*)itemList;
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

static void dsSceneParticlePrepare_update(dsSceneItemList* itemList, const dsScene* scene,
	float time)
{
	DS_UNUSED(scene);
	DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);

	dsSceneParticlePrepare* prepareList = (dsSceneParticlePrepare*)itemList;
	for (uint32_t i = 0; i < prepareList->entryCount; ++i)
	{
		Entry* entry = prepareList->entries + i;
		DS_CHECK(DS_SCENE_PARTICLE_LOG_TAG,
			dsSceneParticleNode_updateEmitter(entry->node, entry->emitter, entry->treeNode, time));
	}

	DS_PROFILE_SCOPE_END();
}

static void dsSceneParticlePrepare_destroy(dsSceneItemList* itemList)
{
	dsSceneParticlePrepare* prepareList = (dsSceneParticlePrepare*)itemList;
	for (uint32_t i = 0; i < prepareList->entryCount; ++i)
		dsParticleEmitter_destroy(prepareList->entries[i].emitter);
	DS_VERIFY(dsAllocator_free(itemList->allocator, prepareList->entries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

dsSceneItemList* dsSceneParticlePrepare_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const char* name, const uint8_t* data, size_t dataSize)
{
	DS_UNUSED(loadContext);
	DS_UNUSED(scratchData);
	DS_UNUSED(resourceAllocator);
	DS_UNUSED(userData);
	DS_UNUSED(data);
	DS_UNUSED(dataSize);
	return dsSceneParticlePrepare_create(allocator, name);
}

const char* const dsSceneParticlePrepare_typeName = "ParticlePrepare";

dsSceneItemListType dsSceneParticlePrepare_type(void)
{
	static int type;
	return &type;
}

dsSceneItemList* dsSceneParticlePrepare_create(dsAllocator* allocator, const char* name)
{
	if (!allocator || !name)
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

	size_t nameLen = strlen(name) + 1;
	size_t fullSize =
		DS_ALIGNED_SIZE(sizeof(dsSceneParticlePrepare)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsSceneParticlePrepare* prepareList =
		DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneParticlePrepare);
	DS_ASSERT(prepareList);

	dsSceneItemList* itemList = (dsSceneItemList*)prepareList;
	itemList->allocator = allocator;
	itemList->type = dsSceneParticlePrepare_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsHashString(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = true;
	itemList->addNodeFunc = &dsSceneParticlePrepare_addNode;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = &dsSceneParticlePrepare_removeNode;
	itemList->updateFunc = &dsSceneParticlePrepare_update;
	itemList->commitFunc = NULL;
	itemList->destroyFunc = &dsSceneParticlePrepare_destroy;

	prepareList->entries = NULL;
	prepareList->entryCount = 0;
	prepareList->maxEntries = 0;
	prepareList->nextNodeID = 0;

	return itemList;
}

/*
 * Copyright 2022-2025 Aaron Barany
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

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Geometry/Frustum3.h>

#include <DeepSea/Particle/ParticleDraw.h>

#include <DeepSea/Render/Renderer.h>

#include <DeepSea/Scene/ItemLists/SceneInstanceData.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneNodeItemData.h>

#include <DeepSea/SceneParticle/PopulateSceneParticleInstanceData.h>
#include <DeepSea/SceneParticle/SceneParticleNode.h>
#include <DeepSea/SceneParticle/SceneParticlePrepare.h>

#include <string.h>

typedef struct Entry
{
	const dsSceneTreeNode* treeNode;
	dsParticleEmitter* emitter;
	const dsSceneNodeItemData* itemData;
	uint64_t nodeID;
} Entry;

typedef struct dsSceneParticleDrawList
{
	dsSceneItemList itemList;

	dsSceneInstanceData** instanceData;
	dsParticleDraw* drawer;
	uint32_t instanceDataCount;
	uint32_t cullListID;

	Entry* entries;
	uint32_t entryCount;
	uint32_t maxEntries;

	const dsParticleEmitter** emitters;
	uint32_t emitterCount;
	uint32_t maxEmitters;

	const dsSceneTreeNode** instances;
	uint32_t instanceCount;
	uint32_t maxInstances;

	uint64_t nextNodeID;
} dsSceneParticleDrawList;

static void destroyInstanceData(dsSceneInstanceData* const* instanceData,
	uint32_t instanceDataCount)
{
	for (uint32_t i = 0; i < instanceDataCount; ++i)
		dsSceneInstanceData_destroy(instanceData[i]);
}

static void setupInstances(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	dsSceneParticleDrawList* drawList = (dsSceneParticleDrawList*)itemList;
	drawList->emitterCount = 0;
	drawList->instanceCount = 0;
	for (uint32_t i = 0; i < drawList->entryCount; ++i)
	{
		// Particle draw uses the view frustum for culling.
		Entry* entry = drawList->entries + i;
		// Non-zero cull result means out of view.
		if (drawList->cullListID &&
			dsSceneNodeItemData_findID(entry->itemData, drawList->cullListID))
		{
			continue;
		}

		uint32_t index = drawList->instanceCount;
		if (!DS_CHECK(DS_SCENE_PARTICLE_LOG_TAG, DS_RESIZEABLE_ARRAY_ADD(itemList->allocator,
				drawList->emitters, drawList->emitterCount, drawList->maxEmitters, 1)))
		{
			return;
		}

		if (!DS_CHECK(DS_SCENE_PARTICLE_LOG_TAG, DS_RESIZEABLE_ARRAY_ADD(itemList->allocator,
				drawList->instances, drawList->instanceCount, drawList->maxInstances, 1)))
		{
			return;
		}

		drawList->emitters[index] = entry->emitter;
		drawList->instances[index] = entry->treeNode;
	}

	if (drawList->instanceCount > 0)
	{
		for (uint32_t i = 0; i < drawList->instanceDataCount; ++i)
		{
			DS_CHECK(DS_SCENE_PARTICLE_LOG_TAG, dsSceneInstanceData_populateData(
				drawList->instanceData[i], view, commandBuffer, drawList->instances,
				drawList->instanceCount));
		}
	}
}

static uint64_t dsSceneParticleDrawList_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(treeNode);
	DS_UNUSED(thisItemData);
	if (!dsSceneNode_isOfType(node, dsSceneParticleNode_type()))
		return DS_NO_SCENE_NODE;

	dsSceneParticleDrawList* drawList = (dsSceneParticleDrawList*)itemList;

	uint32_t prepareListIndex = 0;
	for (; prepareListIndex < node->itemListCount; ++prepareListIndex)
	{
		const dsSceneItemList* itemList = treeNode->itemLists[prepareListIndex].list;
		if (itemList && itemList->type == dsSceneParticlePrepare_type())
			break;
	}

	if (prepareListIndex >= node->itemListCount)
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

	Entry* entry = drawList->entries + index;
	entry->treeNode = treeNode;
	entry->emitter = emitter;
	entry->itemData = itemData;
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

		// Order shouldn't matter, so use constant-time removal.
		drawList->entries[i] = drawList->entries[drawList->entryCount - 1];
		--drawList->entryCount;
		break;
	}
}

static void dsSceneParticleDrawList_preRenderPass(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	dsRenderer_pushDebugGroup(commandBuffer->renderer, commandBuffer, itemList->name);
	setupInstances(itemList, view, commandBuffer);
	dsRenderer_popDebugGroup(commandBuffer->renderer, commandBuffer);
}

static void dsSceneParticleDrawList_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	dsRenderer_pushDebugGroup(commandBuffer->renderer, commandBuffer, itemList->name);

	if (!itemList->preRenderPassFunc)
		setupInstances(itemList, view, NULL);

	dsSceneParticleDrawList* drawList = (dsSceneParticleDrawList*)itemList;
	if (drawList->instanceCount == 0)
		dsRenderer_popDebugGroup(commandBuffer->renderer, commandBuffer);

	dsSceneParticleInstanceData drawData =
	{
		drawList->instanceData,
		drawList->instances,
		drawList->instanceDataCount,
		drawList->instanceCount
	};
	DS_CHECK(DS_SCENE_PARTICLE_LOG_TAG, dsParticleDraw_draw(drawList->drawer, commandBuffer,
		view->globalValues, &view->viewMatrix, drawList->emitters, drawList->emitterCount,
		&drawData));

	for (uint32_t i = 0; i < drawList->instanceDataCount; ++i)
		DS_CHECK(DS_SCENE_PARTICLE_LOG_TAG, dsSceneInstanceData_finish(drawList->instanceData[i]));

	dsRenderer_popDebugGroup(commandBuffer->renderer, commandBuffer);
}

static void dsSceneParticleDrawList_destroy(dsSceneItemList* itemList)
{
	dsSceneParticleDrawList* drawList = (dsSceneParticleDrawList*)itemList;
	DS_VERIFY(dsAllocator_free(itemList->allocator, drawList->entries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, (void*)drawList->emitters));
	DS_VERIFY(dsAllocator_free(itemList->allocator, (void*)drawList->instances));
	destroyInstanceData(drawList->instanceData, drawList->instanceDataCount);
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
	dsResourceManager* resourceManager, dsAllocator* resourceAllocator,
	dsSceneInstanceData* const* instanceData, uint32_t instanceDataCount, const char* cullList)
{
	if (!allocator || !name || !resourceAllocator || (!instanceData && instanceDataCount > 0))
	{
		errno = EINVAL;
		if (instanceData)
			destroyInstanceData(instanceData, instanceDataCount);
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_PARTICLE_LOG_TAG,
			"Particle drfaw list allocator must support freeing memory.");
		destroyInstanceData(instanceData, instanceDataCount);
		return NULL;
	}

	uint32_t instanceValueCount = 0;
	bool needsPreRenderPass = false;
	for (uint32_t i = 0; i < instanceDataCount; ++i)
	{
		if (!instanceData[i])
		{
			errno = EINVAL;
			destroyInstanceData(instanceData, instanceDataCount);
			return NULL;
		}

		instanceValueCount += instanceData[i]->valueCount;
		if (instanceData[i]->needsCommandBuffer)
			needsPreRenderPass = true;
	}

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneParticleDrawList)) +
		DS_ALIGNED_SIZE(nameLen) +
		DS_ALIGNED_SIZE(sizeof(dsSceneInstanceData*)*instanceDataCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		destroyInstanceData(instanceData, instanceDataCount);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsSceneParticleDrawList* drawList =
		DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneParticleDrawList);
	DS_ASSERT(drawList);

	dsSceneItemList* itemList = (dsSceneItemList*)drawList;
	itemList->allocator = allocator;
	itemList->type = dsSceneParticleDrawList_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsUniqueNameID_create(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = true;
	itemList->addNodeFunc = &dsSceneParticleDrawList_addNode;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = dsSceneParticleDrawList_removeNode;
	itemList->reparentNodeFunc = NULL;
	itemList->preTransformUpdateFunc = NULL;
	itemList->updateFunc = NULL;
	itemList->preRenderPassFunc = needsPreRenderPass ? dsSceneParticleDrawList_preRenderPass : NULL;
	itemList->commitFunc = &dsSceneParticleDrawList_commit;
	itemList->destroyFunc = &dsSceneParticleDrawList_destroy;

	drawList->cullListID = cullList ? dsUniqueNameID_create(cullList) : 0;

	if (instanceDataCount > 0)
	{
		drawList->instanceData = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneInstanceData*,
			instanceDataCount);
		DS_ASSERT(drawList->instanceData);
		memcpy(drawList->instanceData, instanceData,
			sizeof(dsSceneInstanceData*)*instanceDataCount);
	}
	else
		drawList->instanceData = NULL;
	drawList->instanceDataCount = instanceDataCount;

	drawList->entries = NULL;
	drawList->entryCount = 0;
	drawList->maxEntries = 0;
	drawList->emitters = NULL;
	drawList->emitterCount = 0;
	drawList->maxEmitters = 0;
	drawList->instances = NULL;
	drawList->instanceCount = 0;
	drawList->maxInstances = 0;
	drawList->nextNodeID = 0;

	drawList->drawer = dsParticleDraw_create(allocator, resourceManager, resourceAllocator,
		instanceValueCount);
	if (!drawList->drawer)
	{
		dsSceneParticleDrawList_destroy(itemList);
		return NULL;
	}

	return itemList;
}

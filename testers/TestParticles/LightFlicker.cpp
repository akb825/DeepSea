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

#include "LightFlicker.h"
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Random.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/SceneLighting/SceneLight.h>
#include <DeepSea/SceneLighting/SceneLightNode.h>
#include <string.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include "LightFlicker_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#endif

typedef struct Entry
{
	const dsSceneTreeNode* treeNode;
	dsSceneLight* light;
	float time;
	float totalTime;
	float startIntensity;
	float targetIntensity;
	uint64_t nodeID;
} Entry;

typedef struct dsLightFlicker
{
	dsSceneItemList iitemList;
	dsRandom random;
	dsVector2f timeRange;
	dsVector2f intensityRange;

	Entry* entries;
	uint32_t entryCount;
	uint32_t maxEntries;
	uint64_t nextNodeID;
} dsLightFlicker;

static int type;

static uint64_t dsLightFlicker_addNode(dsSceneItemList* itemList, const dsSceneNode* node,
	const dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData,
	void** thisItemData)
{
	DS_UNUSED(itemData);
	DS_UNUSED(thisItemData);
	if (!dsSceneNode_isOfType(node, dsSceneLightNode_type()))
		return DS_NO_SCENE_NODE;

	dsLightFlicker* flicker = (dsLightFlicker*)itemList;

	uint32_t index = flicker->entryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, flicker->entries, flicker->entryCount,
			flicker->maxEntries, 1))
	{
		return DS_NO_SCENE_NODE;
	}

	Entry* entry = flicker->entries + index;
	entry->treeNode = treeNode;
	// Lazily set light as this needs to be before the dsSceneLightSetPrepare responsible for
	// creating the light itself.
	entry->light = NULL;
	entry->time = 0;
	entry->totalTime = 0;
	entry->startIntensity = 0;
	entry->targetIntensity = 0;
	entry->nodeID = flicker->nextNodeID++;
	return entry->nodeID;
}

static void dsLightFlicker_removeNode(dsSceneItemList* itemList, uint64_t nodeID)
{
	dsLightFlicker* flicker = (dsLightFlicker*)itemList;
	for (uint32_t i = 0; i < flicker->entryCount; ++i)
	{
		if (flicker->entries[i].nodeID != nodeID)
			continue;

		// Order shouldn't matter, so use constant-time removal.
		flicker->entries[i] = flicker->entries[flicker->entryCount - 1];
		--flicker->entryCount;
		break;
	}
}

static void dsLightFlicker_update(dsSceneItemList* itemList, const dsScene* scene, float time)
{
	DS_UNUSED(scene);
	dsLightFlicker* flicker = (dsLightFlicker*)itemList;
	for (uint32_t i = 0; i < flicker->entryCount; ++i)
	{
		Entry* entry = flicker->entries + i;
		if (!entry->light)
		{
			// Lazily query the light as it won't be created on node add.
			entry->light = dsSceneLightNode_getLightForInstance(entry->treeNode);
			if (!entry->light)
				continue;

			// Ignore the original intensity for the light.
			entry->light->intensity = dsRandom_nextFloatRange(&flicker->random,
				flicker->intensityRange.x, flicker->intensityRange.y);
		}
		else
			entry->time -= time;

		// Expect only one iteration, so don't avoid extra operations if it loops again.
		while (entry->time <= 0)
		{
			entry->totalTime = dsRandom_nextFloatRange(&flicker->random,
				flicker->timeRange.x, flicker->timeRange.y);
			entry->time += entry->totalTime;
			entry->startIntensity = entry->light->intensity;
			entry->targetIntensity = dsRandom_nextFloatRange(&flicker->random,
				flicker->intensityRange.x, flicker->intensityRange.y);
		}

		// Swap lerp since starts at 1 and goes down to 0.
		entry->light->intensity = dsLerp(entry->targetIntensity, entry->startIntensity,
			entry->time/entry->totalTime);
	}
}

static void dsLightFlicker_destroy(dsSceneItemList* itemList)
{
	if (!itemList->allocator)
		return;

	dsLightFlicker* flicker = (dsLightFlicker*)itemList;
	DS_VERIFY(dsAllocator_free(itemList->allocator, flicker->entries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

dsSceneItemList* dsLightFlicker_load(const dsSceneLoadContext*, dsSceneLoadScratchData*,
	dsAllocator* allocator, dsAllocator*, void*, const char* name, const uint8_t* data,
	size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!TestParticles::VerifyLightFlickerBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Invalid light flicker flatbuffer format.");
		return nullptr;
	}

	auto fbFlicker = TestParticles::GetLightFlicker(data);
	dsVector2f timeRange = {{fbFlicker->minTime(), fbFlicker->maxTime()}};
	dsVector2f intensityRange = {{fbFlicker->minIntensity(), fbFlicker->maxIntensity()}};
	return dsLightFlicker_create(allocator, name, &timeRange, &intensityRange);
}

dsSceneItemList* dsLightFlicker_create(dsAllocator* allocator, const char* name,
	const dsVector2f* timeRange, const dsVector2f* intensityRange)
{
	DS_ASSERT(allocator);
	DS_ASSERT(timeRange);
	DS_ASSERT(intensityRange);

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsLightFlicker)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return nullptr;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsLightFlicker* flicker = DS_ALLOCATE_OBJECT(&bufferAlloc, dsLightFlicker);
	DS_VERIFY(flicker);
	dsSceneItemList* itemList = (dsSceneItemList*)flicker;

	itemList->allocator = dsAllocator_keepPointer(allocator);
	itemList->type = &type;
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsHashString(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = false;
	itemList->addNodeFunc = &dsLightFlicker_addNode;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = &dsLightFlicker_removeNode;
	itemList->preTransformUpdateFunc = NULL;
	itemList->updateFunc = &dsLightFlicker_update;
	itemList->commitFunc = NULL;
	itemList->destroyFunc = &dsLightFlicker_destroy;

	dsRandom_initialize(&flicker->random);
	flicker->timeRange = *timeRange;
	flicker->intensityRange = *intensityRange;
	flicker->entries = NULL;
	flicker->entryCount = 0;
	flicker->maxEntries = 0;
	flicker->nextNodeID = 0;

	return itemList;
}

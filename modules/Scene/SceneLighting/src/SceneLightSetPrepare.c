/*
 * Copyright 2020 Aaron Barany
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

#include <DeepSea/SceneLighting/SceneLightSetPrepare.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Scene/ItemLists/SceneItemList.h>
#include <DeepSea/SceneLighting/SceneLightSet.h>

#include <string.h>

typedef struct dsSceneLightSetPrepare
{
	dsSceneItemList itemList;
	dsSceneLightSet** lightSets;
	uint32_t lightSetCount;
	float intensityThreshold;
} dsSceneLightSetPrepare;

const char* const dsSceneLightSetPrepare_typeName = "LightSetPrepare";

uint64_t dsSceneLightSetPrepare_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	const dsMatrix44f* transform, dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(itemList);
	DS_UNUSED(node);
	DS_UNUSED(itemData);
	DS_UNUSED(transform);
	DS_UNUSED(thisItemData);
	return DS_NO_SCENE_NODE;
}

void dsSceneLightSetPrepare_removeNode(dsSceneItemList* itemList, uint64_t nodeID)
{
	DS_UNUSED(itemList);
	DS_UNUSED(nodeID);
}

void dsSceneLightSetPrepare_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(view);
	DS_UNUSED(commandBuffer);
	dsSceneLightSetPrepare* prepare = (dsSceneLightSetPrepare*)itemList;
	for (uint32_t i = 0; i < prepare->lightSetCount; ++i)
		dsSceneLightSet_prepare(prepare->lightSets[i], prepare->intensityThreshold);
}

void dsSceneLightSetPrepare_destroy(dsSceneItemList* itemList)
{
	if (itemList->allocator)
		dsAllocator_free(itemList->allocator, itemList);
}

dsSceneItemList* dsSceneLightSetPrepare_create(dsAllocator* allocator, const char* name,
	dsSceneLightSet* const* lightSets, uint32_t lightSetCount, float intensityThreshold)
{
	if (!allocator || !name || !lightSets || lightSetCount == 0 || intensityThreshold <= 0)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneLightSetPrepare)) + DS_ALIGNED_SIZE(nameLen) +
		DS_ALIGNED_SIZE(sizeof(dsSceneLightSet*)*lightSetCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneLightSetPrepare* prepare = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneLightSetPrepare);
	DS_ASSERT(prepare);

	dsSceneItemList* itemList = (dsSceneItemList*)prepare;
	itemList->allocator = dsAllocator_keepPointer(allocator);
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsHashString(name);
	itemList->needsCommandBuffer = false;
	itemList->addNodeFunc = &dsSceneLightSetPrepare_addNode;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = &dsSceneLightSetPrepare_removeNode;
	itemList->commitFunc = &dsSceneLightSetPrepare_commit;
	itemList->destroyFunc = &dsSceneLightSetPrepare_destroy;

	prepare->lightSets = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneLightSet*, lightSetCount);
	DS_ASSERT(prepare->lightSets);
	memcpy(prepare->lightSets, lightSets, sizeof(dsSceneLightSet*)*lightSetCount);
	prepare->lightSetCount = lightSetCount;
	prepare->intensityThreshold = intensityThreshold;

	return itemList;
}
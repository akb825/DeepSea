/*
 * Copyright 2020-2025 Aaron Barany
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

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Scene/ItemLists/SceneItemListEntries.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <DeepSea/SceneLighting/SceneLightNode.h>
#include <DeepSea/SceneLighting/SceneLightSet.h>

#include <string.h>

typedef struct Entry
{
	const dsSceneTreeNode* treeNode;
	dsSceneLight* light;
	dsVector3f position;
	dsVector3f direction;
	uint64_t nodeID;
} Entry;

struct dsSceneLightSetPrepare
{
	dsSceneItemList itemList;
	dsSceneLightSet* lightSet;
	float intensityThreshold;

	Entry* entries;
	uint32_t entryCount;
	uint32_t maxEntries;
	uint64_t nextNodeID;

	uint64_t* removeEntries;
	uint32_t removeEntryCount;
	uint32_t maxRemoveEntries;
};

static void transformLight(dsSceneLight* light, const dsVector3f* position,
	const dsVector3f* direction, const dsMatrix44f* transform)
{
	dsVector4f position4 = {{position->x, position->y, position->z, 1.0f}};
	dsVector4f direction4 = {{direction->x, direction->y, direction->z, 0.0f}};

	dsMatrix44f inverse;
	dsMatrix44f_affineInvert(&inverse, transform);

	dsVector4f transformedPosition, transformedDirection;
	dsMatrix44f_transform(&transformedPosition, transform, &position4);
	dsMatrix44f_transformTransposed(&transformedDirection, transform, &direction4);

	light->position = *(dsVector3f*)&transformedPosition;
	light->direction = *(dsVector3f*)&transformedDirection;
}

static uint64_t dsSceneLightSetPrepare_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(itemData);
	if (!dsSceneNode_isOfType(node, dsSceneLightNode_type()))
		return DS_NO_SCENE_NODE;

	dsSceneLightSetPrepare* prepare = (dsSceneLightSetPrepare*)itemList;

	uint32_t index = prepare->entryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, prepare->entries, prepare->entryCount,
			prepare->maxEntries, 1))
	{
		return DS_NO_SCENE_NODE;
	}

	const dsSceneLightNode* lightNode = (const dsSceneLightNode*)node;
	const char* baseName = dsSceneLightNode_getLightBaseName(lightNode);
	const char* lightName;
	if (dsSceneLightNode_getSingleInstance(lightNode))
		lightName = baseName;
	else
	{
		size_t baseNameLen = strlen(baseName);
		const size_t maxExtraLen = 21; // Max 64-bit value and a period.
		size_t fullNameLen = baseNameLen + maxExtraLen + 1;
		lightName = DS_ALLOCATE_STACK_OBJECT_ARRAY(char, fullNameLen);
		snprintf((char*)lightName, fullNameLen, "%s.%llu", baseName,
			(unsigned long long)prepare->nextNodeID);
	}

	dsSceneLight* light = dsSceneLightSet_addLightName(prepare->lightSet, lightName);
	if (!light)
	{
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "Couldn't create light '%s' for light node.",
			lightName);
		--prepare->entryCount;
		return DS_NO_SCENE_NODE;
	}

	const dsSceneLight* templateLight = dsSceneLightNode_getTemplateLight(lightNode);
	DS_ASSERT(templateLight);
	// Copy everything except for the name ID.
	memcpy(light, templateLight, offsetof(dsSceneLight, nameID));

	*thisItemData = light;
	transformLight(light, &templateLight->position, &templateLight->direction,
		&treeNode->transform);

	Entry* entry = prepare->entries + index;
	entry->treeNode = treeNode;
	entry->light = light;
	// Copy the template light position and direction to avoid any changes from carrying over after
	// creation.
	entry->position = templateLight->position;
	entry->direction = templateLight->direction;
	entry->nodeID = prepare->nextNodeID++;
	return entry->nodeID;
}

static void dsSceneLightSetPrepare_updateNode(
	dsSceneItemList* itemList, dsSceneTreeNode* treeNode, uint64_t nodeID)
{
	DS_UNUSED(treeNode);
	dsSceneLightSetPrepare* prepare = (dsSceneLightSetPrepare*)itemList;

	Entry* entry = (Entry*)dsSceneItemListEntries_findEntry(prepare->entries, prepare->entryCount,
		sizeof(Entry), offsetof(Entry, nodeID), nodeID);
	if (entry)
	{
		transformLight(entry->light, &entry->position, &entry->direction,
			&entry->treeNode->transform);
	}
}

static void dsSceneLightSetPrepare_removeNode(
	dsSceneItemList* itemList, dsSceneTreeNode* treeNode, uint64_t nodeID)
{
	DS_UNUSED(treeNode);
	dsSceneLightSetPrepare* prepare = (dsSceneLightSetPrepare*)itemList;

	Entry* entry = (Entry*)dsSceneItemListEntries_findEntry(prepare->entries, prepare->entryCount,
		sizeof(Entry), offsetof(Entry, nodeID), nodeID);
	if (!entry)
		return;

	DS_VERIFY(dsSceneLightSet_removeLight(prepare->lightSet, entry->light));

	uint32_t index = prepare->removeEntryCount;
	if (DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, prepare->removeEntries,
			prepare->removeEntryCount, prepare->maxRemoveEntries, 1))
	{
		prepare->removeEntries[index] = nodeID;
	}
	else
	{
		dsSceneItemListEntries_removeSingleIndex(prepare->entries, &prepare->entryCount,
			sizeof(Entry), entry - prepare->entries);
	}
}

static void dsSceneLightSetPrepare_update(dsSceneItemList* itemList, const dsScene* scene,
	float time)
{
	DS_UNUSED(scene);
	DS_UNUSED(time);
	dsSceneLightSetPrepare* prepare = (dsSceneLightSetPrepare*)itemList;

	// Lazily remove entries.
	dsSceneItemListEntries_removeMulti(prepare->entries, &prepare->entryCount,
		sizeof(Entry), offsetof(Entry, nodeID), prepare->removeEntries,
		prepare->removeEntryCount);
	prepare->removeEntryCount = 0;

	dsSceneLightSet_prepare(prepare->lightSet, prepare->intensityThreshold);
}

const char* const dsSceneLightSetPrepare_typeName = "LightSetPrepare";

const dsSceneItemListType* dsSceneLightSetPrepare_type(void)
{
	static dsSceneItemListType type =
	{
		.addNodeFunc = &dsSceneLightSetPrepare_addNode,
		.updateNodeFunc = &dsSceneLightSetPrepare_updateNode,
		.removeNodeFunc = &dsSceneLightSetPrepare_removeNode,
		.updateFunc = &dsSceneLightSetPrepare_update,
		.destroyFunc = (dsDestroySceneItemListFunction)&dsSceneLightSetPrepare_destroy
	};
	return &type;
}

dsSceneLightSetPrepare* dsSceneLightSetPrepare_create(dsAllocator* allocator, const char* name,
	dsSceneLightSet* lightSet, float intensityThreshold)
{
	if (!allocator || !name || !lightSet || intensityThreshold <= 0)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG,
			"Light set prepare allocator must support freeing memory.");
		return NULL;
	}

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneLightSetPrepare)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneLightSetPrepare* prepare = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneLightSetPrepare);
	DS_ASSERT(prepare);

	dsSceneItemList* itemList = (dsSceneItemList*)prepare;
	itemList->allocator = dsAllocator_keepPointer(allocator);
	itemList->type = dsSceneLightSetPrepare_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsUniqueNameID_create(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = false;
	itemList->skipPreRenderPass = false;

	prepare->lightSet = lightSet;
	prepare->intensityThreshold = intensityThreshold;
	prepare->entries = NULL;
	prepare->entryCount = 0;
	prepare->maxEntries = 0;
	prepare->nextNodeID = 0;

	return prepare;
}

const dsSceneLightSet* dsSceneLightSetPrepare_getLightSet(const dsSceneLightSetPrepare* prepare)
{
	if (!prepare)
	{
		errno = EINVAL;
		return NULL;
	}

	return prepare->lightSet;
}

float dsSceneLightSetPrepare_getIntensityThreshold(const dsSceneLightSetPrepare* prepare)
{
	if (!prepare)
	{
		errno = EINVAL;
		return 0;
	}

	return prepare->intensityThreshold;
}

bool dsSceneLightSetPrepare_setIntensityThreshold(dsSceneLightSetPrepare* prepare,
	float intensityThreshold)
{
	if (!prepare || intensityThreshold <= 0)
	{
		errno = EINVAL;
		return false;
	}

	prepare->intensityThreshold = intensityThreshold;
	return true;
}

void dsSceneLightSetPrepare_destroy(dsSceneLightSetPrepare* prepare)
{
	if (!prepare)
		return;

	dsSceneItemList* itemList = (dsSceneItemList*)prepare;
	DS_VERIFY(dsAllocator_free(itemList->allocator, prepare->entries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, prepare->removeEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

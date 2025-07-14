/*
 * Copyright 2025 Aaron Barany
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

#include <DeepSea/Scene/ItemLists/SceneHandoffList.h>

#include "SceneLoadContextInternal.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Quaternion.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Scene/ItemLists/SceneItemListEntries.h>
#include <DeepSea/Scene/Nodes/SceneHandoffNode.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <string.h>

typedef struct Entry
{
	dsMatrix44f transform;
	dsQuaternion4f prevOrientation;
	dsVector3f prevPosition;
	dsVector3f prevScale;
	float t;
	const dsMatrix44f* commonAncestorTransform;
	dsSceneTreeNode* node;
	const dsMatrix44f* parentTransform;
	uint64_t nodeID;
} Entry;

typedef struct dsSceneHandoffList
{
	dsSceneItemList itemList;

	Entry* entries;
	uint32_t entryCount;
	uint32_t maxEntries;
	uint64_t nextNodeID;

	uint64_t* removeEntries;
	uint32_t removeEntryCount;
	uint32_t maxRemoveEntries;
} dsSceneHandoffList;

static uint64_t dsSceneHandoffList_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(itemData);
	if (!dsSceneNode_isOfType(node, dsSceneHandoffNode_type()) || !treeNode->parent)
		return DS_NO_SCENE_NODE;

	dsSceneHandoffList* handoffList = (dsSceneHandoffList*)itemList;

	uint32_t index = handoffList->entryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, handoffList->entries,
			handoffList->entryCount, handoffList->maxEntries, 1))
	{
		return DS_NO_SCENE_NODE;
	}

	const dsSceneTreeNode* rootNode = treeNode->parent;
	while (rootNode->parent)
		rootNode = rootNode->parent;

	Entry* entry = handoffList->entries + index;
	entry->transform = treeNode->parent->transform;
	dsQuaternion4_identityRotation(entry->prevOrientation);
	entry->prevPosition.x = 0.0f;
	entry->prevPosition.y = 0.0f;
	entry->prevPosition.z = 0.0f;
	entry->prevScale.x = 1.0f;
	entry->prevScale.y = 1.0f;
	entry->prevScale.z = 1.0f;
	entry->t = 1.0f;
	entry->commonAncestorTransform = &rootNode->transform;
	entry->node = treeNode;
	entry->parentTransform = &treeNode->parent->transform;
	entry->nodeID = handoffList->nextNodeID++;

	treeNode->baseTransform = &entry->transform;
	treeNode->noParentTransform = true;

	return entry->nodeID;
}

static void dsSceneHandoffList_removeNode(
	dsSceneItemList* itemList, dsSceneTreeNode* treeNode, uint64_t nodeID)
{
	DS_UNUSED(treeNode);
	dsSceneHandoffList* handoffList = (dsSceneHandoffList*)itemList;

	uint32_t index = handoffList->removeEntryCount;
	if (DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, handoffList->removeEntries,
			handoffList->removeEntryCount, handoffList->maxRemoveEntries, 1))
	{
		handoffList->removeEntries[index] = nodeID;
	}
	else
	{
		dsSceneItemListEntries_removeSingle(handoffList->entries, &handoffList->entryCount,
			sizeof(Entry), offsetof(Entry, nodeID), nodeID);
	}
}

static void dsSceneHandoffList_reparentNode(dsSceneItemList* itemList, uint64_t nodeID,
	dsSceneTreeNode* prevAncestor, dsSceneTreeNode* newAncestor)
{
	dsSceneHandoffList* handoffList = (dsSceneHandoffList*)itemList;

	Entry* entry = (Entry*)dsSceneItemListEntries_findEntry(handoffList->entries,
		handoffList->entryCount, sizeof(Entry), offsetof(Entry, nodeID), nodeID);
	if (!entry)
		return;

	// Check if there's a common ancestor between the new and old parent.
	const dsSceneTreeNode* commonAncestor = NULL;
	while (prevAncestor)
	{
		const dsSceneTreeNode* nextNewAncestor = newAncestor;
		while (nextNewAncestor && nextNewAncestor != prevAncestor)
			nextNewAncestor = nextNewAncestor->parent;

		if (nextNewAncestor == prevAncestor)
		{
			commonAncestor = prevAncestor;
			break;
		}

		prevAncestor = prevAncestor->parent;
	}

	// Must be a common ancestor, even if it's the global root node.
	if (!commonAncestor)
		return;

	// Get the decomposed original transform relative to the common ancestor.
	dsMatrix44f commonAncestorInv, relativeTransform;
	dsMatrix44f_affineInvert(&commonAncestorInv, &commonAncestor->transform);
	dsMatrix44f_affineMul(
		&relativeTransform, &commonAncestorInv, entry->parentTransform);
	dsMatrix44f_decomposeTransform(&entry->prevPosition, &entry->prevOrientation,
		&entry->prevScale, &relativeTransform);

	// Update the entry now that we got the previous transform information out.
	entry->commonAncestorTransform = &commonAncestor->transform;
	DS_ASSERT(entry->node->parent);
	entry->parentTransform = &entry->node->parent->transform;
	entry->t = 0.0f;
}

static void dsSceneHandoffList_preTransformUpdate(
	dsSceneItemList* itemList, const dsScene* scene, float time)
{
	dsSceneHandoffList* handoffList = (dsSceneHandoffList*)itemList;

	// Lazily remove entries.
	dsSceneItemListEntries_removeMulti(handoffList->entries, &handoffList->entryCount,
		sizeof(Entry), offsetof(Entry, nodeID), handoffList->removeEntries,
		handoffList->removeEntryCount);
	handoffList->removeEntryCount = 0;

	for (uint32_t i = 0; i < handoffList->entryCount; ++i)
	{
		Entry* entry = handoffList->entries + i;
		float transitionTime = ((dsSceneHandoffNode*)entry->node->node)->transitionTime;
		entry->t += time/transitionTime;
		if (entry->t < 1.0f)
		{
			dsVector3f targetPosition, targetScale;
			dsQuaternion4f targetOrientation;
			dsMatrix44f commonAncestorInv, relativeTransform;
			dsMatrix44f_affineInvert(&commonAncestorInv, entry->commonAncestorTransform);
			dsMatrix44f_affineMul(
				&relativeTransform, &commonAncestorInv, entry->parentTransform);
			dsMatrix44f_decomposeTransform(&targetPosition, &targetOrientation, &targetScale,
				&relativeTransform);

			dsVector3f position, scale;
			dsQuaternion4f orientation;
			dsVector3_lerp(position, entry->prevPosition, targetPosition, entry->t);
			dsVector3_lerp(scale, entry->prevScale, targetScale, entry->t);
			dsQuaternion4f_slerp(
				&orientation, &entry->prevOrientation, &targetOrientation, entry->t);

			dsMatrix44f_composeTransform(&relativeTransform, &position, &orientation, &scale);
			dsMatrix44f_affineMul(
				&entry->transform, entry->commonAncestorTransform, &relativeTransform);
		}
		else
		{
			entry->t = 1.0f;
			entry->transform = *entry->parentTransform;
		}
	}
}

static void dsSceneHandoffList_destroy(dsSceneItemList* itemList)
{
	dsSceneHandoffList* handoffList = (dsSceneHandoffList*)itemList;
	DS_VERIFY(dsAllocator_free(itemList->allocator, handoffList->entries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, handoffList->removeEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

const char* const dsSceneHandoffList_typeName = "HandoffList";

dsSceneItemList* dsSceneHandoffList_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const char* name, const uint8_t* data, size_t dataSize)
{
	DS_UNUSED(loadContext);
	DS_UNUSED(scratchData);
	DS_UNUSED(resourceAllocator);
	DS_UNUSED(userData);
	DS_UNUSED(data);
	DS_UNUSED(dataSize);
	return dsSceneHandoffList_create(allocator, name);
}

const dsSceneItemListType* dsSceneHandoffList_type(void)
{
	static dsSceneItemListType type =
	{
		.addNodeFunc = &dsSceneHandoffList_addNode,
		.removeNodeFunc = &dsSceneHandoffList_removeNode,
		.reparentNodeFunc = &dsSceneHandoffList_reparentNode,
		.preTransformUpdateFunc = &dsSceneHandoffList_preTransformUpdate,
		.destroyFunc = &dsSceneHandoffList_destroy
	};
	return &type;
}

dsSceneItemList* dsSceneHandoffList_create(dsAllocator* allocator, const char* name)
{
	if (!allocator || !name)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene handoff list allocator must support freeing memory.");
		return NULL;
	}

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneHandoffList)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsSceneHandoffList* handoffList = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneHandoffList);
	DS_ASSERT(handoffList);

	dsSceneItemList* itemList = (dsSceneItemList*)handoffList;
	itemList->allocator = allocator;
	itemList->type = dsSceneHandoffList_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsUniqueNameID_create(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = false;
	itemList->skipPreRenderPass = false;

	handoffList->entries = NULL;
	handoffList->entryCount = 0;
	handoffList->maxEntries = 0;
	handoffList->nextNodeID = 0;

	return itemList;
}

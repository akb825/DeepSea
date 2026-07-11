/*
 * Copyright 2025-2026 Aaron Barany
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

#include <DeepSea/Math/RigidTransform3.h>

#include <DeepSea/Scene/ItemLists/SceneItemListEntries.h>
#include <DeepSea/Scene/Nodes/SceneHandoffNode.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneTreeNode.h>

#include <string.h>

typedef struct Entry
{
	dsRigidTransform3f fullStepTransform;
	dsRigidTransform3f prevTransform;
	float t;
	dsSceneTreeNode* node;
	const dsSceneTreeNode* commonAncestor;
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
	DS_ASSERT(itemList);
	DS_UNUSED(itemData);
	DS_UNUSED(thisItemData);
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
	dsRigidTransform3f_identity(&entry->prevTransform);
	entry->t = 1.0f;
	entry->node = treeNode;
	entry->commonAncestor = rootNode;
	entry->nodeID = handoffList->nextNodeID++;
	return entry->nodeID;
}

static void dsSceneHandoffList_removeNode(
	dsSceneItemList* itemList, dsSceneTreeNode* treeNode, uint64_t nodeID)
{
	DS_ASSERT(itemList);
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
	DS_ASSERT(itemList);
	dsSceneHandoffList* handoffList = (dsSceneHandoffList*)itemList;

	Entry* entry = (Entry*)dsSceneItemListEntries_findEntry(handoffList->entries,
		handoffList->entryCount, sizeof(Entry), offsetof(Entry, nodeID), nodeID);
	if (!entry || ((dsSceneHandoffNode*)entry->node->node)->transitionTime <= 0.0f)
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

	// Get the original transform relative to the common ancestor.
	dsSceneTreeNode* node = entry->node;
	entry->commonAncestor = commonAncestor;
	dsSceneTreeNode_getCurrentStepRelativeTransform(&entry->prevTransform, node, commonAncestor);

	dsRigidTransform3f commonTransform;
	dsSceneTreeNode_getCurrentStepTransform(&commonTransform, commonAncestor);
	dsRigidTransform3f_mul(&entry->fullStepTransform, &commonTransform, &entry->prevTransform);
	entry->t = 0.0f;

	// Make sure the node transform follows the computed entry transform.
	node->noParentTransform = true;
	node->baseStepTransform = &entry->fullStepTransform;
	dsSceneTreeNode_markDirty(node);
}

static void dsSceneHandoffList_preTransformUpdate(
	dsSceneItemList* itemList, const dsScene* scene, const dsSceneTick* tick, unsigned int step)
{
	DS_ASSERT(itemList);
	DS_ASSERT(tick);
	DS_UNUSED(scene);
	DS_UNUSED(step);

	dsSceneHandoffList* handoffList = (dsSceneHandoffList*)itemList;

	// Lazily remove entries.
	dsSceneItemListEntries_removeMulti(handoffList->entries, &handoffList->entryCount,
		sizeof(Entry), offsetof(Entry, nodeID), handoffList->removeEntries,
		handoffList->removeEntryCount);
	handoffList->removeEntryCount = 0;

	// If time doesn't advance, expect no changes are required.
	if (tick->stepTime == 0.0f)
		return;

	for (uint32_t i = 0; i < handoffList->entryCount; ++i)
	{
		Entry* entry = handoffList->entries + i;
		if (entry->t == 1.0f)
			continue;

		dsSceneTreeNode* node = entry->node;
		float transitionTime = ((dsSceneHandoffNode*)node->node)->transitionTime;
		entry->t += tick->stepTime/transitionTime;
		if (entry->t < 1.0f)
		{
			// Update the latest transform. Use built-in interpolation for transform between frames.
			// It would technically be more correct to compute the interpolated transform relative
			// to the common ancestor, but this would be somewhat expensive to do.
			const dsSceneTreeNode* commonAncestor = entry->commonAncestor;
			dsRigidTransform3f curTransform, commonTransform;
			dsSceneTreeNode_getCurrentStepRelativeTransform(&curTransform, node, commonAncestor);
			dsSceneTreeNode_getCurrentStepTransform(&commonTransform, commonAncestor);
			dsRigidTransform3f_makeOrientationConsistent(
				&curTransform, &entry->prevTransform.orientation);
			dsRigidTransform3f_nearLerp(
				&curTransform, &entry->prevTransform, &curTransform, entry->t);
			dsRigidTransform3f_mul(&entry->fullStepTransform, &commonTransform, &curTransform);
		}
		else
		{
			entry->t = 1.0f;
			// Switch back to using the parent transform as-is.
			node->noParentTransform = false;
			node->baseStepTransform = NULL;
			dsRigidTransform3f_identity(&node->prevStepLocalTransform);
			node->curStepLocalTransform = node->prevStepLocalTransform;
		}

		dsSceneTreeNode_markDirty(node);
	}
}

static void dsSceneHandoffList_destroy(dsSceneItemList* itemList)
{
	DS_ASSERT(itemList);
	dsSceneHandoffList* handoffList = (dsSceneHandoffList*)itemList;
	DS_VERIFY(dsAllocator_free(itemList->allocator, handoffList->entries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, handoffList->removeEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

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

const char* const dsSceneHandoffList_typeName = "HandoffList";

static dsSceneItemListType itemListType =
{
	.addNodeFunc = &dsSceneHandoffList_addNode,
	.removeNodeFunc = &dsSceneHandoffList_removeNode,
	.reparentNodeFunc = &dsSceneHandoffList_reparentNode,
	.preTransformUpdateFunc = &dsSceneHandoffList_preTransformUpdate,
	.destroyFunc = &dsSceneHandoffList_destroy
};

const dsSceneItemListType* dsSceneHandoffList_type(void)
{
	return &itemListType;
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
	size_t fullSize = sizeof(dsSceneHandoffList);
	if (!dsAddAlignedSize(&fullSize, nameLen, DS_ALLOC_ALIGNMENT))
		return NULL;

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
	itemList->viewFilter = NULL;
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

	handoffList->removeEntries = NULL;
	handoffList->removeEntryCount = 0;
	handoffList->maxRemoveEntries = 0;

	return itemList;
}

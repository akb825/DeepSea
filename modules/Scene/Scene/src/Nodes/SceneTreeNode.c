/*
 * Copyright 2019-2026 Aaron Barany
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

#include <DeepSea/Scene/Nodes/SceneTreeNode.h>

#include "Nodes/SceneTreeNodeInternal.h"
#include "SceneTypes.h"

#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/RigidTransform3.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneTransformNode.h>

#include <string.h>

static inline bool isDirty(const dsSceneTreeNode* node, uint64_t stepNumber, float stepT)
{
	/*
	 * Dirty if:
	 * 1. The last updated step is explicitly set to DS_SCENE_TREE_NODE_DIRTY (max value).
	 * 2. The last updated step is this step, and the stepT doesn't match.
	 * 3. The last updated step is before this step, and the stepT isn't 1.
	 */
	return node->lastUpdatedStep > stepNumber ||
		(node->lastUpdatedStep == stepNumber && node->lastUpdatedStepT != stepT) ||
		(node->lastUpdatedStep < stepNumber && node->lastUpdatedStepT != 1.0f);
}

static inline void updateCurFrameWorldTransform(
	dsSceneTreeNode* node, const dsMatrix44f* localTransform)
{
	if (localTransform)
	{
		if (node->parent && !node->noParentTransform)
		{
			dsMatrix44f_affineMul(&node->curFrameWorldTransform,
				&node->parent->curFrameWorldTransform, localTransform);
		}
		else
			node->curFrameWorldTransform = *localTransform;
	}
	else
	{
		if (node->parent)
			node->curFrameWorldTransform = node->parent->curFrameWorldTransform;
		else
			dsMatrix44f_identity(&node->curFrameWorldTransform);
	}
}

static bool updateTransform(dsSceneTreeNode* node, float stepT, bool advanceStep)
{
	dsMatrix44f derivedLocalTransform;
	const dsMatrix44f* localTransform = node->baseFrameTransform;
	if (node->baseStepTransform)
	{
		if (advanceStep)
		{
			node->prevStepLocalTransform = node->curStepLocalTransform;
			node->curStepLocalTransform = *node->baseStepTransform;
		}

		dsRigidTransform3f_makeOrientationConsistent(
			&node->curStepLocalTransform, &node->prevStepLocalTransform.orientation);
		if (!localTransform)
		{
			dsRigidTransform3f curFrameTransform;
			dsRigidTransform3f_nearLerp(&curFrameTransform, &node->prevStepLocalTransform,
				&node->curStepLocalTransform, stepT);
			dsRigidTransform3f_toMatrix(&derivedLocalTransform, &curFrameTransform);
			localTransform = &derivedLocalTransform;
		}
	}
	else if (localTransform)
	{
		dsRigidTransform3f_fromMatrix(&node->prevStepLocalTransform, localTransform);
		node->curStepLocalTransform = node->prevStepLocalTransform;
	}

	updateCurFrameWorldTransform(node, localTransform);
	// Update is finished if there is no base step transform, which will require re-interpolation
	// when stepT increments.
	return node->baseStepTransform == NULL;
}

static void updateOnlyCurTransform(dsSceneTreeNode* node)
{
	dsMatrix44f derivedLocalTransform;
	const dsMatrix44f* localTransform = node->baseFrameTransform;
	if (node->baseStepTransform)
	{
		node->prevStepLocalTransform = node->curStepLocalTransform = *node->baseStepTransform;
		if (!localTransform)
		{
			dsRigidTransform3f_toMatrix(&derivedLocalTransform, node->baseStepTransform);
			localTransform = &derivedLocalTransform;
		}
	}
	else if (localTransform)
	{
		dsRigidTransform3f_fromMatrix(&node->prevStepLocalTransform, localTransform);
		node->curStepLocalTransform = node->prevStepLocalTransform;
	}

	updateCurFrameWorldTransform(node, localTransform);
}

static dsSceneTreeNode* addNode(
	dsSceneTreeNode* node, dsSceneNode* child, dsScene* scene, dsAllocator* allocator)
{
	uint32_t childIndex = node->childCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(node->allocator, node->children, node->childCount,
			node->maxChildren, 1))
	{
		return NULL;
	}

	uint32_t treeNodeIndex = child->treeNodeCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(child->allocator, child->treeNodes, child->treeNodeCount,
			child->maxTreeNodes, 1))
	{
		node->childCount = childIndex;
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneTreeNode)) +
		DS_ALIGNED_SIZE(sizeof(dsSceneItemEntry)*child->itemListCount) +
		DS_ALIGNED_SIZE(sizeof(dsSceneItemData)*child->itemListCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		node->childCount = childIndex;
		child->treeNodeCount = treeNodeIndex;
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneTreeNode* childTreeNode = DS_ALLOCATE_OBJECT(
		&bufferAlloc, dsSceneTreeNode);
	DS_ASSERT(childTreeNode);

	childTreeNode->allocator = allocator;
	childTreeNode->node = dsSceneNode_addRef(child);
	childTreeNode->parent = node;
	childTreeNode->children = NULL;
	childTreeNode->childCount = 0;
	childTreeNode->maxChildren = 0;
	childTreeNode->lastUpdatedStepT = 1.0f;
	childTreeNode->lastUpdatedStep = 0;
	childTreeNode->lastUpdatedFrame = scene->renderer->frameNumber;
	childTreeNode->noParentTransform = false;
	childTreeNode->baseStepTransform = NULL;
	childTreeNode->baseFrameTransform = NULL;
	dsSetupSceneTreeNodeFunction setupTreeNodeFunc = child->type->setupTreeNodeFunc;
	if (setupTreeNodeFunc)
		setupTreeNodeFunc(child, childTreeNode);

	// prev/cur step transforms won't be set if no "base" transforms are set.
	dsRigidTransform3f_identity(&childTreeNode->prevStepLocalTransform);
	childTreeNode->curStepLocalTransform = childTreeNode->prevStepLocalTransform;

	updateOnlyCurTransform(childTreeNode);

	// Seed the previous frame transform with the current frame transform.
	childTreeNode->prevFrameWorldTransform = childTreeNode->curFrameWorldTransform;

	childTreeNode->itemLists = DS_ALLOCATE_OBJECT_ARRAY(
		&bufferAlloc, dsSceneItemEntry, child->itemListCount);
	DS_ASSERT(childTreeNode->itemLists || child->itemListCount == 0);
	childTreeNode->itemData.itemData = DS_ALLOCATE_OBJECT_ARRAY(
		&bufferAlloc, dsSceneItemData, child->itemListCount);
	DS_ASSERT(childTreeNode->itemData.itemData || child->itemListCount == 0);
	childTreeNode->itemData.count = child->itemListCount;

	// Initialize the item list data so searches through it in a node add function won't ever reveal
	// uninitialized data.
	memset(childTreeNode->itemData.itemData, 0, sizeof(dsSceneItemData)*child->itemListCount);

	for (uint32_t i = 0; i < child->itemListCount; ++i)
	{
		dsSceneItemData* itemData = childTreeNode->itemData.itemData + i;
		dsSceneItemEntry* itemEntry = childTreeNode->itemLists + i;

		// Always initialize to NULL, regardless of the branch.
		itemData->data = NULL;

		dsSceneItemListNode* node = (dsSceneItemListNode*)dsHashTable_find(
			scene->itemLists, child->itemLists[i]);
		if (!node || !node->list->type->addNodeFunc || !node->list->type->removeNodeFunc)
		{
			itemEntry->list = NULL;
			itemEntry->entry = DS_NO_SCENE_NODE;
			continue;
		}

		itemData->nameID = node->list->nameID;
		itemEntry->list = node->list;
		itemEntry->entry = node->list->type->addNodeFunc(
			node->list, child, childTreeNode, &childTreeNode->itemData, &itemData->data);
	}

	node->children[childIndex] = childTreeNode;
	child->treeNodes[treeNodeIndex] = childTreeNode;
	return childTreeNode;
}

static bool buildSubtreeRec(dsSceneTreeNode* node, dsSceneNode* child, dsScene* scene)
{
	dsSceneTreeNode* childTreeNode = addNode(node, child, scene, scene->allocator);
	if (!childTreeNode)
		return false;

	for (uint32_t i = 0; i < child->childCount; ++i)
	{
		if (!buildSubtreeRec(childTreeNode, child->children[i], scene))
			return false;
	}

	return true;
}

static uint32_t findTreeNodeIndex(dsSceneTreeNode* treeNode)
{
	dsSceneNode* node = treeNode->node;
	for (uint32_t i = 0; i < node->treeNodeCount; ++i)
	{
		if (node->treeNodes[i] == treeNode)
			return i;
	}

	DS_ASSERT(false);
	return 0;
}

static void removeSubtreeRec(dsSceneNode* child, uint32_t treeNodeIndex, dsScene* scene)
{
	dsSceneTreeNode* childTreeNode = child->treeNodes[treeNodeIndex];
	// Remove the reference in the main node.
	child->treeNodes[treeNodeIndex] = child->treeNodes[child->treeNodeCount - 1];
	--child->treeNodeCount;

	// Recurse for the children.
	for (uint32_t i = 0; i < childTreeNode->childCount; ++i)
	{
		dsSceneTreeNode* nextTreeNode = childTreeNode->children[i];
		removeSubtreeRec(nextTreeNode->node, findTreeNodeIndex(nextTreeNode), scene);
	}

	// Dispose of the node. Remove in reverse order in case there's dependencies between the item
	// lists.
	for (uint32_t i = child->itemListCount; i-- > 0;)
	{
		uint64_t entry = childTreeNode->itemLists[i].entry;
		if (entry == DS_NO_SCENE_NODE)
			continue;

		dsSceneItemList* list = childTreeNode->itemLists[i].list;
		list->type->removeNodeFunc(list, childTreeNode, entry);
	}

	dsSceneNode_freeRef(childTreeNode->node);
	DS_VERIFY(dsAllocator_free(childTreeNode->allocator, childTreeNode->children));
	DS_VERIFY(dsAllocator_free(childTreeNode->allocator, childTreeNode));

	// Remove the node from the scene dirty list.
	for (uint32_t i = 0; i < scene->dirtyNodeCount; ++i)
	{
		if (scene->dirtyNodes[i] != childTreeNode)
			continue;

		scene->dirtyNodes[i] = scene->dirtyNodes[scene->dirtyNodeCount - 1];
		--scene->dirtyNodeCount;
		break;
	}
}

static void notifyReparentSubtreeRec(
	dsSceneTreeNode* node, dsSceneTreeNode* prevParent, dsSceneTreeNode* newParent)
{
	dsSceneNode* baseNode = node->node;
	for (uint32_t i = 0; i < baseNode->itemListCount; ++i)
	{
		dsSceneItemEntry* entry = node->itemLists + i;
		dsReparentSceneItemListNodeFunction reparentNodeFunc = entry->list->type->reparentNodeFunc;
		if (reparentNodeFunc)
			reparentNodeFunc(entry->list, entry->entry, prevParent, newParent);
	}

	for (uint32_t i = 0; i < node->childCount; ++i)
		notifyReparentSubtreeRec(node->children[i], prevParent, newParent);
}

static bool moveSubtree(dsSceneTreeNode* node, dsSceneNode* child, dsSceneTreeNode* newParent)
{
	for (uint32_t i = 0; i < node->childCount;)
	{
		dsSceneTreeNode* curChild = node->children[i];
		if (curChild->node != child)
		{
			++i;
			continue;
		}

		uint32_t index = newParent->childCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(newParent->allocator, newParent->children,
				newParent->childCount, newParent->maxChildren, 1))
		{
			DS_VERIFY(moveSubtree(newParent, child, node));
			return false;
		}

		newParent->children[index] = curChild;
		node->children[i] = node->children[node->childCount - 1];
		--node->childCount;
		curChild->parent = newParent;
		notifyReparentSubtreeRec(node->children[i], node, newParent);
	}

	return true;
}

static void moveToScene(
	dsSceneTreeNode* node, const dsScene* newScene, const dsHashTable* commonItemLists)
{
	for (uint32_t i = 0; i < node->node->itemListCount; ++i)
	{
		dsSceneItemEntry* entry = node->itemLists + i;
		if (entry->entry != DS_NO_SCENE_NODE)
		{
			DS_ASSERT(entry->list);
			if (dsHashTable_find(commonItemLists, entry->list))
				continue;

			dsRemoveSceneItemListNodeFunction removeNodeFunc = entry->list->type->removeNodeFunc;
			if (removeNodeFunc)
				removeNodeFunc(entry->list, node, node->itemLists[i].entry);
		}

		dsSceneItemListNode* foundList = (dsSceneItemListNode*)dsHashTable_find(
			newScene->itemLists, node->node->itemLists[i]);
		if (foundList && foundList->list->type->addNodeFunc &&
			foundList->list->type->removeNodeFunc)
		{
			dsSceneItemList* list = foundList->list;
			dsSceneItemData* itemData = node->itemData.itemData + i;
			itemData->nameID = list->nameID;
			entry->list = list;
			entry->entry = list->type->addNodeFunc(
				list, node->node, node, &node->itemData, &itemData->data);
		}
		else
		{
			entry->list = NULL;
			entry->entry = DS_NO_SCENE_NODE;
		}
	}

	for (uint32_t i = 0; i < node->childCount; ++i)
		moveToScene(node->children[i], newScene, commonItemLists);
}

static bool updateSubtreeRec(
	dsSceneTreeNode* node, uint64_t frameNumber, uint64_t stepNumber, float stepT)
{
	// Check frame for prevFrameWorldTransform as multiple updates may occur per frame.
	if (node->lastUpdatedFrame != frameNumber)
	{
		node->prevFrameWorldTransform = node->curFrameWorldTransform;
		node->lastUpdatedFrame = frameNumber;
	}

	// If this was last updated in a previous step, then the transform becomes the current step's
	// transform, ignoring stepT for the current step.
	bool updateFinished;
	if (node->lastUpdatedStep < stepNumber)
	{
		updateOnlyCurTransform(node);
		node->lastUpdatedStepT = 1.0f;
		updateFinished = true;
	}
	else
	{
		updateFinished = updateTransform(node, stepT, node->lastUpdatedStep > stepNumber);
		node->lastUpdatedStep = stepNumber;
		node->lastUpdatedStepT = stepT;
	}

	for (uint32_t i = 0; i < node->node->itemListCount; ++i)
	{
		const dsSceneItemEntry* itemListEntry = node->itemLists + i;
		uint64_t entry = itemListEntry->entry;
		dsSceneItemList* list = itemListEntry->list;
		if (entry != DS_NO_SCENE_NODE && list->type->updateNodeFunc)
			list->type->updateNodeFunc(list, node, entry);
	}

	for (uint32_t i = 0; i < node->childCount; ++i)
		updateFinished &= updateSubtreeRec(node->children[i], frameNumber, stepNumber, stepT);
	return updateFinished;
}

static void updateSubtreeOnlyCurTransformRec(
	dsSceneTreeNode* node, uint64_t frameNumber, uint64_t stepNumber)
{
	// Check frame for prevFrameWorldTransform as multiple updates may occur per frame.
	if (node->lastUpdatedFrame != frameNumber)
	{
		node->prevFrameWorldTransform = node->curFrameWorldTransform;
		node->lastUpdatedFrame = frameNumber;
	}

	updateOnlyCurTransform(node);
	node->lastUpdatedStep = stepNumber;
	node->lastUpdatedStepT = 1.0f;
	for (uint32_t i = 0; i < node->node->itemListCount; ++i)
	{
		const dsSceneItemEntry* itemListEntry = node->itemLists + i;
		uint64_t entry = itemListEntry->entry;
		dsSceneItemList* list = itemListEntry->list;
		if (entry != DS_NO_SCENE_NODE && list->type->updateNodeFunc)
			list->type->updateNodeFunc(list, node, entry);
	}

	for (uint32_t i = 0; i < node->childCount; ++i)
		updateSubtreeOnlyCurTransformRec(node->children[i], frameNumber, stepNumber);
}

dsScene* dsSceneTreeNode_getScene(dsSceneTreeNode* node)
{
	DS_ASSERT(node);
	while (node->parent)
		node = node->parent;
	return ((dsSceneTreeRootNode*)node)->scene;
}

bool dsSceneTreeNode_buildSubtree(dsSceneNode* node, dsSceneNode* child)
{
	for (uint32_t i = 0; i < node->treeNodeCount; ++i)
	{
		dsSceneTreeNode* treeNode = node->treeNodes[i];
		dsScene* scene = dsSceneTreeNode_getScene(treeNode);
		DS_ASSERT(scene);
		if (!buildSubtreeRec(treeNode, child, scene))
		{
			dsSceneTreeNode_removeSubtree(node, child);
			return false;
		}
	}

	return true;
}

void dsSceneTreeNode_removeSubtree(dsSceneNode* node, dsSceneNode* child)
{
	// Find all tree nodes of the parent that are a parent to the current child.
	// Don't increment i if removing the child, since the index will be removed.
	for (uint32_t i = 0; i < child->treeNodeCount;)
	{
		dsSceneTreeNode* childTreeNode = child->treeNodes[i];
		dsSceneTreeNode* treeNode = childTreeNode->parent;
		if (treeNode->node != node)
		{
			++i;
			continue;
		}

		dsScene* scene = dsSceneTreeNode_getScene(treeNode);
		DS_ASSERT(scene);
		removeSubtreeRec(child, i, scene);

		// Find and remove the entry in the parent tree node corresponding to the child just
		// removed.
		for (uint32_t j = 0; j < treeNode->childCount; ++j)
		{
			if (treeNode->children[j] == childTreeNode)
			{
				treeNode->children[j] = treeNode->children[treeNode->childCount - 1];
				--treeNode->childCount;
				break;
			}
		}
	}
}

bool dsSceneTreeNode_reparentSubtree(dsSceneNode* node, dsSceneNode* child, dsSceneNode* newParent)
{
	DS_ASSERT(node->treeNodeCount == newParent->treeNodeCount);
	for (uint32_t i = 0; i < node->treeNodeCount; ++i)
	{
		dsSceneTreeNode* fromParent = node->treeNodes[i];
		dsSceneTreeNode* toParent = newParent->treeNodes[i];
		if (!moveSubtree(fromParent, child, toParent))
		{
			// Move children back on failure. Should be guaranteed to succeed as no allocations will
			// take place,
			for (uint32_t j = 0; j < i; ++j)
				DS_VERIFY(moveSubtree(newParent->treeNodes[j], child, node->treeNodes[j]));
			return false;
		}
	}

	return true;
}

bool dsSceneTreeNode_transferSceneNodes(dsSceneNode* prevRoot, dsSceneNode* newRoot,
	const dsScene* newScene, const dsHashTable* commonItemLists)
{
	DS_ASSERT(newRoot->childCount == 0);
	if (!DS_RESIZEABLE_ARRAY_ADD(newRoot->allocator, newRoot->children, newRoot->childCount,
			newRoot->maxChildren, prevRoot->childCount))
	{
		return false;
	}

	DS_ASSERT(prevRoot->treeNodeCount == 1);
	DS_ASSERT(newRoot->treeNodeCount == 1);
	dsSceneTreeNode* prevTreeRoot = prevRoot->treeNodes[0];
	DS_ASSERT(newRoot->treeNodes[0]->childCount == 0);
	dsSceneTreeNode* newTreeRoot = newRoot->treeNodes[0];
	if (!DS_RESIZEABLE_ARRAY_ADD(newRoot->allocator, newTreeRoot->children,
			newTreeRoot->childCount, newTreeRoot->maxChildren, prevTreeRoot->childCount))
	{
		return false;
	}

	for (uint32_t i = 0; i < newRoot->childCount; ++i)
		newRoot->children[i] = prevRoot->children[i];

	for (uint32_t i = 0; i < newTreeRoot->childCount; ++i)
	{
		dsSceneTreeNode* child = prevTreeRoot->children[i];
		newTreeRoot->children[i] = child;
		child->parent = newTreeRoot;
		moveToScene(child, newScene, commonItemLists);
	}

	prevRoot->childCount = 0;
	prevTreeRoot->childCount = 0;
	return true;
}

bool dsSceneTreeNode_updateSubtree(
	dsSceneTreeNode* node, uint64_t frameNumber, uint64_t stepNumber, float stepT)
{
	// This may have already been updated by a different subtree.
	if (!isDirty(node, stepNumber, stepT))
		return true;

	// Find the top-most dirty node to update from.
	while (node->parent && isDirty(node->parent, stepNumber, stepT))
		node = node->parent;

	// Check for stepT equal to 1 for either intermediate steps or dynamic updates where the
	// transforms don't need interpolation.
	if (stepT < 1.0f)
		return updateSubtreeRec(node, frameNumber, stepNumber, stepT);

	updateSubtreeOnlyCurTransformRec(node, frameNumber, stepNumber);
	return true;
}

void dsSceneTreeNode_markDirty(dsSceneTreeNode* node)
{
	DS_ASSERT(node);
	dsScene* scene = dsSceneTreeNode_getScene(node);
	DS_ASSERT(scene);

	node->lastUpdatedStep = DS_SCENE_TREE_NODE_DIRTY;
	// Since the dirty value is used, don't bother a linear search to see if already on the list.
	uint32_t index = scene->dirtyNodeCount;
	if (DS_RESIZEABLE_ARRAY_ADD(
			scene->allocator, scene->dirtyNodes, scene->dirtyNodeCount, scene->maxDirtyNodes, 1))
	{
		scene->dirtyNodes[index] = node;
	}
}

void dsSceneTreeNode_getCurrentStepTransform(
	dsRigidTransform3f* outTransform, const dsSceneTreeNode* node)
{
	DS_ASSERT(outTransform);
	DS_ASSERT(node);

	if (node->baseStepTransform)
		*outTransform = *node->baseStepTransform;
	else if (node->baseFrameTransform)
		dsRigidTransform3f_fromMatrix(outTransform, node->baseFrameTransform);
	else
		dsRigidTransform3f_identity(outTransform);
	while (node->parent && !node->noParentTransform)
	{
		node = node->parent;
		if (node->baseStepTransform)
			dsRigidTransform3f_mul(outTransform, node->baseStepTransform, outTransform);
		else if (node->baseFrameTransform)
		{
			dsRigidTransform3f frameTransform;
			dsRigidTransform3f_fromMatrix(&frameTransform, node->baseFrameTransform);
			dsRigidTransform3f_mul(outTransform, &frameTransform, outTransform);
		}
	}
}

void dsSceneTreeNode_getCurrentStepRelativeTransform(dsRigidTransform3f* outTransform,
	const dsSceneTreeNode* node, const dsSceneTreeNode* ancestorNode)
{
	DS_ASSERT(outTransform);
	DS_ASSERT(node);
	DS_ASSERT(ancestorNode);
	DS_ASSERT(node != ancestorNode);

	if (node->baseStepTransform)
		*outTransform = *node->baseStepTransform;
	else if (node->baseFrameTransform)
		dsRigidTransform3f_fromMatrix(outTransform, node->baseFrameTransform);
	else
		dsRigidTransform3f_identity(outTransform);
	while (node->parent && node->parent != ancestorNode && !node->noParentTransform)
	{
		node = node->parent;
		if (node->baseStepTransform)
			dsRigidTransform3f_mul(outTransform, node->baseStepTransform, outTransform);
		else if (node->baseFrameTransform)
		{
			dsRigidTransform3f frameTransform;
			dsRigidTransform3f_fromMatrix(&frameTransform, node->baseFrameTransform);
			dsRigidTransform3f_mul(outTransform, &frameTransform, outTransform);
		}
	}

	DS_ASSERT(node->noParentTransform || node->parent == ancestorNode);
	// If the chain was cut off by a node with an explicit transform, must manually compute the
	// relative transform.
	if (node->noParentTransform)
	{
		dsRigidTransform3f ancestorTransform, ancestorTransformInv;
		dsSceneTreeNode_getCurrentStepTransform(&ancestorTransform, ancestorNode);
		dsRigidTransform3f_invert(&ancestorTransformInv, &ancestorTransform);
		dsRigidTransform3f_mul(outTransform, &ancestorTransformInv, outTransform);
	}
}

void dsSceneTreeNode_getStepTransforms(dsRigidTransform3f* outPrevTransform,
	dsRigidTransform3f* outCurTransform, const dsSceneTreeNode* node, uint64_t stepNumber)
{
	DS_ASSERT(outPrevTransform);
	DS_ASSERT(outCurTransform);
	DS_ASSERT(node);

	if (node->baseStepTransform)
		*outCurTransform = *node->baseStepTransform;
	else if (node->baseFrameTransform)
		dsRigidTransform3f_fromMatrix(outCurTransform, node->baseFrameTransform);
	else
		dsRigidTransform3f_identity(outCurTransform);

	*outPrevTransform = node->lastUpdatedStep == stepNumber ?
		node->prevStepLocalTransform : node->curStepLocalTransform;

	while (node->parent && !node->noParentTransform)
	{
		node = node->parent;
		if (node->baseStepTransform)
			dsRigidTransform3f_mul(outCurTransform, node->baseStepTransform, outCurTransform);
		else if (node->baseFrameTransform)
		{
			dsRigidTransform3f frameTransform;
			dsRigidTransform3f_fromMatrix(&frameTransform, node->baseFrameTransform);
			dsRigidTransform3f_mul(outCurTransform, &frameTransform, outCurTransform);
		}
		else
			continue; // No need to update previous transform.

		const dsRigidTransform3f* prevLocalTransform = node->lastUpdatedStep == stepNumber ?
			&node->prevStepLocalTransform : &node->curStepLocalTransform;
		dsRigidTransform3f_mul(outPrevTransform, prevLocalTransform, outPrevTransform);
	}
}

void dsSceneTreeNode_getStepRelativeTransforms(dsRigidTransform3f* outPrevTransform,
	dsRigidTransform3f* outCurTransform, const dsSceneTreeNode* node,
	const dsSceneTreeNode* ancestorNode, uint64_t stepNumber)
{
	DS_ASSERT(outPrevTransform);
	DS_ASSERT(outCurTransform);
	DS_ASSERT(node);

	if (node->baseStepTransform)
		*outCurTransform = *node->baseStepTransform;
	else if (node->baseFrameTransform)
		dsRigidTransform3f_fromMatrix(outCurTransform, node->baseFrameTransform);
	else
		dsRigidTransform3f_identity(outCurTransform);

	*outPrevTransform = node->lastUpdatedStep == stepNumber ?
		node->prevStepLocalTransform : node->curStepLocalTransform;

	while (node->parent && node->parent != ancestorNode && !node->noParentTransform)
	{
		node = node->parent;
		if (node->baseStepTransform)
			dsRigidTransform3f_mul(outCurTransform, node->baseStepTransform, outCurTransform);
		else if (node->baseFrameTransform)
		{
			dsRigidTransform3f frameTransform;
			dsRigidTransform3f_fromMatrix(&frameTransform, node->baseFrameTransform);
			dsRigidTransform3f_mul(outCurTransform, &frameTransform, outCurTransform);
		}
		else
			continue; // No need to update previous transform.

		const dsRigidTransform3f* prevLocalTransform = node->lastUpdatedStep == stepNumber ?
			&node->prevStepLocalTransform : &node->curStepLocalTransform;
		dsRigidTransform3f_mul(outPrevTransform, prevLocalTransform, outPrevTransform);
	}

	DS_ASSERT(node->noParentTransform || node->parent == ancestorNode);
	// If the chain was cut off by a node with an explicit transform, must manually compute the
	// relative transforms.
	if (node->noParentTransform)
	{
		dsRigidTransform3f ancestorPrevTransform, ancestorCurTransform, ancestorTransformInv;
		dsSceneTreeNode_getStepTransforms(
			&ancestorPrevTransform, &ancestorCurTransform, ancestorNode, stepNumber);
		dsRigidTransform3f_invert(&ancestorTransformInv, &ancestorPrevTransform);
		dsRigidTransform3f_mul(outPrevTransform, &ancestorTransformInv, outPrevTransform);
		dsRigidTransform3f_invert(&ancestorTransformInv, &ancestorCurTransform);
		dsRigidTransform3f_mul(outCurTransform, &ancestorTransformInv, outCurTransform);
	}
}

uint64_t dsSceneTreeNode_getNodeID(const dsSceneTreeNode* node, const dsSceneItemList* itemList)
{
	DS_ASSERT(node);
	DS_ASSERT(itemList);
	for (uint32_t i = 0; i < node->node->itemListCount; ++i)
	{
		const dsSceneItemEntry* entry = node->itemLists + i;
		if (entry->list == itemList)
			return entry->entry;
	}

	return DS_NO_SCENE_NODE;
}

/*
 * Copyright 2019-2023 Aaron Barany
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
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneTransformNode.h>

#include <string.h>

static void updateTransform(dsSceneTreeNode* node)
{
	if (node->node->type == dsSceneTransformNode_type())
	{
		dsSceneTransformNode* transformNode = (dsSceneTransformNode*)node->node;
		if (node->parent)
		{
			dsMatrix44f_affineMul(&node->transform, &node->parent->transform,
				&transformNode->transform);
		}
		else
			node->transform = transformNode->transform;
	}
	else
	{
		if (node->parent)
			node->transform = node->parent->transform;
		else
			dsMatrix44_identity(node->transform);
	}
	if (node->parent == NULL)
	{
		if (node->node->type == dsSceneTransformNode_type())
			node->transform = ((dsSceneTransformNode*)node)->transform;
		else
			dsMatrix44_identity(node->transform);
	}
}

static dsSceneTreeNode* addNode(dsSceneTreeNode* node, dsSceneNode* child,
	dsScene* scene, dsAllocator* allocator)
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

	dsSceneTreeNode* childTreeNode = DS_ALLOCATE_OBJECT(&bufferAlloc,
		dsSceneTreeNode);
	DS_ASSERT(childTreeNode);

	childTreeNode->allocator = allocator;
	childTreeNode->node = dsSceneNode_addRef(child);
	childTreeNode->parent = node;
	childTreeNode->children = NULL;
	childTreeNode->childCount = 0;
	childTreeNode->maxChildren = 0;
	childTreeNode->dirty = false;
	updateTransform(childTreeNode);

	childTreeNode->itemLists = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneItemEntry,
		child->itemListCount);
	DS_ASSERT(childTreeNode->itemLists || child->itemListCount == 0);
	childTreeNode->itemData.itemData = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneItemData,
		child->itemListCount);
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

		dsSceneItemListNode* node = (dsSceneItemListNode*)dsHashTable_find(scene->itemLists,
			child->itemLists[i]);
		if (!node || !node->list->addNodeFunc || !node->list->removeNodeFunc)
		{
			itemEntry->list = NULL;
			itemEntry->entry = DS_NO_SCENE_NODE;
			continue;
		}

		itemData->nameID = node->list->nameID;
		itemEntry->list = node->list;
		itemEntry->entry = node->list->addNodeFunc(node->list, child, childTreeNode,
			&childTreeNode->itemData, &itemData->data);
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
		list->removeNodeFunc(list, entry);
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
	}

	return true;
}

static void updateSubtreeRec(dsSceneTreeNode* node)
{
	updateTransform(node);
	node->dirty = false;
	for (uint32_t i = 0; i < node->node->itemListCount; ++i)
	{
		uint64_t entry = node->itemLists[i].entry;
		dsSceneItemList* list = node->itemLists[i].list;
		if (entry != DS_NO_SCENE_NODE && list->updateNodeFunc)
			list->updateNodeFunc(list, entry);
	}

	for (uint32_t i = 0; i < node->childCount; ++i)
		updateSubtreeRec(node->children[i]);
}

const dsSceneNode* dsSceneTreeNode_getNode(const dsSceneTreeNode* node)
{
	if (!node)
	{
		errno = EINVAL;
		return NULL;
	}

	return node->node;
}

const dsSceneTreeNode* dsSceneTreeNode_getParent(const dsSceneTreeNode* node)
{
	return node ? node->parent : NULL;
}

uint32_t dsSceneTreeNode_getChildCount(const dsSceneTreeNode* node)
{
	return node ? node->childCount : 0;
}

const dsSceneTreeNode* dsSceneTreeNode_getChild(const dsSceneTreeNode* node, uint32_t index)
{
	if (!node)
	{
		errno = EINVAL;
		return NULL;
	}

	if (index >= node->childCount)
	{
		errno = EINDEX;
		return NULL;
	}

	return node->children[index];
}

uint32_t dsSceneTreeNode_getItemListCount(const dsSceneTreeNode* node)
{
	return node ? node->node->itemListCount : 0;
}

const dsSceneItemList* dsSceneTreeNode_getItemList(const dsSceneTreeNode* node, uint32_t index)
{
	if (!node || index >= node->node->itemListCount)
		return NULL;

	return node->itemLists[index].list;
}

const dsMatrix44f* dsSceneTreeNode_getTransform(const dsSceneTreeNode* node)
{
	if (!node)
	{
		errno = EINVAL;
		return NULL;
	}

	return &node->transform;
}

const dsSceneNodeItemData* dsSceneTreeNode_getItemData(const dsSceneTreeNode* node)
{
	if (!node)
	{
		errno = EINVAL;
		return NULL;
	}

	return &node->itemData;
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

void dsSceneTreeNode_markDirty(dsSceneTreeNode* node)
{
	dsScene* scene = dsSceneTreeNode_getScene(node);
	DS_ASSERT(scene);

	node->dirty = true;
	// Since the dirty flag is used, don't bother a linear search to see if already on the list.
	uint32_t index = scene->dirtyNodeCount;
	if (DS_RESIZEABLE_ARRAY_ADD(scene->allocator, scene->dirtyNodes, scene->dirtyNodeCount,
			scene->maxDirtyNodes, 1))
	{
		scene->dirtyNodes[index] = node;
	}
}

void dsSceneTreeNode_updateSubtree(dsSceneTreeNode* node)
{
	// This may have already been updated by a different subtree.
	if (!node->dirty)
		return;

	// Find the top-most dirty node to update from.
	while (node->parent && node->parent->dirty)
		node = node->parent;

	updateSubtreeRec(node);
}

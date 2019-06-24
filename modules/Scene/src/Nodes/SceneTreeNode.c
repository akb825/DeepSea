/*
 * Copyright 2019 Aaron Barany
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

#include "Nodes/SceneTreeNode.h"

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

static void updateTransform(dsSceneTreeNode* node)
{
	if (node->node.node->type == dsSceneTransformNode_type())
	{
		dsSceneTransformNode* transformNode = (dsSceneTransformNode*)node;
		if (node->parent)
			dsMatrix44_mul(node->transform, node->parent->transform, transformNode->transform);
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
		if (node->node.node->type == dsSceneTransformNode_type())
			node->transform = ((dsSceneTransformNode*)node)->transform;
		else
			dsMatrix44_identity(node->transform);
	}
}

static dsSceneTreeNode* addNode(dsSceneTreeNode* node, const dsSceneNodeChildRef* child,
	dsScene* scene, dsAllocator* allocator)
{
	uint32_t childIndex = node->parent->childCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(node->allocator, node->children, node->childCount,
			node->maxChildren, 1))
	{
		return NULL;
	}

	dsSceneNode* childNode = child->node;
	uint32_t treeNodeIndex = childNode->treeNodeCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(childNode->allocator, childNode->treeNodes,
			childNode->treeNodeCount, childNode->maxTreeNodes, 1))
	{
		node->childCount = childIndex;
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneTreeNode)) +
		DS_ALIGNED_SIZE(sizeof(dsSceneItemEntry)*childNode->drawListCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		node->childCount = childIndex;
		childNode->treeNodeCount = treeNodeIndex;
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneTreeNode* childTreeNode = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc,
		dsSceneTreeNode);
	DS_ASSERT(childTreeNode);

	dsSceneNode_addRef(childNode);
	childTreeNode->allocator = allocator;
	childTreeNode->node = *child;
	childTreeNode->parent = node;
	childTreeNode->children = NULL;
	childTreeNode->childCount = 0;
	childTreeNode->maxChildren = 0;
	childTreeNode->dirty = false;
	updateTransform(childTreeNode);

	childTreeNode->drawItems= DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
		dsSceneItemEntry, childNode->drawListCount);
	DS_ASSERT(childTreeNode->drawItems || childNode->drawListCount == 0);
	for (uint32_t i = 0; i < childNode->drawListCount; ++i)
	{
		dsSceneItemListNode* node = (dsSceneItemListNode*)dsHashTable_find(scene->itemLists,
			childNode->drawLists[i]);
		if (!node)
		{
			childTreeNode->drawItems[i].list = NULL;
			childTreeNode->drawItems[i].entry = DS_NO_SCENE_NODE;
			continue;
		}

		childTreeNode->drawItems[i].list = node->list;
		childTreeNode->drawItems[i].entry = node->list->addNodeFunc(node->list, childNode,
			&childTreeNode->transform);
	}

	node->children[childIndex] = childTreeNode;
	childNode->treeNodes[treeNodeIndex] = childTreeNode;
	return childTreeNode;
}

static bool buildSubtreeRec(dsSceneTreeNode* node, const dsSceneNodeChildRef* child, dsScene* scene)
{
	dsSceneTreeNode* childTreeNode = addNode(node, child, scene, scene->allocator);
	if (!childTreeNode)
		return false;

	dsSceneNode* childNode = child->node;
	for (uint32_t i = 0; i < childNode->childCount; ++i)
	{
		if (!buildSubtreeRec(childTreeNode, childNode->children + i, scene))
			return false;
	}

	return true;
}

static uint32_t findTreeNodeIndex(dsSceneTreeNode* treeNode)
{
	dsSceneNode* node = treeNode->node.node;
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
		removeSubtreeRec(nextTreeNode->node.node, findTreeNodeIndex(nextTreeNode), scene);
	}

	// Dispose of the node.
	for (uint32_t i = 0; i < child->drawListCount; ++i)
	{
		uint32_t entry = childTreeNode->drawItems[i].entry;
		if (entry == DS_NO_SCENE_NODE)
			continue;

		dsSceneItemList* list = childTreeNode->drawItems[i].list;
		list->removeNodeFunc(list, entry);
	}

	dsSceneNode_freeRef(childTreeNode->node.node);
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

static void updateSubtreeRec(dsSceneTreeNode* node)
{
	updateTransform(node);
	node->dirty = false;
	for (uint32_t i = 0; i < node->childCount; ++i)
		updateSubtreeRec(node->children[i]);
}

dsScene* dsSceneTreeNode_getScene(dsSceneTreeNode* node)
{
	DS_ASSERT(node);
	while (node->parent)
		node = node->parent;
	return ((dsSceneTreeRootNode*)node)->scene;
}

bool dsSceneTreeNode_buildSubtree(dsSceneNode* node, const dsSceneNodeChildRef* child)
{
	for (uint32_t i = 0; i < node->treeNodeCount; ++i)
	{
		dsSceneTreeNode* treeNode = node->treeNodes[i];
		dsScene* scene = dsSceneTreeNode_getScene(treeNode);
		DS_ASSERT(scene);
		if (!buildSubtreeRec(treeNode, child, scene))
		{
			dsSceneTreeNode_removeSubtree(node, child->node, child->childID);
			return false;
		}
	}

	return true;
}

void dsSceneTreeNode_removeSubtree(dsSceneNode* node, dsSceneNode* child, uint32_t childID)
{
	// Find all tree nodes of the parent that are a parent to the current child.
	// Don't increment i if removing the child, since the index will be removed.
	for (uint32_t i = 0; i < child->treeNodeCount;)
	{
		dsSceneTreeNode* childTreeNode = child->treeNodes[i];
		if (childID != DS_NO_SCENE_NODE && childTreeNode->node.childID != childID)
			continue;

		dsSceneTreeNode* treeNode = childTreeNode->parent;
		if (treeNode->node.node != node)
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
				treeNode->children[i] = treeNode->children[treeNode->childCount - 1];
				--treeNode->childCount;
				break;
			}
		}
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

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

#include <DeepSea/Scene/Nodes/SceneNode.h>

#include "Nodes/SceneTreeNode.h"
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <string.h>

size_t dsSceneNode_drawListsAllocSize(const char** drawLists, uint32_t drawListCount)
{
	if (drawListCount == 0)
		return 0;

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(const char*)*drawListCount);
	for (uint32_t i = 0; i < drawListCount; ++i)
		fullSize += DS_ALIGNED_SIZE(strlen(drawLists[i]));
	return fullSize;
}

bool dsSceneNode_initialize(dsSceneNode* node, dsAllocator* allocator,
	dsSceneNodeType type, const char** drawLists, uint32_t drawListCount,
	dsDestroySceneNodeFunction destroyFunc)
{
	if (!node || !allocator || !destroyFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene node allocator must support freeing memory.");
		return false;
	}

	node->allocator = allocator;
	node->type = type;
	node->children = NULL;
	node->drawLists = drawLists;
	node->treeNodes = NULL;
	node->childCount = 0;
	node->maxChildren = 0;
	node->drawListCount = drawListCount;
	node->treeNodeCount = 0;
	node->maxTreeNodes = 0;
	node->refCount = 1;
	node->nextChildID = 0;
	node->destroyFunc = destroyFunc;
	return true;
}

uint32_t dsSceneNode_addChild(dsSceneNode* node, dsSceneNode* child)
{
	if (!node || !child)
	{
		errno = EINVAL;
		return DS_NO_SCENE_NODE;
	}

	uint32_t index = node->childCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(node->allocator, node->children, node->childCount,
			node->maxChildren, 1))
	{
		return DS_NO_SCENE_NODE;
	}

	dsSceneNodeChildRef* childRef = node->children + index;
	childRef->node = dsSceneNode_addRef(child);
	childRef->childID = node->nextChildID++;
	if (!dsSceneTreeNode_buildSubtree(node, childRef))
	{
		dsSceneNode_freeRef(child);
		--node->childCount;
		return DS_NO_SCENE_NODE;
	}

	return childRef->childID;
}

bool dsSceneNode_removeChildIndex(dsSceneNode* node, uint32_t childIndex)
{
	if (!node)
	{
		errno = EINVAL;
		return false;
	}

	if (childIndex >= node->childCount)
	{
		errno = EINDEX;
		return false;
	}

	dsSceneNodeChildRef* child = node->children + childIndex;
	dsSceneTreeNode_removeSubtree(node, child->node, child->childID);
	// Order shouldn't matter, so put the last item in this spot for constant-time removal.
	node->children[childIndex] = node->children[node->childCount - 1];
	--node->childCount;
	return true;
}

bool dsSceneNode_removeChildNode(dsSceneNode* node, dsSceneNode* child, uint32_t childID)
{
	if (!node || !child)
	{
		errno = EINVAL;
		return false;
	}

	dsSceneTreeNode_removeSubtree(node, child, childID);
	for (uint32_t i = node->childCount; i-- > 0;)
	{
		dsSceneNodeChildRef* childRef = node->children + i;
		if (childRef->node != child ||
			(childID != DS_NO_SCENE_NODE && childRef->childID != childID))
		{
			continue;
		}

		dsSceneNode_freeRef(child);
		node->children[i] = node->children[node->childCount - 1];
		--node->childCount;
	}
	return true;
}

void dsSceneNode_clear(dsSceneNode* node)
{
	if (!node)
		return;

	for (uint32_t i = 0; i < node->childCount; ++i)
	{
		dsSceneNode* child = node->children[i].node;
		dsSceneTreeNode_removeSubtree(node, child, DS_NO_SCENE_NODE);
		dsSceneNode_freeRef(child);
	}
	node->childCount = 0;
}

dsSceneNode* dsSceneNode_addRef(dsSceneNode* node)
{
	if (!node)
		return NULL;

	DS_ATOMIC_FETCH_ADD32(&node->refCount, 1);
	return node;
}

void dsSceneNode_freeRef(dsSceneNode* node)
{
	if (!node)
		return;

	if (DS_ATOMIC_FETCH_ADD32(&node->refCount, -1) != 1)
		return;

	dsSceneNode_clear(node);
	DS_VERIFY(dsAllocator_free(node->allocator, node->children));
	DS_VERIFY(dsAllocator_free(node->allocator, node->treeNodes));

	if (node->destroyFunc)
		node->destroyFunc(node);
}

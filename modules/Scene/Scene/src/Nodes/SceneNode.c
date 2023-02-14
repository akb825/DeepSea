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

#include <DeepSea/Scene/Nodes/SceneNode.h>

#include "Nodes/SceneTreeNodeInternal.h"
#include "SceneTypes.h"

#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <string.h>

// NOTE: This would ideally be in Scene.c, but this seems to expose an issue on Mac when static
// linking. My guess is it only looks at functions to decide which object files to bring in, so if
// only this global variable is referenced for Scene.o, SceneNode.o doesn't bring in this variable.
// Since Scene.o will reference functions in SceneNode.o, this *should* always work.
dsSceneNodeType dsRootSceneNodeType;

static dsSceneTreeNode* findUniqueTreeNodeRec(dsSceneTreeNode* treeNode,
	const dsSceneNode* descendentNode)
{
	for (uint32_t i = 0; i < treeNode->childCount; ++i)
	{
		dsSceneTreeNode* child = treeNode->children[i];
		if (child->node == descendentNode)
			return child;
		else if (child->childCount > 0)
		{
			dsSceneTreeNode* foundNode = findUniqueTreeNodeRec(child, descendentNode);
			if (foundNode)
				return foundNode;
		}
	}

	return NULL;
}

const char* const dsSceneNodeRef_typeName = "ReferenceNode";

size_t dsSceneNode_itemListsAllocSize(const char* const* itemLists, uint32_t itemListCount)
{
	if (itemListCount == 0)
		return 0;

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(const char*)*itemListCount);
	for (uint32_t i = 0; i < itemListCount; ++i)
	{
		if (!itemLists[i])
			return 0;

		fullSize += DS_ALIGNED_SIZE(strlen(itemLists[i]) + 1);
	}
	return fullSize;
}

const char* const* dsSceneNode_copyItemLists(dsBufferAllocator* allocator,
	const char* const* itemLists, uint32_t itemListCount)
{
	if (!allocator || !itemLists || itemListCount == 0)
		return NULL;

	char** itemListsCopy = DS_ALLOCATE_OBJECT_ARRAY(allocator, char*, itemListCount);
	if (!itemListsCopy)
		return NULL;

	for (uint32_t i = 0; i < itemListCount; ++i)
	{
		size_t nameLen = itemLists[i] ? strlen(itemLists[i]) + 1 : 0;
		itemListsCopy[i] = DS_ALLOCATE_OBJECT_ARRAY(allocator, char, nameLen);
		if (!itemListsCopy[i])
			return NULL;
		memcpy(itemListsCopy[i], itemLists[i], nameLen);
	}

	return (const char* const*)itemListsCopy;
}

const dsSceneNodeType* dsSceneNode_setupParentType(dsSceneNodeType* type,
	const dsSceneNodeType* parentType)
{
	if (!type)
		return parentType;

	dsSceneNodeType* expectedParent = NULL;
	DS_ATOMIC_COMPARE_EXCHANGE_PTR(&type->parent, &expectedParent, &parentType, false);
	return type;
}

dsSceneNode* dsSceneNode_load(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData, const char* type,
	const void* data, size_t size)
{
	if (!allocator || !loadContext || !scratchData || !type || (!data && size > 0))
	{
		errno = EINVAL;
		return NULL;
	}

	dsLoadSceneNodeItem* foundType =
		(dsLoadSceneNodeItem*)dsHashTable_find(&loadContext->nodeTypeTable.hashTable, type);
	if (!foundType)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Unknown scene node type '%s'.", type);
		return NULL;
	}

	dsSceneNode* node = foundType->loadFunc(loadContext, scratchData, allocator, resourceAllocator,
		foundType->userData, (const uint8_t*)data, size);
	if (!node)
	{
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Failed to load scene node '%s': %s.", type,
			dsErrorString(errno));
	}
	return node;
}

bool dsSceneNode_initialize(dsSceneNode* node, dsAllocator* allocator,
	const dsSceneNodeType* type, const char* const* itemLists, uint32_t itemListCount,
	dsDestroySceneNodeFunction destroyFunc)
{
	if (!node || !allocator || !type || !destroyFunc)
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
	node->itemLists = itemLists;
	node->treeNodes = NULL;
	node->childCount = 0;
	node->maxChildren = 0;
	node->itemListCount = itemListCount;
	node->treeNodeCount = 0;
	node->maxTreeNodes = 0;
	node->refCount = 1;
	node->userData = NULL;
	node->destroyUserDataFunc = NULL;
	node->destroyFunc = destroyFunc;
	return true;
}

bool dsSceneNode_isOfType(const dsSceneNode* node, const dsSceneNodeType* type)
{
	if (!node || !node->type || !type)
		return false;

	const dsSceneNodeType* nodeType = node->type;
	do
	{
		if (nodeType == type)
			return true;
		nodeType = nodeType->parent;
	} while(nodeType);
	return false;
}

bool dsSceneNode_addChild(dsSceneNode* node, dsSceneNode* child)
{
	if (!node || !child)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < node->childCount; ++i)
	{
		if (node->children[i] == child)
		{
			errno = EPERM;
			return false;
		}
	}

	uint32_t index = node->childCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(node->allocator, node->children, node->childCount,
			node->maxChildren, 1))
	{
		return false;
	}

	node->children[index] = dsSceneNode_addRef(child);
	if (!dsSceneTreeNode_buildSubtree(node, child))
	{
		dsSceneNode_freeRef(child);
		return false;
	}

	return true;
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

	dsSceneTreeNode_removeSubtree(node, node->children[childIndex]);
	dsSceneNode_freeRef(node->children[childIndex]);
	// Order shouldn't matter, so put the last item in this spot for constant-time removal.
	node->children[childIndex] = node->children[node->childCount - 1];
	--node->childCount;
	return true;
}

bool dsSceneNode_removeChildNode(dsSceneNode* node, dsSceneNode* child)
{
	if (!node || !child)
	{
		errno = EINVAL;
		return false;
	}

	dsSceneTreeNode_removeSubtree(node, child);
	bool found = false;
	for (uint32_t i = node->childCount; i-- > 0;)
	{
		if (node->children[i] != child)
			continue;

		dsSceneNode_freeRef(child);
		node->children[i] = node->children[node->childCount - 1];
		--node->childCount;
		found = true;
		break;
	}

	if (!found)
		errno = ENOTFOUND;
	return found;
}

bool dsSceneNode_reparentChildIndex(dsSceneNode* node, uint32_t childIndex, dsSceneNode* newParent)
{
	if (!node || !newParent)
	{
		errno = EINVAL;
		return false;
	}

	if (childIndex >= node->childCount)
	{
		errno = EINDEX;
		return false;
	}

	if (node->treeNodeCount != newParent->treeNodeCount)
	{
		errno = EPERM;
		return false;
	}

	dsSceneNode* child = node->children[childIndex];
	for (uint32_t i = 0; i < newParent->childCount; ++i)
	{
		if (newParent->children[i] == child)
		{
			errno = EPERM;
			return false;
		}
	}

	uint32_t newIndex = newParent->childCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(newParent->allocator, newParent->children, newParent->childCount,
			newParent->maxChildren, 1))
	{
		return false;
	}

	if (!dsSceneTreeNode_reparentSubtree(node, child, newParent))
	{
		--newParent->childCount;
		return false;
	}

	newParent->children[newIndex] = child;
	node->children[childIndex] = node->children[node->childCount - 1];
	--node->childCount;

	// Once fully succeeded, mark the subtrees as dirty.
	for (uint32_t i = 0; i < newParent->treeNodeCount; ++i)
	{
		dsSceneTreeNode* treeNode = newParent->treeNodes[i];
		for (uint32_t j = 0; j < treeNode->childCount; ++j)
		{
			dsSceneTreeNode* childTreeNode = treeNode->children[j];
			if (childTreeNode->node == child)
				dsSceneTreeNode_markDirty(childTreeNode);
		}
	}

	return true;
}

bool dsSceneNode_reparentChildNode(dsSceneNode* node, dsSceneNode* child, dsSceneNode* newParent)
{
	if (!node || !child || !newParent)
	{
		errno = EINVAL;
		return false;
	}

	uint32_t childIndex = 0;
	for (; childIndex < node->childCount; ++childIndex)
	{
		if (node->children[childIndex] == child)
			break;
	}

	if (childIndex >= node->childCount)
	{
		errno = ENOTFOUND;
		return false;
	}

	return dsSceneNode_reparentChildIndex(node, childIndex, newParent);
}

dsSceneTreeNode* dsSceneNode_findUniqueTreeNode(const dsSceneNode* baseNode,
	const dsSceneNode* descendentNode)
{
	if (!baseNode || !descendentNode)
	{
		errno = EINVAL;
		return NULL;
	}

	if (baseNode->treeNodeCount != 1)
	{
		errno = EPERM;
		return NULL;
	}

	if (baseNode == descendentNode)
		return baseNode->treeNodes[0];

	dsSceneTreeNode* foundTreeNode = findUniqueTreeNodeRec(baseNode->treeNodes[0], descendentNode);
	if (foundTreeNode == NULL)
		errno = ENOTFOUND;
	return foundTreeNode;
}

void dsSceneNode_clear(dsSceneNode* node)
{
	if (!node)
		return;

	for (uint32_t i = 0; i < node->childCount; ++i)
	{
		dsSceneNode* child = node->children[i];
		dsSceneTreeNode_removeSubtree(node, child);
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

	if (node->destroyUserDataFunc)
		node->destroyUserDataFunc(node->userData);

	dsSceneNode_clear(node);
	DS_VERIFY(dsAllocator_free(node->allocator, node->children));
	// Root node doesn't dynamically allocate tree nodes.
	if (node->type != &dsRootSceneNodeType)
		DS_VERIFY(dsAllocator_free(node->allocator, node->treeNodes));

	if (node->destroyFunc)
		node->destroyFunc(node);
}

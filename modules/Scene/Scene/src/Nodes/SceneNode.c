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
#include "SceneLoadContextInternal.h"
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

const char* const dsSceneNodeRef_typeName = "ReferenceNode";

size_t dsSceneNode_itemListsAllocSize(const char* const* itemLists, uint32_t itemListCount)
{
	if (itemListCount == 0)
		return 0;

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(const char*)*itemListCount);
	for (uint32_t i = 0; i < itemListCount; ++i)
	{
		if (itemLists[i])
			fullSize += DS_ALIGNED_SIZE(strlen(itemLists[i]));
	}
	return fullSize;
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
	for (uint32_t i = node->childCount; i-- > 0;)
	{
		if (node->children[i] != child)
			continue;

		dsSceneNode_freeRef(child);
		node->children[i] = node->children[node->childCount - 1];
		--node->childCount;
		break;
	}
	return true;
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

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

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
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
	if (!node || !dsAllocator_keepPointer(allocator))
	{
		errno = EINVAL;
		return false;
	}

	node->allocator = allocator;
	node->type = type;
	node->parents = NULL;
	node->children = NULL;
	node->drawLists = drawLists;
	node->treeNodes = NULL;
	node->parentCount = 0;
	node->maxParents = 0;
	node->childCount = 0;
	node->maxChildren = 0;
	node->drawListCount = drawListCount;
	node->treeNodeCount = 0;
	node->maxTreeNodes = 0;
	node->refCount = 1;
	node->destroyFunc = destroyFunc;
	return true;
}

void dsSceneNode_addRef(dsSceneNode* node)
{
	if (!node)
		return;

	DS_ATOMIC_FETCH_ADD32(&node->refCount, 1);
}

void dsSceneNode_freeRef(dsSceneNode* node)
{
	if (!node)
		return;

	if (DS_ATOMIC_FETCH_ADD32(&node->refCount, -1) != 1)
		return;

	for (uint32_t i = 0; i < node->childCount; ++i)
		dsSceneNode_freeRef(node->children[i]);

	if (node->destroyFunc)
		node->destroyFunc(node);
}

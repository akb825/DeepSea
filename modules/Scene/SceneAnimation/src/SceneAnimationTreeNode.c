/*
 * Copyright 2023 Aaron Barany
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

#include <DeepSea/SceneAnimation/SceneAnimationTreeNode.h>

#include "SceneAnimationTreeInstance.h"

#include <DeepSea/Animation/AnimationNodeMapCache.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <DeepSea/SceneAnimation/SceneAnimationList.h>

static void dsSceneAnimationTreeNode_destroy(dsSceneNode* node)
{
	dsSceneAnimationTreeNode* treeNode = (dsSceneAnimationTreeNode*)node;
	DS_VERIFY(dsAnimationNodeMapCache_removeAnimationTree(treeNode->nodeMapCache,
		treeNode->animationTree));
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

const char* const dsSceneAnimationTreeNode_typeName = "AnimationTreeNode";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsSceneAnimationTreeNode_type(void)
{
	return &nodeType;
}

dsSceneAnimationTreeNode* dsSceneAnimationTreeNode_create(dsAllocator* allocator,
	dsAnimationTree* animationTree, dsAnimationNodeMapCache* nodeMapCache)
{
	if (!allocator || !animationTree || !nodeMapCache)
	{
		errno = EINVAL;
		return NULL;
	}

	dsSceneAnimationTreeNode* node = DS_ALLOCATE_OBJECT(allocator, dsSceneAnimationTreeNode);
	if (!node)
		return NULL;

	if (!dsSceneNode_initialize((dsSceneNode*)node, allocator, dsSceneAnimationTreeNode_type(),
			NULL, 0, &dsSceneAnimationTreeNode_destroy))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		return NULL;
	}

	if (!dsAnimationNodeMapCache_addAnimationTree(nodeMapCache, animationTree))
	{
		DS_VERIFY(dsAllocator_free(allocator, node));
		return NULL;
	}

	node->animationTree = animationTree;
	node->nodeMapCache = nodeMapCache;
	return node;
}

dsAnimationTree* dsSceneAnimationTreeNode_getAnimationTreeForInstance(
	const dsSceneTreeNode* treeNode)
{
	dsSceneAnimationTreeInstance* instance = dsSceneAnimationTreeInstance_find(treeNode);
	if (!instance)
		return NULL;

	// Lazily update the instance. This is thread-safe.
	dsSceneAnimationTreeInstance_update(instance);
	return instance->animationTree;
}

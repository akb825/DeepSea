/*
 * Copyright 2023-2025 Aaron Barany
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

#include <DeepSea/SceneAnimation/SceneAnimationNode.h>

#include "SceneAnimationInstance.h"

#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <DeepSea/SceneAnimation/SceneAnimationList.h>

static void dsSceneAnimationNode_destroy(dsSceneNode* node)
{
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

static dsSceneAnimationInstance* getSceneAnimationInstance(const dsSceneTreeNode* treeNode)
{
	while (treeNode && !dsSceneNode_isOfType(treeNode->node, dsSceneAnimationNode_type()))
		treeNode = treeNode->parent;
	if (!treeNode)
		return NULL;

	const dsSceneNodeItemData* itemData = &treeNode->itemData;
	DS_ASSERT(itemData->count == treeNode->node->itemListCount);
	for (uint32_t i = 0; i < itemData->count; ++i)
	{
		const dsSceneItemList* itemList = treeNode->itemLists[i].list;
		if (itemList && itemList->type == dsSceneAnimationList_type())
			return (dsSceneAnimationInstance*)itemData->itemData[i].data;
	}

	return NULL;
}

const char* const dsSceneAnimationNode_typeName = "AnimationNode";

static dsSceneNodeType nodeType =
{
	.destroyFunc = dsSceneAnimationNode_destroy
};

const dsSceneNodeType* dsSceneAnimationNode_type(void)
{
	return &nodeType;
}

dsSceneAnimationNode* dsSceneAnimationNode_create(dsAllocator* allocator,
	dsAnimationNodeMapCache* nodeMapCache, const char* const* itemLists, uint32_t itemListCount)
{
	if (!allocator || !nodeMapCache || (!itemLists && itemListCount > 0))
	{
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneAnimationNode)) +
		dsSceneNode_itemListsAllocSize(itemLists, itemListCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneAnimationNode* node = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneAnimationNode);
	DS_ASSERT(node);

	const char* const* itemListsCopy = dsSceneNode_copyItemLists((dsAllocator*)&bufferAlloc,
		itemLists, itemListCount);
	DS_ASSERT(itemListCount == 0 || itemListsCopy);

	if (!dsSceneNode_initialize((dsSceneNode*)node, allocator, dsSceneAnimationNode_type(),
			itemListsCopy, itemListCount))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		return NULL;
	}

	node->nodeMapCache = nodeMapCache;
	return node;
}

dsAnimation* dsSceneAnimationNode_getAnimationForInstance(const dsSceneTreeNode* treeNode)
{
	dsSceneAnimationInstance* animationInstance = getSceneAnimationInstance(treeNode);
	return animationInstance ? animationInstance->animation : NULL;
}

float dsSceneAnimationNode_getSkeletonRagdollWeight(const dsSceneTreeNode* treeNode)
{
	dsSceneAnimationInstance* animationInstance = getSceneAnimationInstance(treeNode);
	return animationInstance ? animationInstance->skeletonRagdoll.weight : 0.0f;
}

bool dsSceneAnimationNode_setSkeletonRagdollWeight(const dsSceneTreeNode* treeNode, float weight)
{
	dsSceneAnimationInstance* animationInstance = getSceneAnimationInstance(treeNode);
	if (!animationInstance)
		return false;

	return dsSceneAnimationInstance_setSkeletonRagdollWeight(animationInstance, weight);
}

float dsSceneAnimationNode_getAdditionRagdollWeight(const dsSceneTreeNode* treeNode)
{
	dsSceneAnimationInstance* animationInstance = getSceneAnimationInstance(treeNode);
	return animationInstance ? animationInstance->additionRagdoll.weight : 0.0f;
}

bool dsSceneAnimationNode_setAdditionRagdollWeight(const dsSceneTreeNode* treeNode, float weight)
{
	dsSceneAnimationInstance* animationInstance = getSceneAnimationInstance(treeNode);
	if (!animationInstance)
		return false;

	return dsSceneAnimationInstance_setAdditionRagdollWeight(animationInstance, weight);
}

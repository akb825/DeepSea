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

#include <DeepSea/SceneAnimation/SceneAnimationNode.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <DeepSea/SceneAnimation/SceneAnimationList.h>

static void dsSceneAnimationNode_destroy(dsSceneNode* node)
{
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

const char* const dsSceneAnimationNode_typeName = "AnimationNode";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsSceneAnimationNode_type(void)
{
	return &nodeType;
}

dsSceneAnimationNode* dsSceneAnimationNode_create(dsAllocator* allocator,
	dsSceneAnimationTree* animationTree)
{
	if (!allocator || !animationTree)
	{
		errno = EINVAL;
		return NULL;
	}

	dsSceneAnimationNode* node = DS_ALLOCATE_OBJECT(allocator, dsSceneAnimationNode);
	if (!node)
		return NULL;

	if (!dsSceneNode_initialize((dsSceneNode*)node, allocator, dsSceneAnimationNode_type(), NULL, 0,
			&dsSceneAnimationNode_destroy))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		return NULL;
	}

	node->animationTree = animationTree;
	return node;
}

dsSceneAnimation* dsSceneAnimationNode_getAnimationForInstance(const dsSceneTreeNode* treeNode)
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
			return (dsSceneAnimation*)itemData->itemData[i].data;
	}

	return NULL;
}

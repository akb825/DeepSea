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

#include "SceneAnimationInternal.h"

#include <DeepSea/Animation/Animation.h>
#include <DeepSea/Animation/AnimationTree.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/SceneAnimation/SceneAnimationList.h>
#include <DeepSea/SceneAnimation/SceneAnimationTreeNode.h>

dsSceneAnimationTreeInstance* dsSceneAnimationTreeInstance_create(dsAllocator* allocator,
	const dsAnimation* animation, dsAnimationTree* animationTree)
{
	DS_ASSERT(allocator);
	DS_ASSERT(animation);
	DS_ASSERT(animationTree);

	dsSceneAnimationTreeInstance* instance =
		DS_ALLOCATE_OBJECT(allocator, dsSceneAnimationTreeInstance);
	if (!instance)
		return NULL;

	instance->allocator = dsAllocator_keepPointer(allocator);
	instance->animation = animation;
	instance->animationTree = dsAnimationTree_clone(allocator, animationTree);
	if (!instance->animationTree)
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, instance));
		return NULL;
	}

	instance->dirty = true;
	DS_VERIFY(dsSpinlock_initialize(&instance->lock));
	return instance;
}

dsSceneAnimationTreeInstance* dsSceneAnimationTreeInstance_find(const dsSceneTreeNode* treeNode)
{
	while (treeNode && !dsSceneNode_isOfType(treeNode->node, dsSceneAnimationTreeNode_type()))
		treeNode = treeNode->parent;
	if (!treeNode)
		return NULL;

	const dsSceneNodeItemData* itemData = &treeNode->itemData;
	DS_ASSERT(itemData->count == treeNode->node->itemListCount);
	for (uint32_t i = 0; i < itemData->count; ++i)
	{
		const dsSceneItemList* itemList = treeNode->itemLists[i].list;
		if (itemList && itemList->type == dsSceneAnimationList_type())
			return (dsSceneAnimationTreeInstance*)itemData->itemData[i].data;
	}

	return NULL;
}

void dsSceneAnimationTreeInstance_updateUnlocked(dsSceneAnimationTreeInstance* instance)
{
	DS_ASSERT(instance);

	if (!instance->dirty)
		return;

	DS_CHECK(DS_SCENE_ANIMATION_LOG_TAG,
		dsAnimation_apply(instance->animation, instance->animationTree));
	instance->dirty = false;
}

void dsSceneAnimationTreeInstance_update(dsSceneAnimationTreeInstance* instance)
{
	DS_ASSERT(instance);

	DS_VERIFY(dsSpinlock_lock(&instance->lock));
	dsSceneAnimationTreeInstance_updateUnlocked(instance);
	DS_VERIFY(dsSpinlock_unlock(&instance->lock));
}

void dsSceneAnimationTreeInstance_destroy(dsSceneAnimationTreeInstance* instance)
{
	DS_ASSERT(instance);
	dsAnimationTree_destroy(instance->animationTree);
	dsSpinlock_shutdown(&instance->lock);
	if (instance->allocator)
		DS_VERIFY(dsAllocator_free(instance->allocator, instance));
}

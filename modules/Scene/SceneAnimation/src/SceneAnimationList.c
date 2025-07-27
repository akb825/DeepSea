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

#include <DeepSea/SceneAnimation/SceneAnimationList.h>

#include "SceneAnimationInstance.h"
#include "SceneAnimationTreeInstance.h"

#include <DeepSea/Animation/Animation.h>
#include <DeepSea/Animation/AnimationTree.h>
#include <DeepSea/Animation/DirectAnimation.h>

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Scene/ItemLists/SceneItemListEntries.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneTreeNode.h>

#include <DeepSea/SceneAnimation/SceneAnimationNode.h>
#include <DeepSea/SceneAnimation/SceneAnimationRagdollNode.h>
#include <DeepSea/SceneAnimation/SceneAnimationTransformNode.h>
#include <DeepSea/SceneAnimation/SceneAnimationTreeNode.h>

#include <limits.h>
#include <string.h>

#define MIN_TREE_ENTRY_ID (ULLONG_MAX/4)
#define MIN_TRANSFORM_ENTRY_ID (MIN_TREE_ENTRY_ID*2)

typedef struct AnimationEntry
{
	dsSceneAnimationInstance* instance;
	uint64_t nodeID;
} AnimationEntry;

typedef struct TreeEntry
{
	dsSceneAnimationTreeInstance* instance;
	uint64_t nodeID;
} TreeEntry;

typedef struct TransformEntry
{
	dsSceneTreeNode* treeNode;
	dsSceneAnimationTreeInstance* instance;
	dsMatrix44f prevTransform;
	uint64_t nodeID;
} TransformEntry;

typedef struct dsSceneAnimationList
{
	dsSceneItemList itemList;

	AnimationEntry* animationEntries;
	uint32_t animationEntryCount;
	uint32_t maxAnimationEntries;
	uint64_t nextAnimationNodeID;

	uint64_t* removeAnimationEntries;
	uint32_t removeAnimationEntryCount;
	uint32_t maxRemoveAnimationEntries;

	TreeEntry* treeEntries;
	uint32_t treeEntryCount;
	uint32_t maxTreeEntries;
	uint64_t nextTreeNodeID;

	uint64_t* removeTreeEntries;
	uint32_t removeTreeEntryCount;
	uint32_t maxRemoveTreeEntries;

	TransformEntry* transformEntries;
	uint32_t transformEntryCount;
	uint32_t maxTransformEntries;
	uint64_t nextTransformNodeID;

	uint64_t* removeTransformEntries;
	uint32_t removeTransformEntryCount;
	uint32_t maxRemoveTransformEntries;
} dsSceneAnimationList;

static uint64_t dsSceneAnimationList_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_ASSERT(itemList);
	DS_UNUSED(itemData);
	DS_UNUSED(treeNode);
	dsSceneAnimationList* animationList = (dsSceneAnimationList*)itemList;
	const dsSceneNodeType* animationNodeType = dsSceneAnimationNode_type();
	if (dsSceneNode_isOfType(node, animationNodeType))
	{
		const dsSceneAnimationNode* animationNode = (const dsSceneAnimationNode*)node;
		uint32_t index = animationList->animationEntryCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, animationList->animationEntries,
				animationList->animationEntryCount, animationList->maxAnimationEntries, 1))
		{
			return DS_NO_SCENE_NODE;
		}

		AnimationEntry* entry = animationList->animationEntries + index;
		entry->instance =
			dsSceneAnimationInstance_create(node->allocator, animationNode->nodeMapCache);
		if (!entry->instance)
		{
			--animationList->animationEntryCount;
			return DS_NO_SCENE_NODE;
		}
		*thisItemData = entry->instance;

		entry->nodeID = animationList->nextAnimationNodeID++;
		return entry->nodeID;
	}
	if (dsSceneNode_isOfType(node, dsSceneAnimationTreeNode_type()))
	{
		const dsSceneAnimationTreeNode* animationTreeNode = (const dsSceneAnimationTreeNode*)node;
		const dsAnimation* animation = dsSceneAnimationNode_getAnimationForInstance(treeNode);
		if (!animation)
		{
			DS_LOG_ERROR(DS_SCENE_ANIMATION_LOG_TAG,
				"Couldn't find animation for animation tree node.");
			return DS_NO_SCENE_NODE;
		}

		if (animation->nodeMapCache != animationTreeNode->nodeMapCache)
		{
			DS_LOG_ERROR(DS_SCENE_ANIMATION_LOG_TAG,
				"Animation and animation tree use different node map caches.");
			return DS_NO_SCENE_NODE;
		}

		uint32_t index = animationList->treeEntryCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, animationList->treeEntries,
				animationList->treeEntryCount, animationList->maxTreeEntries, 1))
		{
			return DS_NO_SCENE_NODE;
		}

		TreeEntry* entry = animationList->treeEntries + index;
		entry->instance = dsSceneAnimationTreeInstance_create(itemList->allocator, animation,
			animationTreeNode->animationTree);
		if (!entry->instance)
		{
			--animationList->treeEntryCount;
			return DS_NO_SCENE_NODE;
		}
		*thisItemData = entry->instance;

		entry->nodeID = animationList->nextTreeNodeID++;
		return entry->nodeID;
	}
	if (dsSceneNode_isOfType(node, dsSceneAnimationTransformNode_type()))
	{
		const dsSceneAnimationTransformNode* animationTransformNode =
			(const dsSceneAnimationTransformNode*)node;
		dsSceneAnimationTreeInstance* instance = dsSceneAnimationTreeInstance_find(treeNode);
		if (!instance)
		{
			DS_LOG_ERROR_F(DS_SCENE_ANIMATION_LOG_TAG,
				"Couldn't find animation tree for animation transform '%s'.",
				animationTransformNode->animationNodeName);
			return DS_NO_SCENE_NODE;
		}

		uint32_t nodeIndex = dsAnimationTree_findNodeIndexID(instance->animationTree,
			animationTransformNode->animationNodeID);
		if (nodeIndex == DS_NO_ANIMATION_NODE)
		{
			DS_LOG_ERROR_F(DS_SCENE_ANIMATION_LOG_TAG,
				"Couldn't find animation node for animation transform '%s'.",
				animationTransformNode->animationNodeName);
			return DS_NO_SCENE_NODE;
		}

		uint32_t index = animationList->transformEntryCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, animationList->transformEntries,
				animationList->transformEntryCount, animationList->maxTransformEntries, 1))
		{
			return DS_NO_SCENE_NODE;
		}

		treeNode->baseTransform = &instance->animationTree->nodes[nodeIndex].transform;

		TransformEntry* entry = animationList->transformEntries + index;
		entry->treeNode = treeNode;
		entry->instance = instance;
		dsMatrix44_identity(entry->prevTransform);
		entry->nodeID = animationList->nextTransformNodeID++;
		return entry->nodeID;
	}
	if (dsSceneNode_isOfType(node, dsSceneAnimationRagdollNode_type()))
	{
		const dsSceneAnimationRagdollNode* ragdollNode = (const dsSceneAnimationRagdollNode*)node;
		const dsSceneTreeNode* animationNode = treeNode->parent;
		while (animationNode && !dsSceneNode_isOfType(animationNode->node, animationNodeType))
			animationNode = animationNode->parent;

		uint64_t nodeID = animationNode ? dsSceneTreeNode_getNodeID(animationNode, itemList) :
			DS_NO_SCENE_NODE;
		if (nodeID == DS_NO_SCENE_NODE)
		{
			DS_LOG_ERROR_F(DS_SCENE_ANIMATION_LOG_TAG,
				"Couldn't find animation node for animation ragdoll node '%s'.",
				ragdollNode->animationNodeName);
			return DS_NO_SCENE_NODE;
		}

		AnimationEntry* entry = (AnimationEntry*)dsSceneItemListEntries_findEntry(
			animationList->animationEntries, animationList->animationEntryCount,
			sizeof(AnimationEntry), offsetof(AnimationEntry, nodeID), nodeID);
		DS_ASSERT(entry);
		switch (ragdollNode->ragdollType)
		{
			case dsSceneAnimationRagdollType_Skeleton:
				dsSceneAnimationInstance_addSkeletonRagdollNode(
					entry->instance, ragdollNode, treeNode);
				break;
			case dsSceneAnimationRagdollType_Addition:
				dsSceneAnimationInstance_addAdditionRagdollNode(
					entry->instance, ragdollNode, treeNode);
				break;
		}
		return nodeID;
	}
	return DS_NO_SCENE_NODE;
}

static void dsSceneAnimationList_removeNode(
	dsSceneItemList* itemList, dsSceneTreeNode* treeNode, uint64_t nodeID)
{
	DS_ASSERT(itemList);
	DS_UNUSED(treeNode);
	dsSceneAnimationList* animationList = (dsSceneAnimationList*)itemList;
	if (nodeID < MIN_TREE_ENTRY_ID)
	{
		AnimationEntry* entry = (AnimationEntry*)dsSceneItemListEntries_findEntry(
			animationList->animationEntries, animationList->animationEntryCount,
			sizeof(AnimationEntry), offsetof(AnimationEntry, nodeID), nodeID);
		if (!entry)
			return;

		if (dsSceneNode_isOfType(treeNode->node, dsSceneAnimationNode_type()))
		{
			dsSceneAnimationInstance_destroy(entry->instance);

			uint32_t index = animationList->removeAnimationEntryCount;
			if (DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, animationList->removeAnimationEntries,
					animationList->removeAnimationEntryCount,
					animationList->maxRemoveAnimationEntries, 1))
			{
				animationList->removeAnimationEntries[index] = nodeID;
			}
			else
			{
				dsSceneItemListEntries_removeSingleIndex(animationList->animationEntries,
					&animationList->animationEntryCount, sizeof(AnimationEntry),
					entry - animationList->animationEntries);
			}
		}
		else
		{
			DS_ASSERT(dsSceneNode_isOfType(treeNode->node, dsSceneAnimationRagdollNode_type()));
			const dsSceneAnimationRagdollNode* ragdollNode =
				(const dsSceneAnimationRagdollNode*)treeNode->node;
			switch (ragdollNode->ragdollType)
			{
				case dsSceneAnimationRagdollType_Skeleton:
					dsSceneAnimationInstance_removeSkeletonRagdollNode(entry->instance, treeNode);
					break;
				case dsSceneAnimationRagdollType_Addition:
					dsSceneAnimationInstance_removeAdditionRagdollNode(entry->instance, treeNode);
					break;
			}
		}
	}
	else if (nodeID < MIN_TRANSFORM_ENTRY_ID)
	{
		TreeEntry* entry = (TreeEntry*)dsSceneItemListEntries_findEntry(animationList->treeEntries,
			animationList->treeEntryCount, sizeof(TreeEntry), offsetof(TreeEntry, nodeID), nodeID);
		if (!entry)
			return;

		dsSceneAnimationTreeInstance_destroy(entry->instance);

		uint32_t index = animationList->removeTreeEntryCount;
		if (DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, animationList->removeTreeEntries,
				animationList->removeTreeEntryCount, animationList->maxRemoveTreeEntries, 1))
		{
			animationList->removeTreeEntries[index] = nodeID;
		}
		else
		{
			dsSceneItemListEntries_removeSingleIndex(animationList->treeEntries,
				&animationList->treeEntryCount, sizeof(TreeEntry),
				entry - animationList->treeEntries);
		}
	}
	else
	{
		uint32_t index = animationList->removeTransformEntryCount;
		if (DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, animationList->removeTransformEntries,
				animationList->removeTransformEntryCount, animationList->maxRemoveTransformEntries,
				1))
		{
			animationList->removeTransformEntries[index] = nodeID;
		}
		else
		{
			dsSceneItemListEntries_removeSingle(animationList->transformEntries,
				&animationList->transformEntryCount, sizeof(TransformEntry),
				offsetof(TransformEntry, nodeID), nodeID);
		}
	}
}

static void dsSceneAnimationList_preTransformUpdate(
	dsSceneItemList* itemList, const dsScene* scene, float time)
{
	DS_ASSERT(itemList);
	DS_UNUSED(scene);
	dsSceneAnimationList* animationList = (dsSceneAnimationList*)itemList;

	// Lazily remove entries.
	dsSceneItemListEntries_removeMulti(animationList->animationEntries,
		&animationList->animationEntryCount, sizeof(AnimationEntry),
		offsetof(AnimationEntry, nodeID), animationList->removeAnimationEntries,
		animationList->removeAnimationEntryCount);
	animationList->removeAnimationEntryCount = 0;

	dsSceneItemListEntries_removeMulti(animationList->treeEntries, &animationList->treeEntryCount,
		sizeof(TreeEntry), offsetof(TreeEntry, nodeID), animationList->removeTreeEntries,
		animationList->removeTreeEntryCount);
	animationList->removeTreeEntryCount = 0;

	dsSceneItemListEntries_removeMulti(animationList->transformEntries,
		&animationList->transformEntryCount, sizeof(TransformEntry),
		offsetof(TransformEntry, nodeID), animationList->removeTransformEntries,
		animationList->removeTransformEntryCount);
	animationList->removeTransformEntryCount = 0;

	if (time != 0)
	{
		for (uint32_t i = 0; i < animationList->animationEntryCount; ++i)
		{
			AnimationEntry* entry = animationList->animationEntries + i;
			DS_CHECK(DS_SCENE_ANIMATION_LOG_TAG,
				dsAnimation_update(entry->instance->animation, time));
		}
	}

	for (uint32_t i = 0; i < animationList->treeEntryCount; ++i)
	{
		TreeEntry* entry = animationList->treeEntries + i;
		// Mark as dirty to lazily update the trees.
		entry->instance->dirty = true;
	}

	for (uint32_t i = 0; i < animationList->transformEntryCount; ++i)
	{
		TransformEntry* entry = animationList->transformEntries + i;
		dsSceneAnimationTreeInstance_updateUnlocked(entry->instance);
		dsSceneTreeNode* treeNode = entry->treeNode;
		if (memcmp(treeNode->baseTransform, &entry->prevTransform, sizeof(dsMatrix44f)) != 0)
		{
			dsSceneTreeNode_markDirty(treeNode);
			entry->prevTransform = *treeNode->baseTransform;
		}
	}
}

static void dsSceneAnimationList_destroy(dsSceneItemList* itemList)
{
	DS_ASSERT(itemList);
	dsSceneAnimationList* animationList = (dsSceneAnimationList*)itemList;

	// Handle removed entries before destroying their resources.
	dsSceneItemListEntries_removeMulti(animationList->animationEntries,
		&animationList->animationEntryCount, sizeof(AnimationEntry),
		offsetof(AnimationEntry, nodeID), animationList->removeAnimationEntries,
		animationList->removeAnimationEntryCount);
	dsSceneItemListEntries_removeMulti(animationList->treeEntries, &animationList->treeEntryCount,
		sizeof(TreeEntry), offsetof(TreeEntry, nodeID), animationList->removeTreeEntries,
		animationList->removeTreeEntryCount);

	for (uint32_t i = 0; i < animationList->animationEntryCount; ++i)
		dsSceneAnimationInstance_destroy(animationList->animationEntries[i].instance);
	for (uint32_t i = 0; i < animationList->treeEntryCount; ++i)
		dsSceneAnimationTreeInstance_destroy(animationList->treeEntries[i].instance);

	DS_VERIFY(dsAllocator_free(itemList->allocator, animationList->animationEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, animationList->removeAnimationEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, animationList->treeEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, animationList->removeTreeEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, animationList->transformEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, animationList->removeTransformEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

dsSceneItemList* dsSceneAnimationList_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const char* name, const uint8_t* data, size_t dataSize)
{
	DS_UNUSED(loadContext);
	DS_UNUSED(scratchData);
	DS_UNUSED(resourceAllocator);
	DS_UNUSED(userData);
	DS_UNUSED(data);
	DS_UNUSED(dataSize);
	return (dsSceneItemList*)dsSceneAnimationList_create(allocator, name);
}

const char* const dsSceneAnimationList_typeName = "AnimationList";

static dsSceneItemListType itemListType =
{
	.addNodeFunc = &dsSceneAnimationList_addNode,
	.removeNodeFunc = &dsSceneAnimationList_removeNode,
	.preTransformUpdateFunc = &dsSceneAnimationList_preTransformUpdate,
	.destroyFunc = &dsSceneAnimationList_destroy
};

const dsSceneItemListType* dsSceneAnimationList_type(void)
{
	return &itemListType;
}

dsSceneAnimationList* dsSceneAnimationList_create(dsAllocator* allocator, const char* name)
{
	if (!allocator || !name)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_ANIMATION_LOG_TAG,
			"Scene animation list allocator must support freeing memory.");
		return NULL;
	}

	size_t nameLen = strlen(name) + 1;
	size_t fullSize =
		DS_ALIGNED_SIZE(sizeof(dsSceneAnimationList)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsSceneAnimationList* animationList =
		DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneAnimationList);
	DS_ASSERT(animationList);

	dsSceneItemList* itemList = (dsSceneItemList*)animationList;
	itemList->allocator = allocator;
	itemList->type = dsSceneAnimationList_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsUniqueNameID_create(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = false;
	itemList->skipPreRenderPass = false;

	animationList->animationEntries = NULL;
	animationList->animationEntryCount = 0;
	animationList->maxAnimationEntries = 0;
	animationList->nextAnimationNodeID = 0;

	animationList->treeEntries = NULL;
	animationList->treeEntryCount = 0;
	animationList->maxTreeEntries = 0;
	animationList->nextTreeNodeID = MIN_TREE_ENTRY_ID;

	animationList->transformEntries = NULL;
	animationList->transformEntryCount = 0;
	animationList->maxTransformEntries = 0;
	animationList->nextTransformNodeID = MIN_TRANSFORM_ENTRY_ID;

	return animationList;
}

bool dsSceneAnimationList_updateRagdolls(dsSceneAnimationList* animationList)
{
	if (!animationList)
	{
		errno = EINVAL;
		return false;
	}

	// Lazily remove entries.
	dsSceneItemListEntries_removeMulti(animationList->animationEntries,
		&animationList->animationEntryCount, sizeof(AnimationEntry),
		offsetof(AnimationEntry, nodeID), animationList->removeAnimationEntries,
		animationList->removeAnimationEntryCount);
	animationList->removeAnimationEntryCount = 0;

	for (uint32_t i = 0; i < animationList->animationEntryCount; ++i)
	{
		AnimationEntry* entry = animationList->animationEntries + i;
		dsSceneAnimationInstance_updateRagdolls(entry->instance);
	}
	return true;
}

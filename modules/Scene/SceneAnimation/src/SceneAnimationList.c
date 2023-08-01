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

#include <DeepSea/SceneAnimation/SceneAnimationList.h>

#include "SceneAnimationTreeInstance.h"

#include <DeepSea/Animation/Animation.h>
#include <DeepSea/Animation/AnimationTree.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <DeepSea/SceneAnimation/SceneAnimationNode.h>
#include <DeepSea/SceneAnimation/SceneAnimationTransformNode.h>
#include <DeepSea/SceneAnimation/SceneAnimationTreeNode.h>

#include <limits.h>
#include <string.h>

#define MIN_TREE_ENTRY_ID (ULLONG_MAX/3)
#define MIN_TRANSFORM_ENTRY_ID (MIN_TREE_ENTRY_ID*2)

typedef struct AnimationEntry
{
	dsAnimation* animation;
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

	TreeEntry* treeEntries;
	uint32_t treeEntryCount;
	uint32_t maxTreeEntries;
	uint64_t nextTreeNodeID;

	TransformEntry* transformEntries;
	uint32_t transformEntryCount;
	uint32_t maxTransformEntries;
	uint64_t nextTransformNodeID;
} dsSceneAnimationList;

static uint64_t dsSceneAnimationList_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(itemData);
	DS_UNUSED(treeNode);
	dsSceneAnimationList* animationList = (dsSceneAnimationList*)itemList;
	if (dsSceneNode_isOfType(node, dsSceneAnimationNode_type()))
	{
		const dsSceneAnimationNode* animationNode = (const dsSceneAnimationNode*)node;
		uint32_t index = animationList->animationEntryCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, animationList->animationEntries,
				animationList->animationEntryCount, animationList->maxAnimationEntries, 1))
		{
			return DS_NO_SCENE_NODE;
		}

		AnimationEntry* entry = animationList->animationEntries + index;;
		entry->animation = dsAnimation_create(node->allocator, animationNode->nodeMapCache);
		if (!DS_CHECK_MESSAGE(DS_SCENE_ANIMATION_LOG_TAG, entry->animation != NULL,
				"dsSceneAnimation_create(node->allocator, animationNode->nodeMapCache)"))
		{
			--animationList->animationEntryCount;
			return DS_NO_SCENE_NODE;
		}
		*thisItemData = entry->animation;

		entry->nodeID = animationList->nextAnimationNodeID++;
		return entry->nodeID;
	}
	else if (dsSceneNode_isOfType(node, dsSceneAnimationTreeNode_type()))
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
	else if (dsSceneNode_isOfType(node, dsSceneAnimationTransformNode_type()))
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
	else
		return DS_NO_SCENE_NODE;
}

static void dsSceneAnimationList_removeNode(dsSceneItemList* itemList, uint64_t nodeID)
{
	dsSceneAnimationList* animationList = (dsSceneAnimationList*)itemList;
	if (nodeID < MIN_TREE_ENTRY_ID)
	{
		for (uint32_t i = 0; i < animationList->animationEntryCount; ++i)
		{
			AnimationEntry* entry = animationList->animationEntries + i;
			if (entry->nodeID != nodeID)
				continue;

			dsAnimation_destroy(entry->animation);

			// Order shouldn't matter, so use constant-time removal.
			animationList->animationEntries[i] =
				animationList->animationEntries[animationList->animationEntryCount - 1];
			--animationList->animationEntryCount;
			break;
		}
	}
	else if (nodeID < MIN_TRANSFORM_ENTRY_ID)
	{
		for (uint32_t i = 0; i < animationList->treeEntryCount; ++i)
		{
			TreeEntry* entry = animationList->treeEntries + i;
			if (entry->nodeID != nodeID)
				continue;

			dsSceneAnimationTreeInstance_destroy(entry->instance);

			// Order shouldn't matter, so use constant-time removal.
			animationList->treeEntries[i] =
				animationList->treeEntries[animationList->treeEntryCount - 1];
			--animationList->treeEntryCount;
			break;
		}
	}
	else
	{
		for (uint32_t i = 0; i < animationList->transformEntryCount; ++i)
		{
			TransformEntry* entry = animationList->transformEntries + i;
			if (entry->nodeID != nodeID)
				continue;

			// Order shouldn't matter, so use constant-time removal.
			animationList->transformEntries[i] =
				animationList->transformEntries[animationList->transformEntryCount - 1];
			--animationList->transformEntryCount;
			break;
		}
	}
}

static void dsSceneAnimationList_preTransformUpdate(dsSceneItemList* itemList, const dsScene* scene,
	float time)
{
	DS_UNUSED(scene);
	DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);

	dsSceneAnimationList* animationList = (dsSceneAnimationList*)itemList;

	if (time != 0)
	{
		for (uint32_t i = 0; i < animationList->animationEntryCount; ++i)
		{
			AnimationEntry* entry = animationList->animationEntries + i;
			DS_CHECK(DS_SCENE_ANIMATION_LOG_TAG, dsAnimation_update(entry->animation, time));
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

	DS_PROFILE_SCOPE_END();
}

static void dsSceneAnimationList_destroy(dsSceneItemList* itemList)
{
	dsSceneAnimationList* animationList = (dsSceneAnimationList*)itemList;
	for (uint32_t i = 0; i < animationList->animationEntryCount; ++i)
		dsAnimation_destroy(animationList->animationEntries[i].animation);
	for (uint32_t i = 0; i < animationList->treeEntryCount; ++i)
		dsSceneAnimationTreeInstance_destroy(animationList->treeEntries[i].instance);
	DS_VERIFY(dsAllocator_free(itemList->allocator, animationList->animationEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, animationList->treeEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, animationList->transformEntries));
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
	return dsSceneAnimationList_create(allocator, name);
}

const char* const dsSceneAnimationList_typeName = "AnimationList";

dsSceneItemListType dsSceneAnimationList_type(void)
{
	static int type;
	return &type;
}

dsSceneItemList* dsSceneAnimationList_create(dsAllocator* allocator, const char* name)
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
	itemList->nameID = dsHashString(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = true;
	itemList->addNodeFunc = &dsSceneAnimationList_addNode;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = &dsSceneAnimationList_removeNode;
	itemList->preTransformUpdateFunc = &dsSceneAnimationList_preTransformUpdate;
	itemList->updateFunc = NULL;
	itemList->commitFunc = NULL;
	itemList->preRenderPassFunc = NULL;
	itemList->destroyFunc = &dsSceneAnimationList_destroy;

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

	return itemList;
}

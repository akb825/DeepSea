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

#include <DeepSea/Animation/Animation.h>
#include <DeepSea/Animation/AnimationTree.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <DeepSea/SceneAnimation/SceneAnimation.h>
#include <DeepSea/SceneAnimation/SceneAnimationNode.h>
#include <DeepSea/SceneAnimation/SceneAnimationTransformNode.h>
#include <DeepSea/SceneAnimation/SceneAnimationTree.h>

#include <limits.h>
#include <string.h>

#define MIN_TRANSFORM_ENTRY_ID LLONG_MAX

typedef struct AnimationEntry
{
	const dsSceneAnimationNode* node;
	dsSceneAnimation* animation;
	uint64_t nodeID;
} AnimationEntry;

typedef struct TransformEntry
{
	const dsSceneAnimationTransformNode* node;
	dsSceneTreeNode* treeNode;
	uint32_t animationNodeNameID;
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

	TransformEntry* transformEntries;
	uint32_t transformEntryCount;
	uint32_t maxTransformEntries;
	uint64_t nextTransformNodeID;
} dsSceneAnimationList;

static uint64_t dsSceneAnimationList_addNode(dsSceneItemList* itemList, const dsSceneNode* node,
	const dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(itemData);
	DS_UNUSED(treeNode);
	dsSceneAnimationList* animationList = (dsSceneAnimationList*)itemList;
	if (dsSceneNode_isOfType(node, dsSceneAnimationNode_type()))
	{
		uint32_t index = animationList->animationEntryCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, animationList->animationEntries,
				animationList->animationEntryCount, animationList->maxAnimationEntries, 1))
		{
			return DS_NO_SCENE_NODE;
		}

		AnimationEntry* entry = animationList->animationEntries + index;
		entry->node = (const dsSceneAnimationNode*)node;
		entry->animation = dsSceneAnimation_create(node->allocator, entry->node->animationTree);
		if (!DS_CHECK_MESSAGE(DS_SCENE_ANIMATION_LOG_TAG, entry->animation != NULL,
				"dsSceneAnimation_create(node->allocator, entry->node->animationTree)"))
		{
			--animationList->animationEntryCount;
			return DS_NO_SCENE_NODE;
		}
		*thisItemData = entry->animation;

		entry->nodeID = animationList->nextAnimationNodeID++;
		return entry->nodeID;
	}
	else if (dsSceneNode_isOfType(node, dsSceneAnimationTransformNode_type()))
	{
		uint32_t index = animationList->transformEntryCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, animationList->transformEntries,
				animationList->transformEntryCount, animationList->maxTransformEntries, 1))
		{
			return DS_NO_SCENE_NODE;
		}

		TransformEntry* entry = animationList->transformEntries + index;
		entry->node = (const dsSceneAnimationTransformNode*)node;
		entry->treeNode = (dsSceneTreeNode*)treeNode;
		entry->animationNodeNameID = dsHashString(entry->node->animationNodeName);
		dsMatrix44_identity(entry->prevTransform);
		entry->nodeID = animationList->nextAnimationNodeID++;
		return entry->nodeID;
	}
	else
		return DS_NO_SCENE_NODE;
}

static void dsSceneAnimationList_removeNode(dsSceneItemList* itemList, uint64_t nodeID)
{
	dsSceneAnimationList* animationList = (dsSceneAnimationList*)itemList;
	if (nodeID < MIN_TRANSFORM_ENTRY_ID)
	{
		for (uint32_t i = 0; i < animationList->animationEntryCount; ++i)
		{
			AnimationEntry* entry = animationList->animationEntries + i;
			if (entry->nodeID != nodeID)
				continue;

			dsSceneAnimation_destroy(entry->animation);

			// Order shouldn't matter, so use constant-time removal.
			animationList->animationEntries[i] =
				animationList->animationEntries[animationList->animationEntryCount - 1];
			--animationList->animationEntryCount;
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
			animationList->animationEntries[i] =
				animationList->animationEntries[animationList->animationEntryCount - 1];
			--animationList->animationEntryCount;
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
	for (uint32_t i = 0; i < animationList->animationEntryCount; ++i)
	{
		AnimationEntry* entry = animationList->animationEntries + i;
		dsAnimation* animation = entry->animation->animation;
		dsAnimationTree* tree = entry->animation->animationTree;
		DS_CHECK(DS_SCENE_ANIMATION_LOG_TAG, dsAnimation_update(animation, time));
		DS_CHECK(DS_SCENE_ANIMATION_LOG_TAG, dsAnimation_apply(animation, tree));
	}

	for (uint32_t i = 0; i < animationList->animationEntryCount; ++i)
	{
		TransformEntry* entry = animationList->transformEntries + i;
		dsSceneTreeNode* treeNode = entry->treeNode;
		if (!treeNode->baseTransform)
		{
			dsSceneAnimation* animation = dsSceneAnimationNode_getAnimationForInstance(treeNode);
			if (!animation)
				continue;

			uint32_t nodeIndex = dsAnimationTree_findNodeIndexID(animation->animationTree,
				entry->animationNodeNameID);
			if (nodeIndex == DS_NO_ANIMATION_NODE)
				continue;

			treeNode->baseTransform = &animation->animationTree->nodes[nodeIndex].transform;
		}

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
		dsSceneAnimation_destroy(animationList->animationEntries[i].animation);
	DS_VERIFY(dsAllocator_free(itemList->allocator, animationList->animationEntries));
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
	itemList->destroyFunc = &dsSceneAnimationList_destroy;

	animationList->animationEntries = NULL;
	animationList->animationEntryCount = 0;
	animationList->maxAnimationEntries = 0;
	animationList->nextAnimationNodeID = 0;

	animationList->transformEntries = NULL;
	animationList->transformEntryCount = 0;
	animationList->maxTransformEntries = 0;
	animationList->nextTransformNodeID = MIN_TRANSFORM_ENTRY_ID;

	return itemList;
}

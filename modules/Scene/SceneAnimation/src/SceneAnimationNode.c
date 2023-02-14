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

#include <DeepSea/Animation/DirectAnimationNodeMap.h>
#include <DeepSea/Animation/KeyframeAnimationNodeMap.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

typedef struct KeyframeMapRef
{
	dsKeyframeAnimationNodeMap* map;
	uint32_t refCount;
} KeyframeMapRef;

typedef struct DirectMapRef
{
	dsDirectAnimationNodeMap* map;
	uint32_t refCount;
} DirectMapRef;

struct dsSceneAnimationNode
{
	dsSceneNode node;
	const dsAnimationTree* animationTree;

	KeyframeMapRef* keyframeMaps;
	uint32_t keyframeMapCount;
	uint32_t maxKeyframeMaps;
	dsSpinlock keyframeMapLock;

	DirectMapRef* directMaps;
	uint32_t directMapCount;
	uint32_t maxDirectMaps;
	dsSpinlock directMapLock;
};

static void dsSceneAnimationNode_destroy(dsSceneNode* node)
{
	dsSceneAnimationNode* animationNode = (dsSceneAnimationNode*)node;

	for (uint32_t i = 0; i < animationNode->keyframeMapCount; ++i)
	{
		KeyframeMapRef* mapRef = animationNode->keyframeMaps + i;
		dsKeyframeAnimationNodeMap_destroy(mapRef->map);
	}
	DS_VERIFY(dsAllocator_free(node->allocator, animationNode->keyframeMaps));
	dsSpinlock_shutdown(&animationNode->keyframeMapLock);

	for (uint32_t i = 0; i < animationNode->directMapCount; ++i)
	{
		DirectMapRef* mapRef = animationNode->directMaps + i;
		dsDirectAnimationNodeMap_destroy(mapRef->map);
	}
	DS_VERIFY(dsAllocator_free(node->allocator, animationNode->directMaps));
	dsSpinlock_shutdown(&animationNode->directMapLock);

	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

const char* const dsSceneAnimationNode_typeName = "AnimationNode";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsSceneAnimationNode_type(void)
{
	return &nodeType;
}

dsSceneAnimationNode* dsSceneAnimationNode_create(dsAllocator* allocator,
	const dsAnimationTree* animationTree)
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

	node->keyframeMaps = NULL;
	node->keyframeMapCount = 0;
	node->maxKeyframeMaps = 0;
	DS_VERIFY(dsSpinlock_initialize(&node->keyframeMapLock));

	node->directMaps = NULL;
	node->directMapCount = 0;
	node->maxDirectMaps = 0;
	DS_VERIFY(dsSpinlock_initialize(&node->directMapLock));

	return node;
}

const dsAnimationTree* dsSceneAnimationNode_getAnimationTree(const dsSceneAnimationNode* node)
{
	if (!node)
	{
		errno = EINVAL;
		return NULL;
	}

	return node->animationTree;
}

const dsKeyframeAnimationNodeMap* dsSceneAnimationNode_addKeyframeAnimationNodeMap(
	dsSceneAnimationNode* node, const dsKeyframeAnimation* animation)
{
	if (!node || !animation)
	{
		errno = EINVAL;
		return NULL;
	}

	DS_VERIFY(dsSpinlock_lock(&node->keyframeMapLock));

	// Search for an existing entry.
	for (uint32_t i = 0; i < node->keyframeMapCount; ++i)
	{
		KeyframeMapRef* ref = node->keyframeMaps + i;
		if (ref->map->animation != animation)
			continue;

		++ref->refCount;
		// Keep direct pointer to avoid a separate thread invalidating the ref pointer.
		dsKeyframeAnimationNodeMap* map = ref->map;
		DS_VERIFY(dsSpinlock_unlock(&node->keyframeMapLock));
		return map;
	}

	// Add a new entry if not present.
	dsAllocator* allocator = ((dsSceneNode*)node)->allocator;
	uint32_t index = node->keyframeMapCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(allocator, node->keyframeMaps, node->keyframeMapCount,
			node->maxKeyframeMaps, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&node->keyframeMapLock));
		return NULL;
	}

	KeyframeMapRef* ref = node->keyframeMaps + index;
	ref->refCount = 1;
	ref->map = dsKeyframeAnimationNodeMap_create(allocator, animation, node->animationTree);
	// Keep direct pointer to avoid a separate thread invalidating the ref pointer.
	dsKeyframeAnimationNodeMap* map = ref->map;
	if (!map)
		--node->keyframeMapCount;

	DS_VERIFY(dsSpinlock_unlock(&node->keyframeMapLock));

	return map;
}

bool dsSceneAnimationNode_removeKeyframeAnimationNodeMap(dsSceneAnimationNode* node,
	const dsKeyframeAnimation* animation)
{
	if (!node || !animation)
	{
		errno = EINVAL;
		return false;
	}

	DS_VERIFY(dsSpinlock_lock(&node->keyframeMapLock));

	bool found = false;
	for (uint32_t i = 0; i < node->keyframeMapCount; ++i)
	{
		KeyframeMapRef* ref = node->keyframeMaps + i;
		if (ref->map->animation != animation)
			continue;

		found = true;
		if (--ref->refCount == 0)
		{
			// Order shouldn't matter, so use constant-time removal.
			*ref = node->keyframeMaps[node->keyframeMapCount - 1];
			--node->keyframeMapCount;
		}
		break;
	}

	DS_VERIFY(dsSpinlock_unlock(&node->keyframeMapLock));

	if (!found)
		errno = ENOTFOUND;
	return found;
}

const dsDirectAnimationNodeMap* dsSceneAnimationNode_addDirectAnimationNodeMap(
	dsSceneAnimationNode* node, const dsDirectAnimation* animation)
{
	if (!node || !animation)
	{
		errno = EINVAL;
		return NULL;
	}

	DS_VERIFY(dsSpinlock_lock(&node->directMapLock));

	// Search for an existing entry.
	for (uint32_t i = 0; i < node->directMapCount; ++i)
	{
		DirectMapRef* ref = node->directMaps + i;
		if (ref->map->animation != animation)
			continue;

		++ref->refCount;
		// Keep direct pointer to avoid a separate thread invalidating the ref pointer.
		dsDirectAnimationNodeMap* map = ref->map;
		DS_VERIFY(dsSpinlock_unlock(&node->directMapLock));
		return map;
	}

	// Add a new entry if not present.
	dsAllocator* allocator = ((dsSceneNode*)node)->allocator;
	uint32_t index = node->directMapCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(allocator, node->directMaps, node->directMapCount,
			node->maxDirectMaps, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&node->directMapLock));
		return NULL;
	}

	DirectMapRef* ref = node->directMaps + index;
	ref->refCount = 1;
	ref->map = dsDirectAnimationNodeMap_create(allocator, animation, node->animationTree);
	// Keep direct pointer to avoid a separate thread invalidating the ref pointer.
	dsDirectAnimationNodeMap* map = ref->map;
	if (!map)
		--node->directMapCount;

	DS_VERIFY(dsSpinlock_unlock(&node->directMapLock));

	return map;
}

bool dsSceneAnimationNode_removeDirectAnimationNodeMap(dsSceneAnimationNode* node,
	const dsDirectAnimation* animation)
{
	if (!node || !animation)
	{
		errno = EINVAL;
		return false;
	}

	DS_VERIFY(dsSpinlock_lock(&node->directMapLock));

	bool found = false;
	for (uint32_t i = 0; i < node->directMapCount; ++i)
	{
		DirectMapRef* ref = node->directMaps + i;
		if (ref->map->animation != animation)
			continue;

		found = true;
		if (--ref->refCount == 0)
		{
			// Order shouldn't matter, so use constant-time removal.
			*ref = node->directMaps[node->directMapCount - 1];
			--node->directMapCount;
		}
		break;
	}

	DS_VERIFY(dsSpinlock_unlock(&node->directMapLock));

	if (!found)
		errno = ENOTFOUND;
	return found;
}

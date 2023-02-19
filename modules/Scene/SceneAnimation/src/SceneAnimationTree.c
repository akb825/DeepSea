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

#include <DeepSea/SceneAnimation/SceneAnimationTree.h>

#include <DeepSea/Animation/AnimationTree.h>
#include <DeepSea/Animation/DirectAnimationNodeMap.h>
#include <DeepSea/Animation/KeyframeAnimationNodeMap.h>

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

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

struct dsSceneAnimationTree
{
	dsAllocator* allocator;
	dsAnimationTree* animationTree;

	KeyframeMapRef* keyframeMaps;
	uint32_t keyframeMapCount;
	uint32_t maxKeyframeMaps;
	dsSpinlock keyframeMapLock;

	DirectMapRef* directMaps;
	uint32_t directMapCount;
	uint32_t maxDirectMaps;
	dsSpinlock directMapLock;
};

static bool destroyResource(void* resource)
{
	dsSceneAnimationTree_destroy((dsSceneAnimationTree*)resource);
	return true;
}

const char* const dsSceneAnimationTree_typeName = "AnimationTree";

static dsCustomSceneResourceType resourceType;
const dsCustomSceneResourceType* dsSceneAnimationTree_type(void)
{
	return &resourceType;
}

dsSceneAnimationTree* dsSceneAnimationTree_create(dsAllocator* allocator,
	dsAnimationTree* animationTree)
{
	if (!allocator || !animationTree)
	{
		dsAnimationTree_destroy(animationTree);
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		dsAnimationTree_destroy(animationTree);
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG,
			"Scene animation tree allocator must support freeing memory.");
		return false;
	}

	dsSceneAnimationTree* sceneTree = DS_ALLOCATE_OBJECT(allocator, dsSceneAnimationTree);
	if (!sceneTree)
	{
		dsAnimationTree_destroy(animationTree);
		return NULL;
	}

	sceneTree->allocator = allocator;
	sceneTree->animationTree = animationTree;

	sceneTree->keyframeMaps = NULL;
	sceneTree->keyframeMapCount = 0;
	sceneTree->maxKeyframeMaps = 0;
	DS_VERIFY(dsSpinlock_initialize(&sceneTree->keyframeMapLock));

	sceneTree->directMaps = NULL;
	sceneTree->directMapCount = 0;
	sceneTree->maxDirectMaps = 0;
	DS_VERIFY(dsSpinlock_initialize(&sceneTree->directMapLock));

	return sceneTree;
}

dsCustomSceneResource* dsSceneAnimationTree_createResource(dsAllocator* allocator,
	dsSceneAnimationTree* tree)
{
	if (!allocator || !tree)
	{
		errno = EINVAL;
		return NULL;
	}

	dsCustomSceneResource* customResource = DS_ALLOCATE_OBJECT(allocator, dsCustomSceneResource);
	if (!customResource)
		return NULL;

	customResource->allocator = dsAllocator_keepPointer(allocator);
	customResource->type = &resourceType;
	customResource->resource = tree;
	customResource->destroyFunc = &destroyResource;
	return customResource;
}

const dsAnimationTree* dsSceneAnimationTree_getAnimationTree(const dsSceneAnimationTree* tree)
{
	if (!tree)
	{
		errno = EINVAL;
		return NULL;
	}

	return tree->animationTree;
}

const dsKeyframeAnimationNodeMap* dsSceneAnimationTree_addKeyframeAnimationNodeMap(
	dsSceneAnimationTree* tree, const dsKeyframeAnimation* animation)
{
	if (!tree || !animation)
	{
		errno = EINVAL;
		return NULL;
	}

	DS_VERIFY(dsSpinlock_lock(&tree->keyframeMapLock));

	// Search for an existing entry.
	for (uint32_t i = 0; i < tree->keyframeMapCount; ++i)
	{
		KeyframeMapRef* ref = tree->keyframeMaps + i;
		if (ref->map->animation != animation)
			continue;

		++ref->refCount;
		// Keep direct pointer to avoid a separate thread invalidating the ref pointer.
		dsKeyframeAnimationNodeMap* map = ref->map;
		DS_VERIFY(dsSpinlock_unlock(&tree->keyframeMapLock));
		return map;
	}

	// Add a new entry if not present.
	dsAllocator* allocator = ((dsSceneNode*)tree)->allocator;
	uint32_t index = tree->keyframeMapCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(allocator, tree->keyframeMaps, tree->keyframeMapCount,
			tree->maxKeyframeMaps, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&tree->keyframeMapLock));
		return NULL;
	}

	KeyframeMapRef* ref = tree->keyframeMaps + index;
	ref->refCount = 1;
	ref->map = dsKeyframeAnimationNodeMap_create(allocator, animation, tree->animationTree);
	// Keep direct pointer to avoid a separate thread invalidating the ref pointer.
	dsKeyframeAnimationNodeMap* map = ref->map;
	if (!map)
		--tree->keyframeMapCount;

	DS_VERIFY(dsSpinlock_unlock(&tree->keyframeMapLock));

	return map;
}

bool dsSceneAnimationTree_removeKeyframeAnimationNodeMap(dsSceneAnimationTree* tree,
	const dsKeyframeAnimation* animation)
{
	if (!tree || !animation)
	{
		errno = EINVAL;
		return false;
	}

	DS_VERIFY(dsSpinlock_lock(&tree->keyframeMapLock));

	bool found = false;
	for (uint32_t i = 0; i < tree->keyframeMapCount; ++i)
	{
		KeyframeMapRef* ref = tree->keyframeMaps + i;
		if (ref->map->animation != animation)
			continue;

		found = true;
		if (--ref->refCount == 0)
		{
			// Order shouldn't matter, so use constant-time removal.
			*ref = tree->keyframeMaps[tree->keyframeMapCount - 1];
			--tree->keyframeMapCount;
		}
		break;
	}

	DS_VERIFY(dsSpinlock_unlock(&tree->keyframeMapLock));

	if (!found)
		errno = ENOTFOUND;
	return found;
}

const dsDirectAnimationNodeMap* dsSceneAnimationTree_addDirectAnimationNodeMap(
	dsSceneAnimationTree* tree, const dsDirectAnimation* animation)
{
	if (!tree || !animation)
	{
		errno = EINVAL;
		return NULL;
	}

	DS_VERIFY(dsSpinlock_lock(&tree->directMapLock));

	// Search for an existing entry.
	for (uint32_t i = 0; i < tree->directMapCount; ++i)
	{
		DirectMapRef* ref = tree->directMaps + i;
		if (ref->map->animation != animation)
			continue;

		++ref->refCount;
		// Keep direct pointer to avoid a separate thread invalidating the ref pointer.
		dsDirectAnimationNodeMap* map = ref->map;
		DS_VERIFY(dsSpinlock_unlock(&tree->directMapLock));
		return map;
	}

	// Add a new entry if not present.
	dsAllocator* allocator = ((dsSceneNode*)tree)->allocator;
	uint32_t index = tree->directMapCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(allocator, tree->directMaps, tree->directMapCount,
			tree->maxDirectMaps, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&tree->directMapLock));
		return NULL;
	}

	DirectMapRef* ref = tree->directMaps + index;
	ref->refCount = 1;
	ref->map = dsDirectAnimationNodeMap_create(allocator, animation, tree->animationTree);
	// Keep direct pointer to avoid a separate thread invalidating the ref pointer.
	dsDirectAnimationNodeMap* map = ref->map;
	if (!map)
		--tree->directMapCount;

	DS_VERIFY(dsSpinlock_unlock(&tree->directMapLock));

	return map;
}

bool dsSceneAnimationTree_removeDirectAnimationNodeMap(dsSceneAnimationTree* tree,
	const dsDirectAnimation* animation)
{
	if (!tree || !animation)
	{
		errno = EINVAL;
		return false;
	}

	DS_VERIFY(dsSpinlock_lock(&tree->directMapLock));

	bool found = false;
	for (uint32_t i = 0; i < tree->directMapCount; ++i)
	{
		DirectMapRef* ref = tree->directMaps + i;
		if (ref->map->animation != animation)
			continue;

		found = true;
		if (--ref->refCount == 0)
		{
			// Order shouldn't matter, so use constant-time removal.
			*ref = tree->directMaps[tree->directMapCount - 1];
			--tree->directMapCount;
		}
		break;
	}

	DS_VERIFY(dsSpinlock_unlock(&tree->directMapLock));

	if (!found)
		errno = ENOTFOUND;
	return found;
}

void dsSceneAnimationTree_destroy(dsSceneAnimationTree* tree)
{
	if (!tree)
		return;

	dsAnimationTree_destroy(tree->animationTree);

	for (uint32_t i = 0; i < tree->keyframeMapCount; ++i)
	{
		KeyframeMapRef* mapRef = tree->keyframeMaps + i;
		dsKeyframeAnimationNodeMap_destroy(mapRef->map);
	}
	DS_VERIFY(dsAllocator_free(tree->allocator, tree->keyframeMaps));
	dsSpinlock_shutdown(&tree->keyframeMapLock);

	for (uint32_t i = 0; i < tree->directMapCount; ++i)
	{
		DirectMapRef* mapRef = tree->directMaps + i;
		dsDirectAnimationNodeMap_destroy(mapRef->map);
	}
	DS_VERIFY(dsAllocator_free(tree->allocator, tree->directMaps));
	dsSpinlock_shutdown(&tree->directMapLock);

	DS_VERIFY(dsAllocator_free(tree->allocator, tree));
}

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

#include <DeepSea/Animation/Animation.h>

#include "AnimationNodeMapCacheInternal.h"

#include <DeepSea/Animation/AnimationTree.h>

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Sort.h>

#include <DeepSea/Math/Core.h>

static int directAnimationEntryCompare(const void* left, const void* right, void* context)
{
	const dsDirectAnimation* animation = (const dsDirectAnimation*)left;
	const dsDirectAnimationEntry* ref = (const dsDirectAnimationEntry*)right;
	return DS_CMP(animation, ref->animation);
}

static int keyframeAnimationEntryCompare(const void* left, const void* right, void* context)
{
	const dsKeyframeAnimation* animation = (const dsKeyframeAnimation*)left;
	const dsKeyframeAnimationEntry* ref = (const dsKeyframeAnimationEntry*)right;
	return DS_CMP(animation, ref->animation);
}

dsAnimation* dsAnimation_create(dsAllocator* allocator, dsAnimationNodeMapCache* nodeMapCache)
{
	if (!allocator || !nodeMapCache)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_ANIMATION_LOG_TAG, "Animation allocator must support freeing memory.");
		return NULL;
	}

	dsAnimation* animation = DS_ALLOCATE_OBJECT(allocator, dsAnimation);
	if (!animation)
		return NULL;

	animation->allocator = dsAllocator_keepPointer(allocator);
	animation->nodeMapCache = nodeMapCache;
	animation->keyframeEntries = NULL;
	animation->keyframeEntryCount = 0;
	animation->maxKeyframeEntries = 0;
	animation->directEntries = NULL;
	animation->directEntryCount = 0;
	animation->maxDirectEntries = 0;

	return animation;
}

bool dsAnimation_addKeyframeAnimation(dsAnimation* animation,
	const dsKeyframeAnimation* keyframeAnimation, float weight, double time, double timeScale,
	bool wrap)
{
	if (!animation || !keyframeAnimation)
	{
		errno = EINVAL;
		return false;
	}

	const dsKeyframeAnimationEntry* prevEntry =
		(const dsKeyframeAnimationEntry*)dsBinarySearchLowerBound(keyframeAnimation,
			animation->keyframeEntries, animation->keyframeEntryCount, sizeof(dsKeyframeAnimationEntry),
			&keyframeAnimationEntryCompare, NULL);
	if (prevEntry && prevEntry->animation == keyframeAnimation)
	{
		errno = EPERM;
		return false;
	}

	size_t index = prevEntry ? prevEntry - animation->keyframeEntries :
		animation->keyframeEntryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(animation->allocator, animation->keyframeEntries,
			animation->keyframeEntryCount, animation->maxKeyframeEntries, 1))
	{
		return false;
	}

	if (!dsAnimationNodeMapCache_addKeyframeAnimation(animation->nodeMapCache, keyframeAnimation))
	{
		--animation->directEntryCount;
		return false;
	}

	// Need to shift the entries to keep them sorted.
	for (uint32_t i = animation->keyframeEntryCount; --i > index;)
		animation->keyframeEntries[i] = animation->keyframeEntries[i - 1];

	dsKeyframeAnimationEntry* entry = animation->keyframeEntries + index;
	entry->animation = keyframeAnimation;
	entry->time = time;
	entry->timeScale = timeScale;
	entry->wrap = wrap;
	entry->weight = weight;
	return true;
}

dsKeyframeAnimationEntry* dsAnimation_findKeyframeAnimationEntry(dsAnimation* animation,
	const dsKeyframeAnimation* keyframeAnimation)
{
	if (!animation || !keyframeAnimation)
		return NULL;

	return (dsKeyframeAnimationEntry*)dsBinarySearch(keyframeAnimation, animation->keyframeEntries,
		animation->keyframeEntryCount, sizeof(dsKeyframeAnimationEntry),
		&keyframeAnimationEntryCompare, NULL);
}

bool dsAnimation_removeKeyframeAnimation(dsAnimation* animation,
	const dsKeyframeAnimation* keyframeAnimation)
{
	if (!animation || !keyframeAnimation)
	{
		errno = EINVAL;
		return false;
	}

	const dsKeyframeAnimationEntry* entry = (const dsKeyframeAnimationEntry*)dsBinarySearch(
		keyframeAnimation, animation->keyframeEntries, animation->keyframeEntryCount,
		sizeof(dsKeyframeAnimationEntry), &keyframeAnimationEntryCompare, NULL);
	if (!entry)
	{
		errno = ENOTFOUND;
		return false;
	}

	--animation->keyframeEntryCount;
	// Need to shift the entries back into place.
	for (size_t i = entry - animation->keyframeEntries; i < animation->keyframeEntryCount; ++i)
		animation->keyframeEntries[i] = animation->keyframeEntries[i + 1];
	DS_VERIFY(dsAnimationNodeMapCache_removeKeyframeAnimation(animation->nodeMapCache,
		keyframeAnimation));
	return true;
}

bool dsAnimation_addDirectAnimation(dsAnimation* animation,
	const dsDirectAnimation* directAnimation, float weight)
{
	if (!animation || !directAnimation)
	{
		errno = EINVAL;
		return false;
	}

	const dsDirectAnimationEntry* prevEntry =
		(const dsDirectAnimationEntry*)dsBinarySearchLowerBound(directAnimation,
			animation->directEntries, animation->directEntryCount, sizeof(dsDirectAnimationEntry),
			&directAnimationEntryCompare, NULL);
	if (prevEntry && prevEntry->animation == directAnimation)
	{
		errno = EPERM;
		return false;
	}

	size_t index = prevEntry ? prevEntry - animation->directEntries : animation->directEntryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(animation->allocator, animation->directEntries,
			animation->directEntryCount, animation->maxDirectEntries, 1))
	{
		return false;
	}

	if (!dsAnimationNodeMapCache_addDirectAnimation(animation->nodeMapCache, directAnimation))
	{
		--animation->directEntryCount;
		return false;
	}

	// Need to shift the entries to keep them sorted.
	for (uint32_t i = animation->directEntryCount; --i > index;)
		animation->directEntries[i] = animation->directEntries[i - 1];

	dsDirectAnimationEntry* entry = animation->directEntries + index;
	entry->animation = directAnimation;
	entry->weight = weight;
	return true;
}

dsDirectAnimationEntry* dsAnimation_findDirectAnimationEntry(dsAnimation* animation,
	const dsDirectAnimation* directAnimation)
{
	if (!animation || !directAnimation)
		return NULL;

	return (dsDirectAnimationEntry*)dsBinarySearch(directAnimation, animation->directEntries,
		animation->directEntryCount, sizeof(dsDirectAnimationEntry),
		&directAnimationEntryCompare, NULL);
}

bool dsAnimation_removeDirectAnimation(dsAnimation* animation,
	const dsDirectAnimation* directAnimation)
{
	if (!animation || !directAnimation)
		return false;

	const dsDirectAnimationEntry* entry = (const dsDirectAnimationEntry*)dsBinarySearch(
		directAnimation, animation->directEntries, animation->directEntryCount,
		sizeof(dsDirectAnimationEntry), &directAnimationEntryCompare, NULL);
	if (!entry)
	{
		errno = ENOTFOUND;
		return false;
	}

	--animation->directEntryCount;
	// Need to shift the entries back into place.
	for (size_t i = entry - animation->directEntries; i < animation->directEntryCount; ++i)
		animation->directEntries[i] = animation->directEntries[i + 1];
	DS_VERIFY(dsAnimationNodeMapCache_removeDirectAnimation(animation->nodeMapCache,
		directAnimation));
	return true;
}

bool dsAnimation_update(dsAnimation* animation, double time)
{
	if (!animation)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < animation->keyframeEntryCount; ++i)
	{
		dsKeyframeAnimationEntry* entry = animation->keyframeEntries + i;
		entry->time += time*entry->timeScale;
		if (entry->wrap)
		{
			entry->time =
				dsWrapd(entry->time, entry->animation->minTime, entry->animation->maxTime);
		}
	}

	return true;
}

bool dsAnimation_apply(const dsAnimation* animation, dsAnimationTree* tree)
{
	if (!animation || !tree)
	{
		errno = EINVAL;
		return false;
	}

	return dsAnimationNodeMapCache_applyAnimation(animation->nodeMapCache, animation, tree) &&
		dsAnimationTree_updateTransforms(tree);
}

void dsAnimation_destroy(dsAnimation* animation)
{
	if (!animation)
		return;

	for (uint32_t i = 0; i < animation->keyframeEntryCount; ++i)
	{
		DS_VERIFY(dsAnimationNodeMapCache_removeKeyframeAnimation(animation->nodeMapCache,
			animation->keyframeEntries[i].animation));
	}

	for (uint32_t i = 0; i < animation->directEntryCount; ++i)
	{
		DS_VERIFY(dsAnimationNodeMapCache_removeDirectAnimation(animation->nodeMapCache,
			animation->directEntries[i].animation));
	}

	DS_VERIFY(dsAllocator_free(animation->allocator, animation->keyframeEntries));
	DS_VERIFY(dsAllocator_free(animation->allocator, animation->directEntries));
	DS_VERIFY(dsAllocator_free(animation->allocator, animation));
}

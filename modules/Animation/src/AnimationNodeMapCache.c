/*
 * Copyright 2023-2024 Aaron Barany
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

#include <DeepSea/Animation/AnimationNodeMapCache.h>

#include "AnimationInternal.h"
#include "AnimationNodeMapCacheInternal.h"
#include "DirectAnimationNodeMap.h"
#include "KeyframeAnimationNodeMap.h"

#include <DeepSea/Animation/AnimationTree.h>

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Thread/ReadWriteSpinlock.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Sort.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Quaternion.h>
#include <DeepSea/Math/Vector4.h>

#include <string.h>

typedef struct WeightedTransform
{
	dsVector4f translation;
	dsVector4f rotation;
	dsVector4f scale;
	float totalTranslationWeight;
	float totalRotationWeight;
	float totalScaleWeight;
} WeightedTransform;

static uint32_t findEndKeyframe(const float* keyframeTimes, uint32_t keyframeCount, double time)
{
	// NOTE: If there's regularly many keyframes can use a binary search. Expected to be a fairly
	// small number of keyframes on average, so a linear search should be fine and in many cases
	// faster.
	for (uint32_t i = 1; i < keyframeCount; ++i)
	{
		if (keyframeTimes[i] > time)
			return i;
	}

	return keyframeCount - 1;
}

static void evaluateCubicSpline(dsVector4f* result, const dsMatrix44f* cubicTransposed, float t)
{
	float t2 = t*t;
	dsVector4f eval = {{1.0f, t, t2, t2*t}};
	dsMatrix44f_transform(result, cubicTransposed, &eval);
}

static void applyKeyframeAnimationTransforms(WeightedTransform* transforms,
	dsKeyframeAnimationNodeMap** nodeMaps, uint32_t nodeMapCount, const dsAnimation* animation,
	const dsAnimationTree* tree)
{
	dsKeyframeAnimationNodeMap** curNodeMap = nodeMaps;
	dsKeyframeAnimationNodeMap** nodeMapEnd = nodeMaps + nodeMapCount;
	for (uint32_t i = 0; i < animation->keyframeEntryCount; ++i)
	{
		const dsKeyframeAnimationEntry* entry = animation->keyframeEntries + i;
		if (entry->weight <= 0)
			continue;

		const dsKeyframeAnimation* keyframeAnimation = entry->animation;

		// Advance until we find the current keyframe animation's node map.
		while (curNodeMap != nodeMapEnd && (*curNodeMap)->animation < keyframeAnimation)
			++curNodeMap;
		if (curNodeMap == nodeMapEnd || (*curNodeMap)->animation != keyframeAnimation)
			continue;

		const dsKeyframeAnimationNodeMap* map = *curNodeMap;
		DS_ASSERT(keyframeAnimation->keyframesCount == map->keyframesCount);
		for (uint32_t j = 0; j < keyframeAnimation->keyframesCount; ++j)
		{
			const dsAnimationKeyframes* keyframes = keyframeAnimation->keyframes + j;
			const dsAnimationKeyframesNodeMap* keyframesMap = map->keyframesMaps + j;
			DS_ASSERT(keyframes->channelCount == keyframesMap->channelCount);

			// Find wich pair of keyframes to interpolate between.
			uint32_t startKeyframe;
			float t;
			if (entry->time <= keyframes->keyframeTimes[0] || keyframes->keyframeCount == 1)
			{
				startKeyframe = 0;
				t = 0;
			}
			else if (entry->time >= keyframes->keyframeTimes[keyframes->keyframeCount - 1])
			{
				startKeyframe = keyframes->keyframeCount - 2;
				t = 1;
			}
			else
			{
				uint32_t endKeyframe = findEndKeyframe(keyframes->keyframeTimes,
					keyframes->keyframeCount, entry->time);
				DS_ASSERT(endKeyframe > 0);
				startKeyframe = endKeyframe - 1;
				float startTime = keyframes->keyframeTimes[startKeyframe];
				float endTime = keyframes->keyframeTimes[endKeyframe];
				t = (float)(entry->time - startTime)/(endTime - startTime);
			}

			for (uint32_t k = 0; k < keyframes->channelCount; ++k)
			{
				const dsKeyframeAnimationChannel* channel = keyframes->channels + k;
				uint32_t nodeIndex = keyframesMap->channelNodes[k];
				if (nodeIndex == DS_NO_ANIMATION_NODE)
					continue;

				WeightedTransform* transform = transforms + nodeIndex;
				dsVector4f value;
				switch (channel->interpolation)
				{
					case dsAnimationInterpolation_Step:
						value = channel->values[startKeyframe];
						break;
					case dsAnimationInterpolation_Linear:
						DS_ASSERT(keyframes->keyframeCount > 1);
						if (channel->component == dsAnimationComponent_Rotation)
						{
							const dsQuaternion4f* firstValue =
								(dsQuaternion4f*)channel->values + startKeyframe;
							dsQuaternion4f_slerp((dsQuaternion4f*)&value, firstValue,
								firstValue + 1, t);
						}
						else
						{
							const dsVector4f* firstValue = channel->values + startKeyframe;
							dsVector4f_lerp(&value, firstValue, firstValue + 1, t);
						}
						break;
					case dsAnimationInterpolation_Cubic:
					{
						const dsMatrix44f* cubicTransposed =
							(const dsMatrix44f*)channel->values + startKeyframe;
						evaluateCubicSpline(&value, cubicTransposed, t);
						break;
					}
					default:
						DS_ASSERT(false);
						DS_UNREACHABLE();
				}

				dsVector4f weightedValue;
				dsVector4f_scale(&weightedValue, &value, entry->weight);
				switch (channel->component)
				{
					case dsAnimationComponent_Translation:
						dsVector4f_add(&transform->translation, &transform->translation,
							&weightedValue);
						transform->totalTranslationWeight += entry->weight;
						break;
					case dsAnimationComponent_Rotation:
						dsVector4f_add(&transform->rotation, &transform->rotation, &weightedValue);
						transform->totalRotationWeight += entry->weight;
						break;
					case dsAnimationComponent_Scale:
						dsVector4f_add(&transform->scale, &transform->scale, &weightedValue);
						transform->totalScaleWeight += entry->weight;
						break;
				}
			}
		}
	}
}

static void applyDirectAnimationTransforms(WeightedTransform* transforms,
	dsDirectAnimationNodeMap** nodeMaps, uint32_t nodeMapCount, const dsAnimation* animation, const dsAnimationTree* tree)
{
	dsDirectAnimationNodeMap** curNodeMap = nodeMaps;
	dsDirectAnimationNodeMap** nodeMapEnd = nodeMaps + nodeMapCount;
	for (uint32_t i = 0; i < animation->directEntryCount; ++i)
	{
		const dsDirectAnimationEntry* entry = animation->directEntries + i;
		if (entry->weight <= 0)
			continue;

		const dsDirectAnimation* directAnimation = entry->animation;

		// Advance until we find the current keyframe animation's node map.
		while (curNodeMap != nodeMapEnd && (*curNodeMap)->animation < directAnimation)
			++curNodeMap;
		if (curNodeMap == nodeMapEnd || (*curNodeMap)->animation != directAnimation)
			continue;

		const dsDirectAnimationNodeMap* map = *curNodeMap;
		DS_ASSERT(directAnimation->channelCount == map->channelCount);
		for (uint32_t j = 0; j < directAnimation->channelCount; ++j)
		{
			const dsDirectAnimationChannel* channel = directAnimation->channels + j;
			uint32_t nodeIndex = map->channelNodes[j];
			if (nodeIndex == DS_NO_ANIMATION_NODE)
				continue;
			WeightedTransform* transform = transforms + nodeIndex;

			dsVector4f weightedValue;
			dsVector4f_scale(&weightedValue, &channel->value, entry->weight);
			switch (channel->component)
			{
				case dsAnimationComponent_Translation:
					dsVector4f_add(&transform->translation, &transform->translation,
						&weightedValue);
					transform->totalTranslationWeight += entry->weight;
					break;
				case dsAnimationComponent_Rotation:
					dsVector4f_add(&transform->rotation, &transform->rotation, &weightedValue);
					transform->totalRotationWeight += entry->weight;
					break;
				case dsAnimationComponent_Scale:
					dsVector4f_add(&transform->scale, &transform->scale, &weightedValue);
					transform->totalScaleWeight += entry->weight;
					break;
			}
		}
	}
}

static int animationTreeNodeMapCompare(const void* left, const void* right, void* context)
{
	uint32_t treeID = *(const uint32_t*)left;
	const dsAnimationTreeNodeMap* nodeMap = (const dsAnimationTreeNodeMap*)right;
	return DS_CMP(treeID, nodeMap->tree->id);
}

static int directAnimationRefCompare(const void* left, const void* right, void* context)
{
	const dsDirectAnimation* animation = (const dsDirectAnimation*)left;
	const dsDirectAnimationRef* ref = (const dsDirectAnimationRef*)right;
	return DS_CMP(animation,ref->animation);
}

static int keyframeAnimationRefCompare(const void* left, const void* right, void* context)
{
	const dsKeyframeAnimation* animation = (const dsKeyframeAnimation*)left;
	const dsKeyframeAnimationRef* ref = (const dsKeyframeAnimationRef*)right;
	return DS_CMP(animation, ref->animation);
}

static void freeAnimationTreeNodeMap(dsAnimationNodeMapCache* cache, dsAnimationTreeNodeMap* map)
{
	for (uint32_t i = 0; i < cache->keyframeAnimationCount; ++i)
		dsKeyframeAnimationNodeMap_destroy(map->keyframeMaps[i]);
	DS_VERIFY(dsAllocator_free(cache->allocator, map->keyframeMaps));

	for (uint32_t i = 0; i < cache->directAnimationCount; ++i)
		dsDirectAnimationNodeMap_destroy(map->directMaps[i]);
	DS_VERIFY(dsAllocator_free(cache->allocator, map->directMaps));

	dsAnimationTree_destroy(map->tree);
}

dsAnimationNodeMapCache* dsAnimationNodeMapCache_create(dsAllocator* allocator)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_ANIMATION_LOG_TAG,
			"Animation node map cahe allocator must support freeing memory.");
		return NULL;
	}

	dsAnimationNodeMapCache* cache = DS_ALLOCATE_OBJECT(allocator, dsAnimationNodeMapCache);
	if (!cache)
		return NULL;

	cache->allocator = dsAllocator_keepPointer(allocator);
	cache->keyframeAnimations = NULL;
	cache->keyframeAnimationCount = 0;
	cache->maxKeyframeAnimations = 0;
	cache->directAnimations = NULL;
	cache->directAnimationCount = 0;
	cache->maxDirectAnimations = 0;
	cache->treeMaps = NULL;
	cache->treeMapCount = 0;
	cache->maxTreeMaps = 0;
	DS_VERIFY(dsReadWriteSpinlock_initialize(&cache->lock));
	return cache;
}

bool dsAnimationNodeMapCache_addAnimationTree(dsAnimationNodeMapCache* cache, dsAnimationTree* tree)
{
	if (!cache || !tree)
	{
		errno = EINVAL;
		return false;
	}

	DS_VERIFY(dsReadWriteSpinlock_lockWrite(&cache->lock));

	dsAnimationTreeNodeMap* prevTreeNodeMap = (dsAnimationTreeNodeMap*)dsBinarySearchLowerBound(
		&tree->id, cache->treeMaps, cache->treeMapCount, sizeof(dsAnimationTreeNodeMap),
		animationTreeNodeMapCompare, NULL);
	if (!prevTreeNodeMap || prevTreeNodeMap->tree->id != tree->id)
	{
		size_t index = prevTreeNodeMap ? prevTreeNodeMap - cache->treeMaps : cache->treeMapCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(cache->allocator, cache->treeMaps, cache->treeMapCount,
				cache->maxTreeMaps, 1))
		{
			DS_VERIFY(dsReadWriteSpinlock_unlockWrite(&cache->lock));
			return false;
		}

		dsAnimationTreeNodeMap newTreeMap;
		newTreeMap.tree = dsAnimationTree_clone(cache->allocator, tree);
		newTreeMap.refCount = 1;
		if (!newTreeMap.tree)
		{
			--cache->treeMapCount;
			DS_VERIFY(dsReadWriteSpinlock_unlockWrite(&cache->lock));
			return false;
		}

		if (cache->keyframeAnimationCount > 0)
		{
			newTreeMap.keyframeMaps = DS_ALLOCATE_OBJECT_ARRAY(cache->allocator,
				dsKeyframeAnimationNodeMap*, cache->keyframeAnimationCount);
			if (!newTreeMap.keyframeMaps)
			{
				dsAnimationTree_destroy(newTreeMap.tree);
				--cache->treeMapCount;
				DS_VERIFY(dsReadWriteSpinlock_unlockWrite(&cache->lock));
				return false;
			}

			newTreeMap.maxKeyframeMaps = cache->keyframeAnimationCount;
			for (uint32_t i = 0; i < cache->keyframeAnimationCount; ++i)
			{
				newTreeMap.keyframeMaps[i] = dsKeyframeAnimationNodeMap_create(cache->allocator,
					cache->keyframeAnimations[i].animation, newTreeMap.tree);
				if (!newTreeMap.keyframeMaps[i])
				{
					for (uint32_t j = 0; j < i; ++j)
						dsKeyframeAnimationNodeMap_destroy(newTreeMap.keyframeMaps[j]);
					DS_VERIFY(dsAllocator_free(cache->allocator, newTreeMap.keyframeMaps));

					dsAnimationTree_destroy(newTreeMap.tree);
					--cache->treeMapCount;
					DS_VERIFY(dsReadWriteSpinlock_unlockWrite(&cache->lock));
					return false;
				}
			}
		}
		else
		{
			newTreeMap.keyframeMaps = NULL;
			newTreeMap.maxKeyframeMaps = 0;
		}

		if (cache->directAnimationCount > 0)
		{
			newTreeMap.directMaps = DS_ALLOCATE_OBJECT_ARRAY(cache->allocator,
				dsDirectAnimationNodeMap*, cache->directAnimationCount);
			if (!newTreeMap.directMaps)
			{
				dsAnimationTree_destroy(newTreeMap.tree);
				--cache->treeMapCount;
				DS_VERIFY(dsReadWriteSpinlock_unlockWrite(&cache->lock));
				return false;
			}

			newTreeMap.maxDirectMaps = cache->directAnimationCount;
			for (uint32_t i = 0; i < cache->directAnimationCount; ++i)
			{
				newTreeMap.directMaps[i] = dsDirectAnimationNodeMap_create(cache->allocator,
					cache->directAnimations[i].animation, newTreeMap.tree);
				if (!newTreeMap.directMaps[i])
				{
					for (uint32_t j = 0; j < cache->keyframeAnimationCount; ++j)
						dsKeyframeAnimationNodeMap_destroy(newTreeMap.keyframeMaps[j]);
					DS_VERIFY(dsAllocator_free(cache->allocator, newTreeMap.keyframeMaps));

					for (uint32_t j = 0; j < i; ++j)
						dsDirectAnimationNodeMap_destroy(newTreeMap.directMaps[j]);
					DS_VERIFY(dsAllocator_free(cache->allocator, newTreeMap.directMaps));

					dsAnimationTree_destroy(newTreeMap.tree);
					--cache->treeMapCount;
					DS_VERIFY(dsReadWriteSpinlock_unlockWrite(&cache->lock));
					return false;
				}
			}
		}
		else
		{
			newTreeMap.directMaps = NULL;
			newTreeMap.maxDirectMaps = 0;
		}

		// Need to shift the entries to keep them sorted.
		for (uint32_t i = cache->treeMapCount; --i > index;)
			cache->treeMaps[i] = cache->treeMaps[i - 1];
		cache->treeMaps[index] = newTreeMap;
	}
	else
		++prevTreeNodeMap->refCount;

	DS_VERIFY(dsReadWriteSpinlock_unlockWrite(&cache->lock));
	return true;
}

bool dsAnimationNodeMapCache_removeAnimationTree(dsAnimationNodeMapCache* cache,
	dsAnimationTree* tree)
{
	if (!cache || !tree)
	{
		errno = EINVAL;
		return false;
	}

	DS_VERIFY(dsReadWriteSpinlock_lockWrite(&cache->lock));

	bool found;
	dsAnimationTreeNodeMap* treeNodeMap = (dsAnimationTreeNodeMap*)dsBinarySearch(
		&tree->id, cache->treeMaps, cache->treeMapCount, sizeof(dsAnimationTreeNodeMap),
		animationTreeNodeMapCompare, NULL);
	if (treeNodeMap)
	{
		found = true;
		if (--treeNodeMap->refCount == 0)
		{
			freeAnimationTreeNodeMap(cache, treeNodeMap);
			--cache->treeMapCount;
			for (size_t i = treeNodeMap - cache->treeMaps; i < cache->treeMapCount; ++i)
				cache->treeMaps[i] = cache->treeMaps[i + 1];
		}
	}
	else
	{
		errno = ENOTFOUND;
		found = false;
	}

	DS_VERIFY(dsReadWriteSpinlock_unlockWrite(&cache->lock));
	return found;
}

bool dsAnimationNodeMapCache_addKeyframeAnimation(dsAnimationNodeMapCache* cache,
	const dsKeyframeAnimation* animation)
{
	DS_ASSERT(cache);
	DS_ASSERT(animation);

	DS_VERIFY(dsReadWriteSpinlock_lockWrite(&cache->lock));

	dsKeyframeAnimationRef* prevKeyframeRef = (dsKeyframeAnimationRef*)dsBinarySearchLowerBound(
		animation, cache->keyframeAnimations, cache->keyframeAnimationCount,
		sizeof(dsKeyframeAnimationRef), keyframeAnimationRefCompare, NULL);
	if (!prevKeyframeRef || prevKeyframeRef->animation != animation)
	{
		size_t index = prevKeyframeRef ? prevKeyframeRef - cache->keyframeAnimations :
			cache->keyframeAnimationCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(cache->allocator, cache->keyframeAnimations,
				cache->keyframeAnimationCount, cache->maxKeyframeAnimations, 1))
		{
			DS_VERIFY(dsReadWriteSpinlock_unlockWrite(&cache->lock));
			return false;
		}

		for (uint32_t i = 0; i < cache->treeMapCount; ++i)
		{
			dsAnimationTreeNodeMap* treeMap = cache->treeMaps + i;
			dsKeyframeAnimationNodeMap* map = dsKeyframeAnimationNodeMap_create(cache->allocator,
				animation, treeMap->tree);
			uint32_t dummyCount = cache->keyframeAnimationCount - 1;
			if (map && !DS_RESIZEABLE_ARRAY_ADD(cache->allocator, treeMap->keyframeMaps,
					dummyCount, treeMap->maxKeyframeMaps, 1))
			{
				dsKeyframeAnimationNodeMap_destroy(map);
				map = NULL;
			}

			if (map)
			{
				// Need to shift the entries to keep them sorted.
				for (uint32_t j = cache->keyframeAnimationCount; --j > index;)
					treeMap->keyframeMaps[j] = treeMap->keyframeMaps[j - 1];
				treeMap->keyframeMaps[index] = map;
			}
			else
			{
				// Failed to add an entry, restore the previous state.
				--cache->keyframeAnimationCount;

				for (uint32_t j = 0; j < i; ++j)
				{
					dsAnimationTreeNodeMap* restoreTreeMap = cache->treeMaps + j;
					dsKeyframeAnimationNodeMap_destroy(restoreTreeMap->keyframeMaps[index]);

					// Need to shift the entries back into place.
					for (size_t k = index; k < cache->keyframeAnimationCount; ++k)
						cache->treeMaps[k] = cache->treeMaps[k + 1];
				}

				DS_VERIFY(dsReadWriteSpinlock_unlockWrite(&cache->lock));
				return false;
			}
		}

		// Added all the keyframe animation maps, insert the final animation here. Need to shift the
		// entries to keep them sorted.
		for (uint32_t i = cache->keyframeAnimationCount; --i > index;)
			cache->keyframeAnimations[i] = cache->keyframeAnimations[i - 1];

		dsKeyframeAnimationRef* keyframeRef = cache->keyframeAnimations + index;
		keyframeRef->animation = animation;
		keyframeRef->refCount = 1;
	}
	else
		++prevKeyframeRef->refCount;

	DS_VERIFY(dsReadWriteSpinlock_unlockWrite(&cache->lock));
	return true;

}

bool dsAnimationNodeMapCache_removeKeyframeAnimation(dsAnimationNodeMapCache* cache,
	const dsKeyframeAnimation* animation)
{
	DS_ASSERT(cache);
	DS_ASSERT(animation);

	DS_VERIFY(dsReadWriteSpinlock_lockWrite(&cache->lock));

	bool found;
	dsKeyframeAnimationRef* keyframeRef = (dsKeyframeAnimationRef*)dsBinarySearch(animation,
		cache->keyframeAnimations, cache->keyframeAnimationCount, sizeof(dsKeyframeAnimationRef),
		keyframeAnimationRefCompare, NULL);
	if (keyframeRef)
	{
		found = true;
		if (--keyframeRef->refCount == 0)
		{
			size_t index = keyframeRef - cache->keyframeAnimations;
			--cache->keyframeAnimationCount;

			for (uint32_t i = 0; i < cache->treeMapCount; ++i)
			{
				dsAnimationTreeNodeMap* treeMap = cache->treeMaps + i;
				dsKeyframeAnimationNodeMap_destroy(treeMap->keyframeMaps[index]);
				for (size_t j = index; j < cache->keyframeAnimationCount; ++j)
					treeMap->keyframeMaps[j] = treeMap->keyframeMaps[j + 1];
			}

			for (size_t i = index; i < cache->keyframeAnimationCount; ++i)
				cache->keyframeAnimations[i] = cache->keyframeAnimations[i + 1];
		}
	}
	else
	{
		errno = ENOTFOUND;
		found = false;
	}

	DS_VERIFY(dsReadWriteSpinlock_unlockWrite(&cache->lock));
	return found;
}

bool dsAnimationNodeMapCache_addDirectAnimation(dsAnimationNodeMapCache* cache,
	const dsDirectAnimation* animation)
{
	DS_ASSERT(cache);
	DS_ASSERT(animation);

	DS_VERIFY(dsReadWriteSpinlock_lockWrite(&cache->lock));

	dsDirectAnimationRef* prevDirectRef = (dsDirectAnimationRef*)dsBinarySearchLowerBound(
		animation, cache->directAnimations, cache->directAnimationCount,
		sizeof(dsDirectAnimationRef), directAnimationRefCompare, NULL);
	if (!prevDirectRef || prevDirectRef->animation != animation)
	{
		size_t index = prevDirectRef ? prevDirectRef - cache->directAnimations :
			cache->directAnimationCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(cache->allocator, cache->directAnimations,
				cache->directAnimationCount, cache->maxDirectAnimations, 1))
		{
			DS_VERIFY(dsReadWriteSpinlock_unlockWrite(&cache->lock));
			return false;
		}

		for (uint32_t i = 0; i < cache->treeMapCount; ++i)
		{
			dsAnimationTreeNodeMap* treeMap = cache->treeMaps + i;
			dsDirectAnimationNodeMap* map = dsDirectAnimationNodeMap_create(cache->allocator,
				animation, treeMap->tree);
			uint32_t dummyCount = cache->directAnimationCount - 1;
			if (map && !DS_RESIZEABLE_ARRAY_ADD(cache->allocator, treeMap->directMaps,
					dummyCount, treeMap->maxDirectMaps, 1))
			{
				dsDirectAnimationNodeMap_destroy(map);
				map = NULL;
			}

			if (map)
			{
				// Need to shift the entries to keep them sorted.
				for (uint32_t j = cache->directAnimationCount; --j > index;)
					treeMap->directMaps[j] = treeMap->directMaps[j - 1];
				treeMap->directMaps[index] = map;
			}
			else
			{
				// Failed to add an entry, restore the previous state.
				--cache->directAnimationCount;

				for (uint32_t j = 0; j < i; ++j)
				{
					dsAnimationTreeNodeMap* restoreTreeMap = cache->treeMaps + j;
					dsDirectAnimationNodeMap_destroy(restoreTreeMap->directMaps[index]);

					// Need to shift the entries back into place.
					for (size_t k = index; k < cache->directAnimationCount; ++k)
						cache->treeMaps[k] = cache->treeMaps[k + 1];
				}

				DS_VERIFY(dsReadWriteSpinlock_unlockWrite(&cache->lock));
				return false;
			}
		}

		// Added all the direct animation maps, insert the final animation here. Need to shift the
		// entries to keep them sorted.
		for (uint32_t i = cache->directAnimationCount; --i > index;)
			cache->directAnimations[i] = cache->directAnimations[i - 1];

		dsDirectAnimationRef* directRef = cache->directAnimations + index;
		directRef->animation = animation;
		directRef->refCount = 1;
	}
	else
		++prevDirectRef->refCount;

	DS_VERIFY(dsReadWriteSpinlock_unlockWrite(&cache->lock));
	return true;
}

bool dsAnimationNodeMapCache_removeDirectAnimation(dsAnimationNodeMapCache* cache,
	const dsDirectAnimation* animation)
{
	DS_ASSERT(cache);
	DS_ASSERT(animation);

	DS_VERIFY(dsReadWriteSpinlock_lockWrite(&cache->lock));

	bool found;
	dsDirectAnimationRef* directRef = (dsDirectAnimationRef*)dsBinarySearch(animation,
		cache->directAnimations, cache->directAnimationCount, sizeof(dsDirectAnimationRef),
		directAnimationRefCompare, NULL);
	if (directRef)
	{
		found = true;
		if (--directRef->refCount == 0)
		{
			size_t index = directRef - cache->directAnimations;
			--cache->directAnimationCount;

			for (uint32_t i = 0; i < cache->treeMapCount; ++i)
			{
				dsAnimationTreeNodeMap* treeMap = cache->treeMaps + i;
				dsDirectAnimationNodeMap_destroy(treeMap->directMaps[index]);
				for (size_t j = index; j < cache->directAnimationCount; ++j)
					treeMap->directMaps[j] = treeMap->directMaps[j + 1];
			}

			for (size_t i = index; i < cache->directAnimationCount; ++i)
				cache->directAnimations[i] = cache->directAnimations[i + 1];
		}
	}
	else
	{
		errno = ENOTFOUND;
		found = false;
	}

	DS_VERIFY(dsReadWriteSpinlock_unlockWrite(&cache->lock));
	return found;
}

bool dsAnimationNodeMapCache_applyAnimation(dsAnimationNodeMapCache* cache,
	const dsAnimation* animation, dsAnimationTree* tree)
{
	DS_ASSERT(cache);
	DS_ASSERT(animation);
	DS_ASSERT(tree);

	DS_VERIFY(dsReadWriteSpinlock_lockRead(&cache->lock));

	const dsAnimationTreeNodeMap* treeNodeMap = (const dsAnimationTreeNodeMap*)dsBinarySearch(
		&tree->id, cache->treeMaps, cache->treeMapCount, sizeof(dsAnimationTreeNodeMap),
		animationTreeNodeMapCompare, NULL);
	if (!treeNodeMap)
	{
		errno = ENOTFOUND;
		DS_VERIFY(dsReadWriteSpinlock_unlockRead(&cache->lock));
		return true;
	}

	// Expect we don't have 100s of thousands of nodes.
	WeightedTransform* transforms =
		DS_ALLOCATE_STACK_OBJECT_ARRAY(WeightedTransform, tree->nodeCount);
	memset(transforms, 0, sizeof(WeightedTransform)*tree->nodeCount);

	// Apply the animation channels to the transforms.
	applyKeyframeAnimationTransforms(transforms, treeNodeMap->keyframeMaps,
		cache->keyframeAnimationCount, animation, tree);
	applyDirectAnimationTransforms(transforms, treeNodeMap->directMaps,
		cache->directAnimationCount, animation, tree);

	// Set the final non-zero weight transform values on the animation.
	for (uint32_t i = 0; i < tree->nodeCount; ++i)
	{
		dsAnimationNode* node = tree->nodes + i;
		WeightedTransform* transform = transforms + i;
		if (transform->totalTranslationWeight > 0)
		{
			dsVector4f_scale(&transform->translation, &transform->translation,
				1/transform->totalTranslationWeight);
			node->translation = *(dsVector3f*)&transform->translation;
		}

		if (transform->totalRotationWeight > 0)
		{
			// With normalization no need to scale based on the total weight.
			dsQuaternion4f_normalize(&node->rotation, (const dsQuaternion4f*)&transform->rotation);
		}

		if (transform->totalScaleWeight > 0)
		{
			dsVector4f_scale(&transform->scale, &transform->scale,
				1/transform->totalScaleWeight);
			node->scale = *(dsVector3f*)&transform->scale;
		}
	}

	DS_VERIFY(dsReadWriteSpinlock_unlockRead(&cache->lock));
	return true;
}

void dsAnimationNodeMapCache_destroy(dsAnimationNodeMapCache* cache)
{
	if (!cache)
		return;

	DS_VERIFY(dsAllocator_free(cache->allocator, cache->keyframeAnimations));
	DS_VERIFY(dsAllocator_free(cache->allocator, cache->directAnimations));

	for (uint32_t i = 0; i < cache->treeMapCount; ++i)
		freeAnimationTreeNodeMap(cache, cache->treeMaps + i);
	DS_VERIFY(dsAllocator_free(cache->allocator, cache->treeMaps));
	dsReadWriteSpinlock_shutdown(&cache->lock);

	DS_VERIFY(dsAllocator_free(cache->allocator, cache));
}

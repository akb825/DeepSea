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

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Core.h>
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
	// NOTE: If there's regularly many keyframes can use a binary search. Expected to be fairly
	// on average, so a linear search should be fine and in many cases faster.
	for (uint32_t i = 0; i < keyframeCount; ++i)
	{
		if (keyframeTimes[i] > time)
			return i;
	}

	return keyframeCount - 1;
}

static void evaluateCubicSpline(dsVector4f* result, const dsVector4f* vi, const dsVector4f* bi,
	const dsVector4f* vj, const dsVector4f* aj, float t)
{
	float t2 = t*t;
	float t3 = t2*t;

	float viMul = 2*t3 - 3*t2 + 1;
	float biMul = t3 - 2*t2 + t;
	float vjMul = -2*t3 + 3*t2;
	float ajMul = t3 - t2;

#if DS_SIMD_ALWAYS_FMA
	result->simd = dsSIMD4f_mul(vi->simd, dsSIMD4f_set1(viMul));
	result->simd = dsSIMD4f_fmadd(bi->simd, dsSIMD4f_set1(biMul), result->simd);
	result->simd = dsSIMD4f_fmadd(vj->simd, dsSIMD4f_set1(vjMul), result->simd);
	result->simd = dsSIMD4f_fmadd(aj->simd, dsSIMD4f_set1(ajMul), result->simd);
#else
	dsVector4f viScaled, biScaled, vjScaled, ajScaled;
	dsVector4f_scale(&viScaled, vi, viMul);
	dsVector4f_scale(&biScaled, bi, biMul);
	dsVector4f_scale(&vjScaled, vj, vjMul);
	dsVector4f_scale(&ajScaled, aj, ajMul);

	dsVector4f_add(result, &viScaled, &biScaled);
	dsVector4f_add(result, result, &vjScaled);
	dsVector4f_add(result, result, &ajScaled);
#endif
}

static void applyKeyframeAnimationTransforms(WeightedTransform* transforms,
	const dsAnimation* animation, const dsAnimationTree* tree)
{
	for (uint32_t i = 0; i < animation->keyframeEntryCount; ++i)
	{
		const dsKeyframeAnimationEntry* entry = animation->keyframeEntries + i;
		if (entry->weight <= 0)
			continue;

		const dsKeyframeAnimation* keyframeAnimation = entry->animation;
		const dsKeyframeAnimationNodeMap* map = entry->map;
		DS_ASSERT(keyframeAnimation->keyframesCount == map->keyframesCount);
		for (uint32_t j = 0; j < keyframeAnimation->keyframesCount; ++i)
		{
			const dsAnimationKeyframes* keyframes = keyframeAnimation->keyframes + i;
			const dsAnimationKeyframesNodeMap* keyframesMap = map->keyframesMaps + j;
			DS_ASSERT(keyframes->channelCount == keyframesMap->channelCount);

			// Find wich pair of keyframes to interpolate between.
			uint32_t startKeyframe, endKeyframe;
			float t;
			if (entry->time <= keyframes->keyframeTimes[0])
			{
				startKeyframe = endKeyframe = 0;
				t = 0;
			}
			else if (entry->time >= keyframes->keyframeTimes[keyframes->keyframeCount - 1])
			{
				startKeyframe = endKeyframe = keyframes->keyframeCount - 1;
				t = 0;
			}
			else
			{
				endKeyframe = findEndKeyframe(keyframes->keyframeTimes, keyframes->keyframeCount,
					entry->time);
				startKeyframe = endKeyframe - 1;
				float startTime = keyframes->keyframeTimes[startKeyframe];
				float endTime = keyframes->keyframeTimes[endKeyframe];
				t = (float)(entry->time - startTime)/(endTime - startTime);
			}

			for (uint32_t k = 0; k < keyframes->channelCount; ++k)
			{
				const dsKeyframeAnimationChannel* channel = keyframes->channels + k;
				WeightedTransform* transform = transforms + keyframesMap->channelNodes[k];
				dsVector4f value;
				switch (channel->interpolation)
				{
					case dsAnimationInterpolation_Step:
						value = channel->values[startKeyframe];
						break;
					case dsAnimationInterpolation_Linear:
						dsVector4f_lerp(&value, channel->values + startKeyframe,
							channel->values + endKeyframe, t);
						break;
					case dsAnimationInterpolation_Cubic:
					{
						uint32_t startValueIndex = startKeyframe*3;
						uint32_t endValueIndex = endKeyframe*3;
						evaluateCubicSpline(&value, channel->values + startValueIndex + 1,
							channel->values + startValueIndex + 2,
							channel->values + endValueIndex + 1, channel->values + endValueIndex,
							t);
						break;
					}
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
	const dsAnimation* animation, const dsAnimationTree* tree)
{
	for (uint32_t i = 0; i < animation->directEntryCount; ++i)
	{
		const dsDirectAnimationEntry* entry = animation->directEntries + i;
		if (entry->weight <= 0)
			continue;

		const dsDirectAnimation* directAnimation = entry->animation;
		const dsDirectAnimationNodeMap* map = entry->map;
		DS_ASSERT(directAnimation->channelCount == map->channelCount);
		for (uint32_t j = 0; j < directAnimation->channelCount; ++i)
		{
			const dsDirectAnimationChannel* channel = directAnimation->channels + j;
			WeightedTransform* transform = transforms + map->channelNodes[j];

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

dsAnimation* dsAnimation_create(dsAllocator* allocator, uint32_t treeID)
{
	if (!allocator)
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
	animation->treeID = treeID;
	animation->keyframeEntries = NULL;
	animation->keyframeEntryCount = 0;
	animation->maxKeyframeEntries = 0;
	animation->directEntries = NULL;
	animation->directEntryCount = 0;
	animation->maxDirectEntries = 0;

	return animation;
}

bool dsAnimation_addKeyframeAnimation(dsAnimation* animation,
	const dsKeyframeAnimation* keyframeAnimation, const dsKeyframeAnimationNodeMap* map,
	float weight, double time, double timeScale, bool wrap)
{
	if (!animation || !keyframeAnimation || !map)
	{
		errno = EINVAL;
		return false;
	}

	if (map->animation != keyframeAnimation || map->treeID != animation->treeID)
	{
		errno = EPERM;
		return false;
	}

	for (uint32_t i = 0; i < animation->keyframeEntryCount; ++i)
	{
		if (animation->keyframeEntries[i].animation == keyframeAnimation)
		{
			errno = EPERM;
			return false;
		}
	}

	uint32_t index = animation->keyframeEntryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(animation->allocator, animation->keyframeEntries,
			animation->keyframeEntryCount, animation->maxKeyframeEntries, 1))
	{
		return false;
	}

	dsKeyframeAnimationEntry* entry = animation->keyframeEntries + index;
	entry->animation = keyframeAnimation;
	entry->map = map;
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

	for (uint32_t i = 0; i < animation->keyframeEntryCount; ++i)
	{
		dsKeyframeAnimationEntry* entry = animation->keyframeEntries + i;
		if (entry->animation == keyframeAnimation)
			return entry;
	}

	return NULL;
}

bool dsAnimation_removeKeyframeAnimation(dsAnimation* animation,
	const dsKeyframeAnimation* keyframeAnimation)
{
	if (!animation || !keyframeAnimation)
		return false;

	for (uint32_t i = 0; i < animation->keyframeEntryCount; ++i)
	{
		dsKeyframeAnimationEntry* entry = animation->keyframeEntries + i;
		if (entry->animation != keyframeAnimation)
			continue;

		// Constant-time removal since order doesn't matter.
		*entry = animation->keyframeEntries[--animation->keyframeEntryCount];
		return true;
	}

	return false;
}

bool dsAnimation_addDirectAnimation(dsAnimation* animation,
	const dsDirectAnimation* directAnimation, const dsDirectAnimationNodeMap* map, float weight)
{
	if (!animation || !directAnimation || !map || map->treeID != animation->treeID)
	{
		errno = EINVAL;
		return false;
	}

	if (map->animation != directAnimation)
	{
		errno = EPERM;
		return false;
	}

	for (uint32_t i = 0; i < animation->directEntryCount; ++i)
	{
		if (animation->directEntries[i].animation == directAnimation)
		{
			errno = EPERM;
			return false;
		}
	}

	uint32_t index = animation->directEntryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(animation->allocator, animation->directEntries,
			animation->directEntryCount, animation->maxDirectEntries, 1))
	{
		return false;
	}

	dsDirectAnimationEntry* entry = animation->directEntries + index;
	entry->animation = directAnimation;
	entry->map = map;
	entry->weight = weight;
	return true;
}

dsDirectAnimationEntry* dsAnimation_findDirectAnimationEntry(dsAnimation* animation,
	const dsDirectAnimation* directAnimation)
{
	if (!animation || !directAnimation)
		return NULL;

	for (uint32_t i = 0; i < animation->directEntryCount; ++i)
	{
		dsDirectAnimationEntry* entry = animation->directEntries + i;
		if (entry->animation == directAnimation)
			return entry;
	}

	return NULL;
}

bool dsAnimation_removeDirectAnimation(dsAnimation* animation,
	const dsDirectAnimation* directAnimation)
{
	if (!animation || !directAnimation)
		return false;

	for (uint32_t i = 0; i < animation->directEntryCount; ++i)
	{
		dsDirectAnimationEntry* entry = animation->directEntries + i;
		if (entry->animation != directAnimation)
			continue;

		// Constant-time removal since order doesn't matter.
		*entry = animation->directEntries[--animation->directEntryCount];
		return true;
	}

	return false;
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

	// Expect we don't have 100s of thousands of nodes.
	WeightedTransform* transforms =
		DS_ALLOCATE_STACK_OBJECT_ARRAY(WeightedTransform, tree->nodeCount);
	memset(transforms, 0, sizeof(WeightedTransform)*tree->nodeCount);

	// Apply the animation channels to the transforms.
	applyKeyframeAnimationTransforms(transforms, animation, tree);
	applyDirectAnimationTransforms(transforms, animation, tree);

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
			dsVector4f_scale(&transform->rotation, &transform->rotation,
				1/transform->totalRotationWeight);
			dsQuaternion4f_normalize(&node->rotation, (const dsQuaternion4f*)&transform->rotation);
		}

		if (transform->totalScaleWeight > 0)
		{
			dsVector4f_scale(&transform->scale, &transform->scale,
				1/transform->totalScaleWeight);
			node->scale = *(dsVector3f*)&transform->scale;
		}
	}

	return true;
}

void dsAnimation_destroy(dsAnimation* animation)
{
	if (!animation)
		return;

	DS_VERIFY(dsAllocator_free(animation->allocator, animation->keyframeEntries));
	DS_VERIFY(dsAllocator_free(animation->allocator, animation->directEntries));
	DS_VERIFY(dsAllocator_free(animation->allocator, animation));
}

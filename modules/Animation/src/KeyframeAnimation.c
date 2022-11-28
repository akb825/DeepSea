/*
 * Copyright 2022 Aaron Barany
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

#include <DeepSea/Animation/KeyframeAnimation.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Core.h>

#include <float.h>
#include <string.h>

static uint32_t nextID;

dsKeyframeAnimation* dsKeyframeAnimation_create(dsAllocator* allocator,
	const dsAnimationKeyframes* keyframes, uint32_t keyframesCount)
{
	if (!allocator || !keyframes || keyframesCount == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsKeyframeAnimation)) +
		DS_ALIGNED_SIZE(sizeof(dsAnimationKeyframes)*keyframesCount);
	float minTime = FLT_MAX;
	float maxTime = -FLT_MAX;
	for (uint32_t i = 0; i < keyframesCount; ++i)
	{
		const dsAnimationKeyframes* curKeyframes = keyframes + i;
		if (curKeyframes->keyframeCount == 0 || curKeyframes->channelCount == 0 ||
			!curKeyframes->keyframeTimes || !curKeyframes->channels)
		{
			errno = EINVAL;
			return NULL;
		}

		for (uint32_t j = 0; j < curKeyframes->keyframeCount; ++j)
		{
			float curTime = curKeyframes->keyframeTimes[j];
			minTime = dsMin(minTime, curTime);
			maxTime = dsMax(maxTime, curTime);
			if (j > 0 && curTime <= curKeyframes->keyframeTimes[j - 1])
			{
				DS_LOG_ERROR(DS_ANIMATION_LOG_TAG,
					"Animation keyframe times must be strictly increasing.");
				errno = EINVAL;
				return NULL;
			}
		}

		fullSize += DS_ALIGNED_SIZE(sizeof(float)*curKeyframes->keyframeCount) +
			DS_ALIGNED_SIZE(sizeof(dsAnimationChannel)*curKeyframes->channelCount);

		for (uint32_t j = 0; j < curKeyframes->channelCount; ++j)
		{
			const dsAnimationChannel* curChannel = curKeyframes->channels + j;
			uint32_t expectedValueCount = curKeyframes->keyframeCount;
			switch (curChannel->component)
			{
				case dsAnimationComponent_Translation:
				case dsAnimationComponent_Scale:
					expectedValueCount *= 3;
					break;
				case dsAnimationComponent_Rotation:
					expectedValueCount *= 4;
					break;
				default:
					errno = EINVAL;
					return NULL;
			}

			switch (curChannel->interpolation)
			{
				case dsAnimationInterpolation_Step:
				case dsAnimationInterpolation_Linear:
					break;
				case dsAnimationInterpolation_Cubic:
					expectedValueCount *= 3;
					break;
				default:
					errno = EINVAL;
					return NULL;
			}

			if (!curChannel->node || curChannel->valueCount != expectedValueCount ||
				!curChannel->values)
			{
				errno = EINVAL;
				return NULL;
			}

			fullSize += DS_ALIGNED_SIZE(strlen(curChannel->node) + 1) +
				DS_ALIGNED_SIZE(sizeof(float)*expectedValueCount);
		}
	}

	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsKeyframeAnimation* animation = DS_ALLOCATE_OBJECT(&bufferAlloc, dsKeyframeAnimation);
	DS_ASSERT(animation);

	animation->allocator = dsAllocator_keepPointer(allocator);
	animation->id = DS_ATOMIC_FETCH_ADD32(&nextID, 1);
	animation->minTime = minTime;
	animation->maxTime = maxTime;
	animation->keyframesCount = keyframesCount;

	dsAnimationKeyframes* animationKeyframes = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc,
		dsAnimationKeyframes, keyframesCount);
	DS_ASSERT(animationKeyframes);
	for (uint32_t i = 0; i < keyframesCount; ++i)
	{
		const dsAnimationKeyframes* fromKeyframes = keyframes + i;
		dsAnimationKeyframes* toKeyframes = animationKeyframes + i;
		toKeyframes->keyframeCount = fromKeyframes->keyframeCount;
		toKeyframes->channelCount = fromKeyframes->channelCount;

		float* keyframeTimes = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, float,
			fromKeyframes->keyframeCount);
		DS_ASSERT(keyframeTimes);
		memcpy(keyframeTimes, fromKeyframes->keyframeTimes,
			sizeof(float)*fromKeyframes->keyframeCount);
		toKeyframes->keyframeTimes = keyframeTimes;

		dsAnimationChannel* channels = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsAnimationChannel,
			fromKeyframes->channelCount);
		DS_ASSERT(channels);
		for (uint32_t j = 0; j < fromKeyframes->channelCount; ++j)
		{
			const dsAnimationChannel* fromChannel = fromKeyframes->channels + j;
			dsAnimationChannel* toChannel = channels + j;

			size_t nodeLen = strlen(fromChannel->node) + 1;
			char* node = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nodeLen);
			DS_ASSERT(node);
			memcpy(node, fromChannel->node, nodeLen);
			toChannel->node = node;

			toChannel->component = fromChannel->component;
			toChannel->interpolation = fromChannel->interpolation;
			toChannel->valueCount = fromChannel->valueCount;

			float* values = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, float, fromChannel->valueCount);
			DS_ASSERT(values);
			memcpy(values, fromChannel->values, sizeof(float)*fromChannel->valueCount);
			toChannel->values = values;
		}
		toKeyframes->channels = channels;
	}
	animation->keyframes = animationKeyframes;

	return animation;
}

void dsKeyframeAnimation_destroy(dsKeyframeAnimation* animation)
{
	if (animation && animation->allocator)
		DS_VERIFY(dsAllocator_free(animation->allocator, animation));
}

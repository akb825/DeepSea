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

#include "KeyframeAnimationLoad.h"

#include <DeepSea/Animation/KeyframeAnimation.h>

#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Types.h>

#include <cstring>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/KeyframeAnimation_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

// 128 KB max stack usage through alloca.
#define DS_MAX_STACK_KEYFRAMES_AND_CHANNELS 1024
#define DS_MAX_STACK_VALUES 6144

dsKeyframeAnimation* dsKeyframeAnimation_loadImpl(dsAllocator* allocator, dsAllocator* scratchAllocator,
	const void* data, size_t size, const char* name)
{
	DS_ASSERT(scratchAllocator);
	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), size);
	if (!DeepSeaAnimation::VerifyKeyframeAnimationBuffer(verifier))
	{
		errno = EFORMAT;
		if (name)
		{
			DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG,
				"Invalid keyframe animation flatbuffer format for '%s'.", name);
		}
		else
			DS_LOG_ERROR(DS_ANIMATION_LOG_TAG, "Invalid keyframe animation flatbuffer format.");
		return nullptr;
	}

	auto fbKeyframeAnimation = DeepSeaAnimation::GetKeyframeAnimation(data);
	auto fbKeyframes = fbKeyframeAnimation->keyframes();
	uint32_t keyframesCount = fbKeyframes->size();
	if (keyframesCount == 0)
	{
		errno = EFORMAT;
		if (name)
		{
			DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG,
				"Keyframe animation must have non-empty keyframes for '%s'.", name);
		}
		else
			DS_LOG_ERROR(DS_ANIMATION_LOG_TAG, "Keyframe animation must have non-empty keyframes.");
		return nullptr;
	}

	uint32_t totalChannelCount = 0;
	uint32_t totalValueCount = 0;
	for (auto curFbKeyframes : *fbKeyframes)
	{
		if (!curFbKeyframes)
		{
			errno = EFORMAT;
			if (name)
			{
				DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG,
					"Keyframe not set in keyframe animation for '%s'.", name);
			}
			else
				DS_LOG_ERROR(DS_ANIMATION_LOG_TAG, "Keyframe not set in keyframe animation.");
			return nullptr;
		}

		uint32_t keyframeCount = curFbKeyframes->keyframeTimes()->size();
		if (keyframeCount == 0)
		{
			errno = EFORMAT;
			if (name)
			{
				DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG,
					"Keyframe animation must have non-empty keyframes times for '%s'.", name);
			}
			else
			{
				DS_LOG_ERROR(DS_ANIMATION_LOG_TAG,
					"Keyframe animation must have non-empty keyframe times.");
			}
			return nullptr;
		}

		auto fbChannels = curFbKeyframes->channels();
		uint32_t channelCount = fbChannels->size();
		if (channelCount == 0)
		{
			errno = EFORMAT;
			if (name)
			{
				DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG,
					"Keyframe animation must have non-empty keyframes channels for '%s'.", name);
			}
			else
			{
				DS_LOG_ERROR(DS_ANIMATION_LOG_TAG,
					"Keyframe animation must have non-empty keyframe channels.");
			}
			return nullptr;
		}
		totalChannelCount += channelCount;

		for (auto fbChannel : *fbChannels)
		{
			if (!fbChannel)
			{
				errno = EFORMAT;
				if (name)
				{
					DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG,
						"Channel not set in keyframe animation for '%s'.", name);
				}
				else
					DS_LOG_ERROR(DS_ANIMATION_LOG_TAG, "Channel not set in keyframe animation.");
				return nullptr;
			}

			uint32_t expectedValueCount = keyframeCount;
			if (fbChannel->interpolation() == DeepSeaAnimation::AnimationInterpolation::Cubic)
				expectedValueCount *= 3;
			if (fbChannel->values()->size() != expectedValueCount)
			{
				errno = EFORMAT;
				if (name)
				{
					DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG,
						"Unexpected channel value count in keyframe animation for '%s'.", name);
				}
				else
				{
					DS_LOG_ERROR(DS_ANIMATION_LOG_TAG,
						"Unexpected channel value count in keyframe animation.");
				}
				return nullptr;
			}

			totalValueCount += expectedValueCount;
		}
	}

	bool heapKeyframes;
	uint32_t stackCount;
	dsAnimationKeyframes* keyframes;
	if (keyframesCount <= DS_MAX_STACK_KEYFRAMES_AND_CHANNELS)
	{
		heapKeyframes = false;
		stackCount = keyframesCount;
		keyframes = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsAnimationKeyframes, keyframesCount);
	}
	else
	{
		heapKeyframes = scratchAllocator->freeFunc != nullptr;
		keyframes =
			DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, dsAnimationKeyframes, keyframesCount);
		stackCount = 0;
		if (!keyframes)
			return nullptr;
	}

	bool heapChannels;
	dsKeyframeAnimationChannel* channels;
	if (stackCount + totalChannelCount <= DS_MAX_STACK_KEYFRAMES_AND_CHANNELS)
	{
		heapChannels = false;
		channels = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsKeyframeAnimationChannel, totalChannelCount);
	}
	else
	{
		heapChannels = scratchAllocator->freeFunc != nullptr;
		channels = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, dsKeyframeAnimationChannel,
			totalChannelCount);
		if (!channels)
		{
			if (heapKeyframes)
				DS_VERIFY(dsAllocator_free(scratchAllocator, keyframes));
			return nullptr;
		}
	}

	bool heapValues;
	dsVector4f* values;
	if (totalValueCount <= DS_MAX_STACK_VALUES)
	{
		heapValues = false;
		values = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsVector4f, totalValueCount);
	}
	else
	{
		heapValues = scratchAllocator->freeFunc != nullptr;
		values = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, dsVector4f, totalValueCount);
		if (!values)
		{
			if (heapKeyframes)
				DS_VERIFY(dsAllocator_free(scratchAllocator, keyframes));
			if (heapChannels)
				DS_VERIFY(dsAllocator_free(scratchAllocator, channels));
			return nullptr;
		}
	}

	dsKeyframeAnimationChannel* nextChannels = channels;
	uint32_t valueOffset = 0;
	for (uint32_t i = 0; i < keyframesCount; ++i)
	{
		auto curFbKeyframes = (*fbKeyframes)[i];
		dsAnimationKeyframes* curKeyframes = keyframes + i;

		auto fbKeyframeTimes = curFbKeyframes->keyframeTimes();
		curKeyframes->keyframeCount = fbKeyframeTimes->size();
		curKeyframes->keyframeTimes = fbKeyframeTimes->data();

		auto fbChannels = curFbKeyframes->channels();
		uint32_t channelCount = fbChannels->size();
		for (uint32_t j = 0; j < channelCount; ++j)
		{
			auto fbChannel = (*fbChannels)[j];
			dsKeyframeAnimationChannel* channel = nextChannels + j;
			channel->node = fbChannel->node()->c_str();
			channel->component = static_cast<dsAnimationComponent>(fbChannel->component());
			channel->interpolation =
				static_cast<dsAnimationInterpolation>(fbChannel->interpolation());

			auto fbValues = fbChannel->values();
			channel->valueCount = fbValues->size();
			// memcpy to avoid unaligned access when using SIMD.
			for (uint32_t k = 0; k < channel->valueCount; ++k)
				memcpy(values + valueOffset + k, (*fbValues)[k], sizeof(dsVector4f));
			channel->values = values + valueOffset;
			valueOffset += channel->valueCount;
		}

		curKeyframes->channelCount = channelCount;
		curKeyframes->channels = nextChannels;
		nextChannels += channelCount;
	}
	DS_ASSERT(nextChannels == channels + totalChannelCount);
	DS_ASSERT(valueOffset == totalValueCount);

	dsKeyframeAnimation* animation =
		dsKeyframeAnimation_create(allocator, keyframes, keyframesCount);
	if (heapKeyframes)
		DS_VERIFY(dsAllocator_free(scratchAllocator, keyframes));
	if (heapChannels)
		DS_VERIFY(dsAllocator_free(scratchAllocator, channels));
	if (heapValues)
		DS_VERIFY(dsAllocator_free(scratchAllocator, values));
	return animation;
}

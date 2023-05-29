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

#include "DirectAnimationLoad.h"

#include <DeepSea/Animation/DirectAnimation.h>

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

#include "Flatbuffers/DirectAnimation_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

#define DS_MAX_STACK_CHANNELS 1024

dsDirectAnimation* dsDirectAnimation_loadImpl(dsAllocator* allocator, dsAllocator* scratchAllocator,
	const void* data, size_t size, const char* name)
{
	DS_ASSERT(scratchAllocator);
	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), size);
	if (!DeepSeaAnimation::VerifyDirectAnimationBuffer(verifier))
	{
		errno = EFORMAT;
		if (name)
		{
			DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG,
				"Invalid direct animation flatbuffer format for '%s'.", name);
		}
		else
			DS_LOG_ERROR(DS_ANIMATION_LOG_TAG, "Invalid direct animation flatbuffer format.");
		return nullptr;
	}

	auto fbDirectAnimation = DeepSeaAnimation::GetDirectAnimation(data);
	auto fbChannels = fbDirectAnimation->channels();
	uint32_t channelCount = fbChannels->size();
	if (channelCount == 0)
	{
		errno = EFORMAT;
		if (name)
		{
			DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG,
				"Direct animation must have non-empty channels for '%s'.", name);
		}
		else
			DS_LOG_ERROR(DS_ANIMATION_LOG_TAG, "Direct animation must have non-empty channels.");
		return nullptr;
	}

	bool heapChannels;
	dsDirectAnimationChannel* channels;
	if (channelCount <= DS_MAX_STACK_CHANNELS)
	{
		heapChannels = false;
		channels = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsDirectAnimationChannel, channelCount);
	}
	else
	{
		heapChannels = scratchAllocator->freeFunc != nullptr;
		channels =
			DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, dsDirectAnimationChannel, channelCount);
		if (!channels)
			return nullptr;
	}

	for (uint32_t i = 0; i < channelCount; ++i)
	{
		auto fbChannel = (*fbChannels)[i];
		if (!fbChannel)
		{
			errno = EFORMAT;
			if (name)
			{
				DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG,
					"Channel not set in direct animation for '%s'.", name);
			}
			else
				DS_LOG_ERROR(DS_ANIMATION_LOG_TAG, "Channel not set in direct animation.");

			if (heapChannels)
				DS_VERIFY(dsAllocator_free(scratchAllocator, channels));
			return nullptr;
		}

		dsDirectAnimationChannel* channel = channels + i;
		channel->node = fbChannel->node()->c_str();
		channel->component = static_cast<dsAnimationComponent>(fbChannel->component());
		std::memcpy(&channel->value, fbChannel->value(), sizeof(dsVector4f));
	}

	dsDirectAnimation* animation = dsDirectAnimation_create(allocator, channels, channelCount);
	if (heapChannels)
		DS_VERIFY(dsAllocator_free(scratchAllocator, channels));
	return animation;
}

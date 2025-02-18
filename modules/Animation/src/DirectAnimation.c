/*
 * Copyright 2022-2025 Aaron Barany
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

#include <DeepSea/Animation/DirectAnimation.h>

#include "DirectAnimationLoad.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Streams/FileArchive.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <string.h>

dsDirectAnimation* dsDirectAnimation_create(dsAllocator* allocator,
	const dsDirectAnimationChannel* channels, uint32_t channelCount)
{
	if (!allocator || !channels || channelCount == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsDirectAnimation)) +
		DS_ALIGNED_SIZE(sizeof(dsDirectAnimationChannel)*channelCount);
	for (uint32_t i = 0; i < channelCount; ++i)
	{
		const dsDirectAnimationChannel* curChannel = channels + i;
		if (!curChannel->node)
		{
			errno = EINVAL;
			return NULL;
		}

		fullSize += DS_ALIGNED_SIZE(strlen(curChannel->node) + 1);
	}

	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsDirectAnimation* animation = DS_ALLOCATE_OBJECT(&bufferAlloc, dsDirectAnimation);
	DS_ASSERT(animation);

	animation->allocator = dsAllocator_keepPointer(allocator);
	animation->channelCount = channelCount;

	dsDirectAnimationChannel* animationChannels = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc,
		dsDirectAnimationChannel, channelCount);
	DS_ASSERT(animationChannels);
	for (uint32_t i = 0; i < channelCount; ++i)
	{
		const dsDirectAnimationChannel* fromChannel = channels + i;
		dsDirectAnimationChannel* toChannel = animationChannels + i;

		size_t nodeLen = strlen(fromChannel->node) + 1;
		char* node = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nodeLen);
		DS_ASSERT(node);
		memcpy(node, fromChannel->node, nodeLen);
		toChannel->node = node;

		toChannel->component = fromChannel->component;
		toChannel->value = fromChannel->value;
	}
	animation->channels = animationChannels;

	return animation;
}

dsDirectAnimation* dsDirectAnimation_loadFile(dsAllocator* allocator, dsAllocator* scratchAllocator,
	const char* filePath)
{
	if (!allocator || !filePath)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!scratchAllocator)
		scratchAllocator = allocator;

	dsFileStream stream;
	if (!dsFileStream_openPath(&stream, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG, "Couldn't open direct animation file '%s'.", filePath);
		return NULL;
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, (dsStream*)&stream, scratchAllocator);
	dsFileStream_close(&stream);
	if (!buffer)
		return NULL;

	dsDirectAnimation* tree = dsDirectAnimation_loadImpl(allocator, scratchAllocator, buffer, size,
		filePath);
	DS_VERIFY(dsAllocator_free(scratchAllocator, buffer));
	return tree;
}

dsDirectAnimation* dsDirectAnimation_loadResource(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsFileResourceType type, const char* filePath)
{
	if (!allocator || !filePath)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!scratchAllocator)
		scratchAllocator = allocator;

	dsResourceStream stream;
	if (!dsResourceStream_open(&stream, type, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG, "Couldn't open direct animation file '%s'.", filePath);
		return NULL;
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, (dsStream*)&stream, scratchAllocator);
	dsResourceStream_close(&stream);
	if (!buffer)
		return NULL;

	dsDirectAnimation* tree = dsDirectAnimation_loadImpl(allocator, scratchAllocator, buffer, size,
		filePath);
	DS_VERIFY(dsAllocator_free(scratchAllocator, buffer));
	return tree;
}

dsDirectAnimation* dsDirectAnimation_loadArchive(dsAllocator* allocator,
	dsAllocator* scratchAllocator, const dsFileArchive* archive, const char* filePath)
{
	if (!allocator || !archive || !filePath)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!scratchAllocator)
		scratchAllocator = allocator;

	dsStream* stream = dsFileArchive_openFile(archive, filePath);
	if (!stream)
	{
		DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG, "Couldn't open direct animation file '%s'.", filePath);
		return NULL;
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, stream, scratchAllocator);
	dsStream_close(stream);
	if (!buffer)
		return NULL;

	dsDirectAnimation* tree = dsDirectAnimation_loadImpl(allocator, scratchAllocator, buffer, size,
		filePath);
	DS_VERIFY(dsAllocator_free(scratchAllocator, buffer));
	return tree;
}

dsDirectAnimation* dsDirectAnimation_loadStream(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsStream* stream)
{
	if (!allocator || !stream)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!scratchAllocator)
		scratchAllocator = allocator;

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, stream, scratchAllocator);
	if (!buffer)
		return NULL;

	dsDirectAnimation* tree = dsDirectAnimation_loadImpl(allocator, scratchAllocator, buffer, size,
		NULL);
	DS_VERIFY(dsAllocator_free(scratchAllocator, buffer));
	return tree;
}

dsDirectAnimation* dsDirectAnimation_loadData(dsAllocator* allocator, dsAllocator* scratchAllocator,
	const void* data, size_t size)
{
	if (!allocator || !data || size == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!scratchAllocator)
		scratchAllocator = allocator;

	return dsDirectAnimation_loadImpl(allocator, scratchAllocator, data, size, NULL);
}

void dsDirectAnimation_destroy(dsDirectAnimation* animation)
{
	if (animation && animation->allocator)
		DS_VERIFY(dsAllocator_free(animation->allocator, animation));
}

/*
 * Copyright 2016 Aaron Barany
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

#include <DeepSea/Core/Streams/Stream.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <string.h>

size_t dsStream_read(dsStream* stream, void* data, size_t size);


void* dsStream_readUntilEnd(size_t* outSize, dsStream* stream, dsAllocator* allocator)
{
	void* buffer = NULL;
	size_t capacity = 0;
	if (!dsStream_readUntilEndReuse(&buffer, outSize, &capacity, stream, allocator))
	{
		if (buffer && allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, buffer));
		return NULL;
	}

	return buffer;
}

bool dsStream_readUntilEndReuse(void** buffer, size_t* size, size_t* capacity, dsStream* stream,
	dsAllocator* allocator)
{
	if (!buffer || !size || !capacity || !stream || !allocator)
	{
		errno = EINVAL;
		return false;
	}

	if (stream->seekFunc && stream->tellFunc)
	{
		uint64_t position = dsStream_tell(stream);
		if (position == DS_STREAM_INVALID_POS || !dsStream_seek(stream, 0, dsStreamSeekWay_End))
			return false;

		uint64_t end = dsStream_tell(stream);
		if (end == DS_STREAM_INVALID_POS ||
			!dsStream_seek(stream, position, dsStreamSeekWay_Beginning))
		{
			return false;
		}

		*size = (size_t)(end - position);
		if (!*buffer || *size > *capacity)
		{
			void* newBuffer = (uint8_t*)dsAllocator_reallocWithFallback(allocator, *buffer,
				*capacity, *size);
			if (!newBuffer)
				return false;

			*buffer = newBuffer;
			*capacity = *size;
		}

		size_t read = dsStream_read(stream, *buffer, *size);
		if (read != *size)
		{
			errno = EIO;
			return false;
		}
	}
	else
	{
		if (!allocator->freeFunc)
		{
			errno = EINVAL;
			return false;
		}

		*size = 0;
		uint8_t tempBuffer[1024];
		do
		{
			size_t readSize = dsStream_read(stream, tempBuffer, sizeof(tempBuffer));
			if (readSize == 0)
				break;

			size_t newSize = *size + readSize;
			if (!*buffer || *size < *capacity)
			{
				size_t oldCapacity = *capacity;
				*capacity *= 2;
				if (*capacity < newSize)
					*capacity = newSize;

				uint8_t* newBuffer = (uint8_t*)dsAllocator_reallocWithFallback(allocator, *buffer,
					oldCapacity, *capacity);
				if (!newBuffer)
					return false;

				*buffer = newBuffer;
			}

			memcpy((uint8_t*)*buffer + *size, tempBuffer, readSize);
			*size = newSize;
		} while (true);
	}

	return true;
}

size_t dsStream_write(dsStream* stream, const void* data, size_t size);

bool dsStream_seek(dsStream* stream, int64_t offset, dsStreamSeekWay way);
uint64_t dsStream_tell(dsStream* stream);

uint64_t dsStream_skip(dsStream* stream, uint64_t size)
{
	if (size == 0)
		return 0;

	uint8_t buffer[1024];
	if (stream->seekFunc && size > sizeof(buffer))
	{
		if (!dsStream_seek(stream, size, dsStreamSeekWay_Current))
			return 0;
	}
	else
	{
		uint64_t totalSize = 0;
		while (totalSize < size)
		{
			uint64_t readSize = size - totalSize;
			if (readSize > sizeof(buffer))
				readSize = sizeof(buffer);

			uint64_t thisReadSize = dsStream_read(stream, buffer, (size_t)readSize);
			totalSize += thisReadSize;
			if (thisReadSize != readSize)
				return totalSize;
		}
	}

	return size;
}

void dsStream_flush(dsStream* stream);
bool dsStream_close(dsStream* stream);

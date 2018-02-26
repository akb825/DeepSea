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
#include <string.h>

size_t dsStream_read(dsStream* stream, void* data, size_t size);

void* dsStream_readUntilEnd(size_t* outSize, dsStream* stream, dsAllocator* allocator)
{
	if (!outSize || !stream || !allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	uint8_t* data;
	if (stream->seekFunc && stream->tellFunc)
	{
		uint64_t position = dsStream_tell(stream);
		if (position == DS_STREAM_INVALID_POS || !dsStream_seek(stream, 0, dsStreamSeekWay_End))
			return NULL;

		uint64_t end = dsStream_tell(stream);
		if (end == DS_STREAM_INVALID_POS ||
			!dsStream_seek(stream, position, dsStreamSeekWay_Beginning))
		{
			return NULL;
		}

		*outSize = (size_t)(end - position);
		data = (uint8_t*)dsAllocator_alloc(allocator, *outSize);
		if (!data)
			return NULL;

		size_t read = dsStream_read(stream, data, *outSize);
		if (read != *outSize)
		{
			dsAllocator_free(allocator, data);
			errno = EIO;
			return NULL;
		}
	}
	else
	{
		if (!allocator->freeFunc)
		{
			errno = EINVAL;
			return NULL;
		}

		*outSize = 0;
		data = NULL;
		size_t maxSize = 0;
		uint8_t buffer[1024];
		do
		{
			size_t readSize = dsStream_read(stream, buffer, sizeof(buffer));
			if (readSize == 0)
				break;

			size_t newSize = *outSize + readSize;
			if (maxSize < newSize)
			{
				maxSize *= 2;
				if (maxSize < newSize)
					maxSize = newSize;

				uint8_t* newData = (uint8_t*)dsAllocator_reallocWithFallback(allocator, data,
					*outSize, maxSize);
				if (!newData)
				{
					dsAllocator_free(allocator, data);
					return NULL;
				}
				data = newData;
			}

			memcpy(data + *outSize, buffer, readSize);
			*outSize = newSize;
		} while (true);
	}

	return data;
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

			uint64_t thisReadSize = dsStream_read(stream, buffer, readSize);
			totalSize += thisReadSize;
			if (thisReadSize != readSize)
				return totalSize;
		}
	}

	return size;
}

void dsStream_flush(dsStream* stream);
bool dsStream_close(dsStream* stream);

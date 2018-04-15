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

#include <DeepSea/Core/Streams/MemoryStream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <string.h>

bool dsMemoryStream_open(dsMemoryStream* stream, void* buffer, size_t size)
{
	if (!stream || !buffer)
	{
		errno = EINVAL;
		return false;
	}

	((dsStream*)stream)->readFunc = (dsStreamReadFunction)&dsMemoryStream_read;
	((dsStream*)stream)->writeFunc = (dsStreamWriteFunction)&dsMemoryStream_write;
	((dsStream*)stream)->seekFunc = (dsStreamSeekFunction)&dsMemoryStream_seek;
	((dsStream*)stream)->tellFunc = (dsStreamTellFunction)&dsMemoryStream_tell;
	((dsStream*)stream)->flushFunc = NULL;
	((dsStream*)stream)->closeFunc = (dsStreamCloseFunction)&dsMemoryStream_close;
	stream->buffer = buffer;
	stream->size = size;
	stream->position = 0;

	return true;
}

size_t dsMemoryStream_read(dsMemoryStream* stream, void* data, size_t size)
{
	if (!stream || !stream->buffer || stream->position >= stream->size || !data)
	{
		errno = EINVAL;
		return 0;
	}

	size_t remaining = stream->size - stream->position;
	if (size > remaining)
		size = remaining;

	memcpy(data, (uint8_t*)stream->buffer + stream->position, size);
	stream->position += size;
	DS_ASSERT(stream->position <= stream->size);
	return size;
}

size_t dsMemoryStream_write(dsMemoryStream* stream, const void* data, size_t size)
{
	if (!stream || !stream->buffer || stream->position >= stream->size || !data)
	{
		errno = EINVAL;
		return 0;
	}

	size_t remaining = stream->size - stream->position;
	if (size > remaining)
		size = remaining;

	memcpy((uint8_t*)stream->buffer + stream->position, data, size);
	stream->position += size;
	DS_ASSERT(stream->position <= stream->size);
	return size;
}

bool dsMemoryStream_seek(dsMemoryStream* stream, int64_t offset, dsStreamSeekWay way)
{
	if (!stream || !stream->buffer)
	{
		errno = EINVAL;
		return false;
	}

	size_t position;
	switch (way)
	{
		case dsStreamSeekWay_Beginning:
			position = (size_t)offset;
			break;
		case dsStreamSeekWay_Current:
			position = (size_t)((int64_t)stream->position + offset);
			break;
		case dsStreamSeekWay_End:
			position = (size_t)((int64_t)stream->size + offset);
			break;
		default:
			DS_ASSERT(false);
			errno = EINVAL;
			return false;
	}

	if (position > stream->size)
	{
		errno = EINVAL;
		return false;
	}

	stream->position = position;
	return true;
}

uint64_t dsMemoryStream_tell(dsMemoryStream* stream)
{
	if (!stream || !stream->buffer)
	{
		errno = EINVAL;
		return DS_STREAM_INVALID_POS;
	}

	return stream->position;
}

bool dsMemoryStream_close(dsMemoryStream* stream)
{
	if (!stream || !stream->buffer)
	{
		errno = EINVAL;
		return false;
	}

	stream->buffer = NULL;
	stream->size = 0;
	stream->position = 0;
	return true;
}

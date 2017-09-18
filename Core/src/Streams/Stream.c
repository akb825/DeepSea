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

size_t dsStream_read(dsStream* stream, void* data, size_t size);

size_t dsStream_skip(dsStream* stream, uint64_t size)
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

size_t dsStream_write(dsStream* stream, const void* data, size_t size);

bool dsStream_seek(dsStream* stream, int64_t offset, dsStreamSeekWay way);
uint64_t dsStream_tell(dsStream* stream);

void dsStream_flush(dsStream* stream);
bool dsStream_close(dsStream* stream);

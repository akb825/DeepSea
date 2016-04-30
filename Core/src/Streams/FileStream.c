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

#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Assert.h>

static void initFileStream(dsFileStream* stream, FILE* file)
{
	((dsStream*)stream)->readFunc = (dsStreamReadFunction)&dsFileStream_read;
	((dsStream*)stream)->writeFunc = (dsStreamWriteFunction)&dsFileStream_write;
	((dsStream*)stream)->seekFunc = (dsStreamSeekFunction)&dsFileStream_seek;
	((dsStream*)stream)->tellFunc = (dsStreamTellFunction)&dsFileStream_tell;
	((dsStream*)stream)->flushFunc = (dsStreamFlushFunction)&dsFileStream_flush;
	((dsStream*)stream)->closeFunc = (dsStreamCloseFunction)&dsFileStream_close;
	stream->file = file;
}

bool dsFileStream_openPath(dsFileStream* stream, const char* filePath,
	const char* mode)
{
	if (!stream || !filePath || !mode)
		return false;

	FILE* file = fopen(filePath, mode);
	if (!file)
		return false;

	initFileStream(stream, file);
	return true;
}

bool dsFileStream_openFile(dsFileStream* stream, FILE* file)
{
	if (!stream || !file)
		return false;

	initFileStream(stream, file);
	return true;
}

size_t dsFileStream_read(dsFileStream* stream, void* data, size_t size)
{
	if (!stream || !stream->file || !data)
		return 0;

	return fread(data, 1, size, stream->file);
}

size_t dsFileStream_write(dsFileStream* stream, const void* data, size_t size)
{
	if (!stream || !stream->file || !data)
		return 0;

	return fwrite(data, 1, size, stream->file);
}

bool dsFileStream_seek(dsFileStream* stream, int64_t offset, dsStreamSeekWay way)
{
	if (!stream || !stream->file)
		return false;

	int whence;
	switch (way)
	{
		case dsStreamSeekWay_Beginning:
			whence = SEEK_SET;
			break;
		case dsStreamSeekWay_Current:
			whence = SEEK_CUR;
			break;
		case dsStreamSeekWay_End:
			whence = SEEK_END;
			break;
		default:
			DS_ASSERT(false);
			return false;
	}
	return fseek(stream->file, (long)offset, whence) == 0;
}

uint64_t dsFileStream_tell(dsFileStream* stream)
{
	if (!stream || !stream->file)
		return DS_STREAM_INVALID_POS;

	return ftell(stream->file);
}

void dsFileStream_flush(dsFileStream* stream)
{
	if (stream && stream->file)
		fflush(stream->file);
}

bool dsFileStream_close(dsFileStream* stream)
{
	if (!stream || !stream->file)
		return false;

	fclose(stream->file);
	stream->file = NULL;
	return true;
}

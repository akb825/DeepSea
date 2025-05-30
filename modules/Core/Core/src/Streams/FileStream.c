/*
 * Copyright 2016-2025 Aaron Barany
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

// Guarantee 64-bits on GNU systems.
#define _FILE_OFFSET_BITS 64

#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <sys/stat.h>
#include <string.h>

#if DS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <direct.h>
#include <io.h>
#include <malloc.h>
#include <stdio.h>
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#else
#include <dirent.h>
#include <unistd.h>
#endif

#if DS_WINDOWS
typedef struct dsDirectoryIteratorInfo
{
	HANDLE handle;
	bool alreadyRetrieved;
	WIN32_FIND_DATAA findData;
} dsDirectoryIteratorInfo;

static void setErrno(void)
{
	switch (GetLastError())
	{
		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND:
			errno = ENOENT;
			break;
		case ERROR_ACCESS_DENIED:
			errno = EACCES;
			break;
		case ERROR_NOT_ENOUGH_MEMORY:
			errno = ENOMEM;
			break;
		default:
			errno = EIO;
			break;
	}
}
#endif

static void initFileStream(dsFileStream* stream, FILE* file)
{
	dsStream* baseStream = (dsStream*)stream;
	baseStream->readFunc = (dsStreamReadFunction)&dsFileStream_read;
	baseStream->writeFunc = (dsStreamWriteFunction)&dsFileStream_write;
	baseStream->seekFunc = (dsStreamSeekFunction)&dsFileStream_seek;
	baseStream->tellFunc = (dsStreamTellFunction)&dsFileStream_tell;
	baseStream->remainingBytesFunc = (dsStreamTellFunction)&dsFileStream_remainingBytes;
	baseStream->restartFunc = NULL;
	baseStream->flushFunc = (dsStreamFlushFunction)&dsFileStream_flush;
	baseStream->closeFunc = (dsStreamCloseFunction)&dsFileStream_close;
	stream->file = file;
}

dsPathStatus dsFileStream_pathStatus(const char* path)
{
	if (!path || *path == 0)
	{
		errno = EINVAL;
		return dsPathStatus_Error;
	}

#if DS_WINDOWS
	// Use stat64 in case stat would fail on a large file.
	struct _stat64 info;
	int result = _stat64(path, &info);
#else
	struct stat info;
	int result = stat(path, &info);
#endif

	if (result == 0)
		return S_ISDIR(info.st_mode) ? dsPathStatus_ExistsDirectory : dsPathStatus_ExistsFile;
	return errno == ENOENT ? dsPathStatus_Missing : dsPathStatus_Error;
}

bool dsFileStream_createDirectory(const char* path)
{
	if (!path || *path == 0)
	{
		errno = EINVAL;
		return false;
	}

#if DS_WINDOWS
	return mkdir(path) == 0;
#else
	return mkdir(path, 0755) == 0;
#endif
}

bool dsFileStream_removeFile(const char* path)
{
	if (!path || *path == 0)
	{
		errno = EINVAL;
		return false;
	}

	return unlink(path) == 0;
}

bool dsFileStream_removeDirectory(const char* path)
{
	if (!path || *path == 0)
	{
		errno = EINVAL;
		return false;
	}

	return rmdir(path) == 0;
}

dsDirectoryIterator dsFileStream_openDirectory(const char* path)
{
	if (!path || *path == 0)
	{
		errno = EINVAL;
		return NULL;
	}

#if DS_WINDOWS
	char searchPath[DS_PATH_MAX];
	if (!dsPath_combine(searchPath, sizeof(searchPath), path, "*"))
		return NULL;

	dsDirectoryIteratorInfo* info = malloc(sizeof(dsDirectoryIteratorInfo));
	if (!info)
		return NULL;

	info->handle = FindFirstFileEx(searchPath, FindExInfoBasic, &info->findData,
		FindExSearchNameMatch, NULL, 0);
	if (info->handle == INVALID_HANDLE_VALUE)
	{
		free(info);
		setErrno();
		return NULL;
	}

	info->alreadyRetrieved = true;
	return info;
#else
	return opendir(path);
#endif
}

dsPathStatus dsFileStream_nextDirectoryEntry(
	char* result, size_t resultSize, dsDirectoryIterator iterator)
{
	if (!result || resultSize == 0 || !iterator)
	{
		errno = EINVAL;
		return dsPathStatus_Error;
	}

	char* entryName;
	dsPathStatus status;
	do
	{
#if DS_WINDOWS
		dsDirectoryIteratorInfo* info = (dsDirectoryIteratorInfo*)iterator;
		if (info->alreadyRetrieved)
			info->alreadyRetrieved = false;
		else if (!FindNextFile(info->handle, &info->findData))
		{
			if (GetLastError() == ERROR_NO_MORE_FILES)
				return dsPathStatus_Missing;

			setErrno();
			return dsPathStatus_Error;
		}

		entryName = info->findData.cFileName;
		status = (info->findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ?
			dsPathStatus_ExistsDirectory : dsPathStatus_ExistsFile;
#else
		errno = 0;
		struct dirent* dirEntry = readdir((DIR*)iterator);
		if (!dirEntry)
		{
			if (errno == 0)
				return dsPathStatus_Missing;
			return dsPathStatus_Error;
		}

		entryName = dirEntry->d_name;
		status = dirEntry->d_type == DT_DIR ?
			dsPathStatus_ExistsDirectory : dsPathStatus_ExistsFile;
#endif
	} while (strcmp(entryName, ".") == 0 || strcmp(entryName, "..") == 0);

	size_t nameLen = strlen(entryName) + 1;
	if (nameLen > resultSize)
	{
		errno = ESIZE;
		return dsPathStatus_Error;
	}
	else
		memcpy(result, entryName, nameLen);
	return status;
}

bool dsFileStream_closeDirectory(dsDirectoryIterator iterator)
{
	if (!iterator)
	{
		errno = EINVAL;
		return false;
	}

#if DS_WINDOWS
	dsDirectoryIteratorInfo* info = (dsDirectoryIteratorInfo*)iterator;
	bool success = FindClose(info->handle);
	if (success)
		free(info);
	else
		setErrno();
	return success;
#else
	return closedir((DIR*)iterator) == 0;
#endif
}

bool dsFileStream_openPath(dsFileStream* stream, const char* path, const char* mode)
{
	if (!stream || !path || *path == 0 || !mode)
	{
		errno = EINVAL;
		return false;
	}

	FILE* file = fopen(path, mode);
	if (!file)
		return false;

	initFileStream(stream, file);
	return true;
}

bool dsFileStream_openFile(dsFileStream* stream, FILE* file)
{
	if (!stream || !file)
	{
		errno = EINVAL;
		return false;
	}

	initFileStream(stream, file);
	return true;
}

size_t dsFileStream_read(dsFileStream* stream, void* data, size_t size)
{
	if (!stream || !stream->file || !data)
	{
		errno = EINVAL;
		return 0;
	}

	return fread(data, 1, size, stream->file);
}

size_t dsFileStream_write(dsFileStream* stream, const void* data, size_t size)
{
	if (!stream || !stream->file || !data)
	{
		errno = EINVAL;
		return 0;
	}

	return fwrite(data, 1, size, stream->file);
}

bool dsFileStream_seek(dsFileStream* stream, int64_t offset, dsStreamSeekWay way)
{
	if (!stream || !stream->file)
	{
		errno = EINVAL;
		return false;
	}

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
			errno = EINVAL;
			return false;
	}
#if DS_WINDOWS
	return _fseeki64(stream->file, offset, whence) == 0;
#else
	return fseeko(stream->file, (off_t)offset, whence) == 0;
#endif
}

uint64_t dsFileStream_tell(dsFileStream* stream)
{
	if (!stream || !stream->file)
	{
		errno = EINVAL;
		return DS_STREAM_INVALID_POS;
	}

#if DS_WINDOWS
	return _ftelli64(stream->file);
#else
	return ftello(stream->file);
#endif
}

inline uint64_t dsFileStream_remainingBytes(dsFileStream* stream)
{
	if (!stream || !stream->file)
	{
		errno = EINVAL;
		return DS_STREAM_INVALID_POS;
	}

#if DS_WINDOWS
	int64_t position = _ftelli64(stream->file);
	if (position < 0)
		return DS_STREAM_INVALID_POS;

	if (_fseeki64(stream->file, 0, SEEK_END) != 0)
		return DS_STREAM_INVALID_POS;

	int64_t end = _ftelli64(stream->file);
	if (end < 0 || (end != position && _fseeki64(stream->file, position, SEEK_SET) != 0))
		return DS_STREAM_INVALID_POS;

	return end - position;
#else
	off_t position = ftello(stream->file);
	if (position < 0)
		return DS_STREAM_INVALID_POS;

	if (fseeko(stream->file, 0, SEEK_END) != 0)
		return DS_STREAM_INVALID_POS;

	off_t end = ftello(stream->file);
	if (end < 0 || (end != position && fseeko(stream->file, position, SEEK_SET) != 0))
		return DS_STREAM_INVALID_POS;

	return end - position;
#endif
}

void dsFileStream_flush(dsFileStream* stream)
{
	if (stream && stream->file)
		fflush(stream->file);
}

bool dsFileStream_close(dsFileStream* stream)
{
	if (!stream || !stream->file)
	{
		errno = EINVAL;
		return false;
	}

	fclose(stream->file);
	stream->file = NULL;
	return true;
}

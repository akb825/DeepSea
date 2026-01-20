/*
 * Copyright 2026 Aaron Barany
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

#include <DeepSea/Core/Streams/RelativePathStream.h>

#include <DeepSea/Core/Streams/FileArchive.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

dsStream* dsFileRelativePath_open(void* userData, const char* path, const char* mode)
{
	if (!userData || !path || !mode)
	{
		errno = EINVAL;
		return NULL;
	}

	dsFileRelativePath* fileInfo = (dsFileRelativePath*)userData;
	char finalPath[DS_PATH_MAX];
	if (!dsPath_combine(finalPath, sizeof(finalPath), fileInfo->basePath, path))
	{
		DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "Path '%s%c%s' is too long.", fileInfo->basePath,
			DS_PATH_SEPARATOR, path);
		return NULL;
	}

	if (!dsFileStream_openPath(&fileInfo->stream, finalPath, mode))
	{
		DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "Couldn't open file '%s'.", finalPath);
		return NULL;
	}

	return (dsStream*)&fileInfo->stream;
}

void dsFileRelativePath_close(void* userData, dsStream* stream)
{
	DS_UNUSED(userData);
	dsStream_close(stream);
}

dsStream* dsResourceRelativePath_open(void* userData, const char* path, const char* mode)
{
	if (!userData || !path || !mode)
	{
		errno = EINVAL;
		return NULL;
	}

	dsResourceRelativePath* resourceInfo = (dsResourceRelativePath*)userData;
	char finalPath[DS_PATH_MAX];
	if (!dsPath_combine(finalPath, sizeof(finalPath), resourceInfo->basePath, path))
	{
		DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "Path '%s%c%s' is too long.", resourceInfo->basePath,
			DS_PATH_SEPARATOR, path);
		return NULL;
	}

	if (!dsResourceStream_open(&resourceInfo->stream, resourceInfo->type, finalPath, mode))
	{
		DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "Couldn't open file '%s'.", finalPath);
		return NULL;
	}

	return (dsStream*)&resourceInfo->stream;
}

void dsResourceRelativePath_close(void* userData, dsStream* stream)
{
	DS_UNUSED(userData);
	dsStream_close(stream);
}

dsStream* dsArchiveRelativePath_open(void* userData, const char* path, const char* mode)
{
	if (!userData || !path || !mode)
	{
		errno = EINVAL;
		return NULL;
	}

	dsArchiveRelativePath* archiveInfo = (dsArchiveRelativePath*)userData;
	char finalPath[DS_PATH_MAX];
	if (!dsPath_combine(finalPath, sizeof(finalPath), archiveInfo->basePath, path))
	{
		DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "Path '%s%c%s' is too long.", archiveInfo->basePath,
			DS_PATH_SEPARATOR, path);
		return NULL;
	}

	dsStream* stream = dsFileArchive_openFile(archiveInfo->archive, finalPath);
	if (!stream)
	{
		DS_LOG_ERROR_F(DS_CORE_LOG_TAG, "Couldn't open file '%s'.", finalPath);
		return NULL;
	}

	return stream;
}

void dsArchiveRelativePath_close(void* userData, dsStream* stream)
{
	DS_UNUSED(userData);
	dsStream_close(stream);
}

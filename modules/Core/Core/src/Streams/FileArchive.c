/*
 * Copyright 2025 Aaron Barany
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

#include <DeepSea/Core/Streams/FileArchive.h>

#include <DeepSea/Core/Error.h>

dsPathStatus dsFileArchive_pathStatus(const dsFileArchive* archive, const char* path)
{
	if (!archive || !archive->getPathStatusFunc || !path || *path == 0)
	{
		errno = EINVAL;
		return dsPathStatus_Error;
	}

	return archive->getPathStatusFunc(archive, path);
}

dsDirectoryIterator dsFileArchive_openDirectory(const dsFileArchive* archive, const char* path)
{
	if (!archive || !archive->openDirectoryFunc || !archive->nextDirectoryEntryFunc ||
		!archive->closeDirectoryFunc || !path || *path == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	return archive->openDirectoryFunc(archive, path);
}

dsPathStatus dsFileArchive_nextDirectoryEntry(
	char* result, size_t resultSize, const dsFileArchive* archive, dsDirectoryIterator iterator)
{
	if (!result || resultSize == 0 || !archive || !archive->nextDirectoryEntryFunc || !iterator)
	{
		errno = EINVAL;
		return dsPathStatus_Error;
	}

	return archive->nextDirectoryEntryFunc(result, resultSize, archive, iterator);
}

bool dsFileArchive_closeDirectory(const dsFileArchive* archive, dsDirectoryIterator iterator)
{
	if (!archive || !archive->closeDirectoryFunc || !iterator)
	{
		errno = EINVAL;
		return false;
	}

	return archive->closeDirectoryFunc(archive, iterator);
}

dsStream* dsFileArchive_openFile(const dsFileArchive* archive, const char* path)
{
	if (!archive || !archive->openFileFunc || !path || *path == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	return archive->openFileFunc(archive, path);
}

void dsFileArchive_close(dsFileArchive* archive)
{
	if (archive && archive->closeFunc)
		archive->closeFunc(archive);
}

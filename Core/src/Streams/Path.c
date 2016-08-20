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

#include <DeepSea/Core/Streams/Path.h>
#include <errno.h>
#include <string.h>

bool dsPath_combine(char* result, size_t resultSize, const char* path1, const char* path2)
{
	if (!result || resultSize == 0 || result == path2)
	{
		errno = EINVAL;
		return false;
	}

	size_t len1 = path1 ? strlen(path1) : 0;
	size_t len2 = path2 ? strlen(path2) : 0;

	// Handle when one or both paths are empty.
	if (!len1 && !len2)
	{
		*result = 0;
		return true;
	}

	if (len1 && !len2)
	{
		if (resultSize < len1 + 1)
		{
			errno = ERANGE;
			return false;
		}

		if (result != path1)
			strncpy(result, path1, len1 + 1);
		return true;
	}

	if (!len1 && len2)
	{
		if (resultSize < len2 + 1)
		{
			errno = ERANGE;
			return false;
		}

		strncpy(result, path2, len2 + 1);
		return true;
	}

	// Remove the path separator if present for each path.
	for (; len1 > 0; --len1)
	{
		if (path1[len1 - 1] != DS_PATH_SEPARATOR && path1[len1 - 1] != DS_PATH_ALT_SEPARATOR)
			break;
	}

	for (; len2 > 0; ++path2, --len2)
	{
		if (path2[0] != DS_PATH_SEPARATOR && path2[0] != DS_PATH_ALT_SEPARATOR)
			break;
	}

	if (resultSize < (len1 + len2 + 2))
	{
		errno = ERANGE;
		return false;
	}

	if (result != path1)
		strncpy(result, path1, len1);

	result[len1] = DS_PATH_SEPARATOR;
	strncpy(result + len1 + 1, path2, len2 + 1);
	return true;
}

bool dsPath_getDirectoryName(char* result, size_t resultSize, const char* path)
{
	if (!result || resultSize == 0 || !path)
	{
		errno = EINVAL;
		return false;
	}

	// Search for the last path separator.
	size_t length = strlen(path);
	if (length == 0)
	{
		errno = EINVAL;
		return false;
	}

	size_t end = length - 1;
	do
	{
		if (path[end] == DS_PATH_SEPARATOR || path[end] == DS_PATH_ALT_SEPARATOR)
		{
			// Get rid of any extra path separators.
			for (; end > 0 && (path[end - 1] == DS_PATH_SEPARATOR ||
				path[end - 1] == DS_PATH_ALT_SEPARATOR); --end)
			{
			}

			// Keep separator if at the root.
			if (end == 0)
			{
				if (resultSize < 2)
				{
					errno = ERANGE;
					return false;
				}

				result[0] = DS_PATH_SEPARATOR;
				result[1] = 0;
				return true;
			}
			else
			{
				if (resultSize < end + 1)
				{
					errno = ERANGE;
					return false;
				}

				if (result != path)
					strncpy(result, path, end);
				result[end] = 0;
				return true;
			}
		}

		if (end-- == 0)
		{
			errno = EINVAL;
			return false;
		}
	} while (true);
}

const char* dsPath_getFileName(const char* path)
{
	if (!path)
		return NULL;

	// Search for the last path separator.
	size_t end = strlen(path) - 1;
	do
	{
		if (path[end] == DS_PATH_SEPARATOR || path[end] == DS_PATH_ALT_SEPARATOR)
			return path + end + 1;

		if (end-- == 0)
			return path;
	} while (true);
}

const char* dsPath_getExtension(const char* path)
{
	const char* fileName = dsPath_getFileName(path);
	if (!fileName)
		return NULL;

	// Search for the first '.' in the file name.
	for (size_t i = 0; fileName[i]; ++i)
	{
		if (fileName[i] == '.')
			return fileName + i;
	}

	return NULL;
}

const char* dsPath_getLastExtension(const char* path)
{
	const char* fileName = dsPath_getFileName(path);
	if (!fileName)
		return NULL;

	// Search for the last '.'.
	size_t end = strlen(fileName) - 1;
	do
	{
		if (fileName[end] == '.')
			return fileName + end;

		if (end-- == 0)
			return NULL;
	} while (true);
}

bool dsPath_removeLastExtension(char* result, size_t resultSize, const char* path)
{
	if (!result || resultSize == 0 || !path)
	{
		errno = EINVAL;
		return false;
	}

	const char* extension = dsPath_getLastExtension(path);
	size_t len;
	if (extension)
		len = extension - path;
	else
		len = strlen(path);

	if (resultSize < len + 1)
	{
		errno = ERANGE;
		return false;
	}

	if (result != path)
		strncpy(result, path, len);
	result[len] = 0;
	return true;
}

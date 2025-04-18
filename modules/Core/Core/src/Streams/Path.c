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

#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Error.h>
#include <string.h>

inline static bool isEndOrSep(char c)
{
	return c == 0 || c == DS_PATH_SEPARATOR || c == DS_PATH_ALT_SEPARATOR;
}

bool dsPath_combine(char* result, size_t resultSize, const char* path1, const char* path2)
{
	if (!result || resultSize == 0 || result == path2)
	{
		errno = EINVAL;
		return false;
	}

	size_t len1 = path1 ? strlen(path1) : 0;
	size_t len2 = path2 ? strlen(path2) : 0;

	if (len2 > 0 && dsPath_isAbsolute(path2))
	{
		if (resultSize < len2 + 1)
		{
			errno = ESIZE;
			return false;
		}

		memcpy(result, path2, len2 + 1);
		return true;
	}

	// Handle when one or both paths are empty.
	if (len1 == 0 && len2 == 0)
	{
		*result = 0;
		return true;
	}

	// Handle leading ./ and ../ entries.
	while (len1 > 0 && len2 > 0 && path2[0] == '.')
	{
		if (isEndOrSep(path2[1]))
		{
			++path2;
			--len2;
			if (path2[0] != 0)
			{
				++path2;
				--len2;
			}
		}
		else if (path2[1] == '.' && isEndOrSep(path2[2]))
		{
			// Remove the previous directory.
			for (; len1 > 0; --len1)
			{
				if (path1[len1 - 1] != DS_PATH_SEPARATOR && path1[len1 - 1] != DS_PATH_ALT_SEPARATOR)
					break;
			}

			// If no more characters left, this is attempting to go above an absolute path.
			if (len1 == 0)
			{
				errno = EINVAL;
				return false;
			}

			// Check for Windows absolute path.
#if DS_WINDOWS
			if (len1 == 2 && path1[1] == ':')
			{
				errno = EINVAL;
				return false;
			}
#endif

			for (; len1 > 0; --len1)
			{
				if (path1[len1 - 1] == DS_PATH_SEPARATOR || path1[len1 - 1] == DS_PATH_ALT_SEPARATOR)
					break;
			}

			path2 += 2;
			len2 -= 2;
			if (path2[0] != 0)
			{
				++path2;
				--len2;
			}
		}
		else
			break;
	}

	if (len1 == 0)
	{
		if (resultSize < len2 + 1)
		{
			errno = ESIZE;
			return false;
		}

		memcpy(result, path2, len2 + 1);
		return true;
	}

	if (len1 > 0 && len2 == 0)
	{
		if (resultSize < len1 + 1)
		{
			errno = ESIZE;
			return false;
		}

		if (result != path1)
			memcpy(result, path1, len1 + 1);
		return true;
	}

	// Remove the trailing path separators from the first path.
	for (; len1 > 0; --len1)
	{
		if (path1[len1 - 1] != DS_PATH_SEPARATOR && path1[len1 - 1] != DS_PATH_ALT_SEPARATOR)
			break;
	}

	if (resultSize < (len1 + len2 + 2))
	{
		errno = ESIZE;
		return false;
	}

	if (result != path1)
		memcpy(result, path1, len1);

	result[len1] = DS_PATH_SEPARATOR;
	memcpy(result + len1 + 1, path2, len2 + 1);
	return true;
}

bool dsPath_isAbsolute(const char* path)
{
	if (!path)
		return false;

	size_t length = strlen(path);
	if (length == 0)
		return false;

	if (path[0] == DS_PATH_SEPARATOR || path[0] == DS_PATH_ALT_SEPARATOR)
		return true;

#if DS_WINDOWS
	if (path[1] == ':')
		return true;
#endif

	return false;
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
					errno = ESIZE;
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
					errno = ESIZE;
					return false;
				}

				if (result != path)
					memcpy(result, path, end);
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
		errno = ESIZE;
		return false;
	}

	if (result != path)
		memcpy(result, path, len);
	result[len] = 0;
	return true;
}

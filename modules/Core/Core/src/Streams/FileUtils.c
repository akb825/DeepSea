/*
 * Copyright 2018-2022 Aaron Barany
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

#include <DeepSea/Core/Streams/FileUtils.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#if DS_WINDOWS
#include <direct.h>
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif

bool dsCreateDirectory(const char* dirName)
{
#if DS_WINDOWS
	return mkdir(dirName) == 0;
#else
	return mkdir(dirName, 0755) == 0;
#endif
}

dsFileStatus dsGetFileStatus(const char* fileName)
{
	struct stat info;
	int result = stat(fileName, &info);
	if (result == 0)
		return S_ISDIR(info.st_mode) ? dsFileStatus_ExistsDirectory : dsFileStatus_ExistsFile;
	return errno == ENOENT ? dsFileStatus_DoesntExist : dsFileStatus_Error;
}

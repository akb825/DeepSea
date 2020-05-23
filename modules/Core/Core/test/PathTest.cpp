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

#include "Helpers.h"
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Error.h>
#include <gtest/gtest.h>

TEST(PathTest, Combine)
{
	char result[DS_PATH_MAX];
	EXPECT_FALSE_ERRNO(EINVAL, dsPath_combine(nullptr, DS_PATH_MAX, "", ""));
	EXPECT_FALSE_ERRNO(EINVAL, dsPath_combine(result, 0, "", ""));

	strncpy(result, "test", DS_PATH_MAX);
	EXPECT_FALSE_ERRNO(EINVAL, dsPath_combine(result, DS_PATH_MAX, "", result));

	EXPECT_TRUE(dsPath_combine(result, DS_PATH_MAX, "", ""));
	EXPECT_STREQ("", result);

	EXPECT_TRUE(dsPath_combine(result, DS_PATH_MAX, nullptr, nullptr));
	EXPECT_STREQ("", result);

	EXPECT_TRUE(dsPath_combine(result, DS_PATH_MAX, "path1", nullptr));
	EXPECT_STREQ("path1", result);

	EXPECT_TRUE(dsPath_combine(result, DS_PATH_MAX, nullptr, "path2"));
	EXPECT_STREQ("path2", result);

	EXPECT_TRUE(dsPath_combine(result, DS_PATH_MAX, "path1", "path2"));
#if DS_WINDOWS
	EXPECT_STREQ("path1\\path2", result);
#else
	EXPECT_STREQ("path1/path2", result);
#endif

	EXPECT_TRUE(dsPath_combine(result, DS_PATH_MAX, "otherPath1//", "//path2"));
#if DS_WINDOWS
	EXPECT_STREQ("otherPath1\\path2", result);
#else
	EXPECT_STREQ("otherPath1/path2", result);
#endif

	EXPECT_TRUE(dsPath_combine(result, DS_PATH_MAX, "/", "test"));
#if DS_WINDOWS
	EXPECT_STREQ("\\test", result);
#else
	EXPECT_STREQ("/test", result);
#endif

	EXPECT_FALSE_ERRNO(ESIZE, dsPath_combine(result, 11, "path1", "path2"));
	EXPECT_TRUE(dsPath_combine(result, 12, "path1", "path2"));

	EXPECT_FALSE_ERRNO(ESIZE, dsPath_combine(result, 11, "path1//", "//path2"));
	EXPECT_TRUE(dsPath_combine(result, 12, "path1//", "//path2"));

	result[0] = 0;
	EXPECT_TRUE(dsPath_combine(result, DS_PATH_MAX, result, "test"));
	EXPECT_TRUE(dsPath_combine(result, DS_PATH_MAX, result, "path"));
	EXPECT_TRUE(dsPath_combine(result, DS_PATH_MAX, result, "combine"));
}

TEST(PathTest, IsAbsolute)
{
	EXPECT_FALSE(dsPath_isAbsolute(nullptr));
	EXPECT_FALSE(dsPath_isAbsolute(""));
	EXPECT_FALSE(dsPath_isAbsolute("a"));
	EXPECT_FALSE(dsPath_isAbsolute("a/relative/path"));
	EXPECT_TRUE(dsPath_isAbsolute("/absolute/path"));

#if DS_WINDOWS
	EXPECT_TRUE(dsPath_isAbsolute("\\absolute\\path"));
	EXPECT_TRUE(dsPath_isAbsolute("C:\\absolute\\path"));
	EXPECT_TRUE(dsPath_isAbsolute("C:/absolute/path"));
#endif
}

TEST(PathTest, GetDirectoryName)
{
	char result[DS_PATH_MAX];
	EXPECT_FALSE_ERRNO(EINVAL, dsPath_getDirectoryName(nullptr, DS_PATH_MAX, "test/"));
	EXPECT_FALSE_ERRNO(EINVAL, dsPath_getDirectoryName(result, 0, "test/"));
	EXPECT_FALSE_ERRNO(EINVAL, dsPath_getDirectoryName(result, DS_PATH_MAX, ""));
	EXPECT_FALSE_ERRNO(EINVAL, dsPath_getDirectoryName(result, DS_PATH_MAX, "test"));

	EXPECT_TRUE(dsPath_getDirectoryName(result, DS_PATH_MAX, "test/"));
	EXPECT_STREQ("test", result);

	EXPECT_TRUE(dsPath_getDirectoryName(result, DS_PATH_MAX, "/test"));
#if DS_WINDOWS
	EXPECT_STREQ("\\", result);
#else
	EXPECT_STREQ("/", result);
#endif

	EXPECT_TRUE(dsPath_getDirectoryName(result, DS_PATH_MAX, "///test"));
#if DS_WINDOWS
	EXPECT_STREQ("\\", result);
#else
	EXPECT_STREQ("/", result);
#endif

	EXPECT_TRUE(dsPath_getDirectoryName(result, DS_PATH_MAX, "/test/directory///name"));
	EXPECT_STREQ("/test/directory", result);

	EXPECT_TRUE(dsPath_getDirectoryName(result, DS_PATH_MAX, "/test/directory/name/"));
	EXPECT_STREQ("/test/directory/name", result);

	EXPECT_FALSE_ERRNO(ESIZE, dsPath_getDirectoryName(result, strlen("/test/directory"),
		"/test/directory/name"));
	EXPECT_TRUE(dsPath_getDirectoryName(result, strlen("/test/directory") + 1,
		"/test/directory/name"));

	EXPECT_FALSE_ERRNO(ESIZE, dsPath_getDirectoryName(result, 1, "/test"));
	EXPECT_TRUE(dsPath_getDirectoryName(result, 2, "/test"));

	strncpy(result, "test/directory/name", DS_PATH_MAX);
	EXPECT_TRUE(dsPath_getDirectoryName(result, DS_PATH_MAX, result));
	EXPECT_STREQ("test/directory", result);
}

TEST(PathTest, GetFileName)
{
	EXPECT_EQ(nullptr, dsPath_getFileName(nullptr));
	EXPECT_STREQ("test", dsPath_getFileName("test"));
	EXPECT_STREQ("test", dsPath_getFileName("/test"));
	EXPECT_STREQ("name", dsPath_getFileName("/test/file/name"));
	EXPECT_STREQ("", dsPath_getFileName("/test/file/name/"));
}

TEST(PathTest, GetExtension)
{
	EXPECT_EQ(nullptr, dsPath_getExtension(nullptr));
	EXPECT_EQ(nullptr, dsPath_getExtension("test"));
	EXPECT_STREQ(".foo", dsPath_getExtension("test.foo"));
	EXPECT_STREQ(".foo.bar", dsPath_getExtension("test.foo.bar"));
	EXPECT_STREQ(".foo", dsPath_getExtension("test.file/name.foo"));
	EXPECT_STREQ(nullptr, dsPath_getExtension("test.file/name"));
}

TEST(PathTest, GetLastExtension)
{
	EXPECT_EQ(nullptr, dsPath_getLastExtension(nullptr));
	EXPECT_EQ(nullptr, dsPath_getLastExtension("test"));
	EXPECT_STREQ(".foo", dsPath_getLastExtension("test.foo"));
	EXPECT_STREQ(".bar", dsPath_getLastExtension("test.foo.bar"));
	EXPECT_STREQ(".foo", dsPath_getLastExtension("test.file/name.foo"));
	EXPECT_STREQ(nullptr, dsPath_getLastExtension("test.file/name"));
}

TEST(PathTest, RemoveLastExtension)
{
	char result[DS_PATH_MAX];
	EXPECT_FALSE_ERRNO(EINVAL, dsPath_removeLastExtension(nullptr, DS_PATH_MAX, "test"));
	EXPECT_FALSE_ERRNO(EINVAL, dsPath_removeLastExtension(result, 0, "test"));
	EXPECT_FALSE_ERRNO(EINVAL, dsPath_removeLastExtension(result, DS_PATH_MAX, nullptr));

	EXPECT_TRUE(dsPath_removeLastExtension(result, DS_PATH_MAX, "test"));
	EXPECT_STREQ("test", result);

	EXPECT_TRUE(dsPath_removeLastExtension(result, DS_PATH_MAX, "test.foo"));
	EXPECT_STREQ("test", result);

	EXPECT_TRUE(dsPath_removeLastExtension(result, DS_PATH_MAX, "test.foo.bar"));
	EXPECT_STREQ("test.foo", result);

	EXPECT_TRUE(dsPath_removeLastExtension(result, DS_PATH_MAX, "test.file/name.foo"));
	EXPECT_STREQ("test.file/name", result);

	EXPECT_TRUE(dsPath_removeLastExtension(result, DS_PATH_MAX, "test.file/name"));
	EXPECT_STREQ("test.file/name", result);

	EXPECT_FALSE_ERRNO(ESIZE, dsPath_removeLastExtension(result, strlen("test.file/name"),
		"test.file/name.foo"));
	EXPECT_TRUE(dsPath_removeLastExtension(result, strlen("test.file/name") + 1,
		"test.file/name.foo"));

	strncpy(result, "test.file/name.foo", DS_PATH_MAX);
	EXPECT_TRUE(dsPath_removeLastExtension(result, DS_PATH_MAX, result));
	EXPECT_STREQ("test.file/name", result);
}

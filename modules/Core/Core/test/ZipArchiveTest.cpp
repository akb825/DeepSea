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

#include "Helpers.h"
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Streams/ZipArchive.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <gtest/gtest.h>

class ZipArchiveTest : public testing::Test
{
public:
	ZipArchiveTest()
		: allocator(reinterpret_cast<dsAllocator*>(&systemAllocator))
	{
	}

	void SetUp() override
	{
		EXPECT_TRUE(dsSystemAllocator_initialize(&systemAllocator, DS_ALLOCATOR_NO_LIMIT));
	}

	void TearDown() override
	{
		EXPECT_EQ(0U, allocator->size);
	}

	dsSystemAllocator systemAllocator;
	dsAllocator* allocator;
};

static const char* assetDir = "Core-assets";

TEST_F(ZipArchiveTest, Open)
{
	char path[DS_PATH_MAX];
	ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "empty"));
	EXPECT_FALSE_ERRNO(EFORMAT,
		dsZipArchive_openResource(allocator, dsFileResourceType_Embedded, path, 0));

	ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "text.txt"));
	EXPECT_FALSE_ERRNO(EFORMAT,
		dsZipArchive_openResource(allocator, dsFileResourceType_Embedded, path, 0));

	ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "simple.zip"));
	dsZipArchive* archive =
		dsZipArchive_openResource(allocator, dsFileResourceType_Embedded, path, 0);
	ASSERT_TRUE(archive);
	dsZipArchive_close(archive);
}

TEST_F(ZipArchiveTest, PathStatus)
{
	char path[DS_PATH_MAX];
	ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "simple.zip"));
	dsZipArchive* archive =
		dsZipArchive_openResource(allocator, dsFileResourceType_Embedded, path, 0);
	ASSERT_TRUE(archive);

	EXPECT_EQ(dsPathStatus_Error, dsZipArchive_pathStatus(archive, nullptr));
	EXPECT_EQ(dsPathStatus_Error, dsZipArchive_pathStatus(archive, ""));
	EXPECT_EQ(dsPathStatus_Missing, dsZipArchive_pathStatus(archive, "/"));

	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, "first"));
	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, "second"));

	EXPECT_EQ(dsPathStatus_Missing, dsZipArchive_pathStatus(archive, "firs"));
	EXPECT_EQ(dsPathStatus_Missing, dsZipArchive_pathStatus(archive, "first "));
	EXPECT_EQ(dsPathStatus_Missing, dsZipArchive_pathStatus(archive, "firstt"));

	EXPECT_EQ(dsPathStatus_ExistsDirectory, dsZipArchive_pathStatus(archive, "."));
	EXPECT_EQ(dsPathStatus_ExistsDirectory, dsZipArchive_pathStatus(archive, "./"));
	EXPECT_EQ(dsPathStatus_ExistsDirectory, dsZipArchive_pathStatus(archive, "empty"));
	EXPECT_EQ(dsPathStatus_ExistsDirectory, dsZipArchive_pathStatus(archive, "empty/"));
	EXPECT_EQ(dsPathStatus_ExistsDirectory, dsZipArchive_pathStatus(archive, "directory"));
	EXPECT_EQ(dsPathStatus_ExistsDirectory, dsZipArchive_pathStatus(archive, "directory/"));

	EXPECT_EQ(dsPathStatus_Missing, dsZipArchive_pathStatus(archive, "director"));
	EXPECT_EQ(dsPathStatus_Missing, dsZipArchive_pathStatus(archive, "directory "));
	EXPECT_EQ(dsPathStatus_Missing, dsZipArchive_pathStatus(archive, "directoryy"));

	ASSERT_TRUE(dsPath_combine(path, sizeof(path), "directory", "third"));
	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, path));

	ASSERT_TRUE(dsPath_combine(path, sizeof(path), ".", "directory"));
	EXPECT_EQ(dsPathStatus_ExistsDirectory, dsZipArchive_pathStatus(archive, path));
	ASSERT_TRUE(dsPath_combine(path, sizeof(path), path, "fourth"));
	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, path));

	dsZipArchive_close(archive);
}

TEST_F(ZipArchiveTest, PathStatusFileEntriesOnly)
{
	char path[DS_PATH_MAX];
	ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "files-only.zip"));
	dsZipArchive* archive =
		dsZipArchive_openResource(allocator, dsFileResourceType_Embedded, path, 0);
	ASSERT_TRUE(archive);

	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, "first"));
	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, "second"));
	EXPECT_EQ(dsPathStatus_Missing, dsZipArchive_pathStatus(archive, "empty"));
	EXPECT_EQ(dsPathStatus_ExistsDirectory, dsZipArchive_pathStatus(archive, "directory"));
	EXPECT_EQ(dsPathStatus_ExistsDirectory, dsZipArchive_pathStatus(archive, "directory/"));

	ASSERT_TRUE(dsPath_combine(path, sizeof(path), "directory", "third"));
	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, path));

	ASSERT_TRUE(dsPath_combine(path, sizeof(path), "directory", "fourth"));
	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, path));

	dsZipArchive_close(archive);
}

TEST_F(ZipArchiveTest, PathStatusSharedPrefixes)
{
	char path[DS_PATH_MAX];
	ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "prefix.zip"));
	dsZipArchive* archive =
		dsZipArchive_openResource(allocator, dsFileResourceType_Embedded, path, 0);
	ASSERT_TRUE(archive);

	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, "common/prefix file"));
	EXPECT_EQ(dsPathStatus_ExistsDirectory, dsZipArchive_pathStatus(archive, "common/prefix"));
	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, "common/prefix/first"));
	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, "common/prefix/second"));
	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, "common/prefix1"));

	dsZipArchive_close(archive);
}

TEST_F(ZipArchiveTest, PathStatusLargeFiles)
{
	char path[DS_PATH_MAX];
	ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "large.zip"));
	dsZipArchive* archive =
		dsZipArchive_openResource(allocator, dsFileResourceType_Embedded, path, 0);
	ASSERT_TRUE(archive);

	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, "first"));
	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, "second"));
	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, "large"));
	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, "largest32"));

	dsZipArchive_close(archive);
}

TEST_F(ZipArchiveTest, PathStatusWithComments)
{
	char path[DS_PATH_MAX];
	ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "comments.zip"));
	dsZipArchive* archive =
		dsZipArchive_openResource(allocator, dsFileResourceType_Embedded, path, 0);
	ASSERT_TRUE(archive);

	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, "first"));
	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, "second"));

	dsZipArchive_close(archive);
}

TEST_F(ZipArchiveTest, IterateDirectory)
{
	char path[DS_PATH_MAX];
	ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "simple.zip"));
	dsZipArchive* archive =
		dsZipArchive_openResource(allocator, dsFileResourceType_Embedded, path, 0);
	ASSERT_TRUE(archive);

	EXPECT_FALSE_ERRNO(EINVAL, dsZipArchive_openDirectory(archive, nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsZipArchive_openDirectory(archive, ""));
	EXPECT_FALSE_ERRNO(ENOENT, dsZipArchive_openDirectory(archive, "/"));

	dsDirectoryIterator iterator = dsZipArchive_openDirectory(archive, ".");
	ASSERT_TRUE(iterator);

	char name[DS_FILE_NAME_MAX] = {};
	EXPECT_EQ(dsPathStatus_ExistsDirectory,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("directory", name);
	EXPECT_EQ(dsPathStatus_ExistsDirectory,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("empty", name);
	EXPECT_EQ(dsPathStatus_ExistsFile,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("first", name);
	EXPECT_EQ(dsPathStatus_ExistsFile,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("second", name);
	EXPECT_EQ(dsPathStatus_Missing,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_EQ(dsPathStatus_Missing,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));

	EXPECT_TRUE(dsZipArchive_closeDirectory(archive, iterator));

	iterator = dsZipArchive_openDirectory(archive, "./directory/");
	ASSERT_TRUE(iterator);

	EXPECT_EQ(dsPathStatus_ExistsFile,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("fourth", name);
	EXPECT_EQ(dsPathStatus_ExistsFile,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("third", name);
	EXPECT_EQ(dsPathStatus_Missing,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));

	EXPECT_TRUE(dsZipArchive_closeDirectory(archive, iterator));

	ASSERT_TRUE(dsPath_combine(path, sizeof(path), ".", "directory"));
	iterator = dsZipArchive_openDirectory(archive, path);
	ASSERT_TRUE(iterator);

	EXPECT_EQ(dsPathStatus_ExistsFile,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("fourth", name);
	EXPECT_EQ(dsPathStatus_ExistsFile,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("third", name);
	EXPECT_EQ(dsPathStatus_Missing,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));

	EXPECT_TRUE(dsZipArchive_closeDirectory(archive, iterator));

	iterator = dsZipArchive_openDirectory(archive, "empty");
	ASSERT_TRUE(iterator);

	EXPECT_EQ(dsPathStatus_Missing,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));

	EXPECT_TRUE(dsZipArchive_closeDirectory(archive, iterator));

	EXPECT_FALSE_ERRNO(ENOTDIR, dsZipArchive_openDirectory(archive, "first"));
	EXPECT_FALSE_ERRNO(ENOENT, dsZipArchive_openDirectory(archive, "director"));
	EXPECT_FALSE_ERRNO(ENOENT, dsZipArchive_openDirectory(archive, "directory "));
	EXPECT_FALSE_ERRNO(ENOENT, dsZipArchive_openDirectory(archive, "directoryy"));

	dsZipArchive_close(archive);
}

TEST_F(ZipArchiveTest, IterateDirectoryFileEntriesOnly)
{
	char path[DS_PATH_MAX];
	ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "files-only.zip"));
	dsZipArchive* archive =
		dsZipArchive_openResource(allocator, dsFileResourceType_Embedded, path, 0);
	ASSERT_TRUE(archive);

	dsDirectoryIterator iterator = dsZipArchive_openDirectory(archive, "./");
	ASSERT_TRUE(iterator);

	char name[DS_FILE_NAME_MAX] = {};
	EXPECT_EQ(dsPathStatus_ExistsDirectory,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("directory", name);
	EXPECT_EQ(dsPathStatus_ExistsFile,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("first", name);
	EXPECT_EQ(dsPathStatus_ExistsFile,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("second", name);
	EXPECT_EQ(dsPathStatus_Missing,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));

	EXPECT_TRUE(dsZipArchive_closeDirectory(archive, iterator));

	iterator = dsZipArchive_openDirectory(archive, "directory");
	ASSERT_TRUE(iterator);

	EXPECT_EQ(dsPathStatus_ExistsFile,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("fourth", name);
	EXPECT_EQ(dsPathStatus_ExistsFile,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("third", name);
	EXPECT_EQ(dsPathStatus_Missing,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));

	EXPECT_TRUE(dsZipArchive_closeDirectory(archive, iterator));

	dsZipArchive_close(archive);
}

TEST_F(ZipArchiveTest, IterateDirectorySharedPrefixes)
{
	char path[DS_PATH_MAX];
	ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "prefix.zip"));
	dsZipArchive* archive =
		dsZipArchive_openResource(allocator, dsFileResourceType_Embedded, path, 0);
	ASSERT_TRUE(archive);

	dsDirectoryIterator iterator = dsZipArchive_openDirectory(archive, ".");
	ASSERT_TRUE(iterator);

	char name[DS_FILE_NAME_MAX] = {};
	EXPECT_EQ(dsPathStatus_ExistsDirectory,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("common", name);
	EXPECT_EQ(dsPathStatus_Missing,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));

	EXPECT_TRUE(dsZipArchive_closeDirectory(archive, iterator));

	iterator = dsZipArchive_openDirectory(archive, "common");
	ASSERT_TRUE(iterator);

	EXPECT_EQ(dsPathStatus_ExistsFile,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("prefix file", name);
	EXPECT_EQ(dsPathStatus_ExistsDirectory,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("prefix", name);
	EXPECT_EQ(dsPathStatus_ExistsFile,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("prefix1", name);
	EXPECT_EQ(dsPathStatus_Missing,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));

	EXPECT_TRUE(dsZipArchive_closeDirectory(archive, iterator));

	iterator = dsZipArchive_openDirectory(archive, "common/prefix");
	ASSERT_TRUE(iterator);

	EXPECT_EQ(dsPathStatus_ExistsFile,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("first", name);
	EXPECT_EQ(dsPathStatus_ExistsFile,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));
	EXPECT_STREQ("second", name);
	EXPECT_EQ(dsPathStatus_Missing,
		dsZipArchive_nextDirectoryEntry(name, sizeof(name), archive, iterator));

	EXPECT_TRUE(dsZipArchive_closeDirectory(archive, iterator));

	dsZipArchive_close(archive);
}

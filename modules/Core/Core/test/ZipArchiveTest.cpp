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
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Streams/FileArchive.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Streams/ZipArchive.h>

#include <gtest/gtest.h>
#include <cstring>

#if DS_ZIP_ARCHIVE_ENABLED

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
	EXPECT_EQ(dsPathStatus_ExistsFile, dsZipArchive_pathStatus(archive, "README.md"));

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

TEST_F(ZipArchiveTest, ReadUncompressed)
{
	char buffer[128];
	char path[DS_PATH_MAX];
	ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "uncompressed.zip"));
	dsZipArchive* archive =
		dsZipArchive_openResource(allocator, dsFileResourceType_Embedded, path, 0);
	ASSERT_TRUE(archive);

	dsStream* stream = dsZipArchive_openFile(archive, "first");
	ASSERT_TRUE(stream);

	const char* expected = "first\n";
	size_t expectedSize = std::strlen(expected);
	EXPECT_EQ(expectedSize, dsStream_remainingBytes(stream));
	EXPECT_EQ(0U, dsStream_tell(stream));
	size_t readSize = dsStream_read(stream, buffer, sizeof(buffer));
	ASSERT_EQ(expectedSize, readSize);
	buffer[readSize] = 0;
	EXPECT_STREQ(expected, buffer);
	EXPECT_EQ(0U, dsStream_remainingBytes(stream));
	EXPECT_EQ(expectedSize, dsStream_tell(stream));

	EXPECT_TRUE(dsStream_close(stream));

	stream = dsZipArchive_openFile(archive, "second");
	ASSERT_TRUE(stream);

	expected = "second\n";
	expectedSize = std::strlen(expected);
	EXPECT_EQ(expectedSize, dsStream_remainingBytes(stream));
	readSize = dsStream_read(stream, buffer, 2);
	ASSERT_EQ(2U, readSize);
	buffer[readSize] = 0;
	EXPECT_STREQ("se", buffer);
	EXPECT_EQ(2U, dsStream_tell(stream));

	EXPECT_EQ(expectedSize - 2, dsStream_remainingBytes(stream));
	readSize = dsStream_read(stream, buffer, 2);
	ASSERT_EQ(2U, readSize);
	buffer[readSize] = 0;
	EXPECT_STREQ("co", buffer);
	EXPECT_EQ(4U, dsStream_tell(stream));

	EXPECT_EQ(expectedSize - 4, dsStream_remainingBytes(stream));
	readSize = dsStream_read(stream, buffer, sizeof(buffer));
	ASSERT_EQ(expectedSize - 4, readSize);
	buffer[readSize] = 0;
	EXPECT_STREQ(expected + 4, buffer);
	EXPECT_EQ(0U, dsStream_remainingBytes(stream));
	EXPECT_EQ(expectedSize, dsStream_tell(stream));

	EXPECT_TRUE(dsStream_restart(stream));
	EXPECT_EQ(expectedSize, dsStream_remainingBytes(stream));
	readSize = dsStream_read(stream, buffer, sizeof(buffer));
	ASSERT_EQ(expectedSize, readSize);
	buffer[readSize] = 0;
	EXPECT_STREQ(expected, buffer);
	EXPECT_EQ(0U, dsStream_remainingBytes(stream));
	EXPECT_EQ(expectedSize, dsStream_tell(stream));

	EXPECT_TRUE(dsStream_close(stream));

	stream = dsZipArchive_openFile(archive, "directory/third");
	ASSERT_TRUE(stream);

	expected = "third\n";
	readSize = dsStream_read(stream, buffer, sizeof(buffer));
	ASSERT_EQ(std::strlen(expected), readSize);
	buffer[readSize] = 0;
	EXPECT_STREQ(expected, buffer);
	EXPECT_EQ(0U, dsStream_remainingBytes(stream));

	EXPECT_TRUE(dsStream_close(stream));

	stream = dsZipArchive_openFile(archive, "directory/fourth");
	ASSERT_TRUE(stream);

	expected = "fourth\n";
	readSize = dsStream_read(stream, buffer, sizeof(buffer));
	ASSERT_EQ(std::strlen(expected), readSize);
	buffer[readSize] = 0;
	EXPECT_STREQ(expected, buffer);
	EXPECT_EQ(0U, dsStream_remainingBytes(stream));

	EXPECT_TRUE(dsStream_close(stream));

	dsZipArchive_close(archive);
}

TEST_F(ZipArchiveTest, ReadWithComments)
{
	char buffer[128];
	char path[DS_PATH_MAX];
	ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "comments.zip"));
	dsZipArchive* archive =
		dsZipArchive_openResource(allocator, dsFileResourceType_Embedded, path, 0);
	ASSERT_TRUE(archive);

	dsStream* stream = dsZipArchive_openFile(archive, "first");
	ASSERT_TRUE(stream);

	const char* expected = "first\n";
	EXPECT_EQ(0U, dsStream_tell(stream));
	size_t readSize = dsStream_read(stream, buffer, sizeof(buffer));
	ASSERT_EQ(strlen(expected), readSize);
	buffer[readSize] = 0;
	EXPECT_STREQ(expected, buffer);

	EXPECT_TRUE(dsStream_close(stream));

	stream = dsZipArchive_openFile(archive, "second");
	ASSERT_TRUE(stream);

	expected = "second\n";
	EXPECT_EQ(0U, dsStream_tell(stream));
	readSize = dsStream_read(stream, buffer, sizeof(buffer));
	ASSERT_EQ(strlen(expected), readSize);
	buffer[readSize] = 0;
	EXPECT_STREQ(expected, buffer);

	EXPECT_TRUE(dsStream_close(stream));

	dsZipArchive_close(archive);
}

TEST_F(ZipArchiveTest, UncompressedSeek)
{
	char buffer[128];
	char path[DS_PATH_MAX];
	ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "uncompressed.zip"));
	dsZipArchive* archive =
		dsZipArchive_openResource(allocator, dsFileResourceType_Embedded, path, 0);
	ASSERT_TRUE(archive);

	const char* expected = "second\n";
	size_t expectedSize = std::strlen(expected);

	dsStream* stream = dsZipArchive_openFile(archive, "second");
	ASSERT_TRUE(stream);

	EXPECT_FALSE_ERRNO(EINVAL, dsStream_seek(stream, -10, dsStreamSeekWay_Current));
	EXPECT_FALSE_ERRNO(EINVAL, dsStream_seek(stream, -10, dsStreamSeekWay_Beginning));
	EXPECT_FALSE_ERRNO(EINVAL, dsStream_seek(stream, 10, dsStreamSeekWay_End));

	EXPECT_TRUE(dsStream_seek(stream, -3, dsStreamSeekWay_End));
	EXPECT_EQ(expectedSize - 3U, dsStream_tell(stream));
	EXPECT_EQ(3U, dsStream_remainingBytes(stream));

	size_t readSize = dsStream_read(stream, buffer, sizeof(buffer));
	ASSERT_EQ(3U, readSize);
	buffer[readSize] = 0;
	EXPECT_STREQ("nd\n", buffer);
	EXPECT_EQ(expectedSize, dsStream_tell(stream));
	EXPECT_EQ(0U, dsStream_remainingBytes(stream));

	EXPECT_TRUE(dsStream_seek(stream, 0, dsStreamSeekWay_Beginning));
	EXPECT_EQ(0U, dsStream_tell(stream));
	EXPECT_EQ(expectedSize, dsStream_remainingBytes(stream));

	readSize = dsStream_read(stream, buffer, 2);
	ASSERT_EQ(2U, readSize);
	buffer[readSize] = 0;
	EXPECT_STREQ("se", buffer);
	EXPECT_EQ(2U, dsStream_tell(stream));
	EXPECT_EQ(expectedSize - 2, dsStream_remainingBytes(stream));

	EXPECT_TRUE(dsStream_seek(stream, 2, dsStreamSeekWay_Current));
	EXPECT_EQ(4U, dsStream_tell(stream));
	EXPECT_EQ(expectedSize - 4, dsStream_remainingBytes(stream));

	readSize = dsStream_read(stream, buffer, sizeof(buffer));
	ASSERT_EQ(expectedSize - 4, readSize);
	buffer[readSize] = 0;
	EXPECT_STREQ(expected + 4, buffer);
	EXPECT_EQ(expectedSize, dsStream_tell(stream));
	EXPECT_EQ(0U, dsStream_remainingBytes(stream));

	EXPECT_TRUE(dsStream_close(stream));

	dsZipArchive_close(archive);
}

TEST_F(ZipArchiveTest, ReadCompressed)
{
	char buffer[1024];
	char path[DS_PATH_MAX];
	ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "large.zip"));
	dsZipArchive* archive = dsZipArchive_openResource(allocator, dsFileResourceType_Embedded, path,
		DS_MIN_ZIP_DECOMPRESS_BUFFER_SIZE);
	ASSERT_TRUE(archive);

	dsStream* stream = dsZipArchive_openFile(archive, "large");
	ASSERT_TRUE(stream);

	memset(buffer, 0xFF, sizeof(buffer));
	EXPECT_EQ(sizeof(buffer), dsStream_read(stream, buffer, sizeof(buffer)));
	for (size_t i = 0; i < sizeof(buffer); ++i)
		EXPECT_EQ(0, buffer[i]);

	EXPECT_TRUE(dsStream_close(stream));

	stream = dsZipArchive_openFile(archive, "largest32");
	ASSERT_TRUE(stream);

	memset(buffer, 0xFF, sizeof(buffer));
	EXPECT_EQ(127U, dsStream_read(stream, buffer, 127));
	EXPECT_EQ(sizeof(buffer) - 127, dsStream_read(stream, buffer + 127, sizeof(buffer) - 127));
	for (size_t i = 0; i < sizeof(buffer); ++i)
		EXPECT_EQ(0, buffer[i]);

	EXPECT_TRUE(dsStream_close(stream));

	stream = dsZipArchive_openFile(archive, "README.md");
	ASSERT_TRUE(stream);

	size_t fileSize = 16901;
	EXPECT_EQ(fileSize, dsStream_remainingBytes(stream));

	const char* expected = "# Introduction";
	size_t readSize = 62;
	EXPECT_EQ(readSize, dsStream_read(stream, buffer, readSize));
	EXPECT_EQ(readSize, dsStream_tell(stream));
	EXPECT_EQ(fileSize - readSize, dsStream_remainingBytes(stream));
	EXPECT_EQ(0, std::strncmp(buffer, expected, std::strlen(expected)));

	expected = "actions/workflows/main.yml";
	readSize += sizeof(buffer);
	EXPECT_EQ(sizeof(buffer), dsStream_read(stream, buffer, sizeof(buffer)));
	EXPECT_EQ(readSize, dsStream_tell(stream));
	EXPECT_EQ(fileSize - readSize, dsStream_remainingBytes(stream));
	EXPECT_EQ(0, std::strncmp(buffer, expected, std::strlen(expected)));

	expected = ".com/harfbuzz/harfbuzz) (required for text)";
	readSize += sizeof(buffer);
	EXPECT_EQ(sizeof(buffer), dsStream_read(stream, buffer, sizeof(buffer)));
	EXPECT_EQ(readSize, dsStream_tell(stream));
	EXPECT_EQ(fileSize - readSize, dsStream_remainingBytes(stream));
	EXPECT_EQ(0, std::strncmp(buffer, expected, std::strlen(expected)));

	size_t thisRead;
	do
	{
		thisRead = dsStream_read(stream, buffer, sizeof(buffer));
		readSize += thisRead;
		EXPECT_EQ(readSize, dsStream_tell(stream));
		EXPECT_EQ(fileSize - readSize, dsStream_remainingBytes(stream));
	} while (thisRead > 0);

	EXPECT_EQ(fileSize, readSize);
	EXPECT_EQ(fileSize, dsStream_tell(stream));
	EXPECT_EQ(0U, dsStream_remainingBytes(stream));

	EXPECT_EQ(0U, dsStream_read(stream, buffer, sizeof(buffer)));
	EXPECT_EQ(fileSize, dsStream_tell(stream));
	EXPECT_EQ(0U, dsStream_remainingBytes(stream));

	EXPECT_TRUE(dsStream_restart(stream));
	EXPECT_EQ(0U, dsStream_tell(stream));
	EXPECT_EQ(fileSize, dsStream_remainingBytes(stream));

	expected = "# Introduction";
	readSize = 62;
	EXPECT_EQ(readSize, dsStream_read(stream, buffer, readSize));
	EXPECT_EQ(readSize, dsStream_tell(stream));
	EXPECT_EQ(fileSize - readSize, dsStream_remainingBytes(stream));
	EXPECT_EQ(0, std::strncmp(buffer, expected, std::strlen(expected)));

	EXPECT_TRUE(dsStream_close(stream));

	dsZipArchive_close(archive);
}

TEST_F(ZipArchiveTest, FileArchiveFunctions)
{
	char buffer[32] = {};
	char name[DS_FILE_NAME_MAX] = {};
	char path[DS_PATH_MAX];
	ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "simple.zip"));
	auto archive = reinterpret_cast<dsFileArchive*>(
		dsZipArchive_openResource(allocator, dsFileResourceType_Embedded, path, 0));
	ASSERT_TRUE(archive);

	EXPECT_EQ(dsPathStatus_ExistsFile, dsFileArchive_pathStatus(archive, "first"));

	dsStream* stream = dsFileArchive_openFile(archive, "first");
	ASSERT_TRUE(stream);

	EXPECT_EQ(6U, dsStream_read(stream, buffer, sizeof(buffer)));
	EXPECT_STREQ("first\n", buffer);

	EXPECT_TRUE(dsStream_close(stream));

	dsDirectoryIterator iter = dsFileArchive_openDirectory(archive, "directory");
	ASSERT_TRUE(iter);
	EXPECT_EQ(dsPathStatus_ExistsFile,
		dsFileArchive_nextDirectoryEntry(name, sizeof(name), archive, iter));
	EXPECT_STREQ("fourth", name);
	EXPECT_TRUE(dsFileArchive_closeDirectory(archive, iter));

	dsFileArchive_close(archive);
}

#endif

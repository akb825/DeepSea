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

#include "Helpers.h"

#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Streams/Stream.h>

#include <gtest/gtest.h>
#include <string>
#include <unordered_map>

TEST(FileStream, Null)
{
	int32_t dummyData;
	EXPECT_EQ_ERRNO(EINVAL, 0U, dsFileStream_read(NULL, &dummyData, sizeof(dummyData)));
	EXPECT_EQ_ERRNO(EINVAL, 0U, dsFileStream_write(NULL, &dummyData, sizeof(dummyData)));
	EXPECT_FALSE_ERRNO(EINVAL, dsFileStream_seek(NULL, 0, dsStreamSeekWay_Beginning));
	EXPECT_EQ_ERRNO(EINVAL, DS_STREAM_INVALID_POS, dsFileStream_tell(NULL));
	EXPECT_FALSE_ERRNO(EINVAL, dsFileStream_close(NULL));
}

TEST(FileStream, Empty)
{
	dsFileStream stream = {};
	int32_t dummyData;
	EXPECT_EQ_ERRNO(EINVAL, 0U, dsFileStream_read(&stream, &dummyData, sizeof(dummyData)));
	EXPECT_EQ_ERRNO(EINVAL, 0U, dsFileStream_write(&stream, &dummyData, sizeof(dummyData)));
	EXPECT_FALSE_ERRNO(EINVAL, dsFileStream_seek(&stream, 0, dsStreamSeekWay_Beginning));
	EXPECT_EQ_ERRNO(EINVAL, DS_STREAM_INVALID_POS, dsFileStream_tell(&stream));
	EXPECT_FALSE_ERRNO(EINVAL, dsFileStream_close(&stream));
}

TEST(FileStream, InvalidOpen)
{
	dsFileStream stream = {};
	EXPECT_FALSE_ERRNO(EINVAL, dsFileStream_openPath(NULL, "asdf", "w"));
	EXPECT_FALSE_ERRNO(EINVAL, dsFileStream_openPath(&stream, NULL, "w"));
	EXPECT_FALSE_ERRNO(EINVAL, dsFileStream_openPath(&stream, "asdf", NULL));

	EXPECT_FALSE_ERRNO(EINVAL, dsFileStream_openFile(&stream, NULL));
}

// TODO: iOS has restrictive filesystem permissions and we don't have the application code available
// here to set the proper directory.
#if !DS_IOS

TEST(FileStream, ReadWriteFileFunctions)
{
	dsFileStream stream;
	int32_t dummyData;

	char path[DS_PATH_MAX];
	ASSERT_TRUE(dsResourceStream_getPath( path, sizeof(path), dsFileResourceType_Dynamic, "asdf"));

	ASSERT_TRUE(dsFileStream_openPath(&stream, path, "w"));
	dummyData = 1;
	EXPECT_EQ(sizeof(dummyData), dsFileStream_write(&stream, &dummyData, sizeof(dummyData)));
	dummyData = 2;
	EXPECT_EQ(sizeof(dummyData), dsFileStream_write(&stream, &dummyData, sizeof(dummyData)));

	EXPECT_EQ(2*sizeof(dummyData), dsFileStream_tell(&stream));
	EXPECT_TRUE(dsFileStream_seek(&stream, 0, dsStreamSeekWay_Beginning));
	EXPECT_EQ(2*sizeof(dummyData), dsFileStream_remainingBytes(&stream));
	EXPECT_EQ(0U, dsFileStream_tell(&stream));
	EXPECT_TRUE(dsFileStream_seek(&stream, 3, dsStreamSeekWay_Current));
	EXPECT_EQ(3U, dsFileStream_tell(&stream));
	EXPECT_TRUE(dsFileStream_seek(&stream, 2, dsStreamSeekWay_Current));
	EXPECT_EQ(5U, dsFileStream_tell(&stream));
	EXPECT_EQ(2*sizeof(dummyData) - 5U, dsFileStream_remainingBytes(&stream));
	EXPECT_TRUE(dsFileStream_seek(&stream, 0, dsStreamSeekWay_End));
	EXPECT_EQ(2*sizeof(dummyData), dsFileStream_tell(&stream));
	EXPECT_FALSE(dsFileStream_seek(&stream, -4, dsStreamSeekWay_Beginning));
	EXPECT_EQ(2*sizeof(dummyData), dsFileStream_tell(&stream));

	EXPECT_TRUE(dsFileStream_close(&stream));

	FILE* file = fopen(path, "r");
	ASSERT_TRUE(file);
	ASSERT_TRUE(dsFileStream_openFile(&stream, file));
	EXPECT_EQ(sizeof(dummyData), dsFileStream_read(&stream, &dummyData, sizeof(dummyData)));
	EXPECT_EQ(1, dummyData);

	EXPECT_EQ(sizeof(dummyData)/2, dsFileStream_read(&stream, &dummyData, sizeof(dummyData)/2));
	EXPECT_EQ(sizeof(dummyData)/2, dsFileStream_read(&stream,
		(uint8_t*)&dummyData + sizeof(dummyData)/2, sizeof(dummyData)));
	EXPECT_EQ(2, dummyData);

	EXPECT_TRUE(dsFileStream_close(&stream));
	EXPECT_FALSE_ERRNO(EINVAL, dsFileStream_close(&stream));

	EXPECT_TRUE(dsFileStream_removeFile(path));
}

TEST(FileStream, ReadWriteStreamFunctions)
{
	dsFileStream stream;
	int32_t dummyData;

	char path[DS_PATH_MAX];
	ASSERT_TRUE(dsResourceStream_getPath( path, sizeof(path), dsFileResourceType_Dynamic, "asdf"));

	ASSERT_TRUE(dsFileStream_openPath(&stream, path, "w"));
	dummyData = 1;
	EXPECT_EQ(sizeof(dummyData), dsStream_write((dsStream*)&stream, &dummyData, sizeof(dummyData)));
	dummyData = 2;
	EXPECT_EQ(sizeof(dummyData), dsStream_write((dsStream*)&stream, &dummyData, sizeof(dummyData)));

	EXPECT_EQ(2*sizeof(dummyData), dsStream_tell((dsStream*)&stream));
	EXPECT_TRUE(dsStream_seek((dsStream*)&stream, 0, dsStreamSeekWay_Beginning));
	EXPECT_EQ(2*sizeof(dummyData), dsStream_remainingBytes((dsStream*)&stream));
	EXPECT_EQ(0U, dsStream_tell((dsStream*)&stream));
	EXPECT_TRUE(dsStream_seek((dsStream*)&stream, 3, dsStreamSeekWay_Current));
	EXPECT_EQ(3U, dsStream_tell((dsStream*)&stream));
	EXPECT_TRUE(dsStream_seek((dsStream*)&stream, 2, dsStreamSeekWay_Current));
	EXPECT_EQ(5U, dsStream_tell((dsStream*)&stream));
	EXPECT_EQ(2*sizeof(dummyData) - 5, dsStream_remainingBytes((dsStream*)&stream));
	EXPECT_TRUE(dsStream_seek((dsStream*)&stream, 0, dsStreamSeekWay_End));
	EXPECT_EQ(2*sizeof(dummyData), dsStream_tell((dsStream*)&stream));
	EXPECT_FALSE(dsStream_seek((dsStream*)&stream, -4, dsStreamSeekWay_Beginning));
	EXPECT_EQ(2*sizeof(dummyData), dsStream_tell((dsStream*)&stream));

	EXPECT_TRUE(dsStream_close((dsStream*)&stream));

	FILE* file = fopen(path, "r");
	ASSERT_TRUE(dsFileStream_openFile(&stream, file));
	EXPECT_EQ(sizeof(dummyData), dsStream_read((dsStream*)&stream, &dummyData, sizeof(dummyData)));
	EXPECT_EQ(1, dummyData);

	EXPECT_EQ(sizeof(dummyData)/2, dsStream_read((dsStream*)&stream, &dummyData,
		sizeof(dummyData)/2));
	EXPECT_EQ(sizeof(dummyData)/2, dsStream_read((dsStream*)&stream,
		(uint8_t*)&dummyData + sizeof(dummyData)/2, sizeof(dummyData)));
	EXPECT_EQ(2, dummyData);

	EXPECT_TRUE(dsStream_seek((dsStream*)&stream, 0, dsStreamSeekWay_Beginning));
	EXPECT_EQ(2U, dsStream_skip((dsStream*)&stream, 2));
	EXPECT_EQ(2U, dsStream_tell((dsStream*)&stream));
	EXPECT_EQ(6U, dsStream_skip((dsStream*)&stream, 10));

	EXPECT_TRUE(dsStream_seek((dsStream*)&stream, 0, dsStreamSeekWay_Beginning));
	EXPECT_EQ(1000000U, dsStream_skip((dsStream*)&stream, 1000000));
	EXPECT_EQ(1000000U, dsStream_tell((dsStream*)&stream));
	EXPECT_EQ(0U, dsStream_read((dsStream*)&stream, &dummyData, sizeof(dummyData)));

	EXPECT_TRUE(dsStream_close((dsStream*)&stream));
	EXPECT_FALSE_ERRNO(EINVAL, dsStream_close((dsStream*)&stream));

	EXPECT_TRUE(dsFileStream_removeFile(path));
}

class FileStreamDirectory : public testing::Test
{
public:
	char rootDir[DS_PATH_MAX];
	char firstPath[DS_PATH_MAX];
	char secondPath[DS_PATH_MAX];
	char thirdPath[DS_PATH_MAX];

	void SetUp() override
	{
		ASSERT_TRUE(dsResourceStream_getPath(rootDir, sizeof(rootDir),
			dsFileResourceType_Dynamic, "DirectoryIteratorTest"));
		ASSERT_TRUE(dsPath_combine(firstPath, sizeof(firstPath), rootDir, "first"));
		ASSERT_TRUE(dsPath_combine(secondPath, sizeof(secondPath), rootDir, "second"));
		ASSERT_TRUE(dsPath_combine(thirdPath, sizeof(thirdPath), rootDir, "third"));
		ASSERT_TRUE(dsFileStream_createDirectory(rootDir));
	}

	void TearDown() override
	{
		EXPECT_TRUE(dsFileStream_removeFile(firstPath));
		EXPECT_TRUE(dsFileStream_removeFile(secondPath));
		EXPECT_TRUE(dsFileStream_removeDirectory(thirdPath));
		EXPECT_TRUE(dsFileStream_removeDirectory(rootDir));
	}
};

TEST_F(FileStreamDirectory, DirectoryIterator)
{
	EXPECT_FALSE_ERRNO(EINVAL, dsFileStream_openDirectory(NULL));
	EXPECT_FALSE_ERRNO(EINVAL, dsFileStream_openDirectory(""));

	EXPECT_EQ(dsPathStatus_Missing, dsFileStream_pathStatus(firstPath));
	EXPECT_FALSE(dsFileStream_openDirectory(firstPath));
	EXPECT_TRUE(errno == ENOENT || errno == ENOTDIR);

	dsFileStream stream;
	ASSERT_TRUE(dsFileStream_openPath(&stream, firstPath, "w"));
	EXPECT_TRUE(dsFileStream_close(&stream));
	EXPECT_EQ(dsPathStatus_ExistsFile, dsFileStream_pathStatus(firstPath));

	ASSERT_TRUE(dsFileStream_openPath(&stream, secondPath, "w"));
	EXPECT_TRUE(dsFileStream_close(&stream));
	EXPECT_EQ(dsPathStatus_ExistsFile, dsFileStream_pathStatus(secondPath));

	ASSERT_TRUE(dsFileStream_createDirectory(thirdPath));
	EXPECT_EQ(dsPathStatus_ExistsDirectory, dsFileStream_pathStatus(thirdPath));

	dsDirectoryIterator iterator = dsFileStream_openDirectory(rootDir);
	ASSERT_TRUE(iterator);

	std::unordered_map<std::string, bool> entries;
	dsPathStatus result;
	do
	{
		char entry[DS_FILE_NAME_MAX];
		result = dsFileStream_nextDirectoryEntry(entry, sizeof(entry), iterator);
		if (result > dsPathStatus_Missing)
			entries.emplace(entry, result == dsPathStatus_ExistsDirectory);
	} while (result > dsPathStatus_Missing);
	EXPECT_EQ(dsPathStatus_Missing, result);
	EXPECT_TRUE(dsFileStream_closeDirectory(iterator));

	std::unordered_map<std::string, bool> expectedEntries = {{"first", false},
		{"second", false}, {"third", true}};
	EXPECT_EQ(expectedEntries, entries);
}

#endif // !DS_IOS

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
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <gtest/gtest.h>
#include <stdlib.h>

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

// TODO: Currently don't have default directories set for mobile platforms.
#if !DS_IOS && !DS_ANDROID

TEST(FileStream, ReadWriteFileFunctions)
{
	dsFileStream stream;
	int32_t dummyData;

	char path[DS_PATH_MAX];
	ASSERT_TRUE(dsPath_combine(path, sizeof(path),
		dsResourceStream_getDirectory(dsFileResourceType_Dynamic), "asdf"));

	ASSERT_TRUE(dsFileStream_openPath(&stream, path, "w"));
	dummyData = 1;
	EXPECT_EQ(sizeof(dummyData), dsFileStream_write(&stream, &dummyData, sizeof(dummyData)));
	dummyData = 2;
	EXPECT_EQ(sizeof(dummyData), dsFileStream_write(&stream, &dummyData, sizeof(dummyData)));

	EXPECT_EQ(2*sizeof(dummyData), dsFileStream_tell(&stream));
	EXPECT_TRUE(dsFileStream_seek(&stream, 0, dsStreamSeekWay_Beginning));
	EXPECT_EQ(0U, dsFileStream_tell(&stream));
	EXPECT_TRUE(dsFileStream_seek(&stream, 3, dsStreamSeekWay_Current));
	EXPECT_EQ(3U, dsFileStream_tell(&stream));
	EXPECT_TRUE(dsFileStream_seek(&stream, 2, dsStreamSeekWay_Current));
	EXPECT_EQ(5U, dsFileStream_tell(&stream));
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

	unlink(path);
}

TEST(FileStream, ReadWriteStreamFunctions)
{
	dsFileStream stream;
	int32_t dummyData;

	char path[DS_PATH_MAX];
	ASSERT_TRUE(dsPath_combine(path, sizeof(path),
		dsResourceStream_getDirectory(dsFileResourceType_Dynamic), "asdf"));

	ASSERT_TRUE(dsFileStream_openPath(&stream, path, "w"));
	dummyData = 1;
	EXPECT_EQ(sizeof(dummyData), dsStream_write((dsStream*)&stream, &dummyData, sizeof(dummyData)));
	dummyData = 2;
	EXPECT_EQ(sizeof(dummyData), dsStream_write((dsStream*)&stream, &dummyData, sizeof(dummyData)));

	EXPECT_EQ(2*sizeof(dummyData), dsStream_tell((dsStream*)&stream));
	EXPECT_TRUE(dsStream_seek((dsStream*)&stream, 0, dsStreamSeekWay_Beginning));
	EXPECT_EQ(0U, dsStream_tell((dsStream*)&stream));
	EXPECT_TRUE(dsStream_seek((dsStream*)&stream, 3, dsStreamSeekWay_Current));
	EXPECT_EQ(3U, dsStream_tell((dsStream*)&stream));
	EXPECT_TRUE(dsStream_seek((dsStream*)&stream, 2, dsStreamSeekWay_Current));
	EXPECT_EQ(5U, dsStream_tell((dsStream*)&stream));
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

	unlink(path);
}

#endif // DS_IOS

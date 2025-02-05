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
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Streams/MemoryStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <gtest/gtest.h>
#include <stdlib.h>

TEST(MemoryStream, Null)
{
	int32_t dummyData;
	EXPECT_EQ_ERRNO(EINVAL, 0U, dsMemoryStream_read(NULL, &dummyData, sizeof(dummyData)));
	EXPECT_EQ_ERRNO(EINVAL, 0U, dsMemoryStream_write(NULL, &dummyData, sizeof(dummyData)));
	EXPECT_FALSE_ERRNO(EINVAL, dsMemoryStream_seek(NULL, 0, dsStreamSeekWay_Beginning));
	EXPECT_EQ_ERRNO(EINVAL, DS_STREAM_INVALID_POS, dsMemoryStream_tell(NULL));
	EXPECT_FALSE_ERRNO(EINVAL, dsMemoryStream_close(NULL));
}

TEST(MemoryStream, Empty)
{
	dsMemoryStream stream = {};
	int32_t dummyData;
	EXPECT_EQ_ERRNO(EINVAL, 0U, dsMemoryStream_read(&stream, &dummyData, sizeof(dummyData)));
	EXPECT_EQ_ERRNO(EINVAL, 0U, dsMemoryStream_write(&stream, &dummyData, sizeof(dummyData)));
	EXPECT_FALSE_ERRNO(EINVAL, dsMemoryStream_seek(&stream, 0, dsStreamSeekWay_Beginning));
	EXPECT_EQ_ERRNO(EINVAL, DS_STREAM_INVALID_POS, dsMemoryStream_tell(&stream));
	EXPECT_FALSE_ERRNO(EINVAL, dsMemoryStream_close(&stream));
}

TEST(MemoryStream, InvalidOpen)
{
	dsMemoryStream stream = {};
	int32_t buffer[2];
	EXPECT_FALSE_ERRNO(EINVAL, dsMemoryStream_open(NULL, buffer, sizeof(buffer)));
	EXPECT_FALSE_ERRNO(EINVAL, dsMemoryStream_open(&stream, NULL, sizeof(buffer)));
}

TEST(MemoryStream, ReadWriteFileFunctions)
{
	dsMemoryStream stream;
	int32_t buffer[2];
	int32_t dummyData;

	EXPECT_TRUE(dsMemoryStream_open(&stream, buffer, sizeof(buffer)));
	dummyData = 1;
	EXPECT_EQ(sizeof(dummyData), dsMemoryStream_write(&stream, &dummyData, sizeof(dummyData)));
	dummyData = 2;
	EXPECT_EQ(sizeof(dummyData), dsMemoryStream_write(&stream, &dummyData, sizeof(dummyData)));
	dummyData = 3;
	EXPECT_EQ(0U, dsMemoryStream_write(&stream, &dummyData, sizeof(dummyData)));

	EXPECT_EQ(2*sizeof(dummyData), dsMemoryStream_tell(&stream));
	EXPECT_TRUE(dsMemoryStream_seek(&stream, 0, dsStreamSeekWay_Beginning));
	EXPECT_EQ(0U, dsMemoryStream_tell(&stream));
	EXPECT_TRUE(dsMemoryStream_seek(&stream, 3, dsStreamSeekWay_Current));
	EXPECT_EQ(3U, dsMemoryStream_tell(&stream));
	EXPECT_TRUE(dsMemoryStream_seek(&stream, 2, dsStreamSeekWay_Current));
	EXPECT_EQ(5U, dsMemoryStream_tell(&stream));
	EXPECT_TRUE(dsMemoryStream_seek(&stream, 0, dsStreamSeekWay_End));
	EXPECT_EQ(2*sizeof(dummyData), dsMemoryStream_tell(&stream));
	EXPECT_FALSE(dsMemoryStream_seek(&stream, 4, dsStreamSeekWay_End));
	EXPECT_FALSE(dsMemoryStream_seek(&stream, -4, dsStreamSeekWay_Beginning));
	EXPECT_EQ(2*sizeof(dummyData), dsMemoryStream_tell(&stream));

	EXPECT_TRUE(dsMemoryStream_close(&stream));

	EXPECT_TRUE(dsMemoryStream_open(&stream, buffer, sizeof(buffer)));
	EXPECT_EQ(sizeof(dummyData), dsMemoryStream_read(&stream, &dummyData, sizeof(dummyData)));
	EXPECT_EQ(1, dummyData);

	EXPECT_EQ(sizeof(dummyData)/2, dsMemoryStream_read(&stream, &dummyData, sizeof(dummyData)/2));
	EXPECT_EQ(sizeof(dummyData)/2, dsMemoryStream_read(&stream,
		(uint8_t*)&dummyData + sizeof(dummyData)/2, sizeof(dummyData)));
	EXPECT_EQ(2, dummyData);

	EXPECT_TRUE(dsMemoryStream_close(&stream));
	EXPECT_FALSE_ERRNO(EINVAL, dsMemoryStream_close(&stream));
}

TEST(MemoryStream, ReadWriteStreamFunctions)
{
	dsMemoryStream stream;
	int32_t buffer[2];
	int32_t dummyData;

	EXPECT_TRUE(dsMemoryStream_open(&stream, buffer, sizeof(buffer)));
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
	EXPECT_FALSE(dsStream_seek((dsStream*)&stream, 4, dsStreamSeekWay_End));
	EXPECT_FALSE(dsStream_seek((dsStream*)&stream, -4, dsStreamSeekWay_Beginning));
	EXPECT_EQ(2*sizeof(dummyData), dsStream_tell((dsStream*)&stream));

	EXPECT_TRUE(dsStream_close((dsStream*)&stream));

	EXPECT_TRUE(dsMemoryStream_open(&stream, buffer, sizeof(buffer)));
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
	EXPECT_EQ(0U, dsStream_skip((dsStream*)&stream, 1000000));

	EXPECT_TRUE(dsStream_close((dsStream*)&stream));
	EXPECT_FALSE_ERRNO(EINVAL, dsStream_close((dsStream*)&stream));
}

TEST(MemoryStream, ReadUntilEnd)
{
	dsSystemAllocator allocator;
	EXPECT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));

	dsMemoryStream stream;
	int32_t buffer[3] = {0, 1, 2};

	EXPECT_TRUE(dsMemoryStream_open(&stream, buffer, sizeof(buffer)));
	EXPECT_TRUE(dsMemoryStream_seek(&stream, sizeof(uint32_t), dsStreamSeekWay_Current));

	size_t size;
	int32_t* data = (int32_t*)dsStream_readUntilEnd(&size, (dsStream*)&stream,
		(dsAllocator*)&allocator);
	ASSERT_EQ(sizeof(uint32_t)*2, size);
	ASSERT_TRUE(data);

	EXPECT_EQ(1, data[0]);
	EXPECT_EQ(2, data[1]);

	EXPECT_TRUE(dsAllocator_free((dsAllocator*)&allocator, data));
	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
}

TEST(MemoryStream, ReadUntilEndNoRemainingBytes)
{
	dsSystemAllocator allocator;
	EXPECT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));

	dsMemoryStream stream;
	int32_t buffer[3] = {0, 1, 2};

	EXPECT_TRUE(dsMemoryStream_open(&stream, buffer, sizeof(buffer)));
	EXPECT_TRUE(dsMemoryStream_seek(&stream, sizeof(uint32_t), dsStreamSeekWay_Current));
	((dsStream*)&stream)->seekFunc = NULL;
	((dsStream*)&stream)->remainingBytesFunc = NULL;
	EXPECT_FALSE(dsStream_canGetRemainingBytes((dsStream*)&stream));

	size_t size;
	int32_t* data = (int32_t*)dsStream_readUntilEnd(&size, (dsStream*)&stream,
		(dsAllocator*)&allocator);
	ASSERT_EQ(sizeof(uint32_t)*2, size);
	ASSERT_TRUE(data);

	EXPECT_EQ(1, data[0]);
	EXPECT_EQ(2, data[1]);

	EXPECT_TRUE(dsAllocator_free((dsAllocator*)&allocator, data));
	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
}

TEST(MemoryStream, ReadUntilEndReuse)
{
	dsSystemAllocator allocator;
	EXPECT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));

	dsMemoryStream stream;
	int32_t buffer[3] = {0, 1, 2};

	EXPECT_TRUE(dsMemoryStream_open(&stream, buffer, sizeof(buffer)));
	EXPECT_TRUE(dsMemoryStream_seek(&stream, sizeof(uint32_t), dsStreamSeekWay_Current));

	void* readData = NULL;
	size_t size = 0;
	size_t capacity = 0;
	ASSERT_TRUE(dsStream_readUntilEndReuse(&readData, &size, &capacity, (dsStream*)&stream,
		(dsAllocator*)&allocator));
	ASSERT_EQ(sizeof(uint32_t)*2, size);
	ASSERT_TRUE(readData);
	uint32_t* data = (uint32_t*)readData;

	EXPECT_EQ(1U, data[0]);
	EXPECT_EQ(2U, data[1]);

	EXPECT_TRUE(dsMemoryStream_close(&stream));
	EXPECT_TRUE(dsMemoryStream_open(&stream, buffer, sizeof(buffer)));
	EXPECT_TRUE(dsMemoryStream_seek(&stream, sizeof(uint32_t), dsStreamSeekWay_Current));

	ASSERT_TRUE(dsStream_readUntilEndReuse(&readData, &size, &capacity, (dsStream*)&stream,
		(dsAllocator*)&allocator));
	ASSERT_EQ(sizeof(uint32_t)*2, size);
	ASSERT_TRUE(readData);
	data = (uint32_t*)readData;

	EXPECT_EQ(1U, data[0]);
	EXPECT_EQ(2U, data[1]);

	EXPECT_TRUE(dsAllocator_free((dsAllocator*)&allocator, data));
	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
}

TEST(MemoryStream, ReadUntilEndReuseNoRemainingBytes)
{
	dsSystemAllocator allocator;
	EXPECT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));

	dsMemoryStream stream;
	int32_t buffer[3] = {0, 1, 2};

	EXPECT_TRUE(dsMemoryStream_open(&stream, buffer, sizeof(buffer)));
	EXPECT_TRUE(dsMemoryStream_seek(&stream, sizeof(uint32_t), dsStreamSeekWay_Current));
	((dsStream*)&stream)->seekFunc = NULL;
	((dsStream*)&stream)->remainingBytesFunc = NULL;
	EXPECT_FALSE(dsStream_canGetRemainingBytes((dsStream*)&stream));

	void* readData = NULL;
	size_t size = 0;
	size_t capacity = 0;
	ASSERT_TRUE(dsStream_readUntilEndReuse(&readData, &size, &capacity, (dsStream*)&stream,
		(dsAllocator*)&allocator));
	ASSERT_EQ(sizeof(uint32_t)*2, size);
	ASSERT_TRUE(readData);
	uint32_t* data = (uint32_t*)readData;

	EXPECT_EQ(1U, data[0]);
	EXPECT_EQ(2U, data[1]);

	EXPECT_TRUE(dsMemoryStream_close(&stream));
	EXPECT_TRUE(dsMemoryStream_open(&stream, buffer, sizeof(buffer)));
	EXPECT_TRUE(dsMemoryStream_seek(&stream, sizeof(uint32_t), dsStreamSeekWay_Current));
	((dsStream*)&stream)->seekFunc = NULL;

	ASSERT_TRUE(dsStream_readUntilEndReuse(&readData, &size, &capacity, (dsStream*)&stream,
		(dsAllocator*)&allocator));
	ASSERT_EQ(sizeof(uint32_t)*2, size);
	ASSERT_TRUE(readData);
	data = (uint32_t*)readData;

	EXPECT_EQ(1U, data[0]);
	EXPECT_EQ(2U, data[1]);

	EXPECT_TRUE(dsAllocator_free((dsAllocator*)&allocator, data));
	EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
}

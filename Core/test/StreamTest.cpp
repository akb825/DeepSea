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
#include <DeepSea/Core/Streams/Stream.h>
#include <gtest/gtest.h>
#include <stdlib.h>

TEST(Stream, Null)
{
	int32_t dummyData;
	EXPECT_EQ_ERRNO(EINVAL, 0U, dsStream_read(NULL, &dummyData, sizeof(dummyData)));
	EXPECT_EQ_ERRNO(EINVAL, 0U, dsStream_write(NULL, &dummyData, sizeof(dummyData)));
	EXPECT_FALSE_ERRNO(EINVAL, dsStream_seek(NULL, 0, dsStreamSeekWay_Beginning));
	EXPECT_EQ_ERRNO(EINVAL, DS_STREAM_INVALID_POS, dsStream_tell(NULL));
	EXPECT_FALSE_ERRNO(EINVAL, dsStream_close(NULL));
}

TEST(Stream, Empty)
{
	dsStream stream = {};
	int32_t dummyData;
	EXPECT_EQ_ERRNO(EINVAL, 0U, dsStream_read(&stream, &dummyData, sizeof(dummyData)));
	EXPECT_EQ_ERRNO(EINVAL, 0U, dsStream_write(&stream, &dummyData, sizeof(dummyData)));
	EXPECT_FALSE_ERRNO(EINVAL, dsStream_seek(&stream, 0, dsStreamSeekWay_Beginning));
	EXPECT_EQ_ERRNO(EINVAL, DS_STREAM_INVALID_POS, dsStream_tell(&stream));
	EXPECT_TRUE(dsStream_close(&stream));
}

/*
 * Copyright 2017 Aaron Barany
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

#include <DeepSea/Core/Config.h>
#include <gtest/gtest.h>
#include <limits.h>
#include <stdint.h>

TEST(ConfigTest, IsBufferRangeValid)
{
	uint32_t offset, rangeSize, bufferSize;

	offset = 0;
	rangeSize = 8;
	bufferSize = 10;
	EXPECT_TRUE(DS_IS_BUFFER_RANGE_VALID(offset, rangeSize, bufferSize));

	offset = 2;
	EXPECT_TRUE(DS_IS_BUFFER_RANGE_VALID(offset, rangeSize, bufferSize));

	offset = 3;
	EXPECT_FALSE(DS_IS_BUFFER_RANGE_VALID(offset, rangeSize, bufferSize));

	offset = UINT_MAX - 10;
	rangeSize = 10;
	bufferSize = UINT_MAX;
	EXPECT_TRUE(DS_IS_BUFFER_RANGE_VALID(offset, rangeSize, bufferSize));

	offset += 2;
	EXPECT_FALSE(DS_IS_BUFFER_RANGE_VALID(offset, rangeSize, bufferSize));

	offset = 0;
	rangeSize = 0;
	bufferSize = 0;
	EXPECT_TRUE(DS_IS_BUFFER_RANGE_VALID(offset, rangeSize, bufferSize));

	offset = 2;
	EXPECT_FALSE(DS_IS_BUFFER_RANGE_VALID(offset, rangeSize, bufferSize));
}

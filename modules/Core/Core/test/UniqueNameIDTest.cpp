/*
 * Copyright 2016-2024 Aaron Barany
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
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/UniqueNameID.h>
#include <gtest/gtest.h>

class UniqueNameIDTest : public testing::Test
{
public:
	UniqueNameIDTest()
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

TEST_F(UniqueNameIDTest, CreateGet)
{
	char test1[] = "test1";

	EXPECT_FALSE_ERRNO(EPERM, dsUniqueNameID_create("test"));

	EXPECT_FALSE(dsUniqueNameID_isInitialized());
	EXPECT_TRUE(dsUniqueNameID_initialize(allocator, DS_DEFAULT_INITIAL_UNIQUE_NAME_ID_LIMIT));
	EXPECT_TRUE(dsUniqueNameID_isInitialized());

	EXPECT_EQ(1U, dsUniqueNameID_create(test1));
	test1[0] = 'b'; // Make sure the original string wasn't kept.
	EXPECT_EQ(2U, dsUniqueNameID_create("test2"));
	EXPECT_EQ(3U, dsUniqueNameID_create("test3"));

	EXPECT_EQ(3U, dsUniqueNameID_create("test3"));
	EXPECT_EQ(2U, dsUniqueNameID_create("test2"));
	EXPECT_EQ(1U, dsUniqueNameID_create("test1"));

	EXPECT_EQ(1U, dsUniqueNameID_get("test1"));
	EXPECT_EQ(2U, dsUniqueNameID_get("test2"));
	EXPECT_EQ(3U, dsUniqueNameID_get("test3"));
	EXPECT_EQ(0U, dsUniqueNameID_get("test4"));
	EXPECT_EQ(0U, dsUniqueNameID_get(test1));

	EXPECT_TRUE(dsUniqueNameID_shutdown());
	EXPECT_FALSE(dsUniqueNameID_isInitialized());
}

TEST_F(UniqueNameIDTest, Rehash)
{
	EXPECT_TRUE(dsUniqueNameID_initialize(allocator, 2));
	EXPECT_EQ(1U, dsUniqueNameID_create("test1"));
	EXPECT_EQ(2U, dsUniqueNameID_create("test2"));
	EXPECT_EQ(3U, dsUniqueNameID_create("test3"));

	EXPECT_EQ(1U, dsUniqueNameID_get("test1"));
	EXPECT_EQ(2U, dsUniqueNameID_get("test2"));
	EXPECT_EQ(3U, dsUniqueNameID_get("test3"));
	EXPECT_TRUE(dsUniqueNameID_shutdown());
}

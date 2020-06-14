/*
 * Copyright 2020 Aaron Barany
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

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Text/TextSubstitutionData.h>
#include <DeepSea/Text/TextSubstitutionTable.h>

#include <gtest/gtest.h>

class TextSubstitutionTableTest : public testing::Test
{
public:
	void SetUp() override
	{
		dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT);
	}

	void TearDown() override
	{
		EXPECT_EQ(0U, allocator.allocator.size);
	}

	dsSystemAllocator allocator;
};

TEST_F(TextSubstitutionTableTest, Create)
{
	EXPECT_FALSE(dsTextSubstitutionTable_create(nullptr, 3));
	EXPECT_FALSE(dsTextSubstitutionTable_create((dsAllocator*)&allocator, 0));
	dsTextSubstitutionTable* table = dsTextSubstitutionTable_create((dsAllocator*)&allocator, 3);
	EXPECT_TRUE(table);
	dsTextSubstitutionTable_destroy(table);
}

TEST_F(TextSubstitutionTableTest, CreateData)
{
	EXPECT_FALSE(dsTextSubstitutionData_create(nullptr));
	dsTextSubstitutionData* data = dsTextSubstitutionData_create((dsAllocator*)&allocator);
	EXPECT_TRUE(data);
	dsTextSubstitutionData_destroy(data);
}

TEST_F(TextSubstitutionTableTest, SetString)
{
	dsTextSubstitutionTable* table = dsTextSubstitutionTable_create((dsAllocator*)&allocator, 3);
	ASSERT_TRUE(table);

	EXPECT_EQ(3U, dsTextSubstitutionTable_getRemainingStrings(table));
	EXPECT_TRUE(dsTextSubstitutionTable_setString(table, "foo", "Foo"));
	EXPECT_EQ(2U, dsTextSubstitutionTable_getRemainingStrings(table));
	EXPECT_TRUE(dsTextSubstitutionTable_setString(table, "bar", "Bar"));
	EXPECT_EQ(1U, dsTextSubstitutionTable_getRemainingStrings(table));
	EXPECT_TRUE(dsTextSubstitutionTable_setString(table, "baz", "Baz"));
	EXPECT_EQ(0U, dsTextSubstitutionTable_getRemainingStrings(table));
	EXPECT_FALSE(dsTextSubstitutionTable_setString(table, "asdf", "bla"));

	EXPECT_STREQ("Foo", dsTextSubstitutionTable_getString(table, "foo"));
	EXPECT_STREQ("Bar", dsTextSubstitutionTable_getString(table, "bar"));
	EXPECT_STREQ("Baz", dsTextSubstitutionTable_getString(table, "baz"));
	EXPECT_FALSE(dsTextSubstitutionTable_getString(table, "asdf"));

	uint32_t allocCount = allocator.allocator.totalAllocations;
	EXPECT_TRUE(dsTextSubstitutionTable_setString(table, "foo", "FooBar"));
	EXPECT_STREQ("FooBar", dsTextSubstitutionTable_getString(table, "foo"));
	EXPECT_LT(allocCount, allocator.allocator.totalAllocations);

	allocCount = allocator.allocator.totalAllocations;
	EXPECT_TRUE(dsTextSubstitutionTable_setString(table, "foo", "Foo"));
	EXPECT_STREQ("Foo", dsTextSubstitutionTable_getString(table, "foo"));
	EXPECT_EQ(allocCount, allocator.allocator.totalAllocations);

	EXPECT_FALSE(dsTextSubstitutionTable_removeString(table, "asdf"));
	EXPECT_TRUE(dsTextSubstitutionTable_removeString(table, "foo"));
	EXPECT_EQ(1U, dsTextSubstitutionTable_getRemainingStrings(table));
	EXPECT_TRUE(dsTextSubstitutionTable_setString(table, "asdf", "bla"));
	EXPECT_EQ(0U, dsTextSubstitutionTable_getRemainingStrings(table));

	EXPECT_FALSE(dsTextSubstitutionTable_getString(table, "foo"));
	EXPECT_STREQ("bla", dsTextSubstitutionTable_getString(table, "asdf"));

	dsTextSubstitutionTable_destroy(table);
}

TEST_F(TextSubstitutionTableTest, Substitute)
{
	const dsColor white = {{255, 255, 255, 255}};
	const char* testString = "Replacing ${foo}, ${bar}, and ${baz}.";
	const dsTextStyle testRanges[] =
	{
		{0, 10, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, white, white, 0.0f},
		{10, 6, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, white, white, 0.0f},
		{16, 8, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, white, white, 0.0f},
		{24, 6, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, white, white, 0.0f},
		{30, 7, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, white, white, 0.0f}
	};

	const char* expectedString = "Replacing Foo, FooBar, and x.";
	const dsTextStyle expectedRanges[] =
	{
		{0, 10, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, white, white, 0.0f},
		{10, 3, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, white, white, 0.0f},
		{13, 8, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, white, white, 0.0f},
		{21, 6, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, white, white, 0.0f},
		{27, 2, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, white, white, 0.0f}
	};

	static_assert(DS_ARRAY_SIZE(testRanges) == DS_ARRAY_SIZE(expectedRanges),
		"Mismatched style ranges.");
	constexpr uint32_t rangeCount = DS_ARRAY_SIZE(testRanges);

	dsTextSubstitutionTable* table = dsTextSubstitutionTable_create((dsAllocator*)&allocator, 3);
	ASSERT_TRUE(table);
	EXPECT_TRUE(dsTextSubstitutionTable_setString(table, "foo", "Foo"));
	EXPECT_TRUE(dsTextSubstitutionTable_setString(table, "bar", "FooBar"));

	dsTextSubstitutionData* data = dsTextSubstitutionData_create((dsAllocator*)&allocator);

	EXPECT_STREQ("FooBar", dsTextSubstitutionTable_substitute(table, data, "${bar}", nullptr, 0));
	EXPECT_FALSE(dsTextSubstitutionTable_substitute(table, data, "${bar", nullptr, 0));

	dsTextStyle adjustedRanges[rangeCount];
	memcpy(adjustedRanges, testRanges, sizeof(testRanges));
	EXPECT_FALSE(dsTextSubstitutionTable_substitute(table, data, testString, adjustedRanges,
		rangeCount));

	EXPECT_TRUE(dsTextSubstitutionTable_setString(table, "baz", "x"));

	// Previous ranges were adjusted, so re-copy original ranges.
	memcpy(adjustedRanges, testRanges, sizeof(testRanges));
	EXPECT_STREQ(expectedString, dsTextSubstitutionTable_substitute(table, data, testString,
		adjustedRanges, rangeCount));

	for (uint32_t i = 0; i < rangeCount; ++i)
	{
		EXPECT_EQ(expectedRanges[i].start, adjustedRanges[i].start) << " for range " << i;
		EXPECT_EQ(expectedRanges[i].count, adjustedRanges[i].count) << " for range " << i;
	}

	dsTextSubstitutionTable_destroy(table);
	dsTextSubstitutionData_destroy(data);
}

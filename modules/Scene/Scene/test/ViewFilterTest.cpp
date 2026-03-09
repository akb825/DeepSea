/*
 * Copyright 2026 Aaron Barany
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

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/UniqueNameID.h>
#include <DeepSea/Scene/ViewFilter.h>
#include <gtest/gtest.h>

class ViewFilterTest : public testing::Test
{
public:
	void SetUp() override
	{
		dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT);
		ASSERT_TRUE(dsUniqueNameID_initialize(
			&allocator.allocator, DS_DEFAULT_INITIAL_UNIQUE_NAME_ID_LIMIT));
	}

	void TearDown() override
	{
		EXPECT_TRUE(dsUniqueNameID_shutdown());
		EXPECT_EQ(0U, allocator.allocator.size);
	}

	dsSystemAllocator allocator;
};

TEST_F(ViewFilterTest, Create)
{
	const char* testName = "name";
	const char* nullName = nullptr;
	EXPECT_FALSE(dsViewFilter_create(nullptr, &testName, 1, false));
	EXPECT_FALSE(dsViewFilter_create(
		reinterpret_cast<dsAllocator*>(&allocator), nullptr, 1, false));
	EXPECT_FALSE(dsViewFilter_create(
		reinterpret_cast<dsAllocator*>(&allocator), &nullName, 1, false));
	EXPECT_FALSE(dsViewFilter_create(
		reinterpret_cast<dsAllocator*>(&allocator), &testName, 0, false));

	dsViewFilter* filter = dsViewFilter_create(
		reinterpret_cast<dsAllocator*>(&allocator), &testName, 1, false);
	EXPECT_TRUE(filter);
	dsViewFilter_destroy(filter);
}

TEST_F(ViewFilterTest, Filter)
{
	const char* names[] = {"foo", "bar", "baz"};
	dsViewFilter* filter = dsViewFilter_create(
		reinterpret_cast<dsAllocator*>(&allocator), names, DS_ARRAY_SIZE(names), false);
	ASSERT_TRUE(filter);

	EXPECT_TRUE(dsViewFilter_containsName(nullptr, "foo"));
	EXPECT_FALSE(dsViewFilter_containsName(nullptr, nullptr));

	EXPECT_TRUE(dsViewFilter_containsName(filter, "foo"));
	EXPECT_TRUE(dsViewFilter_containsName(filter, "bar"));
	EXPECT_TRUE(dsViewFilter_containsName(filter, "baz"));
	EXPECT_FALSE(dsViewFilter_containsName(filter, "foobar"));
	EXPECT_FALSE(dsViewFilter_containsName(filter, nullptr));

	EXPECT_TRUE(dsViewFilter_containsID(filter, dsUniqueNameID_get("foo")));
	EXPECT_TRUE(dsViewFilter_containsID(filter, dsUniqueNameID_get("bar")));
	EXPECT_TRUE(dsViewFilter_containsID(filter, dsUniqueNameID_get("baz")));
	EXPECT_FALSE(dsViewFilter_containsID(filter, dsUniqueNameID_create("foobar")));
	EXPECT_FALSE(dsViewFilter_containsID(filter, 0));

	dsViewFilter_destroy(filter);
}

TEST_F(ViewFilterTest, FilterInvert)
{
	const char* names[] = {"foo", "bar", "baz"};
	dsViewFilter* filter = dsViewFilter_create(
		reinterpret_cast<dsAllocator*>(&allocator), names, DS_ARRAY_SIZE(names), true);
	ASSERT_TRUE(filter);

	EXPECT_TRUE(dsViewFilter_containsID(nullptr, dsUniqueNameID_get("foo")));
	EXPECT_FALSE(dsViewFilter_containsID(nullptr, 0));

	EXPECT_FALSE(dsViewFilter_containsName(filter, "foo"));
	EXPECT_FALSE(dsViewFilter_containsName(filter, "bar"));
	EXPECT_FALSE(dsViewFilter_containsName(filter, "baz"));
	EXPECT_TRUE(dsViewFilter_containsName(filter, "foobar"));
	EXPECT_FALSE(dsViewFilter_containsName(filter, nullptr));

	EXPECT_FALSE(dsViewFilter_containsID(filter, dsUniqueNameID_get("foo")));
	EXPECT_FALSE(dsViewFilter_containsID(filter, dsUniqueNameID_get("bar")));
	EXPECT_FALSE(dsViewFilter_containsID(filter, dsUniqueNameID_get("baz")));
	EXPECT_TRUE(dsViewFilter_containsID(filter, dsUniqueNameID_create("foobar")));
	EXPECT_FALSE(dsViewFilter_containsID(filter, 0));

	dsViewFilter_destroy(filter);
}

TEST_F(ViewFilterTest, Hash)
{
	const char* names[] = {"foo", "bar", "baz"};
	dsViewFilter* filter = dsViewFilter_create(
		reinterpret_cast<dsAllocator*>(&allocator), names, DS_ARRAY_SIZE(names), false);
	ASSERT_TRUE(filter);

	uint32_t hash = dsViewFilter_hash(filter, DS_DEFAULT_HASH_SEED);

	dsViewFilter* otherFilter = dsViewFilter_create(
		reinterpret_cast<dsAllocator*>(&allocator), names, DS_ARRAY_SIZE(names), false);
	ASSERT_TRUE(otherFilter);
	EXPECT_EQ(hash, dsViewFilter_hash(otherFilter, DS_DEFAULT_HASH_SEED));
	dsViewFilter_destroy(otherFilter);

	{
		const char* otherNames[] = {"foo", "baz", "bar"};
		otherFilter = dsViewFilter_create(reinterpret_cast<dsAllocator*>(&allocator), otherNames,
			DS_ARRAY_SIZE(otherNames), false);
		ASSERT_TRUE(otherFilter);
		EXPECT_EQ(hash, dsViewFilter_hash(otherFilter, DS_DEFAULT_HASH_SEED));
		dsViewFilter_destroy(otherFilter);
	}

	otherFilter = dsViewFilter_create(
		reinterpret_cast<dsAllocator*>(&allocator), names, DS_ARRAY_SIZE(names), true);
	ASSERT_TRUE(otherFilter);
	EXPECT_NE(hash, dsViewFilter_hash(otherFilter, DS_DEFAULT_HASH_SEED));
	dsViewFilter_destroy(otherFilter);

	otherFilter = dsViewFilter_create(
		reinterpret_cast<dsAllocator*>(&allocator), names, DS_ARRAY_SIZE(names) - 1, false);
	ASSERT_TRUE(otherFilter);
	EXPECT_NE(hash, dsViewFilter_hash(otherFilter, DS_DEFAULT_HASH_SEED));
	dsViewFilter_destroy(otherFilter);

	dsViewFilter_destroy(filter);
}

TEST_F(ViewFilterTest, Equal)
{
	const char* names[] = {"foo", "bar", "baz"};
	dsViewFilter* filter = dsViewFilter_create(
		reinterpret_cast<dsAllocator*>(&allocator), names, DS_ARRAY_SIZE(names), false);
	ASSERT_TRUE(filter);

	EXPECT_TRUE(dsViewFilter_equal(filter, filter));
	EXPECT_FALSE(dsViewFilter_equal(filter, nullptr));
	EXPECT_FALSE(dsViewFilter_equal(nullptr, filter));
	EXPECT_TRUE(dsViewFilter_equal(nullptr, nullptr));

	dsViewFilter* otherFilter = dsViewFilter_create(
		reinterpret_cast<dsAllocator*>(&allocator), names, DS_ARRAY_SIZE(names), false);
	ASSERT_TRUE(otherFilter);
	EXPECT_TRUE(dsViewFilter_equal(filter, otherFilter));
	dsViewFilter_destroy(otherFilter);

	{
		const char* otherNames[] = {"foo", "baz", "bar"};
		otherFilter = dsViewFilter_create(reinterpret_cast<dsAllocator*>(&allocator), otherNames,
			DS_ARRAY_SIZE(otherNames), false);
		ASSERT_TRUE(otherFilter);
		EXPECT_TRUE(dsViewFilter_equal(filter, otherFilter));
		dsViewFilter_destroy(otherFilter);
	}

	otherFilter = dsViewFilter_create(
		reinterpret_cast<dsAllocator*>(&allocator), names, DS_ARRAY_SIZE(names), true);
	ASSERT_TRUE(otherFilter);
	EXPECT_FALSE(dsViewFilter_equal(filter, otherFilter));
	dsViewFilter_destroy(otherFilter);

	otherFilter = dsViewFilter_create(
		reinterpret_cast<dsAllocator*>(&allocator), names, DS_ARRAY_SIZE(names) - 1, false);
	ASSERT_TRUE(otherFilter);
	EXPECT_FALSE(dsViewFilter_equal(filter, otherFilter));
	dsViewFilter_destroy(otherFilter);

	{
		const char* otherNames[] = {"foo", "bar", "foobar"};
		otherFilter = dsViewFilter_create(reinterpret_cast<dsAllocator*>(&allocator), otherNames,
			DS_ARRAY_SIZE(otherNames), false);
		ASSERT_TRUE(otherFilter);
		EXPECT_FALSE(dsViewFilter_equal(filter, otherFilter));
		dsViewFilter_destroy(otherFilter);
	}

	dsViewFilter_destroy(filter);
}

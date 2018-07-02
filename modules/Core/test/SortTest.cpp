/*
 * Copyright 2018 Aaron Barany
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

#include <DeepSea/Core/Sort.h>
#include <gtest/gtest.h>
#include <vector>

namespace
{

struct TestData
{
	std::vector<int> values;
	std::vector<int> order;
};

static int compareIndirect(const void* left, const void* right, void* context)
{
	TestData* data = reinterpret_cast<TestData*>(context);
	return data->values[*(const int*)left] - data->values[*(const int*)right];
}

static int compareInt(const void* left, const void* right, void*)
{
	return *(const int*)left - *(const int*)right;
}

} // namespace

TEST(SortTest, IndirectSort)
{
	TestData data;
	data.values = {5, 4, 3, 2, 1};
	data.order = {0, 1, 2, 3, 4};
	ASSERT_EQ(data.values.size(), data.order.size());
	dsSort(data.order.data(), data.order.size(), sizeof(int), &compareIndirect, &data);
	EXPECT_EQ(std::vector<int>({4, 3, 2, 1, 0}), data.order);
}

TEST(SortTest, BinarySearch)
{
	std::vector<int> values = {1, 2, 3, 4, 5};
	for (std::size_t i = 0; i < values.size(); ++i)
	{
		int key = (int)i + 1;
		EXPECT_EQ(values.data() + i, dsBinarySearch(&key, values.data(), values.size(), sizeof(int),
			&compareInt, NULL));
	}

	values.push_back(6);
	for (std::size_t i = 0; i < values.size(); ++i)
	{
		int key = (int)i + 1;
		EXPECT_EQ(values.data() + i, dsBinarySearch(&key, values.data(), values.size(), sizeof(int),
			&compareInt, NULL));
	}

	int key = 0;
	EXPECT_EQ(NULL, dsBinarySearch(&key, values.data(), values.size(), sizeof(int), &compareInt,
		NULL));
	key = 7;
	EXPECT_EQ(NULL, dsBinarySearch(&key, values.data(), values.size(), sizeof(int), &compareInt,
		NULL));
}

TEST(SortTest, BinarySearchLowerBound)
{
	std::vector<int> values = {1, 2, 3, 3, 3, 5, 6};

	int key = 3;
	EXPECT_EQ(values.data() + 2, dsBinarySearchLowerBound(&key, values.data(), values.size(),
		sizeof(int), &compareInt, NULL));
	key = 4;
	EXPECT_EQ(values.data() + 5, dsBinarySearchLowerBound(&key, values.data(), values.size(),
		sizeof(int), &compareInt, NULL));

	key = 0;
	EXPECT_EQ(&values.front(), dsBinarySearchLowerBound(&key, values.data(), values.size(),
		sizeof(int), &compareInt, NULL));
	key = 6;
	EXPECT_EQ(&values.back(), dsBinarySearchLowerBound(&key, values.data(), values.size(),
		sizeof(int), &compareInt, NULL));
	key = 7;
	EXPECT_EQ(NULL, dsBinarySearchLowerBound(&key, values.data(), values.size(), sizeof(int),
		&compareInt, NULL));

	values = {1, 2, 3, 3, 3, 5, 6, 7, 8, 9, 10, 11, 12};
	key = 3;
	EXPECT_EQ(values.data() + 2, dsBinarySearchLowerBound(&key, values.data(), values.size(),
		sizeof(int), &compareInt, NULL));

	values = {-4, -3, -2, -1, 0, 1, 2, 3, 3, 3, 5, 6};\
	EXPECT_EQ(values.data() + 7, dsBinarySearchLowerBound(&key, values.data(), values.size(),
		sizeof(int), &compareInt, NULL));
}

TEST(SortTest, BinarySearchUpperBound)
{
	std::vector<int> values = {1, 2, 3, 3, 3, 5, 6};

	int key = 3;
	EXPECT_EQ(values.data() + 4, dsBinarySearchUpperBound(&key, values.data(), values.size(),
		sizeof(int), &compareInt, NULL));
	key = 4;
	EXPECT_EQ(values.data() + 4, dsBinarySearchUpperBound(&key, values.data(), values.size(),
		sizeof(int), &compareInt, NULL));

	key = 0;
	EXPECT_EQ(NULL, dsBinarySearchUpperBound(&key, values.data(), values.size(),
		sizeof(int), &compareInt, NULL));
	key = 6;
	EXPECT_EQ(&values.back(), dsBinarySearchUpperBound(&key, values.data(), values.size(),
		sizeof(int), &compareInt, NULL));
	key = 7;
	EXPECT_EQ(&values.back(), dsBinarySearchUpperBound(&key, values.data(), values.size(),
		sizeof(int), &compareInt, NULL));

	values = {1, 2, 3, 3, 3, 5, 6, 7, 8, 9, 10, 11, 12};
	key = 3;
	EXPECT_EQ(values.data() + 4, dsBinarySearchUpperBound(&key, values.data(), values.size(),
		sizeof(int), &compareInt, NULL));

	values = {-4, -3, -2, -1, 0, 1, 2, 3, 3, 3, 5, 6};\
	EXPECT_EQ(values.data() + 9, dsBinarySearchUpperBound(&key, values.data(), values.size(),
		sizeof(int), &compareInt, NULL));
}

/*
 * Copyright 2022 Aaron Barany
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

#include <DeepSea/Core/DeviceRandom.h>
#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include <unordered_set>

TEST(DeviceRandomTest, RandomBytes)
{
	constexpr unsigned int valueCount = 1024;
	std::unordered_set<std::uint64_t> uniqueValues;
	std::uint64_t values[valueCount] = {};
	// Will require multiple internal calls on some systems.
	ASSERT_TRUE(dsDeviceRandomBytes(values, sizeof(values)));
	uniqueValues.insert(values, values + valueCount);

	// Ensure it's re-entrent.
	ASSERT_TRUE(dsDeviceRandomBytes(values, sizeof(values)));
	uniqueValues.insert(values, values + valueCount);

	// This could theoretically fail, but is so improbably that it may as well be impossible.
	EXPECT_EQ(valueCount*2, uniqueValues.size());
}

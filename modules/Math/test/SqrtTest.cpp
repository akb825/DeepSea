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

#include <DeepSea/Math/Sqrt.h>
#include <cmath>
#include <gtest/gtest.h>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

template <typename T>
class SqrtTest : public testing::Test
{
};

using SqrtTypes = testing::Types<float, double>;
TYPED_TEST_SUITE(SqrtTest, SqrtTypes);

inline float dsSqrt(float x)
{
	return dsSqrtf(x);
}

inline double dsSqrt(double x)
{
	return dsSqrtd(x);
}

TYPED_TEST(SqrtTest, Sqrt)
{
	// Should be guaranteed to give identical results to the default sqrt function.
	EXPECT_EQ(std::sqrt(TypeParam(0.01)), dsSqrt(TypeParam(0.01)));
	EXPECT_EQ(std::sqrt(TypeParam(0.1)), dsSqrt(TypeParam(0.1)));
	EXPECT_EQ(std::sqrt(TypeParam(1)), dsSqrt(TypeParam(1)));
	EXPECT_EQ(std::sqrt(TypeParam(10)), dsSqrt(TypeParam(10)));
	EXPECT_EQ(std::sqrt(TypeParam(100)), dsSqrt(TypeParam(100)));
	EXPECT_EQ(std::sqrt(TypeParam(1000)), dsSqrt(TypeParam(1000)));
}

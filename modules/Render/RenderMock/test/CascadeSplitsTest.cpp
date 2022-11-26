/*
 * Copyright 2021 Aaron Barany
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

#include <DeepSea/Render/Shadows/CascadeSplits.h>
#include <gtest/gtest.h>
#include <float.h>
#include <math.h>

TEST(CascadeSplitsTest, ComputeCascadeCount)
{
	EXPECT_EQ(0U, dsComputeCascadeCount(1, 0, 10, 0, 4));
	EXPECT_EQ(0U, dsComputeCascadeCount(0, 1, 10, 0, 4));
	EXPECT_EQ(0U, dsComputeCascadeCount(1, 2, 0, 0, 4));
	EXPECT_EQ(0U, dsComputeCascadeCount(1, 2, 10, -1, 4));
	EXPECT_EQ(0U, dsComputeCascadeCount(1, 2, 10, 2, 4));
	EXPECT_EQ(0U, dsComputeCascadeCount(1, 2, 10, 0, 0));

	EXPECT_EQ(1U, dsComputeCascadeCount(0.1f, 9.0f, 10.0f, 0.0f, 4));
	EXPECT_EQ(2U, dsComputeCascadeCount(0.1f, 19.0f, 10.0f, 0.0f, 4));
	EXPECT_EQ(3U, dsComputeCascadeCount(0.1f, 29.0f, 10.0f, 0.0f, 4));
	EXPECT_EQ(4U, dsComputeCascadeCount(0.1f, 39.0f, 10.0f, 0.0f, 4));
	EXPECT_EQ(4U, dsComputeCascadeCount(0.1f, 1000.0f, 10.0f, 0.0f, 4));
}

TEST(CascadeSplitsTest, ComputeCascadeDistance)
{
	EXPECT_EQ(0.0f, dsComputeCascadeDistance(0, 2, FLT_MAX, 0, 0, 1));
	EXPECT_EQ(0.0f, dsComputeCascadeDistance(2, 1, FLT_MAX, 0, 0, 1));
	EXPECT_EQ(0.0f, dsComputeCascadeDistance(1, 2, FLT_MAX, -1, 0, 1));
	EXPECT_EQ(0.0f, dsComputeCascadeDistance(1, 2, FLT_MAX, 2, 0, 1));
	EXPECT_EQ(0.0f, dsComputeCascadeDistance(1, 2, FLT_MAX, 0, 1, 1));
	EXPECT_EQ(0.0f, dsComputeCascadeDistance(1, 2, FLT_MAX, 0, 0, 0));

	EXPECT_FLOAT_EQ(1.0f + 0.25f*9.0f, dsComputeCascadeDistance(1, 10, FLT_MAX, 0, 0, 4));
	EXPECT_FLOAT_EQ(1.0f + 0.5f*9.0f, dsComputeCascadeDistance(1, 10, FLT_MAX, 0, 1, 4));
	EXPECT_FLOAT_EQ(1.0f + 0.75f*9.0f, dsComputeCascadeDistance(1, 10, FLT_MAX, 0, 2, 4));
	EXPECT_FLOAT_EQ(10.0f, dsComputeCascadeDistance(1, 10, FLT_MAX, 0, 3, 4));

	EXPECT_FLOAT_EQ(2, dsComputeCascadeDistance(1, 10, 2, 0, 0, 4));

	EXPECT_FLOAT_EQ(powf(10.0f, 0.25f), dsComputeCascadeDistance(1, 10, FLT_MAX, 1, 0, 4));
	EXPECT_FLOAT_EQ(sqrtf(10.0f), dsComputeCascadeDistance(1, 10, FLT_MAX, 1, 1, 4));
	EXPECT_FLOAT_EQ(powf(10.0f, 0.75f), dsComputeCascadeDistance(1, 10, FLT_MAX, 1, 2, 4));
	EXPECT_FLOAT_EQ(10.0f, dsComputeCascadeDistance(1, 10, FLT_MAX, 1, 3, 4));
}

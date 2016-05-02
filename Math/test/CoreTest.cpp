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

#include <DeepSea/Math/Core.h>
#include <gtest/gtest.h>

TEST(Core, MinInt)
{
	EXPECT_EQ(0, dsMin(0, 1));
	EXPECT_EQ(0, dsMin(1, 0));
}

TEST(Core, MinFloat)
{
	EXPECT_EQ(-0.5f, dsMin(-0.5f, 3.2f));
	EXPECT_EQ(-0.5f, dsMin(3.2f, -0.5f));
}

TEST(Core, MaxInt)
{
	EXPECT_EQ(1, dsMax(0, 1));
	EXPECT_EQ(1, dsMax(1, 0));
}

TEST(Core, MaxFloat)
{
	EXPECT_EQ(3.2f, dsMax(-0.5f, 3.2f));
	EXPECT_EQ(3.2f, dsMax(3.2f, -0.5f));
}

TEST(Core, Pow2)
{
	EXPECT_EQ(3.2f*3.2f, dsPow2(3.2f));
}

TEST(Core, Pow3)
{
	EXPECT_EQ(3.2f*3.2f*3.2f, dsPow3(3.2f));
}

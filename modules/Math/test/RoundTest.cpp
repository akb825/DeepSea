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

#include <DeepSea/Math/Round.h>
#include <gtest/gtest.h>

TEST(RoundTest, Float)
{
	EXPECT_EQ(1.0f, dsRoundf(0.75f));
	EXPECT_EQ(1.0f, dsRoundf(1.25f));
	EXPECT_EQ(-1.0f, dsRoundf(-0.75f));
	EXPECT_EQ(-1.0f, dsRoundf(-1.25f));

	// Should round toward even.
	EXPECT_EQ(2.0f, dsRoundf(1.5f));
	EXPECT_EQ(2.0f, dsRoundf(2.5f));
	EXPECT_EQ(-2.0f, dsRoundf(-1.5f));
	EXPECT_EQ(-2.0f, dsRoundf(-2.5f));

	EXPECT_EQ(0.0f, dsTruncf(0.75f));
	EXPECT_EQ(1.0f, dsTruncf(1.25f));
	EXPECT_EQ(-0.0f, dsTruncf(-0.75f));
	EXPECT_EQ(-1.0f, dsTruncf(-1.25f));

	EXPECT_EQ(0.0f, dsFloorf(0.75f));
	EXPECT_EQ(1.0f, dsFloorf(1.25f));
	EXPECT_EQ(-1.0f, dsFloorf(-0.75f));
	EXPECT_EQ(-2.0f, dsFloorf(-1.25f));

	EXPECT_EQ(1.0f, dsCeilf(0.75f));
	EXPECT_EQ(2.0f, dsCeilf(1.25f));
	EXPECT_EQ(-0.0f, dsCeilf(-0.75f));
	EXPECT_EQ(-1.0f, dsCeilf(-1.25f));
}

TEST(RoundTest, Double)
{
	EXPECT_EQ(1.0, dsRoundd(0.75));
	EXPECT_EQ(1.0, dsRoundd(1.25));
	EXPECT_EQ(-1.0, dsRoundd(-0.75));
	EXPECT_EQ(-1.0, dsRoundd(-1.25));

	// Should round toward even.
	EXPECT_EQ(2.0, dsRoundd(1.5));
	EXPECT_EQ(2.0, dsRoundd(2.5));
	EXPECT_EQ(-2.0, dsRoundd(-1.5));
	EXPECT_EQ(-2.0, dsRoundd(-2.5));

	EXPECT_EQ(0.0, dsTruncd(0.75));
	EXPECT_EQ(1.0, dsTruncd(1.25));
	EXPECT_EQ(-0.0, dsTruncd(-0.75));
	EXPECT_EQ(-1.0, dsTruncd(-1.25));

	EXPECT_EQ(0.0, dsFloord(0.7));
	EXPECT_EQ(1.0, dsFloord(1.2));
	EXPECT_EQ(-1.0, dsFloorf(-0.75));
	EXPECT_EQ(-2.0, dsFloorf(-1.25));

	EXPECT_EQ(1.0, dsCeild(0.75));
	EXPECT_EQ(2.0, dsCeild(1.25));
	EXPECT_EQ(-0.0, dsCeilf(-0.75));
	EXPECT_EQ(-1.0, dsCeilf(-1.25));
}

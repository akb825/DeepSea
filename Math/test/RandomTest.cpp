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

#include <DeepSea/Math/Random.h>
#include <gtest/gtest.h>
#include <random>

TEST(RandomTest, Random)
{
	std::minstd_rand rand;
	uint32_t seed = 0;
	rand.seed(seed);

	EXPECT_EQ(rand(), dsRandom(&seed));
	EXPECT_EQ(rand(), dsRandom(&seed));
	EXPECT_EQ(rand(), dsRandom(&seed));
	EXPECT_EQ(rand(), dsRandom(&seed));
}

TEST(RandomTest, RandomDouble)
{
	uint32_t seed = 0;

	for (unsigned int i = 0; i < 1000; ++i)
	{
		double val = dsRandomDouble(&seed, -0.3, 7.9);
		EXPECT_LE(-0.3, val);
		EXPECT_GE(7.9, val);
	}
}

TEST(RandomTest, RandomInt)
{
	uint32_t seed = 0;

	for (unsigned int i = 0; i < 1000; ++i)
	{
		int val = dsRandomInt(&seed, -3, 9);
		EXPECT_LE(-3, val);
		EXPECT_GE(9, val);
	}
}

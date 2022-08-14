/*
 * Copyright 2016-2022 Aaron Barany
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
#include <unordered_set>

static constexpr unsigned int iterationCount = 10000;

TEST(RandomTest, CreateRandomSeed)
{
	constexpr unsigned int seedCount = 100;
	std::unordered_set<uint64_t> seeds;
	for (unsigned int i = 0; i < seedCount; ++i)
		seeds.insert(dsRandom_createSeed());
	EXPECT_EQ(seedCount, seeds.size());
}

TEST(RandomTest, KnownValues)
{
	dsRandom random;
	dsRandom_seed(&random, 0);
	EXPECT_EQ(0x99EC5F36CB75F2B4ULL, dsRandom_next(&random));
	EXPECT_EQ(0xBF6E1F784956452AULL, dsRandom_next(&random));
	EXPECT_EQ(0x1A5F849D4933E6E0ULL, dsRandom_next(&random));
	EXPECT_EQ(0x6AA594F1262D2D2CULL, dsRandom_next(&random));
	EXPECT_EQ(0xBBA5AD4A1F842E59ULL, dsRandom_next(&random));
}

TEST(RandomTest, NextDifferentSeeds)
{
	dsRandom random1, random2;
	dsRandom_seed(&random1, 0);
	dsRandom_seed(&random2, 1);
	for (unsigned int i = 0; i < iterationCount; ++i)
		EXPECT_NE(dsRandom_next(&random1), dsRandom_next(&random2));
}

TEST(RandomTest, Initialize)
{
	dsRandom random1, random2;
	dsRandom_initialize(&random1);
	dsRandom_initialize(&random2);
	for (unsigned int i = 0; i < iterationCount; ++i)
		EXPECT_NE(dsRandom_next(&random1), dsRandom_next(&random2));
}

TEST(RandomTest, NextBool)
{
	dsRandom random;
	dsRandom_seed(&random, 0);
	unsigned int counts[2] = {0, 0};
	for (unsigned int i = 0; i < iterationCount; ++i)
	{
		bool val = dsRandom_nextBool(&random);
		ASSERT_TRUE(val == 0 || val == 1);
		++counts[val];
	}

	EXPECT_NEAR(iterationCount/2, counts[0], iterationCount/100);
	EXPECT_NEAR(iterationCount/2, counts[1], iterationCount/100);
}

TEST(RandomTest, NextUInt32)
{
	dsRandom random;
	dsRandom_seed(&random, 0);
	constexpr unsigned int bucketCount = 8;
	unsigned int counts[bucketCount] = {};
	for (unsigned int i = 0; i < iterationCount; ++i)
	{
		uint32_t val = dsRandom_nextUInt32(&random, bucketCount - 1);
		ASSERT_GT(bucketCount, val);
		++counts[val];
	}

	for (unsigned int i = 0; i < bucketCount; ++i)
		EXPECT_NEAR(iterationCount/bucketCount, counts[i], iterationCount/100);

	EXPECT_EQ(0U, dsRandom_nextUInt32(&random, 0));
}

TEST(RandomTest, NextUInt32Range)
{
	dsRandom random;
	dsRandom_seed(&random, 0);
	constexpr unsigned int bucketCount = 8;
	unsigned int min = 5;
	unsigned int max = bucketCount + min - 1;
	unsigned int counts[bucketCount] = {};
	for (unsigned int i = 0; i < iterationCount; ++i)
	{
		uint32_t val = dsRandom_nextUInt32Range(&random, min, max);
		ASSERT_LE(min, val);
		ASSERT_GE(max, val);
		++counts[val - min];
	}

	for (unsigned int i = 0; i < bucketCount; ++i)
		EXPECT_NEAR(iterationCount/bucketCount, counts[i], iterationCount/100);

	EXPECT_EQ(min, dsRandom_nextUInt32Range(&random, min, min));
}

TEST(RandomTest, NextInt32Range)
{
	dsRandom random;
	dsRandom_seed(&random, 0);
	constexpr unsigned int bucketCount = 8;
	int min = -5;
	int max = bucketCount + min - 1;
	unsigned int counts[bucketCount] = {};
	for (unsigned int i = 0; i < iterationCount; ++i)
	{
		int32_t val = dsRandom_nextInt32Range(&random, min, max);
		ASSERT_LE(min, val);
		ASSERT_GE(max, val);
		++counts[val - min];
	}

	for (unsigned int i = 0; i < bucketCount; ++i)
		EXPECT_NEAR(iterationCount/bucketCount, counts[i], iterationCount/100);

	EXPECT_EQ(min, dsRandom_nextInt32Range(&random, min, min));
}

TEST(RandomTest, NextUInt64)
{
	dsRandom random;
	dsRandom_seed(&random, 0);
	constexpr unsigned int bucketCount = 8;
	unsigned int counts[bucketCount] = {};
	for (unsigned int i = 0; i < iterationCount; ++i)
	{
		uint64_t val = dsRandom_nextUInt64(&random, bucketCount - 1);
		ASSERT_GT(bucketCount, val);
		++counts[static_cast<size_t>(val)];
	}

	for (unsigned int i = 0; i < bucketCount; ++i)
		EXPECT_NEAR(iterationCount/bucketCount, counts[i], iterationCount/100);

	EXPECT_EQ(0U, dsRandom_nextUInt64(&random, 0));
}

TEST(RandomTest, NextUInt64Range)
{
	dsRandom random;
	dsRandom_seed(&random, 0);
	constexpr unsigned int bucketCount = 8;
	unsigned int min = 5;
	unsigned int max = bucketCount + min - 1;
	unsigned int counts[bucketCount] = {};
	for (unsigned int i = 0; i < iterationCount; ++i)
	{
		uint64_t val = dsRandom_nextUInt64Range(&random, min, max);
		ASSERT_LE(min, val);
		ASSERT_GE(max, val);
		++counts[static_cast<size_t>(val - min)];
	}

	for (unsigned int i = 0; i < bucketCount; ++i)
		EXPECT_NEAR(iterationCount/bucketCount, counts[i], iterationCount/100);

	EXPECT_EQ(min, dsRandom_nextUInt64Range(&random, min, min));
}

TEST(RandomTest, NextInt64Range)
{
	dsRandom random;
	dsRandom_seed(&random, 0);
	constexpr unsigned int bucketCount = 8;
	int min = -5;
	int max = bucketCount + min - 1;
	unsigned int counts[bucketCount] = {};
	for (unsigned int i = 0; i < iterationCount; ++i)
	{
		int64_t val = dsRandom_nextInt64Range(&random, min, max);
		ASSERT_LE(min, val);
		ASSERT_GE(max, val);
		++counts[static_cast<size_t>(val - min)];
	}

	for (unsigned int i = 0; i < bucketCount; ++i)
		EXPECT_NEAR(iterationCount/bucketCount, counts[i], iterationCount/100);

	EXPECT_EQ(min, dsRandom_nextInt64Range(&random, min, min));
}

TEST(RandomTest, NextFloat)
{
	dsRandom random;
	dsRandom_seed(&random, 0);
	float average = 0;
	float iterationScale = 1.0f/static_cast<float>(iterationCount);
	for (unsigned int i = 0; i < iterationCount; ++i)
	{
		float val = dsRandom_nextFloat(&random);
		EXPECT_LE(0.0f, val);
		EXPECT_GT(1.0f, val);
		average += val*iterationScale;
	}

	EXPECT_NEAR(0.5f, average, 1e-2f);
}

TEST(RandomTest, NextSignedFloat)
{
	dsRandom random;
	dsRandom_seed(&random, 0);
	float average = 0;
	float iterationScale = 1.0f/static_cast<float>(iterationCount);
	for (unsigned int i = 0; i < iterationCount; ++i)
	{
		float val = dsRandom_nextSignedFloat(&random);
		EXPECT_LT(-1.0f, val);
		EXPECT_GT(1.0f, val);
		average += val*iterationScale;
	}

	EXPECT_NEAR(0.0f, average, 1e-2f);
}

TEST(RandomTest, NextFloatRange)
{
	dsRandom random;
	dsRandom_seed(&random, 0);
	float min = -3.5f, max = 7.8945f;
	float average = 0;
	float iterationScale = 1.0f/static_cast<float>(iterationCount);
	for (unsigned int i = 0; i < iterationCount; ++i)
	{
		float val = dsRandom_nextFloatRange(&random, min, max);
		EXPECT_LE(min, val);
		EXPECT_GT(max, val);
		average += val*iterationScale;
	}

	EXPECT_NEAR((min + max)*0.5f, average, 1e-2f*(max - min));
	EXPECT_EQ(min, dsRandom_nextFloatRange(&random, min, min));
}

TEST(RandomTest, NextFloatCenteredRange)
{
	dsRandom random;
	dsRandom_seed(&random, 0);
	float center = -3.5f, range = 7.8945f;
	float average = 0;
	float iterationScale = 1.0f/static_cast<float>(iterationCount);
	for (unsigned int i = 0; i < iterationCount; ++i)
	{
		float val = dsRandom_nextFloatCenteredRange(&random, center, range);
		EXPECT_LT(center - range, val);
		EXPECT_GT(center + range, val);
		average += val*iterationScale;
	}

	EXPECT_NEAR(center, average, 1e-2f*range*2);
	EXPECT_EQ(center, dsRandom_nextFloatCenteredRange(&random, center, 0.0f));
}

TEST(RandomTest, NextDouble)
{
	dsRandom random;
	dsRandom_seed(&random, 0);
	double average = 0;
	double iterationScale = 1.0f/static_cast<double>(iterationCount);
	for (unsigned int i = 0; i < iterationCount; ++i)
	{
		double val = dsRandom_nextDouble(&random);
		EXPECT_LE(0.0, val);
		EXPECT_GT(1.0, val);
		average += val*iterationScale;
	}

	EXPECT_NEAR(0.5, average, 1e-2);
}

TEST(RandomTest, NextSignedDouble)
{
	dsRandom random;
	dsRandom_seed(&random, 0);
	double average = 0;
	double iterationScale = 1.0/static_cast<double>(iterationCount);
	for (unsigned int i = 0; i < iterationCount; ++i)
	{
		double val = dsRandom_nextSignedDouble(&random);
		EXPECT_LT(-1.0, val);
		EXPECT_GT(1.0, val);
		average += val*iterationScale;
	}

	EXPECT_NEAR(0.0, average, 1e-2);
}

TEST(RandomTest, NextDoubleRange)
{
	dsRandom random;
	dsRandom_seed(&random, 0);
	double min = -3.5, max = 7.8945;
	double average = 0;
	double iterationScale = 1.0f/static_cast<double>(iterationCount);
	for (unsigned int i = 0; i < iterationCount; ++i)
	{
		double val = dsRandom_nextDoubleRange(&random, min, max);
		EXPECT_LE(min, val);
		EXPECT_GT(max, val);
		average += val*iterationScale;
	}

	EXPECT_NEAR((min + max)*0.5, average, 1e-2*(max - min));
	EXPECT_EQ(min, dsRandom_nextDoubleRange(&random, min, min));
}

TEST(RandomTest, NextDoubleCenteredRange)
{
	dsRandom random;
	dsRandom_seed(&random, 0);
	double center = -3.5, range = 7.8945;
	double average = 0;
	double iterationScale = 1.0/static_cast<double>(iterationCount);
	for (unsigned int i = 0; i < iterationCount; ++i)
	{
		double val = dsRandom_nextDoubleCenteredRange(&random, center, range);
		EXPECT_LT(center - range, val);
		EXPECT_GT(center + range, val);
		average += val*iterationScale;
	}

	EXPECT_NEAR(center, average, 1e-2*range*2);
	EXPECT_EQ(center, dsRandom_nextDoubleCenteredRange(&random, center, 0.0));
}

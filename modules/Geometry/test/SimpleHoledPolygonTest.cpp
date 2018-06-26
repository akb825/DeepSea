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

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Geometry/SimpleHoledPolygon.h>
#include <DeepSea/Math/Core.h>
#include <gtest/gtest.h>

class SimpleHoledPolygonTest : public testing::Test
{
public:
	void SetUp() override
	{
		ASSERT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));
		polygon = dsSimpleHoledPolygon_create((dsAllocator*)&allocator, NULL);
		ASSERT_TRUE(polygon);
	}

	void TearDown() override
	{
		dsSimpleHoledPolygon_destroy(polygon);
		EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
	}

	dsSystemAllocator allocator;
	dsSimpleHoledPolygon* polygon;
};

TEST_F(SimpleHoledPolygonTest, Triangle)
{
	dsVector2d points[] =
	{
		{{0.0, 0.0}},
		{{1.0, 1.2}},
		{{2.0, 0.4}}
	};

	dsSimplePolygonLoop loop = {0, 3};

	uint32_t indexCount;
	const uint32_t* indices = dsSimpleHoledPolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), &loop, 1, NULL, dsTriangulateWinding_CCW);
	ASSERT_EQ(3U, indexCount);
	ASSERT_TRUE(indices);

	EXPECT_EQ(2U, indices[0]);
	EXPECT_EQ(1U, indices[1]);
	EXPECT_EQ(0U, indices[2]);
}

TEST_F(SimpleHoledPolygonTest, TriangleWithHoleCCW)
{
	dsVector2d points[] =
	{
		{{0.0, 0.0}},
		{{1.0, 0.0}},
		{{0.5, 1.0}},

		{{0.2, 0.2}},
		{{0.8, 0.2}},
		{{0.5, 0.8}}
	};

	dsSimplePolygonLoop loops[] = {{0, 3}, {3, 3}};

	uint32_t indexCount;
	const uint32_t* indices = dsSimpleHoledPolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), loops, DS_ARRAY_SIZE(loops), NULL, dsTriangulateWinding_CCW);
	ASSERT_EQ(18U, indexCount);
	ASSERT_TRUE(indices);

	EXPECT_EQ(4U, indices[0]);
	EXPECT_EQ(3U, indices[1]);
	EXPECT_EQ(0U, indices[2]);

	EXPECT_EQ(1U, indices[3]);
	EXPECT_EQ(4U, indices[4]);
	EXPECT_EQ(0U, indices[5]);

	EXPECT_EQ(5U, indices[6]);
	EXPECT_EQ(0U, indices[7]);
	EXPECT_EQ(3U, indices[8]);

	EXPECT_EQ(2U, indices[9]);
	EXPECT_EQ(0U, indices[10]);
	EXPECT_EQ(5U, indices[11]);

	EXPECT_EQ(4U, indices[12]);
	EXPECT_EQ(2U, indices[13]);
	EXPECT_EQ(5U, indices[14]);

	EXPECT_EQ(1U, indices[15]);
	EXPECT_EQ(2U, indices[16]);
	EXPECT_EQ(4U, indices[17]);
}

TEST_F(SimpleHoledPolygonTest, TriangleWithHoleCW)
{
	dsVector2d points[] =
	{
		{{0.0, 0.0}},
		{{0.5, 1.0}},
		{{1.0, 0.0}},

		{{0.2, 0.2}},
		{{0.5, 0.8}},
		{{0.8, 0.2}}
	};

	dsSimplePolygonLoop loops[] = {{0, 3}, {3, 3}};

	uint32_t indexCount;
	const uint32_t* indices = dsSimpleHoledPolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), loops, DS_ARRAY_SIZE(loops), NULL, dsTriangulateWinding_CCW);
	ASSERT_EQ(18U, indexCount);
	ASSERT_TRUE(indices);

	EXPECT_EQ(5U, indices[0]);
	EXPECT_EQ(3U, indices[1]);
	EXPECT_EQ(0U, indices[2]);

	EXPECT_EQ(2U, indices[3]);
	EXPECT_EQ(5U, indices[4]);
	EXPECT_EQ(0U, indices[5]);

	EXPECT_EQ(4U, indices[6]);
	EXPECT_EQ(0U, indices[7]);
	EXPECT_EQ(3U, indices[8]);

	EXPECT_EQ(1U, indices[9]);
	EXPECT_EQ(0U, indices[10]);
	EXPECT_EQ(4U, indices[11]);

	EXPECT_EQ(5U, indices[12]);
	EXPECT_EQ(1U, indices[13]);
	EXPECT_EQ(4U, indices[14]);

	EXPECT_EQ(2U, indices[15]);
	EXPECT_EQ(1U, indices[16]);
	EXPECT_EQ(5U, indices[17]);
}

TEST_F(SimpleHoledPolygonTest, TwoHoles)
{
	dsVector2d points[] =
	{
		{{0.0, 0.0}},
		{{1.0, 0.0}},
		{{1.0, 1.0}},
		{{0.0, 1.0}},

		{{0.2, 0.2}},
		{{0.4, 0.2}},
		{{0.3, 0.6}},

		{{0.6, 0.8}},
		{{0.8, 0.8}},
		{{0.7, 0.4}}
	};

	dsSimplePolygonLoop loops[] = {{0, 4}, {4, 3}, {7, 3}};

	uint32_t indexCount;
	const uint32_t* indices = dsSimpleHoledPolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), loops, DS_ARRAY_SIZE(loops), NULL, dsTriangulateWinding_CCW);
	ASSERT_EQ(36U, indexCount);
	ASSERT_TRUE(indices);

	EXPECT_EQ(5U, indices[0]);
	EXPECT_EQ(4U, indices[1]);
	EXPECT_EQ(0U, indices[2]);

	EXPECT_EQ(1U, indices[3]);
	EXPECT_EQ(8U, indices[4]);
	EXPECT_EQ(9U, indices[5]);

	EXPECT_EQ(1U, indices[6]);
	EXPECT_EQ(9U, indices[7]);
	EXPECT_EQ(5U, indices[8]);

	EXPECT_EQ(1U, indices[9]);
	EXPECT_EQ(5U, indices[10]);
	EXPECT_EQ(0U, indices[11]);

	EXPECT_EQ(8U, indices[12]);
	EXPECT_EQ(3U, indices[13]);
	EXPECT_EQ(7U, indices[14]);

	EXPECT_EQ(2U, indices[15]);
	EXPECT_EQ(8U, indices[16]);
	EXPECT_EQ(1U, indices[17]);

	EXPECT_EQ(2U, indices[18]);
	EXPECT_EQ(3U, indices[19]);
	EXPECT_EQ(8U, indices[20]);

	EXPECT_EQ(4U, indices[21]);
	EXPECT_EQ(3U, indices[22]);
	EXPECT_EQ(0U, indices[23]);

	EXPECT_EQ(6U, indices[24]);
	EXPECT_EQ(3U, indices[25]);
	EXPECT_EQ(4U, indices[26]);

	EXPECT_EQ(7U, indices[27]);
	EXPECT_EQ(3U, indices[28]);
	EXPECT_EQ(6U, indices[29]);

	EXPECT_EQ(7U, indices[30]);
	EXPECT_EQ(6U, indices[31]);
	EXPECT_EQ(5U, indices[32]);

	EXPECT_EQ(9U, indices[33]);
	EXPECT_EQ(7U, indices[34]);
	EXPECT_EQ(5U, indices[35]);
}

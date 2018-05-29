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

#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Geometry/SimplePolygon.h>
#include <gtest/gtest.h>

// NOTE: These test cases were made in image space, i.e. image space in upper-left. As a result,
// most cases also use CW winding order for the triangulation, which would be CCW when rendered to
// the screen.

class SimplePolygonTest : public testing::Test
{
public:
	void SetUp() override
	{
		ASSERT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));
		polygon = dsSimplePolygon_create((dsAllocator*)&allocator, NULL);
		ASSERT_TRUE(polygon);
	}

	void TearDown() override
	{
		dsSimplePolygon_destroy(polygon);
		EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
	}

	dsSystemAllocator allocator;
	dsSimplePolygon* polygon;
};

TEST_F(SimplePolygonTest, TriangleCW)
{
	dsVector2d points[] =
	{
		{{0.0, 0.0}},
		{{1.0, 1.2}},
		{{2.0, 0.4}}
	};

	uint32_t indexCount;
	const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), NULL, dsTriangulateWinding_CW);
	ASSERT_EQ(3U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(2U, indices[0]);
	EXPECT_EQ(0U, indices[1]);
	EXPECT_EQ(1U, indices[2]);

	indices = dsSimplePolygon_triangulate(&indexCount, polygon, points, DS_ARRAY_SIZE(points),
		NULL, dsTriangulateWinding_CCW);
	ASSERT_EQ(3U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(2U, indices[0]);
	EXPECT_EQ(1U, indices[1]);
	EXPECT_EQ(0U, indices[2]);
}

TEST_F(SimplePolygonTest, TriangleCCW)
{
	dsVector2d points[] =
	{
		{{0.0, 0.0}},
		{{2.0, 0.4}},
		{{1.0, 1.2}}
	};

	uint32_t indexCount;
	const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), NULL, dsTriangulateWinding_CW);
	ASSERT_EQ(3U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(1U, indices[0]);
	EXPECT_EQ(0U, indices[1]);
	EXPECT_EQ(2U, indices[2]);

	indices = dsSimplePolygon_triangulate(&indexCount, polygon, points, DS_ARRAY_SIZE(points),
		NULL, dsTriangulateWinding_CCW);
	ASSERT_EQ(3U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(1U, indices[0]);
	EXPECT_EQ(2U, indices[1]);
	EXPECT_EQ(0U, indices[2]);
}

TEST_F(SimplePolygonTest, ObliqueTriangleCW)
{
	dsVector2d points[] =
	{
		{{0.0, 0.0}},
		{{2.0, 1.2}},
		{{1.0, 0.4}}
	};

	uint32_t indexCount;
	const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), NULL, dsTriangulateWinding_CW);
	ASSERT_EQ(3U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(1U, indices[0]);
	EXPECT_EQ(2U, indices[1]);
	EXPECT_EQ(0U, indices[2]);

	indices = dsSimplePolygon_triangulate(&indexCount, polygon, points, DS_ARRAY_SIZE(points),
		NULL, dsTriangulateWinding_CCW);
	ASSERT_EQ(3U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(1U, indices[0]);
	EXPECT_EQ(0U, indices[1]);
	EXPECT_EQ(2U, indices[2]);
}

TEST_F(SimplePolygonTest, ObliqueTriangleCCW)
{
	dsVector2d points[] =
	{
		{{0.0, 0.0}},
		{{1.0, 0.4}},
		{{2.0, 1.2}}
	};

	uint32_t indexCount;
	const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), NULL, dsTriangulateWinding_CW);
	ASSERT_EQ(3U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(2U, indices[0]);
	EXPECT_EQ(1U, indices[1]);
	EXPECT_EQ(0U, indices[2]);

	indices = dsSimplePolygon_triangulate(&indexCount, polygon, points, DS_ARRAY_SIZE(points),
		NULL, dsTriangulateWinding_CCW);
	ASSERT_EQ(3U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(2U, indices[0]);
	EXPECT_EQ(0U, indices[1]);
	EXPECT_EQ(1U, indices[2]);
}

TEST_F(SimplePolygonTest, QuadCW)
{
	dsVector2d points[] =
	{
		{{2.0, 1.3}},
		{{1.2, 0.4}},
		{{0.0, 0.9}},
		{{0.8, 2.0}}
	};

	uint32_t indexCount;
	const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), NULL, dsTriangulateWinding_CW);
	ASSERT_EQ(6U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(1U, indices[0]);
	EXPECT_EQ(2U, indices[1]);
	EXPECT_EQ(3U, indices[2]);

	EXPECT_EQ(0U, indices[3]);
	EXPECT_EQ(1U, indices[4]);
	EXPECT_EQ(3U, indices[5]);

	indices = dsSimplePolygon_triangulate(&indexCount, polygon, points, DS_ARRAY_SIZE(points),
		NULL, dsTriangulateWinding_CCW);
	ASSERT_EQ(6U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(1U, indices[0]);
	EXPECT_EQ(3U, indices[1]);
	EXPECT_EQ(2U, indices[2]);

	EXPECT_EQ(0U, indices[3]);
	EXPECT_EQ(3U, indices[4]);
	EXPECT_EQ(1U, indices[5]);
}

TEST_F(SimplePolygonTest, QuadCCW)
{
	dsVector2d points[] =
	{
		{{2.0, 1.3}},
		{{0.8, 2.0}},
		{{0.0, 0.9}},
		{{1.2, 0.4}}
	};

	uint32_t indexCount;
	const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), NULL, dsTriangulateWinding_CW);
	ASSERT_EQ(6U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(3U, indices[0]);
	EXPECT_EQ(2U, indices[1]);
	EXPECT_EQ(1U, indices[2]);

	EXPECT_EQ(0U, indices[3]);
	EXPECT_EQ(3U, indices[4]);
	EXPECT_EQ(1U, indices[5]);

	indices = dsSimplePolygon_triangulate(&indexCount, polygon, points, DS_ARRAY_SIZE(points),
		NULL, dsTriangulateWinding_CCW);
	ASSERT_EQ(6U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(3U, indices[0]);
	EXPECT_EQ(1U, indices[1]);
	EXPECT_EQ(2U, indices[2]);

	EXPECT_EQ(0U, indices[3]);
	EXPECT_EQ(1U, indices[4]);
	EXPECT_EQ(3U, indices[5]);
}

TEST_F(SimplePolygonTest, MonotonicCW)
{
	dsVector2d points[] =
	{
		{{0.0, 11.4}},
		{{4.0, 6.5}},
		{{16.0, 1.7}},
		{{18.4, 14.8}},
		{{24.5, 13.2}},
		{{29.2, 9.0}},
		{{31.0, 0.0}},
		{{34.0, 0.0}},
		{{36.0, 16.0}},
		{{12.5, 16.0}},
		{{11.3, 11.2}},
		{{8.8, 8.9}},
		{{6.4, 8.9}}
	};

	uint32_t indexCount;
	const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), NULL, dsTriangulateWinding_CW);
	ASSERT_EQ(33U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(12U, indices[0]);
	EXPECT_EQ(1U, indices[1]);
	EXPECT_EQ(0U, indices[2]);

	EXPECT_EQ(11U, indices[3]);
	EXPECT_EQ(1U, indices[4]);
	EXPECT_EQ(12U, indices[5]);

	EXPECT_EQ(2U, indices[6]);
	EXPECT_EQ(1U, indices[7]);
	EXPECT_EQ(11U, indices[8]);

	EXPECT_EQ(2U, indices[9]);
	EXPECT_EQ(11U, indices[10]);
	EXPECT_EQ(10U, indices[11]);

	EXPECT_EQ(2U, indices[12]);
	EXPECT_EQ(10U, indices[13]);
	EXPECT_EQ(9U, indices[14]);

	EXPECT_EQ(3U, indices[15]);
	EXPECT_EQ(2U, indices[16]);
	EXPECT_EQ(9U, indices[17]);

	EXPECT_EQ(7U, indices[18]);
	EXPECT_EQ(6U, indices[19]);
	EXPECT_EQ(5U, indices[20]);

	EXPECT_EQ(8U, indices[21]);
	EXPECT_EQ(7U, indices[22]);
	EXPECT_EQ(5U, indices[23]);

	EXPECT_EQ(8U, indices[24]);
	EXPECT_EQ(5U, indices[25]);
	EXPECT_EQ(4U, indices[26]);

	EXPECT_EQ(8U, indices[27]);
	EXPECT_EQ(4U, indices[28]);
	EXPECT_EQ(3U, indices[29]);

	EXPECT_EQ(8U, indices[30]);
	EXPECT_EQ(3U, indices[31]);
	EXPECT_EQ(9U, indices[32]);
}

TEST_F(SimplePolygonTest, MonotonicCCW)
{
	dsVector2d points[] =
	{
		{{0.0, 11.4}},
		{{6.4, 8.9}},
		{{8.8, 8.9}},
		{{11.3, 11.2}},
		{{12.5, 16.0}},
		{{36.0, 16.0}},
		{{34.0, 0.0}},
		{{31.0, 0.0}},
		{{29.2, 9.0}},
		{{24.5, 13.2}},
		{{18.4, 14.8}},
		{{16.0, 1.7}},
		{{4.0, 6.5}}
	};

	uint32_t indexCount;
	const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), NULL, dsTriangulateWinding_CW);
	ASSERT_EQ(33U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(1U, indices[0]);
	EXPECT_EQ(12U, indices[1]);
	EXPECT_EQ(0U, indices[2]);

	EXPECT_EQ(2U, indices[3]);
	EXPECT_EQ(12U, indices[4]);
	EXPECT_EQ(1U, indices[5]);

	EXPECT_EQ(11U, indices[6]);
	EXPECT_EQ(12U, indices[7]);
	EXPECT_EQ(2U, indices[8]);

	EXPECT_EQ(11U, indices[9]);
	EXPECT_EQ(2U, indices[10]);
	EXPECT_EQ(3U, indices[11]);

	EXPECT_EQ(11U, indices[12]);
	EXPECT_EQ(3U, indices[13]);
	EXPECT_EQ(4U, indices[14]);

	EXPECT_EQ(10U, indices[15]);
	EXPECT_EQ(11U, indices[16]);
	EXPECT_EQ(4U, indices[17]);

	EXPECT_EQ(6U, indices[18]);
	EXPECT_EQ(7U, indices[19]);
	EXPECT_EQ(8U, indices[20]);

	EXPECT_EQ(5U, indices[21]);
	EXPECT_EQ(6U, indices[22]);
	EXPECT_EQ(8U, indices[23]);

	EXPECT_EQ(5U, indices[24]);
	EXPECT_EQ(8U, indices[25]);
	EXPECT_EQ(9U, indices[26]);

	EXPECT_EQ(5U, indices[27]);
	EXPECT_EQ(9U, indices[28]);
	EXPECT_EQ(10U, indices[29]);

	EXPECT_EQ(5U, indices[30]);
	EXPECT_EQ(10U, indices[31]);
	EXPECT_EQ(4U, indices[32]);
}

TEST_F(SimplePolygonTest, ComplexCW)
{
	dsVector2d points[] =
	{
		{{0.0, 26.0}},
		{{5.4, 7.6f}},
		{{16.0, 5.2}},
		{{14.5, 13.6}},
		{{10.1, 19.2}},
		{{17.0, 22.0}},
		{{21.0, 14.5}},
		{{18.4, 7.3}},
		{{33.1, 0.0}},
		{{38.0, 4.8}},
		{{33.1, 10.6}},
		{{26.8, 12.5}},
		{{37.4, 17.1}},
		{{29.0, 21.7}},
		{{37.6, 24.1}},
		{{43.9, 21.4}},
		{{42.1, 10.3}},
		{{51.7, 5.7}},
		{{63.4, 5.7}},
		{{60.2, 17.0}},
		{{54.1, 12.9}},
		{{47.1, 24.0}},
		{{69.5, 23.0}},
		{{62.4, 31.5}},
		{{64.6, 45.6}},
		{{60.5, 37.0}},
		{{54.4, 34.9}},
		{{58.1, 27.2}},
		{{40.7, 30.2}},
		{{52.5, 33.0}},
		{{45.3, 41.2}},
		{{36.5, 37.9}},
		{{33.1, 27.8}},
		{{23.9, 26.8}},
		{{14.5, 29.9}},
		{{26.8, 31.8}},
		{{25.7, 37.1}},
		{{18.9, 41.4}},
		{{8.4, 38.2}}
	};

	uint32_t indexCount;
	const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), NULL, dsTriangulateWinding_CW);
	ASSERT_EQ(111U, indexCount);
	ASSERT_TRUE(indices);
	// First loop
	EXPECT_EQ(38U, indices[0]);
	EXPECT_EQ(1U, indices[1]);
	EXPECT_EQ(0U, indices[2]);

	EXPECT_EQ(4U, indices[3]);
	EXPECT_EQ(1U, indices[4]);
	EXPECT_EQ(38U, indices[5]);

	EXPECT_EQ(3U, indices[6]);
	EXPECT_EQ(1U, indices[7]);
	EXPECT_EQ(4U, indices[8]);

	EXPECT_EQ(2U, indices[9]);
	EXPECT_EQ(1U, indices[10]);
	EXPECT_EQ(3U, indices[11]);

	// Second loop
	EXPECT_EQ(5U, indices[12]);
	EXPECT_EQ(4U, indices[13]);
	EXPECT_EQ(34U, indices[14]);

	EXPECT_EQ(6U, indices[15]);
	EXPECT_EQ(5U, indices[16]);
	EXPECT_EQ(34U, indices[17]);

	EXPECT_EQ(33U, indices[18]);
	EXPECT_EQ(6U, indices[19]);
	EXPECT_EQ(34U, indices[20]);

	// Third loop
	EXPECT_EQ(11U, indices[21]);
	EXPECT_EQ(6U, indices[22]);
	EXPECT_EQ(33U, indices[23]);

	EXPECT_EQ(11U, indices[24]);
	EXPECT_EQ(7U, indices[25]);
	EXPECT_EQ(6U, indices[26]);

	EXPECT_EQ(8U, indices[27]);
	EXPECT_EQ(7U, indices[28]);
	EXPECT_EQ(11U, indices[29]);

	EXPECT_EQ(10U, indices[30]);
	EXPECT_EQ(8U, indices[31]);
	EXPECT_EQ(11U, indices[32]);

	EXPECT_EQ(9U, indices[33]);
	EXPECT_EQ(8U, indices[34]);
	EXPECT_EQ(10U, indices[35]);

	// Fourth loop
	EXPECT_EQ(12U, indices[36]);
	EXPECT_EQ(11U, indices[37]);
	EXPECT_EQ(13U, indices[38]);

	// Fifth loop
	EXPECT_EQ(13U, indices[39]);
	EXPECT_EQ(11U, indices[40]);
	EXPECT_EQ(33U, indices[41]);

	EXPECT_EQ(32U, indices[42]);
	EXPECT_EQ(13U, indices[43]);
	EXPECT_EQ(33U, indices[44]);

	EXPECT_EQ(14U, indices[45]);
	EXPECT_EQ(13U, indices[46]);
	EXPECT_EQ(32U, indices[47]);

	EXPECT_EQ(14U, indices[48]);
	EXPECT_EQ(32U, indices[49]);
	EXPECT_EQ(31U, indices[50]);

	EXPECT_EQ(28U, indices[51]);
	EXPECT_EQ(14U, indices[52]);
	EXPECT_EQ(31U, indices[53]);

	EXPECT_EQ(30U, indices[54]);
	EXPECT_EQ(28U, indices[55]);
	EXPECT_EQ(31U, indices[56]);

	EXPECT_EQ(29U, indices[57]);
	EXPECT_EQ(28U, indices[58]);
	EXPECT_EQ(30U, indices[59]);

	// Sixth loop
	EXPECT_EQ(15U, indices[60]);
	EXPECT_EQ(14U, indices[61]);
	EXPECT_EQ(28U, indices[62]);

	EXPECT_EQ(21U, indices[63]);
	EXPECT_EQ(15U, indices[64]);
	EXPECT_EQ(28U, indices[65]);

	EXPECT_EQ(27U, indices[66]);
	EXPECT_EQ(21U, indices[67]);
	EXPECT_EQ(28U, indices[68]);

	EXPECT_EQ(23U, indices[69]);
	EXPECT_EQ(27U, indices[70]);
	EXPECT_EQ(25U, indices[71]);

	EXPECT_EQ(22U, indices[72]);
	EXPECT_EQ(27U, indices[73]);
	EXPECT_EQ(23U, indices[74]);

	EXPECT_EQ(22U, indices[75]);
	EXPECT_EQ(21U, indices[76]);
	EXPECT_EQ(27U, indices[77]);

	// Seventh loop
	EXPECT_EQ(21U, indices[78]);
	EXPECT_EQ(16U, indices[79]);
	EXPECT_EQ(15U, indices[80]);

	EXPECT_EQ(17U, indices[81]);
	EXPECT_EQ(16U, indices[82]);
	EXPECT_EQ(21U, indices[83]);

	EXPECT_EQ(20U, indices[84]);
	EXPECT_EQ(17U, indices[85]);
	EXPECT_EQ(21U, indices[86]);

	EXPECT_EQ(19U, indices[87]);
	EXPECT_EQ(17U, indices[88]);
	EXPECT_EQ(20U, indices[89]);

	EXPECT_EQ(18U, indices[90]);
	EXPECT_EQ(17U, indices[91]);
	EXPECT_EQ(19U, indices[92]);

	// Eigth loop
	EXPECT_EQ(24U, indices[93]);
	EXPECT_EQ(23U, indices[94]);
	EXPECT_EQ(25U, indices[95]);

	// Ninth loop
	EXPECT_EQ(25U, indices[96]);
	EXPECT_EQ(27U, indices[97]);
	EXPECT_EQ(26U, indices[98]);

	// Tenth loop
	EXPECT_EQ(34U, indices[99]);
	EXPECT_EQ(4U, indices[100]);
	EXPECT_EQ(38U, indices[101]);

	EXPECT_EQ(37U, indices[102]);
	EXPECT_EQ(34U, indices[103]);
	EXPECT_EQ(38U, indices[104]);

	EXPECT_EQ(36U, indices[105]);
	EXPECT_EQ(34U, indices[106]);
	EXPECT_EQ(37U, indices[107]);

	EXPECT_EQ(35U, indices[108]);
	EXPECT_EQ(34U, indices[109]);
	EXPECT_EQ(36U, indices[110]);
}

TEST_F(SimplePolygonTest, ComplexCCW)
{
	dsVector2d points[] =
	{
		{{0.0, 26.0}},
		{{8.4, 38.2}},
		{{18.9, 41.4}},
		{{25.7, 37.1}},
		{{26.8, 31.8}},
		{{14.5, 29.9}},
		{{23.9, 26.8}},
		{{33.1, 27.8}},
		{{36.5, 37.9}},
		{{45.3, 41.2}},
		{{52.5, 33.0}},
		{{40.7, 30.2}},
		{{58.1, 27.2}},
		{{54.4, 34.9}},
		{{60.5, 37.0}},
		{{64.6, 45.6}},
		{{62.4, 31.5}},
		{{69.5, 23.0}},
		{{47.1, 24.0}},
		{{54.1, 12.9}},
		{{60.2, 17.0}},
		{{63.4, 5.7}},
		{{51.7, 5.7}},
		{{42.1, 10.3}},
		{{43.9, 21.4}},
		{{37.6, 24.1}},
		{{29.0, 21.7}},
		{{37.4, 17.1}},
		{{26.8, 12.5}},
		{{33.1, 10.6}},
		{{38.0, 4.8}},
		{{33.1, 0.0}},
		{{18.4, 7.3}},
		{{21.0, 14.5}},
		{{17.0, 22.0}},
		{{10.1, 19.2}},
		{{14.5, 13.6}},
		{{16.0, 5.2}},
		{{5.4, 7.6f}}
	};

	uint32_t indexCount;
	const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), NULL, dsTriangulateWinding_CW);
	ASSERT_EQ(111U, indexCount);
	ASSERT_TRUE(indices);
	// First loop
	EXPECT_EQ(1U, indices[0]);
	EXPECT_EQ(38U, indices[1]);
	EXPECT_EQ(0U, indices[2]);

	EXPECT_EQ(35U, indices[3]);
	EXPECT_EQ(38U, indices[4]);
	EXPECT_EQ(1U, indices[5]);

	EXPECT_EQ(36U, indices[6]);
	EXPECT_EQ(38U, indices[7]);
	EXPECT_EQ(35U, indices[8]);

	EXPECT_EQ(37U, indices[9]);
	EXPECT_EQ(38U, indices[10]);
	EXPECT_EQ(36U, indices[11]);

	// Second loop
	EXPECT_EQ(5U, indices[12]);
	EXPECT_EQ(35U, indices[13]);
	EXPECT_EQ(1U, indices[14]);

	EXPECT_EQ(2U, indices[15]);
	EXPECT_EQ(5U, indices[16]);
	EXPECT_EQ(1U, indices[17]);

	EXPECT_EQ(3U, indices[18]);
	EXPECT_EQ(5U, indices[19]);
	EXPECT_EQ(2U, indices[20]);

	EXPECT_EQ(4U, indices[21]);
	EXPECT_EQ(5U, indices[22]);
	EXPECT_EQ(3U, indices[23]);

	// Third loop
	EXPECT_EQ(34U, indices[24]);
	EXPECT_EQ(35U, indices[25]);
	EXPECT_EQ(5U, indices[26]);

	EXPECT_EQ(33U, indices[27]);
	EXPECT_EQ(34U, indices[28]);
	EXPECT_EQ(5U, indices[29]);

	EXPECT_EQ(6U, indices[30]);
	EXPECT_EQ(33U, indices[31]);
	EXPECT_EQ(5U, indices[32]);

	// Fourth loop
	EXPECT_EQ(26U, indices[33]);
	EXPECT_EQ(28U, indices[34]);
	EXPECT_EQ(6U, indices[35]);

	EXPECT_EQ(7U, indices[36]);
	EXPECT_EQ(26U, indices[37]);
	EXPECT_EQ(6U, indices[38]);

	EXPECT_EQ(25U, indices[39]);
	EXPECT_EQ(26U, indices[40]);
	EXPECT_EQ(7U, indices[41]);

	EXPECT_EQ(25U, indices[42]);
	EXPECT_EQ(7U, indices[43]);
	EXPECT_EQ(8U, indices[44]);

	EXPECT_EQ(11U, indices[45]);
	EXPECT_EQ(25U, indices[46]);
	EXPECT_EQ(8U, indices[47]);

	EXPECT_EQ(9U, indices[48]);
	EXPECT_EQ(11U, indices[49]);
	EXPECT_EQ(8U, indices[50]);

	EXPECT_EQ(10U, indices[51]);
	EXPECT_EQ(11U, indices[52]);
	EXPECT_EQ(9U, indices[53]);

	// Fifth loop
	EXPECT_EQ(24U, indices[54]);
	EXPECT_EQ(25U, indices[55]);
	EXPECT_EQ(11U, indices[56]);

	EXPECT_EQ(18U, indices[57]);
	EXPECT_EQ(24U, indices[58]);
	EXPECT_EQ(11U, indices[59]);

	EXPECT_EQ(12U, indices[60]);
	EXPECT_EQ(18U, indices[61]);
	EXPECT_EQ(11U, indices[62]);

	EXPECT_EQ(16U, indices[63]);
	EXPECT_EQ(12U, indices[64]);
	EXPECT_EQ(14U, indices[65]);

	EXPECT_EQ(17U, indices[66]);
	EXPECT_EQ(12U, indices[67]);
	EXPECT_EQ(16U, indices[68]);

	EXPECT_EQ(17U, indices[69]);
	EXPECT_EQ(18U, indices[70]);
	EXPECT_EQ(12U, indices[71]);

	// Sixth loop
	EXPECT_EQ(14U, indices[72]);
	EXPECT_EQ(12U, indices[73]);
	EXPECT_EQ(13U, indices[74]);

	// Seventh loop
	EXPECT_EQ(15U, indices[75]);
	EXPECT_EQ(16U, indices[76]);
	EXPECT_EQ(14U, indices[77]);

	// Eighth loop
	EXPECT_EQ(18U, indices[78]);
	EXPECT_EQ(23U, indices[79]);
	EXPECT_EQ(24U, indices[80]);

	EXPECT_EQ(22U, indices[81]);
	EXPECT_EQ(23U, indices[82]);
	EXPECT_EQ(18U, indices[83]);

	EXPECT_EQ(19U, indices[84]);
	EXPECT_EQ(22U, indices[85]);
	EXPECT_EQ(18U, indices[86]);

	EXPECT_EQ(20U, indices[87]);
	EXPECT_EQ(22U, indices[88]);
	EXPECT_EQ(19U, indices[89]);

	EXPECT_EQ(21U, indices[90]);
	EXPECT_EQ(22U, indices[91]);
	EXPECT_EQ(20U, indices[92]);

	// Ninth loop
	EXPECT_EQ(27U, indices[93]);
	EXPECT_EQ(28U, indices[94]);
	EXPECT_EQ(26U, indices[95]);

	// Tenth loop
	EXPECT_EQ(28U, indices[96]);
	EXPECT_EQ(33U, indices[97]);
	EXPECT_EQ(6U, indices[98]);

	EXPECT_EQ(28U, indices[99]);
	EXPECT_EQ(32U, indices[100]);
	EXPECT_EQ(33U, indices[101]);

	EXPECT_EQ(31U, indices[102]);
	EXPECT_EQ(32U, indices[103]);
	EXPECT_EQ(28U, indices[104]);

	EXPECT_EQ(29U, indices[105]);
	EXPECT_EQ(31U, indices[106]);
	EXPECT_EQ(28U, indices[107]);

	EXPECT_EQ(30U, indices[108]);
	EXPECT_EQ(31U, indices[109]);
	EXPECT_EQ(29U, indices[110]);
}

TEST_F(SimplePolygonTest, SawtoothRightCW)
{
	// Test a combination of vertices that do and don't line up exactly.
	dsVector2d points[] =
	{
		{{0.0, 0.0}},
		{{10.0, 0.0}},
		{{11.0, 1.0}},
		{{10.0, 2.0}},
		{{11.0, 3.0}},
		{{10.0, 4.0}},
		{{11.0, 5.0}},
		{{9.5, 6.0}},
		{{11.0, 7.0}},
		{{10.0, 8.0}},
		{{11.0, 9.0}},
		{{10.0, 10.0}},
		{{11.0, 11.0}},
		{{10.5, 12.0}},
		{{11.0, 13.0}},
		{{10.0, 14.0}},
		{{11.0, 15.0}},
		{{10.0, 16.0}},
		{{0.0, 16.0}}
	};

	uint32_t indexCount;
	const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), NULL, dsTriangulateWinding_CW);
	ASSERT_EQ(51U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(7U, indices[0]);
	EXPECT_EQ(0U, indices[1]);
	EXPECT_EQ(18U, indices[2]);

	EXPECT_EQ(1U, indices[3]);
	EXPECT_EQ(0U, indices[4]);
	EXPECT_EQ(7U, indices[5]);

	EXPECT_EQ(3U, indices[6]);
	EXPECT_EQ(1U, indices[7]);
	EXPECT_EQ(7U, indices[8]);

	EXPECT_EQ(5U, indices[9]);
	EXPECT_EQ(3U, indices[10]);
	EXPECT_EQ(7U, indices[11]);

	EXPECT_EQ(6U, indices[12]);
	EXPECT_EQ(5U, indices[13]);
	EXPECT_EQ(7U, indices[14]);

	EXPECT_EQ(2U, indices[15]);
	EXPECT_EQ(1U, indices[16]);
	EXPECT_EQ(3U, indices[17]);

	EXPECT_EQ(4U, indices[18]);
	EXPECT_EQ(3U, indices[19]);
	EXPECT_EQ(5U, indices[20]);

	EXPECT_EQ(8U, indices[21]);
	EXPECT_EQ(7U, indices[22]);
	EXPECT_EQ(9U, indices[23]);

	EXPECT_EQ(10U, indices[24]);
	EXPECT_EQ(9U, indices[25]);
	EXPECT_EQ(11U, indices[26]);

	EXPECT_EQ(13U, indices[27]);
	EXPECT_EQ(11U, indices[28]);
	EXPECT_EQ(15U, indices[29]);

	EXPECT_EQ(12U, indices[30]);
	EXPECT_EQ(11U, indices[31]);
	EXPECT_EQ(13U, indices[32]);

	EXPECT_EQ(14U, indices[33]);
	EXPECT_EQ(13U, indices[34]);
	EXPECT_EQ(15U, indices[35]);

	EXPECT_EQ(9U, indices[36]);
	EXPECT_EQ(7U, indices[37]);
	EXPECT_EQ(18U, indices[38]);

	EXPECT_EQ(11U, indices[39]);
	EXPECT_EQ(9U, indices[40]);
	EXPECT_EQ(18U, indices[41]);

	EXPECT_EQ(15U, indices[42]);
	EXPECT_EQ(11U, indices[43]);
	EXPECT_EQ(18U, indices[44]);

	EXPECT_EQ(17U, indices[45]);
	EXPECT_EQ(15U, indices[46]);
	EXPECT_EQ(18U, indices[47]);

	EXPECT_EQ(16U, indices[48]);
	EXPECT_EQ(15U, indices[49]);
	EXPECT_EQ(17U, indices[50]);
}

TEST_F(SimplePolygonTest, SawtoothRightCCW)
{
	// Test a combination of vertices that do and don't line up exactly.
	dsVector2d points[] =
	{
		{{0.0, 0.0}},
		{{0.0, 16.0}},
		{{10.0, 16.0}},
		{{11.0, 15.0}},
		{{10.0, 14.0}},
		{{11.0, 13.0}},
		{{10.5, 12.0}},
		{{11.0, 11.0}},
		{{10.0, 10.0}},
		{{11.0, 9.0}},
		{{10.0, 8.0}},
		{{11.0, 7.0}},
		{{9.5, 6.0}},
		{{11.0, 5.0}},
		{{10.0, 4.0}},
		{{11.0, 3.0}},
		{{10.0, 2.0}},
		{{11.0, 1.0}},
		{{10.0, 0.0}}
	};

	uint32_t indexCount;
	const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), NULL, dsTriangulateWinding_CW);
	ASSERT_EQ(51U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(12U, indices[0]);
	EXPECT_EQ(0U, indices[1]);
	EXPECT_EQ(1U, indices[2]);

	EXPECT_EQ(18U, indices[3]);
	EXPECT_EQ(0U, indices[4]);
	EXPECT_EQ(12U, indices[5]);

	EXPECT_EQ(16U, indices[6]);
	EXPECT_EQ(18U, indices[7]);
	EXPECT_EQ(12U, indices[8]);

	EXPECT_EQ(14U, indices[9]);
	EXPECT_EQ(16U, indices[10]);
	EXPECT_EQ(12U, indices[11]);

	EXPECT_EQ(13U, indices[12]);
	EXPECT_EQ(14U, indices[13]);
	EXPECT_EQ(12U, indices[14]);

	EXPECT_EQ(10U, indices[15]);
	EXPECT_EQ(12U, indices[16]);
	EXPECT_EQ(1U, indices[17]);

	EXPECT_EQ(8U, indices[18]);
	EXPECT_EQ(10U, indices[19]);
	EXPECT_EQ(1U, indices[20]);

	EXPECT_EQ(4U, indices[21]);
	EXPECT_EQ(8U, indices[22]);
	EXPECT_EQ(1U, indices[23]);

	EXPECT_EQ(2U, indices[24]);
	EXPECT_EQ(4U, indices[25]);
	EXPECT_EQ(1U, indices[26]);

	EXPECT_EQ(3U, indices[27]);
	EXPECT_EQ(4U, indices[28]);
	EXPECT_EQ(2U, indices[29]);

	EXPECT_EQ(5U, indices[30]);
	EXPECT_EQ(6U, indices[31]);
	EXPECT_EQ(4U, indices[32]);

	EXPECT_EQ(6U, indices[33]);
	EXPECT_EQ(8U, indices[34]);
	EXPECT_EQ(4U, indices[35]);

	EXPECT_EQ(7U, indices[36]);
	EXPECT_EQ(8U, indices[37]);
	EXPECT_EQ(6U, indices[38]);

	EXPECT_EQ(9U, indices[39]);
	EXPECT_EQ(10U, indices[40]);
	EXPECT_EQ(8U, indices[41]);

	EXPECT_EQ(11U, indices[42]);
	EXPECT_EQ(12U, indices[43]);
	EXPECT_EQ(10U, indices[44]);

	EXPECT_EQ(15U, indices[45]);
	EXPECT_EQ(16U, indices[46]);
	EXPECT_EQ(14U, indices[47]);

	EXPECT_EQ(17U, indices[48]);
	EXPECT_EQ(18U, indices[49]);
	EXPECT_EQ(16U, indices[50]);
}

TEST_F(SimplePolygonTest, SawtoothLeftCW)
{
	// Test a combination of vertices that do and don't line up exactly.
	dsVector2d points[] =
	{
		{{1.0, 0.0}},
		{{10.0, 0.0}},
		{{10.0, 16.0}},
		{{1.0, 16.0}},
		{{0.0, 15.0}},
		{{1.0, 14.0}},
		{{0.0, 13.0}},
		{{0.5, 12.0}},
		{{0.0, 11.0}},
		{{1.0, 10.0}},
		{{0.0, 9.0}},
		{{1.0, 8.0}},
		{{0.0, 7.0}},
		{{1.5, 6.0}},
		{{0.0, 5.0}},
		{{1.0, 4.0}},
		{{0.0, 3.0}},
		{{1.0, 2.0}},
		{{0.0, 1.0}}
	};

	uint32_t indexCount;
	const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), NULL, dsTriangulateWinding_CW);
	ASSERT_EQ(51U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(17U, indices[0]);
	EXPECT_EQ(0U, indices[1]);
	EXPECT_EQ(18U, indices[2]);

	EXPECT_EQ(13U, indices[3]);
	EXPECT_EQ(17U, indices[4]);
	EXPECT_EQ(15U, indices[5]);

	EXPECT_EQ(13U, indices[6]);
	EXPECT_EQ(0U, indices[7]);
	EXPECT_EQ(17U, indices[8]);

	EXPECT_EQ(1U, indices[9]);
	EXPECT_EQ(0U, indices[10]);
	EXPECT_EQ(13U, indices[11]);

	EXPECT_EQ(13U, indices[12]);
	EXPECT_EQ(12U, indices[13]);
	EXPECT_EQ(11U, indices[14]);

	EXPECT_EQ(13U, indices[15]);
	EXPECT_EQ(11U, indices[16]);
	EXPECT_EQ(9U, indices[17]);

	EXPECT_EQ(13U, indices[18]);
	EXPECT_EQ(9U, indices[19]);
	EXPECT_EQ(5U, indices[20]);

	EXPECT_EQ(13U, indices[21]);
	EXPECT_EQ(5U, indices[22]);
	EXPECT_EQ(3U, indices[23]);

	EXPECT_EQ(1U, indices[24]);
	EXPECT_EQ(13U, indices[25]);
	EXPECT_EQ(3U, indices[26]);

	EXPECT_EQ(2U, indices[27]);
	EXPECT_EQ(1U, indices[28]);
	EXPECT_EQ(3U, indices[29]);

	EXPECT_EQ(3U, indices[30]);
	EXPECT_EQ(5U, indices[31]);
	EXPECT_EQ(4U, indices[32]);

	EXPECT_EQ(5U, indices[33]);
	EXPECT_EQ(9U, indices[34]);
	EXPECT_EQ(7U, indices[35]);

	EXPECT_EQ(5U, indices[36]);
	EXPECT_EQ(7U, indices[37]);
	EXPECT_EQ(6U, indices[38]);

	EXPECT_EQ(9U, indices[39]);
	EXPECT_EQ(8U, indices[40]);
	EXPECT_EQ(7U, indices[41]);

	EXPECT_EQ(9U, indices[42]);
	EXPECT_EQ(11U, indices[43]);
	EXPECT_EQ(10U, indices[44]);

	EXPECT_EQ(13U, indices[45]);
	EXPECT_EQ(15U, indices[46]);
	EXPECT_EQ(14U, indices[47]);

	EXPECT_EQ(15U, indices[48]);
	EXPECT_EQ(17U, indices[49]);
	EXPECT_EQ(16U, indices[50]);
}

TEST_F(SimplePolygonTest, SawtoothLeftCCW)
{
	// Test a combination of vertices that do and don't line up exactly.
	dsVector2d points[] =
	{
		{{1.0, 0.0}},
		{{0.0, 1.0}},
		{{1.0, 2.0}},
		{{0.0, 3.0}},
		{{1.0, 4.0}},
		{{0.0, 5.0}},
		{{1.5, 6.0}},
		{{0.0, 7.0}},
		{{1.0, 8.0}},
		{{0.0, 9.0}},
		{{1.0, 10.0}},
		{{0.0, 11.0}},
		{{0.5, 12.0}},
		{{0.0, 13.0}},
		{{1.0, 14.0}},
		{{0.0, 15.0}},
		{{1.0, 16.0}},
		{{10.0, 16.0}},
		{{10.0, 0.0}}
	};

	uint32_t indexCount;
	const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), NULL, dsTriangulateWinding_CW);
	ASSERT_EQ(51U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(2U, indices[0]);
	EXPECT_EQ(0U, indices[1]);
	EXPECT_EQ(1U, indices[2]);

	EXPECT_EQ(6U, indices[3]);
	EXPECT_EQ(2U, indices[4]);
	EXPECT_EQ(4U, indices[5]);

	EXPECT_EQ(6U, indices[6]);
	EXPECT_EQ(0U, indices[7]);
	EXPECT_EQ(2U, indices[8]);

	EXPECT_EQ(18U, indices[9]);
	EXPECT_EQ(0U, indices[10]);
	EXPECT_EQ(6U, indices[11]);

	EXPECT_EQ(4U, indices[12]);
	EXPECT_EQ(2U, indices[13]);
	EXPECT_EQ(3U, indices[14]);

	EXPECT_EQ(6U, indices[15]);
	EXPECT_EQ(4U, indices[16]);
	EXPECT_EQ(5U, indices[17]);

	EXPECT_EQ(6U, indices[18]);
	EXPECT_EQ(7U, indices[19]);
	EXPECT_EQ(8U, indices[20]);

	EXPECT_EQ(6U, indices[21]);
	EXPECT_EQ(8U, indices[22]);
	EXPECT_EQ(10U, indices[23]);

	EXPECT_EQ(6U, indices[24]);
	EXPECT_EQ(10U, indices[25]);
	EXPECT_EQ(14U, indices[26]);

	EXPECT_EQ(6U, indices[27]);
	EXPECT_EQ(14U, indices[28]);
	EXPECT_EQ(16U, indices[29]);

	EXPECT_EQ(18U, indices[30]);
	EXPECT_EQ(6U, indices[31]);
	EXPECT_EQ(16U, indices[32]);

	EXPECT_EQ(17U, indices[33]);
	EXPECT_EQ(18U, indices[34]);
	EXPECT_EQ(16U, indices[35]);

	EXPECT_EQ(10U, indices[36]);
	EXPECT_EQ(8U, indices[37]);
	EXPECT_EQ(9U, indices[38]);

	EXPECT_EQ(10U, indices[39]);
	EXPECT_EQ(11U, indices[40]);
	EXPECT_EQ(12U, indices[41]);

	EXPECT_EQ(14U, indices[42]);
	EXPECT_EQ(10U, indices[43]);
	EXPECT_EQ(12U, indices[44]);

	EXPECT_EQ(14U, indices[45]);
	EXPECT_EQ(12U, indices[46]);
	EXPECT_EQ(13U, indices[47]);

	EXPECT_EQ(16U, indices[48]);
	EXPECT_EQ(14U, indices[49]);
	EXPECT_EQ(15U, indices[50]);
}

TEST_F(SimplePolygonTest, HoleCW)
{
	dsVector2d points[] =
	{
		{{5.0, 3.0}},
		{{5.0, 5.0}},
		{{0.0, 5.0}},
		{{0.0, 0.0}},
		{{10.0, 0.0}},
		{{10.0, 5.0}},
		{{5.0, 5.0}},
		{{5.0, 3.0}},
		{{6.0, 3.0}},
		{{6.0, 2.0}},
		{{4.0, 2.0}},
		{{4.0, 3.0}}
	};

	uint32_t indexCount;
	const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), NULL, dsTriangulateWinding_CW);
	ASSERT_EQ(30U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(10U, indices[0]);
	EXPECT_EQ(3U, indices[1]);
	EXPECT_EQ(2U, indices[2]);

	EXPECT_EQ(11U, indices[3]);
	EXPECT_EQ(10U, indices[4]);
	EXPECT_EQ(2U, indices[5]);

	EXPECT_EQ(0U, indices[6]);
	EXPECT_EQ(11U, indices[7]);
	EXPECT_EQ(2U, indices[8]);

	EXPECT_EQ(1U, indices[9]);
	EXPECT_EQ(0U, indices[10]);
	EXPECT_EQ(2U, indices[11]);

	EXPECT_EQ(9U, indices[12]);
	EXPECT_EQ(3U, indices[13]);
	EXPECT_EQ(10U, indices[14]);

	EXPECT_EQ(4U, indices[15]);
	EXPECT_EQ(3U, indices[16]);
	EXPECT_EQ(9U, indices[17]);

	EXPECT_EQ(4U, indices[18]);
	EXPECT_EQ(9U, indices[19]);
	EXPECT_EQ(8U, indices[20]);

	EXPECT_EQ(5U, indices[21]);
	EXPECT_EQ(4U, indices[22]);
	EXPECT_EQ(8U, indices[23]);

	EXPECT_EQ(8U, indices[24]);
	EXPECT_EQ(7U, indices[25]);
	EXPECT_EQ(6U, indices[26]);

	EXPECT_EQ(5U, indices[27]);
	EXPECT_EQ(8U, indices[28]);
	EXPECT_EQ(6U, indices[29]);
}

TEST_F(SimplePolygonTest, HoleCCW)
{
	dsVector2d points[] =
	{
		{{5.0, 3.0}},
		{{4.0, 3.0}},
		{{4.0, 2.0}},
		{{6.0, 2.0}},
		{{6.0, 3.0}},
		{{5.0, 3.0}},
		{{5.0, 5.0}},
		{{10.0, 5.0}},
		{{10.0, 0.0}},
		{{0.0, 0.0}},
		{{0.0, 5.0}},
		{{5.0, 5.0}}
	};

	uint32_t indexCount;
	const uint32_t* indices = dsSimplePolygon_triangulate(&indexCount, polygon, points,
		DS_ARRAY_SIZE(points), NULL, dsTriangulateWinding_CW);
	ASSERT_EQ(30U, indexCount);
	ASSERT_TRUE(indices);
	EXPECT_EQ(2U, indices[0]);
	EXPECT_EQ(9U, indices[1]);
	EXPECT_EQ(10U, indices[2]);

	EXPECT_EQ(1U, indices[3]);
	EXPECT_EQ(2U, indices[4]);
	EXPECT_EQ(10U, indices[5]);

	EXPECT_EQ(0U, indices[6]);
	EXPECT_EQ(1U, indices[7]);
	EXPECT_EQ(10U, indices[8]);

	EXPECT_EQ(11U, indices[9]);
	EXPECT_EQ(0U, indices[10]);
	EXPECT_EQ(10U, indices[11]);

	EXPECT_EQ(3U, indices[12]);
	EXPECT_EQ(9U, indices[13]);
	EXPECT_EQ(2U, indices[14]);

	EXPECT_EQ(8U, indices[15]);
	EXPECT_EQ(9U, indices[16]);
	EXPECT_EQ(3U, indices[17]);

	EXPECT_EQ(8U, indices[18]);
	EXPECT_EQ(3U, indices[19]);
	EXPECT_EQ(4U, indices[20]);

	EXPECT_EQ(7U, indices[21]);
	EXPECT_EQ(8U, indices[22]);
	EXPECT_EQ(4U, indices[23]);

	EXPECT_EQ(4U, indices[24]);
	EXPECT_EQ(5U, indices[25]);
	EXPECT_EQ(6U, indices[26]);

	EXPECT_EQ(7U, indices[27]);
	EXPECT_EQ(4U, indices[28]);
	EXPECT_EQ(6U, indices[29]);
}

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
#include <DeepSea/Geometry/ComplexPolygon.h>
#include <gtest/gtest.h>

namespace
{

template <typename T>
struct Types
{
};

template <>
struct Types<float>
{
	using VectorType = dsVector2f;
	static const dsGeometryElement element;
	static const double epsilon;
};

const dsGeometryElement Types<float>::element = dsGeometryElement_Float;
const double Types<float>::epsilon = 1e-5;

template <>
struct Types<double>
{
	using VectorType = dsVector2d;
	static const dsGeometryElement element;
	static const double epsilon;
};

const dsGeometryElement Types<double>::element = dsGeometryElement_Double;
const double Types<double>::epsilon = 1e-14;

template <>
struct Types<int>
{
	using VectorType = dsVector2i;
	static const dsGeometryElement element;
	static const double epsilon;
};

const dsGeometryElement Types<int>::element = dsGeometryElement_Int;
const double Types<int>::epsilon = 0.5;

} // namespace

template <typename T>
class ComplexPolygonTest : public testing::Test
{
public:
	void SetUp() override
	{
		ASSERT_TRUE(dsSystemAllocator_initialize(&allocator, DS_ALLOCATOR_NO_LIMIT));
		polygon = dsComplexPolygon_create((dsAllocator*)&allocator, Types<T>::element, NULL);
		ASSERT_TRUE(polygon);
	}

	void TearDown() override
	{
		dsComplexPolygon_destroy(polygon);
		EXPECT_EQ(0U, ((dsAllocator*)&allocator)->size);
	}

	dsSystemAllocator allocator;
	dsComplexPolygon* polygon;
};

using ComplexPolygonTypes = testing::Types<float, double, int>;
TYPED_TEST_CASE(ComplexPolygonTest, ComplexPolygonTypes);

TYPED_TEST(ComplexPolygonTest, StarEvenOdd)
{
	using VectorType = typename Types<TypeParam>::VectorType;

	VectorType points[] =
	{
		{{-5, -10}}, {{0, 10}}, {{5, -10}}, {{-10, 5}}, {{10, 5}}
	};
	dsComplexPolygonLoop inputLoop = {points, DS_ARRAY_SIZE(points)};

	EXPECT_TRUE(dsComplexPolygon_simplify(this->polygon, &inputLoop, 1, NULL,
		dsPolygonFillRule_EvenOdd));

	ASSERT_EQ(5U, dsComplexPolygon_getLoopCount(this->polygon));

	const dsComplexPolygonLoop* loop = dsComplexPolygon_getLoop(this->polygon, 0);
	ASSERT_TRUE(loop);
	ASSERT_EQ(3U, loop->pointCount);
	auto loopPoints = (const VectorType*)loop->points;
	EXPECT_NEAR(0, loopPoints[0].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(10, loopPoints[0].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-1.25, loopPoints[1].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(5, loopPoints[1].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(1.25, loopPoints[2].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(5, loopPoints[2].y, Types<TypeParam>::epsilon);

	loop = dsComplexPolygon_getLoop(this->polygon, 1);
	ASSERT_TRUE(loop);
	ASSERT_EQ(3U, loop->pointCount);
	loopPoints = (const VectorType*)loop->points;
	EXPECT_NEAR(-1.25, loopPoints[0].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(5, loopPoints[0].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-10, loopPoints[1].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(5, loopPoints[1].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-3, loopPoints[2].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-2, loopPoints[2].y, Types<TypeParam>::epsilon);

	loop = dsComplexPolygon_getLoop(this->polygon, 2);
	ASSERT_TRUE(loop);
	ASSERT_EQ(3U, loop->pointCount);
	loopPoints = (const VectorType*)loop->points;
	EXPECT_NEAR(0, loopPoints[0].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-5, loopPoints[0].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(5, loopPoints[1].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-10, loopPoints[1].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(3, loopPoints[2].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-2, loopPoints[2].y, Types<TypeParam>::epsilon);

	loop = dsComplexPolygon_getLoop(this->polygon, 3);
	ASSERT_TRUE(loop);
	ASSERT_EQ(3U, loop->pointCount);
	loopPoints = (const VectorType*)loop->points;
	EXPECT_NEAR(3, loopPoints[0].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-2, loopPoints[0].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(10, loopPoints[1].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(5, loopPoints[1].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(1.25, loopPoints[2].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(5, loopPoints[2].y, Types<TypeParam>::epsilon);

	loop = dsComplexPolygon_getLoop(this->polygon, 4);
	ASSERT_TRUE(loop);
	ASSERT_EQ(3U, loop->pointCount);
	loopPoints = (const VectorType*)loop->points;
	EXPECT_NEAR(-5, loopPoints[0].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-10, loopPoints[0].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(0, loopPoints[1].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-5, loopPoints[1].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-3, loopPoints[2].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-2, loopPoints[2].y, Types<TypeParam>::epsilon);
}

TYPED_TEST(ComplexPolygonTest, StarNonZero)
{
	using VectorType = typename Types<TypeParam>::VectorType;

	VectorType points[] =
	{
		{{-5, -10}}, {{0, 10}}, {{5, -10}}, {{-10, 5}}, {{10, 5}}
	};
	dsComplexPolygonLoop inputLoop = {points, DS_ARRAY_SIZE(points)};

	EXPECT_TRUE(dsComplexPolygon_simplify(this->polygon, &inputLoop, 1, NULL,
		dsPolygonFillRule_NonZero));

	ASSERT_EQ(1U, dsComplexPolygon_getLoopCount(this->polygon));
	const dsComplexPolygonLoop* loop = dsComplexPolygon_getLoop(this->polygon, 0);
	ASSERT_TRUE(loop);
	ASSERT_EQ(10U, loop->pointCount);
	auto loopPoints = (const VectorType*)loop->points;
	EXPECT_NEAR(3, loopPoints[0].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-2, loopPoints[0].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(10, loopPoints[1].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(5, loopPoints[1].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(1.25, loopPoints[2].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(5, loopPoints[2].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(0, loopPoints[3].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(10, loopPoints[3].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-1.25, loopPoints[4].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(5, loopPoints[4].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-10, loopPoints[5].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(5, loopPoints[5].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-3, loopPoints[6].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-2, loopPoints[6].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-5, loopPoints[7].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-10, loopPoints[7].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(0, loopPoints[8].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-5, loopPoints[8].y, Types<TypeParam>::epsilon);
	EXPECT_NEAR(5, loopPoints[9].x, Types<TypeParam>::epsilon);
	EXPECT_NEAR(-10, loopPoints[9].y, Types<TypeParam>::epsilon);
}

/*
 * Copyright 2016-2023 Aaron Barany
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

#include <DeepSea/Geometry/AlignedBox2.h>
#include <DeepSea/Math/Matrix33.h>
#include <gtest/gtest.h>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

template <typename T>
struct AlignedBox2TypeSelector;

template <>
struct AlignedBox2TypeSelector<float>
{
	typedef dsVector2f Vector2Type;
	typedef dsAlignedBox2f AlignedBox2Type;
	typedef dsMatrix33f Matrix33Type;
	typedef dsVector3f Vector3Type;
	static const float epsilon;
};

template <>
struct AlignedBox2TypeSelector<double>
{
	typedef dsVector2d Vector2Type;
	typedef dsAlignedBox2d AlignedBox2Type;
	typedef dsMatrix33d Matrix33Type;
	typedef dsVector3d Vector3Type;
	static const double epsilon;
};

template <>
struct AlignedBox2TypeSelector<int>
{
	typedef dsVector2i Vector2Type;
	typedef dsAlignedBox2i AlignedBox2Type;
};

const float AlignedBox2TypeSelector<float>::epsilon = 1e-4f;
const double AlignedBox2TypeSelector<double>::epsilon = 1e-13f;

template <typename T>
class AlignedBox2Test : public testing::Test
{
};

using AlignedBox2Types = testing::Types<float, double, int>;
TYPED_TEST_SUITE(AlignedBox2Test, AlignedBox2Types);

template <typename T>
class AlignedBox2FloatTest : public testing::Test
{
};

using AlignedBox2FloatTypes = testing::Types<float, double>;
TYPED_TEST_SUITE(AlignedBox2FloatTest, AlignedBox2FloatTypes);

inline void dsAlignedBox2_makeInvalid(dsAlignedBox2f* result)
{
	dsAlignedBox2f_makeInvalid(result);
}

inline void dsAlignedBox2_makeInvalid(dsAlignedBox2d* result)
{
	dsAlignedBox2d_makeInvalid(result);
}

inline void dsAlignedBox2_makeInvalid(dsAlignedBox2i* result)
{
	dsAlignedBox2i_makeInvalid(result);
}

inline float dsAlignedBox2_dist2(const dsAlignedBox2f* box, const dsVector2f* point)
{
	return dsAlignedBox2f_dist2(box, point);
}

inline double dsAlignedBox2_dist2(const dsAlignedBox2d* box, const dsVector2d* point)
{
	return dsAlignedBox2d_dist2(box, point);
}

inline int dsAlignedBox2_dist2(const dsAlignedBox2i* box, const dsVector2i* point)
{
	return dsAlignedBox2i_dist2(box, point);
}

inline float dsAlignedBox2_dist(const dsAlignedBox2f* box, const dsVector2f* point)
{
	return dsAlignedBox2f_dist(box, point);
}

inline double dsAlignedBox2_dist(const dsAlignedBox2d* box, const dsVector2d* point)
{
	return dsAlignedBox2d_dist(box, point);
}

inline double dsAlignedBox2_dist(const dsAlignedBox2i* box, const dsVector2i* point)
{
	return dsAlignedBox2i_dist(box, point);
}

TYPED_TEST(AlignedBox2Test, Initialize)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;

	AlignedBox2Type box = {{{0, 1}}, {{2, 3}}};
	EXPECT_EQ((TypeParam)0, box.min.x);
	EXPECT_EQ((TypeParam)1, box.min.y);
	EXPECT_EQ((TypeParam)2, box.max.x);
	EXPECT_EQ((TypeParam)3, box.max.y);
}

TYPED_TEST(AlignedBox2Test, IsValid)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;

	AlignedBox2Type box = {{{0, 0}}, {{1, 1}}};
	EXPECT_TRUE(dsAlignedBox2_isValid(box));

	box.min.x = 2;
	EXPECT_FALSE(dsAlignedBox2_isValid(box));

	box.min.x = 0;
	box.min.y = 2;
	EXPECT_FALSE(dsAlignedBox2_isValid(box));
}

TYPED_TEST(AlignedBox2Test, AddPoint)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;
	typedef typename AlignedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;

	AlignedBox2Type box = {{{0, 1}}, {{2, 3}}};

	Vector2Type point1 = {{0, 3}};
	Vector2Type point2 = {{1, 2}};
	Vector2Type point3 = {{-1, 1}};
	Vector2Type point4 = {{0, -2}};
	Vector2Type point5 = {{3, 1}};
	Vector2Type point6 = {{0, 4}};

	dsAlignedBox2_addPoint(box, point1);
	EXPECT_EQ((TypeParam)0, box.min.x);
	EXPECT_EQ((TypeParam)1, box.min.y);
	EXPECT_EQ((TypeParam)2, box.max.x);
	EXPECT_EQ((TypeParam)3, box.max.y);

	dsAlignedBox2_addPoint(box, point2);
	EXPECT_EQ((TypeParam)0, box.min.x);
	EXPECT_EQ((TypeParam)1, box.min.y);
	EXPECT_EQ((TypeParam)2, box.max.x);
	EXPECT_EQ((TypeParam)3, box.max.y);

	dsAlignedBox2_addPoint(box, point3);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)1, box.min.y);
	EXPECT_EQ((TypeParam)2, box.max.x);
	EXPECT_EQ((TypeParam)3, box.max.y);

	dsAlignedBox2_addPoint(box, point4);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)2, box.max.x);
	EXPECT_EQ((TypeParam)3, box.max.y);

	dsAlignedBox2_addPoint(box, point5);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)3, box.max.y);

	dsAlignedBox2_addPoint(box, point6);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
}

TYPED_TEST(AlignedBox2Test, AddBox)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;

	AlignedBox2Type box = {{{0, 1}}, {{2, 3}}};

	AlignedBox2Type box1 = {{{1, 1}}, {{2, 2}}};
	AlignedBox2Type box2 = {{{-1, 1}}, {{2, 2}}};
	AlignedBox2Type box3 = {{{1, -2}}, {{2, 2}}};
	AlignedBox2Type box4 = {{{1, 1}}, {{3, 2}}};
	AlignedBox2Type box5 = {{{1, 1}}, {{2, 4}}};

	dsAlignedBox2_addBox(box, box1);
	EXPECT_EQ((TypeParam)0, box.min.x);
	EXPECT_EQ((TypeParam)1, box.min.y);
	EXPECT_EQ((TypeParam)2, box.max.x);
	EXPECT_EQ((TypeParam)3, box.max.y);

	dsAlignedBox2_addBox(box, box2);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)1, box.min.y);
	EXPECT_EQ((TypeParam)2, box.max.x);
	EXPECT_EQ((TypeParam)3, box.max.y);

	dsAlignedBox2_addBox(box, box3);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)2, box.max.x);
	EXPECT_EQ((TypeParam)3, box.max.y);

	dsAlignedBox2_addBox(box, box4);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)3, box.max.y);

	dsAlignedBox2_addBox(box, box5);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
}

TYPED_TEST(AlignedBox2Test, ContainsPoint)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;
	typedef typename AlignedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;

	AlignedBox2Type box = {{{0, 1}}, {{2, 3}}};

	Vector2Type point1 = {{1, 2}};
	Vector2Type point2 = {{-1, 2}};
	Vector2Type point3 = {{1, -2}};
	Vector2Type point4 = {{3, 2}};
	Vector2Type point5 = {{1, 4}};

	EXPECT_TRUE(dsAlignedBox2_containsPoint(box, box.min));
	EXPECT_TRUE(dsAlignedBox2_containsPoint(box, box.max));
	EXPECT_TRUE(dsAlignedBox2_containsPoint(box, point1));
	EXPECT_FALSE(dsAlignedBox2_containsPoint(box, point2));
	EXPECT_FALSE(dsAlignedBox2_containsPoint(box, point3));
	EXPECT_FALSE(dsAlignedBox2_containsPoint(box, point4));
	EXPECT_FALSE(dsAlignedBox2_containsPoint(box, point5));
}

TYPED_TEST(AlignedBox2Test, ContainsBox)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;

	AlignedBox2Type box = {{{0, 1}}, {{4, 5}}};

	AlignedBox2Type box1 = {{{1, 2}}, {{3, 4}}};
	AlignedBox2Type box2 = {{{-1, 2}}, {{3, 4}}};
	AlignedBox2Type box3 = {{{1, -2}}, {{3, 4}}};
	AlignedBox2Type box4 = {{{1, 2}}, {{5, 4}}};
	AlignedBox2Type box5 = {{{1, 2}}, {{3, 6}}};
	AlignedBox2Type box6 = {{{-4, 2}}, {{-2, 4}}};
	AlignedBox2Type box7 = {{{6, 2}}, {{8, 4}}};
	AlignedBox2Type box8 = {{{1, -2}}, {{3, -1}}};
	AlignedBox2Type box9 = {{{1, 6}}, {{3, 7}}};

	EXPECT_TRUE(dsAlignedBox2_containsBox(box, box));
	EXPECT_TRUE(dsAlignedBox2_containsBox(box, box1));
	EXPECT_FALSE(dsAlignedBox2_containsBox(box, box2));
	EXPECT_FALSE(dsAlignedBox2_containsBox(box, box3));
	EXPECT_FALSE(dsAlignedBox2_containsBox(box, box4));
	EXPECT_FALSE(dsAlignedBox2_containsBox(box, box5));
	EXPECT_FALSE(dsAlignedBox2_containsBox(box, box6));
	EXPECT_FALSE(dsAlignedBox2_containsBox(box, box7));
	EXPECT_FALSE(dsAlignedBox2_containsBox(box, box8));
	EXPECT_FALSE(dsAlignedBox2_containsBox(box, box9));
}

TYPED_TEST(AlignedBox2Test, Intersects)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;

	AlignedBox2Type box = {{{0, 1}}, {{4, 5}}};

	AlignedBox2Type box1 = {{{1, 2}}, {{3, 4}}};
	AlignedBox2Type box2 = {{{-1, 2}}, {{3, 4}}};
	AlignedBox2Type box3 = {{{1, -2}}, {{3, 4}}};
	AlignedBox2Type box4 = {{{1, 2}}, {{5, 4}}};
	AlignedBox2Type box5 = {{{1, 2}}, {{3, 6}}};
	AlignedBox2Type box6 = {{{-4, 2}}, {{-2, 4}}};
	AlignedBox2Type box7 = {{{6, 2}}, {{8, 4}}};
	AlignedBox2Type box8 = {{{1, -2}}, {{3, -1}}};
	AlignedBox2Type box9 = {{{1, 6}}, {{3, 7}}};

	EXPECT_TRUE(dsAlignedBox2_intersects(box, box));
	EXPECT_TRUE(dsAlignedBox2_intersects(box, box1));
	EXPECT_TRUE(dsAlignedBox2_intersects(box, box2));
	EXPECT_TRUE(dsAlignedBox2_intersects(box, box3));
	EXPECT_TRUE(dsAlignedBox2_intersects(box, box4));
	EXPECT_TRUE(dsAlignedBox2_intersects(box, box5));
	EXPECT_FALSE(dsAlignedBox2_intersects(box, box6));
	EXPECT_FALSE(dsAlignedBox2_intersects(box, box7));
	EXPECT_FALSE(dsAlignedBox2_intersects(box, box8));
	EXPECT_FALSE(dsAlignedBox2_intersects(box, box9));
}

TYPED_TEST(AlignedBox2Test, Intersect)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;

	AlignedBox2Type box = {{{0, 1}}, {{4, 5}}};

	AlignedBox2Type box1 = {{{1, 2}}, {{3, 4}}};
	AlignedBox2Type box2 = {{{-1, 2}}, {{3, 4}}};
	AlignedBox2Type box3 = {{{1, -2}}, {{3, 4}}};
	AlignedBox2Type box4 = {{{1, 2}}, {{5, 4}}};
	AlignedBox2Type box5 = {{{1, 2}}, {{3, 6}}};
	AlignedBox2Type box6 = {{{-4, 2}}, {{-2, 4}}};
	AlignedBox2Type box7 = {{{6, 2}}, {{8, 4}}};
	AlignedBox2Type box8 = {{{1, -2}}, {{3, -1}}};
	AlignedBox2Type box9 = {{{1, 6}}, {{3, 7}}};

	AlignedBox2Type intersection;
	dsAlignedBox2_intersect(intersection, box, box);
	EXPECT_EQ((TypeParam)0, intersection.min.x);
	EXPECT_EQ((TypeParam)1, intersection.min.y);
	EXPECT_EQ((TypeParam)4, intersection.max.x);
	EXPECT_EQ((TypeParam)5, intersection.max.y);

	dsAlignedBox2_intersect(intersection, box, box1);
	EXPECT_EQ((TypeParam)1, intersection.min.x);
	EXPECT_EQ((TypeParam)2, intersection.min.y);
	EXPECT_EQ((TypeParam)3, intersection.max.x);
	EXPECT_EQ((TypeParam)4, intersection.max.y);

	dsAlignedBox2_intersect(intersection, box, box2);
	EXPECT_EQ((TypeParam)0, intersection.min.x);
	EXPECT_EQ((TypeParam)2, intersection.min.y);
	EXPECT_EQ((TypeParam)3, intersection.max.x);
	EXPECT_EQ((TypeParam)4, intersection.max.y);

	dsAlignedBox2_intersect(intersection, box, box3);
	EXPECT_EQ((TypeParam)1, intersection.min.x);
	EXPECT_EQ((TypeParam)1, intersection.min.y);
	EXPECT_EQ((TypeParam)3, intersection.max.x);
	EXPECT_EQ((TypeParam)4, intersection.max.y);

	dsAlignedBox2_intersect(intersection, box, box4);
	EXPECT_EQ((TypeParam)1, intersection.min.x);
	EXPECT_EQ((TypeParam)2, intersection.min.y);
	EXPECT_EQ((TypeParam)4, intersection.max.x);
	EXPECT_EQ((TypeParam)4, intersection.max.y);

	dsAlignedBox2_intersect(intersection, box, box5);
	EXPECT_EQ((TypeParam)1, intersection.min.x);
	EXPECT_EQ((TypeParam)2, intersection.min.y);
	EXPECT_EQ((TypeParam)3, intersection.max.x);
	EXPECT_EQ((TypeParam)5, intersection.max.y);

	dsAlignedBox2_intersect(intersection, box, box6);
	EXPECT_FALSE(dsAlignedBox2_isValid(intersection));

	dsAlignedBox2_intersect(intersection, box, box7);
	EXPECT_FALSE(dsAlignedBox2_isValid(intersection));

	dsAlignedBox2_intersect(intersection, box, box8);
	EXPECT_FALSE(dsAlignedBox2_isValid(intersection));

	dsAlignedBox2_intersect(intersection, box, box9);
	EXPECT_FALSE(dsAlignedBox2_isValid(intersection));
}

TYPED_TEST(AlignedBox2Test, Center)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;
	typedef typename AlignedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;

	AlignedBox2Type box = {{{0, 1}}, {{4, 5}}};

	Vector2Type center;
	dsAlignedBox2_center(center, box);
	EXPECT_EQ((TypeParam)2, center.x);
	EXPECT_EQ((TypeParam)3, center.y);
}

TYPED_TEST(AlignedBox2Test, Extents)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;
	typedef typename AlignedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;

	AlignedBox2Type box = {{{0, 1}}, {{4, 6}}};

	Vector2Type extents;
	dsAlignedBox2_extents(extents, box);
	EXPECT_EQ((TypeParam)4, extents.x);
	EXPECT_EQ((TypeParam)5, extents.y);
}

TYPED_TEST(AlignedBox2FloatTest, ToMatrix)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;
	typedef typename AlignedBox2TypeSelector<TypeParam>::Matrix33Type Matrix33Type;
	typedef typename AlignedBox2TypeSelector<TypeParam>::Vector3Type Vector3Type;
	TypeParam epsilon = AlignedBox2TypeSelector<TypeParam>::epsilon;

	AlignedBox2Type box = {{{0, 1}}, {{4, 6}}};

	Matrix33Type matrix;
	dsAlignedBox2_toMatrix(matrix, box);

	Vector3Type lowerLeft = {{-1, -1, 1}};
	Vector3Type boxPoint;
	dsMatrix33_transform(boxPoint, matrix, lowerLeft);
	EXPECT_NEAR(box.min.x, boxPoint.x, epsilon);
	EXPECT_NEAR(box.min.y, boxPoint.y, epsilon);

	Vector3Type upperRight = {{1, 1, 1}};
	dsMatrix33_transform(boxPoint, matrix, upperRight);
	EXPECT_NEAR(box.max.x, boxPoint.x, epsilon);
	EXPECT_NEAR(box.max.y, boxPoint.y, epsilon);
}

TYPED_TEST(AlignedBox2FloatTest, ToMatrixTranspose)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;
	typedef typename AlignedBox2TypeSelector<TypeParam>::Matrix33Type Matrix33Type;

	AlignedBox2Type box = {{{0, 1}}, {{4, 6}}};

	Matrix33Type matrix, transposedMatrix;
	dsAlignedBox2_toMatrix(matrix, box);
	dsAlignedBox2_toMatrixTranspose(transposedMatrix, box);

	for (unsigned int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
			EXPECT_EQ(matrix.values[j][i], transposedMatrix.values[i][j]);
	}
}

TYPED_TEST(AlignedBox2Test, Corners)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;
	typedef typename AlignedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;

	AlignedBox2Type box = {{{0, 1}}, {{4, 6}}};
	Vector2Type corners[DS_BOX2_CORNER_COUNT];

	dsAlignedBox2_corners(corners, box);

	EXPECT_EQ((TypeParam)0, corners[0].x);
	EXPECT_EQ((TypeParam)1, corners[0].y);

	EXPECT_EQ((TypeParam)0, corners[1].x);
	EXPECT_EQ((TypeParam)6, corners[1].y);

	EXPECT_EQ((TypeParam)4, corners[2].x);
	EXPECT_EQ((TypeParam)1, corners[2].y);

	EXPECT_EQ((TypeParam)4, corners[3].x);
	EXPECT_EQ((TypeParam)6, corners[3].y);
}

TYPED_TEST(AlignedBox2Test, ClosestPoint)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;
	typedef typename AlignedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;

	AlignedBox2Type box = {{{0, 1}}, {{2, 3}}};

	Vector2Type point1 = {{1, 2}};
	Vector2Type point2 = {{-1, 2}};
	Vector2Type point3 = {{1, -2}};
	Vector2Type point4 = {{3, 2}};
	Vector2Type point5 = {{1, 4}};

	Vector2Type closest;
	dsAlignedBox2_closestPoint(closest, box, box.min);
	EXPECT_EQ((TypeParam)0, closest.x);
	EXPECT_EQ((TypeParam)1, closest.y);

	dsAlignedBox2_closestPoint(closest, box, box.max);
	EXPECT_EQ((TypeParam)2, closest.x);
	EXPECT_EQ((TypeParam)3, closest.y);

	dsAlignedBox2_closestPoint(closest, box, point1);
	EXPECT_EQ((TypeParam)1, closest.x);
	EXPECT_EQ((TypeParam)2, closest.y);

	dsAlignedBox2_closestPoint(closest, box, point2);
	EXPECT_EQ((TypeParam)0, closest.x);
	EXPECT_EQ((TypeParam)2, closest.y);

	dsAlignedBox2_closestPoint(closest, box, point3);
	EXPECT_EQ((TypeParam)1, closest.x);
	EXPECT_EQ((TypeParam)1, closest.y);

	dsAlignedBox2_closestPoint(closest, box, point4);
	EXPECT_EQ((TypeParam)2, closest.x);
	EXPECT_EQ((TypeParam)2, closest.y);

	dsAlignedBox2_closestPoint(closest, box, point5);
	EXPECT_EQ((TypeParam)1, closest.x);
	EXPECT_EQ((TypeParam)3, closest.y);
}

TYPED_TEST(AlignedBox2Test, MakeInvalid)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;

	AlignedBox2Type box = {{{0, 1}}, {{2, 3}}};

	dsAlignedBox2_makeInvalid(&box);
	EXPECT_FALSE(dsAlignedBox2_isValid(box));
}

TYPED_TEST(AlignedBox2Test, Dist2)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;
	typedef typename AlignedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;

	AlignedBox2Type box = {{{0, 1}}, {{2, 3}}};

	Vector2Type point1 = {{1, 2}};
	Vector2Type point2 = {{-1, 2}};
	Vector2Type point3 = {{1, -2}};
	Vector2Type point4 = {{3, 2}};
	Vector2Type point5 = {{1, 5}};

	EXPECT_EQ((TypeParam)0, dsAlignedBox2_dist2(&box, &box.min));
	EXPECT_EQ((TypeParam)0, dsAlignedBox2_dist2(&box, &box.max));
	EXPECT_EQ((TypeParam)0, dsAlignedBox2_dist2(&box, &point1));
	EXPECT_EQ((TypeParam)1, dsAlignedBox2_dist2(&box, &point2));
	EXPECT_EQ((TypeParam)9, dsAlignedBox2_dist2(&box, &point3));
	EXPECT_EQ((TypeParam)1, dsAlignedBox2_dist2(&box, &point4));
	EXPECT_EQ((TypeParam)4, dsAlignedBox2_dist2(&box, &point5));
}

TYPED_TEST(AlignedBox2Test, Dist)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;
	typedef typename AlignedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;

	AlignedBox2Type box = {{{0, 1}}, {{2, 3}}};

	Vector2Type point1 = {{1, 2}};
	Vector2Type point2 = {{-1, 2}};
	Vector2Type point3 = {{1, -2}};
	Vector2Type point4 = {{3, 2}};
	Vector2Type point5 = {{1, 5}};

	EXPECT_FLOAT_EQ(0.0f, (float)dsAlignedBox2_dist(&box, &box.min));
	EXPECT_FLOAT_EQ(0.0f, (float)dsAlignedBox2_dist(&box, &box.max));
	EXPECT_FLOAT_EQ(0.0f, (float)dsAlignedBox2_dist(&box, &point1));
	EXPECT_FLOAT_EQ(1.0f, (float)dsAlignedBox2_dist(&box, &point2));
	EXPECT_FLOAT_EQ(3.0f, (float)dsAlignedBox2_dist(&box, &point3));
	EXPECT_FLOAT_EQ(1.0f, (float)dsAlignedBox2_dist(&box, &point4));
	EXPECT_FLOAT_EQ(2.0f, (float)dsAlignedBox2_dist(&box, &point5));
}

TEST(AlignedBox2Test, ConvertFloatToDouble)
{
	dsAlignedBox2f boxf = {{{0, 1}}, {{2, 3}}};

	dsAlignedBox2d boxd;
	dsConvertFloatToDouble(boxd, boxf);

	EXPECT_FLOAT_EQ(boxf.min.x, (float)boxd.min.x);
	EXPECT_FLOAT_EQ(boxf.min.y, (float)boxd.min.y);

	EXPECT_FLOAT_EQ(boxf.max.x, (float)boxd.max.x);
	EXPECT_FLOAT_EQ(boxf.max.y, (float)boxd.max.y);
}

TEST(AlignedBox2Test, ConvertDoubleToFloat)
{
	dsAlignedBox2d boxd = {{{0, 1}}, {{2, 3}}};

	dsAlignedBox2f boxf;
	dsConvertDoubleToFloat(boxf, boxd);

	EXPECT_FLOAT_EQ((float)boxd.min.x, boxf.min.x);
	EXPECT_FLOAT_EQ((float)boxd.min.y, boxf.min.y);

	EXPECT_FLOAT_EQ((float)boxd.max.x, boxf.max.x);
	EXPECT_FLOAT_EQ((float)boxd.max.y, boxf.max.y);
}

TEST(AlignedBox2Test, ConvertFloatToInt)
{
	dsAlignedBox2f boxf = {{{0, 1}}, {{2, 3}}};

	dsAlignedBox2i boxi;
	dsConvertFloatToInt(boxi, boxf);

	EXPECT_EQ(boxf.min.x, (float)boxi.min.x);
	EXPECT_EQ(boxf.min.y, (float)boxi.min.y);

	EXPECT_EQ(boxf.max.x, (float)boxi.max.x);
	EXPECT_EQ(boxf.max.y, (float)boxi.max.y);
}

TEST(AlignedBox2Test, ConvertIntToFloat)
{
	dsAlignedBox2i boxi = {{{0, 1}}, {{2, 3}}};

	dsAlignedBox2f boxf;
	dsConvertIntToFloat(boxf, boxi);

	EXPECT_EQ(boxi.min.x, (int)boxf.min.x);
	EXPECT_EQ(boxi.min.y, (int)boxf.min.y);

	EXPECT_EQ(boxi.max.x, (int)boxf.max.x);
	EXPECT_EQ(boxi.max.y, (int)boxf.max.y);
}

TEST(AlignedBox2Test, ConvertDoubleToInt)
{
	dsAlignedBox2d boxd = {{{0, 1}}, {{2, 3}}};

	dsAlignedBox2i boxi;
	dsConvertDoubleToInt(boxi, boxd);

	EXPECT_EQ(boxd.min.x, boxi.min.x);
	EXPECT_EQ(boxd.min.y, boxi.min.y);

	EXPECT_EQ(boxd.max.x, boxi.max.x);
	EXPECT_EQ(boxd.max.y, boxi.max.y);
}

TEST(AlignedBox2Test, ConvertIntToDouble)
{
	dsAlignedBox2i boxi = {{{0, 1}}, {{2, 3}}};

	dsAlignedBox2d boxd;
	dsConvertIntToDouble(boxd, boxi);

	EXPECT_EQ(boxi.min.x, (int)boxd.min.x);
	EXPECT_EQ(boxi.min.y, (int)boxd.min.y);

	EXPECT_EQ(boxi.max.x, (int)boxd.max.x);
	EXPECT_EQ(boxi.max.y, (int)boxd.max.y);
}

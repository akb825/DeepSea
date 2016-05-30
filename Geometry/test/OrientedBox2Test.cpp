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
 * limitations under the License.OO
 */

#include <DeepSea/Geometry/OrientedBox2.h>
#include <DeepSea/Math/Matrix33.h>
#include <gtest/gtest.h>

template <typename T>
struct OrientedBox2TypeSelector;

template <>
struct OrientedBox2TypeSelector<float>
{
	typedef dsVector2f Vector2Type;
	typedef dsVector3f Vector3Type;
	typedef dsAlignedBox2f AlignedBox2Type;
	typedef dsMatrix33f Matrix33Type;
	typedef dsOrientedBox2f OrientedBox2Type;
	static const float epsilon;
};

template <>
struct OrientedBox2TypeSelector<double>
{
	typedef dsVector2d Vector2Type;
	typedef dsVector3d Vector3Type;
	typedef dsAlignedBox2d AlignedBox2Type;
	typedef dsMatrix33d Matrix33Type;
	typedef dsOrientedBox2d OrientedBox2Type;
	static const double epsilon;
};

const float OrientedBox2TypeSelector<float>::epsilon = 1e-4f;
const double OrientedBox2TypeSelector<double>::epsilon = 1e-13f;

template <typename T>
class OrientedBox2Test : public testing::Test
{
};

using OrientedBox2Types = testing::Types<float, double>;
TYPED_TEST_CASE(OrientedBox2Test, OrientedBox2Types);

inline bool dsOrientedBox2_transform(dsOrientedBox2f* box, const dsMatrix33f* transform)
{
	return dsOrientedBox2f_transform(box, transform);
}

inline bool dsOrientedBox2_transform(dsOrientedBox2d* box, const dsMatrix33d* transform)
{
	return dsOrientedBox2d_transform(box, transform);
}

inline void dsOrientedBox2_addPoint(dsOrientedBox2f* box, const dsVector2f* point)
{
	dsOrientedBox2f_addPoint(box, point);
}

inline void dsOrientedBox2_addPoint(dsOrientedBox2d* box, const dsVector2d* point)
{
	dsOrientedBox2d_addPoint(box, point);
}

inline bool dsOrientedBox2_addBox(dsOrientedBox2f* box, const dsOrientedBox2f* otherBox)
{
	return dsOrientedBox2f_addBox(box, otherBox);
}

inline bool dsOrientedBox2_addBox(dsOrientedBox2d* box, const dsOrientedBox2d* otherBox)
{
	return dsOrientedBox2d_addBox(box, otherBox);
}

inline bool dsOrientedBox2_corners(dsVector2f* corners, const dsOrientedBox2f* box)
{
	return dsOrientedBox2f_corners(corners, box);
}

inline bool dsOrientedBox2_corners(dsVector2d* corners, const dsOrientedBox2d* box)
{
	return dsOrientedBox2d_corners(corners, box);
}

inline bool dsOrientedBox2_intersects(const dsOrientedBox2f* box, const dsOrientedBox2f* otherBox)
{
	return dsOrientedBox2f_intersects(box, otherBox);
}

inline bool dsOrientedBox2_intersects(const dsOrientedBox2d* box, const dsOrientedBox2d* otherBox)
{
	return dsOrientedBox2d_intersects(box, otherBox);
}

inline bool dsOrientedBox2_closestPoint(dsVector2f* result, const dsOrientedBox2f* box,
	const dsVector2f* point)
{
	return dsOrientedBox2f_closestPoint(result, box, point);
}

inline bool dsOrientedBox2_closestPoint(dsVector2d* result, const dsOrientedBox2d* box,
	const dsVector2d* point)
{
	return dsOrientedBox2d_closestPoint(result, box, point);
}

inline float dsOrientedBox2_dist2(const dsOrientedBox2f* box, const dsVector2f* otherBox)
{
	return dsOrientedBox2f_dist2(box, otherBox);
}

inline double dsOrientedBox2_dist2(const dsOrientedBox2d* box, const dsVector2d* otherBox)
{
	return dsOrientedBox2d_dist2(box, otherBox);
}

inline float dsOrientedBox2_dist(const dsOrientedBox2f* box, const dsVector2f* otherBox)
{
	return dsOrientedBox2f_dist(box, otherBox);
}

inline double dsOrientedBox2_dist(const dsOrientedBox2d* box, const dsVector2d* otherBox)
{
	return dsOrientedBox2d_dist(box, otherBox);
}

inline void dsMatrix33_makeRotate(dsMatrix33f* result, float angle)
{
	dsMatrix33f_makeRotate(result, angle);
}

inline void dsMatrix33_makeRotate(dsMatrix33d* result, double angle)
{
	dsMatrix33d_makeRotate(result, angle);
}

inline void dsMatrix33_makeTranslate(dsMatrix33f* result, float x, float y)
{
	dsMatrix33f_makeTranslate(result, x, y);
}

inline void dsMatrix33_makeTranslate(dsMatrix33d* result, double x, double y)
{
	dsMatrix33d_makeTranslate(result, x, y);
}

inline void dsMatrix33_makeScale(dsMatrix33f* result, float x, float y)
{
	dsMatrix33f_makeScale(result, x, y);
}

inline void dsMatrix33_makeScale(dsMatrix33d* result, double x, double y)
{
	dsMatrix33d_makeScale(result, x, y);
}

TYPED_TEST(OrientedBox2Test, Initialize)
{
	typedef typename OrientedBox2TypeSelector<TypeParam>::OrientedBox2Type OrientedBox2Type;

	OrientedBox2Type box =
	{
		{{ {1, 0}, {0, 1} }},
		{{1, 2}}, {{3, 4}}
	};

	EXPECT_EQ(1, box.orientation.values[0][0]);
	EXPECT_EQ(0, box.orientation.values[0][1]);
	EXPECT_EQ(0, box.orientation.values[1][0]);
	EXPECT_EQ(1, box.orientation.values[1][1]);

	EXPECT_EQ(1, box.center.x);
	EXPECT_EQ(2, box.center.y);

	EXPECT_EQ(3, box.halfExtents.x);
	EXPECT_EQ(4, box.halfExtents.y);
}

TYPED_TEST(OrientedBox2Test, IsValid)
{
	typedef typename OrientedBox2TypeSelector<TypeParam>::OrientedBox2Type OrientedBox2Type;

	OrientedBox2Type box =
	{
		{{ {1, 0}, {0, 1} }},
		{{1, 2}}, {{3, 4}}
	};

	EXPECT_TRUE(dsOrientedBox2_isValid(box));

	box.halfExtents.x = -1;
	EXPECT_FALSE(dsOrientedBox2_isValid(box));

	box.halfExtents.x = 3;
	box.halfExtents.y = -1;
	EXPECT_FALSE(dsOrientedBox2_isValid(box));
}

TYPED_TEST(OrientedBox2Test, FromAlignedBox)
{
	typedef typename OrientedBox2TypeSelector<TypeParam>::OrientedBox2Type OrientedBox2Type;
	typedef typename OrientedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;

	OrientedBox2Type box =
	{
		{{ {0, 1}, {-1, 0} }},
		{{4, 3}}, {{2, 1}}
	};

	AlignedBox2Type alignedBox = {{0, 1}, {2, 5}};

	dsOrientedBox2_fromAlignedBox(box, alignedBox);
	EXPECT_EQ(1, box.orientation.values[0][0]);
	EXPECT_EQ(0, box.orientation.values[0][1]);
	EXPECT_EQ(0, box.orientation.values[1][0]);
	EXPECT_EQ(1, box.orientation.values[1][1]);

	EXPECT_EQ(1, box.center.x);
	EXPECT_EQ(3, box.center.y);

	EXPECT_EQ(1, box.halfExtents.x);
	EXPECT_EQ(2, box.halfExtents.y);
}

TYPED_TEST(OrientedBox2Test, MakeInvalid)
{
	typedef typename OrientedBox2TypeSelector<TypeParam>::OrientedBox2Type OrientedBox2Type;

	OrientedBox2Type box =
	{
		{{ {1, 0}, {0, 1} }},
		{{1, 2}}, {{3, 4}}
	};

	EXPECT_TRUE(dsOrientedBox2_isValid(box));

	dsOrientedBox2_makeInvalid(box);
	EXPECT_FALSE(dsOrientedBox2_isValid(box));
}

TYPED_TEST(OrientedBox2Test, Transform)
{
	typedef typename OrientedBox2TypeSelector<TypeParam>::OrientedBox2Type OrientedBox2Type;
	typedef typename OrientedBox2TypeSelector<TypeParam>::Matrix33Type Matrix33Type;
	typedef typename OrientedBox2TypeSelector<TypeParam>::Vector3Type Vector3Type;
	TypeParam epsilon = OrientedBox2TypeSelector<TypeParam>::epsilon;

	OrientedBox2Type box =
	{
		{{ {0, 1}, {-1, 0} }},
		{{4, 3}}, {{2, 1}}
	};

	Matrix33Type rotate, translate, scale, temp, transform;

	dsMatrix33_makeRotate(&rotate, (TypeParam)dsDegreesToRadians(30));
	dsMatrix33_makeTranslate(&translate, -2, 5);
	dsMatrix33_makeScale(&scale, 7, 8);

	dsMatrix33_mul(temp, rotate, scale);
	dsMatrix33_mul(transform, translate, temp);

	Vector3Type originalCenter = {box.center.x, box.center.y, 0};
	Vector3Type center;
	dsMatrix33_transform(center, transform, originalCenter);

	EXPECT_TRUE(dsOrientedBox2_transform(&box, &transform));

	EXPECT_NEAR(-rotate.values[0][1], box.orientation.values[0][0], epsilon);
	EXPECT_NEAR(rotate.values[1][1], box.orientation.values[0][1], epsilon);

	EXPECT_NEAR(-rotate.values[0][0], box.orientation.values[1][0], epsilon);
	EXPECT_NEAR(rotate.values[1][0], box.orientation.values[1][1], epsilon);

	EXPECT_NEAR(center.x, box.center.x, epsilon);
	EXPECT_NEAR(center.y, box.center.y, epsilon);

	EXPECT_NEAR(14, box.halfExtents.x, epsilon);
	EXPECT_NEAR(8, box.halfExtents.y, epsilon);
}

TYPED_TEST(OrientedBox2Test, AddPoint)
{
	typedef typename OrientedBox2TypeSelector<TypeParam>::OrientedBox2Type OrientedBox2Type;
	typedef typename OrientedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;
	TypeParam epsilon = OrientedBox2TypeSelector<TypeParam>::epsilon;

	OrientedBox2Type box =
	{
		{{ {0, 1}, {-1, 0} }},
		{{4, 3}}, {{2, 1}}
	};

	Vector2Type point1 = {4, 3};
	Vector2Type point2 = {0, 3};
	Vector2Type point3 = {4, -1};
	Vector2Type point4 = {8, 3};
	Vector2Type point5 = {4, 7};

	dsOrientedBox2_addPoint(&box, &point1);
	EXPECT_EQ(4, box.center.x);
	EXPECT_EQ(3, box.center.y);
	EXPECT_EQ(2, box.halfExtents.x);
	EXPECT_EQ(1, box.halfExtents.y);

	dsOrientedBox2_addPoint(&box, &point2);
	EXPECT_NEAR(2.5, box.center.x, epsilon);
	EXPECT_NEAR(3, box.center.y, epsilon);
	EXPECT_NEAR(2, box.halfExtents.x, epsilon);
	EXPECT_NEAR(2.5, box.halfExtents.y, epsilon);

	dsOrientedBox2_addPoint(&box, &point3);
	EXPECT_NEAR(2.5, box.center.x, epsilon);
	EXPECT_NEAR(2, box.center.y, epsilon);
	EXPECT_NEAR(3, box.halfExtents.x, epsilon);
	EXPECT_NEAR(2.5, box.halfExtents.y, epsilon);

	dsOrientedBox2_addPoint(&box, &point4);
	EXPECT_NEAR(4, box.center.x, epsilon);
	EXPECT_NEAR(2, box.center.y, epsilon);
	EXPECT_NEAR(3, box.halfExtents.x, epsilon);
	EXPECT_NEAR(4, box.halfExtents.y, epsilon);

	dsOrientedBox2_addPoint(&box, &point5);
	EXPECT_NEAR(4, box.center.x, epsilon);
	EXPECT_NEAR(3, box.center.y, epsilon);
	EXPECT_NEAR(4, box.halfExtents.x, epsilon);
	EXPECT_NEAR(4, box.halfExtents.y, epsilon);
}

TYPED_TEST(OrientedBox2Test, Corners)
{
	typedef typename OrientedBox2TypeSelector<TypeParam>::OrientedBox2Type OrientedBox2Type;
	typedef typename OrientedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;
	TypeParam epsilon = OrientedBox2TypeSelector<TypeParam>::epsilon;

	OrientedBox2Type box =
	{
		{{ {0, 1}, {-1, 0} }},
		{{4, 3}}, {{2, 1}}
	};

	Vector2Type corners[DS_BOX2_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox2_corners(corners, &box));

	EXPECT_NEAR(5, corners[0].x, epsilon);
	EXPECT_NEAR(1, corners[0].y, epsilon);

	EXPECT_NEAR(3, corners[1].x, epsilon);
	EXPECT_NEAR(1, corners[1].y, epsilon);

	EXPECT_NEAR(5, corners[2].x, epsilon);
	EXPECT_NEAR(5, corners[2].y, epsilon);

	EXPECT_NEAR(3, corners[3].x, epsilon);
	EXPECT_NEAR(5, corners[3].y, epsilon);
}

TYPED_TEST(OrientedBox2Test, AddBox)
{
	typedef typename OrientedBox2TypeSelector<TypeParam>::OrientedBox2Type OrientedBox2Type;
	typedef typename OrientedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;
	typedef typename OrientedBox2TypeSelector<TypeParam>::Matrix33Type Matrix33Type;
	TypeParam epsilon = OrientedBox2TypeSelector<TypeParam>::epsilon;

	OrientedBox2Type box =
	{
		{{ {0, 1}, {-1, 0} }},
		{{4, 3}}, {{2, 1}}
	};

	OrientedBox2Type otherBox =
	{
		{{ {1, 0}, {0, 1} }},
		{{1, 2}}, {{3, 4}}
	};

	Matrix33Type rotate, translate, scale, temp, transform;

	dsMatrix33_makeRotate(&rotate, (TypeParam)dsDegreesToRadians(30));
	dsMatrix33_makeTranslate(&translate, -2, 5);
	dsMatrix33_makeScale(&scale, 7, 8);

	dsMatrix33_mul(temp, rotate, scale);
	dsMatrix33_mul(transform, translate, temp);

	dsOrientedBox2_transform(&otherBox, &transform);

	Vector2Type otherBoxCorners[DS_BOX2_CORNER_COUNT];
	EXPECT_TRUE(dsOrientedBox2_corners(otherBoxCorners, &otherBox));

	OrientedBox2Type addPointsBox = box;
	dsOrientedBox2_addBox(&box, &otherBox);

	for (unsigned int i = 0; i < DS_BOX2_CORNER_COUNT; ++i)
		dsOrientedBox2_addPoint(&addPointsBox, otherBoxCorners + i);

	EXPECT_NEAR(addPointsBox.center.x, box.center.x, epsilon);
	EXPECT_NEAR(addPointsBox.center.y, box.center.y, epsilon);
	EXPECT_NEAR(addPointsBox.halfExtents.x, box.halfExtents.x, epsilon);
	EXPECT_NEAR(addPointsBox.halfExtents.y, box.halfExtents.y, epsilon);
}

TYPED_TEST(OrientedBox2Test, Intersects)
{
	typedef typename OrientedBox2TypeSelector<TypeParam>::OrientedBox2Type OrientedBox2Type;
	typedef typename OrientedBox2TypeSelector<TypeParam>::Matrix33Type Matrix33Type;

	OrientedBox2Type box =
	{
		{{ {0, 1}, {-1, 0} }},
		{{4, 3}}, {{2, 1}}
	};

	OrientedBox2Type otherBox =
	{
		{{ {1, 0}, {0, 1} }},
		{{0, 0}}, {{2, 1}}
	};

	Matrix33Type rotate;
	dsMatrix33_makeRotate(&rotate, (TypeParam)dsDegreesToRadians(30));
	dsOrientedBox2_transform(&otherBox, &rotate);

	otherBox.center.x = 4;
	otherBox.center.y = 3;
	EXPECT_TRUE(dsOrientedBox2_intersects(&box, &otherBox));

	otherBox.center.x = 2;
	otherBox.center.y = 3;
	EXPECT_TRUE(dsOrientedBox2_intersects(&box, &otherBox));

	otherBox.center.x = 5;
	otherBox.center.y = 3;
	EXPECT_TRUE(dsOrientedBox2_intersects(&box, &otherBox));

	otherBox.center.x = 4;
	otherBox.center.y = 2;
	EXPECT_TRUE(dsOrientedBox2_intersects(&box, &otherBox));

	otherBox.center.x = 4;
	otherBox.center.y = 4;
	EXPECT_TRUE(dsOrientedBox2_intersects(&box, &otherBox));

	otherBox.center.x = 0;
	otherBox.center.y = 3;
	EXPECT_FALSE(dsOrientedBox2_intersects(&box, &otherBox));

	otherBox.center.x = 8;
	otherBox.center.y = 3;
	EXPECT_FALSE(dsOrientedBox2_intersects(&box, &otherBox));

	otherBox.center.x = 4;
	otherBox.center.y = -1;
	EXPECT_FALSE(dsOrientedBox2_intersects(&box, &otherBox));

	otherBox.center.x = 4;
	otherBox.center.y = 7;
	EXPECT_FALSE(dsOrientedBox2_intersects(&box, &otherBox));
}

TYPED_TEST(OrientedBox2Test, ClosestPoint)
{
	typedef typename OrientedBox2TypeSelector<TypeParam>::OrientedBox2Type OrientedBox2Type;
	typedef typename OrientedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;

	OrientedBox2Type box =
	{
		{{ {0, 1}, {-1, 0} }},
		{{4, 3}}, {{2, 1}}
	};

	Vector2Type point1 = {3, 2};
	Vector2Type point2 = {2, 3};
	Vector2Type point3 = {4, 0};
	Vector2Type point4 = {6, 3};
	Vector2Type point5 = {4, 6};

	Vector2Type closest;
	dsOrientedBox2_closestPoint(&closest, &box, &box.center);
	EXPECT_EQ(box.center.x, closest.x);
	EXPECT_EQ(box.center.y, closest.y);

	dsOrientedBox2_closestPoint(&closest, &box, &point1);
	EXPECT_EQ(3, closest.x);
	EXPECT_EQ(2, closest.y);

	dsOrientedBox2_closestPoint(&closest, &box, &point2);
	EXPECT_EQ(3, closest.x);
	EXPECT_EQ(3, closest.y);

	dsOrientedBox2_closestPoint(&closest, &box, &point3);
	EXPECT_EQ(4, closest.x);
	EXPECT_EQ(1, closest.y);

	dsOrientedBox2_closestPoint(&closest, &box, &point4);
	EXPECT_EQ(5, closest.x);
	EXPECT_EQ(3, closest.y);

	dsOrientedBox2_closestPoint(&closest, &box, &point5);
	EXPECT_EQ(4, closest.x);
	EXPECT_EQ(5, closest.y);
}

TYPED_TEST(OrientedBox2Test, Dist2)
{
	typedef typename OrientedBox2TypeSelector<TypeParam>::OrientedBox2Type OrientedBox2Type;
	typedef typename OrientedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;

	OrientedBox2Type box =
	{
		{{ {0, 1}, {-1, 0} }},
		{{4, 3}}, {{2, 1}}
	};

	Vector2Type point1 = {3, 2};
	Vector2Type point2 = {2, 3};
	Vector2Type point3 = {4, -1};
	Vector2Type point4 = {6, 3};
	Vector2Type point5 = {4, 7};

	EXPECT_EQ(0, dsOrientedBox2_dist2(&box, &box.center));
	EXPECT_EQ(0, dsOrientedBox2_dist2(&box, &point1));
	EXPECT_EQ(1, dsOrientedBox2_dist2(&box, &point2));
	EXPECT_EQ(4, dsOrientedBox2_dist2(&box, &point3));
	EXPECT_EQ(1, dsOrientedBox2_dist2(&box, &point4));
	EXPECT_EQ(4, dsOrientedBox2_dist2(&box, &point5));
}

TYPED_TEST(OrientedBox2Test, Dist)
{
	typedef typename OrientedBox2TypeSelector<TypeParam>::OrientedBox2Type OrientedBox2Type;
	typedef typename OrientedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;

	OrientedBox2Type box =
	{
		{{ {0, 1}, {-1, 0} }},
		{{4, 3}}, {{2, 1}}
	};

	Vector2Type point1 = {3, 2};
	Vector2Type point2 = {2, 3};
	Vector2Type point3 = {4, -1};
	Vector2Type point4 = {6, 3};
	Vector2Type point5 = {4, 7};

	EXPECT_FLOAT_EQ(0.0f, (float)dsOrientedBox2_dist(&box, &box.center));
	EXPECT_FLOAT_EQ(0.0f, (float)dsOrientedBox2_dist(&box, &point1));
	EXPECT_FLOAT_EQ(1.0f, (float)dsOrientedBox2_dist(&box, &point2));
	EXPECT_FLOAT_EQ(2.0f, (float)dsOrientedBox2_dist(&box, &point3));
	EXPECT_FLOAT_EQ(1.0f, (float)dsOrientedBox2_dist(&box, &point4));
	EXPECT_FLOAT_EQ(2.0f, (float)dsOrientedBox2_dist(&box, &point5));
}

/*
 * Copyright 2016-2021 Aaron Barany
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

#include <DeepSea/Geometry/AlignedBox3.h>
#include <gtest/gtest.h>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

template <typename T>
struct AlignedBox3TypeSelector;

template <>
struct AlignedBox3TypeSelector<float>
{
	typedef dsVector3f Vector3Type;
	typedef dsAlignedBox3f AlignedBox3Type;
};

template <>
struct AlignedBox3TypeSelector<double>
{
	typedef dsVector3d Vector3Type;
	typedef dsAlignedBox3d AlignedBox3Type;
};

template <>
struct AlignedBox3TypeSelector<int>
{
	typedef dsVector3i Vector3Type;
	typedef dsAlignedBox3i AlignedBox3Type;
};

template <typename T>
class AlignedBox3Test : public testing::Test
{
};

using AlignedBox3Types = testing::Types<float, double, int>;
TYPED_TEST_SUITE(AlignedBox3Test, AlignedBox3Types);

inline void dsAlignedBox3_makeInvalid(dsAlignedBox3f* result)
{
	dsAlignedBox3f_makeInvalid(result);
}

inline void dsAlignedBox3_makeInvalid(dsAlignedBox3d* result)
{
	dsAlignedBox3d_makeInvalid(result);
}

inline void dsAlignedBox3_makeInvalid(dsAlignedBox3i* result)
{
	dsAlignedBox3i_makeInvalid(result);
}

inline float dsAlignedBox3_dist2(const dsAlignedBox3f* box, const dsVector3f* point)
{
	return dsAlignedBox3f_dist2(box, point);
}

inline double dsAlignedBox3_dist2(const dsAlignedBox3d* box, const dsVector3d* point)
{
	return dsAlignedBox3d_dist2(box, point);
}

inline int dsAlignedBox3_dist2(const dsAlignedBox3i* box, const dsVector3i* point)
{
	return dsAlignedBox3i_dist2(box, point);
}

inline float dsAlignedBox3_dist(const dsAlignedBox3f* box, const dsVector3f* point)
{
	return dsAlignedBox3f_dist(box, point);
}

inline double dsAlignedBox3_dist(const dsAlignedBox3d* box, const dsVector3d* point)
{
	return dsAlignedBox3d_dist(box, point);
}

inline double dsAlignedBox3_dist(const dsAlignedBox3i* box, const dsVector3i* point)
{
	return dsAlignedBox3i_dist(box, point);
}

TYPED_TEST(AlignedBox3Test, Initialize)
{
	typedef typename AlignedBox3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;

	AlignedBox3Type box = {{{0, 1, 2}}, {{3, 4, 5}}};
	EXPECT_EQ((TypeParam)0, box.min.x);
	EXPECT_EQ((TypeParam)1, box.min.y);
	EXPECT_EQ((TypeParam)2, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);
}

TYPED_TEST(AlignedBox3Test, IsValid)
{
	typedef typename AlignedBox3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;

	AlignedBox3Type box = {{{0, 0, 0}}, {{1, 1, 1}}};
	EXPECT_TRUE(dsAlignedBox3_isValid(box));

	box.min.x = 2;
	EXPECT_FALSE(dsAlignedBox3_isValid(box));

	box.min.x = 0;
	box.min.y = 2;
	EXPECT_FALSE(dsAlignedBox3_isValid(box));

	box.min.y = 0;
	box.min.z = 2;
	EXPECT_FALSE(dsAlignedBox3_isValid(box));
}

TYPED_TEST(AlignedBox3Test, AddPoint)
{
	typedef typename AlignedBox3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;
	typedef typename AlignedBox3TypeSelector<TypeParam>::Vector3Type Vector3Type;

	AlignedBox3Type box = {{{0, 1, 2}}, {{3, 4, 5}}};

	Vector3Type point1 = {{0, 4, 2}};
	Vector3Type point2 = {{3, 1, 5}};
	Vector3Type point3 = {{-1, 1, 2}};
	Vector3Type point4 = {{0, -2, 2}};
	Vector3Type point5 = {{0, 1, -3}};
	Vector3Type point6 = {{4, 1, 2}};
	Vector3Type point7 = {{0, 5, 2}};
	Vector3Type point8 = {{0, 1, 6}};

	dsAlignedBox3_addPoint(box, point1);
	EXPECT_EQ((TypeParam)0, box.min.x);
	EXPECT_EQ((TypeParam)1, box.min.y);
	EXPECT_EQ((TypeParam)2, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3_addPoint(box, point2);
	EXPECT_EQ((TypeParam)0, box.min.x);
	EXPECT_EQ((TypeParam)1, box.min.y);
	EXPECT_EQ((TypeParam)2, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3_addPoint(box, point3);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)1, box.min.y);
	EXPECT_EQ((TypeParam)2, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3_addPoint(box, point4);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)2, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3_addPoint(box, point5);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)-3, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3_addPoint(box, point6);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)-3, box.min.z);
	EXPECT_EQ((TypeParam)4, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3_addPoint(box, point7);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)-3, box.min.z);
	EXPECT_EQ((TypeParam)4, box.max.x);
	EXPECT_EQ((TypeParam)5, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3_addPoint(box, point8);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)-3, box.min.z);
	EXPECT_EQ((TypeParam)4, box.max.x);
	EXPECT_EQ((TypeParam)5, box.max.y);
	EXPECT_EQ((TypeParam)6, box.max.z);
}

TYPED_TEST(AlignedBox3Test, AddBox)
{
	typedef typename AlignedBox3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;

	AlignedBox3Type box = {{{0, 1, 2}}, {{3, 4, 5}}};

	AlignedBox3Type box1 = {{{2, 2, 2}}, {{3, 3, 3}}};
	AlignedBox3Type box2 = {{{-1, 1, 3}}, {{3, 3, 3}}};
	AlignedBox3Type box3 = {{{1, -2, 3}}, {{3, 3, 3}}};
	AlignedBox3Type box4 = {{{1, 2, -3}}, {{3, 3, 3}}};
	AlignedBox3Type box5 = {{{1, 2, 3}}, {{4, 3, 3}}};
	AlignedBox3Type box6 = {{{1, 2, 3}}, {{3, 5, 3}}};
	AlignedBox3Type box7 = {{{1, 2, 3}}, {{3, 3, 6}}};

	dsAlignedBox3_addBox(box, box1);
	EXPECT_EQ((TypeParam)0, box.min.x);
	EXPECT_EQ((TypeParam)1, box.min.y);
	EXPECT_EQ((TypeParam)2, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3_addBox(box, box2);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)1, box.min.y);
	EXPECT_EQ((TypeParam)2, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3_addBox(box, box3);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)2, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3_addBox(box, box4);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)-3, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3_addBox(box, box5);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)-3, box.min.z);
	EXPECT_EQ((TypeParam)4, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3_addBox(box, box6);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)-3, box.min.z);
	EXPECT_EQ((TypeParam)4, box.max.x);
	EXPECT_EQ((TypeParam)5, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3_addBox(box, box7);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)-3, box.min.z);
	EXPECT_EQ((TypeParam)4, box.max.x);
	EXPECT_EQ((TypeParam)5, box.max.y);
	EXPECT_EQ((TypeParam)6, box.max.z);
}

TYPED_TEST(AlignedBox3Test, ContainsPoint)
{
	typedef typename AlignedBox3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;
	typedef typename AlignedBox3TypeSelector<TypeParam>::Vector3Type Vector3Type;

	AlignedBox3Type box = {{{0, 1, 2}}, {{3, 4, 5}}};

	Vector3Type point1 = {{1, 2, 3}};
	Vector3Type point2 = {{-1, 2, 3}};
	Vector3Type point3 = {{1, -2, 3}};
	Vector3Type point4 = {{4, 2, 3}};
	Vector3Type point5 = {{1, 5, 3}};
	Vector3Type point6 = {{1, 2, 6}};

	EXPECT_TRUE(dsAlignedBox3_containsPoint(box, box.min));
	EXPECT_TRUE(dsAlignedBox3_containsPoint(box, box.max));
	EXPECT_TRUE(dsAlignedBox3_containsPoint(box, point1));
	EXPECT_FALSE(dsAlignedBox3_containsPoint(box, point2));
	EXPECT_FALSE(dsAlignedBox3_containsPoint(box, point3));
	EXPECT_FALSE(dsAlignedBox3_containsPoint(box, point4));
	EXPECT_FALSE(dsAlignedBox3_containsPoint(box, point5));
	EXPECT_FALSE(dsAlignedBox3_containsPoint(box, point6));
}

TYPED_TEST(AlignedBox3Test, ContainsBox)
{
	typedef typename AlignedBox3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;

	AlignedBox3Type box = {{{0, 1, 2}}, {{5, 6, 7}}};

	AlignedBox3Type box1 = {{{1, 2, 3}}, {{4, 5, 6}}};
	AlignedBox3Type box2 = {{{-1, 2, 3}}, {{4, 5, 6}}};
	AlignedBox3Type box3 = {{{1, -2, 3}}, {{4, 5, 6}}};
	AlignedBox3Type box4 = {{{1, 2, -3}}, {{4, 5, 6}}};
	AlignedBox3Type box5 = {{{1, 2, 3}}, {{7, 5, 6}}};
	AlignedBox3Type box6 = {{{1, 2, 3}}, {{4, 8, 6}}};
	AlignedBox3Type box7 = {{{1, 2, 3}}, {{4, 5, 9}}};
	AlignedBox3Type box8 = {{{-4, 2, 3}}, {{-2, 5, 6}}};
	AlignedBox3Type box9 = {{{1, -4, 3}}, {{4, -2, 6}}};
	AlignedBox3Type box10 = {{{1, 2, -4}}, {{4, 5, -2}}};
	AlignedBox3Type box11 = {{{8, 2, 3}}, {{10, 5, 6}}};
	AlignedBox3Type box12 = {{{1, 8, 3}}, {{4, 10, 6}}};
	AlignedBox3Type box13 = {{{1, 2, 8}}, {{4, 5, 10}}};

	EXPECT_TRUE(dsAlignedBox3_containsBox(box, box));
	EXPECT_TRUE(dsAlignedBox3_containsBox(box, box1));
	EXPECT_FALSE(dsAlignedBox3_containsBox(box, box2));
	EXPECT_FALSE(dsAlignedBox3_containsBox(box, box3));
	EXPECT_FALSE(dsAlignedBox3_containsBox(box, box4));
	EXPECT_FALSE(dsAlignedBox3_containsBox(box, box5));
	EXPECT_FALSE(dsAlignedBox3_containsBox(box, box6));
	EXPECT_FALSE(dsAlignedBox3_containsBox(box, box7));
	EXPECT_FALSE(dsAlignedBox3_containsBox(box, box8));
	EXPECT_FALSE(dsAlignedBox3_containsBox(box, box9));
	EXPECT_FALSE(dsAlignedBox3_containsBox(box, box10));
	EXPECT_FALSE(dsAlignedBox3_containsBox(box, box11));
	EXPECT_FALSE(dsAlignedBox3_containsBox(box, box12));
	EXPECT_FALSE(dsAlignedBox3_containsBox(box, box13));
}

TYPED_TEST(AlignedBox3Test, Intersects)
{
	typedef typename AlignedBox3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;

	AlignedBox3Type box = {{{0, 1, 2}}, {{5, 6, 7}}};

	AlignedBox3Type box1 = {{{1, 2, 3}}, {{4, 5, 6}}};
	AlignedBox3Type box2 = {{{-1, 2, 3}}, {{4, 5, 6}}};
	AlignedBox3Type box3 = {{{1, -2, 3}}, {{4, 5, 6}}};
	AlignedBox3Type box4 = {{{1, 2, -3}}, {{4, 5, 6}}};
	AlignedBox3Type box5 = {{{1, 2, 3}}, {{7, 5, 6}}};
	AlignedBox3Type box6 = {{{1, 2, 3}}, {{4, 8, 6}}};
	AlignedBox3Type box7 = {{{1, 2, 3}}, {{4, 5, 9}}};
	AlignedBox3Type box8 = {{{-4, 2, 3}}, {{-2, 5, 6}}};
	AlignedBox3Type box9 = {{{1, -4, 3}}, {{4, -2, 6}}};
	AlignedBox3Type box10 = {{{1, 2, -4}}, {{4, 5, -2}}};
	AlignedBox3Type box11 = {{{8, 2, 3}}, {{10, 5, 6}}};
	AlignedBox3Type box12 = {{{1, 8, 3}}, {{4, 10, 6}}};
	AlignedBox3Type box13 = {{{1, 2, 8}}, {{4, 5, 10}}};

	EXPECT_TRUE(dsAlignedBox3_intersects(box, box));
	EXPECT_TRUE(dsAlignedBox3_intersects(box, box1));
	EXPECT_TRUE(dsAlignedBox3_intersects(box, box2));
	EXPECT_TRUE(dsAlignedBox3_intersects(box, box3));
	EXPECT_TRUE(dsAlignedBox3_intersects(box, box4));
	EXPECT_TRUE(dsAlignedBox3_intersects(box, box5));
	EXPECT_TRUE(dsAlignedBox3_intersects(box, box6));
	EXPECT_TRUE(dsAlignedBox3_intersects(box, box7));
	EXPECT_FALSE(dsAlignedBox3_intersects(box, box8));
	EXPECT_FALSE(dsAlignedBox3_intersects(box, box9));
	EXPECT_FALSE(dsAlignedBox3_intersects(box, box10));
	EXPECT_FALSE(dsAlignedBox3_intersects(box, box11));
	EXPECT_FALSE(dsAlignedBox3_intersects(box, box12));
	EXPECT_FALSE(dsAlignedBox3_intersects(box, box13));
}

TYPED_TEST(AlignedBox3Test, Intersect)
{
	typedef typename AlignedBox3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;

	AlignedBox3Type box = {{{0, 1, 2}}, {{5, 6, 7}}};

	AlignedBox3Type box1 = {{{1, 2, 3}}, {{4, 5, 6}}};
	AlignedBox3Type box2 = {{{-1, 2, 3}}, {{4, 5, 6}}};
	AlignedBox3Type box3 = {{{1, -2, 3}}, {{4, 5, 6}}};
	AlignedBox3Type box4 = {{{1, 2, -3}}, {{4, 5, 6}}};
	AlignedBox3Type box5 = {{{1, 2, 3}}, {{7, 5, 6}}};
	AlignedBox3Type box6 = {{{1, 2, 3}}, {{4, 8, 6}}};
	AlignedBox3Type box7 = {{{1, 2, 3}}, {{4, 5, 9}}};
	AlignedBox3Type box8 = {{{-4, 2, 3}}, {{-2, 5, 6}}};
	AlignedBox3Type box9 = {{{1, -4, 3}}, {{4, -2, 6}}};
	AlignedBox3Type box10 = {{{1, 2, -4}}, {{4, 5, -2}}};
	AlignedBox3Type box11 = {{{8, 2, 3}}, {{10, 5, 6}}};
	AlignedBox3Type box12 = {{{1, 8, 3}}, {{4, 10, 6}}};
	AlignedBox3Type box13 = {{{1, 2, 8}}, {{4, 5, 10}}};

	AlignedBox3Type intersection;
	dsAlignedBox3_intersect(intersection, box, box);
	EXPECT_EQ((TypeParam)0, intersection.min.x);
	EXPECT_EQ((TypeParam)1, intersection.min.y);
	EXPECT_EQ((TypeParam)2, intersection.min.z);
	EXPECT_EQ((TypeParam)5, intersection.max.x);
	EXPECT_EQ((TypeParam)6, intersection.max.y);
	EXPECT_EQ((TypeParam)7, intersection.max.z);

	dsAlignedBox3_intersect(intersection, box, box1);
	EXPECT_EQ((TypeParam)1, intersection.min.x);
	EXPECT_EQ((TypeParam)2, intersection.min.y);
	EXPECT_EQ((TypeParam)3, intersection.min.z);
	EXPECT_EQ((TypeParam)4, intersection.max.x);
	EXPECT_EQ((TypeParam)5, intersection.max.y);
	EXPECT_EQ((TypeParam)6, intersection.max.z);

	dsAlignedBox3_intersect(intersection, box, box2);
	EXPECT_EQ((TypeParam)0, intersection.min.x);
	EXPECT_EQ((TypeParam)2, intersection.min.y);
	EXPECT_EQ((TypeParam)3, intersection.min.z);
	EXPECT_EQ((TypeParam)4, intersection.max.x);
	EXPECT_EQ((TypeParam)5, intersection.max.y);
	EXPECT_EQ((TypeParam)6, intersection.max.z);

	dsAlignedBox3_intersect(intersection, box, box3);
	EXPECT_EQ((TypeParam)1, intersection.min.x);
	EXPECT_EQ((TypeParam)1, intersection.min.y);
	EXPECT_EQ((TypeParam)3, intersection.min.z);
	EXPECT_EQ((TypeParam)4, intersection.max.x);
	EXPECT_EQ((TypeParam)5, intersection.max.y);
	EXPECT_EQ((TypeParam)6, intersection.max.z);

	dsAlignedBox3_intersect(intersection, box, box4);
	EXPECT_EQ((TypeParam)1, intersection.min.x);
	EXPECT_EQ((TypeParam)2, intersection.min.y);
	EXPECT_EQ((TypeParam)2, intersection.min.z);
	EXPECT_EQ((TypeParam)4, intersection.max.x);
	EXPECT_EQ((TypeParam)5, intersection.max.y);
	EXPECT_EQ((TypeParam)6, intersection.max.z);

	dsAlignedBox3_intersect(intersection, box, box5);
	EXPECT_EQ((TypeParam)1, intersection.min.x);
	EXPECT_EQ((TypeParam)2, intersection.min.y);
	EXPECT_EQ((TypeParam)3, intersection.min.z);
	EXPECT_EQ((TypeParam)5, intersection.max.x);
	EXPECT_EQ((TypeParam)5, intersection.max.y);
	EXPECT_EQ((TypeParam)6, intersection.max.z);

	dsAlignedBox3_intersect(intersection, box, box6);
	EXPECT_EQ((TypeParam)1, intersection.min.x);
	EXPECT_EQ((TypeParam)2, intersection.min.y);
	EXPECT_EQ((TypeParam)3, intersection.min.z);
	EXPECT_EQ((TypeParam)4, intersection.max.x);
	EXPECT_EQ((TypeParam)6, intersection.max.y);
	EXPECT_EQ((TypeParam)6, intersection.max.z);

	dsAlignedBox3_intersect(intersection, box, box7);
	EXPECT_EQ((TypeParam)1, intersection.min.x);
	EXPECT_EQ((TypeParam)2, intersection.min.y);
	EXPECT_EQ((TypeParam)3, intersection.min.z);
	EXPECT_EQ((TypeParam)4, intersection.max.x);
	EXPECT_EQ((TypeParam)5, intersection.max.y);
	EXPECT_EQ((TypeParam)7, intersection.max.z);

	dsAlignedBox3_intersect(intersection, box, box8);
	EXPECT_FALSE(dsAlignedBox3_isValid(intersection));

	dsAlignedBox3_intersect(intersection, box, box9);
	EXPECT_FALSE(dsAlignedBox3_isValid(intersection));

	dsAlignedBox3_intersect(intersection, box, box10);
	EXPECT_FALSE(dsAlignedBox3_isValid(intersection));

	dsAlignedBox3_intersect(intersection, box, box11);
	EXPECT_FALSE(dsAlignedBox3_isValid(intersection));

	dsAlignedBox3_intersect(intersection, box, box12);
	EXPECT_FALSE(dsAlignedBox3_isValid(intersection));

	dsAlignedBox3_intersect(intersection, box, box13);
	EXPECT_FALSE(dsAlignedBox3_isValid(intersection));
}

TYPED_TEST(AlignedBox3Test, Center)
{
	typedef typename AlignedBox3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;
	typedef typename AlignedBox3TypeSelector<TypeParam>::Vector3Type Vector3Type;

	AlignedBox3Type box = {{{0, 1, 2}}, {{4, 5, 6}}};

	Vector3Type center;
	dsAlignedBox3_center(center, box);
	EXPECT_EQ((TypeParam)2, center.x);
	EXPECT_EQ((TypeParam)3, center.y);
	EXPECT_EQ((TypeParam)4, center.z);
}

TYPED_TEST(AlignedBox3Test, Extents)
{
	typedef typename AlignedBox3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;
	typedef typename AlignedBox3TypeSelector<TypeParam>::Vector3Type Vector3Type;

	AlignedBox3Type box = {{{0, 2, 3}}, {{4, 7, 9}}};

	Vector3Type extents;
	dsAlignedBox3_extents(extents, box);
	EXPECT_EQ((TypeParam)4, extents.x);
	EXPECT_EQ((TypeParam)5, extents.y);
	EXPECT_EQ((TypeParam)6, extents.z);
}

TYPED_TEST(AlignedBox3Test, Corners)
{
	typedef typename AlignedBox3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;
	typedef typename AlignedBox3TypeSelector<TypeParam>::Vector3Type Vector3Type;

	AlignedBox3Type box = {{{0, 1, 2}}, {{3, 4, 5}}};

	Vector3Type corners[DS_BOX3_CORNER_COUNT];
	dsAlignedBox3_corners(corners, box);

	EXPECT_EQ((TypeParam)0, corners[0].x);
	EXPECT_EQ((TypeParam)1, corners[0].y);
	EXPECT_EQ((TypeParam)2, corners[0].z);

	EXPECT_EQ((TypeParam)0, corners[1].x);
	EXPECT_EQ((TypeParam)1, corners[1].y);
	EXPECT_EQ((TypeParam)5, corners[1].z);

	EXPECT_EQ((TypeParam)0, corners[2].x);
	EXPECT_EQ((TypeParam)4, corners[2].y);
	EXPECT_EQ((TypeParam)2, corners[2].z);

	EXPECT_EQ((TypeParam)0, corners[3].x);
	EXPECT_EQ((TypeParam)4, corners[3].y);
	EXPECT_EQ((TypeParam)5, corners[3].z);

	EXPECT_EQ((TypeParam)3, corners[4].x);
	EXPECT_EQ((TypeParam)1, corners[4].y);
	EXPECT_EQ((TypeParam)2, corners[4].z);

	EXPECT_EQ((TypeParam)3, corners[5].x);
	EXPECT_EQ((TypeParam)1, corners[5].y);
	EXPECT_EQ((TypeParam)5, corners[5].z);

	EXPECT_EQ((TypeParam)3, corners[6].x);
	EXPECT_EQ((TypeParam)4, corners[6].y);
	EXPECT_EQ((TypeParam)2, corners[6].z);

	EXPECT_EQ((TypeParam)3, corners[7].x);
	EXPECT_EQ((TypeParam)4, corners[7].y);
	EXPECT_EQ((TypeParam)5, corners[7].z);
}

TYPED_TEST(AlignedBox3Test, ClosestPoint)
{
	typedef typename AlignedBox3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;
	typedef typename AlignedBox3TypeSelector<TypeParam>::Vector3Type Vector3Type;

	AlignedBox3Type box = {{{0, 1, 2}}, {{3, 4, 5}}};

	Vector3Type point1 = {{1, 2, 3}};
	Vector3Type point2 = {{-1, 2, 3}};
	Vector3Type point3 = {{1, -2, 3}};
	Vector3Type point4 = {{1, 2, -3}};
	Vector3Type point5 = {{4, 2, 3}};
	Vector3Type point6 = {{1, 5, 3}};
	Vector3Type point7 = {{1, 2, 6}};

	Vector3Type closest;
	dsAlignedBox3_closestPoint(closest, box, box.min);
	EXPECT_EQ((TypeParam)0, closest.x);
	EXPECT_EQ((TypeParam)1, closest.y);
	EXPECT_EQ((TypeParam)2, closest.z);

	dsAlignedBox3_closestPoint(closest, box, box.max);
	EXPECT_EQ((TypeParam)3, closest.x);
	EXPECT_EQ((TypeParam)4, closest.y);
	EXPECT_EQ((TypeParam)5, closest.z);

	dsAlignedBox3_closestPoint(closest, box, point1);
	EXPECT_EQ((TypeParam)1, closest.x);
	EXPECT_EQ((TypeParam)2, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsAlignedBox3_closestPoint(closest, box, point2);
	EXPECT_EQ((TypeParam)0, closest.x);
	EXPECT_EQ((TypeParam)2, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsAlignedBox3_closestPoint(closest, box, point3);
	EXPECT_EQ((TypeParam)1, closest.x);
	EXPECT_EQ((TypeParam)1, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsAlignedBox3_closestPoint(closest, box, point4);
	EXPECT_EQ((TypeParam)1, closest.x);
	EXPECT_EQ((TypeParam)2, closest.y);
	EXPECT_EQ((TypeParam)2, closest.z);

	dsAlignedBox3_closestPoint(closest, box, point5);
	EXPECT_EQ((TypeParam)3, closest.x);
	EXPECT_EQ((TypeParam)2, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsAlignedBox3_closestPoint(closest, box, point6);
	EXPECT_EQ((TypeParam)1, closest.x);
	EXPECT_EQ((TypeParam)4, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsAlignedBox3_closestPoint(closest, box, point7);
	EXPECT_EQ((TypeParam)1, closest.x);
	EXPECT_EQ((TypeParam)2, closest.y);
	EXPECT_EQ((TypeParam)5, closest.z);
}

TYPED_TEST(AlignedBox3Test, MakeInvalid)
{
	typedef typename AlignedBox3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;

	AlignedBox3Type box = {{{0, 1, 2}}, {{3, 4, 5}}};

	dsAlignedBox3_makeInvalid(&box);
	EXPECT_FALSE(dsAlignedBox3_isValid(box));
}

TYPED_TEST(AlignedBox3Test, Dist2)
{
	typedef typename AlignedBox3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;
	typedef typename AlignedBox3TypeSelector<TypeParam>::Vector3Type Vector3Type;

	AlignedBox3Type box = {{{0, 1, 2}}, {{3, 4, 5}}};

	Vector3Type point1 = {{1, 2, 3}};
	Vector3Type point2 = {{-1, 2, 3}};
	Vector3Type point3 = {{1, -2, 3}};
	Vector3Type point4 = {{1, 2, -3}};
	Vector3Type point5 = {{4, 2, 3}};
	Vector3Type point6 = {{1, 6, 3}};
	Vector3Type point7 = {{1, 2, 8}};

	EXPECT_EQ((TypeParam)0, dsAlignedBox3_dist2(&box, &box.min));
	EXPECT_EQ((TypeParam)0, dsAlignedBox3_dist2(&box, &box.max));
	EXPECT_EQ((TypeParam)0, dsAlignedBox3_dist2(&box, &point1));
	EXPECT_EQ((TypeParam)1, dsAlignedBox3_dist2(&box, &point2));
	EXPECT_EQ((TypeParam)9, dsAlignedBox3_dist2(&box, &point3));
	EXPECT_EQ((TypeParam)25, dsAlignedBox3_dist2(&box, &point4));
	EXPECT_EQ((TypeParam)1, dsAlignedBox3_dist2(&box, &point5));
	EXPECT_EQ((TypeParam)4, dsAlignedBox3_dist2(&box, &point6));
	EXPECT_EQ((TypeParam)9, dsAlignedBox3_dist2(&box, &point7));
}

TYPED_TEST(AlignedBox3Test, Dist)
{
	typedef typename AlignedBox3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;
	typedef typename AlignedBox3TypeSelector<TypeParam>::Vector3Type Vector3Type;

	AlignedBox3Type box = {{{0, 1, 2}}, {{3, 4, 5}}};

	Vector3Type point1 = {{1, 2, 3}};
	Vector3Type point2 = {{-1, 2, 3}};
	Vector3Type point3 = {{1, -2, 3}};
	Vector3Type point4 = {{1, 2, -3}};
	Vector3Type point5 = {{4, 2, 3}};
	Vector3Type point6 = {{1, 6, 3}};
	Vector3Type point7 = {{1, 2, 8}};

	EXPECT_FLOAT_EQ(0.0f, (float)dsAlignedBox3_dist(&box, &box.min));
	EXPECT_FLOAT_EQ(0.0f, (float)dsAlignedBox3_dist(&box, &box.max));
	EXPECT_FLOAT_EQ(0.0f, (float)dsAlignedBox3_dist(&box, &point1));
	EXPECT_FLOAT_EQ(1.0f, (float)dsAlignedBox3_dist(&box, &point2));
	EXPECT_FLOAT_EQ(3.0f, (float)dsAlignedBox3_dist(&box, &point3));
	EXPECT_FLOAT_EQ(5.0f, (float)dsAlignedBox3_dist(&box, &point4));
	EXPECT_FLOAT_EQ(1.0f, (float)dsAlignedBox3_dist(&box, &point5));
	EXPECT_FLOAT_EQ(2.0f, (float)dsAlignedBox3_dist(&box, &point6));
	EXPECT_FLOAT_EQ(3.0f, (float)dsAlignedBox3_dist(&box, &point7));
}

TEST(AlignedBox3, ConvertFloatToDouble)
{
	dsAlignedBox3f boxf = {{{0, 1, 2}}, {{3, 4, 5}}};

	dsAlignedBox3d boxd;
	dsConvertFloatToDouble(boxd, boxf);

	EXPECT_FLOAT_EQ(boxf.min.x, (float)boxd.min.x);
	EXPECT_FLOAT_EQ(boxf.min.y, (float)boxd.min.y);
	EXPECT_FLOAT_EQ(boxf.min.z, (float)boxd.min.z);

	EXPECT_FLOAT_EQ(boxf.max.x, (float)boxd.max.x);
	EXPECT_FLOAT_EQ(boxf.max.y, (float)boxd.max.y);
	EXPECT_FLOAT_EQ(boxf.max.z, (float)boxd.max.z);
}

TEST(AlignedBox3, ConvertDoubleToFloat)
{
	dsAlignedBox3d boxd = {{{0, 1, 2}}, {{3, 4, 5}}};

	dsAlignedBox3f boxf;
	dsConvertDoubleToFloat(boxf, boxd);

	EXPECT_FLOAT_EQ((float)boxd.min.x, boxf.min.x);
	EXPECT_FLOAT_EQ((float)boxd.min.y, boxf.min.y);
	EXPECT_FLOAT_EQ((float)boxd.min.z, boxf.min.z);

	EXPECT_FLOAT_EQ((float)boxd.max.x, boxf.max.x);
	EXPECT_FLOAT_EQ((float)boxd.max.y, boxf.max.y);
	EXPECT_FLOAT_EQ((float)boxd.max.z, boxf.max.z);
}

TEST(AlignedBox3, ConvertFloatToInt)
{
	dsAlignedBox3f boxf = {{{0, 1, 3}}, {{4, 5, 6}}};

	dsAlignedBox3i boxi;
	dsConvertFloatToInt(boxi, boxf);

	EXPECT_EQ(boxf.min.x, (float)boxi.min.x);
	EXPECT_EQ(boxf.min.y, (float)boxi.min.y);
	EXPECT_EQ(boxf.min.z, (float)boxi.min.z);

	EXPECT_EQ(boxf.max.x, (float)boxi.max.x);
	EXPECT_EQ(boxf.max.y, (float)boxi.max.y);
	EXPECT_EQ(boxf.max.z, (float)boxi.max.z);
}

TEST(AlignedBox3, ConvertIntToFloat)
{
	dsAlignedBox3i boxi = {{{0, 1, 3}}, {{4, 5, 6}}};

	dsAlignedBox3f boxf;
	dsConvertIntToFloat(boxf, boxi);

	EXPECT_EQ(boxi.min.x, (int)boxf.min.x);
	EXPECT_EQ(boxi.min.y, (int)boxf.min.y);
	EXPECT_EQ(boxi.min.z, (int)boxf.min.z);

	EXPECT_EQ(boxi.max.x, (int)boxf.max.x);
	EXPECT_EQ(boxi.max.y, (int)boxf.max.y);
	EXPECT_EQ(boxi.max.z, (int)boxf.max.z);
}

TEST(AlignedBox3, ConvertDoubleToInt)
{
	dsAlignedBox3d boxd = {{{0, 1, 3}}, {{4, 5, 6}}};

	dsAlignedBox3i boxi;
	dsConvertDoubleToInt(boxi, boxd);

	EXPECT_EQ(boxd.min.x, boxi.min.x);
	EXPECT_EQ(boxd.min.y, boxi.min.y);
	EXPECT_EQ(boxd.min.z, boxi.min.z);

	EXPECT_EQ(boxd.max.x, boxi.max.x);
	EXPECT_EQ(boxd.max.y, boxi.max.y);
	EXPECT_EQ(boxd.max.z, boxi.max.z);
}

TEST(AlignedBox3, ConvertIntToDouble)
{
	dsAlignedBox3i boxi = {{{0, 1, 3}}, {{4, 5, 6}}};

	dsAlignedBox3d boxd;
	dsConvertIntToDouble(boxd, boxi);

	EXPECT_EQ(boxi.min.x, (int)boxd.min.x);
	EXPECT_EQ(boxi.min.y, (int)boxd.min.y);
	EXPECT_EQ(boxi.min.z, (int)boxd.min.z);

	EXPECT_EQ(boxi.max.x, (int)boxd.max.x);
	EXPECT_EQ(boxi.max.y, (int)boxd.max.y);
	EXPECT_EQ(boxi.max.z, (int)boxd.max.z);
}

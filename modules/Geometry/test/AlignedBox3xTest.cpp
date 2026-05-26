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

#include <DeepSea/Geometry/AlignedBox3x.h>
#include <DeepSea/Math/Matrix44.h>
#include <gtest/gtest.h>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

template <typename T>
struct AlignedBox3xTypeSelector;

template <>
struct AlignedBox3xTypeSelector<float>
{
	typedef dsVector3xf Vector3xType;
	typedef dsAlignedBox3xf AlignedBox3xType;
	typedef dsMatrix44f Matrix44Type;
	typedef dsVector4f Vector4Type;
	static const float epsilon;
};

template <>
struct AlignedBox3xTypeSelector<double>
{
	typedef dsVector3xd Vector3xType;
	typedef dsAlignedBox3xd AlignedBox3xType;
	typedef dsMatrix44d Matrix44Type;
	typedef dsVector4d Vector4Type;
	static const double epsilon;
};

const float AlignedBox3xTypeSelector<float>::epsilon = 1e-4f;
const double AlignedBox3xTypeSelector<double>::epsilon = 1e-13f;

template <typename T>
class AlignedBox3xTest : public testing::Test
{
};

using AlignedBox3xTypes = testing::Types<float, double>;
TYPED_TEST_SUITE(AlignedBox3xTest, AlignedBox3xTypes);

inline bool dsAlignedBox3x_isValid(const dsAlignedBox3xf* box)
{
	return dsAlignedBox3xf_isValid(box);
}

inline bool dsAlignedBox3x_isValid(const dsAlignedBox3xd* box)
{
	return dsAlignedBox3xd_isValid(box);
}

inline void dsAlignedBox3x_addPoint(dsAlignedBox3xf* box, const dsVector3xf* point)
{
	dsAlignedBox3xf_addPoint(box, point);
}

inline void dsAlignedBox3x_addPoint(dsAlignedBox3xd* box, const dsVector3xd* point)
{
	dsAlignedBox3xd_addPoint(box, point);
}

inline void dsAlignedBox3x_addBox(dsAlignedBox3xf* box, const dsAlignedBox3xf* otherBox)
{
	dsAlignedBox3xf_addBox(box, otherBox);
}

inline void dsAlignedBox3x_addBox(dsAlignedBox3xd* box, const dsAlignedBox3xd* otherBox)
{
	dsAlignedBox3xd_addBox(box, otherBox);
}

inline bool dsAlignedBox3x_containsPoint(const dsAlignedBox3xf* box, const dsVector3xf* point)
{
	return dsAlignedBox3xf_containsPoint(box, point);
}

inline bool dsAlignedBox3x_containsPoint(const dsAlignedBox3xd* box, const dsVector3xd* point)
{
	return dsAlignedBox3xd_containsPoint(box, point);
}

inline bool dsAlignedBox3x_containsBox(const dsAlignedBox3xf* box, const dsAlignedBox3xf* otherBox)
{
	return dsAlignedBox3xf_containsBox(box, otherBox);
}

inline bool dsAlignedBox3x_containsBox(const dsAlignedBox3xd* box, const dsAlignedBox3xd* otherBox)
{
	return dsAlignedBox3xd_containsBox(box, otherBox);
}

inline bool dsAlignedBox3x_intersects(const dsAlignedBox3xf* box, const dsAlignedBox3xf* otherBox)
{
	return dsAlignedBox3xf_intersects(box, otherBox);
}

inline bool dsAlignedBox3x_intersects(const dsAlignedBox3xd* box, const dsAlignedBox3xd* otherBox)
{
	return dsAlignedBox3xd_intersects(box, otherBox);
}

inline void dsAlignedBox3x_intersect(
	dsAlignedBox3xf* result, const dsAlignedBox3xf* a, const dsAlignedBox3xf* b)
{
	dsAlignedBox3xf_intersect(result, a, b);
}

inline void dsAlignedBox3x_intersect(
	dsAlignedBox3xd* result, const dsAlignedBox3xd* a, const dsAlignedBox3xd* b)
{
	dsAlignedBox3xd_intersect(result, a, b);
}

inline void dsAlignedBox3x_center(dsVector3xf* result, const dsAlignedBox3xf* box)
{
	dsAlignedBox3xf_center(result, box);
}

inline void dsAlignedBox3x_center(dsVector3xd* result, const dsAlignedBox3xd* box)
{
	dsAlignedBox3xd_center(result, box);
}

inline void dsAlignedBox3x_extents(dsVector3xf* result, const dsAlignedBox3xf* box)
{
	dsAlignedBox3xf_extents(result, box);
}

inline void dsAlignedBox3x_extents(dsVector3xd* result, const dsAlignedBox3xd* box)
{
	dsAlignedBox3xd_extents(result, box);
}

inline void dsAlignedBox3x_toMatrix(dsMatrix44f* result, const dsAlignedBox3xf* box)
{
	dsAlignedBox3xf_toMatrix(result, box);
}

inline void dsAlignedBox3x_toMatrix(dsMatrix44d* result, const dsAlignedBox3xd* box)
{
	dsAlignedBox3xd_toMatrix(result, box);
}

inline void dsAlignedBox3x_toMatrixTranspose(dsMatrix44f* result, const dsAlignedBox3xf* box)
{
	dsAlignedBox3xf_toMatrixTranspose(result, box);
}

inline void dsAlignedBox3x_toMatrixTranspose(dsMatrix44d* result, const dsAlignedBox3xd* box)
{
	dsAlignedBox3xd_toMatrixTranspose(result, box);
}

inline void dsAlignedBox3x_corners(
	dsVector3xf corners[DS_BOX3_CORNER_COUNT], const dsAlignedBox3xf* box)
{
	dsAlignedBox3xf_corners(corners, box);
}

inline void dsAlignedBox3x_corners(
	dsVector3xd corners[DS_BOX3_CORNER_COUNT], const dsAlignedBox3xd* box)
{
	dsAlignedBox3xd_corners(corners, box);
}

inline void dsAlignedBox3x_closestPoint(
	dsVector3xf* result, dsAlignedBox3xf* box, dsVector3xf* point)
{
	dsAlignedBox3xf_closestPoint(result, box, point);
}

inline void dsAlignedBox3x_closestPoint(
	dsVector3xd* result, dsAlignedBox3xd* box, dsVector3xd* point)
{
	dsAlignedBox3xd_closestPoint(result, box, point);
}

inline void dsAlignedBox3x_makeInvalid(dsAlignedBox3xf* result)
{
	dsAlignedBox3xf_makeInvalid(result);
}

inline void dsAlignedBox3x_makeInvalid(dsAlignedBox3xd* result)
{
	dsAlignedBox3xd_makeInvalid(result);
}

inline float dsAlignedBox3x_dist2(const dsAlignedBox3xf* box, const dsVector3xf* point)
{
	return dsAlignedBox3xf_dist2(box, point);
}

inline double dsAlignedBox3x_dist2(const dsAlignedBox3xd* box, const dsVector3xd* point)
{
	return dsAlignedBox3xd_dist2(box, point);
}

inline float dsAlignedBox3x_dist(const dsAlignedBox3xf* box, const dsVector3xf* point)
{
	return dsAlignedBox3xf_dist(box, point);
}

inline double dsAlignedBox3x_dist(const dsAlignedBox3xd* box, const dsVector3xd* point)
{
	return dsAlignedBox3xd_dist(box, point);
}

TYPED_TEST(AlignedBox3xTest, Initialize)
{
	typedef typename AlignedBox3xTypeSelector<TypeParam>::AlignedBox3xType AlignedBox3xType;

	AlignedBox3xType box = {{{0, 1, 2}}, {{3, 4, 5}}};
	EXPECT_EQ((TypeParam)0, box.min.x);
	EXPECT_EQ((TypeParam)1, box.min.y);
	EXPECT_EQ((TypeParam)2, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);
}

TYPED_TEST(AlignedBox3xTest, IsValid)
{
	typedef typename AlignedBox3xTypeSelector<TypeParam>::AlignedBox3xType AlignedBox3xType;

	AlignedBox3xType box = {{{0, 0, 0, 3}}, {{1, 1, 1, -2}}};
	EXPECT_TRUE(dsAlignedBox3x_isValid(&box));

	box.min.x = 2;
	EXPECT_FALSE(dsAlignedBox3x_isValid(&box));

	box.min.x = 0;
	box.min.y = 2;
	EXPECT_FALSE(dsAlignedBox3x_isValid(&box));

	box.min.y = 0;
	box.min.z = 2;
	EXPECT_FALSE(dsAlignedBox3x_isValid(&box));
}

TYPED_TEST(AlignedBox3xTest, AddPoint)
{
	typedef typename AlignedBox3xTypeSelector<TypeParam>::AlignedBox3xType AlignedBox3xType;
	typedef typename AlignedBox3xTypeSelector<TypeParam>::Vector3xType Vector3xType;

	AlignedBox3xType box = {{{0, 1, 2, 7}}, {{3, 4, 5, -6}}};

	Vector3xType point1 = {{0, 4, 2, 1}};
	Vector3xType point2 = {{3, 1, 5, 2}};
	Vector3xType point3 = {{-1, 1, 2, 3}};
	Vector3xType point4 = {{0, -2, 2, 4}};
	Vector3xType point5 = {{0, 1, -3, 5}};
	Vector3xType point6 = {{4, 1, 2, 6}};
	Vector3xType point7 = {{0, 5, 2, 7}};
	Vector3xType point8 = {{0, 1, 6, 8}};

	dsAlignedBox3x_addPoint(&box, &point1);
	EXPECT_EQ((TypeParam)0, box.min.x);
	EXPECT_EQ((TypeParam)1, box.min.y);
	EXPECT_EQ((TypeParam)2, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3x_addPoint(&box, &point2);
	EXPECT_EQ((TypeParam)0, box.min.x);
	EXPECT_EQ((TypeParam)1, box.min.y);
	EXPECT_EQ((TypeParam)2, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3x_addPoint(&box, &point3);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)1, box.min.y);
	EXPECT_EQ((TypeParam)2, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3x_addPoint(&box, &point4);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)2, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3x_addPoint(&box, &point5);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)-3, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3x_addPoint(&box, &point6);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)-3, box.min.z);
	EXPECT_EQ((TypeParam)4, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3x_addPoint(&box, &point7);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)-3, box.min.z);
	EXPECT_EQ((TypeParam)4, box.max.x);
	EXPECT_EQ((TypeParam)5, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3x_addPoint(&box, &point8);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)-3, box.min.z);
	EXPECT_EQ((TypeParam)4, box.max.x);
	EXPECT_EQ((TypeParam)5, box.max.y);
	EXPECT_EQ((TypeParam)6, box.max.z);
}

TYPED_TEST(AlignedBox3xTest, AddBox)
{
	typedef typename AlignedBox3xTypeSelector<TypeParam>::AlignedBox3xType AlignedBox3xType;

	AlignedBox3xType box = {{{0, 1, 2, 7}}, {{3, 4, 5, -6}}};

	AlignedBox3xType box1 = {{{2, 2, 2, 8}}, {{3, 3, 3, -5}}};
	AlignedBox3xType box2 = {{{-1, 1, 3, 9}}, {{3, 3, 3, -4}}};
	AlignedBox3xType box3 = {{{1, -2, 3, 10}}, {{3, 3, 3, -3}}};
	AlignedBox3xType box4 = {{{1, 2, -3, 11}}, {{3, 3, 3, -2}}};
	AlignedBox3xType box5 = {{{1, 2, 3, 12}}, {{4, 3, 3, -1}}};
	AlignedBox3xType box6 = {{{1, 2, 3, 13}}, {{3, 5, 3, 0}}};
	AlignedBox3xType box7 = {{{1, 2, 3, 14}}, {{3, 3, 6, 1}}};

	dsAlignedBox3x_addBox(&box, &box1);
	EXPECT_EQ((TypeParam)0, box.min.x);
	EXPECT_EQ((TypeParam)1, box.min.y);
	EXPECT_EQ((TypeParam)2, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3x_addBox(&box, &box2);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)1, box.min.y);
	EXPECT_EQ((TypeParam)2, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3x_addBox(&box, &box3);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)2, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3x_addBox(&box, &box4);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)-3, box.min.z);
	EXPECT_EQ((TypeParam)3, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3x_addBox(&box, &box5);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)-3, box.min.z);
	EXPECT_EQ((TypeParam)4, box.max.x);
	EXPECT_EQ((TypeParam)4, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3x_addBox(&box, &box6);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)-3, box.min.z);
	EXPECT_EQ((TypeParam)4, box.max.x);
	EXPECT_EQ((TypeParam)5, box.max.y);
	EXPECT_EQ((TypeParam)5, box.max.z);

	dsAlignedBox3x_addBox(&box, &box7);
	EXPECT_EQ((TypeParam)-1, box.min.x);
	EXPECT_EQ((TypeParam)-2, box.min.y);
	EXPECT_EQ((TypeParam)-3, box.min.z);
	EXPECT_EQ((TypeParam)4, box.max.x);
	EXPECT_EQ((TypeParam)5, box.max.y);
	EXPECT_EQ((TypeParam)6, box.max.z);
}

TYPED_TEST(AlignedBox3xTest, ContainsPoint)
{
	typedef typename AlignedBox3xTypeSelector<TypeParam>::AlignedBox3xType AlignedBox3xType;
	typedef typename AlignedBox3xTypeSelector<TypeParam>::Vector3xType Vector3xType;

	AlignedBox3xType box = {{{0, 1, 2, 6}}, {{3, 4, 5, -7}}};

	Vector3xType point1 = {{1, 2, 3, 1}};
	Vector3xType point2 = {{-1, 2, 3, 2}};
	Vector3xType point3 = {{1, -2, 3, 3}};
	Vector3xType point4 = {{4, 2, 3, 4}};
	Vector3xType point5 = {{1, 5, 3, 5}};
	Vector3xType point6 = {{1, 2, 6, 6}};

	EXPECT_TRUE(dsAlignedBox3x_containsPoint(&box, &box.min));
	EXPECT_TRUE(dsAlignedBox3x_containsPoint(&box, &box.max));
	EXPECT_TRUE(dsAlignedBox3x_containsPoint(&box, &point1));
	EXPECT_FALSE(dsAlignedBox3x_containsPoint(&box, &point2));
	EXPECT_FALSE(dsAlignedBox3x_containsPoint(&box, &point3));
	EXPECT_FALSE(dsAlignedBox3x_containsPoint(&box, &point4));
	EXPECT_FALSE(dsAlignedBox3x_containsPoint(&box, &point5));
	EXPECT_FALSE(dsAlignedBox3x_containsPoint(&box, &point6));
}

TYPED_TEST(AlignedBox3xTest, ContainsBox)
{
	typedef typename AlignedBox3xTypeSelector<TypeParam>::AlignedBox3xType AlignedBox3xType;

	AlignedBox3xType box = {{{0, 1, 2, 8}}, {{5, 6, 7, -9}}};

	AlignedBox3xType box1 = {{{1, 2, 3, 9}}, {{4, 5, 6, -8}}};
	AlignedBox3xType box2 = {{{-1, 2, 3, 10}}, {{4, 5, 6, -7}}};
	AlignedBox3xType box3 = {{{1, -2, 3, 11}}, {{4, 5, 6, -6}}};
	AlignedBox3xType box4 = {{{1, 2, -3, 12}}, {{4, 5, 6, -5}}};
	AlignedBox3xType box5 = {{{1, 2, 3, 13}}, {{7, 5, 6, -4}}};
	AlignedBox3xType box6 = {{{1, 2, 3, 14}}, {{4, 8, 6, -3}}};
	AlignedBox3xType box7 = {{{1, 2, 3, 15}}, {{4, 5, 9, -2}}};
	AlignedBox3xType box8 = {{{-4, 2, 3, 16}}, {{-2, 5, 6, -1}}};
	AlignedBox3xType box9 = {{{1, -4, 3, 17}}, {{4, -2, 6, 0}}};
	AlignedBox3xType box10 = {{{1, 2, -4, 18}}, {{4, 5, -2, 1}}};
	AlignedBox3xType box11 = {{{8, 2, 3, 19}}, {{10, 5, 6, 2}}};
	AlignedBox3xType box12 = {{{1, 8, 3, 20}}, {{4, 10, 6, 3}}};
	AlignedBox3xType box13 = {{{1, 2, 8, 21}}, {{4, 5, 10, 4}}};

	EXPECT_TRUE(dsAlignedBox3x_containsBox(&box, &box));
	EXPECT_TRUE(dsAlignedBox3x_containsBox(&box, &box1));
	EXPECT_FALSE(dsAlignedBox3x_containsBox(&box, &box2));
	EXPECT_FALSE(dsAlignedBox3x_containsBox(&box, &box3));
	EXPECT_FALSE(dsAlignedBox3x_containsBox(&box, &box4));
	EXPECT_FALSE(dsAlignedBox3x_containsBox(&box, &box5));
	EXPECT_FALSE(dsAlignedBox3x_containsBox(&box, &box6));
	EXPECT_FALSE(dsAlignedBox3x_containsBox(&box, &box7));
	EXPECT_FALSE(dsAlignedBox3x_containsBox(&box, &box8));
	EXPECT_FALSE(dsAlignedBox3x_containsBox(&box, &box9));
	EXPECT_FALSE(dsAlignedBox3x_containsBox(&box, &box10));
	EXPECT_FALSE(dsAlignedBox3x_containsBox(&box, &box11));
	EXPECT_FALSE(dsAlignedBox3x_containsBox(&box, &box12));
	EXPECT_FALSE(dsAlignedBox3x_containsBox(&box, &box13));
}

TYPED_TEST(AlignedBox3xTest, Intersects)
{
	typedef typename AlignedBox3xTypeSelector<TypeParam>::AlignedBox3xType AlignedBox3xType;

	AlignedBox3xType box = {{{0, 1, 2, 8}}, {{5, 6, 7, -9}}};

	AlignedBox3xType box1 = {{{1, 2, 3, 9}}, {{4, 5, 6, -8}}};
	AlignedBox3xType box2 = {{{-1, 2, 3, 10}}, {{4, 5, 6, -7}}};
	AlignedBox3xType box3 = {{{1, -2, 3, 11}}, {{4, 5, 6, -6}}};
	AlignedBox3xType box4 = {{{1, 2, -3, 12}}, {{4, 5, 6, -5}}};
	AlignedBox3xType box5 = {{{1, 2, 3, 13}}, {{7, 5, 6, -4}}};
	AlignedBox3xType box6 = {{{1, 2, 3, 14}}, {{4, 8, 6, -3}}};
	AlignedBox3xType box7 = {{{1, 2, 3, 15}}, {{4, 5, 9, -2}}};
	AlignedBox3xType box8 = {{{-4, 2, 3, 16}}, {{-2, 5, 6, -1}}};
	AlignedBox3xType box9 = {{{1, -4, 3, 17}}, {{4, -2, 6, 0}}};
	AlignedBox3xType box10 = {{{1, 2, -4, 18}}, {{4, 5, -2, 1}}};
	AlignedBox3xType box11 = {{{8, 2, 3, 19}}, {{10, 5, 6, 2}}};
	AlignedBox3xType box12 = {{{1, 8, 3, 20}}, {{4, 10, 6, 3}}};
	AlignedBox3xType box13 = {{{1, 2, 8, 21}}, {{4, 5, 10, 4}}};

	EXPECT_TRUE(dsAlignedBox3x_intersects(&box, &box));
	EXPECT_TRUE(dsAlignedBox3x_intersects(&box, &box1));
	EXPECT_TRUE(dsAlignedBox3x_intersects(&box, &box2));
	EXPECT_TRUE(dsAlignedBox3x_intersects(&box, &box3));
	EXPECT_TRUE(dsAlignedBox3x_intersects(&box, &box4));
	EXPECT_TRUE(dsAlignedBox3x_intersects(&box, &box5));
	EXPECT_TRUE(dsAlignedBox3x_intersects(&box, &box6));
	EXPECT_TRUE(dsAlignedBox3x_intersects(&box, &box7));
	EXPECT_FALSE(dsAlignedBox3x_intersects(&box, &box8));
	EXPECT_FALSE(dsAlignedBox3x_intersects(&box, &box9));
	EXPECT_FALSE(dsAlignedBox3x_intersects(&box, &box10));
	EXPECT_FALSE(dsAlignedBox3x_intersects(&box, &box11));
	EXPECT_FALSE(dsAlignedBox3x_intersects(&box, &box12));
	EXPECT_FALSE(dsAlignedBox3x_intersects(&box, &box13));
}

TYPED_TEST(AlignedBox3xTest, Intersect)
{
	typedef typename AlignedBox3xTypeSelector<TypeParam>::AlignedBox3xType AlignedBox3xType;

	AlignedBox3xType box = {{{0, 1, 2, 8}}, {{5, 6, 7, -9}}};

	AlignedBox3xType box1 = {{{1, 2, 3, 9}}, {{4, 5, 6, -8}}};
	AlignedBox3xType box2 = {{{-1, 2, 3, 10}}, {{4, 5, 6, -7}}};
	AlignedBox3xType box3 = {{{1, -2, 3, 11}}, {{4, 5, 6, -6}}};
	AlignedBox3xType box4 = {{{1, 2, -3, 12}}, {{4, 5, 6, -5}}};
	AlignedBox3xType box5 = {{{1, 2, 3, 13}}, {{7, 5, 6, -4}}};
	AlignedBox3xType box6 = {{{1, 2, 3, 14}}, {{4, 8, 6, -3}}};
	AlignedBox3xType box7 = {{{1, 2, 3, 15}}, {{4, 5, 9, -2}}};
	AlignedBox3xType box8 = {{{-4, 2, 3, 16}}, {{-2, 5, 6, -1}}};
	AlignedBox3xType box9 = {{{1, -4, 3, 17}}, {{4, -2, 6, 0}}};
	AlignedBox3xType box10 = {{{1, 2, -4, 18}}, {{4, 5, -2, 1}}};
	AlignedBox3xType box11 = {{{8, 2, 3, 19}}, {{10, 5, 6, 2}}};
	AlignedBox3xType box12 = {{{1, 8, 3, 20}}, {{4, 10, 6, 3}}};
	AlignedBox3xType box13 = {{{1, 2, 8, 21}}, {{4, 5, 10, 4}}};

	AlignedBox3xType intersection;
	dsAlignedBox3x_intersect(&intersection, &box, &box);
	EXPECT_EQ((TypeParam)0, intersection.min.x);
	EXPECT_EQ((TypeParam)1, intersection.min.y);
	EXPECT_EQ((TypeParam)2, intersection.min.z);
	EXPECT_EQ((TypeParam)5, intersection.max.x);
	EXPECT_EQ((TypeParam)6, intersection.max.y);
	EXPECT_EQ((TypeParam)7, intersection.max.z);

	dsAlignedBox3x_intersect(&intersection, &box, &box1);
	EXPECT_EQ((TypeParam)1, intersection.min.x);
	EXPECT_EQ((TypeParam)2, intersection.min.y);
	EXPECT_EQ((TypeParam)3, intersection.min.z);
	EXPECT_EQ((TypeParam)4, intersection.max.x);
	EXPECT_EQ((TypeParam)5, intersection.max.y);
	EXPECT_EQ((TypeParam)6, intersection.max.z);

	dsAlignedBox3x_intersect(&intersection, &box, &box2);
	EXPECT_EQ((TypeParam)0, intersection.min.x);
	EXPECT_EQ((TypeParam)2, intersection.min.y);
	EXPECT_EQ((TypeParam)3, intersection.min.z);
	EXPECT_EQ((TypeParam)4, intersection.max.x);
	EXPECT_EQ((TypeParam)5, intersection.max.y);
	EXPECT_EQ((TypeParam)6, intersection.max.z);

	dsAlignedBox3x_intersect(&intersection, &box, &box3);
	EXPECT_EQ((TypeParam)1, intersection.min.x);
	EXPECT_EQ((TypeParam)1, intersection.min.y);
	EXPECT_EQ((TypeParam)3, intersection.min.z);
	EXPECT_EQ((TypeParam)4, intersection.max.x);
	EXPECT_EQ((TypeParam)5, intersection.max.y);
	EXPECT_EQ((TypeParam)6, intersection.max.z);

	dsAlignedBox3x_intersect(&intersection, &box, &box4);
	EXPECT_EQ((TypeParam)1, intersection.min.x);
	EXPECT_EQ((TypeParam)2, intersection.min.y);
	EXPECT_EQ((TypeParam)2, intersection.min.z);
	EXPECT_EQ((TypeParam)4, intersection.max.x);
	EXPECT_EQ((TypeParam)5, intersection.max.y);
	EXPECT_EQ((TypeParam)6, intersection.max.z);

	dsAlignedBox3x_intersect(&intersection, &box, &box5);
	EXPECT_EQ((TypeParam)1, intersection.min.x);
	EXPECT_EQ((TypeParam)2, intersection.min.y);
	EXPECT_EQ((TypeParam)3, intersection.min.z);
	EXPECT_EQ((TypeParam)5, intersection.max.x);
	EXPECT_EQ((TypeParam)5, intersection.max.y);
	EXPECT_EQ((TypeParam)6, intersection.max.z);

	dsAlignedBox3x_intersect(&intersection, &box, &box6);
	EXPECT_EQ((TypeParam)1, intersection.min.x);
	EXPECT_EQ((TypeParam)2, intersection.min.y);
	EXPECT_EQ((TypeParam)3, intersection.min.z);
	EXPECT_EQ((TypeParam)4, intersection.max.x);
	EXPECT_EQ((TypeParam)6, intersection.max.y);
	EXPECT_EQ((TypeParam)6, intersection.max.z);

	dsAlignedBox3x_intersect(&intersection, &box, &box7);
	EXPECT_EQ((TypeParam)1, intersection.min.x);
	EXPECT_EQ((TypeParam)2, intersection.min.y);
	EXPECT_EQ((TypeParam)3, intersection.min.z);
	EXPECT_EQ((TypeParam)4, intersection.max.x);
	EXPECT_EQ((TypeParam)5, intersection.max.y);
	EXPECT_EQ((TypeParam)7, intersection.max.z);

	dsAlignedBox3x_intersect(&intersection, &box, &box8);
	EXPECT_FALSE(dsAlignedBox3x_isValid(&intersection));

	dsAlignedBox3x_intersect(&intersection, &box, &box9);
	EXPECT_FALSE(dsAlignedBox3x_isValid(&intersection));

	dsAlignedBox3x_intersect(&intersection, &box, &box10);
	EXPECT_FALSE(dsAlignedBox3x_isValid(&intersection));

	dsAlignedBox3x_intersect(&intersection, &box, &box11);
	EXPECT_FALSE(dsAlignedBox3_isValid(intersection));

	dsAlignedBox3x_intersect(&intersection, &box, &box12);
	EXPECT_FALSE(dsAlignedBox3_isValid(intersection));

	dsAlignedBox3x_intersect(&intersection, &box, &box13);
	EXPECT_FALSE(dsAlignedBox3x_isValid(&intersection));
}

TYPED_TEST(AlignedBox3xTest, Center)
{
	typedef typename AlignedBox3xTypeSelector<TypeParam>::AlignedBox3xType AlignedBox3xType;
	typedef typename AlignedBox3xTypeSelector<TypeParam>::Vector3xType Vector3xType;

	AlignedBox3xType box = {{{0, 1, 2, 7}}, {{4, 5, 6, -8}}};

	Vector3xType center;
	dsAlignedBox3x_center(&center, &box);
	EXPECT_EQ((TypeParam)2, center.x);
	EXPECT_EQ((TypeParam)3, center.y);
	EXPECT_EQ((TypeParam)4, center.z);
}

TYPED_TEST(AlignedBox3xTest, Extents)
{
	typedef typename AlignedBox3xTypeSelector<TypeParam>::AlignedBox3xType AlignedBox3xType;
	typedef typename AlignedBox3xTypeSelector<TypeParam>::Vector3xType Vector3xType;

	AlignedBox3xType box = {{{0, 2, 3, 10}}, {{4, 7, 9, -11}}};

	Vector3xType extents;
	dsAlignedBox3x_extents(&extents, &box);
	EXPECT_EQ((TypeParam)4, extents.x);
	EXPECT_EQ((TypeParam)5, extents.y);
	EXPECT_EQ((TypeParam)6, extents.z);
}

TYPED_TEST(AlignedBox3xTest, ToMatrix)
{
	typedef typename AlignedBox3xTypeSelector<TypeParam>::AlignedBox3xType AlignedBox3xType;
	typedef typename AlignedBox3xTypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename AlignedBox3xTypeSelector<TypeParam>::Vector4Type Vector4Type;
	TypeParam epsilon = AlignedBox3xTypeSelector<TypeParam>::epsilon;

	AlignedBox3xType box = {{{0, 1, 2, 3}}, {{3, 5, 7, -9}}};

	Matrix44Type matrix;
	dsAlignedBox3x_toMatrix(&matrix, &box);

	Vector4Type lowerLeft = {{-1, -1, -1, 1}};
	Vector4Type boxPoint;
	dsMatrix44_transform(boxPoint, matrix, lowerLeft);
	EXPECT_NEAR(box.min.x, boxPoint.x, epsilon);
	EXPECT_NEAR(box.min.y, boxPoint.y, epsilon);
	EXPECT_NEAR(box.min.z, boxPoint.z, epsilon);

	Vector4Type upperRight = {{1, 1, 1, 1}};
	dsMatrix44_transform(boxPoint, matrix, upperRight);
	EXPECT_NEAR(box.max.x, boxPoint.x, epsilon);
	EXPECT_NEAR(box.max.y, boxPoint.y, epsilon);
	EXPECT_NEAR(box.max.z, boxPoint.z, epsilon);
}

TYPED_TEST(AlignedBox3xTest, ToMatrixTranspose)
{
	typedef typename AlignedBox3xTypeSelector<TypeParam>::AlignedBox3xType AlignedBox3xType;
	typedef typename AlignedBox3xTypeSelector<TypeParam>::Matrix44Type Matrix44Type;

	AlignedBox3xType box = {{{0, 1, 2, 3}}, {{3, 5, 7, -9}}};

	Matrix44Type matrix, transposedMatrix;
	dsAlignedBox3x_toMatrix(&matrix, &box);
	dsAlignedBox3x_toMatrixTranspose(&transposedMatrix, &box);

	for (unsigned int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
			EXPECT_EQ(matrix.values[j][i], transposedMatrix.values[i][j]);
	}
}

TYPED_TEST(AlignedBox3xTest, Corners)
{
	typedef typename AlignedBox3xTypeSelector<TypeParam>::AlignedBox3xType AlignedBox3xType;
	typedef typename AlignedBox3xTypeSelector<TypeParam>::Vector3xType Vector3xType;

	AlignedBox3xType box = {{{0, 1, 2, 6}}, {{3, 4, 5, -7}}};

	Vector3xType corners[DS_BOX3_CORNER_COUNT];
	dsAlignedBox3x_corners(corners, &box);

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

TYPED_TEST(AlignedBox3xTest, ClosestPoint)
{
	typedef typename AlignedBox3xTypeSelector<TypeParam>::AlignedBox3xType AlignedBox3xType;
	typedef typename AlignedBox3xTypeSelector<TypeParam>::Vector3xType Vector3xType;

	AlignedBox3xType box = {{{0, 1, 2, 6}}, {{3, 4, 5, -7}}};

	Vector3xType point1 = {{1, 2, 3, 1}};
	Vector3xType point2 = {{-1, 2, 3, 2}};
	Vector3xType point3 = {{1, -2, 3, 3}};
	Vector3xType point4 = {{1, 2, -3, 4}};
	Vector3xType point5 = {{4, 2, 3, 5}};
	Vector3xType point6 = {{1, 5, 3, 6}};
	Vector3xType point7 = {{1, 2, 6, 7}};

	Vector3xType closest;
	dsAlignedBox3x_closestPoint(&closest, &box, &box.min);
	EXPECT_EQ((TypeParam)0, closest.x);
	EXPECT_EQ((TypeParam)1, closest.y);
	EXPECT_EQ((TypeParam)2, closest.z);

	dsAlignedBox3x_closestPoint(&closest, &box, &box.max);
	EXPECT_EQ((TypeParam)3, closest.x);
	EXPECT_EQ((TypeParam)4, closest.y);
	EXPECT_EQ((TypeParam)5, closest.z);

	dsAlignedBox3x_closestPoint(&closest, &box, &point1);
	EXPECT_EQ((TypeParam)1, closest.x);
	EXPECT_EQ((TypeParam)2, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsAlignedBox3x_closestPoint(&closest, &box, &point2);
	EXPECT_EQ((TypeParam)0, closest.x);
	EXPECT_EQ((TypeParam)2, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsAlignedBox3x_closestPoint(&closest, &box, &point3);
	EXPECT_EQ((TypeParam)1, closest.x);
	EXPECT_EQ((TypeParam)1, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsAlignedBox3x_closestPoint(&closest, &box, &point4);
	EXPECT_EQ((TypeParam)1, closest.x);
	EXPECT_EQ((TypeParam)2, closest.y);
	EXPECT_EQ((TypeParam)2, closest.z);

	dsAlignedBox3x_closestPoint(&closest, &box, &point5);
	EXPECT_EQ((TypeParam)3, closest.x);
	EXPECT_EQ((TypeParam)2, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsAlignedBox3x_closestPoint(&closest, &box, &point6);
	EXPECT_EQ((TypeParam)1, closest.x);
	EXPECT_EQ((TypeParam)4, closest.y);
	EXPECT_EQ((TypeParam)3, closest.z);

	dsAlignedBox3x_closestPoint(&closest, &box, &point7);
	EXPECT_EQ((TypeParam)1, closest.x);
	EXPECT_EQ((TypeParam)2, closest.y);
	EXPECT_EQ((TypeParam)5, closest.z);
}

TYPED_TEST(AlignedBox3xTest, MakeInvalid)
{
	typedef typename AlignedBox3xTypeSelector<TypeParam>::AlignedBox3xType AlignedBox3xType;

	AlignedBox3xType box = {{{0, 1, 2, 6}}, {{3, 4, 5, -7}}};

	dsAlignedBox3x_makeInvalid(&box);
	EXPECT_FALSE(dsAlignedBox3x_isValid(&box));
}

TYPED_TEST(AlignedBox3xTest, Dist2)
{
	typedef typename AlignedBox3xTypeSelector<TypeParam>::AlignedBox3xType AlignedBox3xType;
	typedef typename AlignedBox3xTypeSelector<TypeParam>::Vector3xType Vector3xType;

	AlignedBox3xType box = {{{0, 1, 2, 6}}, {{3, 4, 5, -7}}};

	Vector3xType point1 = {{1, 2, 3, 1}};
	Vector3xType point2 = {{-1, 2, 3, 2}};
	Vector3xType point3 = {{1, -2, 3, 3}};
	Vector3xType point4 = {{1, 2, -3, 4}};
	Vector3xType point5 = {{4, 2, 3, 5}};
	Vector3xType point6 = {{1, 6, 3, 6}};
	Vector3xType point7 = {{1, 2, 8, 7}};

	EXPECT_EQ((TypeParam)0, dsAlignedBox3x_dist2(&box, &box.min));
	EXPECT_EQ((TypeParam)0, dsAlignedBox3x_dist2(&box, &box.max));
	EXPECT_EQ((TypeParam)0, dsAlignedBox3x_dist2(&box, &point1));
	EXPECT_EQ((TypeParam)1, dsAlignedBox3x_dist2(&box, &point2));
	EXPECT_EQ((TypeParam)9, dsAlignedBox3x_dist2(&box, &point3));
	EXPECT_EQ((TypeParam)25, dsAlignedBox3x_dist2(&box, &point4));
	EXPECT_EQ((TypeParam)1, dsAlignedBox3x_dist2(&box, &point5));
	EXPECT_EQ((TypeParam)4, dsAlignedBox3x_dist2(&box, &point6));
	EXPECT_EQ((TypeParam)9, dsAlignedBox3x_dist2(&box, &point7));
}

TYPED_TEST(AlignedBox3xTest, Dist)
{
	typedef typename AlignedBox3xTypeSelector<TypeParam>::AlignedBox3xType AlignedBox3xType;
	typedef typename AlignedBox3xTypeSelector<TypeParam>::Vector3xType Vector3xType;

	AlignedBox3xType box = {{{0, 1, 2, 6}}, {{3, 4, 5, -7}}};

	Vector3xType point1 = {{1, 2, 3, 1}};
	Vector3xType point2 = {{-1, 2, 3, 2}};
	Vector3xType point3 = {{1, -2, 3, 3}};
	Vector3xType point4 = {{1, 2, -3, 4}};
	Vector3xType point5 = {{4, 2, 3, 5}};
	Vector3xType point6 = {{1, 6, 3, 6}};
	Vector3xType point7 = {{1, 2, 8, 7}};

	EXPECT_FLOAT_EQ(0.0f, (float)dsAlignedBox3x_dist(&box, &box.min));
	EXPECT_FLOAT_EQ(0.0f, (float)dsAlignedBox3x_dist(&box, &box.max));
	EXPECT_FLOAT_EQ(0.0f, (float)dsAlignedBox3x_dist(&box, &point1));
	EXPECT_FLOAT_EQ(1.0f, (float)dsAlignedBox3x_dist(&box, &point2));
	EXPECT_FLOAT_EQ(3.0f, (float)dsAlignedBox3x_dist(&box, &point3));
	EXPECT_FLOAT_EQ(5.0f, (float)dsAlignedBox3x_dist(&box, &point4));
	EXPECT_FLOAT_EQ(1.0f, (float)dsAlignedBox3x_dist(&box, &point5));
	EXPECT_FLOAT_EQ(2.0f, (float)dsAlignedBox3x_dist(&box, &point6));
	EXPECT_FLOAT_EQ(3.0f, (float)dsAlignedBox3x_dist(&box, &point7));
}

TEST(AlignedBox3xTest, ConvertFloatToDouble)
{
	dsAlignedBox3xf boxf = {{{0, 1, 2, 6}}, {{3, 4, 5, -7}}};

	dsAlignedBox3xd boxd;
	dsConvertFloatToDouble(boxd, boxf);

	EXPECT_FLOAT_EQ(boxf.min.x, (float)boxd.min.x);
	EXPECT_FLOAT_EQ(boxf.min.y, (float)boxd.min.y);
	EXPECT_FLOAT_EQ(boxf.min.z, (float)boxd.min.z);

	EXPECT_FLOAT_EQ(boxf.max.x, (float)boxd.max.x);
	EXPECT_FLOAT_EQ(boxf.max.y, (float)boxd.max.y);
	EXPECT_FLOAT_EQ(boxf.max.z, (float)boxd.max.z);
}

TEST(AlignedBox3xTest, ConvertDoubleToFloat)
{
	dsAlignedBox3xd boxd = {{{0, 1, 2, 6}}, {{3, 4, 5, -7}}};

	dsAlignedBox3xf boxf;
	dsConvertDoubleToFloat(boxf, boxd);

	EXPECT_FLOAT_EQ((float)boxd.min.x, boxf.min.x);
	EXPECT_FLOAT_EQ((float)boxd.min.y, boxf.min.y);
	EXPECT_FLOAT_EQ((float)boxd.min.z, boxf.min.z);

	EXPECT_FLOAT_EQ((float)boxd.max.x, boxf.max.x);
	EXPECT_FLOAT_EQ((float)boxd.max.y, boxf.max.y);
	EXPECT_FLOAT_EQ((float)boxd.max.z, boxf.max.z);
}

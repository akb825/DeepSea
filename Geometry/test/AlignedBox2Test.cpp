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

#include <DeepSea/Geometry/AlignedBox2.h>
#include <gtest/gtest.h>

template <typename T>
struct AlignedBox2TypeSelector;

template <>
struct AlignedBox2TypeSelector<float>
{
	typedef dsVector2f Vector2Type;
	typedef dsAlignedBox2f AlignedBox2Type;
};

template <>
struct AlignedBox2TypeSelector<double>
{
	typedef dsVector2d Vector2Type;
	typedef dsAlignedBox2d AlignedBox2Type;
};

template <>
struct AlignedBox2TypeSelector<int>
{
	typedef dsVector2i Vector2Type;
	typedef dsAlignedBox2i AlignedBox2Type;
};

template <typename T>
class AlignedBox2Test : public testing::Test
{
};

using AlignedBox2Types = testing::Types<float, double, int>;
TYPED_TEST_CASE(AlignedBox2Test, AlignedBox2Types);

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

	AlignedBox2Type box = {{0, 1}, {2, 3}};
	EXPECT_EQ(0, box.min.x);
	EXPECT_EQ(1, box.min.y);
	EXPECT_EQ(2, box.max.x);
	EXPECT_EQ(3, box.max.y);
}

TYPED_TEST(AlignedBox2Test, IsValid)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;

	AlignedBox2Type box = {{0, 0}, {1, 1}};
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

	AlignedBox2Type box = {{0, 1}, {2, 3}};

	Vector2Type point1 = {0, 3};
	Vector2Type point2 = {1, 2};
	Vector2Type point3 = {-1, 1};
	Vector2Type point4 = {0, -2};
	Vector2Type point5 = {3, 1};
	Vector2Type point6 = {0, 4};

	dsAlignedBox2_addPoint(box, point1);
	EXPECT_EQ(0, box.min.x);
	EXPECT_EQ(1, box.min.y);
	EXPECT_EQ(2, box.max.x);
	EXPECT_EQ(3, box.max.y);

	dsAlignedBox2_addPoint(box, point2);
	EXPECT_EQ(0, box.min.x);
	EXPECT_EQ(1, box.min.y);
	EXPECT_EQ(2, box.max.x);
	EXPECT_EQ(3, box.max.y);

	dsAlignedBox2_addPoint(box, point3);
	EXPECT_EQ(-1, box.min.x);
	EXPECT_EQ(1, box.min.y);
	EXPECT_EQ(2, box.max.x);
	EXPECT_EQ(3, box.max.y);

	dsAlignedBox2_addPoint(box, point4);
	EXPECT_EQ(-1, box.min.x);
	EXPECT_EQ(-2, box.min.y);
	EXPECT_EQ(2, box.max.x);
	EXPECT_EQ(3, box.max.y);

	dsAlignedBox2_addPoint(box, point5);
	EXPECT_EQ(-1, box.min.x);
	EXPECT_EQ(-2, box.min.y);
	EXPECT_EQ(3, box.max.x);
	EXPECT_EQ(3, box.max.y);

	dsAlignedBox2_addPoint(box, point6);
	EXPECT_EQ(-1, box.min.x);
	EXPECT_EQ(-2, box.min.y);
	EXPECT_EQ(3, box.max.x);
	EXPECT_EQ(4, box.max.y);
}

TYPED_TEST(AlignedBox2Test, AddBox)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;

	AlignedBox2Type box = {{0, 1}, {2, 3}};

	AlignedBox2Type box1 = {{1, 1}, {2, 2}};
	AlignedBox2Type box2 = {{-1, 1}, {2, 2}};
	AlignedBox2Type box3 = {{1, -2}, {2, 2}};
	AlignedBox2Type box4 = {{1, 1}, {3, 2}};
	AlignedBox2Type box5 = {{1, 1}, {2, 4}};

	dsAlignedBox2_addBox(box, box1);
	EXPECT_EQ(0, box.min.x);
	EXPECT_EQ(1, box.min.y);
	EXPECT_EQ(2, box.max.x);
	EXPECT_EQ(3, box.max.y);

	dsAlignedBox2_addBox(box, box2);
	EXPECT_EQ(-1, box.min.x);
	EXPECT_EQ(1, box.min.y);
	EXPECT_EQ(2, box.max.x);
	EXPECT_EQ(3, box.max.y);

	dsAlignedBox2_addBox(box, box3);
	EXPECT_EQ(-1, box.min.x);
	EXPECT_EQ(-2, box.min.y);
	EXPECT_EQ(2, box.max.x);
	EXPECT_EQ(3, box.max.y);

	dsAlignedBox2_addBox(box, box4);
	EXPECT_EQ(-1, box.min.x);
	EXPECT_EQ(-2, box.min.y);
	EXPECT_EQ(3, box.max.x);
	EXPECT_EQ(3, box.max.y);

	dsAlignedBox2_addBox(box, box5);
	EXPECT_EQ(-1, box.min.x);
	EXPECT_EQ(-2, box.min.y);
	EXPECT_EQ(3, box.max.x);
	EXPECT_EQ(4, box.max.y);
}

TYPED_TEST(AlignedBox2Test, ContainsPoint)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;
	typedef typename AlignedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;

	AlignedBox2Type box = {{0, 1}, {2, 3}};

	Vector2Type point1 = {1, 2};
	Vector2Type point2 = {-1, 2};
	Vector2Type point3 = {1, -2};
	Vector2Type point4 = {3, 2};
	Vector2Type point5 = {1, 4};

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

	AlignedBox2Type box = {{0, 1}, {4, 5}};

	AlignedBox2Type box1 = {{1, 2}, {3, 4}};
	AlignedBox2Type box2 = {{-1, 2}, {3, 4}};
	AlignedBox2Type box3 = {{1, -2}, {3, 4}};
	AlignedBox2Type box4 = {{1, 2}, {5, 4}};
	AlignedBox2Type box5 = {{1, 2}, {3, 6}};
	AlignedBox2Type box6 = {{-4, 2}, {-2, 4}};
	AlignedBox2Type box7 = {{6, 2}, {8, 4}};
	AlignedBox2Type box8 = {{1, -2}, {3, -1}};
	AlignedBox2Type box9 = {{1, 6}, {3, 7}};

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

	AlignedBox2Type box = {{0, 1}, {4, 5}};

	AlignedBox2Type box1 = {{1, 2}, {3, 4}};
	AlignedBox2Type box2 = {{-1, 2}, {3, 4}};
	AlignedBox2Type box3 = {{1, -2}, {3, 4}};
	AlignedBox2Type box4 = {{1, 2}, {5, 4}};
	AlignedBox2Type box5 = {{1, 2}, {3, 6}};
	AlignedBox2Type box6 = {{-4, 2}, {-2, 4}};
	AlignedBox2Type box7 = {{6, 2}, {8, 4}};
	AlignedBox2Type box8 = {{1, -2}, {3, -1}};
	AlignedBox2Type box9 = {{1, 6}, {3, 7}};

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

	AlignedBox2Type box = {{0, 1}, {4, 5}};

	AlignedBox2Type box1 = {{1, 2}, {3, 4}};
	AlignedBox2Type box2 = {{-1, 2}, {3, 4}};
	AlignedBox2Type box3 = {{1, -2}, {3, 4}};
	AlignedBox2Type box4 = {{1, 2}, {5, 4}};
	AlignedBox2Type box5 = {{1, 2}, {3, 6}};
	AlignedBox2Type box6 = {{-4, 2}, {-2, 4}};
	AlignedBox2Type box7 = {{6, 2}, {8, 4}};
	AlignedBox2Type box8 = {{1, -2}, {3, -1}};
	AlignedBox2Type box9 = {{1, 6}, {3, 7}};

	AlignedBox2Type intersection;
	dsAlignedBox2_intersect(intersection, box, box);
	EXPECT_EQ(0, intersection.min.x);
	EXPECT_EQ(1, intersection.min.y);
	EXPECT_EQ(4, intersection.max.x);
	EXPECT_EQ(5, intersection.max.y);

	dsAlignedBox2_intersect(intersection, box, box1);
	EXPECT_EQ(1, intersection.min.x);
	EXPECT_EQ(2, intersection.min.y);
	EXPECT_EQ(3, intersection.max.x);
	EXPECT_EQ(4, intersection.max.y);

	dsAlignedBox2_intersect(intersection, box, box2);
	EXPECT_EQ(0, intersection.min.x);
	EXPECT_EQ(2, intersection.min.y);
	EXPECT_EQ(3, intersection.max.x);
	EXPECT_EQ(4, intersection.max.y);

	dsAlignedBox2_intersect(intersection, box, box3);
	EXPECT_EQ(1, intersection.min.x);
	EXPECT_EQ(1, intersection.min.y);
	EXPECT_EQ(3, intersection.max.x);
	EXPECT_EQ(4, intersection.max.y);

	dsAlignedBox2_intersect(intersection, box, box4);
	EXPECT_EQ(1, intersection.min.x);
	EXPECT_EQ(2, intersection.min.y);
	EXPECT_EQ(4, intersection.max.x);
	EXPECT_EQ(4, intersection.max.y);

	dsAlignedBox2_intersect(intersection, box, box5);
	EXPECT_EQ(1, intersection.min.x);
	EXPECT_EQ(2, intersection.min.y);
	EXPECT_EQ(3, intersection.max.x);
	EXPECT_EQ(5, intersection.max.y);

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

	AlignedBox2Type box = {{0, 1}, {4, 5}};

	Vector2Type center;
	dsAlignedBox2_center(center, box);
	EXPECT_EQ(2, center.x);
	EXPECT_EQ(3, center.y);
}

TYPED_TEST(AlignedBox2Test, Extents)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;
	typedef typename AlignedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;

	AlignedBox2Type box = {{0, 1}, {4, 6}};

	Vector2Type extents;
	dsAlignedBox2_extents(extents, box);
	EXPECT_EQ(4, extents.x);
	EXPECT_EQ(5, extents.y);
}

TYPED_TEST(AlignedBox2Test, ClosestPoint)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;
	typedef typename AlignedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;

	AlignedBox2Type box = {{0, 1}, {2, 3}};

	Vector2Type point1 = {1, 2};
	Vector2Type point2 = {-1, 2};
	Vector2Type point3 = {1, -2};
	Vector2Type point4 = {3, 2};
	Vector2Type point5 = {1, 4};

	Vector2Type closest;
	dsAlignedBox2_closestPoint(closest, box, box.min);
	EXPECT_EQ(0, closest.x);
	EXPECT_EQ(1, closest.y);

	dsAlignedBox2_closestPoint(closest, box, box.max);
	EXPECT_EQ(2, closest.x);
	EXPECT_EQ(3, closest.y);

	dsAlignedBox2_closestPoint(closest, box, point1);
	EXPECT_EQ(1, closest.x);
	EXPECT_EQ(2, closest.y);

	dsAlignedBox2_closestPoint(closest, box, point2);
	EXPECT_EQ(0, closest.x);
	EXPECT_EQ(2, closest.y);

	dsAlignedBox2_closestPoint(closest, box, point3);
	EXPECT_EQ(1, closest.x);
	EXPECT_EQ(1, closest.y);

	dsAlignedBox2_closestPoint(closest, box, point4);
	EXPECT_EQ(2, closest.x);
	EXPECT_EQ(2, closest.y);

	dsAlignedBox2_closestPoint(closest, box, point5);
	EXPECT_EQ(1, closest.x);
	EXPECT_EQ(3, closest.y);
}

TYPED_TEST(AlignedBox2Test, MakeInvalid)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;

	AlignedBox2Type box = {{0, 1}, {2, 3}};

	dsAlignedBox2_makeInvalid(&box);
	EXPECT_FALSE(dsAlignedBox2_isValid(box));
}

TYPED_TEST(AlignedBox2Test, Dist2)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;
	typedef typename AlignedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;

	AlignedBox2Type box = {{0, 1}, {2, 3}};

	Vector2Type point1 = {1, 2};
	Vector2Type point2 = {-1, 2};
	Vector2Type point3 = {1, -2};
	Vector2Type point4 = {3, 2};
	Vector2Type point5 = {1, 4};

	EXPECT_EQ(0, dsAlignedBox2_dist2(&box, &box.min));
	EXPECT_EQ(0, dsAlignedBox2_dist2(&box, &box.max));
	EXPECT_EQ(0, dsAlignedBox2_dist2(&box, &point1));
	EXPECT_EQ(1, dsAlignedBox2_dist2(&box, &point2));
	EXPECT_EQ(9, dsAlignedBox2_dist2(&box, &point3));
	EXPECT_EQ(1, dsAlignedBox2_dist2(&box, &point4));
	EXPECT_EQ(1, dsAlignedBox2_dist2(&box, &point5));
}

TYPED_TEST(AlignedBox2Test, Dist)
{
	typedef typename AlignedBox2TypeSelector<TypeParam>::AlignedBox2Type AlignedBox2Type;
	typedef typename AlignedBox2TypeSelector<TypeParam>::Vector2Type Vector2Type;

	AlignedBox2Type box = {{0, 1}, {2, 3}};

	Vector2Type point1 = {1, 2};
	Vector2Type point2 = {-1, 2};
	Vector2Type point3 = {1, -2};
	Vector2Type point4 = {3, 2};
	Vector2Type point5 = {1, 4};

	EXPECT_FLOAT_EQ(0.0f, (float)dsAlignedBox2_dist(&box, &box.min));
	EXPECT_FLOAT_EQ(0.0f, (float)dsAlignedBox2_dist(&box, &box.max));
	EXPECT_FLOAT_EQ(0.0f, (float)dsAlignedBox2_dist(&box, &point1));
	EXPECT_FLOAT_EQ(1.0f, (float)dsAlignedBox2_dist(&box, &point2));
	EXPECT_FLOAT_EQ(3.0f, (float)dsAlignedBox2_dist(&box, &point3));
	EXPECT_FLOAT_EQ(1.0f, (float)dsAlignedBox2_dist(&box, &point4));
	EXPECT_FLOAT_EQ(1.0f, (float)dsAlignedBox2_dist(&box, &point5));
}

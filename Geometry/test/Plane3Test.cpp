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

#include <DeepSea/Geometry/Plane3.h>
#include <gtest/gtest.h>

template <typename T>
struct Plane3TypeSelector;

template <>
struct Plane3TypeSelector<float>
{
	typedef dsVector3f Vector3Type;
	typedef dsPlane3f Plane3Type;
	typedef dsAlignedBox3f AlignedBox3Type;
	typedef dsOrientedBox3f OrientedBox3Type;
};

template <>
struct Plane3TypeSelector<double>
{
	typedef dsVector3d Vector3Type;
	typedef dsPlane3d Plane3Type;
	typedef dsAlignedBox3d AlignedBox3Type;
	typedef dsOrientedBox3d OrientedBox3Type;
};

template <typename T>
class Plane3Test : public testing::Test
{
};

using Plane3Types = testing::Types<float, double>;
TYPED_TEST_CASE(Plane3Test, Plane3Types);

inline dsPlaneSide dsPlane3_intersectAlignedBox(const dsPlane3f* plane, const dsAlignedBox3f* box)
{
	return dsPlane3f_intersectAlignedBox(plane, box);
}

inline dsPlaneSide dsPlane3_intersectAlignedBox(const dsPlane3d* plane, const dsAlignedBox3d* box)
{
	return dsPlane3d_intersectAlignedBox(plane, box);
}

inline dsPlaneSide dsPlane3_intersectOrientedBox(const dsPlane3f* plane, const dsOrientedBox3f* box)
{
	return dsPlane3f_intersectOrientedBox(plane, box);
}

inline dsPlaneSide dsPlane3_intersectOrientedBox(const dsPlane3d* plane, const dsOrientedBox3d* box)
{
	return dsPlane3d_intersectOrientedBox(plane, box);
}

TYPED_TEST(Plane3Test, FromNormalPoint)
{
	typedef typename Plane3TypeSelector<TypeParam>::Vector3Type Vector3Type;
	typedef typename Plane3TypeSelector<TypeParam>::Plane3Type Plane3Type;

	Vector3Type normal1 = {1, 0, 0};
	Vector3Type normal2 = {0, 1, 0};
	Vector3Type normal3 = {0, 0, 1};
	Vector3Type point = {2, 3, 4};

	Plane3Type plane;
	dsPlane3_fromNormalPoint(plane, normal1, point);
	EXPECT_EQ(1, plane.n.x);
	EXPECT_EQ(0, plane.n.y);
	EXPECT_EQ(0, plane.n.z);
	EXPECT_EQ(2, plane.d);

	dsPlane3_fromNormalPoint(plane, normal2, point);
	EXPECT_EQ(0, plane.n.x);
	EXPECT_EQ(1, plane.n.y);
	EXPECT_EQ(0, plane.n.z);
	EXPECT_EQ(3, plane.d);

	dsPlane3_fromNormalPoint(plane, normal3, point);
	EXPECT_EQ(0, plane.n.x);
	EXPECT_EQ(0, plane.n.y);
	EXPECT_EQ(1, plane.n.z);
	EXPECT_EQ(4, plane.d);
}

TYPED_TEST(Plane3Test, DistanceToPoint)
{
	typedef typename Plane3TypeSelector<TypeParam>::Vector3Type Vector3Type;
	typedef typename Plane3TypeSelector<TypeParam>::Plane3Type Plane3Type;

	Plane3Type plane = {{{1, 0, 0}}, 2};
	Vector3Type point = {2, 3, 4};
	EXPECT_EQ(0, dsPlane3_distanceToPoint(plane, point));

	plane.n.x = 0;
	plane.n.y = 1;
	EXPECT_EQ(1, dsPlane3_distanceToPoint(plane, point));

	plane.n.y = 0;
	plane.n.z = 1;
	EXPECT_EQ(2, dsPlane3_distanceToPoint(plane, point));
}

TYPED_TEST(Plane3Test, IntersectAlignedBox)
{
	typedef typename Plane3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;
	typedef typename Plane3TypeSelector<TypeParam>::Plane3Type Plane3Type;

	AlignedBox3Type box = {{0, 1, 2}, {3, 4, 5}};

	Plane3Type plane = {{{1, 0, 0}}, 2};
	EXPECT_EQ(dsPlaneSide_Intersects, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.x = 0;
	plane.n.y = 1;
	plane.d = 3;
	EXPECT_EQ(dsPlaneSide_Intersects, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.y = 0;
	plane.n.z = 1;
	plane.d = 4;
	EXPECT_EQ(dsPlaneSide_Intersects, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.z = 0;
	plane.n.x = 1;
	plane.d = -1;
	EXPECT_EQ(dsPlaneSide_Inside, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.x = 0;
	plane.n.y = 1;
	plane.d = 0;
	EXPECT_EQ(dsPlaneSide_Inside, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.y = 0;
	plane.n.z = 1;
	plane.d = 1;
	EXPECT_EQ(dsPlaneSide_Inside, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.z = 0;
	plane.n.x = 1;
	plane.d = 4;
	EXPECT_EQ(dsPlaneSide_Outside, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.x = 0;
	plane.n.y = 1;
	plane.d = 5;
	EXPECT_EQ(dsPlaneSide_Outside, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.y = 0;
	plane.n.z = 1;
	plane.d = 6;
	EXPECT_EQ(dsPlaneSide_Outside, dsPlane3_intersectAlignedBox(&plane, &box));
}

TYPED_TEST(Plane3Test, IntersectOrientedBox)
{
	typedef typename Plane3TypeSelector<TypeParam>::OrientedBox3Type OrientedBox3Type;
	typedef typename Plane3TypeSelector<TypeParam>::Plane3Type Plane3Type;

	OrientedBox3Type box =
	{
		{{ {0, 0, 1}, {-1, 0, 0}, {0, 1, 0} }},
		{{6, 5, 4}}, {{3, 2, 1}}
	};

	Plane3Type plane = {{{1, 0, 0}}, 5};
	EXPECT_EQ(dsPlaneSide_Intersects, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.x = 0;
	plane.n.y = 1;
	plane.d = 5;
	EXPECT_EQ(dsPlaneSide_Intersects, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.y = 0;
	plane.n.z = 1;
	plane.d = 3;
	EXPECT_EQ(dsPlaneSide_Intersects, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.z = 0;
	plane.n.x = 1;
	plane.d = 3;
	EXPECT_EQ(dsPlaneSide_Inside, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.x = 0;
	plane.n.y = 1;
	plane.d = 3;
	EXPECT_EQ(dsPlaneSide_Inside, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.y = 0;
	plane.n.z = 1;
	plane.d = 0;
	EXPECT_EQ(dsPlaneSide_Inside, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.z = 0;
	plane.n.x = 1;
	plane.d = 9;
	EXPECT_EQ(dsPlaneSide_Outside, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.x = 0;
	plane.n.y = 1;
	plane.d = 7;
	EXPECT_EQ(dsPlaneSide_Outside, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.y = 0;
	plane.n.z = 1;
	plane.d = 8;
	EXPECT_EQ(dsPlaneSide_Outside, dsPlane3_intersectOrientedBox(&plane, &box));
}

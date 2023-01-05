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

#include <DeepSea/Geometry/Plane3.h>
#include <DeepSea/Math/Matrix44.h>
#include <gtest/gtest.h>
#include <limits>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

template <typename T>
struct Plane3TypeSelector;

template <>
struct Plane3TypeSelector<float>
{
	typedef dsVector3f Vector3Type;
	typedef dsVector4f Vector4Type;
	typedef dsPlane3f Plane3Type;
	typedef dsRay3f Ray3Type;
	typedef dsAlignedBox3f AlignedBox3Type;
	typedef dsOrientedBox3f OrientedBox3Type;
	typedef dsMatrix44f Matrix44Type;
	static const float epsilon;
};

template <>
struct Plane3TypeSelector<double>
{
	typedef dsVector3d Vector3Type;
	typedef dsVector4d Vector4Type;
	typedef dsPlane3d Plane3Type;
	typedef dsRay3d Ray3Type;
	typedef dsAlignedBox3d AlignedBox3Type;
	typedef dsOrientedBox3d OrientedBox3Type;
	typedef dsMatrix44d Matrix44Type;
	static const double epsilon;
};

const float Plane3TypeSelector<float>::epsilon = 1e-4f;
const double Plane3TypeSelector<double>::epsilon = 1e-13f;

template <typename T>
class Plane3Test : public testing::Test
{
};

using Plane3Types = testing::Types<float, double>;
TYPED_TEST_SUITE(Plane3Test, Plane3Types);

inline void dsPlane3_normalize(dsPlane3f* result, const dsPlane3f* plane)
{
	dsPlane3f_normalize(result, plane);
}

inline void dsPlane3_normalize(dsPlane3d* result, const dsPlane3d* plane)
{
	dsPlane3d_normalize(result, plane);
}

inline void dsPlane3_transform(dsPlane3f* result, const dsMatrix44f* transform,
	const dsPlane3f* plane)
{
	dsPlane3f_transform(result, transform, plane);
}

inline void dsPlane3_transform(dsPlane3d* result, const dsMatrix44d* transform,
	const dsPlane3d* plane)
{
	dsPlane3d_transform(result, transform, plane);
}

inline void dsPlane3_transformInverseTranspose(dsPlane3f* result, const dsMatrix44f* transform,
	const dsPlane3f* plane)
{
	dsPlane3f_transformInverseTranspose(result, transform, plane);
}

inline void dsPlane3_transformInverseTranspose(dsPlane3d* result, const dsMatrix44d* transform,
	const dsPlane3d* plane)
{
	dsPlane3d_transformInverseTranspose(result, transform, plane);
}

inline bool dsPlane3_intersectingLine(dsRay3f* result, const dsPlane3f* firstPlane,
	const dsPlane3f* secondPlane)
{
	return dsPlane3f_intersectingLine(result, firstPlane, secondPlane);
}

inline bool dsPlane3_intersectingLine(dsRay3d* result, const dsPlane3d* firstPlane,
	const dsPlane3d* secondPlane)
{
	return dsPlane3d_intersectingLine(result, firstPlane, secondPlane);
}

inline bool dsPlane3_intersectingPoint(dsVector3f* result, const dsPlane3f* firstPlane,
	const dsPlane3f* secondPlane, const dsPlane3f* thirdPlane)
{
	return dsPlane3f_intersectingPoint(result, firstPlane, secondPlane, thirdPlane);
}

inline bool dsPlane3_intersectingPoint(dsVector3d* result, const dsPlane3d* firstPlane,
	const dsPlane3d* secondPlane, const dsPlane3d* thirdPlane)
{
	return dsPlane3d_intersectingPoint(result, firstPlane, secondPlane, thirdPlane);
}

inline float dsPlane3_rayIntersection(const dsPlane3f* plane, const dsRay3f* ray)
{
	return dsPlane3f_rayIntersection(plane, ray);
}

inline double dsPlane3_rayIntersection(const dsPlane3d* plane, const dsRay3d* ray)
{
	return dsPlane3d_rayIntersection(plane, ray);
}

inline dsIntersectResult dsPlane3_intersectAlignedBox(const dsPlane3f* plane,
	const dsAlignedBox3f* box)
{
	return dsPlane3f_intersectAlignedBox(plane, box);
}

inline dsIntersectResult dsPlane3_intersectAlignedBox(const dsPlane3d* plane,
	const dsAlignedBox3d* box)
{
	return dsPlane3d_intersectAlignedBox(plane, box);
}

inline dsIntersectResult dsPlane3_intersectOrientedBox(const dsPlane3f* plane,
	const dsOrientedBox3f* box)
{
	return dsPlane3f_intersectOrientedBox(plane, box);
}

inline dsIntersectResult dsPlane3_intersectOrientedBox(const dsPlane3d* plane,
	const dsOrientedBox3d* box)
{
	return dsPlane3d_intersectOrientedBox(plane, box);
}

inline void dsMatrix44_makeRotate(dsMatrix44f* result, float x, float y, float z)
{
	dsMatrix44f_makeRotate(result, x, y, z);
}

inline void dsMatrix44_makeRotate(dsMatrix44d* result, double x, double y, double z)
{
	dsMatrix44d_makeRotate(result, x, y, z);
}

inline void dsMatrix44_makeTranslate(dsMatrix44f* result, float x, float y, float z)
{
	dsMatrix44f_makeTranslate(result, x, y, z);
}

inline void dsMatrix44_makeTranslate(dsMatrix44d* result, double x, double y, double z)
{
	dsMatrix44d_makeTranslate(result, x, y, z);
}

inline void dsMatrix44_affineInvert(dsMatrix44f* result, const dsMatrix44f* a)
{
	dsMatrix44f_affineInvert(result, a);
}

inline void dsMatrix44_affineInvert(dsMatrix44d* result, const dsMatrix44d* a)
{
	dsMatrix44d_affineInvert(result, a);
}

TYPED_TEST(Plane3Test, FromNormalPoint)
{
	typedef typename Plane3TypeSelector<TypeParam>::Vector3Type Vector3Type;
	typedef typename Plane3TypeSelector<TypeParam>::Plane3Type Plane3Type;

	Vector3Type normal1 = {{1, 0, 0}};
	Vector3Type normal2 = {{0, 1, 0}};
	Vector3Type normal3 = {{0, 0, 1}};
	Vector3Type point = {{2, 3, 4}};

	Plane3Type plane;
	dsPlane3_fromNormalPoint(plane, normal1, point);
	EXPECT_EQ((TypeParam)1, plane.n.x);
	EXPECT_EQ((TypeParam)0, plane.n.y);
	EXPECT_EQ((TypeParam)0, plane.n.z);
	EXPECT_EQ((TypeParam)-2, plane.d);

	dsPlane3_fromNormalPoint(plane, normal2, point);
	EXPECT_EQ((TypeParam)0, plane.n.x);
	EXPECT_EQ((TypeParam)1, plane.n.y);
	EXPECT_EQ((TypeParam)0, plane.n.z);
	EXPECT_EQ((TypeParam)-3, plane.d);

	dsPlane3_fromNormalPoint(plane, normal3, point);
	EXPECT_EQ((TypeParam)0, plane.n.x);
	EXPECT_EQ((TypeParam)0, plane.n.y);
	EXPECT_EQ((TypeParam)1, plane.n.z);
	EXPECT_EQ((TypeParam)-4, plane.d);
}

TYPED_TEST(Plane3Test, DistanceToPoint)
{
	typedef typename Plane3TypeSelector<TypeParam>::Vector3Type Vector3Type;
	typedef typename Plane3TypeSelector<TypeParam>::Plane3Type Plane3Type;

	Plane3Type plane = {{1, 0, 0, -2}};
	Vector3Type point = {{2, 3, 4}};
	EXPECT_EQ((TypeParam)0, dsPlane3_distanceToPoint(plane, point));

	plane.n.x = 0;
	plane.n.y = 1;
	EXPECT_EQ((TypeParam)1, dsPlane3_distanceToPoint(plane, point));

	plane.n.y = 0;
	plane.n.z = 1;
	EXPECT_EQ((TypeParam)2, dsPlane3_distanceToPoint(plane, point));
}

TYPED_TEST(Plane3Test, Normalize)
{
	typedef typename Plane3TypeSelector<TypeParam>::Plane3Type Plane3Type;
	TypeParam epsilon = Plane3TypeSelector<TypeParam>::epsilon;

	Plane3Type plane = {{2, 0, 0, 4}};
	dsPlane3_normalize(&plane, &plane);
	EXPECT_NEAR(1, plane.n.x, epsilon);
	EXPECT_NEAR(0, plane.n.y, epsilon);
	EXPECT_NEAR(0, plane.n.z, epsilon);
	EXPECT_NEAR(2, plane.d, epsilon);
}

TYPED_TEST(Plane3Test, Transform)
{
	typedef typename Plane3TypeSelector<TypeParam>::Plane3Type Plane3Type;
	typedef typename Plane3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename Plane3TypeSelector<TypeParam>::Vector4Type Vector4Type;
	TypeParam epsilon = Plane3TypeSelector<TypeParam>::epsilon;

	Plane3Type plane = {{1, 0, 0, -2}};

	Matrix44Type rotate, translate, transform;

	dsMatrix44_makeRotate(&rotate, (TypeParam)dsDegreesToRadiansd(30),
		(TypeParam)dsDegreesToRadiansd(-15), (TypeParam)dsDegreesToRadiansd(60));
	dsMatrix44_makeTranslate(&translate, -3, 5, -1);

	dsMatrix44_mul(transform, translate, rotate);

	Vector4Type origN = {{1, 0, 0, 0}};
	Vector4Type newN;
	dsMatrix44_transform(newN, transform, origN);

	dsPlane3_transform(&plane, &transform, &plane);
	EXPECT_NEAR(newN.x, plane.n.x, epsilon);
	EXPECT_NEAR(newN.y, plane.n.y, epsilon);
	EXPECT_NEAR(newN.z, plane.n.z, epsilon);
	EXPECT_NEAR(-dsVector3_dot(newN, transform.columns[3]) - 2, plane.d, epsilon);
}

TYPED_TEST(Plane3Test, TransformInverseTranspose)
{
	typedef typename Plane3TypeSelector<TypeParam>::Plane3Type Plane3Type;
	typedef typename Plane3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename Plane3TypeSelector<TypeParam>::Vector4Type Vector4Type;
	TypeParam epsilon = Plane3TypeSelector<TypeParam>::epsilon;

	Plane3Type plane = {{1, 0, 0, -2}};

	Matrix44Type rotate, translate, transform, inverse, inverseTranspose;

	dsMatrix44_makeRotate(&rotate, (TypeParam)dsDegreesToRadiansd(30),
		(TypeParam)dsDegreesToRadiansd(-15), (TypeParam)dsDegreesToRadiansd(60));
	dsMatrix44_makeTranslate(&translate, -3, 5, -1);

	dsMatrix44_mul(transform, translate, rotate);
	dsMatrix44_affineInvert(&inverse, &transform);
	dsMatrix44_transpose(inverseTranspose, inverse);

	Vector4Type origN = {{1, 0, 0, 0}};
	Vector4Type newN;
	dsMatrix44_transform(newN, transform, origN);

	dsPlane3_transformInverseTranspose(&plane, &inverseTranspose, &plane);
	EXPECT_NEAR(newN.x, plane.n.x, epsilon);
	EXPECT_NEAR(newN.y, plane.n.y, epsilon);
	EXPECT_NEAR(newN.z, plane.n.z, epsilon);
	EXPECT_NEAR(-dsVector3_dot(newN, transform.columns[3]) - 2, plane.d, epsilon);
}

TYPED_TEST(Plane3Test, IntersectingLine)
{
	typedef typename Plane3TypeSelector<TypeParam>::Plane3Type Plane3Type;
	typedef typename Plane3TypeSelector<TypeParam>::Ray3Type Ray3Type;
	TypeParam epsilon = Plane3TypeSelector<TypeParam>::epsilon;

	Plane3Type firstPlane = {{1, 0, 0, -2}};
	Plane3Type secondPlane = {{0, 1, 0, -3}};

	Ray3Type ray;
	EXPECT_FALSE(dsPlane3_intersectingLine(&ray, &firstPlane, &firstPlane));
	EXPECT_TRUE(dsPlane3_intersectingLine(&ray, &firstPlane, &secondPlane));

	EXPECT_NEAR(0, dsPlane3_distanceToPoint(firstPlane, ray.origin), epsilon);
	EXPECT_NEAR(0, dsPlane3_distanceToPoint(secondPlane, ray.origin), epsilon);
	EXPECT_NEAR(0, ray.direction.x, epsilon);
	EXPECT_NEAR(0, ray.direction.y, epsilon);
	EXPECT_NEAR(1, ray.direction.z, epsilon);
}

TYPED_TEST(Plane3Test, IntersectingPoint)
{
	typedef typename Plane3TypeSelector<TypeParam>::Plane3Type Plane3Type;
	typedef typename Plane3TypeSelector<TypeParam>::Vector3Type Vector3Type;
	TypeParam epsilon = Plane3TypeSelector<TypeParam>::epsilon;

	Plane3Type firstPlane = {{1, 0, 0, -2}};
	Plane3Type secondPlane = {{0, 1, 0, -3}};
	Plane3Type thirdPlane = {{0, 0, 1, -4}};

	Vector3Type point;
	EXPECT_FALSE(dsPlane3_intersectingPoint(&point, &firstPlane, &firstPlane, &thirdPlane));
	EXPECT_FALSE(dsPlane3_intersectingPoint(&point, &firstPlane, &secondPlane, &firstPlane));
	EXPECT_TRUE(dsPlane3_intersectingPoint(&point, &firstPlane, &secondPlane, &thirdPlane));

	EXPECT_NEAR(2, point.x, epsilon);
	EXPECT_NEAR(3, point.y, epsilon);
	EXPECT_NEAR(4, point.z, epsilon);
}

TYPED_TEST(Plane3Test, RayIntersection)
{
	typedef typename Plane3TypeSelector<TypeParam>::Plane3Type Plane3Type;
	typedef typename Plane3TypeSelector<TypeParam>::Ray3Type Ray3Type;
	TypeParam epsilon = Plane3TypeSelector<TypeParam>::epsilon;

	Plane3Type plane = {{1, 0, 0, -2}};
	Ray3Type ray = {{{1, 2, 3}}, {{-1, 0, 0}}};

	EXPECT_NEAR(-1, dsPlane3_rayIntersection(&plane, &ray), epsilon);

	ray.origin.x = 3;
	EXPECT_NEAR(1, dsPlane3_rayIntersection(&plane, &ray), epsilon);

	ray.direction.x = 0;
	ray.direction.y = 1;
	EXPECT_EQ(std::numeric_limits<TypeParam>::max(), dsPlane3_rayIntersection(&plane, &ray));
}

TYPED_TEST(Plane3Test, IntersectAlignedBox)
{
	typedef typename Plane3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;
	typedef typename Plane3TypeSelector<TypeParam>::Plane3Type Plane3Type;

	AlignedBox3Type box = {{{0, 1, 2}}, {{3, 4, 5}}};

	Plane3Type plane = {{1, 0, 0, -2}};
	//Positive normals
	EXPECT_EQ(dsIntersectResult_Intersects, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.x = 0;
	plane.n.y = 1;
	plane.d = -3;
	EXPECT_EQ(dsIntersectResult_Intersects, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.y = 0;
	plane.n.z = 1;
	plane.d = -4;
	EXPECT_EQ(dsIntersectResult_Intersects, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.z = 0;
	plane.n.x = 1;
	plane.d = 1;
	EXPECT_EQ(dsIntersectResult_Inside, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.x = 0;
	plane.n.y = 1;
	plane.d = 0;
	EXPECT_EQ(dsIntersectResult_Inside, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.y = 0;
	plane.n.z = 1;
	plane.d = -1;
	EXPECT_EQ(dsIntersectResult_Inside, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.z = 0;
	plane.n.x = 1;
	plane.d = -4;
	EXPECT_EQ(dsIntersectResult_Outside, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.x = 0;
	plane.n.y = 1;
	plane.d = -5;
	EXPECT_EQ(dsIntersectResult_Outside, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.y = 0;
	plane.n.z = 1;
	plane.d = -6;
	EXPECT_EQ(dsIntersectResult_Outside, dsPlane3_intersectAlignedBox(&plane, &box));

	// Negative normals
	plane.n.z = 0;
	plane.n.x = -1;
	plane.d = 2;
	EXPECT_EQ(dsIntersectResult_Intersects, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.x = 0;
	plane.n.y = -1;
	plane.d = 3;
	EXPECT_EQ(dsIntersectResult_Intersects, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.y = 0;
	plane.n.z = -1;
	plane.d = 4;
	EXPECT_EQ(dsIntersectResult_Intersects, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.z = 0;
	plane.n.x = -1;
	plane.d = -1;
	EXPECT_EQ(dsIntersectResult_Outside, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.x = 0;
	plane.n.y = -1;
	plane.d = 0;
	EXPECT_EQ(dsIntersectResult_Outside, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.y = 0;
	plane.n.z = -1;
	plane.d = 1;
	EXPECT_EQ(dsIntersectResult_Outside, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.z = 0;
	plane.n.x = -1;
	plane.d = 4;
	EXPECT_EQ(dsIntersectResult_Inside, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.x = 0;
	plane.n.y = -1;
	plane.d = 5;
	EXPECT_EQ(dsIntersectResult_Inside, dsPlane3_intersectAlignedBox(&plane, &box));

	plane.n.y = 0;
	plane.n.z = -1;
	plane.d = 6;
	EXPECT_EQ(dsIntersectResult_Inside, dsPlane3_intersectAlignedBox(&plane, &box));
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

	Plane3Type plane = {{1, 0, 0, -5}};
	// Positive normals
	EXPECT_EQ(dsIntersectResult_Intersects, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.x = 0;
	plane.n.y = 1;
	plane.d = -5;
	EXPECT_EQ(dsIntersectResult_Intersects, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.y = 0;
	plane.n.z = 1;
	plane.d = -3;
	EXPECT_EQ(dsIntersectResult_Intersects, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.z = 0;
	plane.n.x = 1;
	plane.d = -3;
	EXPECT_EQ(dsIntersectResult_Inside, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.x = 0;
	plane.n.y = 1;
	plane.d = -3;
	EXPECT_EQ(dsIntersectResult_Inside, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.y = 0;
	plane.n.z = 1;
	plane.d = 0;
	EXPECT_EQ(dsIntersectResult_Inside, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.z = 0;
	plane.n.x = 1;
	plane.d = -9;
	EXPECT_EQ(dsIntersectResult_Outside, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.x = 0;
	plane.n.y = 1;
	plane.d = -7;
	EXPECT_EQ(dsIntersectResult_Outside, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.y = 0;
	plane.n.z = 1;
	plane.d = -8;
	EXPECT_EQ(dsIntersectResult_Outside, dsPlane3_intersectOrientedBox(&plane, &box));

	// Negative normals
	plane.n.z = 0;
	plane.n.x = -1;
	plane.d = 5;
	EXPECT_EQ(dsIntersectResult_Intersects, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.x = 0;
	plane.n.y = -1;
	plane.d = 5;
	EXPECT_EQ(dsIntersectResult_Intersects, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.y = 0;
	plane.n.z = -1;
	plane.d = 3;
	EXPECT_EQ(dsIntersectResult_Intersects, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.z = 0;
	plane.n.x = -1;
	plane.d = 3;
	EXPECT_EQ(dsIntersectResult_Outside, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.x = 0;
	plane.n.y = -1;
	plane.d = 3;
	EXPECT_EQ(dsIntersectResult_Outside, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.y = 0;
	plane.n.z = -1;
	plane.d = 0;
	EXPECT_EQ(dsIntersectResult_Outside, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.z = 0;
	plane.n.x = -1;
	plane.d = 9;
	EXPECT_EQ(dsIntersectResult_Inside, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.x = 0;
	plane.n.y = -1;
	plane.d = 7;
	EXPECT_EQ(dsIntersectResult_Inside, dsPlane3_intersectOrientedBox(&plane, &box));

	plane.n.y = 0;
	plane.n.z = -1;
	plane.d = 8;
	EXPECT_EQ(dsIntersectResult_Inside, dsPlane3_intersectOrientedBox(&plane, &box));
}

TEST(Plane3, ConvertFloatToDouble)
{
	dsPlane3f planef = {{1, 0, 0, -2}};

	dsPlane3d planed;
	dsConvertFloatToDouble(planed, planef);

	EXPECT_FLOAT_EQ(planef.n.x, (float)planed.n.x);
	EXPECT_FLOAT_EQ(planef.n.y, (float)planed.n.y);
	EXPECT_FLOAT_EQ(planef.n.z, (float)planed.n.z);
	EXPECT_FLOAT_EQ(planef.d, (float)planed.d);
}

TEST(Plane3, ConvertDoubleToFloat)
{
	dsPlane3d planed = {{1, 0, 0, -2}};

	dsPlane3f planef;
	dsConvertDoubleToFloat(planef, planed);

	EXPECT_FLOAT_EQ((float)planed.n.x, planef.n.x);
	EXPECT_FLOAT_EQ((float)planed.n.y, planef.n.y);
	EXPECT_FLOAT_EQ((float)planed.n.z, planef.n.z);
	EXPECT_FLOAT_EQ((float)planed.d, planef.d);
}

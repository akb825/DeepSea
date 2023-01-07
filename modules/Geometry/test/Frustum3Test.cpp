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

#include <DeepSea/Geometry/Frustum3.h>
#include <DeepSea/Geometry/OrientedBox3.h>
#include <DeepSea/Geometry/Plane3.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Matrix33.h>
#include <gtest/gtest.h>
#include <cmath>

// Handle older versions of gtest.
#ifndef TYPED_TEST_SUITE
#define TYPED_TEST_SUITE TYPED_TEST_CASE
#endif

template <typename T>
struct Frustum3TypeSelector;

template <>
struct Frustum3TypeSelector<float>
{
	typedef dsMatrix44f Matrix44Type;
	typedef dsPlane3f Plane3Type;
	typedef dsVector3f Vector3Type;
	typedef dsAlignedBox3f AlignedBox3Type;
	typedef dsOrientedBox3f OrientedBox3Type;
	typedef dsFrustum3f Frustum3Type;
	static const float epsilon;
};

template <>
struct Frustum3TypeSelector<double>
{
	typedef dsMatrix44d Matrix44Type;
	typedef dsPlane3d Plane3Type;
	typedef dsVector3d Vector3Type;
	typedef dsAlignedBox3d AlignedBox3Type;
	typedef dsOrientedBox3d OrientedBox3Type;
	typedef dsFrustum3d Frustum3Type;
	static const double epsilon;
};

const float Frustum3TypeSelector<float>::epsilon = 1e-4f;
const double Frustum3TypeSelector<double>::epsilon = 1e-13f;

template <typename T>
class Frustum3Test : public testing::Test
{
};

using Frustum3Types = testing::Types<float, double>;
TYPED_TEST_SUITE(Frustum3Test, Frustum3Types);

inline void dsFrustum3_normalize(dsFrustum3f* frustum)
{
	dsFrustum3f_normalize(frustum);
}

inline void dsFrustum3_normalize(dsFrustum3d* frustum)
{
	dsFrustum3d_normalize(frustum);
}

inline void dsFrustum3_transform(dsFrustum3f* frustum, const dsMatrix44f* transform)
{
	dsFrustum3f_transform(frustum, transform);
}

inline void dsFrustum3_transform(dsFrustum3d* frustum, const dsMatrix44d* transform)
{
	dsFrustum3d_transform(frustum, transform);
}

inline void dsFrustum3_transformInverseTranspose(dsFrustum3f* frustum, const dsMatrix44f* transform)
{
	dsFrustum3f_transformInverseTranspose(frustum, transform);
}

inline void dsFrustum3_transformInverseTranspose(dsFrustum3d* frustum, const dsMatrix44d* transform)
{
	dsFrustum3d_transformInverseTranspose(frustum, transform);
}

inline bool dsFrustum3_isInfinite(const dsFrustum3f* frustum)
{
	return dsFrustum3f_isInfinite(frustum);
}

inline bool dsFrustum3_isInfinite(const dsFrustum3d* frustum)
{
	return dsFrustum3d_isInfinite(frustum);
}

inline dsIntersectResult dsFrustum3_intersectAlignedBox(const dsFrustum3f* frustum,
	const dsAlignedBox3f* box)
{
	return dsFrustum3f_intersectAlignedBox(frustum, box);
}

inline dsIntersectResult dsFrustum3_intersectAlignedBox(const dsFrustum3d* frustum,
	const dsAlignedBox3d* box)
{
	return dsFrustum3d_intersectAlignedBox(frustum, box);
}

inline dsIntersectResult dsFrustum3_intersectOrientedBox(const dsFrustum3f* frustum,
	const dsOrientedBox3f* box)
{
	return dsFrustum3f_intersectOrientedBox(frustum, box);
}

inline dsIntersectResult dsFrustum3_intersectOrientedBox(const dsFrustum3d* frustum,
	const dsOrientedBox3d* box)
{
	return dsFrustum3d_intersectOrientedBox(frustum, box);
}

inline dsIntersectResult dsFrustum3_intersectBoxMatrix(const dsFrustum3f* frustum,
	const dsMatrix44f* boxMatrix)
{
	return dsFrustum3f_intersectBoxMatrix(frustum, boxMatrix);
}

inline dsIntersectResult dsFrustum3_intersectBoxMatrix(const dsFrustum3d* frustum,
	const dsMatrix44d* boxMatrix)
{
	return dsFrustum3d_intersectBoxMatrix(frustum, boxMatrix);
}

inline dsIntersectResult dsFrustum3_intersectSphere(const dsFrustum3f* frustum,
	const dsVector3f* center, float radius)
{
	return dsFrustum3f_intersectSphere(frustum, center, radius);
}

inline dsIntersectResult dsFrustum3_intersectSphere(const dsFrustum3d* frustum,
	const dsVector3d* center, double radius)
{
	return dsFrustum3d_intersectSphere(frustum, center, radius);
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

inline void dsMatrix44_makeOrtho(dsMatrix44f* result, float left, float right,
	float bottom, float top, float near, float far, dsProjectionMatrixOptions options)
{
	dsMatrix44f_makeOrtho(result, left, right, bottom, top, near, far, options);
}

inline void dsMatrix44_makeOrtho(dsMatrix44d* result, double left, double right,
	double bottom, double top, double near, double far, dsProjectionMatrixOptions options)
{
	dsMatrix44d_makeOrtho(result, left, right, bottom, top, near, far, options);
}

inline void dsMatrix44_makePerspective(dsMatrix44f* result, float fovy, float aspect,
	float near, float far, dsProjectionMatrixOptions options)
{
	dsMatrix44f_makePerspective(result, fovy, aspect, near, far, options);
}

inline void dsMatrix44_makePerspective(dsMatrix44d* result, double fovy, double aspect,
	double near, double far, dsProjectionMatrixOptions options)
{
	dsMatrix44d_makePerspective(result, fovy, aspect, near, far, options);
}

inline void dsMatrix33_makeRotate3D(dsMatrix33f* result, float x, float y, float z)
{
	dsMatrix33f_makeRotate3D(result, x, y, z);
}

inline void dsMatrix33_makeRotate3D(dsMatrix33d* result, double x, double y, double z)
{
	dsMatrix33d_makeRotate3D(result, x, y, z);
}

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

TYPED_TEST(Frustum3Test, FromOrtho)
{
	typedef typename Frustum3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename Frustum3TypeSelector<TypeParam>::Frustum3Type Frustum3Type;
	TypeParam epsilon = Frustum3TypeSelector<TypeParam>::epsilon;

	dsProjectionMatrixOptions options = dsProjectionMatrixOptions_HalfZRange;
	Matrix44Type matrix;
	dsMatrix44_makeOrtho(&matrix, -2, 3, -4, 5, -6, 7, options);

	Frustum3Type frustum;
	dsFrustum3_fromMatrix(frustum, matrix, options);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Left,
		frustum.planes + dsFrustumPlanes_Left);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Left].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.z, epsilon);
	EXPECT_NEAR(2, frustum.planes[dsFrustumPlanes_Left].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Right,
		frustum.planes + dsFrustumPlanes_Right);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Right].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.z, epsilon);
	EXPECT_NEAR(3, frustum.planes[dsFrustumPlanes_Right].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Bottom,
		frustum.planes + dsFrustumPlanes_Bottom);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.x, epsilon);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Bottom].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.z, epsilon);
	EXPECT_NEAR(4, frustum.planes[dsFrustumPlanes_Bottom].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Top,
		frustum.planes + dsFrustumPlanes_Top);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.x, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Top].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.z, epsilon);
	EXPECT_NEAR(5, frustum.planes[dsFrustumPlanes_Top].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Near,
		frustum.planes + dsFrustumPlanes_Near);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.y, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].n.z, epsilon);
	EXPECT_NEAR(6, frustum.planes[dsFrustumPlanes_Near].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Far,
		frustum.planes + dsFrustumPlanes_Far);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.y, epsilon);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Far].n.z, epsilon);
	EXPECT_NEAR(7, frustum.planes[dsFrustumPlanes_Far].d, epsilon);

	options = dsProjectionMatrixOptions_None;
	dsMatrix44_makeOrtho(&matrix, -2, 3, -4, 5, -6, 7, options);
	dsFrustum3_fromMatrix(frustum, matrix, options);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Left,
		frustum.planes + dsFrustumPlanes_Left);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Left].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.z, epsilon);
	EXPECT_NEAR(2, frustum.planes[dsFrustumPlanes_Left].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Right,
		frustum.planes + dsFrustumPlanes_Right);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Right].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.z, epsilon);
	EXPECT_NEAR(3, frustum.planes[dsFrustumPlanes_Right].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Bottom,
		frustum.planes + dsFrustumPlanes_Bottom);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.x, epsilon);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Bottom].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.z, epsilon);
	EXPECT_NEAR(4, frustum.planes[dsFrustumPlanes_Bottom].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Top,
		frustum.planes + dsFrustumPlanes_Top);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.x, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Top].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.z, epsilon);
	EXPECT_NEAR(5, frustum.planes[dsFrustumPlanes_Top].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Near,
		frustum.planes + dsFrustumPlanes_Near);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.y, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].n.z, epsilon);
	EXPECT_NEAR(6, frustum.planes[dsFrustumPlanes_Near].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Far,
		frustum.planes + dsFrustumPlanes_Far);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.y, epsilon);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Far].n.z, epsilon);
	EXPECT_NEAR(7, frustum.planes[dsFrustumPlanes_Far].d, epsilon);

	options = dsProjectionMatrixOptions_HalfZRange | dsProjectionMatrixOptions_InvertY;
	dsMatrix44_makeOrtho(&matrix, -2, 3, -4, 5, -6, 7, options);
	dsFrustum3_fromMatrix(frustum, matrix, options);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Left,
		frustum.planes + dsFrustumPlanes_Left);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Left].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.z, epsilon);
	EXPECT_NEAR(2, frustum.planes[dsFrustumPlanes_Left].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Right,
		frustum.planes + dsFrustumPlanes_Right);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Right].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.z, epsilon);
	EXPECT_NEAR(3, frustum.planes[dsFrustumPlanes_Right].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Bottom,
		frustum.planes + dsFrustumPlanes_Bottom);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.x, epsilon);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Bottom].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.z, epsilon);
	EXPECT_NEAR(4, frustum.planes[dsFrustumPlanes_Bottom].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Top,
		frustum.planes + dsFrustumPlanes_Top);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.x, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Top].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.z, epsilon);
	EXPECT_NEAR(5, frustum.planes[dsFrustumPlanes_Top].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Near,
		frustum.planes + dsFrustumPlanes_Near);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.y, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].n.z, epsilon);
	EXPECT_NEAR(6, frustum.planes[dsFrustumPlanes_Near].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Far,
		frustum.planes + dsFrustumPlanes_Far);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.y, epsilon);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Far].n.z, epsilon);
	EXPECT_NEAR(7, frustum.planes[dsFrustumPlanes_Far].d, epsilon);

	options = dsProjectionMatrixOptions_InvertY;
	dsMatrix44_makeOrtho(&matrix, -2, 3, -4, 5, -6, 7, options);
	dsFrustum3_fromMatrix(frustum, matrix, options);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Left,
		frustum.planes + dsFrustumPlanes_Left);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Left].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.z, epsilon);
	EXPECT_NEAR(2, frustum.planes[dsFrustumPlanes_Left].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Right,
		frustum.planes + dsFrustumPlanes_Right);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Right].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.z, epsilon);
	EXPECT_NEAR(3, frustum.planes[dsFrustumPlanes_Right].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Bottom,
		frustum.planes + dsFrustumPlanes_Bottom);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.x, epsilon);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Bottom].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.z, epsilon);
	EXPECT_NEAR(4, frustum.planes[dsFrustumPlanes_Bottom].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Top,
		frustum.planes + dsFrustumPlanes_Top);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.x, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Top].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.z, epsilon);
	EXPECT_NEAR(5, frustum.planes[dsFrustumPlanes_Top].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Near,
		frustum.planes + dsFrustumPlanes_Near);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.y, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].n.z, epsilon);
	EXPECT_NEAR(6, frustum.planes[dsFrustumPlanes_Near].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Far,
		frustum.planes + dsFrustumPlanes_Far);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.y, epsilon);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Far].n.z, epsilon);
	EXPECT_NEAR(7, frustum.planes[dsFrustumPlanes_Far].d, epsilon);

	options = dsProjectionMatrixOptions_HalfZRange | dsProjectionMatrixOptions_InvertZ;
	dsMatrix44_makeOrtho(&matrix, -2, 3, -4, 5, -6, 7, options);
	dsFrustum3_fromMatrix(frustum, matrix, options);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Left,
		frustum.planes + dsFrustumPlanes_Left);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Left].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.z, epsilon);
	EXPECT_NEAR(2, frustum.planes[dsFrustumPlanes_Left].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Right,
		frustum.planes + dsFrustumPlanes_Right);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Right].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.z, epsilon);
	EXPECT_NEAR(3, frustum.planes[dsFrustumPlanes_Right].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Bottom,
		frustum.planes + dsFrustumPlanes_Bottom);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.x, epsilon);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Bottom].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.z, epsilon);
	EXPECT_NEAR(4, frustum.planes[dsFrustumPlanes_Bottom].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Top,
		frustum.planes + dsFrustumPlanes_Top);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.x, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Top].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.z, epsilon);
	EXPECT_NEAR(5, frustum.planes[dsFrustumPlanes_Top].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Near,
		frustum.planes + dsFrustumPlanes_Near);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.y, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].n.z, epsilon);
	EXPECT_NEAR(6, frustum.planes[dsFrustumPlanes_Near].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Far,
		frustum.planes + dsFrustumPlanes_Far);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.y, epsilon);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Far].n.z, epsilon);
	EXPECT_NEAR(7, frustum.planes[dsFrustumPlanes_Far].d, epsilon);
}

TYPED_TEST(Frustum3Test, FromPerspective)
{
	typedef typename Frustum3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename Frustum3TypeSelector<TypeParam>::Frustum3Type Frustum3Type;
	TypeParam epsilon = Frustum3TypeSelector<TypeParam>::epsilon;

	TypeParam fovY = (TypeParam)dsDegreesToRadiansd(30);
	TypeParam aspect = (TypeParam)1.5;
	TypeParam halfFovX = std::atan(std::tan(fovY/2)*aspect);
	dsProjectionMatrixOptions options = dsProjectionMatrixOptions_HalfZRange;
	Matrix44Type matrix;
	dsMatrix44_makePerspective(&matrix, fovY, aspect, 1, 7, options);

	Frustum3Type frustum;
	dsFrustum3_fromMatrix(frustum, matrix, options);

	TypeParam horizNormX = std::cos(halfFovX);
	TypeParam horizNormY = std::sin(halfFovX);
	TypeParam vertNormX = std::cos(fovY/2);
	TypeParam vertNormY = std::sin(fovY/2);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Left,
		frustum.planes + dsFrustumPlanes_Left);
	EXPECT_NEAR(horizNormX, frustum.planes[dsFrustumPlanes_Left].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Left].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Right,
		frustum.planes + dsFrustumPlanes_Right);
	EXPECT_NEAR(-horizNormX, frustum.planes[dsFrustumPlanes_Right].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Right].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Bottom,
		frustum.planes + dsFrustumPlanes_Bottom);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.x, epsilon);
	EXPECT_NEAR(vertNormX, frustum.planes[dsFrustumPlanes_Bottom].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Bottom].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Top,
		frustum.planes + dsFrustumPlanes_Top);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.x, epsilon);
	EXPECT_NEAR(-vertNormX, frustum.planes[dsFrustumPlanes_Top].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Top].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Near,
		frustum.planes + dsFrustumPlanes_Near);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.y, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].n.z, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Far,
		frustum.planes + dsFrustumPlanes_Far);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.y, epsilon);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Far].n.z, epsilon);
	EXPECT_NEAR(7, frustum.planes[dsFrustumPlanes_Far].d, epsilon);

	options = dsProjectionMatrixOptions_None;
	dsMatrix44_makePerspective(&matrix, fovY, aspect, 1, 7, options);
	dsFrustum3_fromMatrix(frustum, matrix, options);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Left,
		frustum.planes + dsFrustumPlanes_Left);
	EXPECT_NEAR(horizNormX, frustum.planes[dsFrustumPlanes_Left].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Left].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Right,
		frustum.planes + dsFrustumPlanes_Right);
	EXPECT_NEAR(-horizNormX, frustum.planes[dsFrustumPlanes_Right].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Right].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Bottom,
		frustum.planes + dsFrustumPlanes_Bottom);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.x, epsilon);
	EXPECT_NEAR(vertNormX, frustum.planes[dsFrustumPlanes_Bottom].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Bottom].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Top,
		frustum.planes + dsFrustumPlanes_Top);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.x, epsilon);
	EXPECT_NEAR(-vertNormX, frustum.planes[dsFrustumPlanes_Top].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Top].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Near,
		frustum.planes + dsFrustumPlanes_Near);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.y, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].n.z, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Far,
		frustum.planes + dsFrustumPlanes_Far);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.y, epsilon);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Far].n.z, epsilon);
	EXPECT_NEAR(7, frustum.planes[dsFrustumPlanes_Far].d, epsilon);

	options = dsProjectionMatrixOptions_HalfZRange | dsProjectionMatrixOptions_InvertY;
	dsMatrix44_makePerspective(&matrix, fovY, aspect, 1, 7, options);
	dsFrustum3_fromMatrix(frustum, matrix, options);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Left,
		frustum.planes + dsFrustumPlanes_Left);
	EXPECT_NEAR(horizNormX, frustum.planes[dsFrustumPlanes_Left].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Left].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Right,
		frustum.planes + dsFrustumPlanes_Right);
	EXPECT_NEAR(-horizNormX, frustum.planes[dsFrustumPlanes_Right].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Right].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Bottom,
		frustum.planes + dsFrustumPlanes_Bottom);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.x, epsilon);
	EXPECT_NEAR(vertNormX, frustum.planes[dsFrustumPlanes_Bottom].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Bottom].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Top,
		frustum.planes + dsFrustumPlanes_Top);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.x, epsilon);
	EXPECT_NEAR(-vertNormX, frustum.planes[dsFrustumPlanes_Top].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Top].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Near,
		frustum.planes + dsFrustumPlanes_Near);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.y, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].n.z, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Far,
		frustum.planes + dsFrustumPlanes_Far);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.y, epsilon);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Far].n.z, epsilon);
	EXPECT_NEAR(7, frustum.planes[dsFrustumPlanes_Far].d, epsilon);

	options = dsProjectionMatrixOptions_InvertY;
	dsMatrix44_makePerspective(&matrix, fovY, aspect, 1, 7, options);
	dsFrustum3_fromMatrix(frustum, matrix, options);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Left,
		frustum.planes + dsFrustumPlanes_Left);
	EXPECT_NEAR(horizNormX, frustum.planes[dsFrustumPlanes_Left].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Left].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Right,
		frustum.planes + dsFrustumPlanes_Right);
	EXPECT_NEAR(-horizNormX, frustum.planes[dsFrustumPlanes_Right].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Right].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Bottom,
		frustum.planes + dsFrustumPlanes_Bottom);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.x, epsilon);
	EXPECT_NEAR(vertNormX, frustum.planes[dsFrustumPlanes_Bottom].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Bottom].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Top,
		frustum.planes + dsFrustumPlanes_Top);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.x, epsilon);
	EXPECT_NEAR(-vertNormX, frustum.planes[dsFrustumPlanes_Top].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Top].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Near,
		frustum.planes + dsFrustumPlanes_Near);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.y, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].n.z, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Far,
		frustum.planes + dsFrustumPlanes_Far);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.y, epsilon);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Far].n.z, epsilon);
	EXPECT_NEAR(7, frustum.planes[dsFrustumPlanes_Far].d, epsilon);

	options = dsProjectionMatrixOptions_HalfZRange | dsProjectionMatrixOptions_InvertZ;
	dsMatrix44_makePerspective(&matrix, fovY, aspect, 1, 7, options);
	dsFrustum3_fromMatrix(frustum, matrix, options);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Left,
		frustum.planes + dsFrustumPlanes_Left);
	EXPECT_NEAR(horizNormX, frustum.planes[dsFrustumPlanes_Left].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Left].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Right,
		frustum.planes + dsFrustumPlanes_Right);
	EXPECT_NEAR(-horizNormX, frustum.planes[dsFrustumPlanes_Right].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Right].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Bottom,
		frustum.planes + dsFrustumPlanes_Bottom);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.x, epsilon);
	EXPECT_NEAR(vertNormX, frustum.planes[dsFrustumPlanes_Bottom].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Bottom].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Top,
		frustum.planes + dsFrustumPlanes_Top);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.x, epsilon);
	EXPECT_NEAR(-vertNormX, frustum.planes[dsFrustumPlanes_Top].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Top].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Near,
		frustum.planes + dsFrustumPlanes_Near);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.y, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].n.z, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Far,
		frustum.planes + dsFrustumPlanes_Far);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.y, epsilon);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Far].n.z, epsilon);
	EXPECT_NEAR(7, frustum.planes[dsFrustumPlanes_Far].d, epsilon);

	options = dsProjectionMatrixOptions_InvertZ;
	dsMatrix44_makePerspective(&matrix, fovY, aspect, 1, 7, options);
	dsFrustum3_fromMatrix(frustum, matrix, options);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Left,
		frustum.planes + dsFrustumPlanes_Left);
	EXPECT_NEAR(horizNormX, frustum.planes[dsFrustumPlanes_Left].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Left].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Right,
		frustum.planes + dsFrustumPlanes_Right);
	EXPECT_NEAR(-horizNormX, frustum.planes[dsFrustumPlanes_Right].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Right].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Bottom,
		frustum.planes + dsFrustumPlanes_Bottom);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.x, epsilon);
	EXPECT_NEAR(vertNormX, frustum.planes[dsFrustumPlanes_Bottom].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Bottom].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Top,
		frustum.planes + dsFrustumPlanes_Top);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.x, epsilon);
	EXPECT_NEAR(-vertNormX, frustum.planes[dsFrustumPlanes_Top].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Top].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Near,
		frustum.planes + dsFrustumPlanes_Near);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.y, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].n.z, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Far,
		frustum.planes + dsFrustumPlanes_Far);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.y, epsilon);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Far].n.z, epsilon);
	EXPECT_NEAR(7, frustum.planes[dsFrustumPlanes_Far].d, epsilon);
}

TYPED_TEST(Frustum3Test, FromInfinitPerspective)
{
	typedef typename Frustum3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename Frustum3TypeSelector<TypeParam>::Frustum3Type Frustum3Type;
	TypeParam epsilon = Frustum3TypeSelector<TypeParam>::epsilon;

	TypeParam fovY = (TypeParam)dsDegreesToRadiansd(30);
	TypeParam aspect = (TypeParam)1.5;
	TypeParam halfFovX = std::atan(std::tan(fovY/2)*aspect);
	dsProjectionMatrixOptions options = dsProjectionMatrixOptions_HalfZRange;
	Matrix44Type matrix;
	dsMatrix44_makePerspective(&matrix, fovY, aspect, 1, INFINITY, options);

	Frustum3Type frustum;
	dsFrustum3_fromMatrix(frustum, matrix, options);

	TypeParam horizNormX = std::cos(halfFovX);
	TypeParam horizNormY = std::sin(halfFovX);
	TypeParam vertNormX = std::cos(fovY/2);
	TypeParam vertNormY = std::sin(fovY/2);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Left,
		frustum.planes + dsFrustumPlanes_Left);
	EXPECT_NEAR(horizNormX, frustum.planes[dsFrustumPlanes_Left].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Left].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Right,
		frustum.planes + dsFrustumPlanes_Right);
	EXPECT_NEAR(-horizNormX, frustum.planes[dsFrustumPlanes_Right].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Right].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Bottom,
		frustum.planes + dsFrustumPlanes_Bottom);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.x, epsilon);
	EXPECT_NEAR(vertNormX, frustum.planes[dsFrustumPlanes_Bottom].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Bottom].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Top,
		frustum.planes + dsFrustumPlanes_Top);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.x, epsilon);
	EXPECT_NEAR(-vertNormX, frustum.planes[dsFrustumPlanes_Top].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Top].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Near,
		frustum.planes + dsFrustumPlanes_Near);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.y, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].n.z, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].d, epsilon);

	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.z, epsilon);

	options = dsProjectionMatrixOptions_None;
	dsMatrix44_makePerspective(&matrix, fovY, aspect, 1, INFINITY, options);
	dsFrustum3_fromMatrix(frustum, matrix, options);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Left,
		frustum.planes + dsFrustumPlanes_Left);
	EXPECT_NEAR(horizNormX, frustum.planes[dsFrustumPlanes_Left].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Left].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Right,
		frustum.planes + dsFrustumPlanes_Right);
	EXPECT_NEAR(-horizNormX, frustum.planes[dsFrustumPlanes_Right].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Right].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Bottom,
		frustum.planes + dsFrustumPlanes_Bottom);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.x, epsilon);
	EXPECT_NEAR(vertNormX, frustum.planes[dsFrustumPlanes_Bottom].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Bottom].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Top,
		frustum.planes + dsFrustumPlanes_Top);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.x, epsilon);
	EXPECT_NEAR(-vertNormX, frustum.planes[dsFrustumPlanes_Top].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Top].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Near,
		frustum.planes + dsFrustumPlanes_Near);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.y, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].n.z, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].d, epsilon);

	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.z, epsilon);

	options = dsProjectionMatrixOptions_HalfZRange | dsProjectionMatrixOptions_InvertY;
	dsMatrix44_makePerspective(&matrix, fovY, aspect, 1, INFINITY, options);
	dsFrustum3_fromMatrix(frustum, matrix, options);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Left,
		frustum.planes + dsFrustumPlanes_Left);
	EXPECT_NEAR(horizNormX, frustum.planes[dsFrustumPlanes_Left].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Left].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Right,
		frustum.planes + dsFrustumPlanes_Right);
	EXPECT_NEAR(-horizNormX, frustum.planes[dsFrustumPlanes_Right].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Right].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Bottom,
		frustum.planes + dsFrustumPlanes_Bottom);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.x, epsilon);
	EXPECT_NEAR(vertNormX, frustum.planes[dsFrustumPlanes_Bottom].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Bottom].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Top,
		frustum.planes + dsFrustumPlanes_Top);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.x, epsilon);
	EXPECT_NEAR(-vertNormX, frustum.planes[dsFrustumPlanes_Top].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Top].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Near,
		frustum.planes + dsFrustumPlanes_Near);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.y, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].n.z, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].d, epsilon);

	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.z, epsilon);

	options = dsProjectionMatrixOptions_InvertY;
	dsMatrix44_makePerspective(&matrix, fovY, aspect, 1, INFINITY, options);
	dsFrustum3_fromMatrix(frustum, matrix, options);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Left,
		frustum.planes + dsFrustumPlanes_Left);
	EXPECT_NEAR(horizNormX, frustum.planes[dsFrustumPlanes_Left].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Left].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Right,
		frustum.planes + dsFrustumPlanes_Right);
	EXPECT_NEAR(-horizNormX, frustum.planes[dsFrustumPlanes_Right].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Right].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Bottom,
		frustum.planes + dsFrustumPlanes_Bottom);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.x, epsilon);
	EXPECT_NEAR(vertNormX, frustum.planes[dsFrustumPlanes_Bottom].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Bottom].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Top,
		frustum.planes + dsFrustumPlanes_Top);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.x, epsilon);
	EXPECT_NEAR(-vertNormX, frustum.planes[dsFrustumPlanes_Top].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Top].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Near,
		frustum.planes + dsFrustumPlanes_Near);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.y, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].n.z, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].d, epsilon);

	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.z, epsilon);

	options = dsProjectionMatrixOptions_HalfZRange | dsProjectionMatrixOptions_InvertZ;
	dsMatrix44_makePerspective(&matrix, fovY, aspect, 1, INFINITY, options);
	dsFrustum3_fromMatrix(frustum, matrix, options);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Left,
		frustum.planes + dsFrustumPlanes_Left);
	EXPECT_NEAR(horizNormX, frustum.planes[dsFrustumPlanes_Left].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Left].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Right,
		frustum.planes + dsFrustumPlanes_Right);
	EXPECT_NEAR(-horizNormX, frustum.planes[dsFrustumPlanes_Right].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Right].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Bottom,
		frustum.planes + dsFrustumPlanes_Bottom);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.x, epsilon);
	EXPECT_NEAR(vertNormX, frustum.planes[dsFrustumPlanes_Bottom].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Bottom].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Top,
		frustum.planes + dsFrustumPlanes_Top);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.x, epsilon);
	EXPECT_NEAR(-vertNormX, frustum.planes[dsFrustumPlanes_Top].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Top].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Near,
		frustum.planes + dsFrustumPlanes_Near);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.y, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].n.z, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].d, epsilon);

	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.z, epsilon);

	options = dsProjectionMatrixOptions_InvertZ;
	dsMatrix44_makePerspective(&matrix, fovY, aspect, 1, INFINITY, options);
	dsFrustum3_fromMatrix(frustum, matrix, options);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Left,
		frustum.planes + dsFrustumPlanes_Left);
	EXPECT_NEAR(horizNormX, frustum.planes[dsFrustumPlanes_Left].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Left].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Right,
		frustum.planes + dsFrustumPlanes_Right);
	EXPECT_NEAR(-horizNormX, frustum.planes[dsFrustumPlanes_Right].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.y, epsilon);
	EXPECT_NEAR(-horizNormY, frustum.planes[dsFrustumPlanes_Right].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Bottom,
		frustum.planes + dsFrustumPlanes_Bottom);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.x, epsilon);
	EXPECT_NEAR(vertNormX, frustum.planes[dsFrustumPlanes_Bottom].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Bottom].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Top,
		frustum.planes + dsFrustumPlanes_Top);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.x, epsilon);
	EXPECT_NEAR(-vertNormX, frustum.planes[dsFrustumPlanes_Top].n.y, epsilon);
	EXPECT_NEAR(-vertNormY, frustum.planes[dsFrustumPlanes_Top].n.z, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].d, epsilon);

	dsPlane3_normalize(frustum.planes + dsFrustumPlanes_Near,
		frustum.planes + dsFrustumPlanes_Near);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.y, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].n.z, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].d, epsilon);

	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.z, epsilon);
}

TYPED_TEST(Frustum3Test, Normalize)
{
	typedef typename Frustum3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename Frustum3TypeSelector<TypeParam>::Frustum3Type Frustum3Type;
	TypeParam epsilon = Frustum3TypeSelector<TypeParam>::epsilon;

	Matrix44Type matrix;
	dsMatrix44_makeOrtho(&matrix, -2, 3, -4, 5, -6, 7, dsProjectionMatrixOptions_None);

	Frustum3Type frustum;
	dsFrustum3_fromMatrix(frustum, matrix, dsProjectionMatrixOptions_None);
	dsFrustum3_normalize(&frustum);

	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Left].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Left].n.z, epsilon);
	EXPECT_NEAR(2, frustum.planes[dsFrustumPlanes_Left].d, epsilon);

	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Right].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Right].n.z, epsilon);
	EXPECT_NEAR(3, frustum.planes[dsFrustumPlanes_Right].d, epsilon);

	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.x, epsilon);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Bottom].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Bottom].n.z, epsilon);
	EXPECT_NEAR(4, frustum.planes[dsFrustumPlanes_Bottom].d, epsilon);

	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.x, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Top].n.y, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Top].n.z, epsilon);
	EXPECT_NEAR(5, frustum.planes[dsFrustumPlanes_Top].d, epsilon);

	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Near].n.y, epsilon);
	EXPECT_NEAR(-1, frustum.planes[dsFrustumPlanes_Near].n.z, epsilon);
	EXPECT_NEAR(6, frustum.planes[dsFrustumPlanes_Near].d, epsilon);

	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.x, epsilon);
	EXPECT_NEAR(0, frustum.planes[dsFrustumPlanes_Far].n.y, epsilon);
	EXPECT_NEAR(1, frustum.planes[dsFrustumPlanes_Far].n.z, epsilon);
	EXPECT_NEAR(7, frustum.planes[dsFrustumPlanes_Far].d, epsilon);
}

TYPED_TEST(Frustum3Test, Transform)
{
	typedef typename Frustum3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename Frustum3TypeSelector<TypeParam>::Plane3Type Plane3Type;
	typedef typename Frustum3TypeSelector<TypeParam>::Frustum3Type Frustum3Type;
	TypeParam epsilon = Frustum3TypeSelector<TypeParam>::epsilon;

	Matrix44Type matrix;
	dsMatrix44_makeOrtho(&matrix, -2, 3, -4, 5, -6, 7, dsProjectionMatrixOptions_None);

	Frustum3Type frustum;
	dsFrustum3_fromMatrix(frustum, matrix, dsProjectionMatrixOptions_None);

	Matrix44Type rotate, translate, transform;
	dsMatrix44_makeRotate(&rotate, (TypeParam)dsDegreesToRadiansd(30),
		(TypeParam)dsDegreesToRadiansd(-15), (TypeParam)dsDegreesToRadiansd(60));
	dsMatrix44_makeTranslate(&translate, -3, 5, -1);

	dsMatrix44_mul(transform, translate, rotate);

	Frustum3Type frustumCopy = frustum;
	dsFrustum3_transform(&frustum, &transform);

	for (int i = 0; i < dsFrustumPlanes_Count; ++i)
	{
		Plane3Type transformedPlane;
		dsPlane3_transform(&transformedPlane, &transform, frustumCopy.planes + i);

		EXPECT_NEAR(transformedPlane.n.x, frustum.planes[i].n.x, epsilon);
		EXPECT_NEAR(transformedPlane.n.y, frustum.planes[i].n.y, epsilon);
		EXPECT_NEAR(transformedPlane.n.z, frustum.planes[i].n.z, epsilon);
		EXPECT_NEAR(transformedPlane.d, frustum.planes[i].d, epsilon);
	}
}

TYPED_TEST(Frustum3Test, TransformInverseTranspose)
{
	typedef typename Frustum3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename Frustum3TypeSelector<TypeParam>::Plane3Type Plane3Type;
	typedef typename Frustum3TypeSelector<TypeParam>::Frustum3Type Frustum3Type;
	TypeParam epsilon = Frustum3TypeSelector<TypeParam>::epsilon;

	Matrix44Type matrix;
	dsMatrix44_makeOrtho(&matrix, -2, 3, -4, 5, -6, 7, dsProjectionMatrixOptions_None);

	Frustum3Type frustum;
	dsFrustum3_fromMatrix(frustum, matrix, dsProjectionMatrixOptions_None);

	Matrix44Type rotate, translate, transform, inverse, inverseTranspose;
	dsMatrix44_makeRotate(&rotate, (TypeParam)dsDegreesToRadiansd(30),
		(TypeParam)dsDegreesToRadiansd(-15), (TypeParam)dsDegreesToRadiansd(60));
	dsMatrix44_makeTranslate(&translate, -3, 5, -1);

	dsMatrix44_mul(transform, translate, rotate);
	dsMatrix44_affineInvert(&inverse, &transform);
	dsMatrix44_transpose(inverseTranspose, inverse);

	Frustum3Type frustumCopy = frustum;
	dsFrustum3_transformInverseTranspose(&frustum, &inverseTranspose);

	for (int i = 0; i < dsFrustumPlanes_Count; ++i)
	{
		Plane3Type transformedPlane;
		dsPlane3_transformInverseTranspose(&transformedPlane, &inverseTranspose,
			frustumCopy.planes + i);

		EXPECT_NEAR(transformedPlane.n.x, frustum.planes[i].n.x, epsilon);
		EXPECT_NEAR(transformedPlane.n.y, frustum.planes[i].n.y, epsilon);
		EXPECT_NEAR(transformedPlane.n.z, frustum.planes[i].n.z, epsilon);
		EXPECT_NEAR(transformedPlane.d, frustum.planes[i].d, epsilon);
	}
}

TYPED_TEST(Frustum3Test, IsInfinite)
{
	typedef typename Frustum3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename Frustum3TypeSelector<TypeParam>::Frustum3Type Frustum3Type;

	TypeParam fovY = (TypeParam)dsDegreesToRadiansd(30);
	TypeParam aspect = (TypeParam)1.5;
	Matrix44Type matrix;
	dsMatrix44_makePerspective(&matrix, fovY, aspect, 1, 10, dsProjectionMatrixOptions_None);

	Frustum3Type frustum;
	dsFrustum3_fromMatrix(frustum, matrix, dsProjectionMatrixOptions_None);
	EXPECT_FALSE(dsFrustum3_isInfinite(&frustum));

	dsMatrix44_makePerspective(&matrix, fovY, aspect, 1, INFINITY, dsProjectionMatrixOptions_None);
	dsFrustum3_fromMatrix(frustum, matrix, dsProjectionMatrixOptions_None);
	EXPECT_TRUE(dsFrustum3_isInfinite(&frustum));

	Matrix44Type rotate, translate, transform;
	dsMatrix44_makeRotate(&rotate, (TypeParam)dsDegreesToRadiansd(30),
		(TypeParam)dsDegreesToRadiansd(-15), (TypeParam)dsDegreesToRadiansd(60));
	dsMatrix44_makeTranslate(&translate, -3, 5, -1);

	Matrix44Type transformedProjection;
	dsMatrix44_mul(transform, translate, rotate);
	dsMatrix44_mul(transformedProjection, matrix, transform);
	dsFrustum3_fromMatrix(frustum, transformedProjection, dsProjectionMatrixOptions_None);
	EXPECT_TRUE(dsFrustum3_isInfinite(&frustum));
}

TYPED_TEST(Frustum3Test, IntersectAlignedBox)
{
	typedef typename Frustum3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename Frustum3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;
	typedef typename Frustum3TypeSelector<TypeParam>::Frustum3Type Frustum3Type;

	// NOTE: Z is inverted for ortho matrices.
	Matrix44Type matrix;
	dsMatrix44_makeOrtho(&matrix, -2, 3, -4, 5, -6, 7, dsProjectionMatrixOptions_None);

	Frustum3Type frustum;
	dsFrustum3_fromMatrix(frustum, matrix, dsProjectionMatrixOptions_None);

	AlignedBox3Type box = {{{0, 1, 2}}, {{2, 3, 4}}};

	EXPECT_EQ(dsIntersectResult_Inside, dsFrustum3_intersectAlignedBox(&frustum, &box));

	// Intersect
	box.min.x = -3;
	box.max.x = -1;
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectAlignedBox(&frustum, &box));

	box.min.x = 2;
	box.max.x = 4;
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectAlignedBox(&frustum, &box));

	box.min.x = 0;
	box.max.x = 2;
	box.min.y = -5;
	box.max.y = -3;
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectAlignedBox(&frustum, &box));

	box.min.y = 4;
	box.max.y = 6;
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectAlignedBox(&frustum, &box));

	box.min.y = 1;
	box.max.y = 3;
	box.min.z = -8;
	box.max.z = -6;
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectAlignedBox(&frustum, &box));

	box.min.z = 5;
	box.max.z = 7;
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectAlignedBox(&frustum, &box));

	// Outside
	box.min.z = 2;
	box.max.z = 4;
	box.min.x = -5;
	box.max.x = -3;
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectAlignedBox(&frustum, &box));

	box.min.x = 4;
	box.max.x = 6;
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectAlignedBox(&frustum, &box));

	box.min.x = 0;
	box.max.x = 2;
	box.min.y = -7;
	box.max.y = -5;
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectAlignedBox(&frustum, &box));

	box.min.y = 6;
	box.max.y = 8;
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectAlignedBox(&frustum, &box));

	box.min.y = 1;
	box.max.y = 3;
	box.min.z = -10;
	box.max.z = -8;
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectAlignedBox(&frustum, &box));

	box.min.z = 7;
	box.max.z = 9;
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectAlignedBox(&frustum, &box));

	// Surrounding
	box.min.x = -5;
	box.max.x = 6;
	box.min.y = -7;
	box.max.y = 8;
	box.min.z = -10;
	box.max.z = 9;
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectAlignedBox(&frustum, &box));
}

TYPED_TEST(Frustum3Test, IntersectedOrientedBox)
{
	typedef typename Frustum3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename Frustum3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;
	typedef typename Frustum3TypeSelector<TypeParam>::OrientedBox3Type OrientedBox3Type;
	typedef typename Frustum3TypeSelector<TypeParam>::Frustum3Type Frustum3Type;

	// NOTE: Z is inverted for ortho matrices.
	Matrix44Type matrix;
	dsMatrix44_makeOrtho(&matrix, -2, 3, -4, 5, -6, 7, dsProjectionMatrixOptions_None);

	Frustum3Type frustum;
	dsFrustum3_fromMatrix(frustum, matrix, dsProjectionMatrixOptions_None);

	AlignedBox3Type alignedBox = {{{0, 1, 2}}, {{1, (TypeParam)2.5, (TypeParam)3.5}}};
	OrientedBox3Type box;
	dsOrientedBox3_fromAlignedBox(box, alignedBox);
	dsMatrix33_makeRotate3D(&box.orientation, (TypeParam)dsDegreesToRadiansd(30),
		(TypeParam)dsDegreesToRadiansd(-15), (TypeParam)dsDegreesToRadiansd(60));

	EXPECT_EQ(dsIntersectResult_Inside, dsFrustum3_intersectOrientedBox(&frustum, &box));

	// Intersect
	box.center.x = -2;
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectOrientedBox(&frustum, &box));

	box.center.y = 3;
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectOrientedBox(&frustum, &box));

	box.center.x = (TypeParam)0.5;
	box.center.y = -4;
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectOrientedBox(&frustum, &box));

	box.center.y = 5;
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectOrientedBox(&frustum, &box));

	box.center.y = (TypeParam)1.75;
	box.center.z = -7;
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectOrientedBox(&frustum, &box));

	box.center.z = 6;
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectOrientedBox(&frustum, &box));

	// Outside
	box.center.z = (TypeParam)2.75;
	box.center.x = -4;
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectOrientedBox(&frustum, &box));

	box.center.x = 5;
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectOrientedBox(&frustum, &box));

	box.center.x = (TypeParam)0.5;
	box.center.y = -6;
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectOrientedBox(&frustum, &box));

	box.center.y = 7;
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectOrientedBox(&frustum, &box));

	box.center.y = (TypeParam)1.75;
	box.center.z = -9;
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectOrientedBox(&frustum, &box));

	box.center.z = 8;
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectOrientedBox(&frustum, &box));

	// Surrounding
	box.center.z = (TypeParam)2.75;
	box.halfExtents.x = 7;
	box.halfExtents.y = 11;
	box.halfExtents.z = 15;
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectOrientedBox(&frustum, &box));
}

TYPED_TEST(Frustum3Test, IntersectedBoxMatrix)
{
	typedef typename Frustum3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename Frustum3TypeSelector<TypeParam>::AlignedBox3Type AlignedBox3Type;
	typedef typename Frustum3TypeSelector<TypeParam>::OrientedBox3Type OrientedBox3Type;
	typedef typename Frustum3TypeSelector<TypeParam>::Frustum3Type Frustum3Type;

	// NOTE: Z is inverted for ortho matrices.
	Matrix44Type matrix;
	dsMatrix44_makeOrtho(&matrix, -2, 3, -4, 5, -6, 7, dsProjectionMatrixOptions_None);

	Frustum3Type frustum;
	dsFrustum3_fromMatrix(frustum, matrix, dsProjectionMatrixOptions_None);

	AlignedBox3Type alignedBox = {{{0, 1, 2}}, {{1, (TypeParam)2.5, (TypeParam)3.5}}};
	OrientedBox3Type box;
	dsOrientedBox3_fromAlignedBox(box, alignedBox);
	dsMatrix33_makeRotate3D(&box.orientation, (TypeParam)dsDegreesToRadiansd(30),
		(TypeParam)dsDegreesToRadiansd(-15), (TypeParam)dsDegreesToRadiansd(60));

	Matrix44Type boxMatrix;
	dsOrientedBox3_toMatrix(boxMatrix, box);

	EXPECT_EQ(dsIntersectResult_Inside, dsFrustum3_intersectBoxMatrix(&frustum, &boxMatrix));

	// Intersect
	box.center.x = -2;
	dsOrientedBox3_toMatrix(boxMatrix, box);
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectBoxMatrix(&frustum, &boxMatrix));

	box.center.y = 3;
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectBoxMatrix(&frustum, &boxMatrix));

	box.center.x = (TypeParam)0.5;
	box.center.y = -4;
	dsOrientedBox3_toMatrix(boxMatrix, box);
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectBoxMatrix(&frustum, &boxMatrix));

	box.center.y = 5;
	dsOrientedBox3_toMatrix(boxMatrix, box);
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectBoxMatrix(&frustum, &boxMatrix));

	box.center.y = (TypeParam)1.75;
	box.center.z = -7;
	dsOrientedBox3_toMatrix(boxMatrix, box);
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectBoxMatrix(&frustum, &boxMatrix));

	box.center.z = 6;
	dsOrientedBox3_toMatrix(boxMatrix, box);
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectBoxMatrix(&frustum, &boxMatrix));

	// Outside
	box.center.z = (TypeParam)2.75;
	box.center.x = -4;
	dsOrientedBox3_toMatrix(boxMatrix, box);
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectBoxMatrix(&frustum, &boxMatrix));

	box.center.x = 5;
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectBoxMatrix(&frustum, &boxMatrix));

	box.center.x = (TypeParam)0.5;
	box.center.y = -6;
	dsOrientedBox3_toMatrix(boxMatrix, box);
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectBoxMatrix(&frustum, &boxMatrix));

	box.center.y = 7;
	dsOrientedBox3_toMatrix(boxMatrix, box);
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectBoxMatrix(&frustum, &boxMatrix));

	box.center.y = (TypeParam)1.75;
	box.center.z = -9;
	dsOrientedBox3_toMatrix(boxMatrix, box);
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectBoxMatrix(&frustum, &boxMatrix));

	box.center.z = 8;
	dsOrientedBox3_toMatrix(boxMatrix, box);
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectBoxMatrix(&frustum, &boxMatrix));

	// Surrounding
	box.center.z = (TypeParam)2.75;
	box.halfExtents.x = 7;
	box.halfExtents.y = 11;
	box.halfExtents.z = 15;
	dsOrientedBox3_toMatrix(boxMatrix, box);
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectBoxMatrix(&frustum, &boxMatrix));
}

TYPED_TEST(Frustum3Test, IntersectedSphere)
{
	typedef typename Frustum3TypeSelector<TypeParam>::Matrix44Type Matrix44Type;
	typedef typename Frustum3TypeSelector<TypeParam>::Vector3Type Vector3Type;
	typedef typename Frustum3TypeSelector<TypeParam>::Frustum3Type Frustum3Type;

	// NOTE: Z is inverted for ortho matrices.
	Matrix44Type matrix;
	dsMatrix44_makeOrtho(&matrix, -2, 3, -4, 5, -6, 7, dsProjectionMatrixOptions_None);

	Frustum3Type frustum;
	dsFrustum3_fromMatrix(frustum, matrix, dsProjectionMatrixOptions_None);
	dsFrustum3_normalize(&frustum);

	Vector3Type center = {{0, 1, 2}};
	TypeParam radius = 1;

	EXPECT_EQ(dsIntersectResult_Inside, dsFrustum3_intersectSphere(&frustum, &center, radius));

	// Intersect
	center.x = TypeParam(-2.5);
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectSphere(&frustum, &center, radius));

	center.x = TypeParam(3.5);
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectSphere(&frustum, &center, radius));

	center.x = 0;
	center.y = TypeParam(-4.5);
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectSphere(&frustum, &center, radius));

	center.y = TypeParam(5.5);
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectSphere(&frustum, &center, radius));

	center.y = 1;
	center.z = TypeParam(6.5);
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectSphere(&frustum, &center, radius));

	center.z = TypeParam(-7.5);
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectSphere(&frustum, &center, radius));

	// Outside
	center.z = 3;
	center.x = TypeParam(-3.5);
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectSphere(&frustum, &center, radius));

	center.x = TypeParam(4.5);
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectSphere(&frustum, &center, radius));

	center.x = 0;
	center.y = TypeParam(-5.5);
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectSphere(&frustum, &center, radius));

	center.y = TypeParam(6.5);
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectSphere(&frustum, &center, radius));

	center.y = 1;
	center.z = TypeParam(7.5);
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectSphere(&frustum, &center, radius));

	center.z = TypeParam(-8.5);
	EXPECT_EQ(dsIntersectResult_Outside, dsFrustum3_intersectSphere(&frustum, &center, radius));

	// Surrounding
	center.z = 3;
	radius = 20;
	EXPECT_EQ(dsIntersectResult_Intersects, dsFrustum3_intersectSphere(&frustum, &center, radius));
}

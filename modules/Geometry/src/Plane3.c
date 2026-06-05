/*
 * Copyright 2016-2026 Aaron Barany
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

#include <DeepSea/Core/Assert.h>

#include <DeepSea/Geometry/AlignedBox3x.h>
#include <DeepSea/Geometry/OrientedBox3x.h>

#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Sqrt.h>
#include <DeepSea/Math/Vector3x.h>

#include <float.h>

bool dsPlane3f_intersectingLine(
	dsRay3f* result, const dsPlane3f* firstPlane, const dsPlane3f* secondPlane)
{
	// http://geomalgorithms.com/a05-_intersect-1.html
	DS_ASSERT(result);
	DS_ASSERT(firstPlane);
	DS_ASSERT(secondPlane);

	const float epsilon2 = dsPow2(1e-6f);
	dsVector3xf_cross(&result->direction, &firstPlane->xyzd, &secondPlane->xyzd);
	float len2 = dsVector3xf_len2(&result->direction);
	if (len2 < epsilon2)
		return false;

	dsVector3xf scaledFirstN;
	dsVector3xf_scale(&scaledFirstN, &firstPlane->xyzd, secondPlane->d);

	dsVector3xf scaledSecondN;
	dsVector3xf_scale(&scaledSecondN, &secondPlane->xyzd, firstPlane->d);

	dsVector3xf diff;
	dsVector3xf_sub(&diff, &scaledFirstN, &scaledSecondN);

	float invLen2 = 1/len2;
	dsVector3xf_cross(&result->origin, &diff, &result->direction);
	dsVector3xf_scale(&result->origin, &result->origin, invLen2);

	float invLen = dsSqrtf(invLen2);
	dsVector3xf_scale(&result->direction, &result->direction, invLen);
	return true;
}

bool dsPlane3d_intersectingLine(
	dsRay3d* result, const dsPlane3d* firstPlane, const dsPlane3d* secondPlane)
{
	// http://geomalgorithms.com/a05-_intersect-1.html
	DS_ASSERT(result);
	DS_ASSERT(firstPlane);
	DS_ASSERT(secondPlane);

	const double epsilon2 = dsPow2(1e-14);
	dsVector3xd_cross(&result->direction, &firstPlane->xyzd, &secondPlane->xyzd);
	double len2 = dsVector3xd_len2(&result->direction);
	if (len2 < epsilon2)
		return false;

	dsVector3xd scaledFirstN;
	dsVector3xd_scale(&scaledFirstN, &firstPlane->xyzd, secondPlane->d);

	dsVector3xd scaledSecondN;
	dsVector3xd_scale(&scaledSecondN, &secondPlane->xyzd, firstPlane->d);

	dsVector3xd diff;
	dsVector3xd_sub(&diff, &scaledFirstN, &scaledSecondN);

	double invLen2 = 1/len2;
	dsVector3xd_cross(&result->origin, &diff, &result->direction);
	dsVector3xd_scale(&result->origin, &result->origin, invLen2);

	double invLen = dsSqrtd(invLen2);
	dsVector3xd_scale(&result->direction, &result->direction, invLen);
	return true;
}

bool dsPlane3f_intersectingPoint(dsVector3f* result, const dsPlane3f* firstPlane,
	const dsPlane3f* secondPlane, const dsPlane3f* thirdPlane)
{
	// http://geomalgorithms.com/a05-_intersect-1.html
	DS_ASSERT(result);
	DS_ASSERT(firstPlane);
	DS_ASSERT(secondPlane);
	DS_ASSERT(thirdPlane);

	const float epsilon2 = dsPow2(1e-6f);
	dsVector3xf crossSecondThird;
	dsVector3xf_cross(&crossSecondThird, &secondPlane->xyzd, &thirdPlane->xyzd);
	float denom = dsVector3xf_dot(&firstPlane->xyzd, &crossSecondThird);
	if (fabsf(denom) < epsilon2)
		return false;

	dsVector3xf_scale(&crossSecondThird, &crossSecondThird, -firstPlane->d);

	dsVector3xf crossThirdFirst;
	dsVector3xf_cross(&crossThirdFirst, &thirdPlane->xyzd, &firstPlane->xyzd);
	dsVector3xf_scale(&crossThirdFirst, &crossThirdFirst, -secondPlane->d);

	dsVector3xf crossFirstSecond;
	dsVector3xf_cross(&crossFirstSecond, &firstPlane->xyzd, &secondPlane->xyzd);
	dsVector3xf_scale(&crossFirstSecond, &crossFirstSecond, -thirdPlane->d);

	dsVector3xf result3x;
	dsVector3xf_add(&result3x, &crossSecondThird, &crossThirdFirst);
	dsVector3xf_add(&result3x, &result3x, &crossFirstSecond);

	float invDenom = 1/denom;
	dsVector3_scale(*result, result3x, invDenom);
	return true;
}

bool dsPlane3d_intersectingPoint(dsVector3d* result, const dsPlane3d* firstPlane,
	const dsPlane3d* secondPlane, const dsPlane3d* thirdPlane)
{
	// http://geomalgorithms.com/a05-_intersect-1.html
	DS_ASSERT(result);
	DS_ASSERT(firstPlane);
	DS_ASSERT(secondPlane);
	DS_ASSERT(thirdPlane);

	const double epsilon2 = dsPow2(1e-14);
	dsVector3xd crossSecondThird;
	dsVector3xd_cross(&crossSecondThird, &secondPlane->xyzd, &thirdPlane->xyzd);
	double denom = dsVector3xd_dot(&firstPlane->xyzd, &crossSecondThird);
	if (fabs(denom) < epsilon2)
		return false;

	dsVector3xd_scale(&crossSecondThird, &crossSecondThird, -firstPlane->d);

	dsVector3xd crossThirdFirst;
	dsVector3xd_cross(&crossThirdFirst, &thirdPlane->xyzd, &firstPlane->xyzd);
	dsVector3xd_scale(&crossThirdFirst, &crossThirdFirst, -secondPlane->d);

	dsVector3xd crossFirstSecond;
	dsVector3xd_cross(&crossFirstSecond, &firstPlane->xyzd, &secondPlane->xyzd);
	dsVector3xd_scale(&crossFirstSecond, &crossFirstSecond, -thirdPlane->d);

	dsVector3xd result3x;
	dsVector3xd_add(&result3x, &crossSecondThird, &crossThirdFirst);
	dsVector3xd_add(&result3x, &result3x, &crossFirstSecond);

	double invDenom = 1/denom;
	dsVector3_scale(*result, result3x, invDenom);
	return true;
}

float dsPlane3f_rayIntersection(const dsPlane3f* plane, const dsRay3f* ray)
{
	DS_ASSERT(plane);
	DS_ASSERT(ray);

	const float epsilon2 = dsPow2(1e-6f);
	float denom = dsVector3xf_dot(&plane->xyzd, &ray->direction);
	if (fabsf(denom) < epsilon2)
		return FLT_MAX;

	dsVector4f origin = ray->origin;
	origin.w = 1.0f;
	return -dsVector4f_dot(&plane->xyzd, &origin)/denom;
}

double dsPlane3d_rayIntersection(const dsPlane3d* plane, const dsRay3d* ray)
{
	DS_ASSERT(plane);
	DS_ASSERT(ray);

	const double epsilon2 = dsPow2(1e-14);
	double denom = dsVector3xd_dot(&plane->xyzd, &ray->direction);
	if (fabs(denom) < epsilon2)
		return DBL_MAX;

	dsVector4d origin = ray->origin;
	origin.w = 1.0;
	return -dsVector4d_dot(&plane->xyzd, &origin)/denom;
}

dsIntersectResult dsPlane3f_intersectAlignedBox(const dsPlane3f* plane, const dsAlignedBox3f* box)
{
	DS_ASSERT(plane);
	DS_ASSERT(box);

	dsVector3xf center, halfExtents;
	dsAlignedBox3_center(center, *box);
	dsAlignedBox3_extents(halfExtents, *box);
	dsVector3xf_scale(&halfExtents, &halfExtents, 0.5f);

	dsVector3xf absNorm;
#if DS_SIMD_ALWAYS_FLOAT4
	absNorm.simd = dsSIMD4f_abs(plane->xyzd.simd);
#else
	absNorm.x = fabsf(plane->n.x);
	absNorm.y = fabsf(plane->n.y);
	absNorm.z = fabsf(plane->n.z);
#endif

	float radius = dsVector3xf_dot(&halfExtents, &absNorm);
	float centerDist = dsVector3xf_dot(&plane->xyzd, &center) + plane->d;

	if (centerDist > radius)
		return dsIntersectResult_Inside;
	else if (centerDist < -radius)
		return dsIntersectResult_Outside;
	else
		return dsIntersectResult_Intersects;
}

dsIntersectResult dsPlane3d_intersectAlignedBox(const dsPlane3d* plane, const dsAlignedBox3d* box)
{
	DS_ASSERT(plane);
	DS_ASSERT(box);

	// https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html
	dsVector3xd center, halfExtents;
	dsAlignedBox3_center(center, *box);
	dsAlignedBox3_extents(halfExtents, *box);
	dsVector3xd_scale(&halfExtents, &halfExtents, 0.5f);

	dsVector3xd absNorm;
#if DS_SIMD_ALWAYS_FLOAT4
	absNorm.simd2[0] = dsSIMD2d_abs(plane->xyzd.simd2[0]);
	absNorm.simd2[1] = dsSIMD2d_abs(plane->xyzd.simd2[1]);
#else
	absNorm.x = fabs(plane->n.x);
	absNorm.y = fabs(plane->n.y);
	absNorm.z = fabs(plane->n.z);
#endif

	double radius = dsVector3xd_dot(&halfExtents, &absNorm);
	double centerDist = dsVector3xd_dot(&plane->xyzd, &center) + plane->d;

	if (centerDist > radius)
		return dsIntersectResult_Inside;
	else if (centerDist < -radius)
		return dsIntersectResult_Outside;
	else
		return dsIntersectResult_Intersects;
}

dsIntersectResult dsPlane3f_intersectAlignedBox3x(
	const dsPlane3f* plane, const dsAlignedBox3xf* box)
{
	DS_ASSERT(plane);
	DS_ASSERT(box);

	dsVector3xf center, halfExtents;
	dsAlignedBox3xf_center(&center, box);
	dsAlignedBox3xf_extents(&halfExtents, box);
	dsVector3xf_scale(&halfExtents, &halfExtents, 0.5f);

	dsVector3xf absNorm;
#if DS_SIMD_ALWAYS_FLOAT4
	absNorm.simd = dsSIMD4f_abs(plane->xyzd.simd);
#else
	absNorm.x = fabsf(plane->n.x);
	absNorm.y = fabsf(plane->n.y);
	absNorm.z = fabsf(plane->n.z);
#endif

	float radius = dsVector3xf_dot(&halfExtents, &absNorm);
	float centerDist = dsVector3xf_dot(&plane->xyzd, &center) + plane->d;

	if (centerDist > radius)
		return dsIntersectResult_Inside;
	else if (centerDist < -radius)
		return dsIntersectResult_Outside;
	else
		return dsIntersectResult_Intersects;
}

dsIntersectResult dsPlane3d_intersectAlignedBox3x(
	const dsPlane3d* plane, const dsAlignedBox3xd* box)
{
	DS_ASSERT(plane);
	DS_ASSERT(box);

	// https://gdbooks.gitbooks.io/3dcollisions/content/Chapter2/static_aabb_plane.html
	dsVector3xd center, halfExtents;
	dsAlignedBox3xd_center(&center, box);
	dsAlignedBox3xd_extents(&halfExtents, box);
	dsVector3xd_scale(&halfExtents, &halfExtents, 0.5f);

	dsVector3xd absNorm;
#if DS_SIMD_ALWAYS_FLOAT4
	absNorm.simd2[0] = dsSIMD2d_abs(plane->xyzd.simd2[0]);
	absNorm.simd2[1] = dsSIMD2d_abs(plane->xyzd.simd2[1]);
#else
	absNorm.x = fabs(plane->n.x);
	absNorm.y = fabs(plane->n.y);
	absNorm.z = fabs(plane->n.z);
#endif

	double radius = dsVector3xd_dot(&halfExtents, &absNorm);
	double centerDist = dsVector3xd_dot(&plane->xyzd, &center) + plane->d;

	if (centerDist > radius)
		return dsIntersectResult_Inside;
	else if (centerDist < -radius)
		return dsIntersectResult_Outside;
	else
		return dsIntersectResult_Intersects;
}

dsIntersectResult dsPlane3f_intersectOrientedBox(const dsPlane3f* plane, const dsOrientedBox3f* box)
{
	DS_ASSERT(plane);
	DS_ASSERT(box);

	dsMatrix44f boxMatrix;
	dsOrientedBox3_toMatrixTranspose(boxMatrix, *box);
	return dsPlane3f_intersectBoxMatrixTranspose(plane, &boxMatrix);
}

dsIntersectResult dsPlane3d_intersectOrientedBox(const dsPlane3d* plane, const dsOrientedBox3d* box)
{
	DS_ASSERT(plane);
	DS_ASSERT(box);

	dsMatrix44d boxMatrix;
	dsOrientedBox3_toMatrixTranspose(boxMatrix, *box);
	return dsPlane3d_intersectBoxMatrixTranspose(plane, &boxMatrix);
}

dsIntersectResult dsPlane3f_intersectOrientedBox3x(
	const dsPlane3f* plane, const dsOrientedBox3xf* box)
{
	DS_ASSERT(plane);
	DS_ASSERT(box);

	dsMatrix44f boxMatrix;
	dsOrientedBox3xf_toMatrixTranspose(&boxMatrix, box);
	return dsPlane3f_intersectBoxMatrixTranspose(plane, &boxMatrix);
}

dsIntersectResult dsPlane3d_intersectOrientedBox3x(
	const dsPlane3d* plane, const dsOrientedBox3xd* box)
{
	DS_ASSERT(plane);
	DS_ASSERT(box);

	dsMatrix44d boxMatrix;
	dsOrientedBox3xd_toMatrixTranspose(&boxMatrix, box);
	return dsPlane3d_intersectBoxMatrixTranspose(plane, &boxMatrix);
}

void dsPlane3f_fromNormalPoint(
	dsPlane3f* result, const dsVector3f* normal, const dsVector3f* point);
void dsPlane3d_fromNormalPoint(
	dsPlane3d* result, const dsVector3d* normal, const dsVector3d* point);

float dsPlane3f_distanceToPoint(const dsPlane3f* plane, const dsVector3f* point);
double dsPlane3d_distanceToPoint(const dsPlane3d* plane, const dsVector3d* point);
void dsPlane3f_normalize(dsPlane3f* result, const dsPlane3f* plane);
void dsPlane3d_normalize(dsPlane3d* result, const dsPlane3d* plane);
void dsPlane3f_transform(dsPlane3f* result, const dsMatrix44f* transform, const dsPlane3f* plane);
void dsPlane3d_transform(dsPlane3d* result, const dsMatrix44d* transform, const dsPlane3d* plane);
void dsPlane3f_transformInverseTranspose(
	dsPlane3f* result, const dsMatrix44f* transform, const dsPlane3f* plane);
void dsPlane3d_transformInverseTranspose(
	dsPlane3d* result, const dsMatrix44d* transform, const dsPlane3d* plane);
dsIntersectResult dsPlane3f_intersectBoxMatrix(
	const dsPlane3f* plane, const dsMatrix44f* boxMatrix);
dsIntersectResult dsPlane3d_intersectBoxMatrix(
	const dsPlane3d* plane, const dsMatrix44d* boxMatrix);
dsIntersectResult dsPlane3f_intersectBoxMatrixTranspose(
	const dsPlane3f* plane, const dsMatrix44f* boxMatrix);
dsIntersectResult dsPlane3d_intersectBoxMatrixTranspose(
	const dsPlane3d* plane, const dsMatrix44d* boxMatrix);

#if DS_HAS_SIMD
dsIntersectResult dsPlane3f_intersectBoxMatrixSIMD(
	const dsPlane3f* plane, const dsMatrix44f* boxMatrix);
dsIntersectResult dsPlane3f_intersectBoxMatrixTransposeSIMD(
	const dsPlane3f* plane, const dsMatrix44f* boxMatrix);

#if !DS_DETERMINISTIC_MATH
dsIntersectResult dsPlane3f_intersectBoxMatrixFMA(
	const dsPlane3f* plane, const dsMatrix44f* boxMatrix);
dsIntersectResult dsPlane3f_intersectBoxMatrixTransposeFMA(
	const dsPlane3f* plane, const dsMatrix44f* boxMatrix);
#endif

dsIntersectResult dsPlane3d_intersectBoxMatrixSIMD2(
	const dsPlane3d* plane, const dsMatrix44d* boxMatrix);
dsIntersectResult dsPlane3d_intersectBoxMatrixTransposeSIMD2(
	const dsPlane3d* plane, const dsMatrix44d* boxMatrix);

#if !DS_DETERMINISTIC_MATH
dsIntersectResult dsPlane3d_intersectBoxMatrixFMA2(
	const dsPlane3d* plane, const dsMatrix44d* boxMatrix);
dsIntersectResult dsPlane3d_intersectBoxMatrixTransposeFMA2(
	const dsPlane3d* plane, const dsMatrix44d* boxMatrix);
#endif

dsIntersectResult dsPlane3d_intersectBoxMatrixSIMD4(
	const dsPlane3d* DS_ALIGN_PARAM(32) plane, const dsMatrix44d* DS_ALIGN_PARAM(32) boxMatrix);
dsIntersectResult dsPlane3d_intersectBoxMatrixTransposeSIMD4(
	const dsPlane3d* DS_ALIGN_PARAM(32) plane, const dsMatrix44d* DS_ALIGN_PARAM(32) boxMatrix);
#endif

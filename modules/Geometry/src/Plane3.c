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

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Geometry/OrientedBox3.h>
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Matrix44.h>

#include <float.h>

bool dsPlane3f_intersectingLine(dsRay3f* result, const dsPlane3f* firstPlane,
	const dsPlane3f* secondPlane)
{
	// http://geomalgorithms.com/a05-_intersect-1.html
	DS_ASSERT(result);
	DS_ASSERT(firstPlane);
	DS_ASSERT(secondPlane);

	const float epsilon2 = dsPow2(1e-6f);
	dsVector3_cross(result->direction, firstPlane->n, secondPlane->n);
	float len2 = dsVector3_len2(result->direction);
	if (len2 < epsilon2)
		return false;

	dsVector3f scaledFirstN;
	dsVector3_scale(scaledFirstN, firstPlane->n, secondPlane->d);

	dsVector3f scaledSecondN;
	dsVector3_scale(scaledSecondN, secondPlane->n, firstPlane->d);

	dsVector3f diff;
	dsVector3_sub(diff, scaledFirstN, scaledSecondN);

	float invLen2 = 1/len2;
	dsVector3_cross(result->origin, diff, result->direction);
	dsVector3_scale(result->origin, result->origin, invLen2);

	float invLen = 1/sqrtf(len2);
	dsVector3_scale(result->direction, result->direction, invLen);
	return true;
}

bool dsPlane3d_intersectingLine(dsRay3d* result, const dsPlane3d* firstPlane,
	const dsPlane3d* secondPlane)
{
	// http://geomalgorithms.com/a05-_intersect-1.html
	DS_ASSERT(result);
	DS_ASSERT(firstPlane);
	DS_ASSERT(secondPlane);

	const double epsilon2 = dsPow2(1e-14);
	dsVector3_cross(result->direction, firstPlane->n, secondPlane->n);
	double len2 = dsVector3_len2(result->direction);
	if (len2 < epsilon2)
		return false;

	dsVector3d scaledFirstN;
	dsVector3_scale(scaledFirstN, firstPlane->n, secondPlane->d);

	dsVector3d scaledSecondN;
	dsVector3_scale(scaledSecondN, secondPlane->n, firstPlane->d);

	dsVector3d diff;
	dsVector3_sub(diff, scaledFirstN, scaledSecondN);

	double invLen2 = 1/len2;
	dsVector3_cross(result->origin, diff, result->direction);
	dsVector3_scale(result->origin, result->origin, invLen2);

	double invLen = 1/sqrt(len2);
	dsVector3_scale(result->direction, result->direction, invLen);
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
	dsVector3f crossSecondThird;
	dsVector3_cross(crossSecondThird, secondPlane->n, thirdPlane->n);
	float denom = dsVector3_dot(firstPlane->n, crossSecondThird);
	if (fabsf(denom) < epsilon2)
		return false;

	dsVector3_scale(crossSecondThird, crossSecondThird, -firstPlane->d);

	dsVector3f crossThirdFirst;
	dsVector3_cross(crossThirdFirst, thirdPlane->n, firstPlane->n);
	dsVector3_scale(crossThirdFirst, crossThirdFirst, -secondPlane->d);

	dsVector3f crossFirstSecond;
	dsVector3_cross(crossFirstSecond, firstPlane->n, secondPlane->n);
	dsVector3_scale(crossFirstSecond, crossFirstSecond, -thirdPlane->d);

	dsVector3_add(*result, crossSecondThird, crossThirdFirst);
	dsVector3_add(*result, *result, crossFirstSecond);

	float invDenom = 1/denom;
	dsVector3_scale(*result, *result, invDenom);
	return true;
}

bool dsPlane3d_intersectingPoint(dsVector3d* result, const dsPlane3d* firstPlane,
	const dsPlane3d* secondPlane, const dsPlane3d* thirdPlane)
{
	// http://geomalgorithms.com/a05-_intersect-1.html
	// D is inverted due to using form A + B + C = D.
	DS_ASSERT(result);
	DS_ASSERT(firstPlane);
	DS_ASSERT(secondPlane);
	DS_ASSERT(thirdPlane);

	const double epsilon2 = dsPow2(1e-14);
	dsVector3d crossSecondThird;
	dsVector3_cross(crossSecondThird, secondPlane->n, thirdPlane->n);
	double denom = dsVector3_dot(firstPlane->n, crossSecondThird);
	if (fabs(denom) < epsilon2)
		return false;

	dsVector3_scale(crossSecondThird, crossSecondThird, -firstPlane->d);

	dsVector3d crossThirdFirst;
	dsVector3_cross(crossThirdFirst, thirdPlane->n, firstPlane->n);
	dsVector3_scale(crossThirdFirst, crossThirdFirst, -secondPlane->d);

	dsVector3d crossFirstSecond;
	dsVector3_cross(crossFirstSecond, firstPlane->n, secondPlane->n);
	dsVector3_scale(crossFirstSecond, crossFirstSecond, -thirdPlane->d);

	dsVector3_add(*result, crossSecondThird, crossThirdFirst);
	dsVector3_add(*result, *result, crossFirstSecond);

	double invDenom = 1/denom;
	dsVector3_scale(*result, *result, invDenom);
	return true;
}

float dsPlane3f_rayIntersection(const dsPlane3f* plane, const dsRay3f* ray)
{
	DS_ASSERT(plane);
	DS_ASSERT(ray);

	const float epsilon2 = dsPow2(1e-6f);
	float denom = dsVector3_dot(plane->n, ray->direction);
	if (fabsf(denom) < epsilon2)
		return FLT_MAX;

	return -(dsVector3_dot(plane->n, ray->origin) + plane->d)/denom;
}

double dsPlane3d_rayIntersection(const dsPlane3d* plane, const dsRay3d* ray)
{
	DS_ASSERT(plane);
	DS_ASSERT(ray);

	const double epsilon2 = dsPow2(1e-14);
	double denom = dsVector3_dot(plane->n, ray->direction);
	if (fabs(denom) < epsilon2)
		return DBL_MAX;

	return -(dsVector3_dot(plane->n, ray->origin) + plane->d)/denom;
}

dsIntersectResult dsPlane3f_intersectAlignedBox(const dsPlane3f* plane, const dsAlignedBox3f* box)
{
	DS_ASSERT(plane);
	DS_ASSERT(box);

	dsVector3f center, halfExtents;
	dsAlignedBox3_center(center, *box);
	dsAlignedBox3_extents(halfExtents, *box);
	dsVector3_scale(halfExtents, halfExtents, 0.5f);

	float radius = halfExtents.x*fabsf(plane->n.x) + halfExtents.y*fabsf(plane->n.y) +
		halfExtents.z*fabsf(plane->n.z);
	float centerDist = dsVector3_dot(plane->n, center) + plane->d;

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
	dsVector3d center, halfExtents;
	dsAlignedBox3_center(center, *box);
	dsAlignedBox3_extents(halfExtents, *box);
	dsVector3_scale(halfExtents, halfExtents, 0.5f);

	double radius = halfExtents.x*fabs(plane->n.x) + halfExtents.y*fabs(plane->n.y) +
		halfExtents.z*fabs(plane->n.z);
	double centerDist = dsVector3_dot(plane->n, center) + plane->d;

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

	// Same as aligned box, but radius computation is projected to box orientation.
	dsVector3f orientedNormal;
	dsMatrix33_transformTransposed(orientedNormal, box->orientation, plane->n);
	float radius = box->halfExtents.x*fabsf(orientedNormal.x) +
		box->halfExtents.y*fabsf(orientedNormal.y) + box->halfExtents.z*fabsf(orientedNormal.z);
	float centerDist = dsVector3_dot(plane->n, box->center) + plane->d;

	if (centerDist > radius)
		return dsIntersectResult_Inside;
	else if (centerDist < -radius)
		return dsIntersectResult_Outside;
	else
		return dsIntersectResult_Intersects;
}

dsIntersectResult dsPlane3d_intersectOrientedBox(const dsPlane3d* plane, const dsOrientedBox3d* box)
{
	DS_ASSERT(plane);
	DS_ASSERT(box);

	// Same as aligned box, but radius computation is projected to box orientation.
	dsVector3d orientedNormal;
	dsMatrix33_transformTransposed(orientedNormal, box->orientation, plane->n);
	double radius = box->halfExtents.x*fabs(orientedNormal.x) +
		box->halfExtents.y*fabs(orientedNormal.y) + box->halfExtents.z*fabs(orientedNormal.z);
	double centerDist = dsVector3_dot(plane->n, box->center) + plane->d;

	if (centerDist > radius)
		return dsIntersectResult_Inside;
	else if (centerDist < -radius)
		return dsIntersectResult_Outside;
	else
		return dsIntersectResult_Intersects;
}

void dsPlane3f_fromNormalPoint(dsPlane3f* result, const dsVector3f* normal,
	const dsVector3f* point);
void dsPlane3d_fromNormalPoint(dsPlane3d* result, const dsVector3d* normal,
	const dsVector3d* point);

float dsPlane3f_distanceToPoint(const dsPlane3f* plane, const dsVector3f* point);
double dsPlane3d_distanceToPoint(const dsPlane3d* plane, const dsVector3d* point);
void dsPlane3f_normalize(dsPlane3f* result, const dsPlane3f* plane);
void dsPlane3d_normalize(dsPlane3d* result, const dsPlane3d* plane);
void dsPlane3f_transform(dsPlane3f* result, const dsMatrix44f* transform, const dsPlane3f* plane);
void dsPlane3d_transform(dsPlane3d* result, const dsMatrix44d* transform, const dsPlane3d* plane);
void dsPlane3f_transformInverseTranspose(dsPlane3f* result, const dsMatrix44f* transform,
	const dsPlane3f* plane);
void dsPlane3d_transformInverseTranspose(dsPlane3d* result, const dsMatrix44d* transform,
	const dsPlane3d* plane);

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

#include <DeepSea/Math/SIMD/Dot.h>
#include <DeepSea/Math/Sqrt.h>

#if DS_SIMD_ALWAYS_FLOAT4
static inline dsSIMD4f dsAlignedBox3xf_dist2Impl(
	const dsAlignedBox3xf* box, const dsVector3xf* point)
{
	dsSIMD4f minDiff = dsSIMD4f_sub(box->min.simd, point->simd);
	dsSIMD4f maxDiff = dsSIMD4f_sub(point->simd, box->max.simd);

	dsSIMD4f baseDist = dsSIMD4f_max(dsSIMD4f_max(minDiff, maxDiff), dsSIMD4f_set1(0.0f));
	return dsDot3SIMD4f(baseDist, baseDist);
}
#endif

#if DS_SIMD_ALWAYS_DOUBLE2
static inline dsSIMD2d dsAlignedBox3xd_dist2Impl(
	const dsAlignedBox3xd* box, const dsVector3xd* point)
{
	dsSIMD2d minDiff0 = dsSIMD2d_sub(box->min.simd2[0], point->simd2[0]);
	dsSIMD2d minDiff1 = dsSIMD2d_sub(box->min.simd2[1], point->simd2[1]);
	dsSIMD2d maxDiff0 = dsSIMD2d_sub(point->simd2[0], box->max.simd2[0]);
	dsSIMD2d maxDiff1 = dsSIMD2d_sub(point->simd2[1], box->max.simd2[1]);

	dsSIMD2d zero = dsSIMD2d_set1(0.0);
	dsSIMD2d baseDist0 = dsSIMD2d_max(dsSIMD2d_max(minDiff0, maxDiff0), zero);
	dsSIMD2d baseDist1 = dsSIMD2d_max(dsSIMD2d_max(minDiff1, maxDiff1), zero);
	return dsDot3SIMD2d(baseDist0, baseDist1, baseDist0, baseDist1);
}
#endif

float dsAlignedBox3xf_dist2(const dsAlignedBox3xf* box, const dsVector3xf* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsAlignedBox3xf_isValid(box))
		return -1.0f;

#if DS_SIMD_ALWAYS_FLOAT4
	dsSIMD4f dist2 = dsAlignedBox3xf_dist2Impl(box, point);
	return dsSIMD4f_get(dist2, 0);
#else
	float dx = dsMax(box->min.x - point->x, point->x - box->max.x);
	dx = dsMax(dx, 0.0f);
	float dy = dsMax(box->min.y - point->y, point->y - box->max.y);
	dy = dsMax(dy, 0.0f);
	float dz = dsMax(box->min.z - point->z, point->z - box->max.z);
	dz = dsMax(dz, 0.0f);

	return dsPow2(dx) + dsPow2(dy) + dsPow2(dz);
#endif
}

double dsAlignedBox3xd_dist2(const dsAlignedBox3xd* box, const dsVector3xd* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsAlignedBox3xd_isValid(box))
		return -1.0;

#if DS_SIMD_ALWAYS_DOUBLE2
	dsSIMD2d dist2 = dsAlignedBox3xd_dist2Impl(box, point);
	return dsSIMD2d_get(dist2, 0);
#else
	double dx = dsMax(box->min.x - point->x, point->x - box->max.x);
	dx = dsMax(dx, 0.0);
	double dy = dsMax(box->min.y - point->y, point->y - box->max.y);
	dy = dsMax(dy, 0.0);
	double dz = dsMax(box->min.z - point->z, point->z - box->max.z);
	dz = dsMax(dz, 0.0);

	return dsPow2(dx) + dsPow2(dy) + dsPow2(dz);
#endif
}

float dsAlignedBox3xf_dist(const dsAlignedBox3xf* box, const dsVector3xf* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsAlignedBox3xf_isValid(box))
		return -1.0f;

#if DS_SIMD_ALWAYS_FLOAT4
	dsSIMD4f dist2 = dsAlignedBox3xf_dist2Impl(box, point);
#if DS_SIMD_EMULATED_DIV_SQRT
	return dsSqrtf(dsSIMD4f_get(dist2, 0));
#else
	dsSIMD4f dist = dsSIMD4f_sqrt(dist2);
	return dsSIMD4f_get(dist, 0);
#endif
#else
	float dx = dsMax(box->min.x - point->x, point->x - box->max.x);
	dx = dsMax(dx, 0.0f);
	float dy = dsMax(box->min.y - point->y, point->y - box->max.y);
	dy = dsMax(dy, 0.0f);
	float dz = dsMax(box->min.z - point->z, point->z - box->max.z);
	dz = dsMax(dz, 0.0f);

	return dsSqrtf(dsPow2(dx) + dsPow2(dy) + dsPow2(dz));
#endif
}

double dsAlignedBox3xd_dist(const dsAlignedBox3xd* box, const dsVector3xd* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsAlignedBox3xd_isValid(box))
		return -1.0;

#if DS_SIMD_ALWAYS_DOUBLE2
	dsSIMD2d dist2 = dsAlignedBox3xd_dist2Impl(box, point);
	dsSIMD2d dist = dsSIMD2d_sqrt(dist2);
	return dsSIMD2d_get(dist, 0);
#else
	double dx = dsMax(box->min.x - point->x, point->x - box->max.x);
	dx = dsMax(dx, 0.0);
	double dy = dsMax(box->min.y - point->y, point->y - box->max.y);
	dy = dsMax(dy, 0.0);
	double dz = dsMax(box->min.z - point->z, point->z - box->max.z);
	dz = dsMax(dz, 0.0);

	return dsSqrtd(dsPow2(dx) + dsPow2(dy) + dsPow2(dz));
#endif
}

bool dsAlignedBox3xf_isValid(const dsAlignedBox3xf* box);
bool dsAlignedBox3xd_isValid(const dsAlignedBox3xd* box);

void dsAlignedBox3xf_addPoint(dsAlignedBox3xf* box, const dsVector3xf* point);
void dsAlignedBox3xd_addPoint(dsAlignedBox3xd* box, const dsVector3xd* point);

void dsAlignedBox3xf_addBox(dsAlignedBox3xf* box, const dsAlignedBox3xf* otherBox);
void dsAlignedBox3xd_addBox(dsAlignedBox3xd* box, const dsAlignedBox3xd* otherBox);

bool dsAlignedBox3xf_containsPoint(const dsAlignedBox3xf* box, const dsVector3xf* point);
bool dsAlignedBox3xd_containsPoint(const dsAlignedBox3xd* box, const dsVector3xd* point);

bool dsAlignedBox3xf_containsBox(const dsAlignedBox3xf* box, const dsAlignedBox3xf* otherBox);
bool dsAlignedBox3xd_containsBox(const dsAlignedBox3xd* box, const dsAlignedBox3xd* otherBox);

bool dsAlignedBox3xf_intersects(const dsAlignedBox3xf* box, const dsAlignedBox3xf* otherBox);
bool dsAlignedBox3xd_intersects(const dsAlignedBox3xd* box, const dsAlignedBox3xd* otherBox);

void dsAlignedBox3xf_intersect(
	dsAlignedBox3xf* result, const dsAlignedBox3xf* a, const dsAlignedBox3xf* b);
void dsAlignedBox3xd_intersect(
	dsAlignedBox3xd* result, const dsAlignedBox3xd* a, const dsAlignedBox3xd* b);

void dsAlignedBox3xf_center(dsVector3xf* result, const dsAlignedBox3xf* box);
void dsAlignedBox3xd_center(dsVector3xd* result, const dsAlignedBox3xd* box);

void dsAlignedBox3xf_extents(dsVector3xf* result, const dsAlignedBox3xf* box);
void dsAlignedBox3xd_extents(dsVector3xd* result, const dsAlignedBox3xd* box);

void dsAlignedBox3xf_toMatrix(dsMatrix44f* result, const dsAlignedBox3xf* box);
void dsAlignedBox3xd_toMatrix(dsMatrix44d* result, const dsAlignedBox3xd* box);

void dsAlignedBox3xf_toMatrixTranspose(dsMatrix44f* result, const dsAlignedBox3xf* box);
void dsAlignedBox3xd_toMatrixTranspose(dsMatrix44d* result, const dsAlignedBox3xd* box);

void dsAlignedBox3xf_corners(dsVector3xf corners[DS_BOX3_CORNER_COUNT], const dsAlignedBox3xf* box);
void dsAlignedBox3xd_corners(dsVector3xd corners[DS_BOX3_CORNER_COUNT], const dsAlignedBox3xd* box);

void dsAlignedBox3xf_closestPoint(
	dsVector3xf* result, const dsAlignedBox3xf* box, const dsVector3xf* point);
void dsAlignedBox3xd_closestPoint(
	dsVector3xd* result, const dsAlignedBox3xd* box, const dsVector3xd* point);

void dsAlignedBox3xf_makeInvalid(dsAlignedBox3xf* result);
void dsAlignedBox3xd_makeInvalid(dsAlignedBox3xd* result);

#if DS_HAS_SIMD
void dsAlignedBox3xf_toMatrixSIMD(dsMatrix44f* result, const dsAlignedBox3xf* box);
void dsAlignedBox3xd_toMatrixSIMD2(dsMatrix44d* result, const dsAlignedBox3xd* box);
void dsAlignedBox3xd_toMatrixSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsAlignedBox3xd* DS_ALIGN_PARAM(32) box);

void dsAlignedBox3xf_toMatrixTransposeSIMD(dsMatrix44f* result, const dsAlignedBox3xf* box);
void dsAlignedBox3xd_toMatrixTransposeSIMD2(dsMatrix44d* result, const dsAlignedBox3xd* box);
void dsAlignedBox3xd_toMatrixTransposeSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsAlignedBox3xd* DS_ALIGN_PARAM(32) box);
#endif // DS_HAS_SIMD

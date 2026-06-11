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

#include <DeepSea/Geometry/OrientedBox3x.h>

#include <DeepSea/Math/Matrix44.h>

#define PARALLEL_CUTOFFf (1.0f - 1e-6f)
#define PARALLEL_CUTOFFd (1.0 - 1e-14)

// TODO: Expose if explicit SIMD operations are provided in the future.
#if DS_SIMD_ALWAYS_FLOAT4
#if DS_X86
#define DS_SIMD_SHUFFLE_PAIRS(a, b) _mm_shuffle_ps((a), (b), _MM_SHUFFLE(3, 2, 1, 0))
#define DS_SIMD_SHUFFLE_012_0(result, a, b) \
	do \
	{ \
		dsSIMD4f _temp = _mm_shuffle_ps((a), (b), _MM_SHUFFLE(0, 0, 2, 2)); \
		(result) = _mm_shuffle_ps((a), _temp, _MM_SHUFFLE(3, 0, 1, 0)); \
	} while (0)
#define DS_SIMD_SHUFFLE_12_01(a, b) _mm_shuffle_ps((a), (b), _MM_SHUFFLE(1, 0, 2, 1))
#elif DS_ARM
#define DS_SIMD_SHUFFLE_PAIRS(a, b) vcombine_f32(vget_low_f32((a)), vget_low_f32((b)))
#define DS_SIMD_SHUFFLE_012_0(result, a, b) \
	do \
	{ \
		(result) = vcombine_f32( \
			vget_low_f32((a)), vext_f32(vget_high_f32((a)), vget_low_f32((b)), 0)); \
	} while (0)
#define DS_SIMD_SHUFFLE_12_01(a, b) vcombine_f32( \
	vext_f32(vget_low_f32((a)), vget_high_f32((a)), 1), vget_low_f32((b)));
#else
#error Need to implement vector combination for this platform.
#endif

static inline bool dsOrientedBox3xf_intersectsSIMD(
	const dsOrientedBox3xf* box, const dsOrientedBox3xf* otherBox)
{
	dsMatrix33xf orientationTrans, otherOrientationTrans;
	dsMatrix33xf_transposeSIMD(&orientationTrans, &box->orientation);
	dsMatrix33xf_transposeSIMD(&otherOrientationTrans, &otherBox->orientation);

	dsMatrix33xf dotAxes;
	dsMatrix33xf_mul(&dotAxes, &otherOrientationTrans, &box->orientation);

	dsMatrix33xf absDotAxes;
	absDotAxes.columns[0].simd = dsSIMD4f_abs(dotAxes.columns[0].simd);
	absDotAxes.columns[1].simd = dsSIMD4f_abs(dotAxes.columns[1].simd);
	absDotAxes.columns[2].simd = dsSIMD4f_abs(dotAxes.columns[2].simd);

	dsVector3xf centerDiff, dotDiffAxes;
	dsVector3xf_sub(&centerDiff, &otherBox->center, &box->center);
	dsMatrix33xf_transform(&dotDiffAxes, &orientationTrans, &centerDiff);

	dsVector3xf radii, dists;
	dsMatrix33xf_transformTransposed(&radii, &absDotAxes, &otherBox->halfExtents);
	radii.simd = dsSIMD4f_add(radii.simd, box->halfExtents.simd);
	dsMatrix33xf_transform(&dists, &orientationTrans, &centerDiff);

	// radiusCmp is for outside radius for this case.
	dsVector4i radiusCmp;
	radiusCmp.simd = dsSIMD4f_cmpgt(dsSIMD4f_abs(dists.simd), radii.simd);

	dsMatrix33xf_transform(&radii, &absDotAxes, &box->halfExtents);
	radii.simd = dsSIMD4f_add(radii.simd, otherBox->halfExtents.simd);
	dsMatrix33xf_transform(&dists, &otherOrientationTrans, &centerDiff);

	radiusCmp.simd = dsSIMD4fb_or(
		radiusCmp.simd, dsSIMD4f_cmpgt(dsSIMD4f_abs(dists.simd), radii.simd));
	if (radiusCmp.x || radiusCmp.y || radiusCmp.z)
		return false;

	// When there's a parallel set of axes it degenerates to 2D.
	dsSIMD4f parallelCutoff = dsSIMD4f_set1(PARALLEL_CUTOFFf);
	dsVector4i hasParallel;
	hasParallel.simd = dsSIMD4fb_or(dsSIMD4fb_or(
			dsSIMD4f_cmpge(absDotAxes.columns[0].simd, parallelCutoff),
			dsSIMD4f_cmpge(absDotAxes.columns[1].simd, parallelCutoff)),
		dsSIMD4f_cmpge(absDotAxes.columns[3].simd, parallelCutoff));
	if (hasParallel.x || hasParallel.y || hasParallel.z)
		return true;

	dsSIMD4f dotDiffAxesX = dsSIMD4f_set1FromVec(dotDiffAxes.simd, 0);
	dsSIMD4f dotDiffAxesY = dsSIMD4f_set1FromVec(dotDiffAxes.simd, 1);
	dsSIMD4f dotDiffAxesZ = dsSIMD4f_set1FromVec(dotDiffAxes.simd, 2);

	// A0 x B0, A0 x B1, A0 x B2, A1 x B0
	// radiusCmp is for inside radius for these and the following cases.
	radii.x = box->halfExtents.y*absDotAxes.values[2][0] +
		box->halfExtents.z*absDotAxes.values[1][0] +
		otherBox->halfExtents.y*absDotAxes.values[0][2] +
		otherBox->halfExtents.z*absDotAxes.values[0][1];
	radii.y = box->halfExtents.y*absDotAxes.values[2][1] +
		box->halfExtents.z*absDotAxes.values[1][1] +
		otherBox->halfExtents.x*absDotAxes.values[0][2] +
		otherBox->halfExtents.z*absDotAxes.values[0][0];
	radii.z = box->halfExtents.y*absDotAxes.values[2][2] +
		box->halfExtents.z*absDotAxes.values[1][2] +
		otherBox->halfExtents.x*absDotAxes.values[0][1] +
		otherBox->halfExtents.y*absDotAxes.values[0][0];
	radii.w = box->halfExtents.x*absDotAxes.values[2][0] +
		box->halfExtents.z*absDotAxes.values[0][0] +
		otherBox->halfExtents.y*absDotAxes.values[1][2] +
		otherBox->halfExtents.z*absDotAxes.values[1][1];

	dsSIMD4f dotDiffAxesZZZX, dotDiffAxesYYYZ, dotAxis1012_20, dotAxes2012_00;
	DS_SIMD_SHUFFLE_012_0(dotDiffAxesZZZX, dotDiffAxesZ, dotDiffAxesX);
	DS_SIMD_SHUFFLE_012_0(dotDiffAxesYYYZ, dotDiffAxesY, dotDiffAxesZ);
	DS_SIMD_SHUFFLE_012_0(dotAxis1012_20, dotAxes.columns[1].simd, dotAxes.columns[2].simd);
	DS_SIMD_SHUFFLE_012_0(dotAxes2012_00, dotAxes.columns[2].simd, dotAxes.columns[0].simd);
#if DS_SIMD_ALWAYS_FMA
	dists.simd = dsSIMD4f_fnmadd(dotDiffAxesYYYZ, dotAxes2012_00,
		dsSIMD4f_mul(dotDiffAxesZZZX, dotAxis1012_20));
#else
	dists.simd = dsSIMD4f_sub(dsSIMD4f_mul(dotDiffAxesZZZX, dotAxis1012_20),
		dsSIMD4f_mul(dotDiffAxesYYYZ, dotAxes2012_00));
#endif
	dists.x = dotDiffAxes.z*dotAxes.values[1][0] - dotDiffAxes.y*dotAxes.values[2][0];
	dists.y = dotDiffAxes.z*dotAxes.values[1][1] - dotDiffAxes.y*dotAxes.values[2][1];
	dists.z = dotDiffAxes.z*dotAxes.values[1][2] - dotDiffAxes.y*dotAxes.values[2][2];
	dists.w = dotDiffAxes.x*dotAxes.values[2][0] - dotDiffAxes.z*dotAxes.values[0][0];
	radiusCmp.simd = dsSIMD4f_cmple(dsSIMD4f_abs(dists.simd), radii.simd);

	// A1 x B1, A1 x B2, A2 x B0, A2 x B1
	radii.x = box->halfExtents.x*absDotAxes.values[2][1] +
		box->halfExtents.z*absDotAxes.values[0][1] +
		otherBox->halfExtents.x*absDotAxes.values[1][2] +
		otherBox->halfExtents.z*absDotAxes.values[1][0];
	radii.y =  box->halfExtents.x*absDotAxes.values[2][2] +
		box->halfExtents.z*absDotAxes.values[0][2] +
		otherBox->halfExtents.x*absDotAxes.values[1][1] +
		otherBox->halfExtents.y*absDotAxes.values[1][0];
	radii.z = box->halfExtents.x*absDotAxes.values[1][0] +
		box->halfExtents.y*absDotAxes.values[0][0] +
		otherBox->halfExtents.y*absDotAxes.values[2][2] +
		otherBox->halfExtents.z*absDotAxes.values[2][1];
	radii.w = box->halfExtents.x*absDotAxes.values[1][1] +
		box->halfExtents.y*absDotAxes.values[0][1] +
		otherBox->halfExtents.x*absDotAxes.values[2][2] +
		otherBox->halfExtents.z*absDotAxes.values[2][0];

	dsSIMD4f dotDiffAxesXXYY = DS_SIMD_SHUFFLE_PAIRS(dotDiffAxesX, dotDiffAxesY);
	dsSIMD4f dotDiffAxesZZXX = DS_SIMD_SHUFFLE_PAIRS(dotDiffAxesZ, dotDiffAxesX);
	dsSIMD4f dotAxes212_001 = DS_SIMD_SHUFFLE_12_01(
		dotAxes.columns[2].simd, dotAxes.columns[0].simd);
	dsSIMD4f dotAxes012_101 = DS_SIMD_SHUFFLE_12_01(
		dotAxes.columns[0].simd, dotAxes.columns[1].simd);
#if DS_SIMD_ALWAYS_FMA
	dists.simd = dsSIMD4f_fnmadd(dotDiffAxesZZXX, dotAxes012_101,
		dsSIMD4f_mul(dotDiffAxesXXYY, dotAxes212_001));
#else
	dists.simd = dsSIMD4f_sub(dsSIMD4f_mul(dotDiffAxesXXYY, dotAxes212_001),
		dsSIMD4f_mul(dotDiffAxesZZXX, dotAxes012_101));
#endif
	radiusCmp.simd = dsSIMD4fb_and(
		radiusCmp.simd, dsSIMD4f_cmple(dsSIMD4f_abs(dists.simd), radii.simd));

	// A2 x B2
	radii.x = box->halfExtents.x*absDotAxes.values[1][2] +
		box->halfExtents.y*absDotAxes.values[0][2] +
		otherBox->halfExtents.x*absDotAxes.values[2][1] +
		otherBox->halfExtents.y*absDotAxes.values[2][0];
	dists.x = dotDiffAxes.y*dotAxes.values[0][2] - dotDiffAxes.x*dotAxes.values[1][2];

#if DS_SIMD_ALWAYS_INT
	return dsSIMD4fb_all(radiusCmp.simd) && fabs(dists.x) <= radii.x;
#else
	return radiusCmp.x && radiusCmp.y && radiusCmp.z && radiusCmp.w && fabs(dists.x) <= radii.x;
#endif
}
#endif

#if DS_SIMD_ALWAYS_DOUBLE2
#if DS_X86
#define DS_SIMD_SHUFFLE_00(a, b) _mm_shuffle_pd((a), (b), _MM_SHUFFLE2(0, 0))
#define DS_SIMD_SHUFFLE_10(a, b) _mm_shuffle_pd((a), (b), _MM_SHUFFLE2(1, 0))
#elif DS_ARM_64
#define DS_SIMD_SHUFFLE_00(a, b) vtrn1q_f64((a), (b))
#define DS_SIMD_SHUFFLE_10(a, b) vextq_f64((a), (b), 1)
#else
#error Need to implement vector combination for this platform.
#endif

static inline bool dsOrientedBox3xd_intersectsSIMD2(
	const dsOrientedBox3xd* box, const dsOrientedBox3xd* otherBox)
{
	dsMatrix33xd orientationTrans, otherOrientationTrans;
	dsMatrix33xd_transposeSIMD2(&orientationTrans, &box->orientation);
	dsMatrix33xd_transposeSIMD2(&otherOrientationTrans, &otherBox->orientation);

	dsMatrix33xd dotAxes;
	dsMatrix33xd_mul(&dotAxes, &otherOrientationTrans, &box->orientation);

	dsMatrix33xd absDotAxes;
	absDotAxes.columns[0].simd2[0] = dsSIMD2d_abs(dotAxes.columns[0].simd2[0]);
	absDotAxes.columns[0].simd2[1] = dsSIMD2d_abs(dotAxes.columns[0].simd2[1]);
	absDotAxes.columns[1].simd2[0] = dsSIMD2d_abs(dotAxes.columns[1].simd2[0]);
	absDotAxes.columns[1].simd2[1] = dsSIMD2d_abs(dotAxes.columns[1].simd2[1]);
	absDotAxes.columns[2].simd2[0] = dsSIMD2d_abs(dotAxes.columns[2].simd2[0]);
	absDotAxes.columns[2].simd2[1] = dsSIMD2d_abs(dotAxes.columns[2].simd2[1]);

	dsVector3xd centerDiff, dotDiffAxes;
	dsVector3xd_sub(&centerDiff, &otherBox->center, &box->center);
	dsMatrix33xd_transform(&dotDiffAxes, &orientationTrans, &centerDiff);

	dsVector3xd radii, dists;
	dsMatrix33xd_transformTransposed(&radii, &absDotAxes, &otherBox->halfExtents);
	radii.simd2[0] = dsSIMD2d_add(radii.simd2[0], box->halfExtents.simd2[0]);
	radii.simd2[1] = dsSIMD2d_add(radii.simd2[1], box->halfExtents.simd2[1]);
	dsMatrix33xd_transform(&dists, &orientationTrans, &centerDiff);

	// radiusCmp is for outside radius for this case.
	dsVector4l radiusCmp;
	radiusCmp.simd2[0] = dsSIMD2d_cmpgt(dsSIMD2d_abs(dists.simd2[0]), radii.simd2[0]);
	radiusCmp.simd2[1] = dsSIMD2d_cmpgt(dsSIMD2d_abs(dists.simd2[1]), radii.simd2[1]);

	dsMatrix33xd_transform(&radii, &absDotAxes, &box->halfExtents);
	radii.simd2[0] = dsSIMD2d_add(radii.simd2[0], otherBox->halfExtents.simd2[0]);
	radii.simd2[1] = dsSIMD2d_add(radii.simd2[1], otherBox->halfExtents.simd2[1]);
	dsMatrix33xd_transform(&dists, &otherOrientationTrans, &centerDiff);

	radiusCmp.simd2[0] = dsSIMD2db_or(
		radiusCmp.simd2[0], dsSIMD2d_cmpgt(dsSIMD2d_abs(dists.simd2[0]), radii.simd2[0]));
	radiusCmp.simd2[1] = dsSIMD2db_or(
		radiusCmp.simd2[1], dsSIMD2d_cmpgt(dsSIMD2d_abs(dists.simd2[1]), radii.simd2[1]));
	if (dsSIMD2db_any(radiusCmp.simd2[0]) || radiusCmp.z)
		return false;

	// When there's a parallel set of axes it degenerates to 2D.
	dsSIMD2d parallelCutoff = dsSIMD2d_set1(PARALLEL_CUTOFFd);
	dsVector4l hasParallel;
	hasParallel.simd2[0] = dsSIMD2db_or(dsSIMD2db_or(
			dsSIMD2d_cmpge(absDotAxes.columns[0].simd2[0], parallelCutoff),
			dsSIMD2d_cmpge(absDotAxes.columns[1].simd2[0], parallelCutoff)),
		dsSIMD2d_cmpge(absDotAxes.columns[3].simd2[0], parallelCutoff));
	hasParallel.simd2[1] = dsSIMD2db_or(dsSIMD2db_or(
			dsSIMD2d_cmpge(absDotAxes.columns[0].simd2[1], parallelCutoff),
			dsSIMD2d_cmpge(absDotAxes.columns[1].simd2[1], parallelCutoff)),
		dsSIMD2d_cmpge(absDotAxes.columns[3].simd2[1], parallelCutoff));
	if (dsSIMD2db_any(hasParallel.simd2[0]) || hasParallel.z)
		return true;

	dsSIMD2d dotDiffAxesX = dsSIMD2d_set1FromVec(dotDiffAxes.simd2[0], 0);
	dsSIMD2d dotDiffAxesY = dsSIMD2d_set1FromVec(dotDiffAxes.simd2[0], 1);
	dsSIMD2d dotDiffAxesZ = dsSIMD2d_set1FromVec(dotDiffAxes.simd2[1], 0);

	// A0 x B0, A0 x B1, A0 x B2, A1 x B0
	// radiusCmp is for inside radius for these and the following cases.
	radii.x = box->halfExtents.y*absDotAxes.values[2][0] +
		box->halfExtents.z*absDotAxes.values[1][0] +
		otherBox->halfExtents.y*absDotAxes.values[0][2] +
		otherBox->halfExtents.z*absDotAxes.values[0][1];
	radii.y = box->halfExtents.y*absDotAxes.values[2][1] +
		box->halfExtents.z*absDotAxes.values[1][1] +
		otherBox->halfExtents.x*absDotAxes.values[0][2] +
		otherBox->halfExtents.z*absDotAxes.values[0][0];
	radii.z = box->halfExtents.y*absDotAxes.values[2][2] +
		box->halfExtents.z*absDotAxes.values[1][2] +
		otherBox->halfExtents.x*absDotAxes.values[0][1] +
		otherBox->halfExtents.y*absDotAxes.values[0][0];
	radii.w = box->halfExtents.x*absDotAxes.values[2][0] +
		box->halfExtents.z*absDotAxes.values[0][0] +
		otherBox->halfExtents.y*absDotAxes.values[1][2] +
		otherBox->halfExtents.z*absDotAxes.values[1][1];

	dsSIMD2d dotDiffAxesZX = DS_SIMD_SHUFFLE_00(dotDiffAxes.simd2[1], dotDiffAxes.simd2[0]);
	dsSIMD2d dotDiffAxesYZ = DS_SIMD_SHUFFLE_10(dotDiffAxes.simd2[0], dotDiffAxes.simd2[1]);
	dsSIMD2d dotAxes12_20 = DS_SIMD_SHUFFLE_00(
		dotAxes.columns[1].simd2[1], dotAxes.columns[2].simd2[0]);
	dsSIMD2d dotAxes22_00 = DS_SIMD_SHUFFLE_00(
		dotAxes.columns[2].simd2[1], dotAxes.columns[0].simd2[0]);
#if DS_SIMD_ALWAYS_FMA
	dists.simd2[0] = dsSIMD2d_fnmadd(dotDiffAxesY, dotAxes.columns[2].simd2[0],
		dsSIMD2d_mul(dotDiffAxesZ, dotAxes.columns[1].simd2[0]));
	dists.simd2[1] = dsSIMD2d_fnmadd(
		dotDiffAxesYZ, dotAxes22_00, dsSIMD2d_mul(dotDiffAxesZX, dotAxes12_20));
#else
	dists.simd2[0] = dsSIMD2d_sub(dsSIMD2d_mul(dotDiffAxesZ, dotAxes.columns[1].simd2[0]),
		dsSIMD2d_mul(dotDiffAxesY, dotAxes.columns[2].simd2[0]));
	dists.simd2[1] = dsSIMD2d_sub(dsSIMD2d_mul(dotDiffAxesZX, dotAxes12_20),
		dsSIMD2d_mul(dotDiffAxesYZ, dotAxes22_00));
#endif
	radiusCmp.simd2[0] = dsSIMD2d_cmple(dsSIMD2d_abs(dists.simd2[0]), radii.simd2[0]);
	radiusCmp.simd2[1] = dsSIMD2d_cmple(dsSIMD2d_abs(dists.simd2[1]), radii.simd2[1]);

	// A1 x B1, A1 x B2, A2 x B0, A2 x B1
	radii.x = box->halfExtents.x*absDotAxes.values[2][1] +
		box->halfExtents.z*absDotAxes.values[0][1] +
		otherBox->halfExtents.x*absDotAxes.values[1][2] +
		otherBox->halfExtents.z*absDotAxes.values[1][0];
	radii.y =  box->halfExtents.x*absDotAxes.values[2][2] +
		box->halfExtents.z*absDotAxes.values[0][2] +
		otherBox->halfExtents.x*absDotAxes.values[1][1] +
		otherBox->halfExtents.y*absDotAxes.values[1][0];
	radii.z = box->halfExtents.x*absDotAxes.values[1][0] +
		box->halfExtents.y*absDotAxes.values[0][0] +
		otherBox->halfExtents.y*absDotAxes.values[2][2] +
		otherBox->halfExtents.z*absDotAxes.values[2][1];
	radii.w = box->halfExtents.x*absDotAxes.values[1][1] +
		box->halfExtents.y*absDotAxes.values[0][1] +
		otherBox->halfExtents.x*absDotAxes.values[2][2] +
		otherBox->halfExtents.z*absDotAxes.values[2][0];

	dsSIMD2d dotAxes2_12 = DS_SIMD_SHUFFLE_10(
		dotAxes.columns[2].simd2[0], dotAxes.columns[2].simd2[1]);
	dsSIMD2d dotAxes0_12 = DS_SIMD_SHUFFLE_10(
		dotAxes.columns[0].simd2[0], dotAxes.columns[0].simd2[1]);
#if DS_SIMD_ALWAYS_FMA
	dists.simd2[0] = dsSIMD2d_fnmadd(dotDiffAxesZ, dotAxes0_12,
		dsSIMD2d_mul(dotDiffAxesX, dotAxes2_12));
	dists.simd2[1] = dsSIMD2d_fnmadd(dotDiffAxesX, dotAxes.columns[1].simd2[0],
		dsSIMD2d_mul(dotDiffAxesY, dotAxes.columns[0].simd2[0]));
#else
	dists.simd2[0] = dsSIMD2d_sub(dsSIMD2d_mul(dotDiffAxesX, dotAxes2_12),
		dsSIMD2d_mul(dotDiffAxesZ, dotAxes0_12));
	dists.simd2[1] = dsSIMD2d_sub(dsSIMD2d_mul(dotDiffAxesY, dotAxes.columns[0].simd2[0]),
		dsSIMD2d_mul(dotDiffAxesX, dotAxes.columns[1].simd2[0]));
#endif
	radiusCmp.simd2[0] = dsSIMD2db_and(
		radiusCmp.simd2[0], dsSIMD2d_cmple(dsSIMD2d_abs(dists.simd2[0]), radii.simd2[0]));
	radiusCmp.simd2[1] = dsSIMD2db_and(
		radiusCmp.simd2[1], dsSIMD2d_cmple(dsSIMD2d_abs(dists.simd2[1]), radii.simd2[1]));

	// A2 x B2
	radii.x = box->halfExtents.x*absDotAxes.values[1][2] +
		box->halfExtents.y*absDotAxes.values[0][2] +
		otherBox->halfExtents.x*absDotAxes.values[2][1] +
		otherBox->halfExtents.y*absDotAxes.values[2][0];
	dists.x = dotDiffAxes.y*dotAxes.values[0][2] - dotDiffAxes.x*dotAxes.values[1][2];

	radiusCmp.simd2[0] = dsSIMD2db_and(radiusCmp.simd2[0], radiusCmp.simd2[1]);
	return dsSIMD2db_all(radiusCmp.simd2[0]) && fabs(dists.x) <= radii.x;
}
#endif

bool dsOrientedBox3xf_transform(dsOrientedBox3xf* box, const dsMatrix44f* transform)
{
	DS_ASSERT(box);
	DS_ASSERT(transform);

	if (!dsOrientedBox3xf_isValid(box))
		return false;

	dsMatrix44f matrix, transformedMatrix;
	dsOrientedBox3xf_toMatrix(&matrix, box);
	dsMatrix44f_affineMul(&transformedMatrix, transform, &matrix);
	dsOrientedBox3xf_fromMatrix(box, &transformedMatrix);
	return true;
}

bool dsOrientedBox3xd_transform(dsOrientedBox3xd* box, const dsMatrix44d* transform)
{
	DS_ASSERT(box);
	DS_ASSERT(transform);

	if (!dsOrientedBox3xd_isValid(box))
		return false;

	dsMatrix44d matrix, transformedMatrix;
	dsOrientedBox3xd_toMatrix(&matrix, box);
	dsMatrix44d_affineMul(&transformedMatrix, transform, &matrix);
	dsOrientedBox3xd_fromMatrix(box, &transformedMatrix);
	return true;
}

void dsOrientedBox3xf_addPoint(dsOrientedBox3xf* box, const dsVector3xf* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (dsOrientedBox3xf_isValid(box))
	{
		dsVector3xf localPoint;
		dsVector3xf centeredPoint;
		dsVector3xf_sub(&centeredPoint, point, &box->center);
		dsMatrix33xf_transformTransposed(&localPoint, &box->orientation, &centeredPoint);

		dsAlignedBox3xf localBox;
		dsVector3xf_neg(&localBox.min, &box->halfExtents);
		localBox.max = box->halfExtents;

		dsAlignedBox3xf_addPoint(&localBox, &localPoint);

		dsVector3xf localCenterOffset, centerOffset;
		dsAlignedBox3xf_center(&localCenterOffset, &localBox);
		dsMatrix33xf_transform(&centerOffset, &box->orientation, &localCenterOffset);
		dsVector3xf_add(&box->center, &box->center, &centerOffset);

		dsAlignedBox3xf_extents(&box->halfExtents, &localBox);
		dsVector3xf_scale(&box->halfExtents, &box->halfExtents, 0.5f);
	}
	else
	{
		box->center = *point;
#if DS_SIMD_ALWAYS_FLOAT4
		box->halfExtents.simd = dsSIMD4f_set1(0.0f);
#else
		box->halfExtents.x = 0;
		box->halfExtents.y = 0;
		box->halfExtents.z = 0;
		box->halfExtents.w = 0;
#endif
	}
}

void dsOrientedBox3xd_addPoint(dsOrientedBox3xd* box, const dsVector3xd* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (dsOrientedBox3xd_isValid(box))
	{
		dsVector3xd localPoint;
		dsVector3xd centeredPoint;
		dsVector3xd_sub(&centeredPoint, point, &box->center);
		dsMatrix33xd_transformTransposed(&localPoint, &box->orientation, &centeredPoint);

		dsAlignedBox3xd localBox;
		dsVector3xd_neg(&localBox.min, &box->halfExtents);
		localBox.max = box->halfExtents;

		dsAlignedBox3xd_addPoint(&localBox, &localPoint);

		dsVector3xd localCenterOffset, centerOffset;
		dsAlignedBox3xd_center(&localCenterOffset, &localBox);
		dsMatrix33xd_transform(&centerOffset, &box->orientation, &localCenterOffset);
		dsVector3xd_add(&box->center, &box->center, &centerOffset);

		dsAlignedBox3xd_extents(&box->halfExtents, &localBox);
		dsVector3xd_scale(&box->halfExtents, &box->halfExtents, 0.5);
	}
	else
	{
		box->center = *point;
#if DS_SIMD_ALWAYS_DOUBLE2
		box->halfExtents.simd2[0] = box->halfExtents.simd2[1] = dsSIMD2d_set1(0.0);
#else
		box->halfExtents.x = 0;
		box->halfExtents.y = 0;
		box->halfExtents.z = 0;
		box->halfExtents.w = 0;
#endif
	}
}

bool dsOrientedBox3xf_addBox(dsOrientedBox3xf* box, const dsOrientedBox3xf* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);

	if (!dsOrientedBox3xf_isValid(otherBox))
		return false;

	if (dsOrientedBox3xf_isValid(box))
	{
		dsAlignedBox3xf localBox;
		dsVector3xf_neg(&localBox.min, &box->halfExtents);
		localBox.max = box->halfExtents;

		// Transposing first will be faster when using SIMD.
		dsMatrix33xf orientationTrans;
		dsMatrix33xf_transpose(&orientationTrans, &box->orientation);

		dsVector3xf corners[DS_BOX3_CORNER_COUNT];
		DS_VERIFY(dsOrientedBox3xf_corners(corners, otherBox));
		for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
		{
			dsVector3xf localCorner;
			dsVector3xf centeredPoint;
			dsVector3xf_sub(&centeredPoint, corners + i, &box->center);
			dsMatrix33xf_transform(&localCorner, &orientationTrans, &centeredPoint);

			dsAlignedBox3xf_addPoint(&localBox, &localCorner);
		}

		dsVector3xf localCenterOffset, centerOffset;
		dsAlignedBox3xf_center(&localCenterOffset, &localBox);
		dsMatrix33xf_transform(&centerOffset, &box->orientation, &localCenterOffset);
		dsVector3xf_add(&box->center, &box->center, &centerOffset);

		dsAlignedBox3xf_extents(&box->halfExtents, &localBox);
		dsVector3xf_scale(&box->halfExtents, &box->halfExtents, 0.5f);
	}
	else
		*box = *otherBox;

	return true;
}

bool dsOrientedBox3xd_addBox(dsOrientedBox3xd* box, const dsOrientedBox3xd* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);

	if (!dsOrientedBox3xd_isValid(otherBox))
		return false;

	if (dsOrientedBox3xd_isValid(box))
	{
		dsAlignedBox3xd localBox;
		dsVector3xd_neg(&localBox.min, &box->halfExtents);
		localBox.max = box->halfExtents;

		// Transposing first will be faster when using SIMD.
		dsMatrix33xd orientationTrans;
		dsMatrix33xd_transpose(&orientationTrans, &box->orientation);

		dsVector3xd corners[DS_BOX3_CORNER_COUNT];
		DS_VERIFY(dsOrientedBox3xd_corners(corners, otherBox));
		for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
		{
			dsVector3xd localCorner;
			dsVector3xd centeredPoint;
			dsVector3xd_sub(&centeredPoint, corners + i, &box->center);
			dsMatrix33xd_transform(&localCorner, &orientationTrans, &centeredPoint);

			dsAlignedBox3xd_addPoint(&localBox, &localCorner);
		}

		dsVector3xd localCenterOffset, centerOffset;
		dsAlignedBox3xd_center(&localCenterOffset, &localBox);
		dsMatrix33xd_transform(&centerOffset, &box->orientation, &localCenterOffset);
		dsVector3xd_add(&box->center, &box->center, &centerOffset);

		dsAlignedBox3xd_extents(&box->halfExtents, &localBox);
		dsVector3xd_scale(&box->halfExtents, &box->halfExtents, 0.5);
	}
	else
		*box = *otherBox;

	return true;
}

bool dsOrientedBox3xf_corners(
	dsVector3xf corners[DS_BOX3_CORNER_COUNT], const dsOrientedBox3xf* box)
{
	DS_ASSERT(corners);
	DS_ASSERT(box);

	if (!dsOrientedBox3xf_isValid(box))
		return false;

#if DS_SIMD_ALWAYS_FLOAT4
	corners[dsBox3Corner_xyz].simd = dsSIMD4f_neg(box->halfExtents.simd);
	corners[dsBox3Corner_xyZ].simd = dsSIMD4f_negComponents(
		box->halfExtents.simd, true, true, false, false);
	corners[dsBox3Corner_xYz].simd = dsSIMD4f_negComponents(
		box->halfExtents.simd, true, false, true, false);
	corners[dsBox3Corner_xYZ].simd = dsSIMD4f_negComponents(
		box->halfExtents.simd, true, false, false, false);
	corners[dsBox3Corner_Xyz].simd = dsSIMD4f_negComponents(
		box->halfExtents.simd, false, true, true, false);
	corners[dsBox3Corner_XyZ].simd = dsSIMD4f_negComponents(
		box->halfExtents.simd, false, true, false, false);
	corners[dsBox3Corner_XYz].simd = dsSIMD4f_negComponents(
		box->halfExtents.simd, false, false, true, false);
	corners[dsBox3Corner_XYZ].simd = box->halfExtents.simd;
#else
	corners[dsBox3Corner_xyz].x = -box->halfExtents.x;
	corners[dsBox3Corner_xyz].y = -box->halfExtents.y;
	corners[dsBox3Corner_xyz].z = -box->halfExtents.z;

	corners[dsBox3Corner_xyZ].x = -box->halfExtents.x;
	corners[dsBox3Corner_xyZ].y = -box->halfExtents.y;
	corners[dsBox3Corner_xyZ].z = box->halfExtents.z;

	corners[dsBox3Corner_xYz].x = -box->halfExtents.x;
	corners[dsBox3Corner_xYz].y = box->halfExtents.y;
	corners[dsBox3Corner_xYz].z = -box->halfExtents.z;

	corners[dsBox3Corner_xYZ].x = -box->halfExtents.x;
	corners[dsBox3Corner_xYZ].y = box->halfExtents.y;
	corners[dsBox3Corner_xYZ].z = box->halfExtents.z;

	corners[dsBox3Corner_Xyz].x = box->halfExtents.x;
	corners[dsBox3Corner_Xyz].y = -box->halfExtents.y;
	corners[dsBox3Corner_Xyz].z = -box->halfExtents.z;

	corners[dsBox3Corner_XyZ].x = box->halfExtents.x;
	corners[dsBox3Corner_XyZ].y = -box->halfExtents.y;
	corners[dsBox3Corner_XyZ].z = box->halfExtents.z;

	corners[dsBox3Corner_XYz].x = box->halfExtents.x;
	corners[dsBox3Corner_XYz].y = box->halfExtents.y;
	corners[dsBox3Corner_XYz].z = -box->halfExtents.z;

	corners[dsBox3Corner_XYZ].x = box->halfExtents.x;
	corners[dsBox3Corner_XYZ].y = box->halfExtents.y;
	corners[dsBox3Corner_XYZ].z = box->halfExtents.z;
#endif

	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
	{
		dsVector3xf worldOffset;
		dsMatrix33xf_transform(&worldOffset, &box->orientation, corners + i);
		dsVector3xf_add(corners + i, &worldOffset, &box->center);
	}

	return true;
}

bool dsOrientedBox3xd_corners(
	dsVector3xd corners[DS_BOX3_CORNER_COUNT], const dsOrientedBox3xd* box)
{
	DS_ASSERT(corners);
	DS_ASSERT(box);

	if (!dsOrientedBox3xd_isValid(box))
		return false;

#if DS_SIMD_ALWAYS_DOUBLE2
	dsSIMD2d halfExtents0Neg = dsSIMD2d_neg(box->halfExtents.simd2[0]);
	dsSIMD2d halfExtents1Neg = dsSIMD2d_neg(box->halfExtents.simd2[1]);
	dsSIMD2d halfExtents0NegPos = dsSIMD2d_negComponents(box->halfExtents.simd2[0], true, false);
	dsSIMD2d halfExtents0PosNeg = dsSIMD2d_negComponents(box->halfExtents.simd2[0], false, true);
	corners[dsBox3Corner_xyz].simd2[0] = halfExtents0Neg;
	corners[dsBox3Corner_xyz].simd2[1] = halfExtents1Neg;
	corners[dsBox3Corner_xyZ].simd2[0] = halfExtents0Neg;
	corners[dsBox3Corner_xyZ].simd2[1] = box->halfExtents.simd2[1];
	corners[dsBox3Corner_xYz].simd2[0] = halfExtents0NegPos;
	corners[dsBox3Corner_xYz].simd2[1] = halfExtents1Neg;
	corners[dsBox3Corner_xYZ].simd2[0] = halfExtents0NegPos;
	corners[dsBox3Corner_xYZ].simd2[1] = box->halfExtents.simd2[1];
	corners[dsBox3Corner_Xyz].simd2[0] = halfExtents0PosNeg;
	corners[dsBox3Corner_Xyz].simd2[1] = halfExtents1Neg;
	corners[dsBox3Corner_XyZ].simd2[0] = halfExtents0PosNeg;
	corners[dsBox3Corner_XyZ].simd2[1] = box->halfExtents.simd2[1];
	corners[dsBox3Corner_XYz].simd2[0] = box->halfExtents.simd2[0];
	corners[dsBox3Corner_XYz].simd2[1] = halfExtents1Neg;
	corners[dsBox3Corner_XYZ].simd2[0] = box->halfExtents.simd2[0];
	corners[dsBox3Corner_XYZ].simd2[1] = box->halfExtents.simd2[1];
#else
	corners[dsBox3Corner_xyz].x = -box->halfExtents.x;
	corners[dsBox3Corner_xyz].y = -box->halfExtents.y;
	corners[dsBox3Corner_xyz].z = -box->halfExtents.z;

	corners[dsBox3Corner_xyZ].x = -box->halfExtents.x;
	corners[dsBox3Corner_xyZ].y = -box->halfExtents.y;
	corners[dsBox3Corner_xyZ].z = box->halfExtents.z;

	corners[dsBox3Corner_xYz].x = -box->halfExtents.x;
	corners[dsBox3Corner_xYz].y = box->halfExtents.y;
	corners[dsBox3Corner_xYz].z = -box->halfExtents.z;

	corners[dsBox3Corner_xYZ].x = -box->halfExtents.x;
	corners[dsBox3Corner_xYZ].y = box->halfExtents.y;
	corners[dsBox3Corner_xYZ].z = box->halfExtents.z;

	corners[dsBox3Corner_Xyz].x = box->halfExtents.x;
	corners[dsBox3Corner_Xyz].y = -box->halfExtents.y;
	corners[dsBox3Corner_Xyz].z = -box->halfExtents.z;

	corners[dsBox3Corner_XyZ].x = box->halfExtents.x;
	corners[dsBox3Corner_XyZ].y = -box->halfExtents.y;
	corners[dsBox3Corner_XyZ].z = box->halfExtents.z;

	corners[dsBox3Corner_XYz].x = box->halfExtents.x;
	corners[dsBox3Corner_XYz].y = box->halfExtents.y;
	corners[dsBox3Corner_XYz].z = -box->halfExtents.z;

	corners[dsBox3Corner_XYZ].x = box->halfExtents.x;
	corners[dsBox3Corner_XYZ].y = box->halfExtents.y;
	corners[dsBox3Corner_XYZ].z = box->halfExtents.z;
#endif

	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
	{
		dsVector3xd worldOffset;
		dsMatrix33xd_transform(&worldOffset, &box->orientation, corners + i);
		dsVector3xd_add(corners + i, &worldOffset, &box->center);
	}

	return true;
}

bool dsOrientedBox3xf_intersects(const dsOrientedBox3xf* box, const dsOrientedBox3xf* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);

	if (!dsOrientedBox3xf_isValid(box) || !dsOrientedBox3xf_isValid(otherBox))
		return false;

#if DS_SIMD_ALWAYS_FLOAT4
	return dsOrientedBox3xf_intersectsSIMD(box, otherBox);
#else
	// Optimized separating axes as explained by
	// https://www.geometrictools.com/Documentation/DynamicCollisionDetection.pdf
	// See also:
	// https://github.com/davideberly/GeometricTools/blob/master/GTE/Mathematics/IntrOrientedBox3OrientedBox3.h
	dsVector3xf centerDiff;
	dsVector3xf_sub(&centerDiff, &otherBox->center, &box->center);

	const float parallelCutoff = PARALLEL_CUTOFFf;
	float dotDiffAxes[3];
	float dotAxes[3][3];
	float absDotAxes[3][3];
	bool hasParallel = false;
	for (unsigned int i = 0; i < 3; ++i)
	{
		const dsVector3xf* boxAxis = box->orientation.columns + i;
		dotDiffAxes[i] = dsVector3xf_dot(&centerDiff, boxAxis);
		for (unsigned int j = 0; j < 3; ++j)
		{
			dotAxes[i][j] = dsVector3xf_dot(boxAxis, otherBox->orientation.columns + j);
			absDotAxes[i][j] = fabsf(dotAxes[i][j]);
			if (dotAxes[i][j] >= parallelCutoff)
				hasParallel = true;
		}

		// Test axes for first box against second box.
		float radius = box->halfExtents.values[i] + otherBox->halfExtents.x*absDotAxes[i][0] +
			otherBox->halfExtents.y*absDotAxes[i][1] + otherBox->halfExtents.z*absDotAxes[i][2];
		if (fabsf(dsVector3xf_dot(box->orientation.columns + i, &centerDiff)) > radius)
			return false;
	}

	// Test axes for second box against first box.
	for (unsigned int i = 0; i < 3; ++i)
	{
		float radius = box->halfExtents.x*absDotAxes[0][i] + box->halfExtents.y*absDotAxes[1][i] +
			box->halfExtents.z*absDotAxes[2][i] + otherBox->halfExtents.values[i];
		if (fabsf(dsVector3xf_dot(otherBox->orientation.columns + i, &centerDiff)) > radius)
			return false;
	}

	// When there's a parallel set of axes it degenerates to 2D.
	if (hasParallel)
		return true;

	// A0 x B0
	float radius = box->halfExtents.y*absDotAxes[2][0] + box->halfExtents.z*absDotAxes[1][0] +
		otherBox->halfExtents.y*absDotAxes[0][2] + otherBox->halfExtents.z*absDotAxes[0][1];
	if (fabsf(dotDiffAxes[2]*dotAxes[1][0] - dotDiffAxes[1]*dotAxes[2][0]) > radius)
		return false;

	// A0 x B1
	radius = box->halfExtents.y*absDotAxes[2][1] + box->halfExtents.z*absDotAxes[1][1] +
		otherBox->halfExtents.x*absDotAxes[0][2] + otherBox->halfExtents.z*absDotAxes[0][0];
	if (fabsf(dotDiffAxes[2]*dotAxes[1][1] - dotDiffAxes[1]*dotAxes[2][1]) > radius)
		return false;

	// A0 x B2
	radius = box->halfExtents.y*absDotAxes[2][2] + box->halfExtents.z*absDotAxes[1][2] +
		otherBox->halfExtents.x*absDotAxes[0][1] + otherBox->halfExtents.y*absDotAxes[0][0];
	if (fabsf(dotDiffAxes[2]*dotAxes[1][2] - dotDiffAxes[1]*dotAxes[2][2]) > radius)
		return false;

	// A1 x B0
	radius = box->halfExtents.x*absDotAxes[2][0] + box->halfExtents.z*absDotAxes[0][0] +
		otherBox->halfExtents.y*absDotAxes[1][2] + otherBox->halfExtents.z*absDotAxes[1][1];
	if (fabsf(dotDiffAxes[0]*dotAxes[2][0] - dotDiffAxes[2]*dotAxes[0][0]) > radius)
		return false;

	// A1 x B1
	radius = box->halfExtents.x*absDotAxes[2][1] + box->halfExtents.z*absDotAxes[0][1] +
		otherBox->halfExtents.x*absDotAxes[1][2] + otherBox->halfExtents.z*absDotAxes[1][0];
	if (fabsf(dotDiffAxes[0]*dotAxes[2][1] - dotDiffAxes[2]*dotAxes[0][1]) > radius)
		return false;

	// A1 x B2
	radius = box->halfExtents.x*absDotAxes[2][2] + box->halfExtents.z*absDotAxes[0][2] +
		otherBox->halfExtents.x*absDotAxes[1][1] + otherBox->halfExtents.y*absDotAxes[1][0];
	if (fabsf(dotDiffAxes[0]*dotAxes[2][2] - dotDiffAxes[2]*dotAxes[0][2]) > radius)
		return false;

	// A2 x B0
	radius = box->halfExtents.x*absDotAxes[1][0] + box->halfExtents.y*absDotAxes[0][0] +
		otherBox->halfExtents.y*absDotAxes[2][2] + otherBox->halfExtents.z*absDotAxes[2][1];
	if (fabsf(dotDiffAxes[1]*dotAxes[0][0] - dotDiffAxes[0]*dotAxes[1][0]) > radius)
		return false;

	// A2 x B1
	radius = box->halfExtents.x*absDotAxes[1][1] + box->halfExtents.y*absDotAxes[0][1] +
		otherBox->halfExtents.x*absDotAxes[2][2] + otherBox->halfExtents.z*absDotAxes[2][0];
	if (fabsf(dotDiffAxes[1]*dotAxes[0][1] - dotDiffAxes[0]*dotAxes[1][1]) > radius)
		return false;

	// A2 x B2
	radius = box->halfExtents.x*absDotAxes[1][2] + box->halfExtents.y*absDotAxes[0][2] +
		otherBox->halfExtents.x*absDotAxes[2][1] + otherBox->halfExtents.y*absDotAxes[2][0];
	if (fabsf(dotDiffAxes[1]*dotAxes[0][2] - dotDiffAxes[0]*dotAxes[1][2]) > radius)
		return false;

	return true;
#endif
}

bool dsOrientedBox3xd_intersects(const dsOrientedBox3xd* box, const dsOrientedBox3xd* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);

	if (!dsOrientedBox3xd_isValid(box) || !dsOrientedBox3xd_isValid(otherBox))
		return false;

#if DS_SIMD_ALWAYS_DOUBLE2
	return dsOrientedBox3xd_intersectsSIMD2(box, otherBox);
#else
	// Optimized separating axes as explained by
	// https://www.geometrictools.com/Documentation/DynamicCollisionDetection.pdf
	// See also:
	// https://github.com/davideberly/GeometricTools/blob/master/GTE/Mathematics/IntrOrientedBox3OrientedBox3.h
	dsVector3xd centerDiff;
	dsVector3xd_sub(&centerDiff, &otherBox->center, &box->center);

	const double parallelCutoff = PARALLEL_CUTOFFd;
	double dotDiffAxes[3];
	double dotAxes[3][3];
	double absDotAxes[3][3];
	bool hasParallel = false;
	for (unsigned int i = 0; i < 3; ++i)
	{
		const dsVector3xd* boxAxis = box->orientation.columns + i;
		dotDiffAxes[i] = dsVector3xd_dot(&centerDiff, boxAxis);
		for (unsigned int j = 0; j < 3; ++j)
		{
			dotAxes[i][j] = dsVector3xd_dot(boxAxis, otherBox->orientation.columns + j);
			absDotAxes[i][j] = fabs(dotAxes[i][j]);
			if (dotAxes[i][j] >= parallelCutoff)
				hasParallel = true;
		}

		// Test axes for first box against second box.
		double radius = box->halfExtents.values[i] + otherBox->halfExtents.x*absDotAxes[i][0] +
			otherBox->halfExtents.y*absDotAxes[i][1] + otherBox->halfExtents.z*absDotAxes[i][2];
		if (fabs(dsVector3xd_dot(box->orientation.columns + i, &centerDiff)) > radius)
			return false;
	}

	// Test axes for second box against first box.
	for (unsigned int i = 0; i < 3; ++i)
	{
		double radius = box->halfExtents.x*absDotAxes[0][i] + box->halfExtents.y*absDotAxes[1][i] +
			box->halfExtents.z*absDotAxes[2][i] + otherBox->halfExtents.values[i];
		if (fabs(dsVector3xd_dot(otherBox->orientation.columns + i, &centerDiff)) > radius)
			return false;
	}

	// When there's a parallel set of axes it degenerates to 2D.
	if (hasParallel)
		return true;

	// A0 x B0
	double radius = box->halfExtents.y*absDotAxes[2][0] + box->halfExtents.z*absDotAxes[1][0] +
		otherBox->halfExtents.y*absDotAxes[0][2] + otherBox->halfExtents.z*absDotAxes[0][1];
	if (fabs(dotDiffAxes[2]*dotAxes[1][0] - dotDiffAxes[1]*dotAxes[2][0]) > radius)
		return false;

	// A0 x B1
	radius = box->halfExtents.y*absDotAxes[2][1] + box->halfExtents.z*absDotAxes[1][1] +
		otherBox->halfExtents.x*absDotAxes[0][2] + otherBox->halfExtents.z*absDotAxes[0][0];
	if (fabs(dotDiffAxes[2]*dotAxes[1][1] - dotDiffAxes[1]*dotAxes[2][1]) > radius)
		return false;

	// A0 x B2
	radius = box->halfExtents.y*absDotAxes[2][2] + box->halfExtents.z*absDotAxes[1][2] +
		otherBox->halfExtents.x*absDotAxes[0][1] + otherBox->halfExtents.y*absDotAxes[0][0];
	if (fabs(dotDiffAxes[2]*dotAxes[1][2] - dotDiffAxes[1]*dotAxes[2][2]) > radius)
		return false;

	// A1 x B0
	radius = box->halfExtents.x*absDotAxes[2][0] + box->halfExtents.z*absDotAxes[0][0] +
		otherBox->halfExtents.y*absDotAxes[1][2] + otherBox->halfExtents.z*absDotAxes[1][1];
	if (fabs(dotDiffAxes[0]*dotAxes[2][0] - dotDiffAxes[2]*dotAxes[0][0]) > radius)
		return false;

	// A1 x B1
	radius = box->halfExtents.x*absDotAxes[2][1] + box->halfExtents.z*absDotAxes[0][1] +
		otherBox->halfExtents.x*absDotAxes[1][2] + otherBox->halfExtents.z*absDotAxes[1][0];
	if (fabs(dotDiffAxes[0]*dotAxes[2][1] - dotDiffAxes[2]*dotAxes[0][1]) > radius)
		return false;

	// A1 x B2
	radius = box->halfExtents.x*absDotAxes[2][2] + box->halfExtents.z*absDotAxes[0][2] +
		otherBox->halfExtents.x*absDotAxes[1][1] + otherBox->halfExtents.y*absDotAxes[1][0];
	if (fabs(dotDiffAxes[0]*dotAxes[2][2] - dotDiffAxes[2]*dotAxes[0][2]) > radius)
		return false;

	// A2 x B0
	radius = box->halfExtents.x*absDotAxes[1][0] + box->halfExtents.y*absDotAxes[0][0] +
		otherBox->halfExtents.y*absDotAxes[2][2] + otherBox->halfExtents.z*absDotAxes[2][1];
	if (fabs(dotDiffAxes[1]*dotAxes[0][0] - dotDiffAxes[0]*dotAxes[1][0]) > radius)
		return false;

	// A2 x B1
	radius = box->halfExtents.x*absDotAxes[1][1] + box->halfExtents.y*absDotAxes[0][1] +
		otherBox->halfExtents.x*absDotAxes[2][2] + otherBox->halfExtents.z*absDotAxes[2][0];
	if (fabs(dotDiffAxes[1]*dotAxes[0][1] - dotDiffAxes[0]*dotAxes[1][1]) > radius)
		return false;

	// A2 x B2
	radius = box->halfExtents.x*absDotAxes[1][2] + box->halfExtents.y*absDotAxes[0][2] +
		otherBox->halfExtents.x*absDotAxes[2][1] + otherBox->halfExtents.y*absDotAxes[2][0];
	if (fabs(dotDiffAxes[1]*dotAxes[0][2] - dotDiffAxes[0]*dotAxes[1][2]) > radius)
		return false;

	return true;
#endif
}

bool dsOrientedBox3xf_containsPoint(const dsOrientedBox3xf* box, const dsVector3xf* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox3xf_isValid(box))
		return false;

	dsAlignedBox3xf localBox;
	dsVector3xf_neg(&localBox.min, &box->halfExtents);
	localBox.max = box->halfExtents;

	dsVector3xf localPoint;
	dsVector3xf centeredPoint;
	dsVector3xf_sub(&centeredPoint, point, &box->center);
	dsMatrix33xf_transformTransposed(&localPoint, &box->orientation, &centeredPoint);
	return dsAlignedBox3xf_containsPoint(&localBox, &localPoint);
}

bool dsOrientedBox3xd_containsPoint(const dsOrientedBox3xd* box, const dsVector3xd* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox3xd_isValid(box))
		return false;

	dsAlignedBox3xd localBox;
	dsVector3xd_neg(&localBox.min, &box->halfExtents);
	localBox.max = box->halfExtents;

	dsVector3xd localPoint;
	dsVector3xd centeredPoint;
	dsVector3xd_sub(&centeredPoint, point, &box->center);
	dsMatrix33xd_transformTransposed(&localPoint, &box->orientation, &centeredPoint);
	return dsAlignedBox3xd_containsPoint(&localBox, &localPoint);
}

bool dsOrientedBox3xf_closestPoint(
	dsVector3xf* result, const dsOrientedBox3xf* box, const dsVector3xf* point)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox3xf_isValid(box))
		return false;

	dsAlignedBox3xf localBox;
	dsVector3xf_neg(&localBox.min, &box->halfExtents);
	localBox.max = box->halfExtents;

	dsVector3xf localPoint;
	dsVector3xf centeredPoint;
	dsVector3xf_sub(&centeredPoint, point, &box->center);
	dsMatrix33xf_transformTransposed(&localPoint, &box->orientation, &centeredPoint);

	dsVector3xf localResult;
	dsAlignedBox3xf_closestPoint(&localResult, &localBox, &localPoint);
	dsMatrix33xf_transform(result, &box->orientation, &localResult);
	dsVector3xf_add(result, result, &box->center);

	return true;
}

bool dsOrientedBox3xd_closestPoint(
	dsVector3xd* result, const dsOrientedBox3xd* box, const dsVector3xd* point)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox3xd_isValid(box))
		return false;

	dsAlignedBox3xd localBox;
	dsVector3xd_neg(&localBox.min, &box->halfExtents);
	localBox.max = box->halfExtents;

	dsVector3xd localPoint;
	dsVector3xd centeredPoint;
	dsVector3xd_sub(&centeredPoint, point, &box->center);
	dsMatrix33xd_transformTransposed(&localPoint, &box->orientation, &centeredPoint);

	dsVector3xd localResult;
	dsAlignedBox3xd_closestPoint(&localResult, &localBox, &localPoint);
	dsMatrix33xd_transform(result, &box->orientation, &localResult);
	dsVector3xd_add(result, result, &box->center);

	return true;
}

float dsOrientedBox3xf_dist2(const dsOrientedBox3xf* box, const dsVector3xf* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox3xf_isValid(box))
		return -1;

	dsAlignedBox3xf localBox;
	dsVector3xf_neg(&localBox.min, &box->halfExtents);
	localBox.max = box->halfExtents;

	dsVector3xf localPoint;
	dsVector3xf centeredPoint;
	dsVector3xf_sub(&centeredPoint, point, &box->center);
	dsMatrix33xf_transformTransposed(&localPoint, &box->orientation, &centeredPoint);

	return dsAlignedBox3xf_dist2(&localBox, &localPoint);
}

double dsOrientedBox3xd_dist2(const dsOrientedBox3xd* box, const dsVector3xd* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox3xd_isValid(box))
		return -1;

	dsAlignedBox3xd localBox;
	dsVector3xd_neg(&localBox.min, &box->halfExtents);
	localBox.max = box->halfExtents;

	dsVector3xd localPoint;
	dsVector3xd centeredPoint;
	dsVector3xd_sub(&centeredPoint, point, &box->center);
	dsMatrix33xd_transformTransposed(&localPoint, &box->orientation, &centeredPoint);

	return dsAlignedBox3xd_dist2(&localBox, &localPoint);
}

float dsOrientedBox3xf_dist(const dsOrientedBox3xf* box, const dsVector3xf* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox3xf_isValid(box))
		return -1;

	dsAlignedBox3xf localBox;
	dsVector3xf_neg(&localBox.min, &box->halfExtents);
	localBox.max = box->halfExtents;

	dsVector3xf localPoint;
	dsVector3xf centeredPoint;
	dsVector3xf_sub(&centeredPoint, point, &box->center);
	dsMatrix33xf_transformTransposed(&localPoint, &box->orientation, &centeredPoint);

	return dsAlignedBox3xf_dist(&localBox, &localPoint);
}

double dsOrientedBox3xd_dist(const dsOrientedBox3xd* box, const dsVector3xd* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox3xd_isValid(box))
		return -1;

	dsAlignedBox3xd localBox;
	dsVector3xd_neg(&localBox.min, &box->halfExtents);
	localBox.max = box->halfExtents;

	dsVector3xd localPoint;
	dsVector3xd centeredPoint;
	dsVector3xd_sub(&centeredPoint, point, &box->center);
	dsMatrix33xd_transformTransposed(&localPoint, &box->orientation, &centeredPoint);

	return dsAlignedBox3xd_dist(&localBox, &localPoint);
}

bool dsOrientedBox3xf_isValid(const dsOrientedBox3xf* box);
bool dsOrientedBox3xd_isValid(const dsOrientedBox3xd* box);

void dsOrientedBox3xf_fromAlignedBox(dsOrientedBox3xf* result, const dsAlignedBox3xf* alignedBox);
void dsOrientedBox3xd_fromAlignedBox(dsOrientedBox3xd* result, const dsAlignedBox3xd* alignedBox);

void dsOrientedBox3xf_toMatrix(dsMatrix44f* result, const dsOrientedBox3xf* box);
void dsOrientedBox3xd_toMatrix(dsMatrix44d* result, const dsOrientedBox3xd* box);

void dsOrientedBox3xf_toMatrixTranspose(dsMatrix44f* result, const dsOrientedBox3xf* box);
void dsOrientedBox3xd_toMatrixTranspose(dsMatrix44d* result, const dsOrientedBox3xd* box);

void dsOrientedBox3xf_makeInvalid(dsOrientedBox3xf* result);
void dsOrientedBox3xd_makeInvalid(dsOrientedBox3xd* result);

void dsOrientedBox3xf_fromMatrix(dsOrientedBox3xf* result, const dsMatrix44f* matrix);
void dsOrientedBox3xd_fromMatrix(dsOrientedBox3xd* result, const dsMatrix44d* matrix);

#if DS_HAS_SIMD
void dsOrientedBox3xf_toMatrixSIMD(dsMatrix44f* result, const dsOrientedBox3xf* box);
void dsOrientedBox3xf_toMatrixTransposeSIMD(dsMatrix44f* result, const dsOrientedBox3xf* box);
void dsOrientedBox3xf_fromMatrixSIMD(dsOrientedBox3xf* result, const dsMatrix44f* matrix);

void dsOrientedBox3xd_toMatrixSIMD2(dsMatrix44d* result, const dsOrientedBox3xd* box);
void dsOrientedBox3xd_toMatrixTransposeSIMD2(dsMatrix44d* result, const dsOrientedBox3xd* box);
void dsOrientedBox3xd_fromMatrixSIMD2(dsOrientedBox3xd* result, const dsMatrix44d* matrix);

#if !DS_DETERMINISTIC_MATH
void dsOrientedBox3xf_fromMatrixFMA(dsOrientedBox3xf* result, const dsMatrix44f* matrix);

void dsOrientedBox3xd_fromMatrixFMA2(dsOrientedBox3xd* result, const dsMatrix44d* matrix);
#endif // !DS_DETERMINISTIC_MATH

void dsOrientedBox3xd_toMatrixSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsOrientedBox3xd* DS_ALIGN_PARAM(32) box);
void dsOrientedBox3xd_toMatrixTransposeSIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsOrientedBox3xd* DS_ALIGN_PARAM(32) box);
void dsOrientedBox3xd_fromMatrixSIMD4(
	dsOrientedBox3xd* DS_ALIGN_PARAM(32) result, const dsMatrix44d* DS_ALIGN_PARAM(32) matrix);
#endif // DS_HAS_SIMD

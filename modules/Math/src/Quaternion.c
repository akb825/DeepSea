/*
 * Copyright 2020-2026 Aaron Barany
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

#include <DeepSea/Math/Quaternion.h>

#define dsQuaternion4_fromEulerAnglesImpl(result, cosX, sinX, cosY, sinY, cosZ, sinZ) \
	do \
	{ \
		(result).values[0] = sinX*cosY*cosZ - cosX*sinY*sinZ; \
		(result).values[1] = cosX*sinY*cosZ + sinX*cosY*sinZ; \
		(result).values[2] = cosX*cosY*sinZ - sinX*sinY*cosZ; \
		(result).values[3] = cosX*cosY*cosZ + sinX*sinY*sinZ; \
	} \
	while (0)

void dsQuaternion4f_fromEulerAngles(dsQuaternion4f* result, float x, float y, float z)
{
	DS_ASSERT(result);

	dsVector4f sinAngles, cosAngles;
#if DS_SIMD_ALWAYS_FLOAT4 && DS_SIMD_ALWAYS_INT
	dsSIMD4f angles = dsSIMD4f_mul(dsSIMD4f_set4(x, y, z, 0.0f), dsSIMD4f_set1(0.5f));
#if DS_SIMD_ALWAYS_FMA
	dsSinCosFMA4f(&sinAngles.simd, &cosAngles.simd, angles);
#else
	dsSinCosSIMD4f(&sinAngles.simd, &cosAngles.simd, angles);
#endif
#else
	dsSinCosf(&sinAngles.x, &cosAngles.x, x*0.5f);
	dsSinCosf(&sinAngles.y, &cosAngles.y, y*0.5f);
	dsSinCosf(&sinAngles.z, &cosAngles.z, z*0.5f);
#endif

	dsQuaternion4_fromEulerAnglesImpl(
		*result, cosAngles.x, sinAngles.x, cosAngles.y, sinAngles.y, cosAngles.z, sinAngles.z);
}

void dsQuaternion4d_fromEulerAngles(dsQuaternion4d* result, double x, double y, double z)
{
	DS_ASSERT(result);

	DS_ALIGN(32) dsVector4d sinAngles, cosAngles;
#if DS_SIMD_ALWAYS_DOUBLE4 && DS_SIMD_ALWAYS_INT
	dsSIMD4d angles = dsSIMD4d_mul(dsSIMD4d_set4(x, y, z, 0.0), dsSIMD4d_set1(0.5));
	dsSIMD4d simdSin, simdCos;
	dsSinCosSIMD4d(&simdSin, &simdCos, angles);
	dsSIMD4d_store(&sinAngles, simdSin);
	dsSIMD4d_store(&cosAngles, simdCos);
#elif DS_SIMD_ALWAYS_DOUBLE2 && DS_SIMD_ALWAYS_INT
	dsSIMD2d angles = dsSIMD2d_mul(dsSIMD2d_set2(x, y), dsSIMD2d_set1(0.5));
#if DS_SIMD_ALWAYS_FMA
	dsSinCosFMA2d(sinAngles.simd2, cosAngles.simd2, angles);
#else
	dsSinCosSIMD2d(sinAngles.simd2, cosAngles.simd2, angles);
#endif
	// Use scalar version for last angle.
	dsSinCosd(&sinAngles.z, &cosAngles.z, z*0.5);
#else
	dsSinCosd(&sinAngles.x, &cosAngles.x, x*0.5);
	dsSinCosd(&sinAngles.y, &cosAngles.y, y*0.5);
	dsSinCosd(&sinAngles.z, &cosAngles.z, z*0.5);
#endif

	dsQuaternion4_fromEulerAnglesImpl(
		*result, cosAngles.x, sinAngles.x, cosAngles.y, sinAngles.y, cosAngles.z, sinAngles.z);
}

void dsQuaternion4f_slerp(
	dsQuaternion4f* result, const dsQuaternion4f* a, const dsQuaternion4f* b, float t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	float cosAB = dsVector4f_dot((const dsVector4f*)a, (const dsVector4f*)b);

	// Make sure the shortest path is taken.
	dsQuaternion4f negB;
	if (cosAB < 0)
	{
		dsVector4f_neg((dsVector4f*)&negB, (const dsVector4f*)b);
		b = &negB;
		cosAB = -cosAB;
	}

	// If too close, do a lerp instead.
	const float epsilon = 1e-6f;
	if (cosAB > (1.0f - epsilon))
	{
		dsVector4f_lerp((dsVector4f*)result, (const dsVector4f*)a, (const dsVector4f*)b, t);
		dsQuaternion4f_normalize(result, result);
		return;
	}

	float thetaAB = dsACosf(cosAB);
	float theta = thetaAB*t;
	float sinTheta, cosTheta;
	dsSinCosf(&sinTheta, &cosTheta, theta);
	float sinThetaAB = dsSinf(thetaAB);

	float scaleB = sinTheta/sinThetaAB;
	float scaleA = cosTheta - cosAB*scaleB;

#if DS_SIMD_ALWAYS_FMA
	result->simd = dsSIMD4f_fmadd(a->simd, dsSIMD4f_set1(scaleA),
		dsSIMD4f_mul(b->simd, dsSIMD4f_set1(scaleB)));
#else
	dsVector4f scaledA, scaledB;
	dsVector4f_scale(&scaledA, (const dsVector4f*)a, scaleA);
	dsVector4f_scale(&scaledB, (const dsVector4f*)b, scaleB);
	dsVector4f_add((dsVector4f*)result, &scaledA, &scaledB);
#endif
}

void dsQuaternion4d_slerp(
	dsQuaternion4d* result, const dsQuaternion4d* a, const dsQuaternion4d* b, double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);

	double cosAB = dsVector4d_dot((const dsVector4d*)a, (const dsVector4d*)b);

	// Make sure the shortest path is taken.
	dsQuaternion4d negB;
	if (cosAB < 0)
	{
		dsVector4d_neg((dsVector4d*)&negB, (const dsVector4d*)b);
		b = &negB;
		cosAB = -cosAB;
	}

	// If too close, do a lerp instead.
	const double epsilon = 1e-15;
	if (cosAB > (1.0 - epsilon))
	{
		dsVector4d_lerp((dsVector4d*)result, (const dsVector4d*)a, (const dsVector4d*)b, t);
		dsQuaternion4d_normalize(result, result);
		return;
	}

	double thetaAB = dsACosd(cosAB);
	double theta = thetaAB*t;
	double sinTheta, cosTheta;
	dsSinCosd(&sinTheta, &cosTheta, theta);
	double sinThetaAB = dsSind(thetaAB);

	double scaleB = sinTheta/sinThetaAB;
	double scaleA = cosTheta - cosAB*scaleB;

#if DS_SIMD_PREFER_DOUBLE4 && DS_SIMD_ALWAYS_FMA
	result->simd = dsSIMD4d_fmadd(a->simd, dsSIMD4d_set1(scaleA),
		dsSIMD4d_mul(b->simd, dsSIMD4d_set1(scaleB)));
#elif DS_SIMD_ALWAYS_DOUBLE2 && DS_SIMD_ALWAYS_FMA
	result->simd2[0] = dsSIMD2d_fmadd(a->simd2[0], dsSIMD2d_set1(scaleA),
		dsSIMD2d_mul(b->simd2[0], dsSIMD2d_set1(scaleB)));
	result->simd2[1] = dsSIMD2d_fmadd(a->simd2[1], dsSIMD2d_set1(scaleA),
		dsSIMD2d_mul(b->simd2[1], dsSIMD2d_set1(scaleB)));
#else
	dsVector4d scaledA, scaledB;
	dsVector4d_scale(&scaledA, (const dsVector4d*)a, scaleA);
	dsVector4d_scale(&scaledB, (const dsVector4d*)b, scaleB);
	dsVector4d_add((dsVector4d*)result, &scaledA, &scaledB);
#endif
}

void dsQuaternion4f_fromAxisAngle(dsQuaternion4f* result, const dsVector3f* axis, float angle);
void dsQuaternion4d_fromAxisAngle(dsQuaternion4d* result, const dsVector3d* axis, double angle);

void dsQuaternion4f_fromMatrix33(dsQuaternion4f* result, const dsMatrix33f* matrix);
void dsQuaternion4d_fromMatrix33(dsQuaternion4d* result, const dsMatrix33d* matrix);

void dsQuaternion4f_fromMatrix33x(dsQuaternion4f* result, const dsMatrix33xf* matrix);
void dsQuaternion4d_fromMatrix33x(dsQuaternion4d* result, const dsMatrix33xd* matrix);

void dsQuaternion4f_fromMatrix44(dsQuaternion4f* result, const dsMatrix44f* matrix);
void dsQuaternion4d_fromMatrix44(dsQuaternion4d* result, const dsMatrix44d* matrix);

float dsQuaternion4f_getXAngle(const dsQuaternion4f* a);
double dsQuaternion4d_getXAngle(const dsQuaternion4d* a);

float dsQuaternion4f_getYAngle(const dsQuaternion4f* a);
double dsQuaternion4d_getYAngle(const dsQuaternion4d* a);

float dsQuaternion4f_getZAngle(const dsQuaternion4f* a);
double dsQuaternion4d_getZAngle(const dsQuaternion4d* a);

void dsQuaternion4f_getRotationAxis(dsVector3f* result, const dsQuaternion4f* a);
void dsQuaternion4d_getRotationAxis(dsVector3d* result, const dsQuaternion4d* a);

void dsQuaternion4f_getRotationAxis3x(dsVector3xf* result, const dsQuaternion4f* a);
void dsQuaternion4d_getRotationAxis3x(dsVector3xd* result, const dsQuaternion4d* a);

float dsQuaternion4f_getAxisAngle(const dsQuaternion4f* a);
double dsQuaternion4d_getAxisAngle(const dsQuaternion4d* a);

void dsQuaternion4f_toMatrix33(dsMatrix33f* result, const dsQuaternion4f* a);
void dsQuaternion4d_toMatrix33(dsMatrix33d* result, const dsQuaternion4d* a);

void dsQuaternion4f_toMatrix33x(dsMatrix33xf* result, const dsQuaternion4f* a);
void dsQuaternion4d_toMatrix33x(dsMatrix33xd* result, const dsQuaternion4d* a);

void dsQuaternion4f_toMatrix44(dsMatrix44f* result, const dsQuaternion4f* a);
void dsQuaternion4d_toMatrix44(dsMatrix44d* result, const dsQuaternion4d* a);

void dsQuaternion4f_normalize(dsQuaternion4f* result, const dsQuaternion4f* a);
void dsQuaternion4d_normalize(dsQuaternion4d* result, const dsQuaternion4d* a);

void dsQuaternion4f_rotate(dsVector3f* result, const dsQuaternion4f* a, const dsVector3f* v);
void dsQuaternion4d_rotate(dsVector3d* result, const dsQuaternion4d* a, const dsVector3d* v);

void dsQuaternion4f_rotate3x(dsVector3xf* result, const dsQuaternion4f* a, const dsVector3xf* v);
void dsQuaternion4d_rotate3x(dsVector3xd* result, const dsQuaternion4d* a, const dsVector3xd* v);

void dsQuaternion4f_mul(dsQuaternion4f* result, const dsQuaternion4f* a, const dsQuaternion4f* b);
void dsQuaternion4d_mul(dsQuaternion4d* result, const dsQuaternion4d* a, const dsQuaternion4d* b);

void dsQuaternion4f_conjugate(dsQuaternion4f* result, const dsQuaternion4f* a);
void dsQuaternion4d_conjugate(dsQuaternion4d* result, const dsQuaternion4d* a);

void dsQuaternion4f_unitLerp(
	dsQuaternion4f* result, const dsQuaternion4f* a, const dsQuaternion4f* b, float t);
void dsQuaternion4d_unitLerp(
	dsQuaternion4d* result, const dsQuaternion4d* a, const dsQuaternion4d* b, double t);

#if DS_HAS_SIMD

void dsQuaternion4f_mulSIMD(
	dsQuaternion4f* result, const dsQuaternion4f* a, const dsQuaternion4f* b);
void dsQuaternion4f_conjugateSIMD(dsQuaternion4f* result, const dsQuaternion4f* a);
void dsQuaternion4f_toMatrix33SIMD(dsMatrix33xf* result, const dsQuaternion4f* a);
void dsQuaternion4f_toMatrix44SIMD(dsMatrix44f* result, const dsQuaternion4f* a);
void dsQuaternion4f_normalizeSIMD(dsQuaternion4f* result, const dsQuaternion4f* a);
void dsQuaternion4f_rotateSIMD(dsVector3xf* result, const dsQuaternion4f* a, const dsVector3xf* v);
void dsQuaternion4f_unitLerpSIMD(
	dsQuaternion4f* result, const dsQuaternion4f* a, const dsQuaternion4f* b, float t);

#if !DS_DETERMINISTIC_MATH
void dsQuaternion4f_mulFMA(
	dsQuaternion4f* result, const dsQuaternion4f* a, const dsQuaternion4f* b);
void dsQuaternion4f_toMatrix33FMA(dsMatrix33xf* result, const dsQuaternion4f* a);
void dsQuaternion4f_toMatrix44FMA(dsMatrix44f* result, const dsQuaternion4f* a);
void dsQuaternion4f_normalizeFMA(dsQuaternion4f* result, const dsQuaternion4f* a);
void dsQuaternion4f_rotateFMA(dsVector3xf* result, const dsQuaternion4f* a, const dsVector3xf* v);
void dsQuaternion4f_unitLerpFMA(
	dsQuaternion4f* result, const dsQuaternion4f* a, const dsQuaternion4f* b, float t);
#endif // !DS_DETERMINISTIC_MATH

void dsQuaternion4d_mulSIMD2(
	dsQuaternion4d* result, const dsQuaternion4d* a, const dsQuaternion4d* b);
void dsQuaternion4d_conjugateSIMD2(dsQuaternion4d* result, const dsQuaternion4d* a);
void dsQuaternion4d_toMatrix33SIMD2(dsMatrix33xd* result, const dsQuaternion4d* a);
void dsQuaternion4d_toMatrix44SIMD2(dsMatrix44d* result, const dsQuaternion4d* a);
void dsQuaternion4d_normalizeSIMD2(dsQuaternion4d* result, const dsQuaternion4d* a);
void dsQuaternion4d_rotateSIMD2(dsVector3xd* result, const dsQuaternion4d* a, const dsVector3xd* v);
void dsQuaternion4d_unitLerpSIMD2(
	dsQuaternion4d* result, const dsQuaternion4d* a, const dsQuaternion4d* b, double t);

#if !DS_DETERMINISTIC_MATH
void dsQuaternion4d_mulFMA2(
	dsQuaternion4d* result, const dsQuaternion4d* a, const dsQuaternion4d* b);
void dsQuaternion4d_toMatrix33FMA2(dsMatrix33xd* result, const dsQuaternion4d* a);
void dsQuaternion4d_toMatrix44FMA2(dsMatrix44d* result, const dsQuaternion4d* a);
void dsQuaternion4d_normalizeFMA2(dsQuaternion4d* result, const dsQuaternion4d* a);
void dsQuaternion4d_rotateFMA2(dsVector3xd* result, const dsQuaternion4d* a, const dsVector3xd* v);
void dsQuaternion4d_unitLerpFMA2(
	dsQuaternion4d* result, const dsQuaternion4d* a, const dsQuaternion4d* b, double t);
#endif // !DS_DETERMINISTIC_MATH

void dsQuaternion4d_mulSIMD4(dsQuaternion4d* DS_ALIGN_PARAM(32) result,
	const dsQuaternion4d* DS_ALIGN_PARAM(32) a, const dsQuaternion4d* DS_ALIGN_PARAM(32) b);
void dsQuaternion4d_conjugateSIMD4(
	dsQuaternion4d* DS_ALIGN_PARAM(32) result, const dsQuaternion4d* DS_ALIGN_PARAM(32) a);
void dsQuaternion4d_toMatrix33SIMD4(
	dsMatrix33xd* DS_ALIGN_PARAM(32) result, const dsQuaternion4d* DS_ALIGN_PARAM(32) a);
void dsQuaternion4d_toMatrix44SIMD4(
	dsMatrix44d* DS_ALIGN_PARAM(32) result, const dsQuaternion4d* DS_ALIGN_PARAM(32) a);
void dsQuaternion4d_normalizeSIMD4(
	dsQuaternion4d* DS_ALIGN_PARAM(32) result, const dsQuaternion4d* DS_ALIGN_PARAM(32) a);
void dsQuaternion4d_rotateSIMD4(dsVector3xd* DS_ALIGN_PARAM(32) result,
	const dsQuaternion4d* DS_ALIGN_PARAM(32) a, const dsVector3xd* DS_ALIGN_PARAM(32) v);
void dsQuaternion4d_unitLerpSIMD4(dsQuaternion4d* DS_ALIGN_PARAM(32) result,
	const dsQuaternion4d* DS_ALIGN_PARAM(32) a, const dsQuaternion4d* DS_ALIGN_PARAM(32) b,
	double t);

#endif // DS_HAS_SIMD

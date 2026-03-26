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
#include <DeepSea/Math/Vector4.h>

#define dsQuaternion4_fromEulerAnglesImpl(result, cosX, sinX, cosY, sinY, cosZ, sinZ) \
	do \
	{ \
		(result).values[0] = sinX*cosY*cosZ - cosX*sinY*sinZ; \
		(result).values[1] = cosX*sinY*cosZ + sinX*cosY*sinZ; \
		(result).values[2] = cosX*cosY*sinZ - sinX*sinY*cosZ; \
		(result).values[3] = cosX*cosY*cosZ + sinX*sinY*sinZ; \
	} \
	while (0)

#define dsQuaternion4_fromMatrixImpl(result, matrix, w, inv4w) \
	do \
	{ \
		(result).values[0] = ((matrix).values[1][2] - (matrix).values[2][1])*inv4w; \
		(result).values[1] = ((matrix).values[2][0] - (matrix).values[0][2])*inv4w; \
		(result).values[2] = ((matrix).values[0][1] - (matrix).values[1][0])*inv4w; \
		(result).values[3] = w; \
	} \
	while (0)

#define dsQuaternion4_toMatrixImpl(result, a) \
	do \
	{ \
		(result).values[0][0] = 1 - 2*(dsPow2((a).values[1]) + dsPow2((a).values[2])); \
		(result).values[0][1] = 2*((a).values[0]*(a).values[1] + (a).values[3]*(a).values[2]); \
		(result).values[0][2] = 2*((a).values[0]*(a).values[2] - (a).values[3]*(a).values[1]); \
		\
		(result).values[1][0] = 2*((a).values[0]*(a).values[1] - (a).values[3]*(a).values[2]); \
		(result).values[1][1] = 1 - 2*(dsPow2((a).values[0]) + dsPow2((a).values[2])); \
		(result).values[1][2] = 2*((a).values[3]*(a).values[0] + (a).values[1]*(a).values[2]); \
		\
		(result).values[2][0] = 2*((a).values[3]*(a).values[1] + (a).values[0]*(a).values[2]); \
		(result).values[2][1] = 2*((a).values[1]*(a).values[2] - (a).values[3]*(a).values[0]); \
		(result).values[2][2] = 1 - 2*(dsPow2((a).values[0]) + dsPow2((a).values[1])); \
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

void dsQuaternion4f_fromAxisAngle(dsQuaternion4f* result, const dsVector3f* axis,
	float angle)
{
	DS_ASSERT(result);
	DS_ASSERT(axis);

	float sinAngle, cosAngle;
	dsSinCosf(&sinAngle, &cosAngle, angle*0.5f);

	result->i = axis->x*sinAngle;
	result->j = axis->y*sinAngle;
	result->k = axis->z*sinAngle;
	result->r = cosAngle;
}

void dsQuaternion4d_fromAxisAngle(dsQuaternion4d* result, const dsVector3d* axis,
	double angle)
{
	DS_ASSERT(result);
	DS_ASSERT(axis);

	double sinAngle, cosAngle;
	dsSinCosd(&sinAngle, &cosAngle, angle*0.5);

	result->i = axis->x*sinAngle;
	result->j = axis->y*sinAngle;
	result->k = axis->z*sinAngle;
	result->r = cosAngle;
}

void dsQuaternion4f_fromMatrix33(dsQuaternion4f* result, const dsMatrix33f* matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);

	float w = dsSqrtf(
		1.0f + matrix->values[0][0] + matrix->values[1][1] + matrix->values[2][2])/2.0f;
	float inv4w = 1.0f/(4.0f*w);
	dsQuaternion4_fromMatrixImpl(*result, *matrix, w, inv4w);
}

void dsQuaternion4d_fromMatrix33(dsQuaternion4d* result, const dsMatrix33d* matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);

	double w = dsSqrtd(
		1.0 + matrix->values[0][0] + matrix->values[1][1] + matrix->values[2][2])/2.0;
	double inv4w = 1.0/(4.0*w);
	dsQuaternion4_fromMatrixImpl(*result, *matrix, w, inv4w);
}

void dsQuaternion4f_fromMatrix44(dsQuaternion4f* result, const dsMatrix44f* matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);

	float w = dsSqrtf(
		1.0f + matrix->values[0][0] + matrix->values[1][1] + matrix->values[2][2])/2.0f;
	float inv4w = 1.0f/(4.0f*w);
	dsQuaternion4_fromMatrixImpl(*result, *matrix, w, inv4w);
}

void dsQuaternion4d_fromMatrix44(dsQuaternion4d* result, const dsMatrix44d* matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);

	double w = dsSqrtd(
		1.0 + matrix->values[0][0] + matrix->values[1][1] + matrix->values[2][2])/2.0;
	double inv4w = 1.0/(4.0*w);
	dsQuaternion4_fromMatrixImpl(*result, *matrix, w, inv4w);
}

void dsQuaternion4f_toMatrix33(dsMatrix33f* result, const dsQuaternion4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsQuaternion4_toMatrixImpl(*result, *a);
}

void dsQuaternion4d_toMatrix33(dsMatrix33d* result, const dsQuaternion4d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	dsQuaternion4_toMatrixImpl(*result, *a);
}

void dsQuaternion4f_toMatrix44(dsMatrix44f* result, const dsQuaternion4f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsQuaternion4_toMatrixImpl(*result, *a);
	result->values[0][3] = 0.0f;
	result->values[1][3] = 0.0f;
	result->values[2][3] = 0.0f;

	result->values[3][0] = 0.0f;
	result->values[3][1] = 0.0f;
	result->values[3][2] = 0.0f;
	result->values[3][3] = 1.0f;
}

void dsQuaternion4d_toMatrix44(dsMatrix44d* result, const dsQuaternion4d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);

	dsQuaternion4_toMatrixImpl(*result, *a);
	result->values[0][3] = 0.0;
	result->values[1][3] = 0.0;
	result->values[2][3] = 0.0;

	result->values[3][0] = 0.0;
	result->values[3][1] = 0.0;
	result->values[3][2] = 0.0;
	result->values[3][3] = 1.0;
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

	double cosAB = dsVector4_dot(*a, *b);

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

#if DS_SIMD_ALWAYS_DOUBLE2 && DS_SIMD_ALWAYS_FMA
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

float dsQuaternion4f_getXAngle(const dsQuaternion4f* a);
double dsQuaternion4d_getXAngle(const dsQuaternion4d* a);

float dsQuaternion4f_getYAngle(const dsQuaternion4f* a);
double dsQuaternion4d_getYAngle(const dsQuaternion4d* a);

float dsQuaternion4f_getZAngle(const dsQuaternion4f* a);
double dsQuaternion4d_getZAngle(const dsQuaternion4d* a);

void dsQuaternion4f_getRotationAxis(dsVector3f* result, const dsQuaternion4f* a);
void dsQuaternion4d_getRotationAxis(dsVector3d* result, const dsQuaternion4d* a);

float dsQuaternion4f_getAxisAngle(const dsQuaternion4f* a);
double dsQuaternion4d_getAxisAngle(const dsQuaternion4d* a);

void dsQuaternion4f_normalize(dsQuaternion4f* result, const dsQuaternion4f* a);
void dsQuaternion4d_normalize(dsQuaternion4d* result, const dsQuaternion4d* a);

void dsQuaternion4f_rotate(dsVector3f* result, const dsQuaternion4f* a, const dsVector3f* v);
void dsQuaternion4d_rotate(dsVector3d* result, const dsQuaternion4d* a, const dsVector3d* v);

void dsQuaternion4f_mul(dsQuaternion4f* result, const dsQuaternion4f* a, const dsQuaternion4f* b);
void dsQuaternion4d_mul(dsQuaternion4d* result, const dsQuaternion4d* a, const dsQuaternion4d* b);

void dsQuaternion4f_conjugate(dsQuaternion4f* result, const dsQuaternion4f* a);
void dsQuaternion4d_conjugate(dsQuaternion4d* result, const dsQuaternion4d* a);

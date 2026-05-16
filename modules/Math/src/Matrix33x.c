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

#include <DeepSea/Math/Matrix33x.h>

#include <DeepSea/Math/Trig.h>

// Set last row so it can be used with dsMatrix44.
#define dsMatrix33x_makeRotate3DImpl(result, cosX, sinX, cosY, sinY, cosZ, sinZ) \
	do \
	{ \
		(result).values[0][0] = (cosY)*(cosZ); \
		(result).values[0][1] = (cosY)*(sinZ); \
		(result).values[0][2] = -(sinY); \
		(result).values[0][3] = 0; \
		\
		(result).values[1][0] = (sinX)*(sinY)*(cosZ) - (cosX)*(sinZ); \
		(result).values[1][1] = (cosX)*(cosZ) + (sinX)*(sinY)*(sinZ); \
		(result).values[1][2] = (sinX)*(cosY); \
		(result).values[1][3] = 0; \
		\
		(result).values[2][0] = (sinX)*(sinZ) + (cosX)*(sinY)*(cosZ); \
		(result).values[2][1] = (cosX)*(sinY)*(sinZ) - (sinX)*(cosZ); \
		(result).values[2][2] = (cosX)*(cosY); \
		(result).values[2][3] = 0; \
	} while (0)

#define dsMatrix33x_makeRotate3DAxisAngleImpl(result, axis, cosAngle, sinAngle, invCosAngle) \
	do \
	{ \
		(result).values[0][0] = (invCosAngle)*(axis).values[0]*(axis).values[0] + (cosAngle); \
		(result).values[0][1] = (invCosAngle)*(axis).values[0]*(axis).values[1] + \
			(axis).values[2]*(sinAngle); \
		(result).values[0][2] = (invCosAngle)*(axis).values[0]*(axis).values[2] - \
			(axis).values[1]*(sinAngle); \
		(result).values[0][3] = 0; \
		\
		(result).values[1][0] = (invCosAngle)*(axis).values[0]*(axis).values[1] - \
			(axis).values[2]*(sinAngle); \
		(result).values[1][1] = (invCosAngle)*(axis).values[1]*(axis).values[1] + (cosAngle); \
		(result).values[1][2] = (invCosAngle)*(axis).values[1]*(axis).values[2] + \
			(axis).values[0]*(sinAngle); \
		(result).values[1][3] = 0; \
		\
		(result).values[2][0] = (invCosAngle)*(axis).values[0]*(axis).values[2] + \
			(axis).values[1]*(sinAngle); \
		(result).values[2][1] = (invCosAngle)*(axis).values[1]*(axis).values[2] - \
			(axis).values[0]*(sinAngle); \
		(result).values[2][2] = (invCosAngle)*(axis).values[2]*(axis).values[2] + (cosAngle); \
		(result).values[2][3] = 0; \
	} while (0)

void dsMatrix33xf_makeRotate(dsMatrix33xf* result, float angle)
{
	DS_ASSERT(result);
	float sinAngle, cosAngle;
	dsSinCosf(&sinAngle, &cosAngle, angle);

#if DS_SIMD_ALWAYS_FLOAT4
	result->columns[0].simd = dsSIMD4f_set4(cosAngle, sinAngle, 0.0f, 0.0f);
	result->columns[1].simd = dsSIMD4f_set4(-sinAngle, cosAngle, 0.0f, 0.0f);
	result->columns[2].simd = dsSIMD4f_set4(0.0f, 0.0f, 1.0f, 0.0f);
#else
	result->values[0][0] = cosAngle;
	result->values[0][1] = sinAngle;
	result->values[0][2] = 0;

	result->values[1][0] = -sinAngle;
	result->values[1][1] = cosAngle;
	result->values[1][2] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	result->values[2][2] = 1;
#endif
}

void dsMatrix33xd_makeRotate(dsMatrix33xd* result, double angle)
{
	DS_ASSERT(result);
	double sinAngle, cosAngle;
	dsSinCosd(&sinAngle, &cosAngle, angle);

#if DS_SIMD_ALWAYS_DOUBLE2
	result->columns[0].simd2[0] = dsSIMD2d_set2(cosAngle, sinAngle);
	result->columns[0].simd2[1] = dsSIMD2d_set1(0.0);
	result->columns[1].simd2[0] = dsSIMD2d_set2(-sinAngle, cosAngle);
	result->columns[1].simd2[1] = dsSIMD2d_set1(0.0);
	result->columns[2].simd2[0] = dsSIMD2d_set1(0.0);
	result->columns[2].simd2[1] = dsSIMD2d_set2(1.0, 0.0);
#else
	result->values[0][0] = cosAngle;
	result->values[0][1] = sinAngle;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = -sinAngle;
	result->values[1][1] = cosAngle;
	result->values[1][2] = 0;
	result->values[0][3] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	result->values[2][2] = 1;
	result->values[0][3] = 0;
#endif
}

void dsMatrix33xf_makeRotate3D(dsMatrix33xf* result, float x, float y, float z)
{
	DS_ASSERT(result);

	dsVector4f sinAngles, cosAngles;
#if DS_SIMD_ALWAYS_FLOAT4 && DS_SIMD_ALWAYS_INT
	dsSIMD4f angles = dsSIMD4f_set4(x, y, z, 0.0f);
#if DS_SIMD_ALWAYS_FMA
	dsSinCosFMA4f(&sinAngles.simd, &cosAngles.simd, angles);
#else
	dsSinCosSIMD4f(&sinAngles.simd, &cosAngles.simd, angles);
#endif
#else
	dsSinCosf(&sinAngles.x, &cosAngles.x, x);
	dsSinCosf(&sinAngles.y, &cosAngles.y, y);
	dsSinCosf(&sinAngles.z, &cosAngles.z, z);
#endif

	dsMatrix33x_makeRotate3DImpl(
		*result, cosAngles.x, sinAngles.x, cosAngles.y, sinAngles.y, cosAngles.z, sinAngles.z);
}

void dsMatrix33xd_makeRotate3D(dsMatrix33xd* result, double x, double y, double z)
{
	DS_ASSERT(result);

	DS_ALIGN(32) dsVector4d sinAngles, cosAngles;
#if DS_SIMD_ALWAYS_DOUBLE4 && DS_SIMD_ALWAYS_INT
	dsSIMD4d angles = dsSIMD4d_set4(x, y, z, 0.0);
	dsSIMD4d simdSin, simdCos;
	dsSinCosSIMD4d(&simdSin, &simdCos, angles);
	dsSIMD4d_store(&sinAngles, simdSin);
	dsSIMD4d_store(&cosAngles, simdCos);
#elif DS_SIMD_ALWAYS_DOUBLE2 && DS_SIMD_ALWAYS_INT
	dsSIMD2d angles = dsSIMD2d_set2(x, y);
#if DS_SIMD_ALWAYS_FMA
	dsSinCosFMA2d(sinAngles.simd2, cosAngles.simd2, angles);
#else
	dsSinCosSIMD2d(sinAngles.simd2, cosAngles.simd2, angles);
#endif
	// Use scalar version for last angle.
	dsSinCosd(&sinAngles.z, &cosAngles.z, z);
#else
	dsSinCosd(&sinAngles.x, &cosAngles.x, x);
	dsSinCosd(&sinAngles.y, &cosAngles.y, y);
	dsSinCosd(&sinAngles.z, &cosAngles.z, z);
#endif

	dsMatrix33x_makeRotate3DImpl(
		*result, cosAngles.x, sinAngles.x, cosAngles.y, sinAngles.y, cosAngles.z, sinAngles.z);
}

void dsMatrix33xf_makeRotate3DAxisAngle(dsMatrix33xf* result, const dsVector3f* axis, float angle)
{
	DS_ASSERT(result);
	DS_ASSERT(axis);

	float sinAngle, cosAngle;
	dsSinCosf(&sinAngle, &cosAngle, angle);
	float invCosAngle = 1 - cosAngle;

	dsMatrix33x_makeRotate3DAxisAngleImpl(*result, *axis, cosAngle, sinAngle, invCosAngle);
}

void dsMatrix33xd_makeRotate3DAxisAngle(dsMatrix33xd* result, const dsVector3d* axis, double angle)
{
	DS_ASSERT(result);
	DS_ASSERT(axis);

	double sinAngle, cosAngle;
	dsSinCosd(&sinAngle, &cosAngle, angle);
	double invCosAngle = 1 - cosAngle;

	dsMatrix33x_makeRotate3DAxisAngleImpl(*result, *axis, cosAngle, sinAngle, invCosAngle);
}

void dsMatrix33xf_identity(dsMatrix33xf* result);
void dsMatrix33xd_identity(dsMatrix33xd* result);

void dsMatrix33xf_mul(dsMatrix33xf* result, const dsMatrix33xf* a, const dsMatrix33xf* b);
void dsMatrix33xd_mul(dsMatrix33xd* result, const dsMatrix33xd* a, const dsMatrix33xd* b);

void dsMatrix33xf_affineMul(dsMatrix33xf* result, const dsMatrix33xf* a, const dsMatrix33xf* b);
void dsMatrix33xd_affineMul(dsMatrix33xd* result, const dsMatrix33xd* a, const dsMatrix33xd* b);

void dsMatrix33xf_transform(dsVector3xf* result, const dsMatrix33xf* mat, const dsVector3xf* vec);
void dsMatrix33xd_transform(dsVector3xd* result, const dsMatrix33xd* mat, const dsVector3xd* vec);

void dsMatrix33xf_transformTransposed(
	dsVector3xf* result, const dsMatrix33xf* mat, const dsVector3xf* vec);
void dsMatrix33xd_transformTransposed(
	dsVector3xd* result, const dsMatrix33xd* mat, const dsVector3xd* vec);

void dsMatrix33xf_transpose(dsMatrix33xf* result, const dsMatrix33xf* a);
void dsMatrix33xd_transpose(dsMatrix33xd* result, const dsMatrix33xd* a);

float dsMatrix33xf_determinant(const dsMatrix33xf* a);
double dsMatrix33xd_determinant(const dsMatrix33xd* a);

void dsMatrix33xf_fastInvert(dsMatrix33xf* result, const dsMatrix33xf* a);
void dsMatrix33xd_fastInvert(dsMatrix33xd* result, const dsMatrix33xd* a);

void dsMatrix33xf_affineInvert(dsMatrix33xf* result, const dsMatrix33xf* a);
void dsMatrix33xd_affineInvert(dsMatrix33xd* result, const dsMatrix33xd* a);

void dsMatrix33xf_invert(dsMatrix33xf* result, const dsMatrix33xf* a);
void dsMatrix33xd_invert(dsMatrix33xd* result, const dsMatrix33xd* a);

void dsMatrix33xf_inverseTranspose(dsMatrix22f* result, const dsMatrix33xf* a);
void dsMatrix33xd_inverseTranspose(dsMatrix22d* result, const dsMatrix33xd* a);

void dsMatrix33xf_makeTranslate(dsMatrix33xf* result, float x, float y);
void dsMatrix33xd_makeTranslate(dsMatrix33xd* result, double x, double y);

void dsMatrix33xf_makeScale(dsMatrix33xf* result, float x, float y);
void dsMatrix33xd_makeScale(dsMatrix33xd* result, double x, double y);

void dsMatrix33xf_makeScale3D(dsMatrix33xf* result, float x, float y, float z);
void dsMatrix33xd_makeScale3D(dsMatrix33xd* result, double x, double y, double z);

bool dsMatrix33xf_jacobiEigenvalues(
	dsMatrix33xf* outEigenvectors, dsVector3f* outEigenvalues, const dsMatrix33xf* a);
bool dsMatrix33xd_jacobiEigenvalues(
	dsMatrix33xd* outEigenvectors, dsVector3d* outEigenvalues, const dsMatrix33xd* a);

void dsMatrix33xf_sortEigenvalues(dsMatrix33xf* eigenvectors, dsVector3f* eigenvalues);
void dsMatrix33xd_sortEigenvalues(dsMatrix33xd* eigenvectors, dsVector3d* eigenvalues);

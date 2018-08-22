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

#include <DeepSea/Math/Matrix44.h>

#include "Matrix33Impl.h"
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Vector3.h>

#define dsMatrix44_invertImpl(result, a, invDet) \
	do \
	{ \
		(result).values[0][0] = ((a).values[1][1]*(a).values[2][2]*(a).values[3][3] + \
								 (a).values[2][1]*(a).values[3][2]*(a).values[1][3] + \
								 (a).values[3][1]*(a).values[1][2]*(a).values[2][3] - \
								 (a).values[1][1]*(a).values[3][2]*(a).values[2][3] - \
								 (a).values[2][1]*(a).values[1][2]*(a).values[3][3] - \
								 (a).values[3][1]*(a).values[2][2]*(a).values[1][3])*invDet; \
		(result).values[0][1] = ((a).values[0][1]*(a).values[3][2]*(a).values[2][3] + \
								 (a).values[2][1]*(a).values[0][2]*(a).values[3][3] + \
								 (a).values[3][1]*(a).values[2][2]*(a).values[0][3] - \
								 (a).values[0][1]*(a).values[2][2]*(a).values[3][3] - \
								 (a).values[2][1]*(a).values[3][2]*(a).values[0][3] - \
								 (a).values[3][1]*(a).values[0][2]*(a).values[2][3])*invDet; \
		(result).values[0][2] = ((a).values[0][1]*(a).values[1][2]*(a).values[3][3] + \
								 (a).values[1][1]*(a).values[3][2]*(a).values[0][3] + \
								 (a).values[3][1]*(a).values[0][2]*(a).values[1][3] - \
								 (a).values[0][1]*(a).values[3][2]*(a).values[1][3] - \
								 (a).values[1][1]*(a).values[0][2]*(a).values[3][3] - \
								 (a).values[3][1]*(a).values[1][2]*(a).values[0][3])*invDet; \
		(result).values[0][3] = ((a).values[0][1]*(a).values[2][2]*(a).values[1][3] + \
								 (a).values[1][1]*(a).values[0][2]*(a).values[2][3] + \
								 (a).values[2][1]*(a).values[1][2]*(a).values[0][3] - \
								 (a).values[0][1]*(a).values[1][2]*(a).values[2][3] - \
								 (a).values[1][1]*(a).values[2][2]*(a).values[0][3] - \
								 (a).values[2][1]*(a).values[0][2]*(a).values[1][3])*invDet; \
		\
		(result).values[1][0] = ((a).values[1][0]*(a).values[3][2]*(a).values[2][3] + \
								 (a).values[2][0]*(a).values[1][2]*(a).values[3][3] + \
								 (a).values[3][0]*(a).values[2][2]*(a).values[1][3] - \
								 (a).values[1][0]*(a).values[2][2]*(a).values[3][3] - \
								 (a).values[2][0]*(a).values[3][2]*(a).values[1][3] - \
								 (a).values[3][0]*(a).values[1][2]*(a).values[2][3])*invDet; \
		(result).values[1][1] = ((a).values[0][0]*(a).values[2][2]*(a).values[3][3] + \
								 (a).values[2][0]*(a).values[3][2]*(a).values[0][3] + \
								 (a).values[3][0]*(a).values[0][2]*(a).values[2][3] - \
								 (a).values[0][0]*(a).values[3][2]*(a).values[2][3] - \
								 (a).values[2][0]*(a).values[0][2]*(a).values[3][3] - \
								 (a).values[3][0]*(a).values[2][2]*(a).values[0][3])*invDet; \
		(result).values[1][2] = ((a).values[0][0]*(a).values[3][2]*(a).values[1][3] + \
								 (a).values[1][0]*(a).values[0][2]*(a).values[3][3] + \
								 (a).values[3][0]*(a).values[1][2]*(a).values[0][3] - \
								 (a).values[0][0]*(a).values[1][2]*(a).values[3][3] - \
								 (a).values[1][0]*(a).values[3][2]*(a).values[0][3] - \
								 (a).values[3][0]*(a).values[0][2]*(a).values[1][3])*invDet; \
		(result).values[1][3] = ((a).values[0][0]*(a).values[1][2]*(a).values[2][3] + \
								 (a).values[1][0]*(a).values[2][2]*(a).values[0][3] + \
								 (a).values[2][0]*(a).values[0][2]*(a).values[1][3] - \
								 (a).values[0][0]*(a).values[2][2]*(a).values[1][3] - \
								 (a).values[1][0]*(a).values[0][2]*(a).values[2][3] - \
								 (a).values[2][0]*(a).values[1][2]*(a).values[0][3])*invDet; \
		\
		(result).values[2][0] = ((a).values[1][0]*(a).values[2][1]*(a).values[3][3] + \
								 (a).values[2][0]*(a).values[3][1]*(a).values[1][3] + \
								 (a).values[3][0]*(a).values[1][1]*(a).values[2][3] - \
								 (a).values[1][0]*(a).values[3][1]*(a).values[2][3] - \
								 (a).values[2][0]*(a).values[1][1]*(a).values[3][3] - \
								 (a).values[3][0]*(a).values[2][1]*(a).values[1][3])*invDet; \
		(result).values[2][1] = ((a).values[0][0]*(a).values[3][1]*(a).values[2][3] + \
								 (a).values[2][0]*(a).values[0][1]*(a).values[3][3] + \
								 (a).values[3][0]*(a).values[2][1]*(a).values[0][3] - \
								 (a).values[0][0]*(a).values[2][1]*(a).values[3][3] - \
								 (a).values[2][0]*(a).values[3][1]*(a).values[0][3] - \
								 (a).values[3][0]*(a).values[0][1]*(a).values[2][3])*invDet; \
		(result).values[2][2] = ((a).values[0][0]*(a).values[1][1]*(a).values[3][3] + \
								 (a).values[1][0]*(a).values[3][1]*(a).values[0][3] + \
								 (a).values[3][0]*(a).values[0][1]*(a).values[1][3] - \
								 (a).values[0][0]*(a).values[3][1]*(a).values[1][3] - \
								 (a).values[1][0]*(a).values[0][1]*(a).values[3][3] - \
								 (a).values[3][0]*(a).values[1][1]*(a).values[0][3])*invDet; \
		(result).values[2][3] = ((a).values[0][0]*(a).values[2][1]*(a).values[1][3] + \
								 (a).values[1][0]*(a).values[0][1]*(a).values[2][3] + \
								 (a).values[2][0]*(a).values[1][1]*(a).values[0][3] - \
								 (a).values[0][0]*(a).values[1][1]*(a).values[2][3] - \
								 (a).values[1][0]*(a).values[2][1]*(a).values[0][3] - \
								 (a).values[2][0]*(a).values[0][1]*(a).values[1][3])*invDet; \
		\
		(result).values[3][0] = ((a).values[1][0]*(a).values[3][1]*(a).values[2][2] + \
								 (a).values[2][0]*(a).values[1][1]*(a).values[3][2] + \
								 (a).values[3][0]*(a).values[2][1]*(a).values[1][2] - \
								 (a).values[1][0]*(a).values[2][1]*(a).values[3][2] - \
								 (a).values[2][0]*(a).values[3][1]*(a).values[1][2] - \
								 (a).values[3][0]*(a).values[1][1]*(a).values[2][2])*invDet; \
		(result).values[3][1] = ((a).values[0][0]*(a).values[2][1]*(a).values[3][2] + \
								 (a).values[2][0]*(a).values[3][1]*(a).values[0][2] + \
								 (a).values[3][0]*(a).values[0][1]*(a).values[2][2] - \
								 (a).values[0][0]*(a).values[3][1]*(a).values[2][2] - \
								 (a).values[2][0]*(a).values[0][1]*(a).values[3][2] - \
								 (a).values[3][0]*(a).values[2][1]*(a).values[0][2])*invDet; \
		(result).values[3][2] = ((a).values[0][0]*(a).values[3][1]*(a).values[1][2] + \
								 (a).values[1][0]*(a).values[0][1]*(a).values[3][2] + \
								 (a).values[3][0]*(a).values[1][1]*(a).values[0][2] - \
								 (a).values[0][0]*(a).values[1][1]*(a).values[3][2] - \
								 (a).values[1][0]*(a).values[3][1]*(a).values[0][2] - \
								 (a).values[3][0]*(a).values[0][1]*(a).values[1][2])*invDet; \
		(result).values[3][3] = ((a).values[0][0]*(a).values[1][1]*(a).values[2][2] + \
								 (a).values[1][0]*(a).values[2][1]*(a).values[0][2] + \
								 (a).values[2][0]*(a).values[0][1]*(a).values[1][2] - \
								 (a).values[0][0]*(a).values[2][1]*(a).values[1][2] - \
								 (a).values[1][0]*(a).values[0][1]*(a).values[2][2] - \
								 (a).values[2][0]*(a).values[1][1]*(a).values[0][2])*invDet; \
	} while (0)

#define dsMatrix44_makeRotateImpl(result, cosX, sinX, cosY, sinY, cosZ, sinZ) \
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
		\
		(result).values[3][0] = 0; \
		(result).values[3][1] = 0; \
		(result).values[3][2] = 0; \
		(result).values[3][3] = 1; \
	} while (0)

#define dsMatrix44_makeRotateAxisAngleImpl(result, axis, cosAngle, sinAngle, invCosAngle) \
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
		\
		(result).values[3][0] = 0; \
		(result).values[3][1] = 0; \
		(result).values[3][2] = 0; \
		(result).values[3][3] = 1; \
	} while (0)

void dsMatrix44f_affineInvert(dsMatrix44f* result, const dsMatrix44f* a)
{
	// Macros for 3x3 matrix will work on the upper 3x3 for a 4x4 matrix.
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	float upperDet = dsMatrix33_determinant(*a);
	DS_ASSERT(upperDet != 0);
	float invUpperDet = 1/upperDet;

	dsMatrix33_invertImpl(*result, *a, invUpperDet);

	result->values[0][3] = 0;
	result->values[1][3] = 0;
	result->values[2][3] = 0;

	result->values[3][0] = -a->values[3][0]*result->values[0][0] -
		a->values[3][1]*result->values[1][0] - a->values[3][2]*result->values[2][0];
	result->values[3][1] = -a->values[3][0]*result->values[0][1] -
		a->values[3][1]*result->values[1][1] - a->values[3][2]*result->values[2][1];
	result->values[3][2] = -a->values[3][0]*result->values[0][2] -
		a->values[3][1]*result->values[1][2] - a->values[3][2]*result->values[2][2];
	result->values[3][3] = 1;
}

void dsMatrix44d_affineInvert(dsMatrix44d* result, const dsMatrix44d* a)
{
	// Macros for 3x3 matrix will work on the upper 3x3 for a 4x4 matrix.
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	double upperDet = dsMatrix33_determinant(*a);
	DS_ASSERT(upperDet != 0);
	double invUpperDet = 1/upperDet;

	dsMatrix33_invertImpl(*result, *a, invUpperDet);

	result->values[0][3] = 0;
	result->values[1][3] = 0;
	result->values[2][3] = 0;

	result->values[3][0] = -a->values[3][0]*result->values[0][0] -
		a->values[3][1]*result->values[1][0] - a->values[3][2]*result->values[2][0];
	result->values[3][1] = -a->values[3][0]*result->values[0][1] -
		a->values[3][1]*result->values[1][1] - a->values[3][2]*result->values[2][1];
	result->values[3][2] = -a->values[3][0]*result->values[0][2] -
		a->values[3][1]*result->values[1][2] - a->values[3][2]*result->values[2][2];
	result->values[3][3] = 1;
}

void dsMatrix44f_invert(dsMatrix44f* result, const dsMatrix44f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	float det = dsMatrix44_determinant(*a);
	DS_ASSERT(det != 0);
	float invDet = 1/det;

	dsMatrix44_invertImpl(*result, *a, invDet);
}

void dsMatrix44d_invert(dsMatrix44d* result, const dsMatrix44d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	double det = dsMatrix44_determinant(*a);
	DS_ASSERT(det != 0);
	double invDet = 1/det;

	dsMatrix44_invertImpl(*result, *a, invDet);
}

void dsMatrix44f_inverseTranspose(dsMatrix44f* result, const dsMatrix44f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	dsMatrix44f inverse;
	dsMatrix44f_affineInvert(&inverse, a);
	dsMatrix44_transpose(*result, inverse);
}

void dsMatrix44d_inverseTranspose(dsMatrix44d* result, const dsMatrix44d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	dsMatrix44d inverse;
	dsMatrix44d_affineInvert(&inverse, a);
	dsMatrix44_transpose(*result, inverse);
}

void dsMatrix44f_makeRotate(dsMatrix44f* result, float x, float y, float z)
{
	DS_ASSERT(result);

	float cosX = cosf(x);
	float sinX = sinf(x);
	float cosY = cosf(y);
	float sinY = sinf(y);
	float cosZ = cosf(z);
	float sinZ = sinf(z);

	dsMatrix44_makeRotateImpl(*result, cosX, sinX, cosY, sinY, cosZ, sinZ);
}

void dsMatrix44d_makeRotate(dsMatrix44d* result, double x, double y, double z)
{
	DS_ASSERT(result);

	double cosX = cos(x);
	double sinX = sin(x);
	double cosY = cos(y);
	double sinY = sin(y);
	double cosZ = cos(z);
	double sinZ = sin(z);

	dsMatrix44_makeRotateImpl(*result, cosX, sinX, cosY, sinY, cosZ, sinZ);
}

void dsMatrix44f_makeRotateAxisAngle(dsMatrix44f* result, const dsVector3f* axis,
	float angle)
{
	DS_ASSERT(result);
	DS_ASSERT(axis);

	float cosAngle = cosf(angle);
	float sinAngle = sinf(angle);
	float invCosAngle = 1 - cosAngle;

	dsMatrix44_makeRotateAxisAngleImpl(*result, *axis, cosAngle, sinAngle, invCosAngle);
}

void dsMatrix44d_makeRotateAxisAngle(dsMatrix44d* result, const dsVector3d* axis,
	double angle)
{
	DS_ASSERT(result);
	DS_ASSERT(axis);

	double cosAngle = cos(angle);
	double sinAngle = sin(angle);
	double invCosAngle = 1 - cosAngle;

	dsMatrix44_makeRotateAxisAngleImpl(*result, *axis, cosAngle, sinAngle, invCosAngle);
}

void dsMatrix44f_makeTranslate(dsMatrix44f* result, float x, float y, float z)
{
	DS_ASSERT(result);
	result->values[0][0] = 1;
	result->values[0][1] = 0;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = 1;
	result->values[1][2] = 0;
	result->values[1][3] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	result->values[2][2] = 1;
	result->values[2][3] = 0;

	result->values[3][0] = x;
	result->values[3][1] = y;
	result->values[3][2] = z;
	result->values[3][3] = 1;
}

void dsMatrix44d_makeTranslate(dsMatrix44d* result, double x, double y, double z)
{
	DS_ASSERT(result);
	result->values[0][0] = 1;
	result->values[0][1] = 0;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = 1;
	result->values[1][2] = 0;
	result->values[1][3] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	result->values[2][2] = 1;
	result->values[2][3] = 0;

	result->values[3][0] = x;
	result->values[3][1] = y;
	result->values[3][2] = z;
	result->values[3][3] = 1;
}

void dsMatrix44f_makeScale(dsMatrix44f* result, float x, float y, float z)
{
	DS_ASSERT(result);
	result->values[0][0] = x;
	result->values[0][1] = 0;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = y;
	result->values[1][2] = 0;
	result->values[1][3] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	result->values[2][2] = z;
	result->values[2][3] = 0;

	result->values[3][0] = 0;
	result->values[3][1] = 0;
	result->values[3][2] = 0;
	result->values[3][3] = 1;
}

void dsMatrix44d_makeScale(dsMatrix44d* result, double x, double y, double z)
{
	DS_ASSERT(result);
	result->values[0][0] = x;
	result->values[0][1] = 0;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = y;
	result->values[1][2] = 0;
	result->values[1][3] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	result->values[2][2] = z;
	result->values[2][3] = 0;

	result->values[3][0] = 0;
	result->values[3][1] = 0;
	result->values[3][2] = 0;
	result->values[3][3] = 1;
}

void dsMatrix44f_lookAt(dsMatrix44f* result, const dsVector3f* eyePos, const dsVector3f* lookAtPos,
	const dsVector3f* upDir)
{
	DS_ASSERT(result);
	DS_ASSERT(eyePos);
	DS_ASSERT(lookAtPos);
	DS_ASSERT(upDir);

	dsVector3f zDir;
	dsVector3_sub(zDir, *eyePos, *lookAtPos);
	dsVector3f_normalize(&zDir, &zDir);

	dsVector3f xDir;
	dsVector3_cross(xDir, *upDir, zDir);
	dsVector3f_normalize(&xDir, &xDir);

	dsVector3f yDir;
	dsVector3_cross(yDir, zDir, xDir);

	result->values[0][0] = xDir.x;
	result->values[0][1] = xDir.y;
	result->values[0][2] = xDir.z;
	result->values[0][3] = 0;

	result->values[1][0] = yDir.x;
	result->values[1][1] = yDir.y;
	result->values[1][2] = yDir.z;
	result->values[1][3] = 0;

	result->values[2][0] = zDir.x;
	result->values[2][1] = zDir.y;
	result->values[2][2] = zDir.z;
	result->values[2][3] = 0;

	result->values[3][0] = eyePos->x;
	result->values[3][1] = eyePos->y;
	result->values[3][2] = eyePos->z;
	result->values[3][3] = 1;
}

void dsMatrix44d_lookAt(dsMatrix44d* result, const dsVector3d* eyePos, const dsVector3d* lookAtPos,
	const dsVector3d* upDir)
{
	DS_ASSERT(result);
	DS_ASSERT(eyePos);
	DS_ASSERT(lookAtPos);
	DS_ASSERT(upDir);

	dsVector3d zDir;
	dsVector3_sub(zDir, *eyePos, *lookAtPos);
	dsVector3d_normalize(&zDir, &zDir);

	dsVector3d xDir;
	dsVector3_cross(xDir, *upDir, zDir);
	dsVector3d_normalize(&xDir, &xDir);

	dsVector3d yDir;
	dsVector3_cross(yDir, zDir, xDir);

	result->values[0][0] = xDir.x;
	result->values[0][1] = xDir.y;
	result->values[0][2] = xDir.z;
	result->values[0][3] = 0;

	result->values[1][0] = yDir.x;
	result->values[1][1] = yDir.y;
	result->values[1][2] = yDir.z;
	result->values[1][3] = 0;

	result->values[2][0] = zDir.x;
	result->values[2][1] = zDir.y;
	result->values[2][2] = zDir.z;
	result->values[2][3] = 0;

	result->values[3][0] = eyePos->x;
	result->values[3][1] = eyePos->y;
	result->values[3][2] = eyePos->z;
	result->values[3][3] = 1;
}

void dsMatrix44f_makeOrtho(dsMatrix44f* result, float left, float right, float bottom,
	float top, float near, float far, bool halfDepth, bool invertY)
{
	DS_ASSERT(result);
	DS_ASSERT(left != right);
	DS_ASSERT(bottom != top);
	DS_ASSERT(near != far);

	float yMult = invertY ? -1.0f : 1.0f;

	result->values[0][0] = 2/(right - left);
	result->values[0][1] = 0;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = 2/(top - bottom)*yMult;
	result->values[1][2] = 0;
	result->values[1][3] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	if (halfDepth)
		result->values[2][2] = 1/(near - far);
	else
		result->values[2][2] = 2/(near - far);
	result->values[2][3] = 0;

	result->values[3][0] = (left + right)/(left - right);
	result->values[3][1] = (bottom + top)/(bottom - top)*yMult;
	if (halfDepth)
		result->values[3][2] = near/(near - far);
	else
		result->values[3][2] = (near + far)/(near - far);
	result->values[3][3] = 1;
}

void dsMatrix44d_makeOrtho(dsMatrix44d* result, double left, double right, double bottom,
	double top, double near, double far, bool halfDepth, bool invertY)
{
	DS_ASSERT(result);
	DS_ASSERT(left != right);
	DS_ASSERT(bottom != top);
	DS_ASSERT(near != far);

	double yMult = invertY ? -1.0 : 1.0;

	result->values[0][0] = 2/(right - left);
	result->values[0][1] = 0;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = 2/(top - bottom)*yMult;
	result->values[1][2] = 0;
	result->values[1][3] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	if (halfDepth)
		result->values[2][2] = 1/(near - far);
	else
		result->values[2][2] = 2/(near - far);
	result->values[2][3] = 0;

	result->values[3][0] = (left + right)/(left - right);
	result->values[3][1] = (bottom + top)/(bottom - top)*yMult;
	if (halfDepth)
		result->values[3][2] = near/(near - far);
	else
		result->values[3][2] = (near + far)/(near - far);
	result->values[3][3] = 1;
}

void dsMatrix44f_makeFrustum(dsMatrix44f* result, float left, float right, float bottom, float top,
	float near, float far, bool halfDepth, bool invertY)
{
	DS_ASSERT(result);
	DS_ASSERT(left != right);
	DS_ASSERT(bottom != top);
	DS_ASSERT(near != far);

	float yMult = invertY ? -1.0f : 1.0f;

	result->values[0][0] = 2*near/(right - left);
	result->values[0][1] = 0;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = 2*near/(top - bottom)*yMult;
	result->values[1][2] = 0;
	result->values[1][3] = 0;

	result->values[2][0] = (right + left)/(right - left);
	result->values[2][1] = (top + bottom)/(top - bottom)*yMult;
	if (halfDepth)
		result->values[2][2] = far/(near - far);
	else
		result->values[2][2] = (near + far)/(near - far);
	result->values[2][3] = -1;

	result->values[3][0] = 0;
	result->values[3][1] = 0;
	if (halfDepth)
		result->values[3][2] = near*far/(near - far);
	else
		result->values[3][2] = 2*near*far/(near - far);
	result->values[3][3] = 0;
}

void dsMatrix44d_makeFrustum(dsMatrix44d* result, double left, double right, double bottom,
	double top, double near, double far, bool halfDepth, bool invertY)
{
	DS_ASSERT(result);
	DS_ASSERT(left != right);
	DS_ASSERT(bottom != top);
	DS_ASSERT(near != far);

	double yMult = invertY ? -1.0 : 1.0;

	result->values[0][0] = 2*near/(right - left);
	result->values[0][1] = 0;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = 2*near/(top - bottom)*yMult;
	result->values[1][2] = 0;
	result->values[1][3] = 0;

	result->values[2][0] = (right + left)/(right - left);
	result->values[2][1] = (top + bottom)/(top - bottom)*yMult;
	if (halfDepth)
		result->values[2][2] = far/(near - far);
	else
		result->values[2][2] = (near + far)/(near - far);
	result->values[2][3] = -1;

	result->values[3][0] = 0;
	result->values[3][1] = 0;
	if (halfDepth)
		result->values[3][2] = near*far/(near - far);
	else
		result->values[3][2] = 2*near*far/(near - far);
	result->values[3][3] = 0;
}

void dsMatrix44f_makePerspective(dsMatrix44f* result, float fovy, float aspect, float near,
	float far, bool halfDepth, bool invertY)
{
	DS_ASSERT(result);
	DS_ASSERT(fovy > 0);
	DS_ASSERT(aspect > 0);
	DS_ASSERT(near != far);

	float height = 1/tanf(fovy/2);
	float width = height/aspect;
	float yMult = invertY ? -1.0f : 1.0f;

	result->values[0][0] = width;
	result->values[0][1] = 0;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = height*yMult;
	result->values[1][2] = 0;
	result->values[1][3] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	if (halfDepth)
		result->values[2][2] = far/(near - far);
	else
		result->values[2][2] = (near + far)/(near - far);
	result->values[2][3] = -1;

	result->values[3][0] = 0;
	result->values[3][1] = 0;
	if (halfDepth)
		result->values[3][2] = near*far/(near - far);
	else
		result->values[3][2] = 2*near*far/(near - far);
	result->values[3][3] = 0;
}

void dsMatrix44d_makePerspective(dsMatrix44d* result, double fovy, double aspect, double near,
	double far, bool halfDepth, bool invertY)
{
	DS_ASSERT(result);
	DS_ASSERT(fovy != 0);
	DS_ASSERT(aspect != 0);
	DS_ASSERT(near != far);

	double height = 1/tan(fovy/2);
	double width = height/aspect;
	double yMult = invertY ? -1.0 : 1.0;

	result->values[0][0] = width;
	result->values[0][1] = 0;
	result->values[0][2] = 0;
	result->values[0][3] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = height*yMult;
	result->values[1][2] = 0;
	result->values[1][3] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	if (halfDepth)
		result->values[2][2] = far/(near - far);
	else
		result->values[2][2] = (near + far)/(near - far);
	result->values[2][3] = -1;

	result->values[3][0] = 0;
	result->values[3][1] = 0;
	if (halfDepth)
		result->values[3][2] = near*far/(near - far);
	else
		result->values[3][2] = 2*near*far/(near - far);
	result->values[3][3] = 0;
}

void dsMatrix44f_identity(dsMatrix44f* result);
void dsMatrix44d_identity(dsMatrix44d* result);

void dsMatrix44f_mul(dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b);
void dsMatrix44d_mul(dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b);

void dsMatrix44f_affineMul(dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b);
void dsMatrix44d_affineMul(dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b);

void dsMatrix44f_transform(dsVector4f* result, const dsMatrix44f* mat, const dsVector4f* vec);
void dsMatrix44d_transform(dsVector4d* result, const dsMatrix44d* mat, const dsVector4d* vec);

void dsMatrix44f_transformTransposed(dsVector4f* result, const dsMatrix44f* mat,
	const dsVector4f* vec);
void dsMatrix44d_transformTransposed(dsVector4d* result, const dsMatrix44d* mat,
	const dsVector4d* vec);

void dsMatrix44f_transpose(dsMatrix44f* result, const dsMatrix44f* a);
void dsMatrix44d_transpose(dsMatrix44d* result, const dsMatrix44d* a);

float dsMatrix44f_determinant(dsMatrix44f* a);
double dsMatrix44d_determinant(dsMatrix44d* a);

void dsMatrix44f_fastInvert(dsMatrix44f* result, const dsMatrix44f* a);
void dsMatrix44d_fastInvert(dsMatrix44d* result, const dsMatrix44d* a);

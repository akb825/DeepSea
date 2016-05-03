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
#include <DeepSea/Core/Assert.h>

#define dsMatrix44_invertImpl(result, a, invDet) \
	do \
	{ \
		(result).values[0][0] = dsMatrix33_determinantImpl(a, 1, 2, 3, 1, 2, 3)*invDet; \
		(result).values[0][1] = dsMatrix33_determinantImpl(a, 1, 2, 3, 0, 2, 3)*invDet; \
		(result).values[0][2] = dsMatrix33_determinantImpl(a, 1, 2, 3, 0, 1, 3)*invDet; \
		(result).values[0][3] = dsMatrix33_determinantImpl(a, 1, 2, 3, 0, 1, 2)*invDet; \
		\
		(result).values[1][0] = dsMatrix33_determinantImpl(a, 0, 2, 3, 1, 2, 3)*invDet; \
		(result).values[1][1] = dsMatrix33_determinantImpl(a, 0, 2, 3, 0, 2, 3)*invDet; \
		(result).values[1][2] = dsMatrix33_determinantImpl(a, 0, 2, 3, 0, 1, 3)*invDet; \
		(result).values[1][3] = dsMatrix33_determinantImpl(a, 0, 2, 3, 0, 1, 2)*invDet; \
		\
		(result).values[2][0] = dsMatrix33_determinantImpl(a, 0, 1, 3, 1, 2, 3)*invDet; \
		(result).values[2][1] = dsMatrix33_determinantImpl(a, 0, 1, 3, 0, 2, 3)*invDet; \
		(result).values[2][2] = dsMatrix33_determinantImpl(a, 0, 1, 3, 0, 1, 3)*invDet; \
		(result).values[2][3] = dsMatrix33_determinantImpl(a, 0, 1, 3, 0, 1, 2)*invDet; \
		\
		(result).values[3][0] = dsMatrix33_determinantImpl(a, 0, 1, 2, 1, 2, 3)*invDet; \
		(result).values[3][1] = dsMatrix33_determinantImpl(a, 0, 1, 2, 0, 2, 3)*invDet; \
		(result).values[3][2] = dsMatrix33_determinantImpl(a, 0, 1, 2, 0, 1, 3)*invDet; \
		(result).values[3][3] = dsMatrix33_determinantImpl(a, 0, 1, 2, 0, 1, 2)*invDet; \
	} while (0)

#define dsMatrix44_makeRotateImpl(result, cosX, sinX, cosY, sinY, cosZ, sinZ) \
	do \
	{ \
		(result).values[0][0] = (cosY); \
		(result).values[0][1] = (sinX)*(sinZ); \
		(result).values[0][2] = -(cosZ)*(sinX); \
		(result).values[0][3] = 0; \
		\
		(result).values[1][0] = (sinY)*(sinZ); \
		(result).values[1][1] = (cosX)*(cosZ) - (cosY)*(sinX)*(sinZ); \
		(result).values[1][2] = (cosZ)*(sinX) + (cosX)*(cosY)*(sinZ); \
		(result).values[1][3] = 0; \
		\
		(result).values[2][0] = (cosZ)*(sinY); \
		(result).values[2][1] = -(cosX)*(sinZ) - (cosY)*(cosZ)*(sinX); \
		(result).values[2][2] = (cosX)*(cosY)*(cosZ) - (sinX)*(sinZ); \
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

void dsMatrix44f_affineInvert(dsMatrix44f* result, dsMatrix44f* a)
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

	result->values[3][0] = -a->values[3][0];
	result->values[3][1] = -a->values[3][1];
	result->values[3][2] = -a->values[3][2];
	result->values[3][3] = 1;
}

void dsMatrix44d_affineInvert(dsMatrix44d* result, dsMatrix44d* a)
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

	result->values[3][0] = -a->values[3][0];
	result->values[3][1] = -a->values[3][1];
	result->values[3][2] = -a->values[3][2];
	result->values[3][3] = 1;
}

void dsMatrix44f_invert(dsMatrix44f* result, dsMatrix44f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	float det = dsMatrix44_determinant(*a);
	DS_ASSERT(det != 0);
	float invDet = 1/det;

	dsMatrix44_invertImpl(*result, *a, invDet);
}

void dsMatrix44d_invert(dsMatrix44d* result, dsMatrix44d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	float det = dsMatrix44_determinant(*a);
	DS_ASSERT(det != 0);
	float invDet = 1/det;

	dsMatrix44_invertImpl(*result, *a, invDet);
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
	result->values[0][2] = 0;

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
	result->values[0][2] = 0;

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
	result->values[0][2] = 0;

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
	result->values[0][2] = 0;

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

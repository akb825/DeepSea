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

#include <DeepSea/Math/Matrix33.h>

#include "Matrix33Impl.h"
#include <DeepSea/Math/Core.h>

#define dsMatrix33_makeRotate3DImpl(result, cosX, sinX, cosY, sinY, cosZ, sinZ) \
	do \
	{ \
		(result).values[0][0] = (cosY)*(cosZ); \
		(result).values[0][1] = (cosY)*(sinZ); \
		(result).values[0][2] = -(sinY); \
		\
		(result).values[1][0] = (sinX)*(sinY)*(cosZ) - (cosX)*(sinZ); \
		(result).values[1][1] = (cosX)*(cosZ) + (sinX)*(sinY)*(sinZ); \
		(result).values[1][2] = (sinX)*(cosY); \
		\
		(result).values[2][0] = (sinX)*(sinZ) + (cosX)*(sinY)*(cosZ); \
		(result).values[2][1] = (cosX)*(sinY)*(sinZ) - (sinX)*(cosZ); \
		(result).values[2][2] = (cosX)*(cosY); \
	} while (0)

#define dsMatrix33_makeRotate3DAxisAngleImpl(result, axis, cosAngle, sinAngle, invCosAngle) \
	do \
	{ \
		(result).values[0][0] = (invCosAngle)*(axis).values[0]*(axis).values[0] + (cosAngle); \
		(result).values[0][1] = (invCosAngle)*(axis).values[0]*(axis).values[1] + \
			(axis).values[2]*(sinAngle); \
		(result).values[0][2] = (invCosAngle)*(axis).values[0]*(axis).values[2] - \
			(axis).values[1]*(sinAngle); \
		\
		(result).values[1][0] = (invCosAngle)*(axis).values[0]*(axis).values[1] - \
			(axis).values[2]*(sinAngle); \
		(result).values[1][1] = (invCosAngle)*(axis).values[1]*(axis).values[1] + (cosAngle); \
		(result).values[1][2] = (invCosAngle)*(axis).values[1]*(axis).values[2] + \
			(axis).values[0]*(sinAngle); \
		\
		(result).values[2][0] = (invCosAngle)*(axis).values[0]*(axis).values[2] + \
			(axis).values[1]*(sinAngle); \
		(result).values[2][1] = (invCosAngle)*(axis).values[1]*(axis).values[2] - \
			(axis).values[0]*(sinAngle); \
		(result).values[2][2] = (invCosAngle)*(axis).values[2]*(axis).values[2] + (cosAngle); \
	} while (0)

void dsMatrix33f_affineInvert(dsMatrix33f* result, const dsMatrix33f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	float upperDet = a->values[0][0]*a->values[1][1] -
		a->values[1][0]*a->values[0][1];
	DS_ASSERT(upperDet != 0);
	float invUpperDet = 1/upperDet;

	result->values[0][0] = a->values[1][1]*invUpperDet;
	result->values[0][1] = -a->values[0][1]*invUpperDet;
	result->values[0][2] = 0;

	result->values[1][0] = -a->values[1][0]*invUpperDet;
	result->values[1][1] = a->values[0][0]*invUpperDet;
	result->values[1][2] = 0;

	result->values[2][0] = -a->values[2][0]*result->values[0][0] -
		a->values[2][1]*result->values[1][0];
	result->values[2][1] = -a->values[2][0]*result->values[0][1] -
		a->values[2][1]*result->values[1][1];
	result->values[2][2] = 1;
}

void dsMatrix33d_affineInvert(dsMatrix33d* result, const dsMatrix33d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	double upperDet = a->values[0][0]*a->values[1][1] -
		a->values[1][0]*a->values[0][1];
	DS_ASSERT(upperDet != 0);
	double invUpperDet = 1/upperDet;

	result->values[0][0] = a->values[1][1]*invUpperDet;
	result->values[0][1] = -a->values[0][1]*invUpperDet;
	result->values[0][2] = 0;

	result->values[1][0] = -a->values[1][0]*invUpperDet;
	result->values[1][1] = a->values[0][0]*invUpperDet;
	result->values[1][2] = 0;

	result->values[2][0] = -a->values[2][0]*result->values[0][0] -
		a->values[2][1]*result->values[1][0];
	result->values[2][1] = -a->values[2][0]*result->values[0][1] -
		a->values[2][1]*result->values[1][1];
	result->values[2][2] = 1;
}

void dsMatrix33f_invert(dsMatrix33f* result, const dsMatrix33f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	float det = dsMatrix33_determinant(*a);
	DS_ASSERT(det != 0);
	float invDet = 1/det;
	dsMatrix33_invertImpl(*result, *a, invDet);
}

void dsMatrix33d_invert(dsMatrix33d* result, const dsMatrix33d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	double det = dsMatrix33_determinant(*a);
	DS_ASSERT(det != 0);
	double invDet = 1/det;
	dsMatrix33_invertImpl(*result, *a, invDet);
}

void dsMatrix33f_inverseTranspose(dsMatrix33f* result, const dsMatrix33f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	float upperDet = a->values[0][0]*a->values[1][1] -
		a->values[1][0]*a->values[0][1];
	DS_ASSERT(upperDet != 0);
	float invUpperDet = 1/upperDet;

	result->values[0][0] = a->values[1][1]*invUpperDet;
	result->values[0][1] = -a->values[1][0]*invUpperDet;
	result->values[0][2] = 0;

	result->values[1][0] = -a->values[0][1]*invUpperDet;
	result->values[1][1] = a->values[0][0]*invUpperDet;
	result->values[1][2] = 0;

	result->values[2][0] = a->values[2][0];
	result->values[2][1] = a->values[2][1];
	result->values[2][2] = 1;
}

void dsMatrix33d_inverseTranspose(dsMatrix33d* result, const dsMatrix33d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	double upperDet = a->values[0][0]*a->values[1][1] -
		a->values[1][0]*a->values[0][1];
	DS_ASSERT(upperDet != 0);
	double invUpperDet = 1/upperDet;

	result->values[0][0] = a->values[1][1]*invUpperDet;
	result->values[0][1] = -a->values[1][0]*invUpperDet;
	result->values[0][2] = 0;

	result->values[1][0] = -a->values[0][1]*invUpperDet;
	result->values[1][1] = a->values[0][0]*invUpperDet;
	result->values[1][2] = 0;

	result->values[2][0] = a->values[2][0];
	result->values[2][1] = a->values[2][1];
	result->values[2][2] = 1;
}

void dsMatrix33f_makeRotate(dsMatrix33f* result, float angle)
{
	DS_ASSERT(result);
	float cosAngle = cosf(angle);
	float sinAngle = sinf(angle);

	result->values[0][0] = cosAngle;
	result->values[0][1] = sinAngle;
	result->values[0][2] = 0;

	result->values[1][0] = -sinAngle;
	result->values[1][1] = cosAngle;
	result->values[1][2] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	result->values[2][2] = 1;
}

void dsMatrix33d_makeRotate(dsMatrix33d* result, double angle)
{
	DS_ASSERT(result);
	double cosAngle = cos(angle);
	double sinAngle = sin(angle);

	result->values[0][0] = cosAngle;
	result->values[0][1] = sinAngle;
	result->values[0][2] = 0;

	result->values[1][0] = -sinAngle;
	result->values[1][1] = cosAngle;
	result->values[1][2] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	result->values[2][2] = 1;
}

void dsMatrix33f_makeRotate3D(dsMatrix33f* result, float x, float y, float z)
{
	DS_ASSERT(result);

	float cosX = cosf(x);
	float sinX = sinf(x);
	float cosY = cosf(y);
	float sinY = sinf(y);
	float cosZ = cosf(z);
	float sinZ = sinf(z);

	dsMatrix33_makeRotate3DImpl(*result, cosX, sinX, cosY, sinY, cosZ, sinZ);
}

void dsMatrix33d_makeRotate3D(dsMatrix33d* result, double x, double y, double z)
{
	DS_ASSERT(result);

	double cosX = cos(x);
	double sinX = sin(x);
	double cosY = cos(y);
	double sinY = sin(y);
	double cosZ = cos(z);
	double sinZ = sin(z);

	dsMatrix33_makeRotate3DImpl(*result, cosX, sinX, cosY, sinY, cosZ, sinZ);
}

void dsMatrix33f_makeRotate3DAxisAngle(dsMatrix33f* result, const dsVector3f* axis,
	float angle)
{
	DS_ASSERT(result);
	DS_ASSERT(axis);

	float cosAngle = cosf(angle);
	float sinAngle = sinf(angle);
	float invCosAngle = 1 - cosAngle;

	dsMatrix33_makeRotate3DAxisAngleImpl(*result, *axis, cosAngle, sinAngle, invCosAngle);
}

void dsMatrix33d_makeRotate3DAxisAngle(dsMatrix33d* result, const dsVector3d* axis,
	double angle)
{
	DS_ASSERT(result);
	DS_ASSERT(axis);

	double cosAngle = cos(angle);
	double sinAngle = sin(angle);
	double invCosAngle = 1 - cosAngle;

	dsMatrix33_makeRotate3DAxisAngleImpl(*result, *axis, cosAngle, sinAngle, invCosAngle);
}

void dsMatrix33f_makeTranslate(dsMatrix33f* result, float x, float y)
{
	DS_ASSERT(result);
	result->values[0][0] = 1;
	result->values[0][1] = 0;
	result->values[0][2] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = 1;
	result->values[1][2] = 0;

	result->values[2][0] = x;
	result->values[2][1] = y;
	result->values[2][2] = 1;
}

void dsMatrix33d_makeTranslate(dsMatrix33d* result, double x, double y)
{
	DS_ASSERT(result);
	result->values[0][0] = 1;
	result->values[0][1] = 0;
	result->values[0][2] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = 1;
	result->values[1][2] = 0;

	result->values[2][0] = x;
	result->values[2][1] = y;
	result->values[2][2] = 1;
}

void dsMatrix33f_makeScale(dsMatrix33f* result, float x, float y)
{
	DS_ASSERT(result);
	result->values[0][0] = x;
	result->values[0][1] = 0;
	result->values[0][2] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = y;
	result->values[1][2] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	result->values[2][2] = 1;
}

void dsMatrix33d_makeScale(dsMatrix33d* result, double x, double y)
{
	DS_ASSERT(result);
	result->values[0][0] = x;
	result->values[0][1] = 0;
	result->values[0][2] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = y;
	result->values[1][2] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	result->values[2][2] = 1;
}

void dsMatrix33f_makeScale3D(dsMatrix33f* result, float x, float y, float z)
{
	DS_ASSERT(result);
	result->values[0][0] = x;
	result->values[0][1] = 0;
	result->values[0][2] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = y;
	result->values[1][2] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	result->values[2][2] = z;
}

void dsMatrix33d_makeScale3D(dsMatrix33d* result, double x, double y, double z)
{
	DS_ASSERT(result);
	result->values[0][0] = x;
	result->values[0][1] = 0;
	result->values[0][2] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = y;
	result->values[1][2] = 0;

	result->values[2][0] = 0;
	result->values[2][1] = 0;
	result->values[2][2] = z;
}

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

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Quaternion.h>
#include <DeepSea/Math/Vector3.h>
#include <DeepSea/Math/Vector4.h>

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

void dsMatrix44f_affineLerpScalar(dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b,
	float t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsVector3f one = {{1.0f, 1.0f, 1.0f}};
	dsVector4f lastColumn = {{0.0f, 0.0f, 0.0f, 1.0f}};

	dsVector3f scaleA =
	{{
		dsVector3f_len((const dsVector3f*)a->columns),
		dsVector3f_len((const dsVector3f*)(a->columns + 1)),
		dsVector3f_len((const dsVector3f*)(a->columns + 2))
	}};

	dsVector3f invScaleA;
	dsVector3_div(invScaleA, one, scaleA);

	dsVector3f scaleB =
	{{
		dsVector3f_len((const dsVector3f*)b->columns),
		dsVector3f_len((const dsVector3f*)(b->columns + 1)),
		dsVector3f_len((const dsVector3f*)(b->columns + 2))
	}};

	dsVector3f invScaleB;
	dsVector3_div(invScaleB, one, scaleB);

	dsMatrix44f rotateMatA;
	dsVector3_scale(rotateMatA.columns[0], a->columns[0], invScaleA.x);
	dsVector3_scale(rotateMatA.columns[1], a->columns[1], invScaleA.y);
	dsVector3_scale(rotateMatA.columns[2], a->columns[2], invScaleA.z);
	rotateMatA.columns[3] = lastColumn;

	dsQuaternion4f quatA;
	dsQuaternion4f_fromMatrix44(&quatA, &rotateMatA);

	dsMatrix44f rotateMatB;
	dsVector3_scale(rotateMatB.columns[0], b->columns[0], invScaleB.x);
	dsVector3_scale(rotateMatB.columns[1], b->columns[1], invScaleB.y);
	dsVector3_scale(rotateMatB.columns[2], b->columns[2], invScaleB.z);
	rotateMatB.columns[3] = lastColumn;

	dsQuaternion4f quatB;
	dsQuaternion4f_fromMatrix44(&quatB, &rotateMatB);

	dsQuaternion4f quatInterp;
	dsQuaternion4f_slerp(&quatInterp, &quatA, &quatB, t);

	dsVector3f scaleInterp;
	dsVector3f_lerp(&scaleInterp, &scaleA, &scaleB, t);

	dsMatrix44f scaleMatInterp;
	dsMatrix44f_makeScale(&scaleMatInterp, scaleInterp.x, scaleInterp.y, scaleInterp.z);

	dsMatrix44f rotateMatInterp;
	dsQuaternion4f_toMatrix44(&rotateMatInterp, &quatInterp);

	dsMatrix44f_affineMul(result, &rotateMatInterp, &scaleMatInterp);
	dsVector3f_lerp((dsVector3f*)(result->columns + 3), (const dsVector3f*)(a->columns + 3),
		(const dsVector3f*)(b->columns + 3), t);
}

void dsMatrix44d_affineLerpScalar(dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b,
	double t)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(b);
	DS_ASSERT(result != a);
	DS_ASSERT(result != b);

	dsVector3d one = {{1.0, 1.0, 1.0}};
	dsVector4d lastColumn = {{0.0, 0.0, 0.0, 1.0}};

	dsVector3d scaleA =
	{{
		dsVector3d_len((const dsVector3d*)a->columns),
		dsVector3d_len((const dsVector3d*)(a->columns + 1)),
		dsVector3d_len((const dsVector3d*)(a->columns + 2))
	}};

	dsVector3d invScaleA;
	dsVector3_div(invScaleA, one, scaleA);

	dsVector3d scaleB =
	{{
		dsVector3d_len((const dsVector3d*)b->columns),
		dsVector3d_len((const dsVector3d*)(b->columns + 1)),
		dsVector3d_len((const dsVector3d*)(b->columns + 2))
	}};

	dsVector3d invScaleB;
	dsVector3_div(invScaleB, one, scaleB);

	dsMatrix44d rotateMatA;
	dsVector3_scale(rotateMatA.columns[0], a->columns[0], invScaleA.x);
	dsVector3_scale(rotateMatA.columns[1], a->columns[1], invScaleA.y);
	dsVector3_scale(rotateMatA.columns[2], a->columns[2], invScaleA.z);
	rotateMatA.columns[3] = lastColumn;

	dsQuaternion4d quatA;
	dsQuaternion4d_fromMatrix44(&quatA, &rotateMatA);

	dsMatrix44d rotateMatB;
	dsVector3_scale(rotateMatB.columns[0], b->columns[0], invScaleB.x);
	dsVector3_scale(rotateMatB.columns[1], b->columns[1], invScaleB.y);
	dsVector3_scale(rotateMatB.columns[2], b->columns[2], invScaleB.z);
	rotateMatB.columns[3] = lastColumn;

	dsQuaternion4d quatB;
	dsQuaternion4d_fromMatrix44(&quatB, &rotateMatB);

	dsQuaternion4d quatInterp;
	dsQuaternion4d_slerp(&quatInterp, &quatA, &quatB, t);

	dsVector3d scaleInterp;
	dsVector3d_lerp(&scaleInterp, &scaleA, &scaleB, t);

	dsMatrix44d scaleMatInterp;
	dsMatrix44d_makeScale(&scaleMatInterp, scaleInterp.x, scaleInterp.y, scaleInterp.z);

	dsMatrix44d rotateMatInterp;
	dsQuaternion4d_toMatrix44(&rotateMatInterp, &quatInterp);

	dsMatrix44d_affineMul(result, &rotateMatInterp, &scaleMatInterp);
	dsVector3d_lerp((dsVector3d*)(result->columns + 3), (const dsVector3d*)(a->columns + 3),
		(const dsVector3d*)(b->columns + 3), t);
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
	float top, float near, float far, dsProjectionMatrixOptions options)
{
	DS_ASSERT(result);
	DS_ASSERT(left != right);
	DS_ASSERT(bottom != top);
	DS_ASSERT(near != far);

	float yMult = options & dsProjectionMatrixOptions_InvertY ? -1.0f : 1.0f;
	int invertZ = options & dsProjectionMatrixOptions_InvertZ;
	int halfZRange = options & dsProjectionMatrixOptions_HalfZRange;

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
	if (invertZ)
	{
		if (halfZRange)
			result->values[2][2] = 1/(far - near);
		else
			result->values[2][2] = 2/(far - near);
	}
	else
	{
		if (halfZRange)
			result->values[2][2] = 1/(near - far);
		else
			result->values[2][2] = 2/(near - far);
	}
	result->values[2][3] = 0;

	result->values[3][0] = (left + right)/(left - right);
	result->values[3][1] = (bottom + top)/(bottom - top)*yMult;
	if (invertZ)
	{
		if (halfZRange)
			result->values[3][2] = far/(far - near);
		else
			result->values[3][2] = (near + far)/(far - near);
	}
	else
	{
		if (halfZRange)
			result->values[3][2] = near/(near - far);
		else
			result->values[3][2] = (near + far)/(near - far);
	}
	result->values[3][3] = 1;
}

void dsMatrix44d_makeOrtho(dsMatrix44d* result, double left, double right, double bottom,
	double top, double near, double far, dsProjectionMatrixOptions options)
{
	DS_ASSERT(result);
	DS_ASSERT(left != right);
	DS_ASSERT(bottom != top);
	DS_ASSERT(near != far);

	double yMult = options & dsProjectionMatrixOptions_InvertY ? -1.0 : 1.0;
	int invertZ = options & dsProjectionMatrixOptions_InvertZ;
	int halfZRange = options & dsProjectionMatrixOptions_HalfZRange;

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
	if (invertZ)
	{
		if (halfZRange)
			result->values[2][2] = 1/(far - near);
		else
			result->values[2][2] = 2/(far - near);
	}
	else
	{
		if (halfZRange)
			result->values[2][2] = 1/(near - far);
		else
			result->values[2][2] = 2/(near - far);
	}
	result->values[2][3] = 0;

	result->values[3][0] = (left + right)/(left - right);
	result->values[3][1] = (bottom + top)/(bottom - top)*yMult;
	if (invertZ)
	{
		if (halfZRange)
			result->values[3][2] = far/(far - near);
		else
			result->values[3][2] = (near + far)/(far - near);
	}
	else
	{
		if (halfZRange)
			result->values[3][2] = near/(near - far);
		else
			result->values[3][2] = (near + far)/(near - far);
	}
	result->values[3][3] = 1;
}

void dsMatrix44f_makeFrustum(dsMatrix44f* result, float left, float right, float bottom, float top,
	float near, float far, dsProjectionMatrixOptions options)
{
	DS_ASSERT(result);
	DS_ASSERT(left != right);
	DS_ASSERT(bottom != top);
	DS_ASSERT(near != far);

	float yMult = options & dsProjectionMatrixOptions_InvertY ? -1.0f : 1.0f;
	int invertZ = options & dsProjectionMatrixOptions_InvertZ;
	int halfZRange = options & dsProjectionMatrixOptions_HalfZRange;
	bool infiniteFar = isinf(far);

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
	if (invertZ)
	{
		if (halfZRange)
		{
			if (infiniteFar)
				result->values[2][2] = 0;
			else
				result->values[2][2] = near/(far - near);
		}
		else
		{
			if (infiniteFar)
				result->values[2][2] = 1;
			else
				result->values[2][2] = (near + far)/(far - near);
		}
	}
	else
	{
		if (infiniteFar)
			result->values[2][2] = -1;
		else if (halfZRange)
			result->values[2][2] = far/(near - far);
		else
			result->values[2][2] = (near + far)/(near - far);
	}
	result->values[2][3] = -1;

	result->values[3][0] = 0;
	result->values[3][1] = 0;
	if (invertZ)
	{
		if (infiniteFar)
		{
			if (halfZRange)
				result->values[3][2] = near;
			else
				result->values[3][2] = 2*near;
		}
		else
		{
			if (halfZRange)
				result->values[3][2] = near*far/(far - near);
			else
				result->values[3][2] = 2*near*far/(far - near);
		}
	}
	else
	{
		if (infiniteFar)
		{
			if (halfZRange)
				result->values[3][2] = -near;
			else
				result->values[3][2] = -2*near;
		}
		else
		{
			if (halfZRange)
				result->values[3][2] = near*far/(near - far);
			else
				result->values[3][2] = 2*near*far/(near - far);
		}
	}
	result->values[3][3] = 0;
}

void dsMatrix44d_makeFrustum(dsMatrix44d* result, double left, double right, double bottom,
	double top, double near, double far, dsProjectionMatrixOptions options)
{
	DS_ASSERT(result);
	DS_ASSERT(left != right);
	DS_ASSERT(bottom != top);
	DS_ASSERT(near != far);

	double yMult = options & dsProjectionMatrixOptions_InvertY ? -1.0 : 1.0;
	int invertZ = options & dsProjectionMatrixOptions_InvertZ;
	int halfZRange = options & dsProjectionMatrixOptions_HalfZRange;
	bool infiniteFar = isinf(far);

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
	if (invertZ)
	{
		if (halfZRange)
		{
			if (infiniteFar)
				result->values[2][2] = 0;
			else
				result->values[2][2] = near/(far - near);
		}
		else
		{
			if (infiniteFar)
				result->values[2][2] = 1;
			else
				result->values[2][2] = (near + far)/(far - near);
		}
	}
	else
	{
		if (infiniteFar)
			result->values[2][2] = -1;
		else if (halfZRange)
			result->values[2][2] = far/(near - far);
		else
			result->values[2][2] = (near + far)/(near - far);
	}
	result->values[2][3] = -1;

	result->values[3][0] = 0;
	result->values[3][1] = 0;
	if (invertZ)
	{
		if (infiniteFar)
		{
			if (halfZRange)
				result->values[3][2] = near;
			else
				result->values[3][2] = 2*near;
		}
		else
		{
			if (halfZRange)
				result->values[3][2] = near*far/(far - near);
			else
				result->values[3][2] = 2*near*far/(far - near);
		}
	}
	else
	{
		if (infiniteFar)
		{
			if (halfZRange)
				result->values[3][2] = -near;
			else
				result->values[3][2] = -2*near;
		}
		else
		{
			if (halfZRange)
				result->values[3][2] = near*far/(near - far);
			else
				result->values[3][2] = 2*near*far/(near - far);
		}
	}
	result->values[3][3] = 0;
}

void dsMatrix44f_makePerspective(dsMatrix44f* result, float fovy, float aspect, float near,
	float far, dsProjectionMatrixOptions options)
{
	DS_ASSERT(result);
	DS_ASSERT(fovy > 0);
	DS_ASSERT(aspect > 0);
	DS_ASSERT(near != far);

	float height = 1/tanf(fovy/2);
	float width = height/aspect;
	float yMult = options & dsProjectionMatrixOptions_InvertY ? -1.0f : 1.0f;
	int invertZ = options & dsProjectionMatrixOptions_InvertZ;
	int halfZRange = options & dsProjectionMatrixOptions_HalfZRange;
	bool infiniteFar = isinf(far);

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
	if (invertZ)
	{
		if (halfZRange)
		{
			if (infiniteFar)
				result->values[2][2] = 0;
			else
				result->values[2][2] = near/(far - near);
		}
		else
		{
			if (infiniteFar)
				result->values[2][2] = 1;
			else
				result->values[2][2] = (near + far)/(far - near);
		}
	}
	else
	{
		if (infiniteFar)
			result->values[2][2] = -1;
		else if (halfZRange)
			result->values[2][2] = far/(near - far);
		else
			result->values[2][2] = (near + far)/(near - far);
	}
	result->values[2][3] = -1;

	result->values[3][0] = 0;
	result->values[3][1] = 0;
	if (invertZ)
	{
		if (infiniteFar)
		{
			if (halfZRange)
				result->values[3][2] = near;
			else
				result->values[3][2] = 2*near;
		}
		else
		{
			if (halfZRange)
				result->values[3][2] = near*far/(far - near);
			else
				result->values[3][2] = 2*near*far/(far - near);
		}
	}
	else
	{
		if (infiniteFar)
		{
			if (halfZRange)
				result->values[3][2] = -near;
			else
				result->values[3][2] = -2*near;
		}
		else
		{
			if (halfZRange)
				result->values[3][2] = near*far/(near - far);
			else
				result->values[3][2] = 2*near*far/(near - far);
		}
	}
	result->values[3][3] = 0;
}

void dsMatrix44d_makePerspective(dsMatrix44d* result, double fovy, double aspect, double near,
	double far, dsProjectionMatrixOptions options)
{
	DS_ASSERT(result);
	DS_ASSERT(fovy != 0);
	DS_ASSERT(aspect != 0);
	DS_ASSERT(near != far);

	double height = 1/tan(fovy/2);
	double width = height/aspect;
	double yMult = options & dsProjectionMatrixOptions_InvertY ? -1.0 : 1.0;
	int invertZ = options & dsProjectionMatrixOptions_InvertZ;
	int halfZRange = options & dsProjectionMatrixOptions_HalfZRange;
	bool infiniteFar = isinf(far);

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
	if (invertZ)
	{
		if (halfZRange)
		{
			if (infiniteFar)
				result->values[2][2] = 0;
			else
				result->values[2][2] = near/(far - near);
		}
		else
		{
			if (infiniteFar)
				result->values[2][2] = 1;
			else
				result->values[2][2] = (near + far)/(far - near);
		}
	}
	else
	{
		if (infiniteFar)
			result->values[2][2] = -1;
		else if (halfZRange)
			result->values[2][2] = far/(near - far);
		else
			result->values[2][2] = (near + far)/(near - far);
	}
	result->values[2][3] = -1;

	result->values[3][0] = 0;
	result->values[3][1] = 0;
	if (invertZ)
	{
		if (infiniteFar)
		{
			if (halfZRange)
				result->values[3][2] = near;
			else
				result->values[3][2] = 2*near;
		}
		else
		{
			if (halfZRange)
				result->values[3][2] = near*far/(far - near);
			else
				result->values[3][2] = 2*near*far/(far - near);
		}
	}
	else
	{
		if (infiniteFar)
		{
			if (halfZRange)
				result->values[3][2] = -near;
			else
				result->values[3][2] = -2*near;
		}
		else
		{
			if (halfZRange)
				result->values[3][2] = near*far/(near - far);
			else
				result->values[3][2] = 2*near*far/(near - far);
		}
	}
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
void dsMatrix44f_affineInvert(dsMatrix44f* result, const dsMatrix44f* a);
void dsMatrix44d_affineInvert(dsMatrix44d* result, const dsMatrix44d* a);
void dsMatrix44f_affineInvert33(dsMatrix33f* result, const dsMatrix44f* a);
void dsMatrix44d_affineInvert33(dsMatrix33d* result, const dsMatrix44d* a);
void dsMatrix44f_invert(dsMatrix44f* result, const dsMatrix44f* a);
void dsMatrix44d_invert(dsMatrix44d* result, const dsMatrix44d* a);
void dsMatrix44f_inverseTranspose(dsMatrix33f* result, const dsMatrix44f* a);
void dsMatrix44d_inverseTranspose(dsMatrix33d* result, const dsMatrix44d* a);
void dsMatrix44f_makeTranslate(dsMatrix44f* result, float x, float y, float z);
void dsMatrix44d_makeTranslate(dsMatrix44d* result, double x, double y, double z);
void dsMatrix44f_makeScale(dsMatrix44f* result, float x, float y, float z);
void dsMatrix44d_makeScale(dsMatrix44d* result, double x, double y, double z);
void dsMatrix44f_affineLerpScalar(dsMatrix44f* result, const dsMatrix44f* a, const dsMatrix44f* b,
	float t);
void dsMatrix44d_affineLerpScalar(dsMatrix44d* result, const dsMatrix44d* a, const dsMatrix44d* b,
	double t);

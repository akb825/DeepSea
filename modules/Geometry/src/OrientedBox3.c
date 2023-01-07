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

#include <DeepSea/Geometry/OrientedBox3.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector3.h>
#include <float.h>

void dsOrientedBox3f_fromMatrix(dsOrientedBox3f* result, const dsMatrix44f* matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);

	result->halfExtents.x = dsVector3f_len((dsVector3f*)matrix->columns);
	result->halfExtents.y = dsVector3f_len((dsVector3f*)(matrix->columns + 1));
	result->halfExtents.z = dsVector3f_len((dsVector3f*)(matrix->columns + 2));

	float invLen = 1/result->halfExtents.x;
	dsVector3_scale(result->orientation.columns[0], matrix->columns[0], invLen);
	invLen = 1/result->halfExtents.y;
	dsVector3_scale(result->orientation.columns[1], matrix->columns[1], invLen);
	invLen = 1/result->halfExtents.z;
	dsVector3_scale(result->orientation.columns[2], matrix->columns[2], invLen);

	result->center = *(dsVector3f*)(matrix->columns + 3);
}

void dsOrientedBox3d_fromMatrix(dsOrientedBox3d* result, const dsMatrix44d* matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);

	result->halfExtents.x = dsVector3d_len((dsVector3d*)matrix->columns);
	result->halfExtents.y = dsVector3d_len((dsVector3d*)(matrix->columns + 1));
	result->halfExtents.z = dsVector3d_len((dsVector3d*)(matrix->columns + 2));

	double invLen = 1/result->halfExtents.x;
	dsVector3_scale(result->orientation.columns[0], matrix->columns[0], invLen);
	invLen = 1/result->halfExtents.y;
	dsVector3_scale(result->orientation.columns[1], matrix->columns[1], invLen);
	invLen = 1/result->halfExtents.z;
	dsVector3_scale(result->orientation.columns[2], matrix->columns[2], invLen);

	result->center = *(dsVector3d*)(matrix->columns + 3);
}

bool dsOrientedBox3f_transform(dsOrientedBox3f* box, const dsMatrix44f* transform)
{
	DS_ASSERT(box);
	DS_ASSERT(transform);

	if (!dsOrientedBox3_isValid(*box))
		return false;

	dsMatrix44f matrix, transformedMatrix;
	dsOrientedBox3_toMatrix(matrix, *box);
	dsMatrix44f_affineMul(&transformedMatrix, transform, &matrix);
	dsOrientedBox3f_fromMatrix(box, &transformedMatrix);
	return true;
}

bool dsOrientedBox3d_transform(dsOrientedBox3d* box, const dsMatrix44d* transform)
{
	DS_ASSERT(box);
	DS_ASSERT(transform);

	if (!dsOrientedBox3_isValid(*box))
		return false;

	dsMatrix44d matrix, transformedMatrix;
	dsOrientedBox3_toMatrix(matrix, *box);
	dsMatrix44_affineMul(transformedMatrix, *transform, matrix);
	dsOrientedBox3d_fromMatrix(box, &transformedMatrix);
	return true;
}

void dsOrientedBox3f_addPoint(dsOrientedBox3f* box, const dsVector3f* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (dsOrientedBox3_isValid(*box))
	{
		dsVector3f localPoint;
		dsVector3f centeredPoint;
		dsVector3_sub(centeredPoint, *point, box->center);
		dsMatrix33_transformTransposed(localPoint, box->orientation, centeredPoint);

		dsAlignedBox3f localBox =
		{
			{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
			{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
		};

		dsAlignedBox3_addPoint(localBox, localPoint);

		dsVector3f localCenterOffset, centerOffset;
		dsAlignedBox3_center(localCenterOffset, localBox);
		dsMatrix33_transform(centerOffset, box->orientation, localCenterOffset);
		dsVector3_add(box->center, box->center, centerOffset);

		dsAlignedBox3_extents(box->halfExtents, localBox);
		dsVector3_scale(box->halfExtents, box->halfExtents, 0.5f);
	}
	else
	{
		box->center = *point;
		box->halfExtents.x = 0;
		box->halfExtents.y = 0;
		box->halfExtents.z = 0;
	}
}

void dsOrientedBox3d_addPoint(dsOrientedBox3d* box, const dsVector3d* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (dsOrientedBox3_isValid(*box))
	{
		dsVector3d localPoint;
		dsVector3d centeredPoint;
		dsVector3_sub(centeredPoint, *point, box->center);
		dsMatrix33_transformTransposed(localPoint, box->orientation, centeredPoint);

		dsAlignedBox3d localBox =
		{
			{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
			{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
		};

		dsAlignedBox3_addPoint(localBox, localPoint);

		dsVector3d localCenterOffset, centerOffset;
		dsAlignedBox3_center(localCenterOffset, localBox);
		dsMatrix33_transform(centerOffset, box->orientation, localCenterOffset);
		dsVector3_add(box->center, box->center, centerOffset);

		dsAlignedBox3_extents(box->halfExtents, localBox);
		dsVector3_scale(box->halfExtents, box->halfExtents, 0.5);
	}
	else
	{
		box->center = *point;
		box->halfExtents.x = 0;
		box->halfExtents.y = 0;
		box->halfExtents.z = 0;
	}
}

bool dsOrientedBox3f_addBox(dsOrientedBox3f* box, const dsOrientedBox3f* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);

	if (!dsOrientedBox3_isValid(*otherBox))
		return false;

	if (dsOrientedBox3_isValid(*box))
	{
		dsAlignedBox3f localBox =
		{
			{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
			{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
		};

		dsVector3f corners[DS_BOX3_CORNER_COUNT];
		DS_VERIFY(dsOrientedBox3f_corners(corners, otherBox));
		for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
		{
			dsVector3f localCorner;
			dsVector3f centeredPoint;
			dsVector3_sub(centeredPoint, corners[i], box->center);
			dsMatrix33_transformTransposed(localCorner, box->orientation, centeredPoint);

			dsAlignedBox3_addPoint(localBox, localCorner);
		}

		dsVector3f localCenterOffset, centerOffset;
		dsAlignedBox3_center(localCenterOffset, localBox);
		dsMatrix33_transform(centerOffset, box->orientation, localCenterOffset);
		dsVector3_add(box->center, box->center, centerOffset);

		dsAlignedBox3_extents(box->halfExtents, localBox);
		dsVector3_scale(box->halfExtents, box->halfExtents, 0.5f);
	}
	else
		*box = *otherBox;

	return true;
}

bool dsOrientedBox3d_addBox(dsOrientedBox3d* box, const dsOrientedBox3d* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);

	if (!dsOrientedBox3_isValid(*otherBox))
		return false;

	if (dsOrientedBox3_isValid(*box))
	{
		dsAlignedBox3d localBox =
		{
			{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
			{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
		};

		dsVector3d corners[DS_BOX3_CORNER_COUNT];
		DS_VERIFY(dsOrientedBox3d_corners(corners, otherBox));
		for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
		{
			dsVector3d localCorner;
			dsVector3d centeredPoint;
			dsVector3_sub(centeredPoint, corners[i], box->center);
			dsMatrix33_transformTransposed(localCorner, box->orientation, centeredPoint);

			dsAlignedBox3_addPoint(localBox, localCorner);
		}

		dsVector3d localCenterOffset, centerOffset;
		dsAlignedBox3_center(localCenterOffset, localBox);
		dsMatrix33_transform(centerOffset, box->orientation, localCenterOffset);
		dsVector3_add(box->center, box->center, centerOffset);

		dsAlignedBox3_extents(box->halfExtents, localBox);
		dsVector3_scale(box->halfExtents, box->halfExtents, 0.5);
	}
	else
		*box = *otherBox;

	return true;
}

bool dsOrientedBox3f_corners(dsVector3f corners[DS_BOX3_CORNER_COUNT], const dsOrientedBox3f* box)
{
	DS_ASSERT(corners);
	DS_ASSERT(box);

	if (!dsOrientedBox3_isValid(*box))
		return false;

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

	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
	{
		dsVector3f worldOffset;
		dsMatrix33_transform(worldOffset, box->orientation, corners[i]);
		dsVector3_add(corners[i], worldOffset, box->center);
	}

	return true;
}

bool dsOrientedBox3d_corners(dsVector3d corners[DS_BOX3_CORNER_COUNT], const dsOrientedBox3d* box)
{
	DS_ASSERT(corners);
	DS_ASSERT(box);

	if (!dsOrientedBox3_isValid(*box))
		return false;

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

	for (unsigned int i = 0; i < DS_BOX3_CORNER_COUNT; ++i)
	{
		dsVector3d worldOffset;
		dsMatrix33_transform(worldOffset, box->orientation, corners[i]);
		dsVector3_add(corners[i], worldOffset, box->center);
	}

	return true;
}

bool dsOrientedBox3f_intersects(const dsOrientedBox3f* box, const dsOrientedBox3f* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);

	if (!dsOrientedBox3_isValid(*box) || !dsOrientedBox3_isValid(*otherBox))
		return false;

	// Optimized separating axes as explained by
	// https://www.geometrictools.com/Documentation/DynamicCollisionDetection.pdf
	// See also:
	// https://github.com/davideberly/GeometricTools/blob/master/GTE/Mathematics/IntrOrientedBox3OrientedBox3.h
	dsVector3f centerDiff;
	dsVector3_sub(centerDiff, otherBox->center, box->center);

	const float parallelCutoff = 1.0f - 1e-6f;
	float dotDiffAxes[3];
	float dotAxes[3][3];
	float absDotAxes[3][3];
	bool hasParallel = false;
	for (unsigned int i = 0; i < 3; ++i)
	{
		const dsVector3f* boxAxis = box->orientation.columns + i;
		dotDiffAxes[i] = dsVector3_dot(centerDiff, *boxAxis);
		for (unsigned int j = 0; j < 3; ++j)
		{
			dotAxes[i][j] = dsVector3_dot(*boxAxis, otherBox->orientation.columns[j]);
			absDotAxes[i][j] = fabsf(dotAxes[i][j]);
			if (dotAxes[i][j] >= parallelCutoff)
				hasParallel = true;
		}

		// Test axes for first box against second box.
		float radius = box->halfExtents.values[i] + otherBox->halfExtents.x*absDotAxes[i][0] +
			otherBox->halfExtents.y*absDotAxes[i][1] + otherBox->halfExtents.z*absDotAxes[i][2];
		if (fabsf(dsVector3_dot(box->orientation.columns[i], centerDiff)) > radius)
			return false;
	}

	// Test axes for second box against first box.
	for (unsigned int i = 0; i < 3; ++i)
	{
		float radius = box->halfExtents.x*absDotAxes[0][i] + box->halfExtents.y*absDotAxes[1][i] +
			box->halfExtents.z*absDotAxes[2][i] + otherBox->halfExtents.values[i];
		if (fabsf(dsVector3_dot(otherBox->orientation.columns[i], centerDiff)) > radius)
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
}

bool dsOrientedBox3d_intersects(const dsOrientedBox3d* box, const dsOrientedBox3d* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);

	if (!dsOrientedBox3_isValid(*box) || !dsOrientedBox3_isValid(*otherBox))
		return false;

	// Optimized separating axes as explained by
	// https://www.geometrictools.com/Documentation/DynamicCollisionDetection.pdf
	// See also:
	// https://github.com/davideberly/GeometricTools/blob/master/GTE/Mathematics/IntrOrientedBox3OrientedBox3.h
	dsVector3d centerDiff;
	dsVector3_sub(centerDiff, otherBox->center, box->center);

	const double parallelCutoff = 1.0 - 1e-14f;
	double dotDiffAxes[3];
	double dotAxes[3][3];
	double absDotAxes[3][3];
	bool hasParallel = false;
	for (unsigned int i = 0; i < 3; ++i)
	{
		const dsVector3d* boxAxis = box->orientation.columns + i;
		dotDiffAxes[i] = dsVector3_dot(centerDiff, *boxAxis);
		for (unsigned int j = 0; j < 3; ++j)
		{
			dotAxes[i][j] = dsVector3_dot(*boxAxis, otherBox->orientation.columns[j]);
			absDotAxes[i][j] = fabs(dotAxes[i][j]);
			if (dotAxes[i][j] >= parallelCutoff)
				hasParallel = true;
		}

		// Test axes for first box against second box.
		double radius = box->halfExtents.values[i] + otherBox->halfExtents.x*absDotAxes[i][0] +
			otherBox->halfExtents.y*absDotAxes[i][1] + otherBox->halfExtents.z*absDotAxes[i][2];
		if (fabs(dsVector3_dot(box->orientation.columns[i], centerDiff)) > radius)
			return false;
	}

	// Test axes for second box against first box.
	for (unsigned int i = 0; i < 3; ++i)
	{
		double radius = box->halfExtents.x*absDotAxes[0][i] + box->halfExtents.y*absDotAxes[1][i] +
			box->halfExtents.z*absDotAxes[2][i] + otherBox->halfExtents.values[i];
		if (fabs(dsVector3_dot(otherBox->orientation.columns[i], centerDiff)) > radius)
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
}

bool dsOrientedBox3f_containsPoint(const dsOrientedBox3f* box, const dsVector3f* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox3_isValid(*box))
		return false;

	dsAlignedBox3f localBox =
	{
		{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
		{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
	};

	dsVector3f localPoint;
	dsVector3f centeredPoint;
	dsVector3_sub(centeredPoint, *point, box->center);
	dsMatrix33_transformTransposed(localPoint, box->orientation, centeredPoint);
	return dsAlignedBox3_containsPoint(localBox, localPoint);
}

bool dsOrientedBox3d_containsPoint(const dsOrientedBox3d* box, const dsVector3d* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox3_isValid(*box))
		return false;

	dsAlignedBox3d localBox =
	{
		{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
		{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
	};

	dsVector3d localPoint;
	dsVector3d centeredPoint;
	dsVector3_sub(centeredPoint, *point, box->center);
	dsMatrix33_transformTransposed(localPoint, box->orientation, centeredPoint);
	return dsAlignedBox3_containsPoint(localBox, localPoint);
}

bool dsOrientedBox3f_closestPoint(dsVector3f* result, const dsOrientedBox3f* box,
	const dsVector3f* point)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox3_isValid(*box))
		return false;

	dsAlignedBox3f localBox =
	{
		{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
		{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
	};

	dsVector3f localPoint;
	dsVector3f centeredPoint;
	dsVector3_sub(centeredPoint, *point, box->center);
	dsMatrix33_transformTransposed(localPoint, box->orientation, centeredPoint);

	dsVector3f localResult;
	dsAlignedBox3_closestPoint(localResult, localBox, localPoint);
	dsMatrix33_transform(*result, box->orientation, localResult);
	dsVector3_add(*result, *result, box->center);

	return true;
}

bool dsOrientedBox3d_closestPoint(dsVector3d* result, const dsOrientedBox3d* box,
	const dsVector3d* point)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox3_isValid(*box))
		return false;

	dsAlignedBox3d localBox =
	{
		{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
		{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
	};

	dsVector3d localPoint;
	dsVector3d centeredPoint;
	dsVector3_sub(centeredPoint, *point, box->center);
	dsMatrix33_transformTransposed(localPoint, box->orientation, centeredPoint);

	dsVector3d localResult;
	dsAlignedBox3_closestPoint(localResult, localBox, localPoint);
	dsMatrix33_transform(*result, box->orientation, localResult);
	dsVector3_add(*result, *result, box->center);

	return true;
}

float dsOrientedBox3f_dist2(const dsOrientedBox3f* box, const dsVector3f* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox3_isValid(*box))
		return -1;

	dsAlignedBox3f localBox =
	{
		{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
		{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
	};

	dsVector3f localPoint;
	dsVector3f centeredPoint;
	dsVector3_sub(centeredPoint, *point, box->center);
	dsMatrix33_transformTransposed(localPoint, box->orientation, centeredPoint);

	return dsAlignedBox3f_dist2(&localBox, &localPoint);
}

double dsOrientedBox3d_dist2(const dsOrientedBox3d* box, const dsVector3d* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox3_isValid(*box))
		return -1;

	dsAlignedBox3d localBox =
	{
		{{-box->halfExtents.x, -box->halfExtents.y, -box->halfExtents.z}},
		{{box->halfExtents.x, box->halfExtents.y, box->halfExtents.z}}
	};

	dsVector3d localPoint;
	dsVector3d centeredPoint;
	dsVector3_sub(centeredPoint, *point, box->center);
	dsMatrix33_transformTransposed(localPoint, box->orientation, centeredPoint);

	return dsAlignedBox3d_dist2(&localBox, &localPoint);
}

float dsOrientedBox3f_dist(const dsOrientedBox3f* box, const dsVector3f* point)
{
	float distance2 = dsOrientedBox3f_dist2(box, point);
	if (distance2 <= 0)
		return distance2;

	return sqrtf(distance2);
}

double dsOrientedBox3d_dist(const dsOrientedBox3d* box, const dsVector3d* point)
{
	double distance2 = dsOrientedBox3d_dist2(box, point);
	if (distance2 <= 0)
		return distance2;

	return sqrt(distance2);
}

bool dsOrientedBox3f_isValid(const dsOrientedBox3f* box);
bool dsOrientedBox3d_isValid(const dsOrientedBox3d* box);
void dsOrientedBox3f_toMatrix(dsMatrix44f* result, const dsOrientedBox3f* box);
void dsOrientedBox3d_toMatrix(dsMatrix44d* result, const dsOrientedBox3d* box);
void dsOrientedBox3f_toMatrixTranspose(dsMatrix44f* result, const dsOrientedBox3f* box);
void dsOrientedBox3d_toMatrixTranspose(dsMatrix44d* result, const dsOrientedBox3d* box);
void dsOrientedBox3f_fromAlignedBox(dsOrientedBox3f* result, const dsAlignedBox3f* alignedBox);
void dsOrientedBox3d_fromAlignedBox(dsOrientedBox3d* result, const dsAlignedBox3d* alignedBox);
void dsOrientedBox3f_makeInvalid(dsOrientedBox3f* result);
void dsOrientedBox3d_makeInvalid(dsOrientedBox3d* result);

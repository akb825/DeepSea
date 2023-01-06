/*
 * Copyright 2016-2022 Aaron Barany
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

#include <DeepSea/Geometry/OrientedBox2.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Vector2.h>
#include <float.h>

void dsOrientedBox2f_fromMatrix(dsOrientedBox2f* result, const dsMatrix33f* matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);

	result->halfExtents.x = dsVector2f_len((dsVector2f*)matrix->columns);
	result->halfExtents.y = dsVector2f_len((dsVector2f*)(matrix->columns + 1));

	float invLen = 1/result->halfExtents.x;
	dsVector2_scale(result->orientation.columns[0], matrix->columns[0], invLen);
	invLen = 1/result->halfExtents.y;
	dsVector2_scale(result->orientation.columns[1], matrix->columns[1], invLen);

	result->center = *(dsVector2f*)(matrix->columns + 2);
}

void dsOrientedBox2d_fromMatrix(dsOrientedBox2d* result, const dsMatrix33d* matrix)
{
	DS_ASSERT(result);
	DS_ASSERT(matrix);

	result->halfExtents.x = dsVector2d_len((dsVector2d*)matrix->columns);
	result->halfExtents.y = dsVector2d_len((dsVector2d*)(matrix->columns + 1));

	double invLen = 1/result->halfExtents.x;
	dsVector2_scale(result->orientation.columns[0], matrix->columns[0], invLen);
	invLen = 1/result->halfExtents.y;
	dsVector2_scale(result->orientation.columns[1], matrix->columns[1], invLen);

	result->center = *(dsVector2d*)(matrix->columns + 2);
}

bool dsOrientedBox2f_transform(dsOrientedBox2f* box, const dsMatrix33f* transform)
{
	DS_ASSERT(box);
	DS_ASSERT(transform);

	if (!dsOrientedBox2_isValid(*box))
		return false;

	dsMatrix33f matrix, transformedMatrix;
	dsOrientedBox2_toMatrix(matrix, *box);
	dsMatrix33_affineMul(transformedMatrix, *transform, matrix);
	dsOrientedBox2f_fromMatrix(box, &transformedMatrix);
	return true;
}

bool dsOrientedBox2d_transform(dsOrientedBox2d* box, const dsMatrix33d* transform)
{
	DS_ASSERT(box);
	DS_ASSERT(transform);

	if (!dsOrientedBox2_isValid(*box))
		return false;

	dsMatrix33d matrix, transformedMatrix;
	dsOrientedBox2_toMatrix(matrix, *box);
	dsMatrix33_affineMul(transformedMatrix, *transform, matrix);
	dsOrientedBox2d_fromMatrix(box, &transformedMatrix);
	return true;
}

void dsOrientedBox2f_addPoint(dsOrientedBox2f* box, const dsVector2f* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (dsOrientedBox2_isValid(*box))
	{
		dsVector2f localPoint;
		dsVector2f centeredPoint;
		dsVector2_sub(centeredPoint, *point, box->center);
		dsMatrix22_transform(localPoint, box->orientation, centeredPoint);

		dsAlignedBox2f localBox =
		{
			{{-box->halfExtents.x, -box->halfExtents.y}},
			{{box->halfExtents.x, box->halfExtents.y}}
		};

		dsAlignedBox2_addPoint(localBox, localPoint);

		dsVector2f localCenterOffset, centerOffset;
		dsAlignedBox2_center(localCenterOffset, localBox);
		dsMatrix22_transformTransposed(centerOffset, box->orientation, localCenterOffset);
		dsVector2_add(box->center, box->center, centerOffset);

		dsAlignedBox2_extents(box->halfExtents, localBox);
		dsVector2_scale(box->halfExtents, box->halfExtents, 0.5f);
	}
	else
	{
		box->center = *point;
		box->halfExtents.x = 0;
		box->halfExtents.y = 0;
	}
}

void dsOrientedBox2d_addPoint(dsOrientedBox2d* box, const dsVector2d* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (dsOrientedBox2_isValid(*box))
	{
		dsVector2d localPoint;
		dsVector2d centeredPoint;
		dsVector2_sub(centeredPoint, *point, box->center);
		dsMatrix22_transformTransposed(localPoint, box->orientation, centeredPoint);

		dsAlignedBox2d localBox =
		{
			{{-box->halfExtents.x, -box->halfExtents.y}},
			{{box->halfExtents.x, box->halfExtents.y}}
		};

		dsAlignedBox2_addPoint(localBox, localPoint);

		dsVector2d localCenterOffset, centerOffset;
		dsAlignedBox2_center(localCenterOffset, localBox);
		dsMatrix22_transform(centerOffset, box->orientation, localCenterOffset);
		dsVector2_add(box->center, box->center, centerOffset);

		dsAlignedBox2_extents(box->halfExtents, localBox);
		dsVector2_scale(box->halfExtents, box->halfExtents, 0.5);
	}
	else
	{
		box->center = *point;
		box->halfExtents.x = 0;
		box->halfExtents.y = 0;
	}
}

bool dsOrientedBox2f_addBox(dsOrientedBox2f* box, const dsOrientedBox2f* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);

	if (!dsOrientedBox2_isValid(*otherBox))
		return false;

	if (dsOrientedBox2_isValid(*box))
	{
		dsAlignedBox2f localBox =
		{
			{{-box->halfExtents.x, -box->halfExtents.y}},
			{{box->halfExtents.x, box->halfExtents.y}}
		};

		dsVector2f corners[DS_BOX2_CORNER_COUNT];
		DS_VERIFY(dsOrientedBox2f_corners(corners, otherBox));
		for (unsigned int i = 0; i < DS_BOX2_CORNER_COUNT; ++i)
		{
			dsVector2f localCorner;
			dsVector2f centeredPoint;
			dsVector2_sub(centeredPoint, corners[i], box->center);
			dsMatrix22_transformTransposed(localCorner, box->orientation, centeredPoint);

			dsAlignedBox2_addPoint(localBox, localCorner);
		}

		dsVector2f localCenterOffset, centerOffset;
		dsAlignedBox2_center(localCenterOffset, localBox);
		dsMatrix22_transform(centerOffset, box->orientation, localCenterOffset);
		dsVector2_add(box->center, box->center, centerOffset);

		dsAlignedBox2_extents(box->halfExtents, localBox);
		dsVector2_scale(box->halfExtents, box->halfExtents, 0.5f);
	}
	else
		*box = *otherBox;

	return true;
}

bool dsOrientedBox2d_addBox(dsOrientedBox2d* box, const dsOrientedBox2d* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);

	if (!dsOrientedBox2_isValid(*otherBox))
		return false;

	if (dsOrientedBox2_isValid(*box))
	{
		dsAlignedBox2d localBox =
		{
			{{-box->halfExtents.x, -box->halfExtents.y}},
			{{box->halfExtents.x, box->halfExtents.y}}
		};

		dsVector2d corners[DS_BOX2_CORNER_COUNT];
		DS_VERIFY(dsOrientedBox2d_corners(corners, otherBox));
		for (unsigned int i = 0; i < DS_BOX2_CORNER_COUNT; ++i)
		{
			dsVector2d localCorner;
			dsVector2d centeredPoint;
			dsVector2_sub(centeredPoint, corners[i], box->center);
			dsMatrix22_transformTransposed(localCorner, box->orientation, centeredPoint);

			dsAlignedBox2_addPoint(localBox, localCorner);
		}

		dsVector2d localCenterOffset, centerOffset;
		dsAlignedBox2_center(localCenterOffset, localBox);
		dsMatrix22_transform(centerOffset, box->orientation, localCenterOffset);
		dsVector2_add(box->center, box->center, centerOffset);

		dsAlignedBox2_extents(box->halfExtents, localBox);
		dsVector2_scale(box->halfExtents, box->halfExtents, 0.5);
	}
	else
		*box = *otherBox;

	return true;
}

bool dsOrientedBox2f_corners(dsVector2f corners[DS_BOX2_CORNER_COUNT], const dsOrientedBox2f* box)
{
	DS_ASSERT(corners);
	DS_ASSERT(box);

	if (!dsOrientedBox2_isValid(*box))
		return false;

	corners[dsBox2Corner_xy].x = -box->halfExtents.x;
	corners[dsBox2Corner_xy].y = -box->halfExtents.y;

	corners[dsBox2Corner_xY].x = -box->halfExtents.x;
	corners[dsBox2Corner_xY].y = box->halfExtents.y;

	corners[dsBox2Corner_Xy].x = box->halfExtents.x;
	corners[dsBox2Corner_Xy].y = -box->halfExtents.y;

	corners[dsBox2Corner_XY].x = box->halfExtents.x;
	corners[dsBox2Corner_XY].y = box->halfExtents.y;

	for (unsigned int i = 0; i < DS_BOX2_CORNER_COUNT; ++i)
	{
		dsVector2f worldOffset;
		dsMatrix22_transform(worldOffset, box->orientation, corners[i]);
		dsVector2_add(corners[i], worldOffset, box->center);
	}

	return true;
}

bool dsOrientedBox2d_corners(dsVector2d corners[DS_BOX2_CORNER_COUNT], const dsOrientedBox2d* box)
{
	DS_ASSERT(corners);
	DS_ASSERT(box);

	if (!dsOrientedBox2_isValid(*box))
		return false;

	corners[dsBox2Corner_xy].x = -box->halfExtents.x;
	corners[dsBox2Corner_xy].y = -box->halfExtents.y;

	corners[dsBox2Corner_xY].x = -box->halfExtents.x;
	corners[dsBox2Corner_xY].y = box->halfExtents.y;

	corners[dsBox2Corner_Xy].x = box->halfExtents.x;
	corners[dsBox2Corner_Xy].y = -box->halfExtents.y;

	corners[dsBox2Corner_XY].x = box->halfExtents.x;
	corners[dsBox2Corner_XY].y = box->halfExtents.y;

	for (unsigned int i = 0; i < DS_BOX2_CORNER_COUNT; ++i)
	{
		dsVector2d worldOffset;
		dsMatrix22_transform(worldOffset, box->orientation, corners[i]);
		dsVector2_add(corners[i], worldOffset, box->center);
	}

	return true;
}

bool dsOrientedBox2f_intersects(const dsOrientedBox2f* box, const dsOrientedBox2f* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);

	if (!dsOrientedBox2_isValid(*box) || !dsOrientedBox2_isValid(*otherBox))
		return false;

	// 2D subset for separating axes as explained by
	// https://www.geometrictools.com/Documentation/DynamicCollisionDetection.pdf
	// See also:
	// https://github.com/davideberly/GeometricTools/blob/master/GTE/Mathematics/IntrOrientedBox2OrientedBox2.h
	dsVector2f centerDiff;
	dsVector2_sub(centerDiff, otherBox->center, box->center);

	float absDotAxes[2][2];
	for (unsigned int i = 0; i < 2; ++i)
	{
		for (unsigned int j = 0; j < 2; ++j)
		{
			absDotAxes[i][j] = fabsf(dsVector2_dot(box->orientation.columns[i],
				otherBox->orientation.columns[j]));
		}

		// Test axes for first box against second box.
		float radius = box->halfExtents.values[i] + otherBox->halfExtents.x*absDotAxes[i][0] +
			otherBox->halfExtents.y*absDotAxes[i][1];
		if (fabsf(dsVector2_dot(box->orientation.columns[i], centerDiff)) > radius)
			return false;
	}

	// Test axes for second box against first box.
	for (unsigned int i = 0; i < 2; ++i)
	{
		float radius = box->halfExtents.x*absDotAxes[0][i] + box->halfExtents.y*absDotAxes[1][i] +
			otherBox->halfExtents.values[i];
		if (fabsf(dsVector2_dot(otherBox->orientation.columns[i], centerDiff)) > radius)
			return false;
	}

	return true;
}

bool dsOrientedBox2d_intersects(const dsOrientedBox2d* box, const dsOrientedBox2d* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);

	if (!dsOrientedBox2_isValid(*box) || !dsOrientedBox2_isValid(*otherBox))
		return false;

	// 2D subset for separating axes as explained by
	// https://www.geometrictools.com/Documentation/DynamicCollisionDetection.pdf
	// See also:
	// https://github.com/davideberly/GeometricTools/blob/master/GTE/Mathematics/IntrOrientedBox2OrientedBox2.h
	dsVector2d centerDiff;
	dsVector2_sub(centerDiff, otherBox->center, box->center);

	double absDotAxes[2][2];
	for (unsigned int i = 0; i < 2; ++i)
	{
		for (unsigned int j = 0; j < 2; ++j)
		{
			absDotAxes[i][j] = fabs(dsVector2_dot(box->orientation.columns[i],
				otherBox->orientation.columns[j]));
		}

		// Test axes for first box against second box.
		double radius = box->halfExtents.values[i] + otherBox->halfExtents.x*absDotAxes[i][0] +
			otherBox->halfExtents.y*absDotAxes[i][1];
		if (fabs(dsVector2_dot(box->orientation.columns[i], centerDiff)) > radius)
			return false;
	}

	// Test axes for second box against first box.
	for (unsigned int i = 0; i < 2; ++i)
	{
		double radius = box->halfExtents.x*absDotAxes[0][i] + box->halfExtents.y*absDotAxes[1][i] +
			otherBox->halfExtents.values[i];
		if (fabs(dsVector2_dot(otherBox->orientation.columns[i], centerDiff)) > radius)
			return false;
	}

	return true;
}

bool dsOrientedBox2f_containsPoint(const dsOrientedBox2f* box, const dsVector2f* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox2_isValid(*box))
		return false;

	dsAlignedBox2f localBox =
	{
		{{-box->halfExtents.x, -box->halfExtents.y}},
		{{box->halfExtents.x, box->halfExtents.y}}
	};

	dsVector2f localPoint;
	dsVector2f centeredPoint;
	dsVector2_sub(centeredPoint, *point, box->center);
	dsMatrix22_transformTransposed(localPoint, box->orientation, centeredPoint);
	return dsAlignedBox2_containsPoint(localBox, localPoint);
}

bool dsOrientedBox2d_containsPoint(const dsOrientedBox2d* box, const dsVector2d* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox2_isValid(*box))
		return false;

	dsAlignedBox2d localBox =
	{
		{{-box->halfExtents.x, -box->halfExtents.y}},
		{{box->halfExtents.x, box->halfExtents.y}}
	};

	dsVector2d localPoint;
	dsVector2d centeredPoint;
	dsVector2_sub(centeredPoint, *point, box->center);
	dsMatrix22_transformTransposed(localPoint, box->orientation, centeredPoint);
	return dsAlignedBox2_containsPoint(localBox, localPoint);
}

bool dsOrientedBox2f_closestPoint(dsVector2f* result, const dsOrientedBox2f* box,
	const dsVector2f* point)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox2_isValid(*box))
		return false;

	dsAlignedBox2f localBox =
	{
		{{-box->halfExtents.x, -box->halfExtents.y}},
		{{box->halfExtents.x, box->halfExtents.y}}
	};

	dsVector2f localPoint;
	dsVector2f centeredPoint;
	dsVector2_sub(centeredPoint, *point, box->center);
	dsMatrix22_transformTransposed(localPoint, box->orientation, centeredPoint);

	dsVector2f localResult;
	dsAlignedBox2_closestPoint(localResult, localBox, localPoint);
	dsMatrix22_transform(*result, box->orientation, localResult);
	dsVector2_add(*result, *result, box->center);

	return true;
}

bool dsOrientedBox2d_closestPoint(dsVector2d* result, const dsOrientedBox2d* box,
	const dsVector2d* point)
{
	DS_ASSERT(result);
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox2_isValid(*box))
		return false;

	dsAlignedBox2d localBox =
	{
		{{-box->halfExtents.x, -box->halfExtents.y}},
		{{box->halfExtents.x, box->halfExtents.y}}
	};

	dsVector2d localPoint;
	dsVector2d centeredPoint;
	dsVector2_sub(centeredPoint, *point, box->center);
	dsMatrix22_transformTransposed(localPoint, box->orientation, centeredPoint);

	dsVector2d localResult;
	dsAlignedBox2_closestPoint(localResult, localBox, localPoint);
	dsMatrix22_transform(*result, box->orientation, localResult);
	dsVector2_add(*result, *result, box->center);

	return true;
}

float dsOrientedBox2f_dist2(const dsOrientedBox2f* box, const dsVector2f* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox2_isValid(*box))
		return -1;

	dsAlignedBox2f localBox =
	{
		{{-box->halfExtents.x, -box->halfExtents.y}},
		{{box->halfExtents.x, box->halfExtents.y}}
	};

	dsVector2f localPoint;
	dsVector2f centeredPoint;
	dsVector2_sub(centeredPoint, *point, box->center);
	dsMatrix22_transformTransposed(localPoint, box->orientation, centeredPoint);

	return dsAlignedBox2f_dist2(&localBox, &localPoint);
}

double dsOrientedBox2d_dist2(const dsOrientedBox2d* box, const dsVector2d* point)
{
	DS_ASSERT(box);
	DS_ASSERT(point);

	if (!dsOrientedBox2_isValid(*box))
		return -1;

	dsAlignedBox2d localBox =
	{
		{{-box->halfExtents.x, -box->halfExtents.y}},
		{{box->halfExtents.x, box->halfExtents.y}}
	};

	dsVector2d localPoint;
	dsVector2d centeredPoint;
	dsVector2_sub(centeredPoint, *point, box->center);
	dsMatrix22_transformTransposed(localPoint, box->orientation, centeredPoint);

	return dsAlignedBox2d_dist2(&localBox, &localPoint);
}

float dsOrientedBox2f_dist(const dsOrientedBox2f* box, const dsVector2f* point)
{
	float distance2 = dsOrientedBox2f_dist2(box, point);
	if (distance2 <= 0)
		return distance2;

	return sqrtf(distance2);
}

double dsOrientedBox2d_dist(const dsOrientedBox2d* box, const dsVector2d* point)
{
	double distance2 = dsOrientedBox2d_dist2(box, point);
	if (distance2 <= 0)
		return distance2;

	return sqrt(distance2);
}

bool dsOrientedBox2f_isValid(const dsOrientedBox2f* box);
bool dsOrientedBox2d_isValid(const dsOrientedBox2d* box);
void dsOrientedBox3f_toMatrix(dsMatrix33f* result, const dsOrientedBox2f* box);
void dsOrientedBox3d_toMatrix(dsMatrix33d* result, const dsOrientedBox2d* box);
void dsOrientedBox2f_fromAlignedBox(dsOrientedBox2f* result, const dsAlignedBox2f* alignedBox);
void dsOrientedBox2d_fromAlignedBox(dsOrientedBox2d* result, const dsAlignedBox2d* alignedBox);
void dsOrientedBox2f_makeInvalid(dsOrientedBox2f* result);
void dsOrientedBox2d_makeInvalid(dsOrientedBox2d* result);

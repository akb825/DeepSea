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

#include <DeepSea/Geometry/OrientedBox2.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Vector2.h>
#include <float.h>

bool dsOrientedBox2f_transform(dsOrientedBox2f* box, const dsMatrix33f* transform)
{
	DS_ASSERT(box);
	DS_ASSERT(transform);

	if (!dsOrientedBox2_isValid(*box))
		return false;

	dsVector3f center = {{box->center.x, box->center.y, 1}};

	dsMatrix22f newOrientation;
	dsVector3f newCenter;

	// Macro will work correctly for treating a 3x3 matrix as a 2x2 matrix.
	dsMatrix22_mul(newOrientation, *transform, box->orientation);

	// Extract scales to apply to the extent.
	float scaleX = dsVector2f_len(&newOrientation.columns[0]);
	float scaleY = dsVector2f_len(&newOrientation.columns[1]);

	dsMatrix33_transform(newCenter, *transform, center);

	float invScaleX = 1/scaleX;
	float invScaleY = 1/scaleY;
	dsVector2_scale(box->orientation.columns[0], newOrientation.columns[0], invScaleX);
	dsVector2_scale(box->orientation.columns[1], newOrientation.columns[1], invScaleY);

	box->center.x = newCenter.x;
	box->center.y = newCenter.y;

	box->halfExtents.x *= scaleX;
	box->halfExtents.y *= scaleY;

	return true;
}

bool dsOrientedBox2d_transform(dsOrientedBox2d* box, const dsMatrix33d* transform)
{
	DS_ASSERT(box);
	DS_ASSERT(transform);

	if (!dsOrientedBox2_isValid(*box))
		return false;

	dsVector3d center = {{box->center.x, box->center.y, 1}};

	dsMatrix22d newOrientation;
	dsVector3d newCenter;

	// Macro will work correctly for treating a 3x3 matrix as a 2x2 matrix.
	dsMatrix22_mul(newOrientation, *transform, box->orientation);

	// Extract scales to apply to the extent.
	double scaleX = dsVector2d_len(&newOrientation.columns[0]);
	double scaleY = dsVector2d_len(&newOrientation.columns[1]);

	dsMatrix33_transform(newCenter, *transform, center);

	double invScaleX = 1/scaleX;
	double invScaleY = 1/scaleY;
	dsVector2_scale(box->orientation.columns[0], newOrientation.columns[0], invScaleX);
	dsVector2_scale(box->orientation.columns[1], newOrientation.columns[1], invScaleY);

	box->center.x = newCenter.x;
	box->center.y = newCenter.y;

	box->halfExtents.x *= scaleX;
	box->halfExtents.y *= scaleY;

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

bool dsOrientedBox2f_corners(dsVector2f corners[], const dsOrientedBox2f* box)
{
	DS_ASSERT(corners);
	DS_ASSERT(box);

	if (!dsOrientedBox2_isValid(*box))
		return false;

	corners[0].x = -box->halfExtents.x;
	corners[0].y = -box->halfExtents.y;

	corners[1].x = -box->halfExtents.x;
	corners[1].y = box->halfExtents.y;

	corners[2].x = box->halfExtents.x;
	corners[2].y = -box->halfExtents.y;

	corners[3].x = box->halfExtents.x;
	corners[3].y = box->halfExtents.y;

	for (unsigned int i = 0; i < DS_BOX2_CORNER_COUNT; ++i)
	{
		dsVector2f worldOffset;
		dsMatrix22_transform(worldOffset, box->orientation, corners[i]);
		dsVector2_add(corners[i], worldOffset, box->center);
	}

	return true;
}

bool dsOrientedBox2d_corners(dsVector2d corners[], const dsOrientedBox2d* box)
{
	DS_ASSERT(corners);
	DS_ASSERT(box);

	if (!dsOrientedBox2_isValid(*box))
		return false;

	corners[0].x = -box->halfExtents.x;
	corners[0].y = -box->halfExtents.y;

	corners[1].x = -box->halfExtents.x;
	corners[1].y = box->halfExtents.y;

	corners[2].x = box->halfExtents.x;
	corners[2].y = -box->halfExtents.y;

	corners[3].x = box->halfExtents.x;
	corners[3].y = box->halfExtents.y;

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

	// Use separating axis theorem.
	// Test axes of box.
	{
		dsVector2f otherBoxCorners[DS_BOX2_CORNER_COUNT];
		dsOrientedBox2f_corners(otherBoxCorners, otherBox);
		for (unsigned int i = 0; i < DS_BOX2_CORNER_COUNT; ++i)
		{
			dsVector2_sub(otherBoxCorners[i], otherBoxCorners[i], box->center);
		}

		for (unsigned int i = 0; i < 2; ++i)
		{
			float boxMin = -box->halfExtents.values[i];
			float boxMax = box->halfExtents.values[i];

			float otherBoxMin = FLT_MAX;
			float otherBoxMax = -FLT_MAX;
			for (unsigned int j = 0; j < DS_BOX2_CORNER_COUNT; ++j)
			{
				float projectedPoint = dsVector2_dot(box->orientation.columns[i],
					otherBoxCorners[j]);
				otherBoxMin = dsMin(otherBoxMin, projectedPoint);
				otherBoxMax = dsMax(otherBoxMax, projectedPoint);
			}

			if (!((boxMin >= otherBoxMin && boxMin <= otherBoxMax) ||
				  (otherBoxMin >= boxMin && otherBoxMin <= boxMax)))
			{
				return false;
			}
		}
	}

	// Test axes of other box.
	{
		dsVector2f boxCorners[DS_BOX2_CORNER_COUNT];
		dsOrientedBox2f_corners(boxCorners, box);
		for (unsigned int i = 0; i < DS_BOX2_CORNER_COUNT; ++i)
		{
			dsVector2_sub(boxCorners[i], boxCorners[i], otherBox->center);
		}

		for (unsigned int i = 0; i < 2; ++i)
		{
			float boxMin = FLT_MAX;
			float boxMax = -FLT_MAX;
			for (unsigned int j = 0; j < DS_BOX2_CORNER_COUNT; ++j)
			{
				float projectedPoint = dsVector2_dot(otherBox->orientation.columns[i],
					boxCorners[j]);
				boxMin = dsMin(boxMin, projectedPoint);
				boxMax = dsMax(boxMax, projectedPoint);
			}

			float otherBoxMin = -otherBox->halfExtents.values[i];
			float otherBoxMax = otherBox->halfExtents.values[i];

			if (!((boxMin >= otherBoxMin && boxMin <= otherBoxMax) ||
				  (otherBoxMin >= boxMin && otherBoxMin <= boxMax)))
			{
				return false;
			}
		}
	}

	return true;
}

bool dsOrientedBox2d_intersects(const dsOrientedBox2d* box, const dsOrientedBox2d* otherBox)
{
	DS_ASSERT(box);
	DS_ASSERT(otherBox);

	if (!dsOrientedBox2_isValid(*box) || !dsOrientedBox2_isValid(*otherBox))
		return false;

	// Use separating axis theorem.
	// Test axes of box.
	{
		dsVector2d otherBoxCorners[DS_BOX2_CORNER_COUNT];
		dsOrientedBox2d_corners(otherBoxCorners, otherBox);
		for (unsigned int i = 0; i < DS_BOX2_CORNER_COUNT; ++i)
		{
			dsVector2_sub(otherBoxCorners[i], otherBoxCorners[i], box->center);
		}

		for (unsigned int i = 0; i < 2; ++i)
		{
			double boxMin = -box->halfExtents.values[i];
			double boxMax = box->halfExtents.values[i];

			double otherBoxMin = DBL_MAX;
			double otherBoxMax = -DBL_MAX;
			for (unsigned int j = 0; j < DS_BOX2_CORNER_COUNT; ++j)
			{
				double projectedPoint = dsVector2_dot(box->orientation.columns[i],
					otherBoxCorners[j]);
				otherBoxMin = dsMin(otherBoxMin, projectedPoint);
				otherBoxMax = dsMax(otherBoxMax, projectedPoint);
			}

			if (!((boxMin >= otherBoxMin && boxMin <= otherBoxMax) ||
				  (otherBoxMin >= boxMin && otherBoxMin <= boxMax)))
			{
				return false;
			}
		}
	}

	// Test axes of other box.
	{
		dsVector2d boxCorners[DS_BOX2_CORNER_COUNT];
		dsOrientedBox2d_corners(boxCorners, box);
		for (unsigned int i = 0; i < DS_BOX2_CORNER_COUNT; ++i)
		{
			dsVector2_sub(boxCorners[i], boxCorners[i], otherBox->center);
		}

		for (unsigned int i = 0; i < 2; ++i)
		{
			double boxMin = DBL_MAX;
			double boxMax = -DBL_MAX;
			for (unsigned int j = 0; j < DS_BOX2_CORNER_COUNT; ++j)
			{
				double projectedPoint = dsVector2_dot(otherBox->orientation.columns[i],
					boxCorners[j]);
				boxMin = dsMin(boxMin, projectedPoint);
				boxMax = dsMax(boxMax, projectedPoint);
			}

			double otherBoxMin = -otherBox->halfExtents.values[i];
			double otherBoxMax = otherBox->halfExtents.values[i];

			if (!((boxMin >= otherBoxMin && boxMin <= otherBoxMax) ||
				  (otherBoxMin >= boxMin && otherBoxMin <= boxMax)))
			{
				return false;
			}
		}
	}

	return true;
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

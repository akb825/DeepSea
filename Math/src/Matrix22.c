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

#include <DeepSea/Math/Matrix22.h>
#include <DeepSea/Math/Core.h>

void dsMatrix22f_invert(dsMatrix22f* result, const dsMatrix22f* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	float det = dsMatrix22_determinant(*a);
	DS_ASSERT(det != 0);
	float invDet = 1/det;

	result->values[0][0] = a->values[1][1]*invDet;
	result->values[0][1] = -a->values[0][1]*invDet;

	result->values[1][0] = -a->values[1][0]*invDet;
	result->values[1][1] = a->values[0][0]*invDet;
}

void dsMatrix22d_invert(dsMatrix22d* result, const dsMatrix22d* a)
{
	DS_ASSERT(result);
	DS_ASSERT(a);
	DS_ASSERT(result != a);

	double det = dsMatrix22_determinant(*a);
	DS_ASSERT(det != 0);
	double invDet = 1/det;

	result->values[0][0] = a->values[1][1]*invDet;
	result->values[0][1] = -a->values[0][1]*invDet;

	result->values[1][0] = -a->values[1][0]*invDet;
	result->values[1][1] = a->values[0][0]*invDet;
}

void dsMatrix22f_makeRotate(dsMatrix22f* result, float angle)
{
	DS_ASSERT(result);
	float cosAngle = cosf(angle);
	float sinAngle = sinf(angle);

	result->values[0][0] = cosAngle;
	result->values[0][1] = sinAngle;

	result->values[1][0] = -sinAngle;
	result->values[1][1] = cosAngle;
}

void dsMatrix22d_makeRotate(dsMatrix22d* result, double angle)
{
	DS_ASSERT(result);
	double cosAngle = cos(angle);
	double sinAngle = sin(angle);

	result->values[0][0] = cosAngle;
	result->values[0][1] = sinAngle;

	result->values[1][0] = -sinAngle;
	result->values[1][1] = cosAngle;
}

void dsMatrix22f_makeScale(dsMatrix22f* result, float x, float y)
{
	DS_ASSERT(result);
	result->values[0][0] = x;
	result->values[0][1] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = y;
}

void dsMatrix22d_makeScale(dsMatrix22d* result, double x, double y)
{
	DS_ASSERT(result);
	result->values[0][0] = x;
	result->values[0][1] = 0;

	result->values[1][0] = 0;
	result->values[1][1] = y;
}

/*
 * Copyright 2021 Aaron Barany
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

#include "Fixtures/FixtureBase.h"

#include <DeepSea/Geometry/Frustum3.h>
#include <DeepSea/Render/Shadows/ShadowCullVolume.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Vector3.h>

#include <gtest/gtest.h>

class ShadowVolumeCullTest : public FixtureBase
{
};

static const uint32_t notFound = (uint32_t)-1;

static uint32_t findPlane(const dsShadowCullVolume* volume, const dsPlane3f* plane)
{
	float epsilon = 1e-4f;
	for (uint32_t i = 0; i < volume->planeCount; ++i)
	{
		const dsPlane3f* curPlane = volume->planes + i;
		if (dsEpsilonEqualf(curPlane->n.x, plane->n.x, epsilon) &&
			dsEpsilonEqualf(curPlane->n.y, plane->n.y, epsilon) &&
			dsEpsilonEqualf(curPlane->n.z, plane->n.z, epsilon) &&
			dsEpsilonEqualf(curPlane->d, plane->d, epsilon))
		{
			return i;
		}
	}

	return notFound;
}

static bool hasCorner(const dsShadowCullVolume* volume, float x, float y, float z, uint32_t p0,
	uint32_t p1, uint32_t p2)
{
	uint32_t minFirstP = dsMin(p0, p1);
	uint32_t maxFirstP = dsMax(p0, p1);
	uint32_t minP = dsMin(minFirstP, p2);
	uint32_t middleP = p2 < maxFirstP ? dsMax(minFirstP, p2) : dsMin(maxFirstP, p2);
	uint32_t maxP = dsMax(maxFirstP, p2);

	const float epsilon = 1e-2f;
	for (uint32_t i = 0; i < volume->cornerCount; ++i)
	{
		const dsShadowCullCorner* corner = volume->corners + i;
		if (corner->planes[0] != minP || corner->planes[1] != middleP || corner->planes[2] != maxP)
			continue;

		if (dsEpsilonEqualf(corner->point.x, x, epsilon) &&
			dsEpsilonEqualf(corner->point.y, y, epsilon) &&
			dsEpsilonEqualf(corner->point.z, z, epsilon))
		{
			return true;
		}
	}

	return false;
}

static uint32_t countCorners(const dsShadowCullVolume* volume, float x, float y, float z)
{
	const float epsilon = 1e-2f;
	uint32_t count = 0;
	for (uint32_t i = 0; i < volume->cornerCount; ++i)
	{
		const dsShadowCullCorner* corner = volume->corners + i;
		if (dsEpsilonEqualf(corner->point.x, x, epsilon) &&
			dsEpsilonEqualf(corner->point.y, y, epsilon) &&
			dsEpsilonEqualf(corner->point.z, z, epsilon))
		{
			++count;
		}
	}

	return count;
}

TEST_F(ShadowVolumeCullTest, DirectionalPerpendicular)
{
	dsMatrix44f projection;
	dsRenderer_makeFrustum(&projection, renderer, -2, 4, -3, 5, 1, 100);
	dsFrustum3f frustum;
	dsRenderer_frustumFromMatrix(&frustum, renderer, &projection);

	dsVector3f lightDir = {{0.0f, 1.0f, 0.0f}};
	dsShadowCullVolume volume;
	dsShadowCullVolume_buildDirectional(&volume, &frustum, &lightDir);

	ASSERT_EQ(5U, volume.planeCount);
	uint32_t left = findPlane(&volume, frustum.planes + dsFrustumPlanes_Left);
	uint32_t right = findPlane(&volume, frustum.planes + dsFrustumPlanes_Right);
	uint32_t bottom = findPlane(&volume, frustum.planes + dsFrustumPlanes_Bottom);
	uint32_t top = findPlane(&volume, frustum.planes + dsFrustumPlanes_Top);
	uint32_t near = findPlane(&volume, frustum.planes + dsFrustumPlanes_Near);
	uint32_t far = findPlane(&volume, frustum.planes + dsFrustumPlanes_Far);
	EXPECT_NE(notFound, left);
	EXPECT_NE(notFound, right);
	EXPECT_NE(notFound, bottom);
	EXPECT_EQ(notFound, top);
	EXPECT_NE(notFound, near);
	EXPECT_NE(notFound, far);

	ASSERT_EQ(8U, volume.edgeCount);
	ASSERT_EQ(4U, volume.cornerCount);

	EXPECT_TRUE(hasCorner(&volume, -2.0f, -3.0f, -1.0f, left, bottom, near));
	EXPECT_TRUE(hasCorner(&volume, 4.0f, -3.0f, -1.0f, right, bottom, near));
	EXPECT_TRUE(hasCorner(&volume, -200.0f, -300.0f, -100.0f, left, bottom, far));
	EXPECT_TRUE(hasCorner(&volume, 400.0f, -300.0f, -100.0f, right, bottom, far));
}

TEST_F(ShadowVolumeCullTest, DirectionalOrthoPerpendicular)
{
	dsMatrix44f projection;
	dsRenderer_makeOrtho(&projection, renderer, -2, 4, -3, 5, 1, 100);
	dsFrustum3f frustum;
	dsRenderer_frustumFromMatrix(&frustum, renderer, &projection);

	dsVector3f lightDir = {{0.0f, 1.0f, 0.0f}};
	dsShadowCullVolume volume;
	dsShadowCullVolume_buildDirectional(&volume, &frustum, &lightDir);

	ASSERT_EQ(5U, volume.planeCount);
	uint32_t left = findPlane(&volume, frustum.planes + dsFrustumPlanes_Left);
	uint32_t right = findPlane(&volume, frustum.planes + dsFrustumPlanes_Right);
	uint32_t bottom = findPlane(&volume, frustum.planes + dsFrustumPlanes_Bottom);
	uint32_t top = findPlane(&volume, frustum.planes + dsFrustumPlanes_Top);
	uint32_t near = findPlane(&volume, frustum.planes + dsFrustumPlanes_Near);
	uint32_t far = findPlane(&volume, frustum.planes + dsFrustumPlanes_Far);
	EXPECT_NE(notFound, left);
	EXPECT_NE(notFound, right);
	EXPECT_NE(notFound, bottom);
	EXPECT_EQ(notFound, top);
	EXPECT_NE(notFound, near);
	EXPECT_NE(notFound, far);

	ASSERT_EQ(8U, volume.edgeCount);
	ASSERT_EQ(4U, volume.cornerCount);

	EXPECT_TRUE(hasCorner(&volume, -2.0f, -3.0f, -1.0f, left, bottom, near));
	EXPECT_TRUE(hasCorner(&volume, 4.0f, -3.0f, -1.0f, right, bottom, near));
	EXPECT_TRUE(hasCorner(&volume, -2.0f, -3.0f, -100.0f, left, bottom, far));
	EXPECT_TRUE(hasCorner(&volume, 4.0f, -3.0f, -100.0f, right, bottom, far));
}

TEST_F(ShadowVolumeCullTest, DirectionalAngled)
{
	dsMatrix44f projection;
	dsRenderer_makeFrustum(&projection, renderer, -0.5f, 0.7f, -0.9f, 1.1f, 1, 100);
	dsFrustum3f frustum;
	dsRenderer_frustumFromMatrix(&frustum, renderer, &projection);

	dsVector3f lightDir = {{-0.75f, 1.0f, 0.5f}};
	dsVector3f_normalize(&lightDir, &lightDir);
	dsShadowCullVolume volume;
	dsShadowCullVolume_buildDirectional(&volume, &frustum, &lightDir);

	ASSERT_EQ(9U, volume.planeCount);
	uint32_t left = findPlane(&volume, frustum.planes + dsFrustumPlanes_Left);
	uint32_t right = findPlane(&volume, frustum.planes + dsFrustumPlanes_Right);
	uint32_t bottom = findPlane(&volume, frustum.planes + dsFrustumPlanes_Bottom);
	uint32_t top = findPlane(&volume, frustum.planes + dsFrustumPlanes_Top);
	uint32_t near = findPlane(&volume, frustum.planes + dsFrustumPlanes_Near);
	uint32_t far = findPlane(&volume, frustum.planes + dsFrustumPlanes_Far);
	EXPECT_EQ(notFound, left);
	EXPECT_NE(notFound, right);
	EXPECT_NE(notFound, bottom);
	EXPECT_EQ(notFound, top);
	EXPECT_EQ(notFound, near);
	EXPECT_NE(notFound, far);

	ASSERT_EQ(15U, volume.edgeCount);

	uint32_t cornerCount = 0;
	uint32_t curCornerCount = countCorners(&volume, -0.5f, -0.9f, -1.0f);
	EXPECT_LT(0U, curCornerCount);
	cornerCount += curCornerCount;

	curCornerCount = countCorners(&volume, 0.7f, -0.9f, -1.0f);
	EXPECT_LT(0U, curCornerCount);
	cornerCount += curCornerCount;

	curCornerCount = countCorners(&volume, 0.7f, 1.1f, -1.0f);
	EXPECT_LT(0U, curCornerCount);
	cornerCount += curCornerCount;

	curCornerCount = countCorners(&volume, -50.0f, -90.0f, -100.0f);
	EXPECT_LT(0U, curCornerCount);
	cornerCount += curCornerCount;

	curCornerCount = countCorners(&volume, 70.0f, -90.0f, -100.0f);
	EXPECT_LT(0U, curCornerCount);
	cornerCount += curCornerCount;

	curCornerCount = countCorners(&volume, -50.0f, 110.0f, -100.0f);
	EXPECT_LT(0U, curCornerCount);
	cornerCount += curCornerCount;

	curCornerCount = countCorners(&volume, 70.0f, 110.0f, -100.0f);
	EXPECT_LT(0U, curCornerCount);
	cornerCount += curCornerCount;

	EXPECT_EQ(volume.cornerCount, cornerCount);
}

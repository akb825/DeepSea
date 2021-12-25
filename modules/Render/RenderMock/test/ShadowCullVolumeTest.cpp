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

#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Geometry/Frustum3.h>
#include <DeepSea/Render/Shadows/ShadowCullVolume.h>
#include <DeepSea/Render/Shadows/ShadowProjection.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
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

static bool hasPlane(const dsShadowCullVolume* volume, const dsPlane3f* plane)
{
	return findPlane(volume, plane) != notFound;
}

static bool hasCorner(const dsShadowCullVolume* volume, float x, float y, float z, uint32_t p0,
	uint32_t p1, uint32_t p2)
{
	uint32_t planes = (1 << p0) | (1 << p1) | (1 << p2);
	const float epsilon = 1e-2f;
	for (uint32_t i = 0; i < volume->cornerCount; ++i)
	{
		const dsShadowCullCorner* corner = volume->corners + i;
		if (corner->planes != planes)
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

static uint32_t findCorner(const dsShadowCullVolume* volume, float x, float y, float z)
{
	const float epsilon = 1e-2f;
	for (uint32_t i = 0; i < volume->cornerCount; ++i)
	{
		const dsShadowCullCorner* corner = volume->corners + i;
		if (dsEpsilonEqualf(corner->point.x, x, epsilon) &&
			dsEpsilonEqualf(corner->point.y, y, epsilon) &&
			dsEpsilonEqualf(corner->point.z, z, epsilon))
		{
			return i;
		}
	}

	return notFound;
}

static bool hasCorner(const dsShadowCullVolume* volume, float x, float y, float z)
{
	return findCorner(volume, x, y, z) != notFound;
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
	ASSERT_EQ(7U, volume.cornerCount);

	EXPECT_TRUE(hasCorner(&volume, -0.5f, -0.9f, -1.0f));
	EXPECT_TRUE(hasCorner(&volume, 0.7f, -0.9f, -1.0f));
	EXPECT_TRUE(hasCorner(&volume, 0.7f, 1.1f, -1.0f));
	EXPECT_TRUE(hasCorner(&volume, -50.0f, -90.0f, -100.0f));
	EXPECT_TRUE(hasCorner(&volume, 70.0f, -90.0f, -100.0f));
	EXPECT_TRUE(hasCorner(&volume, -50.0f, 110.0f, -100.0f));
	EXPECT_TRUE(hasCorner(&volume, 70.0f, 110.0f, -100.0f));
}

TEST_F(ShadowVolumeCullTest, SpotNonIntersecting)
{
	dsMatrix44f projection;
	dsRenderer_makeOrtho(&projection, renderer, -2, 4, -3, 5, 1, 100);
	dsFrustum3f frustum;
	dsRenderer_frustumFromMatrix(&frustum, renderer, &projection);

	dsMatrix44f baseLightProjection;
	dsRenderer_makeFrustum(&baseLightProjection, renderer, -1, 1, -1, 1, 1, 100);
	dsMatrix44f translate, rotate, transform;
	dsMatrix44f_makeTranslate(&translate, 0, 0, 5);
	dsMatrix44f_makeRotate(&rotate, 0, -float(M_PI)/2, 0);
	dsMatrix44_mul(transform, translate, rotate);

	dsMatrix44f lightProjection;
	dsMatrix44_mul(lightProjection, baseLightProjection, transform);
	dsFrustum3f lightFrustum;
	dsRenderer_frustumFromMatrix(&lightFrustum, renderer, &lightProjection);

	dsShadowCullVolume volume;
	dsShadowCullVolume_buildSpot(&volume, &frustum, &lightFrustum);

	EXPECT_EQ(0U, volume.planeCount);
	EXPECT_EQ(0U, volume.edgeCount);
	EXPECT_EQ(0U, volume.cornerCount);
}

TEST_F(ShadowVolumeCullTest, SpotIntersecting)
{
	dsMatrix44f projection;
	dsRenderer_makeOrtho(&projection, renderer, -2, 4, -3, 5, 1, 100);
	dsFrustum3f frustum;
	dsRenderer_frustumFromMatrix(&frustum, renderer, &projection);

	dsMatrix44f baseLightProjection;
	dsRenderer_makeFrustum(&baseLightProjection, renderer, -1, 1, -1, 1, 1, 100);
	dsMatrix44f translate, rotate, transform;
	dsMatrix44f_makeTranslate(&translate, 0, 0, 10);
	dsMatrix44f_makeRotate(&rotate, float(M_PI)/4, -float(M_PI)/4, float(M_PI)/4);
	dsMatrix44_mul(transform, rotate, translate);

	dsMatrix44f lightProjection;
	dsMatrix44_mul(lightProjection, baseLightProjection, transform);
	dsFrustum3f lightFrustum;
	dsRenderer_frustumFromMatrix(&lightFrustum, renderer, &lightProjection);

	dsShadowCullVolume volume;
	dsShadowCullVolume_buildSpot(&volume, &frustum, &lightFrustum);

	ASSERT_EQ(6U, volume.planeCount);
	EXPECT_TRUE(hasPlane(&volume, frustum.planes + dsFrustumPlanes_Left));
	EXPECT_TRUE(hasPlane(&volume, frustum.planes + dsFrustumPlanes_Bottom));

	uint32_t lightLeft = findPlane(&volume, lightFrustum.planes + dsFrustumPlanes_Left);
	uint32_t lightRight = findPlane(&volume, lightFrustum.planes + dsFrustumPlanes_Right);
	uint32_t lightBottom = findPlane(&volume, lightFrustum.planes + dsFrustumPlanes_Bottom);
	uint32_t lightTop = findPlane(&volume, lightFrustum.planes + dsFrustumPlanes_Top);
	ASSERT_NE(notFound, lightLeft);
	ASSERT_NE(notFound, lightRight);
	ASSERT_NE(notFound, lightBottom);
	ASSERT_NE(notFound, lightTop);

	EXPECT_EQ(11U, volume.edgeCount);
	EXPECT_EQ(7U, volume.cornerCount);

	// Transform is world to local. Take inverse for local to world, where the last column is the
	// light position in world space.
	dsMatrix44f transformInv;
	dsMatrix44f_affineInvert(&transformInv, &transform);

	uint32_t lightPosCorner = findCorner(&volume, transformInv.values[3][0],
		transformInv.values[3][1], transformInv.values[3][2]);
	ASSERT_LT(lightPosCorner, volume.cornerCount);
	uint32_t lightCornerPlanes = (1 << lightLeft) | (1 << lightRight) | (1 << lightTop) |
		(1 << lightBottom);
	EXPECT_EQ(lightCornerPlanes, volume.corners[lightPosCorner].planes);
}

TEST_F(ShadowVolumeCullTest, IntersectInside)
{
	dsMatrix44f projection;
	dsRenderer_makeOrtho(&projection, renderer, -2, 4, -3, 5, 1, 100);
	dsFrustum3f frustum;
	dsRenderer_frustumFromMatrix(&frustum, renderer, &projection);

	dsVector3f lightDir = {{0.0f, 1.0f, 0.0f}};
	dsShadowCullVolume volume;
	dsShadowCullVolume_buildDirectional(&volume, &frustum, &lightDir);

	dsMatrix44f camera;
	dsMatrix44_identity(camera);

	dsShadowProjection shadowProj;
	DS_VERIFY(dsShadowProjection_initialize(&shadowProj, renderer, &camera, &lightDir, NULL, NULL,
		true));

	dsAlignedBox3f box = {{{-1.0f, -2.0f, -6.0f}}, {{3.0f, 20.0f, -3.0f}}};

	dsShadowProjection expectedShadowProj = shadowProj;
	dsVector3f corners[DS_BOX3_CORNER_COUNT];
	dsAlignedBox3_corners(corners, box);
	dsShadowProjection_addPoints(&expectedShadowProj, corners, DS_BOX3_CORNER_COUNT);

	EXPECT_EQ(dsIntersectResult_Inside,
		dsShadowCullVolume_intersectAlignedBox(&volume, &box, &shadowProj, true));

	EXPECT_TRUE(dsVector3_equal(expectedShadowProj.pointBounds.min, shadowProj.pointBounds.min));
	EXPECT_TRUE(dsVector3_equal(expectedShadowProj.pointBounds.max, shadowProj.pointBounds.max));
}

TEST_F(ShadowVolumeCullTest, IntersectOutside)
{
	dsMatrix44f projection;
	dsRenderer_makeOrtho(&projection, renderer, -2, 4, -3, 5, 1, 100);
	dsFrustum3f frustum;
	dsRenderer_frustumFromMatrix(&frustum, renderer, &projection);

	dsVector3f lightDir = {{0.0f, 1.0f, 0.0f}};
	dsShadowCullVolume volume;
	dsShadowCullVolume_buildDirectional(&volume, &frustum, &lightDir);

	dsMatrix44f camera;
	dsMatrix44_identity(camera);

	dsShadowProjection shadowProj;
	DS_VERIFY(dsShadowProjection_initialize(&shadowProj, renderer, &camera, &lightDir, NULL, NULL,
		true));

	dsAlignedBox3f box = {{{-1.0f, -2.0f, 0.0f}}, {{3.0f, 20.0f, 3.0f}}};
	EXPECT_EQ(dsIntersectResult_Outside,
		dsShadowCullVolume_intersectAlignedBox(&volume, &box, &shadowProj, true));

	EXPECT_FALSE(dsAlignedBox3_isValid(shadowProj.pointBounds));
}

TEST_F(ShadowVolumeCullTest, IntersectClamp)
{
	dsMatrix44f projection;
	dsRenderer_makeOrtho(&projection, renderer, -2, 4, -3, 5, 1, 100);
	dsFrustum3f frustum;
	dsRenderer_frustumFromMatrix(&frustum, renderer, &projection);

	dsVector3f lightDir = {{0.0f, 1.0f, 0.0f}};
	dsShadowCullVolume volume;
	dsShadowCullVolume_buildDirectional(&volume, &frustum, &lightDir);

	dsMatrix44f camera;
	dsMatrix44_identity(camera);

	dsShadowProjection shadowProj;
	DS_VERIFY(dsShadowProjection_initialize(&shadowProj, renderer, &camera, &lightDir, NULL, NULL,
		true));

	dsAlignedBox3f box = {{{-1.0f, -2.0f, -6.0f}}, {{5.0f, 20.0f, 0.0f}}};

	dsVector3f expectedCorners[] =
	{
		{{-1.0f, -2.0f, -6.0f}},
		{{-1.0f, -2.0f, -1.0f}},
		{{-1.0f, 20.0f, -6.0f}},
		{{-1.0f, 20.0f, -1.0f}},
		{{4.0f, -2.0f, -6.0f}},
		{{4.0f, -2.0f, -1.0f}},
		{{4.0f, 20.0f, -6.0f}},
		{{4.0f, 20.0f, -1.0f}}
	};
	dsShadowProjection expectedShadowProj = shadowProj;
	dsShadowProjection_addPoints(&expectedShadowProj, expectedCorners,
		DS_ARRAY_SIZE(expectedCorners));

	EXPECT_EQ(dsIntersectResult_Intersects,
		dsShadowCullVolume_intersectAlignedBox(&volume, &box, &shadowProj, true));

	const float epsilon = 1e-4f;
	EXPECT_TRUE(dsVector3f_epsilonEqual(&expectedShadowProj.pointBounds.min,
		&shadowProj.pointBounds.min, epsilon));
	EXPECT_TRUE(dsVector3f_epsilonEqual(&expectedShadowProj.pointBounds.max,
		&shadowProj.pointBounds.max, epsilon));
}

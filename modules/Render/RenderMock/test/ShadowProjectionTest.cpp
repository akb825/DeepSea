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

#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Render/Shadows/ShadowProjection.h>
#include <DeepSea/Render/Renderer.h>

#include <gtest/gtest.h>

#include <cmath>

class ShadowProjectionTest : public FixtureBase
{
};

TEST_F(ShadowProjectionTest, Initialize)
{
	dsMatrix44f camera;
	dsMatrix44_identity(camera);
	dsVector3f lightDir = {{0.0f, 1.0f, 0.0f}};

	dsShadowProjection shadowProj;
	EXPECT_FALSE(dsShadowProjection_initialize(NULL, renderer, &camera, &lightDir, NULL, NULL,
		false));
	EXPECT_FALSE(dsShadowProjection_initialize(&shadowProj, NULL, &camera, &lightDir, NULL, NULL,
		false));
	EXPECT_FALSE(dsShadowProjection_initialize(&shadowProj, renderer, NULL, &lightDir, NULL, NULL,
		false));
	EXPECT_FALSE(dsShadowProjection_initialize(&shadowProj, renderer, &camera, NULL, NULL, NULL,
		false));
	EXPECT_TRUE(dsShadowProjection_initialize(&shadowProj, renderer, &camera, &lightDir, NULL, NULL,
		false));
}

TEST_F(ShadowProjectionTest, LightSpacePerspective)
{
	dsMatrix44f camera =
	{{
		{0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f, 1.0f, 0.0f, 0.0f},
		{-1.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 2.0f, 3.0f, 0.0f}
	}};

	dsVector3f lightDir = {{0.0f, 1.0f, 0.0f}};
	dsShadowProjection shadowProj;
	ASSERT_TRUE(dsShadowProjection_initialize(&shadowProj, renderer, &camera, &lightDir, NULL, NULL,
		false));

	dsVector3f points[] =
	{
		{{-1.0f, -1.0f, -1.0f}},
		{{1.0f, 1.0f, 1.0f}},
	};
	EXPECT_TRUE(dsShadowProjection_addPoints(&shadowProj, points, DS_ARRAY_SIZE(points)));

	dsMatrix44f projection;
	ASSERT_TRUE(dsShadowProjection_computeMatrix(&projection, &shadowProj, 0.0f, 0.0f));

	dsVector4f testPoints[] =
	{
		{{-1.0f, 1.0f, 0.0f, 1.0f}},
		{{1.0f, -1.0f, 0.0f, 1.0f}}
	};

	dsVector4f projPoint;
	dsMatrix44_transform(projPoint, projection, testPoints[0]);
	float y = projPoint.y/projPoint.w;
	float z = projPoint.z/projPoint.w;
	EXPECT_NEAR(-1.0f, y, 1e-6f);
	EXPECT_NEAR(0.0f, z, 1e-6f);

	dsMatrix44_transform(projPoint, projection, testPoints[1]);
	y = projPoint.y/projPoint.w;
	z = projPoint.z/projPoint.w;
	EXPECT_NEAR(1.0f, y, 1e-6f);
	EXPECT_LT(0.5f, z);
	EXPECT_GT(1.0f, z);
}

TEST_F(ShadowProjectionTest, LightSpacePerspectiveFullRange)
{
	dsMatrix44f camera =
	{{
		{0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f, 1.0f, 0.0f, 0.0f},
		{-1.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 2.0f, 3.0f, 0.0f}
	}};

	renderer->clipHalfDepth = false;

	dsVector3f lightDir = {{0.0f, 1.0f, 0.0f}};
	dsShadowProjection shadowProj;
	ASSERT_TRUE(dsShadowProjection_initialize(&shadowProj, renderer, &camera, &lightDir, NULL, NULL,
		false));

	dsVector3f points[] =
	{
		{{-1.0f, -1.0f, -1.0f}},
		{{1.0f, 1.0f, 1.0f}},
	};
	EXPECT_TRUE(dsShadowProjection_addPoints(&shadowProj, points, DS_ARRAY_SIZE(points)));

	dsMatrix44f projection;
	ASSERT_TRUE(dsShadowProjection_computeMatrix(&projection, &shadowProj, 0.0f, 0.0f));

	dsVector4f testPoints[] =
	{
		{{-1.0f, 1.0f, 0.0f, 1.0f}},
		{{1.0f, -1.0f, 0.0f, 1.0f}}
	};

	dsVector4f projPoint;
	dsMatrix44_transform(projPoint, projection, testPoints[0]);
	float y = projPoint.y/projPoint.w;
	float z = projPoint.z/projPoint.w;
	EXPECT_NEAR(-1.0f, y, 1e-6f);
	EXPECT_NEAR(-1.0f, z, 1e-6f);

	dsMatrix44_transform(projPoint, projection, testPoints[1]);
	y = projPoint.y/projPoint.w;
	z = projPoint.z/projPoint.w;
	EXPECT_NEAR(1.0f, y, 1e-6f);
	EXPECT_LT(0.5f, z);
	EXPECT_GT(1.0f, z);
}

TEST_F(ShadowProjectionTest, LightSpacePerspectiveInvertY)
{
	dsMatrix44f camera =
	{{
		{0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f, 1.0f, 0.0f, 0.0f},
		{-1.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 2.0f, 3.0f, 0.0f}
	}};

	renderer->clipInvertY = true;

	dsVector3f lightDir = {{0.0f, 1.0f, 0.0f}};
	dsShadowProjection shadowProj;
	ASSERT_TRUE(dsShadowProjection_initialize(&shadowProj, renderer, &camera, &lightDir, NULL, NULL,
		false));

	dsVector3f points[] =
	{
		{{-1.0f, -1.0f, -1.0f}},
		{{1.0f, 1.0f, 1.0f}},
	};
	EXPECT_TRUE(dsShadowProjection_addPoints(&shadowProj, points, DS_ARRAY_SIZE(points)));

	dsMatrix44f projection;
	ASSERT_TRUE(dsShadowProjection_computeMatrix(&projection, &shadowProj, 0.0f, 0.0f));

	dsVector4f testPoints[] =
	{
		{{-1.0f, 1.0f, 0.0f, 1.0f}},
		{{1.0f, -1.0f, 0.0f, 1.0f}}
	};

	dsVector4f projPoint;
	dsMatrix44_transform(projPoint, projection, testPoints[0]);
	float y = projPoint.y/projPoint.w;
	float z = projPoint.z/projPoint.w;
	EXPECT_NEAR(1.0f, y, 1e-6f);
	EXPECT_NEAR(0.0f, z, 1e-6f);

	dsMatrix44_transform(projPoint, projection, testPoints[1]);
	y = projPoint.y/projPoint.w;
	z = projPoint.z/projPoint.w;
	EXPECT_NEAR(-1.0f, y, 1e-6f);
	EXPECT_LT(0.5f, z);
	EXPECT_GT(1.0f, z);
}

TEST_F(ShadowProjectionTest, Uniform)
{
	dsMatrix44f camera =
	{{
		{0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f, 1.0f, 0.0f, 0.0f},
		{-1.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 2.0f, 3.0f, 0.0f}
	}};

	dsVector3f lightDir = {{0.0f, 1.0f, 0.0f}};
	dsShadowProjection shadowProj;
	ASSERT_TRUE(dsShadowProjection_initialize(&shadowProj, renderer, &camera, &lightDir, NULL, NULL,
		true));

	dsVector3f points[] =
	{
		{{-1.0f, -1.0f, -1.0f}},
		{{1.0f, 1.0f, 1.0f}},
	};
	EXPECT_TRUE(dsShadowProjection_addPoints(&shadowProj, points, DS_ARRAY_SIZE(points)));

	dsMatrix44f projection;
	ASSERT_TRUE(dsShadowProjection_computeMatrix(&projection, &shadowProj, 0.0f, 0.0f));

	dsVector4f testPoints[] =
	{
		{{-1.0f, 1.0f, 0.0f, 1.0f}},
		{{1.0f, -1.0f, 0.0f, 1.0f}}
	};

	dsVector4f projPoint;
	dsMatrix44_transform(projPoint, projection, testPoints[0]);
	float y = projPoint.y/projPoint.w;
	float z = projPoint.z/projPoint.w;
	EXPECT_NEAR(-1.0f, y, 1e-6f);
	EXPECT_NEAR(0.0f, z, 1e-6f);

	dsMatrix44_transform(projPoint, projection, testPoints[1]);
	y = projPoint.y/projPoint.w;
	z = projPoint.z/projPoint.w;
	EXPECT_NEAR(1.0f, y, 1e-6f);
	EXPECT_NEAR(1.0f, z, 1e-6f);
}

TEST_F(ShadowProjectionTest, UniformFullRange)
{
	dsMatrix44f camera =
	{{
		{0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f, 1.0f, 0.0f, 0.0f},
		{-1.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 2.0f, 3.0f, 0.0f}
	}};

	renderer->clipHalfDepth = false;

	dsVector3f lightDir = {{0.0f, 1.0f, 0.0f}};
	dsShadowProjection shadowProj;
	ASSERT_TRUE(dsShadowProjection_initialize(&shadowProj, renderer, &camera, &lightDir, NULL, NULL,
		true));

	dsVector3f points[] =
	{
		{{-1.0f, -1.0f, -1.0f}},
		{{1.0f, 1.0f, 1.0f}},
	};
	EXPECT_TRUE(dsShadowProjection_addPoints(&shadowProj, points, DS_ARRAY_SIZE(points)));

	dsMatrix44f projection;
	ASSERT_TRUE(dsShadowProjection_computeMatrix(&projection, &shadowProj, 0.0f, 0.0f));

	dsVector4f testPoints[] =
	{
		{{-1.0f, 1.0f, 0.0f, 1.0f}},
		{{1.0f, -1.0f, 0.0f, 1.0f}}
	};

	dsVector4f projPoint;
	dsMatrix44_transform(projPoint, projection, testPoints[0]);
	float y = projPoint.y/projPoint.w;
	float z = projPoint.z/projPoint.w;
	EXPECT_NEAR(-1.0f, y, 1e-6f);
	EXPECT_NEAR(-1.0f, z, 1e-6f);

	dsMatrix44_transform(projPoint, projection, testPoints[1]);
	y = projPoint.y/projPoint.w;
	z = projPoint.z/projPoint.w;
	EXPECT_NEAR(1.0f, y, 1e-6f);
	EXPECT_NEAR(1.0f, z, 1e-6f);
}

TEST_F(ShadowProjectionTest, UniformInvertY)
{
	dsMatrix44f camera =
	{{
		{0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f, 1.0f, 0.0f, 0.0f},
		{-1.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 2.0f, 3.0f, 0.0f}
	}};

	renderer->clipInvertY = true;

	dsVector3f lightDir = {{0.0f, 1.0f, 0.0f}};
	dsShadowProjection shadowProj;
	ASSERT_TRUE(dsShadowProjection_initialize(&shadowProj, renderer, &camera, &lightDir, NULL, NULL,
		true));

	dsVector3f points[] =
	{
		{{-1.0f, -1.0f, -1.0f}},
		{{1.0f, 1.0f, 1.0f}},
	};
	EXPECT_TRUE(dsShadowProjection_addPoints(&shadowProj, points, DS_ARRAY_SIZE(points)));

	dsMatrix44f projection;
	ASSERT_TRUE(dsShadowProjection_computeMatrix(&projection, &shadowProj, 0.0f, 0.0f));

	dsVector4f testPoints[] =
	{
		{{-1.0f, 1.0f, 0.0f, 1.0f}},
		{{1.0f, -1.0f, 0.0f, 1.0f}}
	};

	dsVector4f projPoint;
	dsMatrix44_transform(projPoint, projection, testPoints[0]);
	float y = projPoint.y/projPoint.w;
	float z = projPoint.z/projPoint.w;
	EXPECT_NEAR(1.0f, y, 1e-6f);
	EXPECT_NEAR(0.0f, z, 1e-6f);

	dsMatrix44_transform(projPoint, projection, testPoints[1]);
	y = projPoint.y/projPoint.w;
	z = projPoint.z/projPoint.w;
	EXPECT_NEAR(-1.0f, y, 1e-6f);
	EXPECT_NEAR(1.0f, z, 1e-6f);
}

TEST_F(ShadowProjectionTest, LookIntoLight)
{
	dsMatrix44f camera =
	{{
		{0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f, 1.0f, 0.0f, 0.0f},
		{-1.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 2.0f, 3.0f, 0.0f}
	}};

	dsVector3f lightDir = {{1.0f, 0.0f, 0.0f}};
	dsShadowProjection shadowProj;
	ASSERT_TRUE(dsShadowProjection_initialize(&shadowProj, renderer, &camera, &lightDir, NULL, NULL,
		false));

	dsVector3f points[] =
	{
		{{-1.0f, -1.0f, -1.0f}},
		{{1.0f, 1.0f, 1.0f}},
	};
	EXPECT_TRUE(dsShadowProjection_addPoints(&shadowProj, points, DS_ARRAY_SIZE(points)));

	dsMatrix44f projection;
	ASSERT_TRUE(dsShadowProjection_computeMatrix(&projection, &shadowProj, 0.0f, 0.0f));

	dsVector4f testPoints[] =
	{
		{{-1.0f, 1.0f, 0.0f, 1.0f}},
		{{1.0f, -1.0f, 0.0f, 1.0f}}
	};

	dsVector4f projPoint;
	dsMatrix44_transform(projPoint, projection, testPoints[0]);
	float y = projPoint.y/projPoint.w;
	float z = projPoint.z/projPoint.w;
	EXPECT_NEAR(1.0f, y, 1e-6f);
	EXPECT_NEAR(1.0f, z, 1e-6f);

	dsMatrix44_transform(projPoint, projection, testPoints[1]);
	y = projPoint.y/projPoint.w;
	z = projPoint.z/projPoint.w;
	EXPECT_NEAR(-1.0f, y, 1e-6f);
	EXPECT_NEAR(0.0f, z, 1e-6f);
}

TEST_F(ShadowProjectionTest, LookAwayFromLight)
{
	dsMatrix44f camera =
	{{
		{0.0f, 0.0f, 1.0f, 0.0f},
		{0.0f, 1.0f, 0.0f, 0.0f},
		{-1.0f, 0.0f, 0.0f, 0.0f},
		{1.0f, 2.0f, 3.0f, 0.0f}
	}};

	dsVector3f lightDir = {{-1.0f, 0.0f, 0.0f}};
	dsShadowProjection shadowProj;
	ASSERT_TRUE(dsShadowProjection_initialize(&shadowProj, renderer, &camera, &lightDir, NULL, NULL,
		false));

	dsVector3f points[] =
	{
		{{-1.0f, -1.0f, -1.0f}},
		{{1.0f, 1.0f, 1.0f}},
	};
	EXPECT_TRUE(dsShadowProjection_addPoints(&shadowProj, points, DS_ARRAY_SIZE(points)));

	dsMatrix44f projection;
	ASSERT_TRUE(dsShadowProjection_computeMatrix(&projection, &shadowProj, 0.0f, 0.0f));

	dsVector4f testPoints[] =
	{
		{{-1.0f, 1.0f, 0.0f, 1.0f}},
		{{1.0f, -1.0f, 0.0f, 1.0f}}
	};

	dsVector4f projPoint;
	dsMatrix44_transform(projPoint, projection, testPoints[0]);
	float y = projPoint.y/projPoint.w;
	float z = projPoint.z/projPoint.w;
	EXPECT_NEAR(1.0f, y, 1e-6f);
	EXPECT_NEAR(0.0f, z, 1e-6f);

	dsMatrix44_transform(projPoint, projection, testPoints[1]);
	y = projPoint.y/projPoint.w;
	z = projPoint.z/projPoint.w;
	EXPECT_NEAR(-1.0f, y, 1e-6f);
	EXPECT_NEAR(1.0f, z, 1e-6f);
}

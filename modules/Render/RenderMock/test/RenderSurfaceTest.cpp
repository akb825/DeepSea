/*
 * Copyright 2017-2025 Aaron Barany
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
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix22.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Render/RenderSurface.h>
#include <gtest/gtest.h>

class RenderSurfaceTest : public FixtureBase
{
};

TEST_F(RenderSurfaceTest, Roation22)
{
	dsMatrix22f rotation, expected;
	EXPECT_FALSE(dsRenderSurface_makeRotationMatrix22(NULL, dsRenderSurfaceRotation_0));

	EXPECT_TRUE(dsRenderSurface_makeRotationMatrix22(&rotation, dsRenderSurfaceRotation_0));
	dsMatrix22_identity(expected);
	for (unsigned int i = 0; i < 2; ++i)
	{
		for (unsigned int j = 0; j < 2; ++j)
			EXPECT_NEAR(expected.values[i][j], rotation.values[i][j], 1e-6);
	}

	EXPECT_TRUE(dsRenderSurface_makeRotationMatrix22(&rotation, dsRenderSurfaceRotation_90));
	dsMatrix22f_makeRotate(&expected, M_PI_2f);
	for (unsigned int i = 0; i < 2; ++i)
	{
		for (unsigned int j = 0; j < 2; ++j)
			EXPECT_NEAR(expected.values[i][j], rotation.values[i][j], 1e-6);
	}

	EXPECT_TRUE(dsRenderSurface_makeRotationMatrix22(&rotation, dsRenderSurfaceRotation_180));
	dsMatrix22f_makeRotate(&expected, M_PIf);
	for (unsigned int i = 0; i < 2; ++i)
	{
		for (unsigned int j = 0; j < 2; ++j)
			EXPECT_NEAR(expected.values[i][j], rotation.values[i][j], 1e-6);
	}

	EXPECT_TRUE(dsRenderSurface_makeRotationMatrix22(&rotation, dsRenderSurfaceRotation_270));
	dsMatrix22f_makeRotate(&expected, M_PIf*1.5f);
	for (unsigned int i = 0; i < 2; ++i)
	{
		for (unsigned int j = 0; j < 2; ++j)
			EXPECT_NEAR(expected.values[i][j], rotation.values[i][j], 1e-6);
	}
}

TEST_F(RenderSurfaceTest, Roation44)
{
	dsMatrix44f rotation, expected;
	EXPECT_FALSE(dsRenderSurface_makeRotationMatrix44(NULL, dsRenderSurfaceRotation_0));

	EXPECT_TRUE(dsRenderSurface_makeRotationMatrix44(&rotation, dsRenderSurfaceRotation_0));
	dsMatrix44_identity(expected);
	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
			EXPECT_NEAR(expected.values[i][j], rotation.values[i][j], 1e-6);
	}

	EXPECT_TRUE(dsRenderSurface_makeRotationMatrix44(&rotation, dsRenderSurfaceRotation_90));
	dsMatrix44f_makeRotate(&expected, 0.0f, 0.0f, M_PI_2f);
	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
			EXPECT_NEAR(expected.values[i][j], rotation.values[i][j], 1e-6);
	}

	EXPECT_TRUE(dsRenderSurface_makeRotationMatrix44(&rotation, dsRenderSurfaceRotation_180));
	dsMatrix44f_makeRotate(&expected, 0.0f, 0.0f, M_PIf);
	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
			EXPECT_NEAR(expected.values[i][j], rotation.values[i][j], 1e-6);
	}

	EXPECT_TRUE(dsRenderSurface_makeRotationMatrix44(&rotation, dsRenderSurfaceRotation_270));
	dsMatrix44f_makeRotate(&expected, 0.0f, 0.0f, M_PIf*1.5f);
	for (unsigned int i = 0; i < 4; ++i)
	{
		for (unsigned int j = 0; j < 4; ++j)
			EXPECT_NEAR(expected.values[i][j], rotation.values[i][j], 1e-6);
	}
}

TEST_F(RenderSurfaceTest, Create)
{
	EXPECT_FALSE(dsRenderSurface_create(NULL, NULL, NULL, NULL, NULL, dsRenderSurfaceType_Direct,
		dsRenderSurfaceUsage_Standard, 0, 0));

	dsRenderSurface* renderSurface = dsRenderSurface_create(renderer, NULL, "test", NULL, NULL,
		dsRenderSurfaceType_Direct, dsRenderSurfaceUsage_Standard, 0, 0);
	ASSERT_TRUE(renderSurface);

	EXPECT_TRUE(dsRenderSurface_destroy(renderSurface));
}

TEST_F(RenderSurfaceTest, Update)
{
	dsRenderSurface* renderSurface = dsRenderSurface_create(renderer, NULL, "test", NULL, NULL,
		dsRenderSurfaceType_Direct, dsRenderSurfaceUsage_Standard, 1920, 1080);
	ASSERT_TRUE(renderSurface);

	EXPECT_FALSE(dsRenderSurface_update(NULL, 0, 0));

	EXPECT_EQ(1920U, renderSurface->width);
	EXPECT_EQ(1080U, renderSurface->height);

	EXPECT_TRUE(dsRenderSurface_update(renderSurface, 1910, 1070));

	EXPECT_EQ(1910U, renderSurface->width);
	EXPECT_EQ(1070U, renderSurface->height);

	EXPECT_TRUE(dsRenderSurface_destroy(renderSurface));
}

TEST_F(RenderSurfaceTest, BeginEnd)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsRenderSurface* renderSurface = dsRenderSurface_create(renderer, NULL, "test", NULL, NULL,
		dsRenderSurfaceType_Direct, dsRenderSurfaceUsage_Standard, 0, 0);
	ASSERT_TRUE(renderSurface);

	EXPECT_FALSE(dsRenderSurface_beginDraw(renderSurface, NULL));
	EXPECT_FALSE(dsRenderSurface_beginDraw(NULL, commandBuffer));
	EXPECT_TRUE(dsRenderSurface_beginDraw(renderSurface, commandBuffer));
	EXPECT_FALSE(dsRenderSurface_beginDraw(renderSurface, commandBuffer));

	EXPECT_FALSE(dsRenderSurface_endDraw(renderSurface, NULL));
	EXPECT_FALSE(dsRenderSurface_endDraw(NULL, commandBuffer));
	EXPECT_TRUE(dsRenderSurface_endDraw(renderSurface, commandBuffer));
	EXPECT_FALSE(dsRenderSurface_endDraw(renderSurface, commandBuffer));

	EXPECT_TRUE(dsRenderSurface_destroy(renderSurface));
}

TEST_F(RenderSurfaceTest, SwapBuffers)
{
	dsRenderSurface* renderSurface = dsRenderSurface_create(renderer, NULL, "test", NULL, NULL,
		dsRenderSurfaceType_Direct, dsRenderSurfaceUsage_Standard, 0, 0);
	ASSERT_TRUE(renderSurface);

	EXPECT_TRUE(dsRenderSurface_swapBuffers(NULL, 0));
	dsRenderSurface* nullSurface = NULL;
	EXPECT_FALSE(dsRenderSurface_swapBuffers(&nullSurface, 1));
	EXPECT_TRUE(dsRenderSurface_swapBuffers(&renderSurface, 1));

	EXPECT_TRUE(dsRenderSurface_destroy(renderSurface));
}

/*
 * Copyright 2017 Aaron Barany
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
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Framebuffer.h>
#include <DeepSea/Render/Resources/Renderbuffer.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/RenderSurface.h>
#include <gtest/gtest.h>

class FramebufferTest : public FixtureBase
{
};

TEST_F(FramebufferTest, Create)
{
	dsTextureInfo info = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
		dsGfxFormat_UNorm), dsTextureDim_2D, 1920, 1080, 0, 1, 4};
	dsOffscreen* offscreen = dsTexture_createOffscreen(resourceManager, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Static, &info, true);
	ASSERT_TRUE(offscreen);

	info.samples = 1;
	dsTexture* texture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, &info, NULL, 0);
	ASSERT_TRUE(texture);

	dsRenderbuffer* depthBuffer = dsRenderbuffer_create(resourceManager, NULL,
		dsRenderbufferUsage_Standard, dsGfxFormat_D24S8, 1920, 1080, 4);
	ASSERT_TRUE(depthBuffer);

	dsRenderSurface* renderSurface = dsRenderSurface_create(renderer, NULL, "test", NULL,
		dsRenderSurfaceType_Direct);
	ASSERT_TRUE(renderSurface);

	dsFramebufferSurface surfaces[] =
	{
		{dsGfxSurfaceType_Texture, dsCubeFace_None, 0, 0, offscreen},
		{dsGfxSurfaceType_Renderbuffer, dsCubeFace_None, 0, 0, depthBuffer},
		{dsGfxSurfaceType_ColorRenderSurface, dsCubeFace_None, 0, 0, renderSurface},
		{dsGfxSurfaceType_DepthRenderSurface, dsCubeFace_None, 0, 0, renderSurface},
		{dsGfxSurfaceType_Texture, dsCubeFace_None, 0, 0, texture}
	};

	EXPECT_FALSE(dsFramebuffer_create(resourceManager, NULL, "test", surfaces,
		DS_ARRAY_SIZE(surfaces), 1280, 720, 1));

	EXPECT_FALSE(dsFramebuffer_create(resourceManager, NULL, "test", surfaces,
		DS_ARRAY_SIZE(surfaces), 1920, 1080, 1));

	dsFramebuffer* framebuffer = dsFramebuffer_create(resourceManager, NULL, "test", surfaces,
		DS_ARRAY_SIZE(surfaces) - 1, 1920, 1080, 1);
	ASSERT_TRUE(framebuffer);

	EXPECT_TRUE(dsFramebuffer_destroy(framebuffer));
	EXPECT_TRUE(dsTexture_destroy(offscreen));
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_TRUE(dsRenderbuffer_destroy(depthBuffer));
	EXPECT_TRUE(dsRenderSurface_destroy(renderSurface));
}

TEST_F(FramebufferTest, CreateLayers)
{
	dsTextureInfo info = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
		dsGfxFormat_UNorm), dsTextureDim_2D, 1920, 1080, 4, 1, 4};
	dsOffscreen* offscreen1 = dsTexture_createOffscreen(resourceManager, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Static, &info, false);
	ASSERT_TRUE(offscreen1);

	info.format = dsGfxFormat_D24S8;
	dsOffscreen* offscreen2 = dsTexture_createOffscreen(resourceManager, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Static, &info, false);
	ASSERT_TRUE(offscreen2);

	dsFramebufferSurface surfaces[] =
	{
		{dsGfxSurfaceType_Texture, dsCubeFace_None, 15, 0, offscreen1},
		{dsGfxSurfaceType_Texture, dsCubeFace_None, 0, 0, offscreen2}
	};

	EXPECT_FALSE(dsFramebuffer_create(resourceManager, NULL, "test", surfaces,
		DS_ARRAY_SIZE(surfaces), 1920, 1080, 2));

	dsFramebuffer* framebuffer = dsFramebuffer_create(resourceManager, NULL, "test", surfaces,
		DS_ARRAY_SIZE(surfaces), 1920, 1080, 4);
	ASSERT_TRUE(framebuffer);

	EXPECT_TRUE(dsFramebuffer_destroy(framebuffer));
	EXPECT_TRUE(dsTexture_destroy(offscreen1));
	EXPECT_TRUE(dsTexture_destroy(offscreen2));
}

TEST_F(FramebufferTest, CreateMipmaps)
{
	dsTextureInfo colorInfo = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
		dsGfxFormat_UNorm), dsTextureDim_2D, 1920, 1080, 0, 2, 1};
	dsOffscreen* offscreen1 = dsTexture_createOffscreen(resourceManager, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Static, &colorInfo, false);
	ASSERT_TRUE(offscreen1);

	dsTextureInfo depthInfo = {dsGfxFormat_D24S8, dsTextureDim_2D, 960, 540, 0, 1, 1};
	dsOffscreen* offscreen2 = dsTexture_createOffscreen(resourceManager, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Static, &depthInfo, false);
	ASSERT_TRUE(offscreen2);

	dsFramebufferSurface surfaces[] =
	{
		{dsGfxSurfaceType_Texture, dsCubeFace_None, 0, 2, offscreen1},
		{dsGfxSurfaceType_Texture, dsCubeFace_None, 0, 0, offscreen2}
	};

	EXPECT_FALSE(dsFramebuffer_create(resourceManager, NULL, "test", surfaces,
		DS_ARRAY_SIZE(surfaces), 960, 540, 1));

	surfaces[0].mipLevel = 1;
	dsFramebuffer* framebuffer = dsFramebuffer_create(resourceManager, NULL, "test", surfaces,
		DS_ARRAY_SIZE(surfaces), 960, 540, 1);
	ASSERT_TRUE(framebuffer);

	EXPECT_TRUE(dsFramebuffer_destroy(framebuffer));
	EXPECT_TRUE(dsTexture_destroy(offscreen1));
	EXPECT_TRUE(dsTexture_destroy(offscreen2));
}

TEST_F(FramebufferTest, NoColorSurface)
{
	dsRenderbuffer* depthBuffer = dsRenderbuffer_create(resourceManager, NULL,
		dsRenderbufferUsage_Standard, dsGfxFormat_D24S8, 1920, 1080, 4);
	ASSERT_TRUE(depthBuffer);

	dsFramebufferSurface surfaces[] =
	{
		{dsGfxSurfaceType_Renderbuffer, dsCubeFace_None, 0, 0, depthBuffer}
	};

	resourceManager->requiresColorBuffer = true;
	EXPECT_FALSE(dsFramebuffer_create(resourceManager, NULL, "test", surfaces,
		DS_ARRAY_SIZE(surfaces), 1920, 1080, 1));
	resourceManager->requiresColorBuffer = false;

	dsFramebuffer* framebuffer = dsFramebuffer_create(resourceManager, NULL, "test", surfaces,
		DS_ARRAY_SIZE(surfaces), 1920, 1080, 1);
	ASSERT_TRUE(framebuffer);

	EXPECT_TRUE(dsFramebuffer_destroy(framebuffer));
	EXPECT_TRUE(dsRenderbuffer_destroy(depthBuffer));
}

TEST_F(FramebufferTest, Stereoscopic)
{
	dsRenderSurface* renderSurface = dsRenderSurface_create(renderer, NULL, "test", NULL,
		dsRenderSurfaceType_Direct);
	ASSERT_TRUE(renderSurface);

	dsFramebufferSurface surfaces[] =
	{
		{dsGfxSurfaceType_ColorRenderSurfaceLeft, dsCubeFace_None, 0, 0, renderSurface},
		{dsGfxSurfaceType_DepthRenderSurfaceLeft, dsCubeFace_None, 0, 0, renderSurface}
	};

	EXPECT_FALSE(dsFramebuffer_create(resourceManager, NULL, "test", surfaces,
		DS_ARRAY_SIZE(surfaces), renderSurface->width, renderSurface->height, 1));

	renderer->stereoscopic = true;

	dsFramebuffer* framebuffer = dsFramebuffer_create(resourceManager, NULL, "test", surfaces,
		DS_ARRAY_SIZE(surfaces), renderSurface->width, renderSurface->height, 1);
	ASSERT_TRUE(framebuffer);

	EXPECT_TRUE(dsFramebuffer_destroy(framebuffer));
	EXPECT_TRUE(dsRenderSurface_destroy(renderSurface));
}

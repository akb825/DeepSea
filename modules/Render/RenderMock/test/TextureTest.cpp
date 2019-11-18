/*
 * Copyright 2016-2019 Aaron Barany
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

#include "Fixtures/RenderPassFixtureBase.h"
#include <DeepSea/Math/Types.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/RenderPass.h>
#include <gtest/gtest.h>
#include <string.h>

class TextureTest : public RenderPassFixtureBase
{
};

TEST_F(TextureTest, MaxMipmapLevels)
{
	EXPECT_EQ(0U, dsTexture_maxMipmapLevels(0, 0, 0));
	EXPECT_EQ(1U, dsTexture_maxMipmapLevels(1, 1, 0));
	EXPECT_EQ(2U, dsTexture_maxMipmapLevels(2, 2, 0));
	EXPECT_EQ(6U, dsTexture_maxMipmapLevels(32, 32, 0));
	EXPECT_EQ(6U, dsTexture_maxMipmapLevels(32, 16, 0));
	EXPECT_EQ(6U, dsTexture_maxMipmapLevels(16, 32, 0));
	EXPECT_EQ(6U, dsTexture_maxMipmapLevels(16, 33, 0));
	EXPECT_EQ(7U, dsTexture_maxMipmapLevels(16, 33, 65));
}

TEST_F(TextureTest, Size)
{
	dsTextureInfo info = {dsGfxFormat_R8G8B8A8, dsTextureDim_2D, 512, 512, 0, 1, 1};
	EXPECT_EQ(0U, dsTexture_size(&info));

	info.format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	EXPECT_EQ(1048576U, dsTexture_size(&info));

	info.dimension = dsTextureDim_Cube;
	EXPECT_EQ(6291456U, dsTexture_size(&info));

	info.dimension = dsTextureDim_2D;
	info.depth = 3;
	EXPECT_EQ(3145728U, dsTexture_size(&info));

	info.depth = 1;
	info.mipLevels = DS_ALL_MIP_LEVELS;
	EXPECT_EQ(1398100U, dsTexture_size(&info));

	info.mipLevels = 1;
	info.samples = 4;
	EXPECT_EQ(4194304U, dsTexture_size(&info));

	info.dimension = dsTextureDim_3D;
	info.depth = 128;
	info.mipLevels = DS_ALL_MIP_LEVELS;
	info.samples = 1;
	EXPECT_EQ(153391700U, dsTexture_size(&info));

	info.dimension = dsTextureDim_2D;
	EXPECT_EQ(178956800U, dsTexture_size(&info));

	info.format = dsGfxFormat_decorate(dsGfxFormat_BC1_RGB, dsGfxFormat_UNorm);
	info.width = info.height = 1;
	info.depth = 1;
	info.mipLevels = 1;
	EXPECT_EQ(8U, dsTexture_size(&info));
}

TEST_F(TextureTest, SurfaceCount)
{
	dsTextureInfo info = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		dsTextureDim_2D, 512, 512, 0, 1, 1};
	EXPECT_EQ(1U, dsTexture_surfaceCount(&info));

	info.mipLevels = DS_ALL_MIP_LEVELS;
	EXPECT_EQ(10U, dsTexture_surfaceCount(&info));

	info.dimension = dsTextureDim_Cube;
	EXPECT_EQ(60U, dsTexture_surfaceCount(&info));

	info.depth = 5;
	EXPECT_EQ(300U, dsTexture_surfaceCount(&info));

	info.dimension = dsTextureDim_2D;
	EXPECT_EQ(50U, dsTexture_surfaceCount(&info));

	info.dimension = dsTextureDim_3D;
	EXPECT_EQ(15U, dsTexture_surfaceCount(&info));
}

TEST_F(TextureTest, SurfaceIndex)
{
	dsTextureInfo info = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		dsTextureDim_2D, 512, 512, 0, DS_ALL_MIP_LEVELS, 1};
	EXPECT_EQ(0U, dsTexture_surfaceIndex(&info, dsCubeFace_None, 0, 0));
	EXPECT_EQ(3U, dsTexture_surfaceIndex(&info, dsCubeFace_None, 0, 3));

	info.dimension = dsTextureDim_Cube;
	EXPECT_EQ(21U, dsTexture_surfaceIndex(&info, dsCubeFace_NegY, 0, 3));

	info.depth = 5;
	EXPECT_EQ(105U, dsTexture_surfaceIndex(&info, dsCubeFace_NegY, 2, 3));

	info.dimension = dsTextureDim_2D;
	EXPECT_EQ(17U, dsTexture_surfaceIndex(&info, dsCubeFace_None, 2, 3));

	info.dimension = dsTextureDim_3D;
	EXPECT_EQ(DS_INVALID_TEXTURE_SURFACE, dsTexture_surfaceIndex(&info, dsCubeFace_None, 2, 3));
	EXPECT_EQ(8U, dsTexture_surfaceIndex(&info, dsCubeFace_None, 0, 3));
}

TEST_F(TextureTest, SurfaceOffset)
{
	dsTextureInfo info = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		dsTextureDim_2D, 512, 512, 0, 1, 1};
	EXPECT_EQ(0U, dsTexture_surfaceOffset(&info, dsCubeFace_None, 0, 0));

	info.mipLevels = DS_ALL_MIP_LEVELS;
	EXPECT_EQ(1048576U, dsTexture_surfaceOffset(&info, dsCubeFace_None, 0, 1));
	EXPECT_EQ(1310720U, dsTexture_surfaceOffset(&info, dsCubeFace_None, 0, 2));

	info.depth = 3;
	EXPECT_EQ(4063232U, dsTexture_surfaceOffset(&info, dsCubeFace_None, 2, 2));

	info.dimension = dsTextureDim_3D;
	info.depth = 128;
	EXPECT_EQ(151191552U, dsTexture_surfaceOffset(&info, dsCubeFace_None, 3, 2));

	info.dimension = dsTextureDim_Cube;
	info.depth = 3;
	EXPECT_EQ(24576000U, dsTexture_surfaceOffset(&info, dsCubeFace_NegY, 2, 2));
}

TEST_F(TextureTest, LayerOffset)
{
	dsTextureInfo info = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		dsTextureDim_2D, 512, 512, 0, 1, 1};
	EXPECT_EQ(0U, dsTexture_layerOffset(&info, 0, 0));

	info.mipLevels = DS_ALL_MIP_LEVELS;
	EXPECT_EQ(1048576U, dsTexture_layerOffset(&info, 0, 1));
	EXPECT_EQ(1310720U, dsTexture_layerOffset(&info, 0, 2));

	info.depth = 3;
	EXPECT_EQ(4063232U, dsTexture_layerOffset(&info, 2, 2));

	info.dimension = dsTextureDim_3D;
	info.depth = 128;
	EXPECT_EQ(151191552U, dsTexture_layerOffset(&info, 3, 2));

	info.dimension = dsTextureDim_Cube;
	info.depth = 3;
	EXPECT_EQ(24576000U, dsTexture_layerOffset(&info, 15, 2));
}

TEST_F(TextureTest, Create)
{
	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	dsTextureInfo info = {format, dsTextureDim_2D, 128, 256, 0, 1, 1};
	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture, dsGfxMemory_Read,
		NULL, NULL, 0));
	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, (dsTextureUsage)0, dsGfxMemory_Read,
		&info, NULL, 0));
	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture, (dsGfxMemory)0,
		&info, NULL, 0));
	info.format = dsGfxFormat_R8G8B8A8;
	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, NULL, 0));
	info.format = format;
	info.samples = 4;
	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, NULL, 0));
	info.samples = 1;

	dsTexture* texture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, NULL, 0);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ((size_t)(128*256*4), resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	info.dimension = dsTextureDim_3D;
	info.depth = 257;
	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, NULL, 0));
	info.depth = 256;
	texture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture, dsGfxMemory_Read,
		&info, NULL, 0);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ((size_t)(128*256*4*256), resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	info.dimension = dsTextureDim_2D;
	info.depth = 513;
	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, NULL, 0));
	info.depth = 512;
	texture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture, dsGfxMemory_Read,
		&info, NULL, 0);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ((size_t)(128*256*4*512), resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	info.format = dsGfxFormat_decorate(dsGfxFormat_BC3, dsGfxFormat_UNorm);
	info.depth = 0;
	texture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture, dsGfxMemory_Read,
		&info, NULL, 0);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ((size_t)(128*256), resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	info.format = format;
	info.mipLevels = 3;
	texture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture, dsGfxMemory_Read,
		&info, NULL, 0);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ((size_t)((128*256 + 64*128 + 32*64)*4), resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	resourceManager->hasArbitraryMipmapping = false;
	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture, dsGfxMemory_Read,
		&info, NULL, 0));

	resourceManager->hasCubeArrays = false;
	info.dimension = dsTextureDim_Cube;
	info.depth = 3;
	info.mipLevels = 1;
	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture, dsGfxMemory_Read,
		&info, NULL, 0));
}

TEST_F(TextureTest, CreateOffscreen)
{
	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	dsTextureInfo info = {format, dsTextureDim_2D, 128, 256, 0, 1, 1};
	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, NULL, true));
	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, (dsTextureUsage)0,
		dsGfxMemory_Read, &info, true));
	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		(dsGfxMemory)0, &info, true));
	info.format = dsGfxFormat_R8G8B8A8;
	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, true));
	info.format = format;

	dsTexture* texture = dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, true);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ((size_t)(128*256*4), resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	info.samples = 4;
	texture = dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, false);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ((size_t)(128*256*4*4), resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	texture = dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, true);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ((size_t)(128*256*4*5), resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	info.dimension = dsTextureDim_3D;
	info.depth = 257;
	info.samples = 1;
	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, true));
	info.depth = 256;
	texture = dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, true);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ((size_t)(128*256*4*256), resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	info.dimension = dsTextureDim_2D;
	info.depth = 513;
	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, true));
	info.depth = 512;
	texture = dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, true);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ((size_t)(128*256*4*512), resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	info.format = dsGfxFormat_decorate(dsGfxFormat_BC3, dsGfxFormat_UNorm);
	info.depth = 0;
	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, true));

	info.format = format;
	info.mipLevels = 3;
	texture = dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, true);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ(((size_t)(128*256 + 64*128 + 32*64)*4), resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	info.mipLevels = 1;
	info.samples = 32;
	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, true));

	resourceManager->hasArbitraryMipmapping = false;
	info.samples = 1;
	info.mipLevels = 3;
	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, true));

	resourceManager->hasCubeArrays = false;
	info.dimension = dsTextureDim_Cube;
	info.depth = 3;
	info.mipLevels = 1;
	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, true));

	resourceManager->maxTextureSamples = 1;
	info.dimension = dsTextureDim_2D;
	info.depth = 1;
	info.samples = 4;
	texture = dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, true);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ((size_t)(128*256*4*5), resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, false));
}

TEST_F(TextureTest, GetData)
{
	dsColor textureData[32*16 + 16*8 + 8*4];
	for (unsigned int level = 0, index = 0; level < 3; ++level)
	{
		unsigned int width = 32 >> level;
		unsigned int height = 16 >> level;
		for (unsigned int y = 0; y < height; ++y)
		{
			for (unsigned int x = 0; x < width; ++x, ++index)
			{
				textureData[index].r = (uint8_t)x;
				textureData[index].g = (uint8_t)y;
				textureData[index].b = (uint8_t)level;
				textureData[index].a = 0;
			}
		}
	}

	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	dsTextureInfo info = {format, dsTextureDim_2D, 32, 16, 0, 3, 1};
	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, textureData, 100));

	dsTexture* texture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, textureData, sizeof(textureData));
	ASSERT_TRUE(texture);

	dsColor readTextureData[sizeof(textureData)];
	dsTexturePosition position = {dsCubeFace_None, 3, 4, 0, 1};
	EXPECT_FALSE(dsTexture_getData(readTextureData, 8*4*4, texture, &position, 8, 4));
	EXPECT_TRUE(dsTexture_destroy(texture));

	texture = dsTexture_create(resourceManager, NULL,
		(dsTextureUsage)(dsTextureUsage_Texture | dsTextureUsage_CopyFrom), dsGfxMemory_Read,
		&info, textureData, sizeof(textureData));
	ASSERT_TRUE(texture);
	EXPECT_FALSE(dsTexture_getData(NULL, 8*4*4, texture, &position, 8, 4));
	EXPECT_FALSE(dsTexture_getData(readTextureData, 100, texture, &position, 8, 4));
	EXPECT_FALSE(dsTexture_getData(readTextureData, 8*4*4, texture, NULL, 8, 4));
	EXPECT_TRUE(dsTexture_getData(readTextureData, 8*4*4, texture, &position, 8, 4));

	for (unsigned int y = 0, index = 0; y < 4; ++y)
	{
		for (unsigned int x = 0; x < 8; ++x, ++index)
		{
			EXPECT_EQ(3 + x, readTextureData[index].r);
			EXPECT_EQ(4 + y, readTextureData[index].g);
			EXPECT_EQ(1U, readTextureData[index].b);
			EXPECT_EQ(0U, readTextureData[index].a);
		}
	}

	position.x = 9;
	EXPECT_FALSE(dsTexture_getData(readTextureData, 8*4*4, texture, &position, 8, 4));

	position.x = 3;
	position.y = 5;
	EXPECT_FALSE(dsTexture_getData(readTextureData, 8*4*4, texture, &position, 8, 4));

	position.x = 0;
	position.y = 0;
	position.mipLevel = 5;
	EXPECT_FALSE(dsTexture_getData(readTextureData, 8*4*4, texture, &position, 8, 4));

	position.mipLevel = 0;
	position.depth = 1;
	EXPECT_FALSE(dsTexture_getData(readTextureData, 8*4*4, texture, &position, 8, 4));

	position.depth = 0;
	EXPECT_TRUE(dsTexture_getData(readTextureData, 8*4*4, texture, &position, 8, 4));
	resourceManager->texturesReadable = false;
	EXPECT_FALSE(dsTexture_getData(readTextureData, 8*4*4, texture, &position, 8, 4));

	EXPECT_TRUE(dsTexture_destroy(texture));
}

TEST_F(TextureTest, CopyData)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	dsTextureInfo info = {format, dsTextureDim_2D, 32, 16, 0, 3, 1};
	dsTexture* texture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read, &info, NULL, 0);
	ASSERT_TRUE(texture);

	dsColor textureData[8*4];
	for (unsigned int y = 0, index = 0; y < 4; ++y)
	{
		for (unsigned int x = 0; x < 8; ++x, ++index)
		{
			textureData[index].r = (uint8_t)x;
			textureData[index].g = (uint8_t)y;
			textureData[index].b = 0;
			textureData[index].a = 1;
		}
	}
	dsTexturePosition position = {dsCubeFace_None, 3, 4, 0, 1};
	EXPECT_FALSE(dsTexture_copyData(texture, commandBuffer, &position, 8, 4, 1, textureData,
		8*4*4));
	EXPECT_TRUE(dsTexture_destroy(texture));

	texture = dsTexture_create(resourceManager, NULL,
		(dsTextureUsage)(dsTextureUsage_Texture | dsTextureUsage_CopyTo | dsTextureUsage_CopyFrom),
		dsGfxMemory_Read, &info, NULL, 0);
	ASSERT_TRUE(texture);
	EXPECT_FALSE(dsTexture_copyData(texture, NULL, &position, 8, 4, 1, textureData,
		sizeof(textureData)));
	EXPECT_FALSE(dsTexture_copyData(texture, commandBuffer, NULL, 8, 4, 1, textureData,
		sizeof(textureData)));
	EXPECT_FALSE(dsTexture_copyData(texture, commandBuffer, &position, 8, 4, 1, NULL,
		sizeof(textureData)));
	EXPECT_FALSE(dsTexture_copyData(texture, commandBuffer, &position, 8, 4, 1, textureData,
		100));
	EXPECT_FALSE(dsTexture_copyData(texture, commandBuffer, &position, 8, 4, 2, textureData,
		sizeof(textureData)));

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0, false));
	EXPECT_FALSE(dsTexture_copyData(texture, commandBuffer, &position, 8, 4, 1, textureData,
		sizeof(textureData)));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	EXPECT_TRUE(dsTexture_copyData(texture, commandBuffer, &position, 8, 4, 1, textureData,
		sizeof(textureData)));

	memset(textureData, 0, sizeof(textureData));
	EXPECT_TRUE(dsTexture_getData(textureData, sizeof(textureData), texture, &position, 8, 4));
	for (unsigned int y = 0, index = 0; y < 4; ++y)
	{
		for (unsigned int x = 0; x < 8; ++x, ++index)
		{
			EXPECT_EQ(x, textureData[index].r);
			EXPECT_EQ(y, textureData[index].g);
			EXPECT_EQ(0U, textureData[index].b);
			EXPECT_EQ(1U, textureData[index].a);
		}
	}

	position.x = 9;
	EXPECT_FALSE(dsTexture_copyData(texture, commandBuffer, &position, 8, 4, 1, textureData,
		sizeof(textureData)));

	position.x = 3;
	position.y = 5;
	EXPECT_FALSE(dsTexture_copyData(texture, commandBuffer, &position, 8, 4, 1, textureData,
		sizeof(textureData)));

	position.x = 0;
	position.y = 0;
	position.mipLevel = 5;
	EXPECT_FALSE(dsTexture_copyData(texture, commandBuffer, &position, 8, 4, 1, textureData,
		sizeof(textureData)));

	position.mipLevel = 0;
	position.depth = 1;
	EXPECT_FALSE(dsTexture_copyData(texture, commandBuffer, &position, 8, 4, 1, textureData,
		sizeof(textureData)));

	EXPECT_TRUE(dsTexture_destroy(texture));
}

TEST_F(TextureTest, Copy)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsColor textureData[(32*16 + 16*8 + 8*4)*4];
	for (unsigned int level = 0, index = 0; level < 3; ++level)
	{
		unsigned int width = 32 >> level;
		unsigned int height = 16 >> level;
		for (unsigned int depth = 0; depth < 4; ++depth)
		{
			for (unsigned int y = 0; y < height; ++y)
			{
				for (unsigned int x = 0; x < width; ++x, ++index)
				{
					textureData[index].r = (uint8_t)x;
					textureData[index].g = (uint8_t)y;
					textureData[index].b = (uint8_t)level;
					textureData[index].a = (uint8_t)depth;
				}
			}
		}
	}

	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	dsTextureInfo fromInfo = {format, dsTextureDim_2D, 32, 16, 4, 3, 1};
	dsTexture* fromTexture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_GPUOnly, &fromInfo, textureData, sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	dsTextureInfo toInfo = {format, dsTextureDim_2D, 16, 32, 5, 2, 1};
	dsTexture* toTexture = dsTexture_create(resourceManager, NULL,
		(dsTextureUsage)(dsTextureUsage_Texture | dsTextureUsage_CopyTo | dsTextureUsage_CopyFrom),
		dsGfxMemory_Read, &toInfo, NULL, 0);
	ASSERT_TRUE(toTexture);

	dsTextureCopyRegion copyRegion =
	{
		{dsCubeFace_None, 1, 2, 2, 1},
		{dsCubeFace_None, 3, 4, 1, 0},
		8, 4, 2
	};

	EXPECT_FALSE(dsTexture_copy(commandBuffer, fromTexture, toTexture, &copyRegion, 1));
	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsTexture_destroy(toTexture));

	fromTexture = dsTexture_create(resourceManager, NULL,
		(dsTextureUsage)(dsTextureUsage_Texture | dsTextureUsage_CopyFrom), dsGfxMemory_GPUOnly,
		&fromInfo, textureData, sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	toTexture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture, dsGfxMemory_Read,
		&toInfo, NULL, 0);
	ASSERT_TRUE(toTexture);

	EXPECT_FALSE(dsTexture_copy(commandBuffer, fromTexture, toTexture, &copyRegion, 1));
	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsTexture_destroy(toTexture));

	fromTexture = dsTexture_create(resourceManager, NULL,
		(dsTextureUsage)(dsTextureUsage_Texture | dsTextureUsage_CopyFrom), dsGfxMemory_GPUOnly,
		&fromInfo, textureData, sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	toTexture = dsTexture_create(resourceManager, NULL,
		(dsTextureUsage)(dsTextureUsage_Texture | dsTextureUsage_CopyTo | dsTextureUsage_CopyFrom),
		dsGfxMemory_Read, &toInfo, NULL, 0);
	ASSERT_TRUE(toTexture);

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0, false));
	EXPECT_FALSE(dsTexture_copy(commandBuffer, fromTexture, toTexture, &copyRegion, 1));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	EXPECT_TRUE(dsTexture_copy(commandBuffer, fromTexture, toTexture, &copyRegion, 1));

	dsColor readTextureData[8*4];
	EXPECT_TRUE(dsTexture_getData(readTextureData, sizeof(readTextureData), toTexture,
		&copyRegion.dstPosition, 8, 4));
	for (unsigned int y = 0, index = 0; y < 4; ++y)
	{
		for (unsigned int x = 0; x < 8; ++x, ++index)
		{
			EXPECT_EQ(x + 1, readTextureData[index].r);
			EXPECT_EQ(y + 2, readTextureData[index].g);
			EXPECT_EQ(1U, readTextureData[index].b);
			EXPECT_EQ(2U, readTextureData[index].a);
		}
	}

	copyRegion.dstPosition.depth = 2;
	EXPECT_TRUE(dsTexture_getData(readTextureData, sizeof(readTextureData), toTexture,
		&copyRegion.dstPosition, 8, 4));
	for (unsigned int y = 0, index = 0; y < 4; ++y)
	{
		for (unsigned int x = 0; x < 8; ++x, ++index)
		{
			EXPECT_EQ(x + 1, readTextureData[index].r);
			EXPECT_EQ(y + 2, readTextureData[index].g);
			EXPECT_EQ(1U, readTextureData[index].b);
			EXPECT_EQ(3U, readTextureData[index].a);
		}
	}

	copyRegion.srcPosition.x = 25;
	EXPECT_FALSE(dsTexture_copy(commandBuffer, fromTexture, toTexture, &copyRegion, 1));

	copyRegion.srcPosition.x = 1;
	copyRegion.srcPosition.y = 13;
	EXPECT_FALSE(dsTexture_copy(commandBuffer, fromTexture, toTexture, &copyRegion, 1));

	copyRegion.srcPosition.x = 0;
	copyRegion.srcPosition.y = 0;
	copyRegion.srcPosition.mipLevel = 5;
	EXPECT_FALSE(dsTexture_copy(commandBuffer, fromTexture, toTexture, &copyRegion, 1));

	copyRegion.srcPosition.mipLevel = 0;
	copyRegion.srcPosition.depth = 3;
	EXPECT_FALSE(dsTexture_copy(commandBuffer, fromTexture, toTexture, &copyRegion, 1));

	copyRegion.srcPosition.depth = 0;
	copyRegion.dstPosition.x = 17;
	EXPECT_FALSE(dsTexture_copy(commandBuffer, fromTexture, toTexture, &copyRegion, 1));

	copyRegion.dstPosition.x = 3;
	copyRegion.dstPosition.y = 29;
	EXPECT_FALSE(dsTexture_copy(commandBuffer, fromTexture, toTexture, &copyRegion, 1));

	copyRegion.dstPosition.y = 4;
	copyRegion.dstPosition.mipLevel = 3;
	EXPECT_FALSE(dsTexture_copy(commandBuffer, fromTexture, toTexture, &copyRegion, 1));

	copyRegion.dstPosition.mipLevel = 0;
	copyRegion.dstPosition.depth = 4;
	EXPECT_FALSE(dsTexture_copy(commandBuffer, fromTexture, toTexture, &copyRegion, 1));

	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsTexture_destroy(toTexture));
}

TEST_F(TextureTest, CopyToBuffer)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsColor textureData[(32*16 + 16*8 + 8*4)*4];
	for (unsigned int level = 0, index = 0; level < 3; ++level)
	{
		unsigned int width = 32 >> level;
		unsigned int height = 16 >> level;
		for (unsigned int depth = 0; depth < 4; ++depth)
		{
			for (unsigned int y = 0; y < height; ++y)
			{
				for (unsigned int x = 0; x < width; ++x, ++index)
				{
					textureData[index].r = (uint8_t)x;
					textureData[index].g = (uint8_t)y;
					textureData[index].b = (uint8_t)level;
					textureData[index].a = (uint8_t)depth;
				}
			}
		}
	}

	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	dsTextureInfo fromInfo = {format, dsTextureDim_2D, 32, 16, 4, 3, 1};
	dsTexture* fromTexture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_GPUOnly, &fromInfo, textureData,
		sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	dsTextureInfo toInfo = {format, dsTextureDim_2D, 16, 32, 5, 2, 1};
	size_t toSize = dsTexture_size(&toInfo);
	dsGfxBuffer* toBuffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_CopyTo,
		dsGfxMemory_Read, NULL, toSize);
	ASSERT_TRUE(toBuffer);

	// array index 1, mip level 0, position (3, 4)
	size_t bufferOffset = dsTexture_layerOffset(&fromInfo, 1, 0) + (4*16 + 3)*sizeof(dsColor);
	dsGfxBufferTextureCopyRegion copyRegion =
	{
		bufferOffset, 16, 32,
		{dsCubeFace_None, 1, 2, 2, 1},
		8, 4, 2
	};

	EXPECT_FALSE(dsTexture_copyToBuffer(commandBuffer, fromTexture, toBuffer, &copyRegion, 1));
	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsGfxBuffer_destroy(toBuffer));

	fromTexture = dsTexture_create(resourceManager, NULL,
		(dsTextureUsage)(dsTextureUsage_Texture | dsTextureUsage_CopyFrom), dsGfxMemory_GPUOnly,
		&fromInfo, textureData, sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	toBuffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_Vertex, dsGfxMemory_Read,
		NULL, toSize);
	ASSERT_TRUE(toBuffer);

	EXPECT_FALSE(dsTexture_copyToBuffer(commandBuffer, fromTexture, toBuffer, &copyRegion, 1));
	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsGfxBuffer_destroy(toBuffer));

	fromTexture = dsTexture_create(resourceManager, NULL,
		(dsTextureUsage)(dsTextureUsage_Texture | dsTextureUsage_CopyFrom), dsGfxMemory_GPUOnly,
		&fromInfo, textureData, sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	toBuffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_CopyTo,
		dsGfxMemory_Read, NULL, toSize);
	ASSERT_TRUE(toBuffer);

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0, false));
	EXPECT_FALSE(dsTexture_copyToBuffer(commandBuffer, fromTexture, toBuffer, &copyRegion, 1));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	EXPECT_TRUE(dsTexture_copyToBuffer(commandBuffer, fromTexture, toBuffer, &copyRegion, 1));

	const uint8_t* bufferData =
		(const uint8_t*)dsGfxBuffer_map(toBuffer, dsGfxBufferMap_Read, 0, DS_MAP_FULL_BUFFER);
	ASSERT_TRUE(bufferData);
	const dsColor* readTextureData = (const dsColor*)(bufferData + bufferOffset);
	for (unsigned int y = 0, index = 0; y < 4; ++y)
	{
		for (unsigned int x = 0; x < 8; ++x, ++index)
		{
			EXPECT_EQ(x + 1, readTextureData[index].r);
			EXPECT_EQ(y + 2, readTextureData[index].g);
			EXPECT_EQ(1U, readTextureData[index].b);
			EXPECT_EQ(2U, readTextureData[index].a);
		}
		index += copyRegion.bufferWidth - copyRegion.textureWidth;
	}

	size_t nextBufferOffset = dsTexture_layerOffset(&fromInfo, 2, 0) + (4*16 + 3)*sizeof(dsColor);
	readTextureData = (const dsColor*)(bufferData + nextBufferOffset);
	for (unsigned int y = 0, index = 0; y < 4; ++y)
	{
		for (unsigned int x = 0; x < 8; ++x, ++index)
		{
			EXPECT_EQ(x + 1, readTextureData[index].r);
			EXPECT_EQ(y + 2, readTextureData[index].g);
			EXPECT_EQ(1U, readTextureData[index].b);
			EXPECT_EQ(3U, readTextureData[index].a);
		}
		index += copyRegion.bufferWidth - copyRegion.textureWidth;
	}
	EXPECT_TRUE(dsGfxBuffer_unmap(toBuffer));

	copyRegion.texturePosition.x = 25;
	EXPECT_FALSE(dsTexture_copyToBuffer(commandBuffer, fromTexture, toBuffer, &copyRegion, 1));

	copyRegion.texturePosition.x = 1;
	copyRegion.texturePosition.y = 13;
	EXPECT_FALSE(dsTexture_copyToBuffer(commandBuffer, fromTexture, toBuffer, &copyRegion, 1));

	copyRegion.texturePosition.x = 0;
	copyRegion.texturePosition.y = 0;
	copyRegion.texturePosition.mipLevel = 5;
	EXPECT_FALSE(dsTexture_copyToBuffer(commandBuffer, fromTexture, toBuffer, &copyRegion, 1));

	copyRegion.texturePosition.mipLevel = 0;
	copyRegion.texturePosition.depth = 3;
	EXPECT_FALSE(dsTexture_copyToBuffer(commandBuffer, fromTexture, toBuffer, &copyRegion, 1));

	copyRegion.texturePosition.depth = 0;
	copyRegion.bufferWidth = 1;
	EXPECT_FALSE(dsTexture_copyToBuffer(commandBuffer, fromTexture, toBuffer, &copyRegion, 1));

	copyRegion.bufferWidth = 16;
	copyRegion.bufferHeight = 1;
	EXPECT_FALSE(dsTexture_copyToBuffer(commandBuffer, fromTexture, toBuffer, &copyRegion, 1));

	copyRegion.bufferHeight = 32;
	copyRegion.bufferOffset = dsTexture_layerOffset(&toInfo, 4, 1);
	EXPECT_FALSE(dsTexture_copyToBuffer(commandBuffer, fromTexture, toBuffer, &copyRegion, 1));

	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsGfxBuffer_destroy(toBuffer));
}

TEST_F(TextureTest, GenerateMipmaps)
{
	dsTextureUsage usage = (dsTextureUsage)(dsTextureUsage_Texture | dsTextureUsage_CopyFrom |
		dsTextureUsage_CopyTo);
	dsTextureInfo info = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		dsTextureDim_2D, 32, 16, 0, DS_ALL_MIP_LEVELS, 1};
	dsTexture* texture1 = dsTexture_create(resourceManager, NULL, usage, dsGfxMemory_Read, &info,
		NULL, 0);
	ASSERT_TRUE(texture1);

	info.format = dsGfxFormat_decorate(dsGfxFormat_BC1_RGB, dsGfxFormat_UNorm);
	dsTexture* texture2 = dsTexture_create(resourceManager, NULL, usage, dsGfxMemory_Read, &info,
		NULL, 0);
	ASSERT_TRUE(texture1);

	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;
	EXPECT_FALSE(dsTexture_generateMipmaps(texture1, NULL));
	EXPECT_FALSE(dsTexture_generateMipmaps(NULL, commandBuffer));
	EXPECT_TRUE(dsTexture_generateMipmaps(texture1, commandBuffer));
	EXPECT_FALSE(dsTexture_generateMipmaps(texture2, commandBuffer));

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0, false));
	EXPECT_FALSE(dsTexture_generateMipmaps(texture1, commandBuffer));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	EXPECT_TRUE(dsTexture_destroy(texture1));
	EXPECT_TRUE(dsTexture_destroy(texture2));
}

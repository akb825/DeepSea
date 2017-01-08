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

#include "FixtureBase.h"
#include <DeepSea/Math/Types.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <gtest/gtest.h>
#include <string.h>

class TextureTest : public FixtureBase
{
};

TEST_F(TextureTest, MaxMipmapLevels)
{
	EXPECT_EQ(0U, dsTexture_maxMipmapLevels(0, 0));
	EXPECT_EQ(1U, dsTexture_maxMipmapLevels(1, 1));
	EXPECT_EQ(2U, dsTexture_maxMipmapLevels(2, 2));
	EXPECT_EQ(6U, dsTexture_maxMipmapLevels(32, 32));
	EXPECT_EQ(6U, dsTexture_maxMipmapLevels(32, 16));
	EXPECT_EQ(6U, dsTexture_maxMipmapLevels(16, 32));
	EXPECT_EQ(6U, dsTexture_maxMipmapLevels(16, 33));
}

TEST_F(TextureTest, Size)
{
	EXPECT_EQ(0U, dsTexture_size(dsGfxFormat_R8G8B8A8, dsTextureDim_2D, 512, 512, 1, 1, 1));
	EXPECT_EQ(1048576U, dsTexture_size(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
		dsGfxFormat_SNorm), dsTextureDim_2D, 512, 512, 1, 1, 1));
	EXPECT_EQ(6291456U, dsTexture_size(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
		dsGfxFormat_SNorm), dsTextureDim_Cube, 512, 512, 1, 1, 1));
	EXPECT_EQ(3145728U, dsTexture_size(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
		dsGfxFormat_SNorm), dsTextureDim_2D, 512, 512, 3, 1, 1));
	EXPECT_EQ(1398100U, dsTexture_size(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
		dsGfxFormat_SNorm), dsTextureDim_2D, 512, 512, 1, DS_ALL_MIP_LEVELS, 1));
	EXPECT_EQ(4194304U, dsTexture_size(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
		dsGfxFormat_SNorm), dsTextureDim_2D, 512, 512, 1, 1, 4));
	EXPECT_EQ(8U, dsTexture_size(dsGfxFormat_BC1_RGB, dsTextureDim_2D, 1, 1, 1, 1, 1));
}

TEST_F(TextureTest, SurfaceOffset)
{
	EXPECT_EQ(0U, dsTexture_surfaceOffset(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
		dsGfxFormat_SNorm), dsTextureDim_2D, 512, 512, 1, 1, dsCubeFace_PosX, 0, 0));
	EXPECT_EQ(1048576U, dsTexture_surfaceOffset(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
		dsGfxFormat_SNorm), dsTextureDim_2D, 512, 512, 1, DS_ALL_MIP_LEVELS, dsCubeFace_PosX, 0,
		1));
	EXPECT_EQ(1310720U, dsTexture_surfaceOffset(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
		dsGfxFormat_SNorm), dsTextureDim_2D, 512, 512, 1, DS_ALL_MIP_LEVELS, dsCubeFace_PosX, 0,
		2));
	EXPECT_EQ(4063232U, dsTexture_surfaceOffset(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
		dsGfxFormat_SNorm), dsTextureDim_2D, 512, 512, 3, DS_ALL_MIP_LEVELS, dsCubeFace_PosX, 2,
		2));
	EXPECT_EQ(24576000U, dsTexture_surfaceOffset(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
		dsGfxFormat_SNorm), dsTextureDim_Cube, 512, 512, 3, DS_ALL_MIP_LEVELS, dsCubeFace_NegY, 2,
		2));
}

TEST_F(TextureTest, Create)
{
	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, 0, 0, format, dsTextureDim_2D, 128, 256, 0,
		1, NULL, 0));
	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture, 0, format,
		dsTextureDim_2D, 128, 256, 0, 1, NULL, 0));
	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, 0, dsGfxMemory_Static, format,
		dsTextureDim_2D, 128, 256, 0, 1, NULL, 0));
	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, dsGfxFormat_R8G8B8A8, dsTextureDim_2D, 128, 256, 0, 1, NULL, 0));

	dsTexture* texture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, format, dsTextureDim_2D, 128, 256, 0, 1, NULL, 0);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ(128*256*4, resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, format, dsTextureDim_3D, 128, 256, 257, 1, NULL, 0));
	texture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture, dsGfxMemory_Static,
		format, dsTextureDim_3D, 128, 256, 256, 1, NULL, 0);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ(128*256*4*256, resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, format, dsTextureDim_2D, 128, 256, 513, 1, NULL, 0));
	texture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture, dsGfxMemory_Static,
		format, dsTextureDim_2D, 128, 256, 512, 1, NULL, 0);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ(128*256*4*512, resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, dsGfxFormat_BC3, dsTextureDim_2D, 127, 255, 0, 1, NULL, 0));
	texture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture, dsGfxMemory_Static,
		dsGfxFormat_BC3, dsTextureDim_2D, 128, 256, 0, 1, NULL, 0);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ(128*256, resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	texture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture, dsGfxMemory_Static,
		format, dsTextureDim_2D, 128, 256, 0, 3, NULL, 0);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ((128*256 + 64*128 + 32*64)*4, resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	resourceManager->arbitraryMipmapping = false;
	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture, dsGfxMemory_Static,
		format, dsTextureDim_2D, 128, 256, 0, 3, NULL, 0));
}

TEST_F(TextureTest, CreateOffscreen)
{
	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, 0, 0, format, dsTextureDim_2D, 128,
		256, 0, 1, 1, true));
	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture, 0, format,
		dsTextureDim_2D, 128, 256, 0, 1, 1, true));
	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, 0, dsGfxMemory_Static, format,
		dsTextureDim_2D, 128, 256, 0, 1, 1, true));
	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, dsGfxFormat_R8G8B8A8, dsTextureDim_2D, 128, 256, 0, 1, 1, true));

	dsTexture* texture = dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, format, dsTextureDim_2D, 128, 256, 0, 1, 1, true);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ(128*256*4, resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	texture = dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, format, dsTextureDim_2D, 128, 256, 0, 1, 4, true);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ(128*256*4*4, resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, format, dsTextureDim_3D, 128, 256, 257, 1, 1, true));
	texture = dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, format, dsTextureDim_3D, 128, 256, 256, 1, 1, true);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ(128*256*4*256, resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, format, dsTextureDim_2D, 128, 256, 513, 1, 1, true));
	texture = dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, format, dsTextureDim_2D, 128, 256, 512, 1, 1, true);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ(128*256*4*512, resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, dsGfxFormat_BC3, dsTextureDim_2D, 128, 256, 0, 1, 1, true));

	texture = dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, format, dsTextureDim_2D, 128, 256, 0, 3, 1, true);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, resourceManager->textureCount);
	EXPECT_EQ((128*256 + 64*128 + 32*64)*4, resourceManager->textureMemorySize);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_EQ(0U, resourceManager->textureCount);
	EXPECT_EQ(0U, resourceManager->textureMemorySize);

	resourceManager->arbitraryMipmapping = false;
	EXPECT_FALSE(dsTexture_createOffscreen(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, format, dsTextureDim_2D, 128, 256, 0, 3, 1, true));
}

TEST_F(TextureTest, GetData)
{
	dsColor textureData[32*16 + 16*8 + 8*4];
	for (unsigned int level = 0, index = 0; level < 3; ++level)
	{
		unsigned int width = 32/(1 << level);
		unsigned int height = 16/(1 << level);
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
	EXPECT_FALSE(dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, format, dsTextureDim_2D, 32, 16, 0, 3, textureData, 100));

	dsTexture* texture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, format, dsTextureDim_2D, 32, 16, 0, 3, textureData,
		sizeof(textureData));
	ASSERT_TRUE(texture);

	dsColor readTextureData[sizeof(textureData)];
	dsTexturePosition position = {dsCubeFace_PosX, 3, 4, 0, 1};
	EXPECT_FALSE(dsTexture_getData(readTextureData, 8*4*4, texture, &position, 8, 4));
	EXPECT_TRUE(dsTexture_destroy(texture));

	texture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyFrom, dsGfxMemory_Static,
		format, dsTextureDim_2D, 32, 16, 0, 3, textureData, sizeof(textureData));
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
	int commandBufferData;
	dsCommandBuffer* commandBuffer = (dsCommandBuffer*)&commandBufferData;

	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	dsTexture* texture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, format, dsTextureDim_2D, 32, 16, 0, 3, NULL, 0);
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
	dsTexturePosition position = {dsCubeFace_PosX, 3, 4, 0, 1};
	EXPECT_FALSE(dsTexture_copyData(commandBuffer, texture, &position, 8, 4, textureData, 8*4*4));
	EXPECT_TRUE(dsTexture_destroy(texture));

	texture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo | dsTextureUsage_CopyFrom,
		dsGfxMemory_Static, format, dsTextureDim_2D, 32, 16, 0, 3, NULL, 0);
	ASSERT_TRUE(texture);
	EXPECT_FALSE(dsTexture_copyData(NULL, texture, &position, 8, 4, textureData,
		sizeof(textureData)));
	EXPECT_FALSE(dsTexture_copyData(commandBuffer, texture, NULL, 8, 4, textureData,
		sizeof(textureData)));
	EXPECT_FALSE(dsTexture_copyData(commandBuffer, texture, &position, 8, 4, NULL,
		sizeof(textureData)));
	EXPECT_FALSE(dsTexture_copyData(commandBuffer, texture, &position, 8, 4, textureData,
		100));
	EXPECT_TRUE(dsTexture_copyData(commandBuffer, texture, &position, 8, 4, textureData,
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
	EXPECT_FALSE(dsTexture_copyData(commandBuffer, texture, &position, 8, 4, textureData,
		sizeof(textureData)));

	position.x = 3;
	position.y = 5;
	EXPECT_FALSE(dsTexture_copyData(commandBuffer, texture, &position, 8, 4, textureData,
		sizeof(textureData)));

	position.x = 0;
	position.y = 0;
	position.mipLevel = 5;
	EXPECT_FALSE(dsTexture_copyData(commandBuffer, texture, &position, 8, 4, textureData,
		sizeof(textureData)));

	position.mipLevel = 0;
	position.depth = 1;
	EXPECT_FALSE(dsTexture_copyData(commandBuffer, texture, &position, 8, 4, textureData,
		sizeof(textureData)));

	EXPECT_TRUE(dsTexture_destroy(texture));
}

TEST_F(TextureTest, Copy)
{
	int commandBufferData;
	dsCommandBuffer* commandBuffer = (dsCommandBuffer*)&commandBufferData;

	dsColor textureData[(32*16 + 16*8 + 8*4)*4];
	for (unsigned int level = 0, index = 0; level < 3; ++level)
	{
		unsigned int width = 32/(1 << level);
		unsigned int height = 16/(1 << level);
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
	dsTexture* fromTexture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, format, dsTextureDim_2D, 32, 16, 4, 3, textureData,
		sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	dsTexture* toTexture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo | dsTextureUsage_CopyFrom,
		dsGfxMemory_Static, format, dsTextureDim_2D, 16, 32, 5, 2, NULL, 0);
	ASSERT_TRUE(toTexture);

	dsTextureCopyRegion copyRegion =
	{
		{dsCubeFace_PosX, 1, 2, 2, 1},
		{dsCubeFace_PosX, 3, 4, 1, 0},
		8, 4, 2
	};

	EXPECT_FALSE(dsTexture_copy(commandBuffer, fromTexture, toTexture, &copyRegion, 1));
	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsTexture_destroy(toTexture));

	fromTexture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyFrom, dsGfxMemory_Static, format,
		dsTextureDim_2D, 32, 16, 4, 3, textureData, sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	toTexture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Static, format, dsTextureDim_2D, 16, 32, 5, 2, NULL, 0);
	ASSERT_TRUE(toTexture);

	EXPECT_FALSE(dsTexture_copy(commandBuffer, fromTexture, toTexture, &copyRegion, 1));
	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsTexture_destroy(toTexture));

	fromTexture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyFrom, dsGfxMemory_Static, format,
		dsTextureDim_2D, 32, 16, 4, 3, textureData, sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	toTexture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo | dsTextureUsage_CopyFrom,
		dsGfxMemory_Static, format, dsTextureDim_2D, 16, 32, 5, 2, NULL, 0);
	ASSERT_TRUE(toTexture);

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

TEST_F(TextureTest, Blit)
{
	int commandBufferData;
	dsCommandBuffer* commandBuffer = (dsCommandBuffer*)&commandBufferData;

	dsColor textureData[(32*16 + 16*8 + 8*4)*4];
	for (unsigned int level = 0, index = 0; level < 3; ++level)
	{
		unsigned int width = 32/(1 << level);
		unsigned int height = 16/(1 << level);
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
	dsTexture* fromTexture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, format, dsTextureDim_2D, 32, 16, 4, 3, textureData,
		sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	dsTexture* toTexture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo | dsTextureUsage_CopyFrom,
		dsGfxMemory_Static, format, dsTextureDim_2D, 16, 32, 5, 2, NULL, 0);
	ASSERT_TRUE(toTexture);

	dsTextureBlitRegion blitRegion =
	{
		{dsCubeFace_PosX, 1, 2, 2, 1},
		{dsCubeFace_PosX, 3, 4, 1, 0},
		8, 4, 2, 8, 4, 2
	};

	EXPECT_FALSE(dsTexture_blit(commandBuffer, fromTexture, toTexture, &blitRegion, 1,
		dsFilter_Nearest));
	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsTexture_destroy(toTexture));

	fromTexture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyFrom, dsGfxMemory_Static, format,
		dsTextureDim_2D, 32, 16, 4, 3, textureData, sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	toTexture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Static, format, dsTextureDim_2D, 16, 32, 5, 2, NULL, 0);
	ASSERT_TRUE(toTexture);

	EXPECT_FALSE(dsTexture_blit(commandBuffer, fromTexture, toTexture, &blitRegion, 1,
		dsFilter_Nearest));
	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsTexture_destroy(toTexture));

	fromTexture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyFrom, dsGfxMemory_Static, format,
		dsTextureDim_2D, 32, 16, 4, 3, textureData, sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	toTexture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo | dsTextureUsage_CopyFrom,
		dsGfxMemory_Static, format, dsTextureDim_2D, 16, 32, 5, 2, NULL, 0);
	ASSERT_TRUE(toTexture);

	EXPECT_TRUE(dsTexture_blit(commandBuffer, fromTexture, toTexture, &blitRegion, 1,
		dsFilter_Nearest));

	dsColor readTextureData[8*4];
	EXPECT_TRUE(dsTexture_getData(readTextureData, sizeof(readTextureData), toTexture,
		&blitRegion.dstPosition, 8, 4));
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

	blitRegion.dstPosition.depth = 2;
	EXPECT_TRUE(dsTexture_getData(readTextureData, sizeof(readTextureData), toTexture,
		&blitRegion.dstPosition, 8, 4));
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

	blitRegion.srcPosition.x = 25;
	EXPECT_FALSE(dsTexture_blit(commandBuffer, fromTexture, toTexture, &blitRegion, 1,
		dsFilter_Nearest));

	blitRegion.srcPosition.x = 1;
	blitRegion.srcPosition.y = 13;
	EXPECT_FALSE(dsTexture_blit(commandBuffer, fromTexture, toTexture, &blitRegion, 1,
		dsFilter_Nearest));

	blitRegion.srcPosition.x = 0;
	blitRegion.srcPosition.y = 0;
	blitRegion.srcPosition.mipLevel = 5;
	EXPECT_FALSE(dsTexture_blit(commandBuffer, fromTexture, toTexture, &blitRegion, 1,
		dsFilter_Nearest));

	blitRegion.srcPosition.mipLevel = 0;
	blitRegion.srcPosition.depth = 3;
	EXPECT_FALSE(dsTexture_blit(commandBuffer, fromTexture, toTexture, &blitRegion, 1,
		dsFilter_Nearest));

	blitRegion.srcPosition.depth = 0;
	blitRegion.dstPosition.x = 17;
	EXPECT_FALSE(dsTexture_blit(commandBuffer, fromTexture, toTexture, &blitRegion, 1,
		dsFilter_Nearest));

	blitRegion.dstPosition.x = 3;
	blitRegion.dstPosition.y = 29;
	EXPECT_FALSE(dsTexture_blit(commandBuffer, fromTexture, toTexture, &blitRegion, 1,
		dsFilter_Nearest));

	blitRegion.dstPosition.y = 4;
	blitRegion.dstPosition.mipLevel = 3;
	EXPECT_FALSE(dsTexture_blit(commandBuffer, fromTexture, toTexture, &blitRegion, 1,
		dsFilter_Nearest));

	blitRegion.dstPosition.mipLevel = 0;
	blitRegion.dstPosition.depth = 4;
	EXPECT_FALSE(dsTexture_blit(commandBuffer, fromTexture, toTexture, &blitRegion, 1,
		dsFilter_Nearest));

	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsTexture_destroy(toTexture));
}

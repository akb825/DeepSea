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

#include "Fixtures/AssetFixtureBase.h"
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Packing.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/TextureData.h>
#include <gtest/gtest.h>

extern char assetsDir[];

class TextureDataTest : public AssetFixtureBase
{
public:
	TextureDataTest()
		: AssetFixtureBase("textures")
	{
	}
};

static bool operator==(const dsColor& color1, const dsColor& color2)
{
	return color1.r == color2.r && color1.g == color2.g && color1.b == color2.b &&
		color1.a == color2.a;
}

#if !DS_ANDROID

struct Color16f
{
	dsHalfFloat r;
	dsHalfFloat g;
	dsHalfFloat b;
	dsHalfFloat a;
};

static bool operator==(const dsColor& color1, const Color16f& color2)
{
	dsVector4f color4f = {{dsUnpackHalfFloat(color2.r), dsUnpackHalfFloat(color2.g),
		dsUnpackHalfFloat(color2.b), dsUnpackHalfFloat(color2.a)}};
	return color1.r == round(color4f.r*255) && color1.g == round(color4f.g*255) &&
		color1.b == round(color4f.b*255) && color1.a == round(color4f.a*255);
}

static bool operator==(const dsColor& color1, uint16_t color2)
{
	dsVector3f color3f;
	dsUnpackUIntR5G6B5(&color3f, color2);
	return color1.r == round(color3f.r*255) && color1.g == round(color3f.g*255) &&
		color1.b == round(color3f.b*255) && color1.a == 255;
}

#endif

static bool noSrgbSupported(const dsResourceManager*, dsGfxFormat format)
{
	return (format & dsGfxFormat_DecoratorMask) != dsGfxFormat_SRGB;
}

TEST_F(TextureDataTest, Create)
{
	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	dsTextureInfo info = {format, dsTextureDim_2D, 2, 4, 5, 6, 1};
	EXPECT_FALSE(dsTextureData_create(NULL, &info));
	EXPECT_FALSE(dsTextureData_create((dsAllocator*)&allocator, NULL));
	info.format = dsGfxFormat_R8G8B8A8;
	EXPECT_FALSE(dsTextureData_create((dsAllocator*)&allocator, &info));
	info.format = format;
	info.samples = 4;
	EXPECT_FALSE(dsTextureData_create((dsAllocator*)&allocator, &info));
	info.samples = 1;

	dsTextureData* textureData = dsTextureData_create((dsAllocator*)&allocator, &info);
	ASSERT_TRUE(textureData);
	EXPECT_EQ(format, textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(2U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(5U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);
	EXPECT_EQ(dsTexture_size(&info), textureData->dataSize);

	dsTextureData_destroy(textureData);
}

#if !DS_ANDROID

TEST_F(TextureDataTest, LoadDDSFile_R8G8B8A8)
{
	EXPECT_FALSE(dsTextureData_loadDDSFile((dsAllocator*)&allocator, getPath("asdf")));
	EXPECT_FALSE(dsTextureData_loadDDSFile((dsAllocator*)&allocator, getPath("test.txt")));
	EXPECT_FALSE(dsTextureData_loadDDSFile((dsAllocator*)&allocator, getPath("empty.txt")));

	dsTextureData* textureData = dsTextureData_loadDDSFile((dsAllocator*)&allocator,
		getPath("texture.r8g8b8a8.dds"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(1U, textureData->info.mipLevels);

	ASSERT_EQ(4*4*sizeof(dsColor), textureData->dataSize);
	const dsColor* textureColors = (const dsColor*)textureData->data;
	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 128, 128, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{128, 255, 255, 255}}), textureColors[15]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadDDSStream_R8G8B8A8)
{
	dsFileStream fileStream;
	ASSERT_TRUE(dsFileStream_openPath(&fileStream, getPath("texture.r8g8b8a8.dds"), "rb"));
	EXPECT_FALSE(dsTextureData_loadDDSStream((dsAllocator*)&allocator, NULL));

	dsTextureData*textureData = dsTextureData_loadDDSStream((dsAllocator*)&allocator,
		(dsStream*)&fileStream);
	ASSERT_TRUE(textureData);
	EXPECT_TRUE(dsFileStream_close(&fileStream));

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(1U, textureData->info.mipLevels);

	ASSERT_EQ(4*4*sizeof(dsColor), textureData->dataSize);
	const dsColor* textureColors = (const dsColor*)textureData->data;
	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 128, 128, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{128, 255, 255, 255}}), textureColors[15]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadDDSFile_B8G8R8A8)
{
	dsTextureData* textureData = dsTextureData_loadDDSFile((dsAllocator*)&allocator,
		getPath("texture.b8g8r8a8.dds"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_UNorm),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);

	ASSERT_EQ((4*4 + 2*2 + 1)*sizeof(dsColor), textureData->dataSize);
	const dsColor* textureColors = (const dsColor*)textureData->data;
	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{128, 128, 255, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{255, 255, 128, 255}}), textureColors[15]);

	EXPECT_EQ((dsColor{{86, 124, 114, 255}}), textureColors[16]);
	EXPECT_EQ((dsColor{{159, 140, 134, 255}}), textureColors[17]);
	EXPECT_EQ((dsColor{{161, 92, 156, 255}}), textureColors[18]);
	EXPECT_EQ((dsColor{{147, 205, 146, 255}}), textureColors[19]);

	EXPECT_EQ((dsColor{{138, 140, 137, 255}}), textureColors[20]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadDDSFile_R16G16B16A16F)
{
	dsTextureData* textureData = dsTextureData_loadDDSFile((dsAllocator*)&allocator,
		getPath("texture.r16g16b16a16f.dds"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_Float),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);

	ASSERT_EQ((4*4 + 2*2 + 1)*sizeof(uint16_t)*4, textureData->dataSize);
	const Color16f* textureColors = (const Color16f*)textureData->data;
	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 128, 128, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{128, 255, 255, 255}}), textureColors[15]);

	EXPECT_EQ((dsColor{{114, 124, 86, 255}}), textureColors[16]);
	EXPECT_EQ((dsColor{{134, 140, 159, 255}}), textureColors[17]);
	EXPECT_EQ((dsColor{{156, 92, 161, 255}}), textureColors[18]);
	EXPECT_EQ((dsColor{{146, 205, 147, 255}}), textureColors[19]);

	EXPECT_EQ((dsColor{{137, 140, 138, 255}}), textureColors[20]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadDDSFile_R5G6B5)
{
	dsTextureData* textureData = dsTextureData_loadDDSFile((dsAllocator*)&allocator,
		getPath("texture.r5g6b5.dds"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R5G6B5, dsGfxFormat_UNorm),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);

	ASSERT_EQ((4*4 + 2*2 + 1)*sizeof(uint16_t), textureData->dataSize);
	const uint16_t* textureColors = (const uint16_t*)textureData->data;
	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{132, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 130, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 132, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{132, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 130, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 132, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 130, 132, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{132, 255, 255, 255}}), textureColors[15]);

	EXPECT_EQ((dsColor{{115, 125, 90, 255}}), textureColors[16]);
	EXPECT_EQ((dsColor{{132, 142, 156, 255}}), textureColors[17]);
	EXPECT_EQ((dsColor{{156, 93, 165, 255}}), textureColors[18]);
	EXPECT_EQ((dsColor{{148, 206, 148, 255}}), textureColors[19]);

	EXPECT_EQ((dsColor{{140, 142, 140, 255}}), textureColors[20]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadDDSFile_BC1SRGB)
{
	dsTextureData* textureData = dsTextureData_loadDDSFile((dsAllocator*)&allocator,
		getPath("texture.bc1srgb.dds"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_BC1_RGB, dsGfxFormat_SRGB),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadDDSFile_Array)
{
	dsTextureData* textureData = dsTextureData_loadDDSFile((dsAllocator*)&allocator,
		getPath("array.dds"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(2U, textureData->info.height);
	EXPECT_EQ(3U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);

	ASSERT_EQ((4*2 + 2 + 1)*3*sizeof(dsColor), textureData->dataSize);
	const dsColor* textureColors = (const dsColor*)textureData->data;
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[4*2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[4*2*2]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadDDSFile_Cube)
{
	dsTextureData* textureData = dsTextureData_loadDDSFile((dsAllocator*)&allocator,
		getPath("cube.dds"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_Cube, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);

	ASSERT_EQ((4*4 + 2*2 + 1)*6*sizeof(dsColor), textureData->dataSize);
	const dsColor* textureColors = (const dsColor*)textureData->data;
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[4*4]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[4*4*2]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[4*4*3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4*4*4]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[4*4*5]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadDDSFileToTexture)
{
	EXPECT_FALSE(dsTextureData_loadDDSFileToTexture(NULL, NULL, NULL,
		getPath("texture.r8g8b8a8.dds"), NULL, dsTextureUsage_Texture, dsGfxMemory_Read));
	EXPECT_FALSE(dsTextureData_loadDDSFileToTexture(resourceManager, NULL, NULL, NULL, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Read));
	EXPECT_FALSE(dsTextureData_loadDDSFileToTexture(resourceManager, NULL, NULL,
		getPath("texture.r8g8b8a8.dds"), NULL, (dsTextureUsage)0, (dsGfxMemory)0));

	dsTexture* texture = dsTextureData_loadDDSFileToTexture(resourceManager, NULL, NULL,
		getPath("texture.r8g8b8a8.dds"), NULL, dsTextureUsage_Texture | dsTextureUsage_CopyFrom,
		dsGfxMemory_Read);
	ASSERT_TRUE(texture);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		texture->info.format);
	EXPECT_EQ(dsTextureDim_2D, texture->info.dimension);
	EXPECT_EQ(4U, texture->info.width);
	EXPECT_EQ(4U, texture->info.height);
	EXPECT_EQ(0U, texture->info.depth);
	EXPECT_EQ(1U, texture->info.mipLevels);

	dsColor textureColors[4*4];
	dsTexturePosition position = {dsCubeFace_None, 0, 0, 0, 0};
	ASSERT_TRUE(dsTexture_getData(textureColors, sizeof(textureColors), texture, &position, 4, 4));

	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 128, 128, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{128, 255, 255, 255}}), textureColors[15]);

	EXPECT_TRUE(dsTexture_destroy(texture));
}

TEST_F(TextureDataTest, LoadDDSStreamToTexture)
{
	dsFileStream fileStream;
	ASSERT_TRUE(dsFileStream_openPath(&fileStream, getPath("texture.r8g8b8a8.dds"), "rb"));

	EXPECT_FALSE(dsTextureData_loadDDSStreamToTexture(NULL, NULL, NULL,
		(dsStream*)&fileStream, NULL, dsTextureUsage_Texture, dsGfxMemory_Read));
	EXPECT_FALSE(dsTextureData_loadDDSStreamToTexture(resourceManager, NULL, NULL, NULL, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Read));

	dsTexture* texture = dsTextureData_loadDDSStreamToTexture(resourceManager, NULL, NULL,
		(dsStream*)&fileStream, NULL, dsTextureUsage_Texture | dsTextureUsage_CopyFrom,
		dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_TRUE(dsFileStream_close(&fileStream));

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), texture->info.format);
	EXPECT_EQ(dsTextureDim_2D, texture->info.dimension);
	EXPECT_EQ(4U, texture->info.width);
	EXPECT_EQ(4U, texture->info.height);
	EXPECT_EQ(0U, texture->info.depth);
	EXPECT_EQ(1U, texture->info.mipLevels);

	dsColor textureColors[4*4];
	dsTexturePosition position = {dsCubeFace_None, 0, 0, 0, 0};
	ASSERT_TRUE(dsTexture_getData(textureColors, sizeof(textureColors), texture, &position, 4, 4));

	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 128, 128, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{128, 255, 255, 255}}), textureColors[15]);

	EXPECT_TRUE(dsTexture_destroy(texture));
}

TEST_F(TextureDataTest, LoadKTXFile_R8G8B8A8)
{
	EXPECT_FALSE(dsTextureData_loadKTXFile((dsAllocator*)&allocator, getPath("asdf")));
	EXPECT_FALSE(dsTextureData_loadKTXFile((dsAllocator*)&allocator, getPath("test.txt")));
	EXPECT_FALSE(dsTextureData_loadKTXFile((dsAllocator*)&allocator, getPath("empty.txt")));

	dsTextureData* textureData = dsTextureData_loadKTXFile((dsAllocator*)&allocator,
		getPath("texture.r8g8b8a8.ktx"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(1U, textureData->info.mipLevels);

	ASSERT_EQ(4*4*sizeof(dsColor), textureData->dataSize);
	const dsColor* textureColors = (const dsColor*)textureData->data;
	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 128, 128, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{128, 255, 255, 255}}), textureColors[15]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadKTXStream_R8G8B8A8)
{
	dsFileStream fileStream;
	ASSERT_TRUE(dsFileStream_openPath(&fileStream, getPath("texture.r8g8b8a8.ktx"), "rb"));
	EXPECT_FALSE(dsTextureData_loadKTXStream((dsAllocator*)&allocator, NULL));

	dsTextureData* textureData = dsTextureData_loadKTXStream((dsAllocator*)&allocator,
		(dsStream*)&fileStream);
	ASSERT_TRUE(textureData);
	EXPECT_TRUE(dsFileStream_close(&fileStream));

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(1U, textureData->info.mipLevels);

	ASSERT_EQ(4*4*sizeof(dsColor), textureData->dataSize);
	const dsColor* textureColors = (const dsColor*)textureData->data;
	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 128, 128, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{128, 255, 255, 255}}), textureColors[15]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadKTXFile_B8G8R8A8)
{
	dsTextureData* textureData = dsTextureData_loadKTXFile((dsAllocator*)&allocator,
		getPath("texture.b8g8r8a8.ktx"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_UNorm),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);

	ASSERT_EQ((4*4 + 2*2 + 1)*sizeof(dsColor), textureData->dataSize);
	const dsColor* textureColors = (const dsColor*)textureData->data;
	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{128, 128, 255, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{255, 255, 128, 255}}), textureColors[15]);

	EXPECT_EQ((dsColor{{86, 124, 114, 255}}), textureColors[16]);
	EXPECT_EQ((dsColor{{159, 140, 134, 255}}), textureColors[17]);
	EXPECT_EQ((dsColor{{161, 92, 156, 255}}), textureColors[18]);
	EXPECT_EQ((dsColor{{147, 205, 146, 255}}), textureColors[19]);

	EXPECT_EQ((dsColor{{138, 140, 137, 255}}), textureColors[20]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadKTXFile_R16G16B16A16F)
{
	dsTextureData* textureData = dsTextureData_loadKTXFile((dsAllocator*)&allocator,
		getPath("texture.r16g16b16a16f.ktx"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_Float),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);

	ASSERT_EQ((4*4 + 2*2 + 1)*sizeof(uint16_t)*4, textureData->dataSize);
	const Color16f* textureColors = (const Color16f*)textureData->data;
	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 128, 128, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{128, 255, 255, 255}}), textureColors[15]);

	EXPECT_EQ((dsColor{{114, 124, 86, 255}}), textureColors[16]);
	EXPECT_EQ((dsColor{{134, 140, 159, 255}}), textureColors[17]);
	EXPECT_EQ((dsColor{{156, 92, 161, 255}}), textureColors[18]);
	EXPECT_EQ((dsColor{{146, 205, 147, 255}}), textureColors[19]);

	EXPECT_EQ((dsColor{{137, 140, 138, 255}}), textureColors[20]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadKTXFile_R5G6B5)
{
	dsTextureData* textureData = dsTextureData_loadKTXFile((dsAllocator*)&allocator,
		getPath("texture.r5g6b5.ktx"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R5G6B5, dsGfxFormat_UNorm),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);

	ASSERT_EQ((4*4 + 2*2 + 1)*sizeof(uint16_t), textureData->dataSize);
	const uint16_t* textureColors = (const uint16_t*)textureData->data;
	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{132, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 130, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 132, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{132, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 130, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 132, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 130, 132, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{132, 255, 255, 255}}), textureColors[15]);

	EXPECT_EQ((dsColor{{115, 125, 90, 255}}), textureColors[16]);
	EXPECT_EQ((dsColor{{132, 142, 156, 255}}), textureColors[17]);
	EXPECT_EQ((dsColor{{156, 93, 165, 255}}), textureColors[18]);
	EXPECT_EQ((dsColor{{148, 206, 148, 255}}), textureColors[19]);

	EXPECT_EQ((dsColor{{140, 142, 140, 255}}), textureColors[20]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadKTXFile_BC1SRGB)
{
	dsTextureData* textureData = dsTextureData_loadKTXFile((dsAllocator*)&allocator,
		getPath("texture.bc1srgb.ktx"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_BC1_RGB, dsGfxFormat_SRGB),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadKTXFile_Array)
{
	dsTextureData* textureData = dsTextureData_loadKTXFile((dsAllocator*)&allocator,
		getPath("array.ktx"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(2U, textureData->info.height);
	EXPECT_EQ(3U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);

	ASSERT_EQ((4*2 + 2 + 1)*3*sizeof(dsColor), textureData->dataSize);
	const dsColor* textureColors = (const dsColor*)textureData->data;
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[4*2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[4*2*2]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadKTXFile_Cube)
{
	dsTextureData* textureData = dsTextureData_loadKTXFile((dsAllocator*)&allocator,
		getPath("cube.ktx"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_Cube, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);

	ASSERT_EQ((4*4 + 2*2 + 1)*6*sizeof(dsColor), textureData->dataSize);
	const dsColor* textureColors = (const dsColor*)textureData->data;
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[4*4]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[4*4*2]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[4*4*3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4*4*4]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[4*4*5]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadKTXFileToTexture)
{
	EXPECT_FALSE(dsTextureData_loadKTXFileToTexture(NULL, NULL, NULL,
		getPath("texture.r8g8b8a8.ktx"), NULL, dsTextureUsage_Texture, dsGfxMemory_Read));
	EXPECT_FALSE(dsTextureData_loadKTXFileToTexture(resourceManager, NULL, NULL, NULL, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Read));
	EXPECT_FALSE(dsTextureData_loadKTXFileToTexture(resourceManager, NULL, NULL,
		getPath("texture.r8g8b8a8.ktx"), NULL, (dsTextureUsage)0, (dsGfxMemory)0));

	dsTexture* texture = dsTextureData_loadKTXFileToTexture(resourceManager, NULL, NULL,
		getPath("texture.r8g8b8a8.ktx"), NULL, dsTextureUsage_Texture | dsTextureUsage_CopyFrom,
		dsGfxMemory_Read);
	ASSERT_TRUE(texture);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), texture->info.format);
	EXPECT_EQ(dsTextureDim_2D, texture->info.dimension);
	EXPECT_EQ(4U, texture->info.width);
	EXPECT_EQ(4U, texture->info.height);
	EXPECT_EQ(0U, texture->info.depth);
	EXPECT_EQ(1U, texture->info.mipLevels);

	dsColor textureColors[4*4];
	dsTexturePosition position = {dsCubeFace_None, 0, 0, 0, 0};
	ASSERT_TRUE(dsTexture_getData(textureColors, sizeof(textureColors), texture, &position, 4, 4));

	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 128, 128, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{128, 255, 255, 255}}), textureColors[15]);

	EXPECT_TRUE(dsTexture_destroy(texture));
}

TEST_F(TextureDataTest, LoadKTXStreamToTexture)
{
	dsFileStream fileStream;
	ASSERT_TRUE(dsFileStream_openPath(&fileStream, getPath("texture.r8g8b8a8.ktx"), "rb"));

	EXPECT_FALSE(dsTextureData_loadKTXStreamToTexture(NULL, NULL, NULL,
		(dsStream*)&fileStream, NULL, dsTextureUsage_Texture, dsGfxMemory_Read));
	EXPECT_FALSE(dsTextureData_loadKTXStreamToTexture(resourceManager, NULL, NULL, NULL, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Read));

	dsTexture* texture = dsTextureData_loadKTXStreamToTexture(resourceManager, NULL, NULL,
		(dsStream*)&fileStream, NULL, dsTextureUsage_Texture | dsTextureUsage_CopyFrom,
		dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_TRUE(dsFileStream_close(&fileStream));

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), texture->info.format);
	EXPECT_EQ(dsTextureDim_2D, texture->info.dimension);
	EXPECT_EQ(4U, texture->info.width);
	EXPECT_EQ(4U, texture->info.height);
	EXPECT_EQ(0U, texture->info.depth);
	EXPECT_EQ(1U, texture->info.mipLevels);

	dsColor textureColors[4*4];
	dsTexturePosition position = {dsCubeFace_None, 0, 0, 0, 0};
	ASSERT_TRUE(dsTexture_getData(textureColors, sizeof(textureColors), texture, &position, 4, 4));

	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 128, 128, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{128, 255, 255, 255}}), textureColors[15]);

	EXPECT_TRUE(dsTexture_destroy(texture));
}

TEST_F(TextureDataTest, LoadPVRFile_R8G8B8A8)
{
	EXPECT_FALSE(dsTextureData_loadPVRFile((dsAllocator*)&allocator, getPath("asdf")));
	EXPECT_FALSE(dsTextureData_loadPVRFile((dsAllocator*)&allocator, getPath("test.txt")));
	EXPECT_FALSE(dsTextureData_loadPVRFile((dsAllocator*)&allocator, getPath("empty.txt")));

	dsTextureData* textureData = dsTextureData_loadPVRFile((dsAllocator*)&allocator,
		getPath("texture.r8g8b8a8.pvr"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(1U, textureData->info.mipLevels);

	ASSERT_EQ(4*4*sizeof(dsColor), textureData->dataSize);
	const dsColor* textureColors = (const dsColor*)textureData->data;
	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 128, 128, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{128, 255, 255, 255}}), textureColors[15]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadPVRStream_R8G8B8A8)
{
	dsFileStream fileStream;
	ASSERT_TRUE(dsFileStream_openPath(&fileStream, getPath("texture.r8g8b8a8.pvr"), "rb"));
	EXPECT_FALSE(dsTextureData_loadPVRStream((dsAllocator*)&allocator, NULL));

	dsTextureData* textureData = dsTextureData_loadPVRStream((dsAllocator*)&allocator,
		(dsStream*)&fileStream);
	ASSERT_TRUE(textureData);
	EXPECT_TRUE(dsFileStream_close(&fileStream));

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(1U, textureData->info.mipLevels);

	ASSERT_EQ(4*4*sizeof(dsColor), textureData->dataSize);
	const dsColor* textureColors = (const dsColor*)textureData->data;
	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 128, 128, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{128, 255, 255, 255}}), textureColors[15]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadPVRFile_B8G8R8A8)
{
	dsTextureData* textureData = dsTextureData_loadPVRFile((dsAllocator*)&allocator,
		getPath("texture.b8g8r8a8.pvr"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_B8G8R8A8, dsGfxFormat_UNorm),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);

	ASSERT_EQ((4*4 + 2*2 + 1)*sizeof(dsColor), textureData->dataSize);
	const dsColor* textureColors = (const dsColor*)textureData->data;
	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{128, 128, 255, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{255, 255, 128, 255}}), textureColors[15]);

	EXPECT_EQ((dsColor{{86, 124, 114, 255}}), textureColors[16]);
	EXPECT_EQ((dsColor{{159, 140, 134, 255}}), textureColors[17]);
	EXPECT_EQ((dsColor{{161, 92, 156, 255}}), textureColors[18]);
	EXPECT_EQ((dsColor{{147, 205, 146, 255}}), textureColors[19]);

	EXPECT_EQ((dsColor{{138, 140, 137, 255}}), textureColors[20]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadPVRFile_R16G16B16A16F)
{
	dsTextureData* textureData = dsTextureData_loadPVRFile((dsAllocator*)&allocator,
		getPath("texture.r16g16b16a16f.pvr"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R16G16B16A16, dsGfxFormat_Float),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);

	ASSERT_EQ((4*4 + 2*2 + 1)*sizeof(uint16_t)*4, textureData->dataSize);
	const Color16f* textureColors = (const Color16f*)textureData->data;
	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 128, 128, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{128, 255, 255, 255}}), textureColors[15]);

	EXPECT_EQ((dsColor{{114, 124, 86, 255}}), textureColors[16]);
	EXPECT_EQ((dsColor{{134, 140, 159, 255}}), textureColors[17]);
	EXPECT_EQ((dsColor{{156, 92, 161, 255}}), textureColors[18]);
	EXPECT_EQ((dsColor{{146, 205, 147, 255}}), textureColors[19]);

	EXPECT_EQ((dsColor{{137, 140, 138, 255}}), textureColors[20]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadPVRFile_R5G6B5)
{
	dsTextureData* textureData = dsTextureData_loadPVRFile((dsAllocator*)&allocator,
		getPath("texture.r5g6b5.pvr"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R5G6B5, dsGfxFormat_UNorm),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);

	ASSERT_EQ((4*4 + 2*2 + 1)*sizeof(uint16_t), textureData->dataSize);
	const uint16_t* textureColors = (const uint16_t*)textureData->data;
	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{132, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 130, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 132, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{132, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 130, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 132, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 130, 132, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{132, 255, 255, 255}}), textureColors[15]);

	EXPECT_EQ((dsColor{{115, 125, 90, 255}}), textureColors[16]);
	EXPECT_EQ((dsColor{{132, 142, 156, 255}}), textureColors[17]);
	EXPECT_EQ((dsColor{{156, 93, 165, 255}}), textureColors[18]);
	EXPECT_EQ((dsColor{{148, 206, 148, 255}}), textureColors[19]);

	EXPECT_EQ((dsColor{{140, 142, 140, 255}}), textureColors[20]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadPVRFile_BC1SRGB)
{
	dsTextureData* textureData = dsTextureData_loadPVRFile((dsAllocator*)&allocator,
		getPath("texture.bc1srgb.pvr"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_BC1_RGB, dsGfxFormat_SRGB),
		textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadPVRFile_Array)
{
	dsTextureData* textureData = dsTextureData_loadPVRFile((dsAllocator*)&allocator,
		getPath("array.pvr"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), textureData->info.format);
	EXPECT_EQ(dsTextureDim_2D, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(2U, textureData->info.height);
	EXPECT_EQ(3U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);

	ASSERT_EQ((4*2 + 2 + 1)*3*sizeof(dsColor), textureData->dataSize);
	const dsColor* textureColors = (const dsColor*)textureData->data;
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[4*2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[4*2*2]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadPVRFile_Cube)
{
	dsTextureData* textureData = dsTextureData_loadPVRFile((dsAllocator*)&allocator,
		getPath("cube.pvr"));
	ASSERT_TRUE(textureData);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), textureData->info.format);
	EXPECT_EQ(dsTextureDim_Cube, textureData->info.dimension);
	EXPECT_EQ(4U, textureData->info.width);
	EXPECT_EQ(4U, textureData->info.height);
	EXPECT_EQ(0U, textureData->info.depth);
	EXPECT_EQ(3U, textureData->info.mipLevels);

	ASSERT_EQ((4*4 + 2*2 + 1)*6*sizeof(dsColor), textureData->dataSize);
	const dsColor* textureColors = (const dsColor*)textureData->data;
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[4*4]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[4*4*2]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[4*4*3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4*4*4]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[4*4*5]);

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, LoadPVRFileToTexture)
{
	EXPECT_FALSE(dsTextureData_loadPVRFileToTexture(NULL, NULL, NULL,
		getPath("texture.r8g8b8a8.pvr"), NULL, dsTextureUsage_Texture, dsGfxMemory_Read));
	EXPECT_FALSE(dsTextureData_loadPVRFileToTexture(resourceManager, NULL, NULL, NULL, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Read));
	EXPECT_FALSE(dsTextureData_loadPVRFileToTexture(resourceManager, NULL, NULL,
		getPath("texture.r8g8b8a8.pvr"), NULL, (dsTextureUsage)0, (dsGfxMemory)0));

	dsTexture* texture = dsTextureData_loadPVRFileToTexture(resourceManager, NULL, NULL,
		getPath("texture.r8g8b8a8.pvr"), NULL, dsTextureUsage_Texture | dsTextureUsage_CopyFrom,
		dsGfxMemory_Read);
	ASSERT_TRUE(texture);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), texture->info.format);
	EXPECT_EQ(dsTextureDim_2D, texture->info.dimension);
	EXPECT_EQ(4U, texture->info.width);
	EXPECT_EQ(4U, texture->info.height);
	EXPECT_EQ(0U, texture->info.depth);
	EXPECT_EQ(1U, texture->info.mipLevels);

	dsColor textureColors[4*4];
	dsTexturePosition position = {dsCubeFace_None, 0, 0, 0, 0};
	ASSERT_TRUE(dsTexture_getData(textureColors, sizeof(textureColors), texture, &position, 4, 4));

	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 128, 128, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{128, 255, 255, 255}}), textureColors[15]);

	EXPECT_TRUE(dsTexture_destroy(texture));
}

TEST_F(TextureDataTest, LoadPVRStreamToTexture)
{
	dsFileStream fileStream;
	ASSERT_TRUE(dsFileStream_openPath(&fileStream, getPath("texture.r8g8b8a8.pvr"), "rb"));

	EXPECT_FALSE(dsTextureData_loadPVRStreamToTexture(NULL, NULL, NULL,
		(dsStream*)&fileStream, NULL, dsTextureUsage_Texture, dsGfxMemory_Read));
	EXPECT_FALSE(dsTextureData_loadPVRStreamToTexture(resourceManager, NULL, NULL, NULL, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Read));

	dsTexture* texture = dsTextureData_loadPVRStreamToTexture(resourceManager, NULL, NULL,
		(dsStream*)&fileStream, NULL, dsTextureUsage_Texture | dsTextureUsage_CopyFrom,
		dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_TRUE(dsFileStream_close(&fileStream));

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), texture->info.format);
	EXPECT_EQ(dsTextureDim_2D, texture->info.dimension);
	EXPECT_EQ(4U, texture->info.width);
	EXPECT_EQ(4U, texture->info.height);
	EXPECT_EQ(0U, texture->info.depth);
	EXPECT_EQ(1U, texture->info.mipLevels);

	dsColor textureColors[4*4];
	dsTexturePosition position = {dsCubeFace_None, 0, 0, 0, 0};
	ASSERT_TRUE(dsTexture_getData(textureColors, sizeof(textureColors), texture, &position, 4, 4));

	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 128, 128, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{128, 255, 255, 255}}), textureColors[15]);

	EXPECT_TRUE(dsTexture_destroy(texture));
}

TEST_F(TextureDataTest, LoadFileToTexture)
{
	EXPECT_FALSE(dsTextureData_loadFileToTexture(resourceManager, NULL, NULL,
		getPath("test.txt"), NULL, dsTextureUsage_Texture | dsTextureUsage_CopyFrom,
		dsGfxMemory_Read));
	EXPECT_FALSE(dsTextureData_loadFileToTexture(NULL, NULL, NULL, getPath("texture.r8g8b8a8.dds"),
		NULL, dsTextureUsage_Texture, dsGfxMemory_Read));
	EXPECT_FALSE(dsTextureData_loadFileToTexture(resourceManager, NULL, NULL, NULL, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Read));
	EXPECT_FALSE(dsTextureData_loadFileToTexture(resourceManager, NULL, NULL,
		getPath("texture.r8g8b8a8.dds"), NULL, (dsTextureUsage)0, (dsGfxMemory)0));

	dsTexture* texture = dsTextureData_loadFileToTexture(resourceManager, NULL, NULL,
		getPath("texture.r8g8b8a8.dds"), NULL, dsTextureUsage_Texture | dsTextureUsage_CopyFrom,
		dsGfxMemory_Read);
	ASSERT_TRUE(texture);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), texture->info.format);
	EXPECT_EQ(dsTextureDim_2D, texture->info.dimension);
	EXPECT_EQ(4U, texture->info.width);
	EXPECT_EQ(4U, texture->info.height);
	EXPECT_EQ(0U, texture->info.depth);
	EXPECT_EQ(1U, texture->info.mipLevels);

	EXPECT_TRUE(dsTexture_destroy(texture));

	texture = dsTextureData_loadFileToTexture(resourceManager, NULL, NULL,
		getPath("texture.r8g8b8a8.ktx"), NULL, dsTextureUsage_Texture | dsTextureUsage_CopyFrom,
		dsGfxMemory_Read);
	ASSERT_TRUE(texture);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), texture->info.format);
	EXPECT_EQ(dsTextureDim_2D, texture->info.dimension);
	EXPECT_EQ(4U, texture->info.width);
	EXPECT_EQ(4U, texture->info.height);
	EXPECT_EQ(0U, texture->info.depth);
	EXPECT_EQ(1U, texture->info.mipLevels);

	EXPECT_TRUE(dsTexture_destroy(texture));

	texture = dsTextureData_loadFileToTexture(resourceManager, NULL, NULL,
		getPath("texture.r8g8b8a8.pvr"), NULL,
		(dsTextureUsage)(dsTextureUsage_Texture | dsTextureUsage_CopyFrom), dsGfxMemory_Read);
	ASSERT_TRUE(texture);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), texture->info.format);
	EXPECT_EQ(dsTextureDim_2D, texture->info.dimension);
	EXPECT_EQ(4U, texture->info.width);
	EXPECT_EQ(4U, texture->info.height);
	EXPECT_EQ(0U, texture->info.depth);
	EXPECT_EQ(1U, texture->info.mipLevels);

	EXPECT_TRUE(dsTexture_destroy(texture));
}

TEST_F(TextureDataTest, LoadStreamToTexture)
{
	dsFileStream fileStream;
	ASSERT_TRUE(dsFileStream_openPath(&fileStream, getPath("test.txt"), "rb"));
	EXPECT_FALSE(dsTextureData_loadStreamToTexture(resourceManager, NULL, NULL,
		(dsStream*)&fileStream, NULL, dsTextureUsage_Texture | dsTextureUsage_CopyFrom,
		dsGfxMemory_Read));
	EXPECT_TRUE(dsFileStream_close(&fileStream));

	ASSERT_TRUE(dsFileStream_openPath(&fileStream, getPath("texture.r8g8b8a8.dds"), "rb"));
	EXPECT_FALSE(dsTextureData_loadStreamToTexture(NULL, NULL, NULL, (dsStream*)&fileStream, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Read));
	EXPECT_TRUE(dsFileStream_close(&fileStream));

	EXPECT_FALSE(dsTextureData_loadStreamToTexture(resourceManager, NULL, NULL, NULL, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Read));

	ASSERT_TRUE(dsFileStream_openPath(&fileStream, getPath("texture.r8g8b8a8.dds"), "rb"));
	EXPECT_FALSE(dsTextureData_loadStreamToTexture(resourceManager, NULL, NULL,
		(dsStream*)&fileStream, NULL, (dsTextureUsage)0, (dsGfxMemory)0));
	EXPECT_TRUE(dsFileStream_close(&fileStream));

	ASSERT_TRUE(dsFileStream_openPath(&fileStream, getPath("texture.r8g8b8a8.dds"), "rb"));
	dsTexture* texture = dsTextureData_loadStreamToTexture(resourceManager, NULL, NULL,
		(dsStream*)&fileStream, NULL, dsTextureUsage_Texture | dsTextureUsage_CopyFrom,
		dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_TRUE(dsFileStream_close(&fileStream));

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), texture->info.format);
	EXPECT_EQ(dsTextureDim_2D, texture->info.dimension);
	EXPECT_EQ(4U, texture->info.width);
	EXPECT_EQ(4U, texture->info.height);
	EXPECT_EQ(0U, texture->info.depth);
	EXPECT_EQ(1U, texture->info.mipLevels);

	EXPECT_TRUE(dsTexture_destroy(texture));

	ASSERT_TRUE(dsFileStream_openPath(&fileStream, getPath("texture.r8g8b8a8.ktx"), "rb"));
	texture = dsTextureData_loadStreamToTexture(resourceManager, NULL, NULL, (dsStream*)&fileStream,
		NULL, dsTextureUsage_Texture | dsTextureUsage_CopyFrom, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_TRUE(dsFileStream_close(&fileStream));

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), texture->info.format);
	EXPECT_EQ(dsTextureDim_2D, texture->info.dimension);
	EXPECT_EQ(4U, texture->info.width);
	EXPECT_EQ(4U, texture->info.height);
	EXPECT_EQ(0U, texture->info.depth);
	EXPECT_EQ(1U, texture->info.mipLevels);

	EXPECT_TRUE(dsTexture_destroy(texture));

	ASSERT_TRUE(dsFileStream_openPath(&fileStream, getPath("texture.r8g8b8a8.pvr"), "rb"));
	texture = dsTextureData_loadStreamToTexture(resourceManager, NULL, NULL, (dsStream*)&fileStream,
		NULL, dsTextureUsage_Texture | dsTextureUsage_CopyFrom, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_TRUE(dsFileStream_close(&fileStream));

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), texture->info.format);
	EXPECT_EQ(dsTextureDim_2D, texture->info.dimension);
	EXPECT_EQ(4U, texture->info.width);
	EXPECT_EQ(4U, texture->info.height);
	EXPECT_EQ(0U, texture->info.depth);
	EXPECT_EQ(1U, texture->info.mipLevels);

	EXPECT_TRUE(dsTexture_destroy(texture));
}

#endif

TEST_F(TextureDataTest, LoadResourceToTexture)
{
	EXPECT_FALSE(dsTextureData_loadResourceToTexture(resourceManager, NULL, NULL,
		dsFileResourceType_Embedded, getRelativePath("test.txt"), NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyFrom, dsGfxMemory_Read));
	EXPECT_FALSE(dsTextureData_loadResourceToTexture(NULL, NULL, NULL, dsFileResourceType_Embedded,
		getRelativePath("texture.r8g8b8a8.dds"), NULL, dsTextureUsage_Texture, dsGfxMemory_Read));
	EXPECT_FALSE(dsTextureData_loadResourceToTexture(resourceManager, NULL, NULL,
		dsFileResourceType_Embedded, NULL, NULL, dsTextureUsage_Texture, dsGfxMemory_Read));
	EXPECT_FALSE(dsTextureData_loadResourceToTexture(resourceManager, NULL, NULL,
		dsFileResourceType_Embedded, getRelativePath("texture.r8g8b8a8.dds"), NULL,
		(dsTextureUsage)0, (dsGfxMemory)0));

	dsTexture* texture = dsTextureData_loadResourceToTexture(resourceManager, NULL, NULL,
		dsFileResourceType_Embedded, getRelativePath("texture.r8g8b8a8.dds"), NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyFrom, dsGfxMemory_Read);
	ASSERT_TRUE(texture);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), texture->info.format);
	EXPECT_EQ(dsTextureDim_2D, texture->info.dimension);
	EXPECT_EQ(4U, texture->info.width);
	EXPECT_EQ(4U, texture->info.height);
	EXPECT_EQ(0U, texture->info.depth);
	EXPECT_EQ(1U, texture->info.mipLevels);

	EXPECT_TRUE(dsTexture_destroy(texture));

	texture = dsTextureData_loadResourceToTexture(resourceManager, NULL, NULL,
		dsFileResourceType_Embedded, getRelativePath("texture.r8g8b8a8.ktx"), NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyFrom, dsGfxMemory_Read);
	ASSERT_TRUE(texture);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), texture->info.format);
	EXPECT_EQ(dsTextureDim_2D, texture->info.dimension);
	EXPECT_EQ(4U, texture->info.width);
	EXPECT_EQ(4U, texture->info.height);
	EXPECT_EQ(0U, texture->info.depth);
	EXPECT_EQ(1U, texture->info.mipLevels);

	EXPECT_TRUE(dsTexture_destroy(texture));

	texture = dsTextureData_loadResourceToTexture(resourceManager, NULL, NULL,
		dsFileResourceType_Embedded, getRelativePath("texture.r8g8b8a8.pvr"), NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyFrom, dsGfxMemory_Read);
	ASSERT_TRUE(texture);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), texture->info.format);
	EXPECT_EQ(dsTextureDim_2D, texture->info.dimension);
	EXPECT_EQ(4U, texture->info.width);
	EXPECT_EQ(4U, texture->info.height);
	EXPECT_EQ(0U, texture->info.depth);
	EXPECT_EQ(1U, texture->info.mipLevels);

	EXPECT_TRUE(dsTexture_destroy(texture));
}

TEST_F(TextureDataTest, CreateTexture)
{
	dsTextureData* textureData = dsTextureData_loadPVRResource((dsAllocator*)&allocator,
		dsFileResourceType_Embedded, getRelativePath("texture.r8g8b8a8.pvr"));
	ASSERT_TRUE(textureData);

	EXPECT_FALSE(dsTextureData_createTexture(NULL, NULL, textureData, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Read));
	EXPECT_FALSE(dsTextureData_createTexture(resourceManager, NULL, NULL, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Read));
	EXPECT_FALSE(dsTextureData_createTexture(resourceManager, NULL, textureData, NULL,
		(dsTextureUsage)0, (dsGfxMemory)0));

	dsTexture* texture = dsTextureData_createTexture(resourceManager, NULL, textureData, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyFrom, dsGfxMemory_Read);
	ASSERT_TRUE(texture);

	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), texture->info.format);
	EXPECT_EQ(dsTextureDim_2D, texture->info.dimension);
	EXPECT_EQ(4U, texture->info.width);
	EXPECT_EQ(4U, texture->info.height);
	EXPECT_EQ(0U, texture->info.depth);
	EXPECT_EQ(1U, texture->info.mipLevels);

	dsColor textureColors[4*4];
	dsTexturePosition position = {dsCubeFace_None, 0, 0, 0, 0};
	ASSERT_TRUE(dsTexture_getData(textureColors, sizeof(textureColors), texture, &position, 4, 4));

	EXPECT_EQ((dsColor{{0, 0, 0, 255}}), textureColors[0]);
	EXPECT_EQ((dsColor{{255, 0, 0, 255}}), textureColors[1]);
	EXPECT_EQ((dsColor{{0, 255, 0, 255}}), textureColors[2]);
	EXPECT_EQ((dsColor{{0, 0, 255, 255}}), textureColors[3]);
	EXPECT_EQ((dsColor{{0, 255, 255, 255}}), textureColors[4]);
	EXPECT_EQ((dsColor{{255, 255, 0, 255}}), textureColors[5]);
	EXPECT_EQ((dsColor{{255, 0, 255, 255}}), textureColors[6]);
	EXPECT_EQ((dsColor{{255, 255, 255, 255}}), textureColors[7]);
	EXPECT_EQ((dsColor{{128, 0, 255, 255}}), textureColors[8]);
	EXPECT_EQ((dsColor{{0, 128, 255, 255}}), textureColors[9]);
	EXPECT_EQ((dsColor{{0, 255, 128, 255}}), textureColors[10]);
	EXPECT_EQ((dsColor{{128, 255, 0, 255}}), textureColors[11]);
	EXPECT_EQ((dsColor{{255, 128, 0, 255}}), textureColors[12]);
	EXPECT_EQ((dsColor{{255, 0, 128, 255}}), textureColors[13]);
	EXPECT_EQ((dsColor{{255, 128, 128, 255}}), textureColors[14]);
	EXPECT_EQ((dsColor{{128, 255, 255, 255}}), textureColors[15]);

	EXPECT_TRUE(dsTexture_destroy(texture));
	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, sRGBFallback)
{
	dsTextureInfo info = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_SRGB),
		dsTextureDim_2D, 1024, 1024, 0, 0, 0};
	dsTextureData* textureData = dsTextureData_create((dsAllocator*)&allocator, &info);
	ASSERT_TRUE(textureData);

	dsTextureDataOptions options = {0, 0, 0, true};
	dsTexture* texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(textureData->info.format, texture->info.format);
	EXPECT_TRUE(dsTexture_destroy(texture));

	resourceManager->textureFormatSupportedFunc = &noSrgbSupported;
	EXPECT_FALSE(dsTextureData_createTexture(resourceManager, NULL, textureData, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Read));

	texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), texture->info.format);
	EXPECT_TRUE(dsTexture_destroy(texture));

	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, SkipLevels)
{
	dsTextureInfo info = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		dsTextureDim_2D, 1024, 512, 0, 0, 0};
	dsTextureData* textureData = dsTextureData_create((dsAllocator*)&allocator, &info);
	ASSERT_TRUE(textureData);

	dsTextureDataOptions options = {100, 0, 0, false};
	dsTexture* texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, texture->info.mipLevels);
	EXPECT_EQ(1024U, texture->info.width);
	EXPECT_EQ(512U, texture->info.height);

	EXPECT_TRUE(dsTexture_destroy(texture));
	dsTextureData_destroy(textureData);

	info.depth = 5;
	info.mipLevels = DS_ALL_MIP_LEVELS;
	textureData = dsTextureData_create((dsAllocator*)&allocator, &info);
	ASSERT_TRUE(textureData);

	texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, texture->info.mipLevels);
	EXPECT_EQ(1U, texture->info.width);
	EXPECT_EQ(1U, texture->info.height);
	EXPECT_EQ(5U, texture->info.depth);

	EXPECT_TRUE(dsTexture_destroy(texture));

	options.skipLevels = 3;
	texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(8U, texture->info.mipLevels);
	EXPECT_EQ(128U, texture->info.width);
	EXPECT_EQ(64U, texture->info.height);
	EXPECT_EQ(5U, texture->info.depth);

	EXPECT_TRUE(dsTexture_destroy(texture));
	dsTextureData_destroy(textureData);

	info.dimension = dsTextureDim_3D;
	info.depth = 128;
	textureData = dsTextureData_create((dsAllocator*)&allocator, &info);
	ASSERT_TRUE(textureData);

	options.skipLevels = 100;
	texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, texture->info.mipLevels);
	EXPECT_EQ(1U, texture->info.width);
	EXPECT_EQ(1U, texture->info.height);
	EXPECT_EQ(1U, texture->info.depth);

	EXPECT_TRUE(dsTexture_destroy(texture));

	options.skipLevels = 3;
	texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(8U, texture->info.mipLevels);
	EXPECT_EQ(128U, texture->info.width);
	EXPECT_EQ(64U, texture->info.height);
	EXPECT_EQ(16U, texture->info.depth);

	EXPECT_TRUE(dsTexture_destroy(texture));
	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, TargetHeight)
{
	dsTextureInfo info = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		dsTextureDim_2D, 1024, 512, 0, 0, 0};
	dsTextureData* textureData = dsTextureData_create((dsAllocator*)&allocator, &info);
	ASSERT_TRUE(textureData);

	dsTextureDataOptions options = {100, 1, 0, false};
	dsTexture* texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, texture->info.mipLevels);
	EXPECT_EQ(1024U, texture->info.width);
	EXPECT_EQ(512U, texture->info.height);

	EXPECT_TRUE(dsTexture_destroy(texture));
	dsTextureData_destroy(textureData);

	info.depth = 5;
	info.mipLevels = DS_ALL_MIP_LEVELS;
	textureData = dsTextureData_create((dsAllocator*)&allocator, &info);
	ASSERT_TRUE(textureData);

	texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(2U, texture->info.mipLevels);
	EXPECT_EQ(2U, texture->info.width);
	EXPECT_EQ(1U, texture->info.height);
	EXPECT_EQ(5U, texture->info.depth);

	EXPECT_TRUE(dsTexture_destroy(texture));

	options.targetHeight = 70;
	texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(8U, texture->info.mipLevels);
	EXPECT_EQ(128U, texture->info.width);
	EXPECT_EQ(64U, texture->info.height);
	EXPECT_EQ(5U, texture->info.depth);

	EXPECT_TRUE(dsTexture_destroy(texture));

	options.targetHeight = 50;
	texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(8U, texture->info.mipLevels);
	EXPECT_EQ(128U, texture->info.width);
	EXPECT_EQ(64U, texture->info.height);
	EXPECT_EQ(5U, texture->info.depth);

	EXPECT_TRUE(dsTexture_destroy(texture));
	dsTextureData_destroy(textureData);

	info.dimension = dsTextureDim_3D;
	info.depth = 128;
	textureData = dsTextureData_create((dsAllocator*)&allocator, &info);
	ASSERT_TRUE(textureData);

	options.targetHeight = 1;
	texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(2U, texture->info.mipLevels);
	EXPECT_EQ(2U, texture->info.width);
	EXPECT_EQ(1U, texture->info.height);
	EXPECT_EQ(1U, texture->info.depth);

	EXPECT_TRUE(dsTexture_destroy(texture));

	options.targetHeight = 70;
	texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(8U, texture->info.mipLevels);
	EXPECT_EQ(128U, texture->info.width);
	EXPECT_EQ(64U, texture->info.height);
	EXPECT_EQ(16U, texture->info.depth);

	EXPECT_TRUE(dsTexture_destroy(texture));
	dsTextureData_destroy(textureData);
}

TEST_F(TextureDataTest, TargetWidth)
{
	dsTextureInfo info = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		dsTextureDim_2D, 1024, 512, 0, 0, 0};
	dsTextureData* textureData = dsTextureData_create((dsAllocator*)&allocator, &info);
	ASSERT_TRUE(textureData);

	dsTextureDataOptions options = {100, 1024, 1, false};
	dsTexture* texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, texture->info.mipLevels);
	EXPECT_EQ(1024U, texture->info.width);
	EXPECT_EQ(512U, texture->info.height);

	EXPECT_TRUE(dsTexture_destroy(texture));
	dsTextureData_destroy(textureData);

	info.depth = 5;
	info.mipLevels = DS_ALL_MIP_LEVELS;
	textureData = dsTextureData_create((dsAllocator*)&allocator, &info);
	ASSERT_TRUE(textureData);

	texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, texture->info.mipLevels);
	EXPECT_EQ(1U, texture->info.width);
	EXPECT_EQ(1U, texture->info.height);
	EXPECT_EQ(5U, texture->info.depth);

	EXPECT_TRUE(dsTexture_destroy(texture));

	options.targetWidth = 140;
	texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(8U, texture->info.mipLevels);
	EXPECT_EQ(128U, texture->info.width);
	EXPECT_EQ(64U, texture->info.height);
	EXPECT_EQ(5U, texture->info.depth);

	EXPECT_TRUE(dsTexture_destroy(texture));

	options.targetWidth = 100;
	texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(8U, texture->info.mipLevels);
	EXPECT_EQ(128U, texture->info.width);
	EXPECT_EQ(64U, texture->info.height);
	EXPECT_EQ(5U, texture->info.depth);

	EXPECT_TRUE(dsTexture_destroy(texture));
	dsTextureData_destroy(textureData);

	info.dimension = dsTextureDim_3D;
	info.depth = 128;
	textureData = dsTextureData_create((dsAllocator*)&allocator, &info);
	ASSERT_TRUE(textureData);

	options.targetWidth = 1;
	texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(1U, texture->info.mipLevels);
	EXPECT_EQ(1U, texture->info.width);
	EXPECT_EQ(1U, texture->info.height);
	EXPECT_EQ(1U, texture->info.depth);

	EXPECT_TRUE(dsTexture_destroy(texture));

	options.targetWidth = 140;
	texture = dsTextureData_createTexture(resourceManager, NULL, textureData, &options,
		dsTextureUsage_Texture, dsGfxMemory_Read);
	ASSERT_TRUE(texture);
	EXPECT_EQ(8U, texture->info.mipLevels);
	EXPECT_EQ(128U, texture->info.width);
	EXPECT_EQ(64U, texture->info.height);
	EXPECT_EQ(16U, texture->info.depth);

	EXPECT_TRUE(dsTexture_destroy(texture));
	dsTextureData_destroy(textureData);
}

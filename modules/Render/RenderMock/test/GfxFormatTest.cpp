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

#include "Fixtures/FixtureBase.h"
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <gtest/gtest.h>

class GfxFormatTest : public FixtureBase
{
};

TEST_F(GfxFormatTest, IsValid)
{
	EXPECT_FALSE(dsGfxFormat_isValid(dsGfxFormat_R8G8B8A8));
	EXPECT_TRUE(dsGfxFormat_isValid(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
		dsGfxFormat_Float)));

	EXPECT_TRUE(dsGfxFormat_isValid(dsGfxFormat_D16));
	EXPECT_FALSE(dsGfxFormat_isValid(dsGfxFormat_decorate(dsGfxFormat_D16, dsGfxFormat_Float)));

	EXPECT_FALSE(dsGfxFormat_isValid(dsGfxFormat_ETC1));
	EXPECT_TRUE(dsGfxFormat_isValid(dsGfxFormat_decorate(dsGfxFormat_ETC1, dsGfxFormat_UNorm)));

	EXPECT_FALSE(dsGfxFormat_isValid((dsGfxFormat)(dsGfxFormat_R8G8B8A8 | dsGfxFormat_D16 |
		dsGfxFormat_UNorm)));
	EXPECT_FALSE(dsGfxFormat_isValid((dsGfxFormat)(dsGfxFormat_R8G8B8A8 | dsGfxFormat_ETC1 |
		dsGfxFormat_UNorm)));
	EXPECT_FALSE(dsGfxFormat_isValid((dsGfxFormat)(dsGfxFormat_D16 | dsGfxFormat_ETC1 |
		dsGfxFormat_UNorm)));
}

TEST_F(GfxFormatTest, Indices)
{
	EXPECT_EQ(6U, dsGfxFormat_standardIndex(dsGfxFormat_decorate(dsGfxFormat_B5G6R5,
		dsGfxFormat_SInt)));
	EXPECT_EQ(0U, dsGfxFormat_standardIndex(dsGfxFormat_D16));
	EXPECT_EQ(dsGfxFormat_B5G6R5, dsGfxFormat_standardEnum(6));
	EXPECT_EQ(dsGfxFormat_Unknown, dsGfxFormat_standardEnum(dsGfxFormat_StandardCount));

	EXPECT_EQ(5U, dsGfxFormat_specialIndex(dsGfxFormat_D32_Float));
	EXPECT_EQ(0U, dsGfxFormat_specialIndex(dsGfxFormat_B5G6R5));
	EXPECT_EQ(dsGfxFormat_D32_Float, dsGfxFormat_specialEnum(5));
	EXPECT_EQ(dsGfxFormat_Unknown, dsGfxFormat_specialEnum(dsGfxFormat_SpecialCount));

	EXPECT_EQ(5U, dsGfxFormat_compressedIndex(dsGfxFormat_BC4));
	EXPECT_EQ(0U, dsGfxFormat_compressedIndex(dsGfxFormat_B5G6R5));
	EXPECT_EQ(dsGfxFormat_BC4, dsGfxFormat_compressedEnum(5));
	EXPECT_EQ(dsGfxFormat_Unknown, dsGfxFormat_compressedEnum(dsGfxFormat_CompressedCount));

	EXPECT_EQ(5U, dsGfxFormat_decoratorIndex(dsGfxFormat_UInt));
	EXPECT_EQ(0U, dsGfxFormat_decoratorIndex(dsGfxFormat_B5G6R5));
	EXPECT_EQ(dsGfxFormat_UInt, dsGfxFormat_decoratorEnum(5));
	EXPECT_EQ(dsGfxFormat_Unknown, dsGfxFormat_decoratorEnum(dsGfxFormat_DecoratorCount));
}

TEST_F(GfxFormatTest, Size)
{
	EXPECT_EQ(0U, dsGfxFormat_size((dsGfxFormat)(dsGfxFormat_R8G8B8A8 | dsGfxFormat_D16 |
		dsGfxFormat_UNorm)));
	EXPECT_EQ(16U, dsGfxFormat_size(dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32W32,
		dsGfxFormat_Float)));
	EXPECT_EQ(4U, dsGfxFormat_size(dsGfxFormat_D24S8));
	EXPECT_EQ(16U, dsGfxFormat_size(dsGfxFormat_decorate(dsGfxFormat_BC3, dsGfxFormat_SNorm)));
}

TEST_F(GfxFormatTest, BlockDimensions)
{
	unsigned int x, y;
	EXPECT_FALSE(dsGfxFormat_blockDimensions(&x, &y, (dsGfxFormat)(dsGfxFormat_R8G8B8A8 |
		dsGfxFormat_D16 | dsGfxFormat_UNorm)));
	EXPECT_FALSE(dsGfxFormat_blockDimensions(NULL, NULL, dsGfxFormat_decorate(
		dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float)));

	EXPECT_TRUE(dsGfxFormat_blockDimensions(&x, &y, dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32W32,
		dsGfxFormat_Float)));
	EXPECT_EQ(1U, x);
	EXPECT_EQ(1U, y);

	EXPECT_TRUE(dsGfxFormat_blockDimensions(&x, &y, dsGfxFormat_D24S8));
	EXPECT_EQ(1U, x);
	EXPECT_EQ(1U, y);

	EXPECT_TRUE(dsGfxFormat_blockDimensions(&x, &y, dsGfxFormat_decorate(dsGfxFormat_BC3,
		dsGfxFormat_SNorm)));
	EXPECT_EQ(4U, x);
	EXPECT_EQ(4U, y);

	EXPECT_TRUE(dsGfxFormat_blockDimensions(&x, &y, dsGfxFormat_decorate(dsGfxFormat_ASTC_8x5,
		dsGfxFormat_SNorm)));
	EXPECT_EQ(8U, x);
	EXPECT_EQ(5U, y);
}

TEST_F(GfxFormatTest, MinDimensions)
{
	unsigned int x, y;
	EXPECT_FALSE(dsGfxFormat_minDimensions(&x, &y, (dsGfxFormat)(dsGfxFormat_R8G8B8A8 |
		dsGfxFormat_D16 | dsGfxFormat_UNorm)));
	EXPECT_FALSE(dsGfxFormat_minDimensions(NULL, NULL, dsGfxFormat_decorate(
		dsGfxFormat_X32Y32Z32W32, dsGfxFormat_Float)));

	EXPECT_TRUE(dsGfxFormat_minDimensions(&x, &y, dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32W32,
		dsGfxFormat_Float)));
	EXPECT_EQ(1U, x);
	EXPECT_EQ(1U, y);

	EXPECT_TRUE(dsGfxFormat_minDimensions(&x, &y, dsGfxFormat_D24S8));
	EXPECT_EQ(1U, x);
	EXPECT_EQ(1U, y);

	EXPECT_TRUE(dsGfxFormat_minDimensions(&x, &y, dsGfxFormat_decorate(dsGfxFormat_BC3,
		dsGfxFormat_SNorm)));
	EXPECT_EQ(4U, x);
	EXPECT_EQ(4U, y);

	EXPECT_TRUE(dsGfxFormat_minDimensions(&x, &y, dsGfxFormat_decorate(dsGfxFormat_ASTC_8x5,
		dsGfxFormat_SNorm)));
	EXPECT_EQ(8U, x);
	EXPECT_EQ(5U, y);
}

TEST_F(GfxFormatTest, VertexSupported)
{
	EXPECT_FALSE(dsGfxFormat_vertexSupported(nullptr, dsGfxFormat_X32));
	EXPECT_FALSE(dsGfxFormat_vertexSupported(resourceManager, dsGfxFormat_X32));
	EXPECT_TRUE(dsGfxFormat_vertexSupported(resourceManager, dsGfxFormat_decorate(dsGfxFormat_X32,
		dsGfxFormat_Float)));
	EXPECT_FALSE(dsGfxFormat_vertexSupported(resourceManager, dsGfxFormat_D16));
	EXPECT_FALSE(dsGfxFormat_vertexSupported(resourceManager, dsGfxFormat_BC3));
}

TEST_F(GfxFormatTest, TextureSupported)
{
	EXPECT_FALSE(dsGfxFormat_textureSupported(nullptr, dsGfxFormat_X32));
	EXPECT_FALSE(dsGfxFormat_textureSupported(resourceManager, dsGfxFormat_X32));
	EXPECT_TRUE(dsGfxFormat_textureSupported(resourceManager, dsGfxFormat_decorate(dsGfxFormat_X32,
		dsGfxFormat_Float)));
	EXPECT_TRUE(dsGfxFormat_textureSupported(resourceManager, dsGfxFormat_D16));
	EXPECT_TRUE(dsGfxFormat_textureSupported(resourceManager, dsGfxFormat_decorate(dsGfxFormat_BC3,
		dsGfxFormat_UNorm)));
}

TEST_F(GfxFormatTest, TextureBufferSupported)
{
	EXPECT_FALSE(dsGfxFormat_textureBufferSupported(nullptr, dsGfxFormat_X32));
	EXPECT_FALSE(dsGfxFormat_textureBufferSupported(resourceManager, dsGfxFormat_X32));
	EXPECT_TRUE(dsGfxFormat_textureBufferSupported(resourceManager,
		dsGfxFormat_decorate(dsGfxFormat_X32, dsGfxFormat_Float)));
	EXPECT_FALSE(dsGfxFormat_textureBufferSupported(resourceManager, dsGfxFormat_D16));
	EXPECT_FALSE(dsGfxFormat_textureBufferSupported(resourceManager,
		dsGfxFormat_decorate(dsGfxFormat_BC3, dsGfxFormat_UNorm)));
}

TEST_F(GfxFormatTest, ImageSupported)
{
	EXPECT_FALSE(dsGfxFormat_imageSupported(nullptr, dsGfxFormat_X32));
	EXPECT_FALSE(dsGfxFormat_imageSupported(resourceManager, dsGfxFormat_X32));
	EXPECT_TRUE(dsGfxFormat_imageSupported(resourceManager,
		dsGfxFormat_decorate(dsGfxFormat_X32, dsGfxFormat_Float)));
	EXPECT_TRUE(dsGfxFormat_imageSupported(resourceManager, dsGfxFormat_B10G11R11_UFloat));
	EXPECT_FALSE(dsGfxFormat_imageSupported(resourceManager, dsGfxFormat_D16));
	EXPECT_FALSE(dsGfxFormat_imageSupported(resourceManager,
		dsGfxFormat_decorate(dsGfxFormat_BC3, dsGfxFormat_UNorm)));
}

TEST_F(GfxFormatTest, RenderTargetSupported)
{
	EXPECT_FALSE(dsGfxFormat_renderTargetSupported(nullptr, dsGfxFormat_X32));
	EXPECT_FALSE(dsGfxFormat_renderTargetSupported(resourceManager, dsGfxFormat_X32));
	EXPECT_TRUE(dsGfxFormat_renderTargetSupported(resourceManager,
		dsGfxFormat_decorate(dsGfxFormat_X32, dsGfxFormat_Float)));
	EXPECT_TRUE(dsGfxFormat_renderTargetSupported(resourceManager, dsGfxFormat_D16));
	EXPECT_FALSE(dsGfxFormat_renderTargetSupported(resourceManager,
		dsGfxFormat_decorate(dsGfxFormat_BC3, dsGfxFormat_UNorm)));
}

TEST_F(GfxFormatTest, TextureCopySupported)
{
	EXPECT_FALSE(dsGfxFormat_textureCopySupported(nullptr, dsGfxFormat_X32, dsGfxFormat_X32));
	EXPECT_FALSE(dsGfxFormat_textureCopySupported(resourceManager,
		dsGfxFormat_X32, dsGfxFormat_X32));
	EXPECT_TRUE(dsGfxFormat_textureCopySupported(resourceManager,
		dsGfxFormat_decorate(dsGfxFormat_X32, dsGfxFormat_Float),
		dsGfxFormat_decorate(dsGfxFormat_X32, dsGfxFormat_Float)));
	EXPECT_TRUE(dsGfxFormat_textureCopySupported(resourceManager, dsGfxFormat_D16,
		dsGfxFormat_D16));
	EXPECT_TRUE(dsGfxFormat_textureCopySupported(resourceManager,
		dsGfxFormat_decorate(dsGfxFormat_BC3, dsGfxFormat_UNorm),
		dsGfxFormat_decorate(dsGfxFormat_BC3, dsGfxFormat_UNorm)));
}

TEST_F(GfxFormatTest, TextureBlitSupported)
{
	EXPECT_FALSE(dsGfxFormat_surfaceBlitSupported(nullptr, dsGfxFormat_X32, dsGfxFormat_X32,
	dsBlitFilter_Nearest));
	EXPECT_FALSE(dsGfxFormat_surfaceBlitSupported(resourceManager,
		dsGfxFormat_X32, dsGfxFormat_X32, dsBlitFilter_Nearest));
	EXPECT_TRUE(dsGfxFormat_surfaceBlitSupported(resourceManager,
		dsGfxFormat_decorate(dsGfxFormat_X32, dsGfxFormat_Float),
		dsGfxFormat_decorate(dsGfxFormat_X32, dsGfxFormat_Float),
		dsBlitFilter_Nearest));
	EXPECT_TRUE(dsGfxFormat_surfaceBlitSupported(resourceManager, dsGfxFormat_D16,
		dsGfxFormat_D16, dsBlitFilter_Nearest));
	EXPECT_FALSE(dsGfxFormat_surfaceBlitSupported(resourceManager, dsGfxFormat_D16,
		dsGfxFormat_D16, dsBlitFilter_Linear));
	EXPECT_FALSE(dsGfxFormat_surfaceBlitSupported(resourceManager,
		dsGfxFormat_decorate(dsGfxFormat_BC3, dsGfxFormat_UNorm),
		dsGfxFormat_decorate(dsGfxFormat_BC3, dsGfxFormat_UNorm),
		dsBlitFilter_Nearest));
}

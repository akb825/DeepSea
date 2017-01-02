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
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <gtest/gtest.h>

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

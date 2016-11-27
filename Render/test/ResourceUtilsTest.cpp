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

#include <DeepSea/Render/Resources/ResourceUtils.h>
#include <gtest/gtest.h>

TEST(ResourceUtilsTest, GfxFormatIsValid)
{
	EXPECT_FALSE(dsGfxFormat_isValid(dsGfxFormat_R8G8B8A8));
	EXPECT_TRUE(dsGfxFormat_isValid(dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
		dsGfxFormat_Float)));

	EXPECT_TRUE(dsGfxFormat_isValid(dsGfxFormat_D16));
	EXPECT_FALSE(dsGfxFormat_isValid(dsGfxFormat_decorate(dsGfxFormat_D16, dsGfxFormat_Float)));

	EXPECT_TRUE(dsGfxFormat_isValid(dsGfxFormat_ETC1));
	EXPECT_TRUE(dsGfxFormat_isValid(dsGfxFormat_decorate(dsGfxFormat_ETC1, dsGfxFormat_UNorm)));

	EXPECT_FALSE(dsGfxFormat_isValid((dsGfxFormat)(dsGfxFormat_R8G8B8A8 | dsGfxFormat_D16 |
		dsGfxFormat_UNorm)));
	EXPECT_FALSE(dsGfxFormat_isValid((dsGfxFormat)(dsGfxFormat_R8G8B8A8 | dsGfxFormat_ETC1 |
		dsGfxFormat_UNorm)));
	EXPECT_FALSE(dsGfxFormat_isValid((dsGfxFormat)(dsGfxFormat_D16 | dsGfxFormat_ETC1 |
		dsGfxFormat_UNorm)));
}

TEST(ResourceUtilsTest, GfxFormatIndices)
{
	EXPECT_EQ(5U, dsGfxFormat_standardIndex(dsGfxFormat_decorate(dsGfxFormat_B5G6R5,
		dsGfxFormat_SInt)));
	EXPECT_EQ(0U, dsGfxFormat_standardIndex(dsGfxFormat_D16));
	EXPECT_EQ(dsGfxFormat_B5G6R5, dsGfxFormat_standardEnum(5));
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

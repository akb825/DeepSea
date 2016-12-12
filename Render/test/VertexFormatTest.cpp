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

#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <gtest/gtest.h>

TEST(VertexFormat, Initialize)
{
	dsVertexFormat vertexFormat;
	EXPECT_FALSE(dsVertexFormat_initialize(nullptr));
	EXPECT_TRUE(dsVertexFormat_initialize(&vertexFormat));

	EXPECT_EQ(0U, vertexFormat.size);
	EXPECT_EQ(0U, vertexFormat.enabledMask);
}

TEST(VertexFormat, Enabled)
{
	dsVertexFormat vertexFormat;
	EXPECT_TRUE(dsVertexFormat_initialize(&vertexFormat));

	EXPECT_FALSE(dsVertexFormat_getAttribEnabled(&vertexFormat, dsVertexAttrib_Position));
	EXPECT_FALSE(dsVertexFormat_getAttribEnabled(&vertexFormat, dsVertexAttrib_Normal));
	EXPECT_FALSE(dsVertexFormat_getAttribEnabled(&vertexFormat, dsVertexAttrib_Color));

	EXPECT_FALSE(dsVertexFormat_setAttribEnabled(nullptr, dsVertexAttrib_Position, true));
	EXPECT_TRUE(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Position, true));
	EXPECT_TRUE(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Normal, true));
	EXPECT_TRUE(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Color, true));

	EXPECT_FALSE(dsVertexFormat_getAttribEnabled(nullptr, dsVertexAttrib_Position));
	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&vertexFormat, dsVertexAttrib_Position));
	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&vertexFormat, dsVertexAttrib_Normal));
	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&vertexFormat, dsVertexAttrib_Color));
	EXPECT_FALSE(dsVertexFormat_getAttribEnabled(&vertexFormat, dsVertexAttrib_TexCoord0));

	EXPECT_TRUE(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Normal, false));
	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&vertexFormat, dsVertexAttrib_Position));
	EXPECT_FALSE(dsVertexFormat_getAttribEnabled(&vertexFormat, dsVertexAttrib_Normal));
	EXPECT_TRUE(dsVertexFormat_getAttribEnabled(&vertexFormat, dsVertexAttrib_Color));
	EXPECT_FALSE(dsVertexFormat_getAttribEnabled(&vertexFormat, dsVertexAttrib_TexCoord0));
}

TEST(VertexFormat, ComputeOffsetsAndSize)
{
	dsVertexFormat vertexFormat;
	EXPECT_TRUE(dsVertexFormat_initialize(&vertexFormat));
	EXPECT_TRUE(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Position, true));
	EXPECT_TRUE(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Normal, true));
	EXPECT_TRUE(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Color, true));

	EXPECT_FALSE(dsVertexFormat_computeOffsetsAndSize(nullptr));
	EXPECT_FALSE(dsVertexFormat_computeOffsetsAndSize(&vertexFormat));

	vertexFormat.elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
	vertexFormat.elements[dsVertexAttrib_Normal].format =
		dsGfxFormat_decorate(dsGfxFormat_W2X10Y10Z10, dsGfxFormat_SNorm);
	vertexFormat.elements[dsVertexAttrib_Color].format =
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);

	EXPECT_TRUE(dsVertexFormat_computeOffsetsAndSize(&vertexFormat));
	EXPECT_EQ(20U, vertexFormat.size);
	EXPECT_EQ(0U, vertexFormat.elements[dsVertexAttrib_Position].offset);
	EXPECT_EQ(12U, vertexFormat.elements[dsVertexAttrib_Position].size);
	EXPECT_EQ(12U, vertexFormat.elements[dsVertexAttrib_Normal].offset);
	EXPECT_EQ(4U, vertexFormat.elements[dsVertexAttrib_Normal].size);
	EXPECT_EQ(16U, vertexFormat.elements[dsVertexAttrib_Color].offset);
	EXPECT_EQ(4U, vertexFormat.elements[dsVertexAttrib_Color].size);
}

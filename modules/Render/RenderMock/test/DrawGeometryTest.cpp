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
#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <gtest/gtest.h>

class DrawGeometryTest : public FixtureBase
{
};

TEST_F(DrawGeometryTest, Create)
{
	EXPECT_FALSE(dsDrawGeometry_create(NULL, NULL, NULL, NULL));
	EXPECT_FALSE(dsDrawGeometry_create(resourceManager, NULL, NULL, NULL));

	dsVertexBuffer vertexBuffer1 = {};
	dsVertexBuffer vertexBuffer2 = {};
	dsIndexBuffer indexBuffer = {};

	dsGfxBuffer* vertexGfxBuffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Vertex, dsGfxMemory_Static | dsGfxMemory_Draw, NULL, 1024);
	ASSERT_TRUE(vertexGfxBuffer);
	dsGfxBuffer* indexGfxBuffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Index, dsGfxMemory_Static | dsGfxMemory_Draw, NULL, 1024);
	ASSERT_TRUE(indexGfxBuffer);

	dsVertexBuffer* vertexBufferArray[DS_MAX_GEOMETRY_VERTEX_BUFFERS] = {};
	EXPECT_FALSE(dsDrawGeometry_create(resourceManager, NULL, vertexBufferArray, NULL));

	vertexBufferArray[0] = &vertexBuffer1;
	vertexBufferArray[1] = &vertexBuffer2;
	EXPECT_FALSE(dsDrawGeometry_create(resourceManager, NULL, vertexBufferArray, NULL));

	EXPECT_TRUE(dsVertexFormat_setAttribEnabled(&vertexBuffer1.format, dsVertexAttrib_Position,
		true));
	vertexBuffer1.format.elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);

	EXPECT_TRUE(dsVertexFormat_setAttribEnabled(&vertexBuffer2.format, dsVertexAttrib_Normal,
		true));
	EXPECT_TRUE(dsVertexFormat_setAttribEnabled(&vertexBuffer2.format, dsVertexAttrib_Color, true));
	vertexBuffer2.format.elements[dsVertexAttrib_Normal].format =
		dsGfxFormat_decorate(dsGfxFormat_W2X10Y10Z10, dsGfxFormat_SNorm);
	vertexBuffer2.format.elements[dsVertexAttrib_Color].format =
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);

	vertexBuffer1.buffer = vertexGfxBuffer;
	vertexBuffer1.offset = 0;
	vertexBuffer1.count = 10;
	vertexBuffer2.buffer = vertexGfxBuffer;
	vertexBuffer2.offset = 0;
	vertexBuffer2.count = 10;
	EXPECT_FALSE(dsDrawGeometry_create(resourceManager, NULL, vertexBufferArray, NULL));

	EXPECT_TRUE(dsVertexFormat_computeOffsetsAndSize(&vertexBuffer1.format));
	EXPECT_TRUE(dsVertexFormat_computeOffsetsAndSize(&vertexBuffer2.format));
	vertexBuffer1.count = 9;
	EXPECT_FALSE(dsDrawGeometry_create(resourceManager, NULL, vertexBufferArray, NULL));

	vertexBuffer1.count = 10;
	dsDrawGeometry* drawGeometry = dsDrawGeometry_create(resourceManager, NULL, vertexBufferArray,
		NULL);
	ASSERT_TRUE(drawGeometry);
	EXPECT_EQ(1U, resourceManager->geometryCount);
	EXPECT_EQ(10U, dsDrawGeometry_getVertexCount(drawGeometry));
	EXPECT_EQ(0U, dsDrawGeometry_getIndexCount(drawGeometry));
	EXPECT_TRUE(dsDrawGeometry_destroy(drawGeometry));
	EXPECT_EQ(0U, resourceManager->geometryCount);

	drawGeometry = dsDrawGeometry_create(resourceManager, NULL, vertexBufferArray, &indexBuffer);
	EXPECT_FALSE(drawGeometry);

	indexBuffer.buffer = indexGfxBuffer;
	indexBuffer.offset = 0;
	indexBuffer.count = 20;
	indexBuffer.indexSize = 5;
	EXPECT_FALSE(dsDrawGeometry_create(resourceManager, NULL, vertexBufferArray, &indexBuffer));

	indexBuffer.indexSize = 2;
	drawGeometry = dsDrawGeometry_create(resourceManager, NULL, vertexBufferArray, &indexBuffer);
	ASSERT_TRUE(drawGeometry);
	EXPECT_EQ(10U, dsDrawGeometry_getVertexCount(drawGeometry));
	EXPECT_EQ(20U, dsDrawGeometry_getIndexCount(drawGeometry));
	EXPECT_TRUE(dsDrawGeometry_destroy(drawGeometry));

	vertexBuffer2.offset = 1000;
	EXPECT_FALSE(dsDrawGeometry_create(resourceManager, NULL, vertexBufferArray, &indexBuffer));

	vertexBuffer2.offset = 0;
	indexBuffer.offset = 1000;
	EXPECT_FALSE(dsDrawGeometry_create(resourceManager, NULL, vertexBufferArray, &indexBuffer));

	indexBuffer.offset = 0;
	vertexBuffer1.buffer = indexGfxBuffer;
	EXPECT_FALSE(dsDrawGeometry_create(resourceManager, NULL, vertexBufferArray, &indexBuffer));

	vertexBuffer1.buffer = vertexGfxBuffer;
	indexBuffer.buffer = vertexGfxBuffer;
	EXPECT_FALSE(dsDrawGeometry_create(resourceManager, NULL, vertexBufferArray, &indexBuffer));

	EXPECT_TRUE(dsGfxBuffer_destroy(vertexGfxBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(indexGfxBuffer));
}

/*
 * Copyright 2018 Aaron Barany
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
#include "VectorImageImpl.h"
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/RenderMock/MockRenderer.h>
#include <DeepSea/VectorDraw/VectorImage.h>
#include <DeepSea/VectorDraw/VectorMaterial.h>
#include <DeepSea/VectorDraw/VectorMaterialSet.h>
#include <DeepSea/VectorDraw/VectorScratchData.h>

// NOTE: Image space has the origin in the upper-left, so winding is reversed compared to Cartesian
// coordinates.

class TriangulateTest : public FixtureBase
{
};

TEST_F(TriangulateTest, TriangleCW)
{
	dsVectorMaterialSet* materialSet = dsVectorMaterialSet_create((dsAllocator*)&allocator,
		resourceManager, NULL, 1);
	dsVectorMaterial material;
	dsColor color = {{255, 255, 255, 255}};
	ASSERT_TRUE(dsVectorMaterial_setColor(&material, color));
	ASSERT_TRUE(dsVectorMaterialSet_addMaterial(materialSet, "fill", &material, true));

	dsVectorScratchData* scratchData = dsVectorScratchData_create((dsAllocator*)&allocator);
	dsVectorCommand commands[6];
	commands[0].commandType = dsVectorCommandType_StartPath;
	dsMatrix33_identity(commands[0].startPath.transform);
	commands[1].commandType = dsVectorCommandType_Move;
	commands[1].move.position.x = 0.0f;
	commands[1].move.position.y = 0.0f;
	commands[2].commandType = dsVectorCommandType_Line;
	commands[2].line.end.x = 1.0f;
	commands[2].line.end.y = 1.2f;
	commands[3].commandType = dsVectorCommandType_Line;
	commands[3].line.end.x = 2.0f;
	commands[3].line.end.y = 0.4f;
	commands[4].commandType = dsVectorCommandType_ClosePath;
	commands[5].commandType = dsVectorCommandType_FillPath;
	commands[5].fillPath.material = "fill";
	commands[5].fillPath.opacity = 1.0f;

	dsVector2f size = {{2.0f, 2.0f}};
	dsVectorImage* image = dsVectorImage_create((dsAllocator*)&allocator, scratchData,
		resourceManager, NULL, commands, DS_ARRAY_SIZE(commands), materialSet, true,
		NULL, &size, 0.1f);
	ASSERT_TRUE(image);

	dsGfxBuffer* buffer = dsVectorImage_getBuffer(image);
	ASSERT_TRUE(buffer);
	ASSERT_EQ(sizeof(ShapeVertex)*3 + sizeof(uint16_t)*3, buffer->size);

	const void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, buffer->size);
	ASSERT_TRUE(data);

	const ShapeVertex* vertices = (const ShapeVertex*)data;
	EXPECT_EQ(0.0f, vertices[0].position.x);
	EXPECT_EQ(0.0f, vertices[0].position.y);
	EXPECT_EQ(1.0f, vertices[1].position.x);
	EXPECT_EQ(1.2f, vertices[1].position.y);
	EXPECT_EQ(2.0f, vertices[2].position.x);
	EXPECT_EQ(0.4f, vertices[2].position.y);

	const uint16_t* indices = (const uint16_t*)((const uint8_t*)data + 3*sizeof(ShapeVertex));
	EXPECT_EQ(2U, indices[0]);
	EXPECT_EQ(0U, indices[1]);
	EXPECT_EQ(1U, indices[2]);

	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsVectorImage_destroy(image));
	dsVectorScratchData_destroy(scratchData);
}

TEST_F(TriangulateTest, TriangleCCW)
{
	dsVectorMaterialSet* materialSet = dsVectorMaterialSet_create((dsAllocator*)&allocator,
		resourceManager, NULL, 1);
	dsVectorMaterial material;
	dsColor color = {{255, 255, 255, 255}};
	ASSERT_TRUE(dsVectorMaterial_setColor(&material, color));
	ASSERT_TRUE(dsVectorMaterialSet_addMaterial(materialSet, "fill", &material, true));

	dsVectorScratchData* scratchData = dsVectorScratchData_create((dsAllocator*)&allocator);
	dsVectorCommand commands[6];
	commands[0].commandType = dsVectorCommandType_StartPath;
	dsMatrix33_identity(commands[0].startPath.transform);
	commands[1].commandType = dsVectorCommandType_Move;
	commands[1].move.position.x = 0.0f;
	commands[1].move.position.y = 0.0f;
	commands[2].commandType = dsVectorCommandType_Line;
	commands[2].line.end.x = 2.0f;
	commands[2].line.end.y = 0.4f;
	commands[3].commandType = dsVectorCommandType_Line;
	commands[3].line.end.x = 1.0f;
	commands[3].line.end.y = 1.2f;
	commands[4].commandType = dsVectorCommandType_ClosePath;
	commands[5].commandType = dsVectorCommandType_FillPath;
	commands[5].fillPath.material = "fill";
	commands[5].fillPath.opacity = 1.0f;

	dsVector2f size = {{2.0f, 2.0f}};
	dsVectorImage* image = dsVectorImage_create((dsAllocator*)&allocator, scratchData,
		resourceManager, NULL, commands, DS_ARRAY_SIZE(commands), materialSet, true,
		NULL, &size, 0.1f);
	ASSERT_TRUE(image);

	dsGfxBuffer* buffer = dsVectorImage_getBuffer(image);
	ASSERT_TRUE(buffer);
	ASSERT_EQ(sizeof(ShapeVertex)*3 + sizeof(uint16_t)*3, buffer->size);

	const void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, buffer->size);
	ASSERT_TRUE(data);

	const ShapeVertex* vertices = (const ShapeVertex*)data;
	EXPECT_EQ(0.0f, vertices[0].position.x);
	EXPECT_EQ(0.0f, vertices[0].position.y);
	EXPECT_EQ(2.0f, vertices[1].position.x);
	EXPECT_EQ(0.4f, vertices[1].position.y);
	EXPECT_EQ(1.0f, vertices[2].position.x);
	EXPECT_EQ(1.2f, vertices[2].position.y);

	const uint16_t* indices = (const uint16_t*)((const uint8_t*)data + 3*sizeof(ShapeVertex));
	EXPECT_EQ(1U, indices[0]);
	EXPECT_EQ(0U, indices[1]);
	EXPECT_EQ(2U, indices[2]);

	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsVectorImage_destroy(image));
	dsVectorScratchData_destroy(scratchData);
}

TEST_F(TriangulateTest, ObliqueTriangleCW)
{
	dsVectorMaterialSet* materialSet = dsVectorMaterialSet_create((dsAllocator*)&allocator,
		resourceManager, NULL, 1);
	dsVectorMaterial material;
	dsColor color = {{255, 255, 255, 255}};
	ASSERT_TRUE(dsVectorMaterial_setColor(&material, color));
	ASSERT_TRUE(dsVectorMaterialSet_addMaterial(materialSet, "fill", &material, true));

	dsVectorScratchData* scratchData = dsVectorScratchData_create((dsAllocator*)&allocator);
	dsVectorCommand commands[6];
	commands[0].commandType = dsVectorCommandType_StartPath;
	dsMatrix33_identity(commands[0].startPath.transform);
	commands[1].commandType = dsVectorCommandType_Move;
	commands[1].move.position.x = 0.0f;
	commands[1].move.position.y = 0.0f;
	commands[2].commandType = dsVectorCommandType_Line;
	commands[2].line.end.x = 2.0f;
	commands[2].line.end.y = 1.2f;
	commands[3].commandType = dsVectorCommandType_Line;
	commands[3].line.end.x = 1.0f;
	commands[3].line.end.y = 0.4f;
	commands[4].commandType = dsVectorCommandType_ClosePath;
	commands[5].commandType = dsVectorCommandType_FillPath;
	commands[5].fillPath.material = "fill";
	commands[5].fillPath.opacity = 1.0f;

	dsVector2f size = {{2.0f, 2.0f}};
	dsVectorImage* image = dsVectorImage_create((dsAllocator*)&allocator, scratchData,
		resourceManager, NULL, commands, DS_ARRAY_SIZE(commands), materialSet, true,
		NULL, &size, 0.1f);
	ASSERT_TRUE(image);

	dsGfxBuffer* buffer = dsVectorImage_getBuffer(image);
	ASSERT_TRUE(buffer);
	ASSERT_EQ(sizeof(ShapeVertex)*3 + sizeof(uint16_t)*3, buffer->size);

	const void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, buffer->size);
	ASSERT_TRUE(data);

	const ShapeVertex* vertices = (const ShapeVertex*)data;
	EXPECT_EQ(0.0f, vertices[0].position.x);
	EXPECT_EQ(0.0f, vertices[0].position.y);
	EXPECT_EQ(2.0f, vertices[1].position.x);
	EXPECT_EQ(1.2f, vertices[1].position.y);
	EXPECT_EQ(1.0f, vertices[2].position.x);
	EXPECT_EQ(0.4f, vertices[2].position.y);

	const uint16_t* indices = (const uint16_t*)((const uint8_t*)data + 3*sizeof(ShapeVertex));
	EXPECT_EQ(1U, indices[0]);
	EXPECT_EQ(2U, indices[1]);
	EXPECT_EQ(0U, indices[2]);

	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsVectorImage_destroy(image));
	dsVectorScratchData_destroy(scratchData);
}

TEST_F(TriangulateTest, ObliqueTriangleCCW)
{
	dsVectorMaterialSet* materialSet = dsVectorMaterialSet_create((dsAllocator*)&allocator,
		resourceManager, NULL, 1);
	dsVectorMaterial material;
	dsColor color = {{255, 255, 255, 255}};
	ASSERT_TRUE(dsVectorMaterial_setColor(&material, color));
	ASSERT_TRUE(dsVectorMaterialSet_addMaterial(materialSet, "fill", &material, true));

	dsVectorScratchData* scratchData = dsVectorScratchData_create((dsAllocator*)&allocator);
	dsVectorCommand commands[6];
	commands[0].commandType = dsVectorCommandType_StartPath;
	dsMatrix33_identity(commands[0].startPath.transform);
	commands[1].commandType = dsVectorCommandType_Move;
	commands[1].move.position.x = 0.0f;
	commands[1].move.position.y = 0.0f;
	commands[2].commandType = dsVectorCommandType_Line;
	commands[2].line.end.x = 1.0f;
	commands[2].line.end.y = 0.4f;
	commands[3].commandType = dsVectorCommandType_Line;
	commands[3].line.end.x = 2.0f;
	commands[3].line.end.y = 1.2f;
	commands[4].commandType = dsVectorCommandType_ClosePath;
	commands[5].commandType = dsVectorCommandType_FillPath;
	commands[5].fillPath.material = "fill";
	commands[5].fillPath.opacity = 1.0f;

	dsVector2f size = {{2.0f, 2.0f}};
	dsVectorImage* image = dsVectorImage_create((dsAllocator*)&allocator, scratchData,
		resourceManager, NULL, commands, DS_ARRAY_SIZE(commands), materialSet, true,
		NULL, &size, 0.1f);
	ASSERT_TRUE(image);

	dsGfxBuffer* buffer = dsVectorImage_getBuffer(image);
	ASSERT_TRUE(buffer);
	ASSERT_EQ(sizeof(ShapeVertex)*3 + sizeof(uint16_t)*3, buffer->size);

	const void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, buffer->size);
	ASSERT_TRUE(data);

	const ShapeVertex* vertices = (const ShapeVertex*)data;
	EXPECT_EQ(0.0f, vertices[0].position.x);
	EXPECT_EQ(0.0f, vertices[0].position.y);
	EXPECT_EQ(1.0f, vertices[1].position.x);
	EXPECT_EQ(0.4f, vertices[1].position.y);
	EXPECT_EQ(2.0f, vertices[2].position.x);
	EXPECT_EQ(1.2f, vertices[2].position.y);

	const uint16_t* indices = (const uint16_t*)((const uint8_t*)data + 3*sizeof(ShapeVertex));
	EXPECT_EQ(2U, indices[0]);
	EXPECT_EQ(1U, indices[1]);
	EXPECT_EQ(0U, indices[2]);

	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsVectorImage_destroy(image));
	dsVectorScratchData_destroy(scratchData);
}

TEST_F(TriangulateTest, QuadCW)
{
	dsVectorMaterialSet* materialSet = dsVectorMaterialSet_create((dsAllocator*)&allocator,
		resourceManager, NULL, 1);
	dsVectorMaterial material;
	dsColor color = {{255, 255, 255, 255}};
	ASSERT_TRUE(dsVectorMaterial_setColor(&material, color));
	ASSERT_TRUE(dsVectorMaterialSet_addMaterial(materialSet, "fill", &material, true));

	dsVectorScratchData* scratchData = dsVectorScratchData_create((dsAllocator*)&allocator);
	dsVectorCommand commands[7];
	commands[0].commandType = dsVectorCommandType_StartPath;
	dsMatrix33_identity(commands[0].startPath.transform);
	commands[1].commandType = dsVectorCommandType_Move;
	commands[1].move.position.x = 2.0f;
	commands[1].move.position.y = 1.3f;
	commands[2].commandType = dsVectorCommandType_Line;
	commands[2].line.end.x = 1.2f;
	commands[2].line.end.y = 0.4f;
	commands[3].commandType = dsVectorCommandType_Line;
	commands[3].line.end.x = 0.0f;
	commands[3].line.end.y = 0.9f;
	commands[4].commandType = dsVectorCommandType_Line;
	commands[4].line.end.x = 0.8f;
	commands[4].line.end.y = 2.0f;
	commands[5].commandType = dsVectorCommandType_ClosePath;
	commands[6].commandType = dsVectorCommandType_FillPath;
	commands[6].fillPath.material = "fill";
	commands[6].fillPath.opacity = 1.0f;

	dsVector2f size = {{2.0f, 2.0f}};
	dsVectorImage* image = dsVectorImage_create((dsAllocator*)&allocator, scratchData,
		resourceManager, NULL, commands, DS_ARRAY_SIZE(commands), materialSet, true,
		NULL, &size, 0.1f);
	ASSERT_TRUE(image);

	dsGfxBuffer* buffer = dsVectorImage_getBuffer(image);
	ASSERT_TRUE(buffer);
	ASSERT_EQ(sizeof(ShapeVertex)*4 + sizeof(uint16_t)*6, buffer->size);

	const void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, buffer->size);
	ASSERT_TRUE(data);

	const ShapeVertex* vertices = (const ShapeVertex*)data;
	EXPECT_EQ(2.0f, vertices[0].position.x);
	EXPECT_EQ(1.3f, vertices[0].position.y);
	EXPECT_EQ(1.2f, vertices[1].position.x);
	EXPECT_EQ(0.4f, vertices[1].position.y);
	EXPECT_EQ(0.0f, vertices[2].position.x);
	EXPECT_EQ(0.9f, vertices[2].position.y);
	EXPECT_EQ(0.8f, vertices[3].position.x);
	EXPECT_EQ(2.0f, vertices[3].position.y);

	const uint16_t* indices = (const uint16_t*)((const uint8_t*)data + 4*sizeof(ShapeVertex));
	EXPECT_EQ(1U, indices[0]);
	EXPECT_EQ(2U, indices[1]);
	EXPECT_EQ(3U, indices[2]);

	EXPECT_EQ(0U, indices[3]);
	EXPECT_EQ(1U, indices[4]);
	EXPECT_EQ(3U, indices[5]);

	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsVectorImage_destroy(image));
	dsVectorScratchData_destroy(scratchData);
}

TEST_F(TriangulateTest, QuadCCW)
{
	dsVectorMaterialSet* materialSet = dsVectorMaterialSet_create((dsAllocator*)&allocator,
		resourceManager, NULL, 1);
	dsVectorMaterial material;
	dsColor color = {{255, 255, 255, 255}};
	ASSERT_TRUE(dsVectorMaterial_setColor(&material, color));
	ASSERT_TRUE(dsVectorMaterialSet_addMaterial(materialSet, "fill", &material, true));

	dsVectorScratchData* scratchData = dsVectorScratchData_create((dsAllocator*)&allocator);
	dsVectorCommand commands[7];
	commands[0].commandType = dsVectorCommandType_StartPath;
	dsMatrix33_identity(commands[0].startPath.transform);
	commands[1].commandType = dsVectorCommandType_Move;
	commands[1].move.position.x = 2.0f;
	commands[1].move.position.y = 1.3f;
	commands[2].commandType = dsVectorCommandType_Line;
	commands[2].line.end.x = 0.8f;
	commands[2].line.end.y = 2.0f;
	commands[3].commandType = dsVectorCommandType_Line;
	commands[3].line.end.x = 0.0f;
	commands[3].line.end.y = 0.9f;
	commands[4].commandType = dsVectorCommandType_Line;
	commands[4].line.end.x = 1.2f;
	commands[4].line.end.y = 0.4f;
	commands[5].commandType = dsVectorCommandType_ClosePath;
	commands[6].commandType = dsVectorCommandType_FillPath;
	commands[6].fillPath.material = "fill";
	commands[6].fillPath.opacity = 1.0f;

	dsVector2f size = {{2.0f, 2.0f}};
	dsVectorImage* image = dsVectorImage_create((dsAllocator*)&allocator, scratchData,
		resourceManager, NULL, commands, DS_ARRAY_SIZE(commands), materialSet, true,
		NULL, &size, 0.1f);
	ASSERT_TRUE(image);

	dsGfxBuffer* buffer = dsVectorImage_getBuffer(image);
	ASSERT_TRUE(buffer);
	ASSERT_EQ(sizeof(ShapeVertex)*4 + sizeof(uint16_t)*6, buffer->size);

	const void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, buffer->size);
	ASSERT_TRUE(data);

	const ShapeVertex* vertices = (const ShapeVertex*)data;
	EXPECT_EQ(2.0f, vertices[0].position.x);
	EXPECT_EQ(1.3f, vertices[0].position.y);
	EXPECT_EQ(0.8f, vertices[1].position.x);
	EXPECT_EQ(2.0f, vertices[1].position.y);
	EXPECT_EQ(0.0f, vertices[2].position.x);
	EXPECT_EQ(0.9f, vertices[2].position.y);
	EXPECT_EQ(1.2f, vertices[3].position.x);
	EXPECT_EQ(0.4f, vertices[3].position.y);

	const uint16_t* indices = (const uint16_t*)((const uint8_t*)data + 4*sizeof(ShapeVertex));
	EXPECT_EQ(3U, indices[0]);
	EXPECT_EQ(2U, indices[1]);
	EXPECT_EQ(1U, indices[2]);

	EXPECT_EQ(0U, indices[3]);
	EXPECT_EQ(3U, indices[4]);
	EXPECT_EQ(1U, indices[5]);

	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsVectorImage_destroy(image));
	dsVectorScratchData_destroy(scratchData);
}

TEST_F(TriangulateTest, MonotonicCW)
{
	dsVectorMaterialSet* materialSet = dsVectorMaterialSet_create((dsAllocator*)&allocator,
		resourceManager, NULL, 1);
	dsVectorMaterial material;
	dsColor color = {{255, 255, 255, 255}};
	ASSERT_TRUE(dsVectorMaterial_setColor(&material, color));
	ASSERT_TRUE(dsVectorMaterialSet_addMaterial(materialSet, "fill", &material, true));

	dsVectorScratchData* scratchData = dsVectorScratchData_create((dsAllocator*)&allocator);
	dsVectorCommand commands[16];
	commands[0].commandType = dsVectorCommandType_StartPath;
	dsMatrix33_identity(commands[0].startPath.transform);
	commands[1].commandType = dsVectorCommandType_Move;
	commands[1].move.position.x = 0.0f;
	commands[1].move.position.y = 11.4f;
	commands[2].commandType = dsVectorCommandType_Line;
	commands[2].line.end.x = 4.0f;
	commands[2].line.end.y = 6.5f;
	commands[3].commandType = dsVectorCommandType_Line;
	commands[3].line.end.x = 16.0f;
	commands[3].line.end.y = 1.7f;
	commands[4].commandType = dsVectorCommandType_Line;
	commands[4].line.end.x = 18.4f;
	commands[4].line.end.y = 14.8f;
	commands[5].commandType = dsVectorCommandType_Line;
	commands[5].line.end.x = 24.5f;
	commands[5].line.end.y = 13.2f;
	commands[6].commandType = dsVectorCommandType_Line;
	commands[6].line.end.x = 29.2f;
	commands[6].line.end.y = 9.0f;
	commands[7].commandType = dsVectorCommandType_Line;
	commands[7].line.end.x = 31.0f;
	commands[7].line.end.y = 0.0f;
	commands[8].commandType = dsVectorCommandType_Line;
	commands[8].line.end.x = 34.0f;
	commands[8].line.end.y = 0.0f;
	commands[9].commandType = dsVectorCommandType_Line;
	commands[9].line.end.x = 36.0f;
	commands[9].line.end.y = 16.0f;
	commands[10].commandType = dsVectorCommandType_Line;
	commands[10].line.end.x = 12.5f;
	commands[10].line.end.y = 16.0f;
	commands[11].commandType = dsVectorCommandType_Line;
	commands[11].line.end.x = 11.3f;
	commands[11].line.end.y = 11.2f;
	commands[12].commandType = dsVectorCommandType_Line;
	commands[12].line.end.x = 8.8f;
	commands[12].line.end.y = 8.9f;
	commands[13].commandType = dsVectorCommandType_Line;
	commands[13].line.end.x = 6.4f;
	commands[13].line.end.y = 8.9f;
	commands[14].commandType = dsVectorCommandType_ClosePath;
	commands[15].commandType = dsVectorCommandType_FillPath;
	commands[15].fillPath.material = "fill";
	commands[15].fillPath.opacity = 1.0f;

	dsVector2f size = {{36.0f, 16.0f}};
	dsVectorImage* image = dsVectorImage_create((dsAllocator*)&allocator, scratchData,
		resourceManager, NULL, commands, DS_ARRAY_SIZE(commands), materialSet, true,
		NULL, &size, 0.1f);
	ASSERT_TRUE(image);

	dsGfxBuffer* buffer = dsVectorImage_getBuffer(image);
	ASSERT_TRUE(buffer);
	ASSERT_EQ(sizeof(ShapeVertex)*13 + sizeof(uint16_t)*33, buffer->size);

	const void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, buffer->size);
	ASSERT_TRUE(data);

	const ShapeVertex* vertices = (const ShapeVertex*)data;
	EXPECT_EQ(0.0f, vertices[0].position.x);
	EXPECT_EQ(11.4f, vertices[0].position.y);
	EXPECT_EQ(4.0f, vertices[1].position.x);
	EXPECT_EQ(6.5f, vertices[1].position.y);
	EXPECT_EQ(16.0f, vertices[2].position.x);
	EXPECT_EQ(1.7f, vertices[2].position.y);
	EXPECT_EQ(18.4f, vertices[3].position.x);
	EXPECT_EQ(14.8f, vertices[3].position.y);
	EXPECT_EQ(24.5f, vertices[4].position.x);
	EXPECT_EQ(13.2f, vertices[4].position.y);
	EXPECT_EQ(29.2f, vertices[5].position.x);
	EXPECT_EQ(9.0f, vertices[5].position.y);
	EXPECT_EQ(31.0f, vertices[6].position.x);
	EXPECT_EQ(0.0f, vertices[6].position.y);
	EXPECT_EQ(34.0f, vertices[7].position.x);
	EXPECT_EQ(0.0f, vertices[7].position.y);
	EXPECT_EQ(36.0f, vertices[8].position.x);
	EXPECT_EQ(16.0f, vertices[8].position.y);
	EXPECT_EQ(12.5f, vertices[9].position.x);
	EXPECT_EQ(16.0f, vertices[9].position.y);
	EXPECT_EQ(11.3f, vertices[10].position.x);
	EXPECT_EQ(11.2f, vertices[10].position.y);
	EXPECT_EQ(8.8f, vertices[11].position.x);
	EXPECT_EQ(8.9f, vertices[11].position.y);
	EXPECT_EQ(6.4f, vertices[12].position.x);
	EXPECT_EQ(8.9f, vertices[12].position.y);

	const uint16_t* indices = (const uint16_t*)((const uint8_t*)data + 13*sizeof(ShapeVertex));
	EXPECT_EQ(12U, indices[0]);
	EXPECT_EQ(1U, indices[1]);
	EXPECT_EQ(0U, indices[2]);

	EXPECT_EQ(11U, indices[3]);
	EXPECT_EQ(1U, indices[4]);
	EXPECT_EQ(12U, indices[5]);

	EXPECT_EQ(2U, indices[6]);
	EXPECT_EQ(1U, indices[7]);
	EXPECT_EQ(11U, indices[8]);

	EXPECT_EQ(2U, indices[9]);
	EXPECT_EQ(11U, indices[10]);
	EXPECT_EQ(10U, indices[11]);

	EXPECT_EQ(2U, indices[12]);
	EXPECT_EQ(10U, indices[13]);
	EXPECT_EQ(9U, indices[14]);

	EXPECT_EQ(3U, indices[15]);
	EXPECT_EQ(2U, indices[16]);
	EXPECT_EQ(9U, indices[17]);

	EXPECT_EQ(7U, indices[18]);
	EXPECT_EQ(6U, indices[19]);
	EXPECT_EQ(5U, indices[20]);

	EXPECT_EQ(8U, indices[21]);
	EXPECT_EQ(7U, indices[22]);
	EXPECT_EQ(5U, indices[23]);

	EXPECT_EQ(8U, indices[24]);
	EXPECT_EQ(5U, indices[25]);
	EXPECT_EQ(4U, indices[26]);

	EXPECT_EQ(8U, indices[27]);
	EXPECT_EQ(4U, indices[28]);
	EXPECT_EQ(3U, indices[29]);

	EXPECT_EQ(8U, indices[30]);
	EXPECT_EQ(3U, indices[31]);
	EXPECT_EQ(9U, indices[32]);

	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsVectorImage_destroy(image));
	dsVectorScratchData_destroy(scratchData);
}

TEST_F(TriangulateTest, MonotonicCCW)
{
	dsVectorMaterialSet* materialSet = dsVectorMaterialSet_create((dsAllocator*)&allocator,
		resourceManager, NULL, 1);
	dsVectorMaterial material;
	dsColor color = {{255, 255, 255, 255}};
	ASSERT_TRUE(dsVectorMaterial_setColor(&material, color));
	ASSERT_TRUE(dsVectorMaterialSet_addMaterial(materialSet, "fill", &material, true));

	dsVectorScratchData* scratchData = dsVectorScratchData_create((dsAllocator*)&allocator);
	dsVectorCommand commands[16];
	commands[0].commandType = dsVectorCommandType_StartPath;
	dsMatrix33_identity(commands[0].startPath.transform);
	commands[1].commandType = dsVectorCommandType_Move;
	commands[1].move.position.x = 0.0f;
	commands[1].move.position.y = 11.4f;
	commands[2].commandType = dsVectorCommandType_Line;
	commands[2].line.end.x = 6.4f;
	commands[2].line.end.y = 8.9f;
	commands[3].commandType = dsVectorCommandType_Line;
	commands[3].line.end.x = 8.8f;
	commands[3].line.end.y = 8.9f;
	commands[4].commandType = dsVectorCommandType_Line;
	commands[4].line.end.x = 11.3f;
	commands[4].line.end.y = 11.2f;
	commands[5].commandType = dsVectorCommandType_Line;
	commands[5].line.end.x = 12.5f;
	commands[5].line.end.y = 16.0f;
	commands[6].commandType = dsVectorCommandType_Line;
	commands[6].line.end.x = 36.0f;
	commands[6].line.end.y = 16.0f;
	commands[7].commandType = dsVectorCommandType_Line;
	commands[7].line.end.x = 34.0f;
	commands[7].line.end.y = 0.0f;
	commands[8].commandType = dsVectorCommandType_Line;
	commands[8].line.end.x = 31.0f;
	commands[8].line.end.y = 0.0f;
	commands[9].commandType = dsVectorCommandType_Line;
	commands[9].line.end.x = 29.2f;
	commands[9].line.end.y = 9.0f;
	commands[10].commandType = dsVectorCommandType_Line;
	commands[10].line.end.x = 24.5f;
	commands[10].line.end.y = 13.2f;
	commands[11].commandType = dsVectorCommandType_Line;
	commands[11].line.end.x = 18.4f;
	commands[11].line.end.y = 14.8f;
	commands[12].commandType = dsVectorCommandType_Line;
	commands[12].line.end.x = 16.0f;
	commands[12].line.end.y = 1.7f;
	commands[13].commandType = dsVectorCommandType_Line;
	commands[13].line.end.x = 4.0f;
	commands[13].line.end.y = 6.5f;
	commands[14].commandType = dsVectorCommandType_ClosePath;
	commands[15].commandType = dsVectorCommandType_FillPath;
	commands[15].fillPath.material = "fill";
	commands[15].fillPath.opacity = 1.0f;

	dsVector2f size = {{36.0f, 16.0f}};
	dsVectorImage* image = dsVectorImage_create((dsAllocator*)&allocator, scratchData,
		resourceManager, NULL, commands, DS_ARRAY_SIZE(commands), materialSet, true,
		NULL, &size, 0.1f);
	ASSERT_TRUE(image);

	dsGfxBuffer* buffer = dsVectorImage_getBuffer(image);
	ASSERT_TRUE(buffer);
	ASSERT_EQ(sizeof(ShapeVertex)*13 + sizeof(uint16_t)*33, buffer->size);

	const void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, buffer->size);
	ASSERT_TRUE(data);

	const ShapeVertex* vertices = (const ShapeVertex*)data;
	EXPECT_EQ(0.0f, vertices[0].position.x);
	EXPECT_EQ(11.4f, vertices[0].position.y);
	EXPECT_EQ(6.4f, vertices[1].position.x);
	EXPECT_EQ(8.9f, vertices[1].position.y);
	EXPECT_EQ(8.8f, vertices[2].position.x);
	EXPECT_EQ(8.9f, vertices[2].position.y);
	EXPECT_EQ(11.3f, vertices[3].position.x);
	EXPECT_EQ(11.2f, vertices[3].position.y);
	EXPECT_EQ(12.5f, vertices[4].position.x);
	EXPECT_EQ(16.0f, vertices[4].position.y);
	EXPECT_EQ(36.0f, vertices[5].position.x);
	EXPECT_EQ(16.0f, vertices[5].position.y);
	EXPECT_EQ(34.0f, vertices[6].position.x);
	EXPECT_EQ(0.0f, vertices[6].position.y);
	EXPECT_EQ(31.0f, vertices[7].position.x);
	EXPECT_EQ(0.0f, vertices[7].position.y);
	EXPECT_EQ(29.2f, vertices[8].position.x);
	EXPECT_EQ(9.0f, vertices[8].position.y);
	EXPECT_EQ(24.5f, vertices[9].position.x);
	EXPECT_EQ(13.2f, vertices[9].position.y);
	EXPECT_EQ(18.4f, vertices[10].position.x);
	EXPECT_EQ(14.8f, vertices[10].position.y);
	EXPECT_EQ(16.0f, vertices[11].position.x);
	EXPECT_EQ(1.7f, vertices[11].position.y);
	EXPECT_EQ(4.0f, vertices[12].position.x);
	EXPECT_EQ(6.5f, vertices[12].position.y);

	const uint16_t* indices = (const uint16_t*)((const uint8_t*)data + 13*sizeof(ShapeVertex));
	EXPECT_EQ(1U, indices[0]);
	EXPECT_EQ(12U, indices[1]);
	EXPECT_EQ(0U, indices[2]);

	EXPECT_EQ(2U, indices[3]);
	EXPECT_EQ(12U, indices[4]);
	EXPECT_EQ(1U, indices[5]);

	EXPECT_EQ(11U, indices[6]);
	EXPECT_EQ(12U, indices[7]);
	EXPECT_EQ(2U, indices[8]);

	EXPECT_EQ(11U, indices[9]);
	EXPECT_EQ(2U, indices[10]);
	EXPECT_EQ(3U, indices[11]);

	EXPECT_EQ(11U, indices[12]);
	EXPECT_EQ(3U, indices[13]);
	EXPECT_EQ(4U, indices[14]);

	EXPECT_EQ(10U, indices[15]);
	EXPECT_EQ(11U, indices[16]);
	EXPECT_EQ(4U, indices[17]);

	EXPECT_EQ(6U, indices[18]);
	EXPECT_EQ(7U, indices[19]);
	EXPECT_EQ(8U, indices[20]);

	EXPECT_EQ(5U, indices[21]);
	EXPECT_EQ(6U, indices[22]);
	EXPECT_EQ(8U, indices[23]);

	EXPECT_EQ(5U, indices[24]);
	EXPECT_EQ(8U, indices[25]);
	EXPECT_EQ(9U, indices[26]);

	EXPECT_EQ(5U, indices[27]);
	EXPECT_EQ(9U, indices[28]);
	EXPECT_EQ(10U, indices[29]);

	EXPECT_EQ(5U, indices[30]);
	EXPECT_EQ(10U, indices[31]);
	EXPECT_EQ(4U, indices[32]);

	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsVectorImage_destroy(image));
	dsVectorScratchData_destroy(scratchData);
}

TEST_F(TriangulateTest, ComplexCW)
{
	dsVectorMaterialSet* materialSet = dsVectorMaterialSet_create((dsAllocator*)&allocator,
		resourceManager, NULL, 1);
	dsVectorMaterial material;
	dsColor color = {{255, 255, 255, 255}};
	ASSERT_TRUE(dsVectorMaterial_setColor(&material, color));
	ASSERT_TRUE(dsVectorMaterialSet_addMaterial(materialSet, "fill", &material, true));

	dsVectorScratchData* scratchData = dsVectorScratchData_create((dsAllocator*)&allocator);
	dsVectorCommand commands[42];
	commands[0].commandType = dsVectorCommandType_StartPath;
	dsMatrix33_identity(commands[0].startPath.transform);
	commands[1].commandType = dsVectorCommandType_Move;
	commands[1].move.position.x = 0.0f;
	commands[1].move.position.y = 26.0f;
	commands[2].commandType = dsVectorCommandType_Line;
	commands[2].line.end.x = 5.4f;
	commands[2].line.end.y = 7.6f;
	commands[3].commandType = dsVectorCommandType_Line;
	commands[3].line.end.x = 16.0f;
	commands[3].line.end.y = 5.2f;
	commands[4].commandType = dsVectorCommandType_Line;
	commands[4].line.end.x = 14.5f;
	commands[4].line.end.y = 13.6f;
	commands[5].commandType = dsVectorCommandType_Line;
	commands[5].line.end.x = 10.1f;
	commands[5].line.end.y = 19.2f;
	commands[6].commandType = dsVectorCommandType_Line;
	commands[6].line.end.x = 17.0f;
	commands[6].line.end.y = 22.0f;
	commands[7].commandType = dsVectorCommandType_Line;
	commands[7].line.end.x = 21.0f;
	commands[7].line.end.y = 14.5f;
	commands[8].commandType = dsVectorCommandType_Line;
	commands[8].line.end.x = 18.4f;
	commands[8].line.end.y = 7.3f;
	commands[9].commandType = dsVectorCommandType_Line;
	commands[9].line.end.x = 33.1f;
	commands[9].line.end.y = 0.0f;
	commands[10].commandType = dsVectorCommandType_Line;
	commands[10].line.end.x = 38.0f;
	commands[10].line.end.y = 4.8f;
	commands[11].commandType = dsVectorCommandType_Line;
	commands[11].line.end.x = 33.1f;
	commands[11].line.end.y = 10.6f;
	commands[12].commandType = dsVectorCommandType_Line;
	commands[12].line.end.x = 26.8f;
	commands[12].line.end.y = 12.5f;
	commands[13].commandType = dsVectorCommandType_Line;
	commands[13].line.end.x = 37.4f;
	commands[13].line.end.y = 17.1f;
	commands[14].commandType = dsVectorCommandType_Line;
	commands[14].line.end.x = 29.0f;
	commands[14].line.end.y = 21.7f;
	commands[15].commandType = dsVectorCommandType_Line;
	commands[15].line.end.x = 37.6f;
	commands[15].line.end.y = 24.1f;
	commands[16].commandType = dsVectorCommandType_Line;
	commands[16].line.end.x = 43.9f;
	commands[16].line.end.y = 21.4f;
	commands[17].commandType = dsVectorCommandType_Line;
	commands[17].line.end.x = 42.1f;
	commands[17].line.end.y = 10.3f;
	commands[18].commandType = dsVectorCommandType_Line;
	commands[18].line.end.x = 51.7f;
	commands[18].line.end.y = 5.7f;
	commands[19].commandType = dsVectorCommandType_Line;
	commands[19].line.end.x = 63.4f;
	commands[19].line.end.y = 5.7f;
	commands[20].commandType = dsVectorCommandType_Line;
	commands[20].line.end.x = 60.2f;
	commands[20].line.end.y = 17.0f;
	commands[21].commandType = dsVectorCommandType_Line;
	commands[21].line.end.x = 54.1f;
	commands[21].line.end.y = 12.9f;
	commands[22].commandType = dsVectorCommandType_Line;
	commands[22].line.end.x = 47.1f;
	commands[22].line.end.y = 24.0f;
	commands[23].commandType = dsVectorCommandType_Line;
	commands[23].line.end.x = 69.5f;
	commands[23].line.end.y = 23.0f;
	commands[24].commandType = dsVectorCommandType_Line;
	commands[24].line.end.x = 62.4f;
	commands[24].line.end.y = 31.5f;
	commands[25].commandType = dsVectorCommandType_Line;
	commands[25].line.end.x = 64.6f;
	commands[25].line.end.y = 45.6f;
	commands[26].commandType = dsVectorCommandType_Line;
	commands[26].line.end.x = 60.5f;
	commands[26].line.end.y = 37.0f;
	commands[27].commandType = dsVectorCommandType_Line;
	commands[27].line.end.x = 54.4f;
	commands[27].line.end.y = 34.9f;
	commands[28].commandType = dsVectorCommandType_Line;
	commands[28].line.end.x = 58.1f;
	commands[28].line.end.y = 27.2f;
	commands[29].commandType = dsVectorCommandType_Line;
	commands[29].line.end.x = 40.7f;
	commands[29].line.end.y = 30.2f;
	commands[30].commandType = dsVectorCommandType_Line;
	commands[30].line.end.x = 52.5f;
	commands[30].line.end.y = 33.0f;
	commands[31].commandType = dsVectorCommandType_Line;
	commands[31].line.end.x = 45.3f;
	commands[31].line.end.y = 41.2f;
	commands[32].commandType = dsVectorCommandType_Line;
	commands[32].line.end.x = 36.5f;
	commands[32].line.end.y = 37.9f;
	commands[33].commandType = dsVectorCommandType_Line;
	commands[33].line.end.x = 33.1f;
	commands[33].line.end.y = 27.8f;
	commands[34].commandType = dsVectorCommandType_Line;
	commands[34].line.end.x = 23.9f;
	commands[34].line.end.y = 26.8f;
	commands[35].commandType = dsVectorCommandType_Line;
	commands[35].line.end.x = 14.5f;
	commands[35].line.end.y = 29.9f;
	commands[36].commandType = dsVectorCommandType_Line;
	commands[36].line.end.x = 26.8f;
	commands[36].line.end.y = 31.8f;
	commands[37].commandType = dsVectorCommandType_Line;
	commands[37].line.end.x = 25.7f;
	commands[37].line.end.y = 37.1f;
	commands[38].commandType = dsVectorCommandType_Line;
	commands[38].line.end.x = 18.9f;
	commands[38].line.end.y = 41.4f;
	commands[39].commandType = dsVectorCommandType_Line;
	commands[39].line.end.x = 8.4f;
	commands[39].line.end.y = 38.2f;
	commands[40].commandType = dsVectorCommandType_ClosePath;
	commands[41].commandType = dsVectorCommandType_FillPath;
	commands[41].fillPath.material = "fill";
	commands[41].fillPath.opacity = 1.0f;

	dsVector2f size = {{36.0f, 16.0f}};
	dsVectorImage* image = dsVectorImage_create((dsAllocator*)&allocator, scratchData,
		resourceManager, NULL, commands, DS_ARRAY_SIZE(commands), materialSet, true,
		NULL, &size, 0.1f);
	ASSERT_TRUE(image);

	dsGfxBuffer* buffer = dsVectorImage_getBuffer(image);
	ASSERT_TRUE(buffer);
	ASSERT_EQ(sizeof(ShapeVertex)*39 + sizeof(uint16_t)*111, buffer->size);

	const void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, buffer->size);
	ASSERT_TRUE(data);

	const ShapeVertex* vertices = (const ShapeVertex*)data;
	EXPECT_EQ(0.0f, vertices[0].position.x);
	EXPECT_EQ(26.0f, vertices[0].position.y);
	EXPECT_EQ(5.4f, vertices[1].position.x);
	EXPECT_EQ(7.6f, vertices[1].position.y);
	EXPECT_EQ(16.0f, vertices[2].position.x);
	EXPECT_EQ(5.2f, vertices[2].position.y);
	EXPECT_EQ(14.5f, vertices[3].position.x);
	EXPECT_EQ(13.6f, vertices[3].position.y);
	EXPECT_EQ(10.1f, vertices[4].position.x);
	EXPECT_EQ(19.2f, vertices[4].position.y);
	EXPECT_EQ(17.0f, vertices[5].position.x);
	EXPECT_EQ(22.0f, vertices[5].position.y);
	EXPECT_EQ(21.0f, vertices[6].position.x);
	EXPECT_EQ(14.5f, vertices[6].position.y);
	EXPECT_EQ(18.4f, vertices[7].position.x);
	EXPECT_EQ(7.3f, vertices[7].position.y);
	EXPECT_EQ(33.1f, vertices[8].position.x);
	EXPECT_EQ(0.0f, vertices[8].position.y);
	EXPECT_EQ(38.0f, vertices[9].position.x);
	EXPECT_EQ(4.8f, vertices[9].position.y);
	EXPECT_EQ(33.1f, vertices[10].position.x);
	EXPECT_EQ(10.6f, vertices[10].position.y);
	EXPECT_EQ(26.8f, vertices[11].position.x);
	EXPECT_EQ(12.5f, vertices[11].position.y);
	EXPECT_EQ(37.4f, vertices[12].position.x);
	EXPECT_EQ(17.1f, vertices[12].position.y);
	EXPECT_EQ(29.0f, vertices[13].position.x);
	EXPECT_EQ(21.7f, vertices[13].position.y);
	EXPECT_EQ(37.6f, vertices[14].position.x);
	EXPECT_EQ(24.1f, vertices[14].position.y);
	EXPECT_EQ(43.9f, vertices[15].position.x);
	EXPECT_EQ(21.4f, vertices[15].position.y);
	EXPECT_EQ(42.1f, vertices[16].position.x);
	EXPECT_EQ(10.3f, vertices[16].position.y);
	EXPECT_EQ(51.7f, vertices[17].position.x);
	EXPECT_EQ(5.7f, vertices[17].position.y);
	EXPECT_EQ(63.4f, vertices[18].position.x);
	EXPECT_EQ(5.7f, vertices[18].position.y);
	EXPECT_EQ(60.2f, vertices[19].position.x);
	EXPECT_EQ(17.0f, vertices[19].position.y);
	EXPECT_EQ(54.1f, vertices[20].position.x);
	EXPECT_EQ(12.9f, vertices[20].position.y);
	EXPECT_EQ(47.1f, vertices[21].position.x);
	EXPECT_EQ(24.0f, vertices[21].position.y);
	EXPECT_EQ(69.5f, vertices[22].position.x);
	EXPECT_EQ(23.0f, vertices[22].position.y);
	EXPECT_EQ(62.4f, vertices[23].position.x);
	EXPECT_EQ(31.5f, vertices[23].position.y);
	EXPECT_EQ(64.6f, vertices[24].position.x);
	EXPECT_EQ(45.6f, vertices[24].position.y);
	EXPECT_EQ(60.5f, vertices[25].position.x);
	EXPECT_EQ(37.0f, vertices[25].position.y);
	EXPECT_EQ(54.4f, vertices[26].position.x);
	EXPECT_EQ(34.9f, vertices[26].position.y);
	EXPECT_EQ(58.1f, vertices[27].position.x);
	EXPECT_EQ(27.2f, vertices[27].position.y);
	EXPECT_EQ(40.7f, vertices[28].position.x);
	EXPECT_EQ(30.2f, vertices[28].position.y);
	EXPECT_EQ(52.5f, vertices[29].position.x);
	EXPECT_EQ(33.0f, vertices[29].position.y);
	EXPECT_EQ(45.3f, vertices[30].position.x);
	EXPECT_EQ(41.2f, vertices[30].position.y);
	EXPECT_EQ(36.5f, vertices[31].position.x);
	EXPECT_EQ(37.9f, vertices[31].position.y);
	EXPECT_EQ(33.1f, vertices[32].position.x);
	EXPECT_EQ(27.8f, vertices[32].position.y);
	EXPECT_EQ(23.9f, vertices[33].position.x);
	EXPECT_EQ(26.8f, vertices[33].position.y);
	EXPECT_EQ(14.5f, vertices[34].position.x);
	EXPECT_EQ(29.9f, vertices[34].position.y);
	EXPECT_EQ(26.8f, vertices[35].position.x);
	EXPECT_EQ(31.8f, vertices[35].position.y);
	EXPECT_EQ(25.7f, vertices[36].position.x);
	EXPECT_EQ(37.1f, vertices[36].position.y);
	EXPECT_EQ(18.9f, vertices[37].position.x);
	EXPECT_EQ(41.4f, vertices[37].position.y);
	EXPECT_EQ(8.4f, vertices[38].position.x);
	EXPECT_EQ(38.2f, vertices[38].position.y);

	const uint16_t* indices = (const uint16_t*)((const uint8_t*)data + 39*sizeof(ShapeVertex));
	// First loop
	EXPECT_EQ(38U, indices[0]);
	EXPECT_EQ(1U, indices[1]);
	EXPECT_EQ(0U, indices[2]);

	EXPECT_EQ(4U, indices[3]);
	EXPECT_EQ(1U, indices[4]);
	EXPECT_EQ(38U, indices[5]);

	EXPECT_EQ(3U, indices[6]);
	EXPECT_EQ(1U, indices[7]);
	EXPECT_EQ(4U, indices[8]);

	EXPECT_EQ(2U, indices[9]);
	EXPECT_EQ(1U, indices[10]);
	EXPECT_EQ(3U, indices[11]);

	// Second loop
	EXPECT_EQ(5U, indices[12]);
	EXPECT_EQ(4U, indices[13]);
	EXPECT_EQ(34U, indices[14]);

	EXPECT_EQ(6U, indices[15]);
	EXPECT_EQ(5U, indices[16]);
	EXPECT_EQ(34U, indices[17]);

	EXPECT_EQ(33U, indices[18]);
	EXPECT_EQ(6U, indices[19]);
	EXPECT_EQ(34U, indices[20]);

	// Third loop
	EXPECT_EQ(11U, indices[21]);
	EXPECT_EQ(6U, indices[22]);
	EXPECT_EQ(33U, indices[23]);

	EXPECT_EQ(11U, indices[24]);
	EXPECT_EQ(7U, indices[25]);
	EXPECT_EQ(6U, indices[26]);

	EXPECT_EQ(8U, indices[27]);
	EXPECT_EQ(7U, indices[28]);
	EXPECT_EQ(11U, indices[29]);

	EXPECT_EQ(10U, indices[30]);
	EXPECT_EQ(8U, indices[31]);
	EXPECT_EQ(11U, indices[32]);

	EXPECT_EQ(9U, indices[33]);
	EXPECT_EQ(8U, indices[34]);
	EXPECT_EQ(10U, indices[35]);

	// Fourth loop
	EXPECT_EQ(12U, indices[36]);
	EXPECT_EQ(11U, indices[37]);
	EXPECT_EQ(13U, indices[38]);

	// Fifth loop
	EXPECT_EQ(13U, indices[39]);
	EXPECT_EQ(11U, indices[40]);
	EXPECT_EQ(33U, indices[41]);

	EXPECT_EQ(32U, indices[42]);
	EXPECT_EQ(13U, indices[43]);
	EXPECT_EQ(33U, indices[44]);

	EXPECT_EQ(14U, indices[45]);
	EXPECT_EQ(13U, indices[46]);
	EXPECT_EQ(32U, indices[47]);

	EXPECT_EQ(14U, indices[48]);
	EXPECT_EQ(32U, indices[49]);
	EXPECT_EQ(31U, indices[50]);

	EXPECT_EQ(28U, indices[51]);
	EXPECT_EQ(14U, indices[52]);
	EXPECT_EQ(31U, indices[53]);

	EXPECT_EQ(30U, indices[54]);
	EXPECT_EQ(28U, indices[55]);
	EXPECT_EQ(31U, indices[56]);

	EXPECT_EQ(29U, indices[57]);
	EXPECT_EQ(28U, indices[58]);
	EXPECT_EQ(30U, indices[59]);

	// Sixth loop
	EXPECT_EQ(15U, indices[60]);
	EXPECT_EQ(14U, indices[61]);
	EXPECT_EQ(28U, indices[62]);

	EXPECT_EQ(21U, indices[63]);
	EXPECT_EQ(15U, indices[64]);
	EXPECT_EQ(28U, indices[65]);

	EXPECT_EQ(27U, indices[66]);
	EXPECT_EQ(21U, indices[67]);
	EXPECT_EQ(28U, indices[68]);

	EXPECT_EQ(23U, indices[69]);
	EXPECT_EQ(27U, indices[70]);
	EXPECT_EQ(25U, indices[71]);

	EXPECT_EQ(22U, indices[72]);
	EXPECT_EQ(27U, indices[73]);
	EXPECT_EQ(23U, indices[74]);

	EXPECT_EQ(22U, indices[75]);
	EXPECT_EQ(21U, indices[76]);
	EXPECT_EQ(27U, indices[77]);

	// Seventh loop
	EXPECT_EQ(21U, indices[78]);
	EXPECT_EQ(16U, indices[79]);
	EXPECT_EQ(15U, indices[80]);

	EXPECT_EQ(17U, indices[81]);
	EXPECT_EQ(16U, indices[82]);
	EXPECT_EQ(21U, indices[83]);

	EXPECT_EQ(20U, indices[84]);
	EXPECT_EQ(17U, indices[85]);
	EXPECT_EQ(21U, indices[86]);

	EXPECT_EQ(19U, indices[87]);
	EXPECT_EQ(17U, indices[88]);
	EXPECT_EQ(20U, indices[89]);

	EXPECT_EQ(18U, indices[90]);
	EXPECT_EQ(17U, indices[91]);
	EXPECT_EQ(19U, indices[92]);

	// Eigth loop
	EXPECT_EQ(24U, indices[93]);
	EXPECT_EQ(23U, indices[94]);
	EXPECT_EQ(25U, indices[95]);

	// Ninth loop
	EXPECT_EQ(25U, indices[96]);
	EXPECT_EQ(27U, indices[97]);
	EXPECT_EQ(26U, indices[98]);

	// Tenth loop
	EXPECT_EQ(34U, indices[99]);
	EXPECT_EQ(4U, indices[100]);
	EXPECT_EQ(38U, indices[101]);

	EXPECT_EQ(37U, indices[102]);
	EXPECT_EQ(34U, indices[103]);
	EXPECT_EQ(38U, indices[104]);

	EXPECT_EQ(36U, indices[105]);
	EXPECT_EQ(34U, indices[106]);
	EXPECT_EQ(37U, indices[107]);

	EXPECT_EQ(35U, indices[108]);
	EXPECT_EQ(34U, indices[109]);
	EXPECT_EQ(36U, indices[110]);

	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsVectorImage_destroy(image));
	dsVectorScratchData_destroy(scratchData);
}

TEST_F(TriangulateTest, ComplexCCW)
{
	dsVectorMaterialSet* materialSet = dsVectorMaterialSet_create((dsAllocator*)&allocator,
		resourceManager, NULL, 1);
	dsVectorMaterial material;
	dsColor color = {{255, 255, 255, 255}};
	ASSERT_TRUE(dsVectorMaterial_setColor(&material, color));
	ASSERT_TRUE(dsVectorMaterialSet_addMaterial(materialSet, "fill", &material, true));

	dsVectorScratchData* scratchData = dsVectorScratchData_create((dsAllocator*)&allocator);
	dsVectorCommand commands[42];
	commands[0].commandType = dsVectorCommandType_StartPath;
	dsMatrix33_identity(commands[0].startPath.transform);
	commands[1].commandType = dsVectorCommandType_Move;
	commands[1].move.position.x = 0.0f;
	commands[1].move.position.y = 26.0f;
	commands[2].commandType = dsVectorCommandType_Line;
	commands[2].line.end.x = 8.4f;
	commands[2].line.end.y = 38.2f;
	commands[3].commandType = dsVectorCommandType_Line;
	commands[3].line.end.x = 18.9f;
	commands[3].line.end.y = 41.4f;
	commands[4].commandType = dsVectorCommandType_Line;
	commands[4].line.end.x = 25.7f;
	commands[4].line.end.y = 37.1f;
	commands[5].commandType = dsVectorCommandType_Line;
	commands[5].line.end.x = 26.8f;
	commands[5].line.end.y = 31.8f;
	commands[6].commandType = dsVectorCommandType_Line;
	commands[6].line.end.x = 14.5f;
	commands[6].line.end.y = 29.9f;
	commands[7].commandType = dsVectorCommandType_Line;
	commands[7].line.end.x = 23.9f;
	commands[7].line.end.y = 26.8f;
	commands[8].commandType = dsVectorCommandType_Line;
	commands[8].line.end.x = 33.1f;
	commands[8].line.end.y = 27.8f;
	commands[9].commandType = dsVectorCommandType_Line;
	commands[9].line.end.x = 36.5f;
	commands[9].line.end.y = 37.9f;
	commands[10].commandType = dsVectorCommandType_Line;
	commands[10].line.end.x = 45.3f;
	commands[10].line.end.y = 41.2f;
	commands[11].commandType = dsVectorCommandType_Line;
	commands[11].line.end.x = 52.5f;
	commands[11].line.end.y = 33.0f;
	commands[12].commandType = dsVectorCommandType_Line;
	commands[12].line.end.x = 40.7f;
	commands[12].line.end.y = 30.2f;
	commands[13].commandType = dsVectorCommandType_Line;
	commands[13].line.end.x = 58.1f;
	commands[13].line.end.y = 27.2f;
	commands[14].commandType = dsVectorCommandType_Line;
	commands[14].line.end.x = 54.4f;
	commands[14].line.end.y = 34.9f;
	commands[15].commandType = dsVectorCommandType_Line;
	commands[15].line.end.x = 60.5f;
	commands[15].line.end.y = 37.0f;
	commands[16].commandType = dsVectorCommandType_Line;
	commands[16].line.end.x = 64.6f;
	commands[16].line.end.y = 45.6f;
	commands[17].commandType = dsVectorCommandType_Line;
	commands[17].line.end.x = 62.4f;
	commands[17].line.end.y = 31.5f;
	commands[18].commandType = dsVectorCommandType_Line;
	commands[18].line.end.x = 69.5f;
	commands[18].line.end.y = 23.0f;
	commands[19].commandType = dsVectorCommandType_Line;
	commands[19].line.end.x = 47.1f;
	commands[19].line.end.y = 24.0f;
	commands[20].commandType = dsVectorCommandType_Line;
	commands[20].line.end.x = 54.1f;
	commands[20].line.end.y = 12.9f;
	commands[21].commandType = dsVectorCommandType_Line;
	commands[21].line.end.x = 60.2f;
	commands[21].line.end.y = 17.0f;
	commands[22].commandType = dsVectorCommandType_Line;
	commands[22].line.end.x = 63.4f;
	commands[22].line.end.y = 5.7f;
	commands[23].commandType = dsVectorCommandType_Line;
	commands[23].line.end.x = 51.7f;
	commands[23].line.end.y = 5.7f;
	commands[24].commandType = dsVectorCommandType_Line;
	commands[24].line.end.x = 42.1f;
	commands[24].line.end.y = 10.3f;
	commands[25].commandType = dsVectorCommandType_Line;
	commands[25].line.end.x = 43.9f;
	commands[25].line.end.y = 21.4f;
	commands[26].commandType = dsVectorCommandType_Line;
	commands[26].line.end.x = 37.6f;
	commands[26].line.end.y = 24.1f;
	commands[27].commandType = dsVectorCommandType_Line;
	commands[27].line.end.x = 29.0f;
	commands[27].line.end.y = 21.7f;
	commands[28].commandType = dsVectorCommandType_Line;
	commands[28].line.end.x = 37.4f;
	commands[28].line.end.y = 17.1f;
	commands[29].commandType = dsVectorCommandType_Line;
	commands[29].line.end.x = 26.8f;
	commands[29].line.end.y = 12.5f;
	commands[30].commandType = dsVectorCommandType_Line;
	commands[30].line.end.x = 33.1f;
	commands[30].line.end.y = 10.6f;
	commands[31].commandType = dsVectorCommandType_Line;
	commands[31].line.end.x = 38.0f;
	commands[31].line.end.y = 4.8f;
	commands[32].commandType = dsVectorCommandType_Line;
	commands[32].line.end.x = 33.1f;
	commands[32].line.end.y = 0.0f;
	commands[33].commandType = dsVectorCommandType_Line;
	commands[33].line.end.x = 18.4f;
	commands[33].line.end.y = 7.3f;
	commands[34].commandType = dsVectorCommandType_Line;
	commands[34].line.end.x = 21.0f;
	commands[34].line.end.y = 14.5f;
	commands[35].commandType = dsVectorCommandType_Line;
	commands[35].line.end.x = 17.0f;
	commands[35].line.end.y = 22.0f;
	commands[36].commandType = dsVectorCommandType_Line;
	commands[36].line.end.x = 10.1f;
	commands[36].line.end.y = 19.2f;
	commands[37].commandType = dsVectorCommandType_Line;
	commands[37].line.end.x = 14.5f;
	commands[37].line.end.y = 13.6f;
	commands[38].commandType = dsVectorCommandType_Line;
	commands[38].line.end.x = 16.0f;
	commands[38].line.end.y = 5.2f;
	commands[39].commandType = dsVectorCommandType_Line;
	commands[39].line.end.x = 5.4f;
	commands[39].line.end.y = 7.6f;
	commands[40].commandType = dsVectorCommandType_ClosePath;
	commands[41].commandType = dsVectorCommandType_FillPath;
	commands[41].fillPath.material = "fill";
	commands[41].fillPath.opacity = 1.0f;

	dsVector2f size = {{36.0f, 16.0f}};
	dsVectorImage* image = dsVectorImage_create((dsAllocator*)&allocator, scratchData,
		resourceManager, NULL, commands, DS_ARRAY_SIZE(commands), materialSet, true,
		NULL, &size, 0.1f);
	ASSERT_TRUE(image);

	dsGfxBuffer* buffer = dsVectorImage_getBuffer(image);
	ASSERT_TRUE(buffer);
	ASSERT_EQ(sizeof(ShapeVertex)*39 + sizeof(uint16_t)*111, buffer->size);

	const void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, buffer->size);
	ASSERT_TRUE(data);

	const ShapeVertex* vertices = (const ShapeVertex*)data;
	EXPECT_EQ(0.0f, vertices[0].position.x);
	EXPECT_EQ(26.0f, vertices[0].position.y);
	EXPECT_EQ(8.4f, vertices[1].position.x);
	EXPECT_EQ(38.2f, vertices[1].position.y);
	EXPECT_EQ(18.9f, vertices[2].position.x);
	EXPECT_EQ(41.4f, vertices[2].position.y);
	EXPECT_EQ(25.7f, vertices[3].position.x);
	EXPECT_EQ(37.1f, vertices[3].position.y);
	EXPECT_EQ(26.8f, vertices[4].position.x);
	EXPECT_EQ(31.8f, vertices[4].position.y);
	EXPECT_EQ(14.5f, vertices[5].position.x);
	EXPECT_EQ(29.9f, vertices[5].position.y);
	EXPECT_EQ(23.9f, vertices[6].position.x);
	EXPECT_EQ(26.8f, vertices[6].position.y);
	EXPECT_EQ(33.1f, vertices[7].position.x);
	EXPECT_EQ(27.8f, vertices[7].position.y);
	EXPECT_EQ(36.5f, vertices[8].position.x);
	EXPECT_EQ(37.9f, vertices[8].position.y);
	EXPECT_EQ(45.3f, vertices[9].position.x);
	EXPECT_EQ(41.2f, vertices[9].position.y);
	EXPECT_EQ(52.5f, vertices[10].position.x);
	EXPECT_EQ(33.0f, vertices[10].position.y);
	EXPECT_EQ(40.7f, vertices[11].position.x);
	EXPECT_EQ(30.2f, vertices[11].position.y);
	EXPECT_EQ(58.1f, vertices[12].position.x);
	EXPECT_EQ(27.2f, vertices[12].position.y);
	EXPECT_EQ(54.4f, vertices[13].position.x);
	EXPECT_EQ(34.9f, vertices[13].position.y);
	EXPECT_EQ(60.5f, vertices[14].position.x);
	EXPECT_EQ(37.0f, vertices[14].position.y);
	EXPECT_EQ(64.6f, vertices[15].position.x);
	EXPECT_EQ(45.6f, vertices[15].position.y);
	EXPECT_EQ(62.4f, vertices[16].position.x);
	EXPECT_EQ(31.5f, vertices[16].position.y);
	EXPECT_EQ(69.5f, vertices[17].position.x);
	EXPECT_EQ(23.0f, vertices[17].position.y);
	EXPECT_EQ(47.1f, vertices[18].position.x);
	EXPECT_EQ(24.0f, vertices[18].position.y);
	EXPECT_EQ(54.1f, vertices[19].position.x);
	EXPECT_EQ(12.9f, vertices[19].position.y);
	EXPECT_EQ(60.2f, vertices[20].position.x);
	EXPECT_EQ(17.0f, vertices[20].position.y);
	EXPECT_EQ(63.4f, vertices[21].position.x);
	EXPECT_EQ(5.7f, vertices[21].position.y);
	EXPECT_EQ(51.7f, vertices[22].position.x);
	EXPECT_EQ(5.7f, vertices[22].position.y);
	EXPECT_EQ(42.1f, vertices[23].position.x);
	EXPECT_EQ(10.3f, vertices[23].position.y);
	EXPECT_EQ(43.9f, vertices[24].position.x);
	EXPECT_EQ(21.4f, vertices[24].position.y);
	EXPECT_EQ(37.6f, vertices[25].position.x);
	EXPECT_EQ(24.1f, vertices[25].position.y);
	EXPECT_EQ(29.0f, vertices[26].position.x);
	EXPECT_EQ(21.7f, vertices[26].position.y);
	EXPECT_EQ(37.4f, vertices[27].position.x);
	EXPECT_EQ(17.1f, vertices[27].position.y);
	EXPECT_EQ(26.8f, vertices[28].position.x);
	EXPECT_EQ(12.5f, vertices[28].position.y);
	EXPECT_EQ(33.1f, vertices[29].position.x);
	EXPECT_EQ(10.6f, vertices[29].position.y);
	EXPECT_EQ(38.0f, vertices[30].position.x);
	EXPECT_EQ(4.8f, vertices[30].position.y);
	EXPECT_EQ(33.1f, vertices[31].position.x);
	EXPECT_EQ(0.0f, vertices[31].position.y);
	EXPECT_EQ(18.4f, vertices[32].position.x);
	EXPECT_EQ(7.3f, vertices[32].position.y);
	EXPECT_EQ(21.0f, vertices[33].position.x);
	EXPECT_EQ(14.5f, vertices[33].position.y);
	EXPECT_EQ(17.0f, vertices[34].position.x);
	EXPECT_EQ(22.0f, vertices[34].position.y);
	EXPECT_EQ(10.1f, vertices[35].position.x);
	EXPECT_EQ(19.2f, vertices[35].position.y);
	EXPECT_EQ(14.5f, vertices[36].position.x);
	EXPECT_EQ(13.6f, vertices[36].position.y);
	EXPECT_EQ(16.0f, vertices[37].position.x);
	EXPECT_EQ(5.2f, vertices[37].position.y);
	EXPECT_EQ(5.4f, vertices[38].position.x);
	EXPECT_EQ(7.6f, vertices[38].position.y);

	const uint16_t* indices = (const uint16_t*)((const uint8_t*)data + 39*sizeof(ShapeVertex));
	// First loop
	EXPECT_EQ(1U, indices[0]);
	EXPECT_EQ(38U, indices[1]);
	EXPECT_EQ(0U, indices[2]);

	EXPECT_EQ(35U, indices[3]);
	EXPECT_EQ(38U, indices[4]);
	EXPECT_EQ(1U, indices[5]);

	EXPECT_EQ(36U, indices[6]);
	EXPECT_EQ(38U, indices[7]);
	EXPECT_EQ(35U, indices[8]);

	EXPECT_EQ(37U, indices[9]);
	EXPECT_EQ(38U, indices[10]);
	EXPECT_EQ(36U, indices[11]);

	// Second loop
	EXPECT_EQ(5U, indices[12]);
	EXPECT_EQ(35U, indices[13]);
	EXPECT_EQ(1U, indices[14]);

	EXPECT_EQ(2U, indices[15]);
	EXPECT_EQ(5U, indices[16]);
	EXPECT_EQ(1U, indices[17]);

	EXPECT_EQ(3U, indices[18]);
	EXPECT_EQ(5U, indices[19]);
	EXPECT_EQ(2U, indices[20]);

	EXPECT_EQ(4U, indices[21]);
	EXPECT_EQ(5U, indices[22]);
	EXPECT_EQ(3U, indices[23]);

	// Third loop
	EXPECT_EQ(34U, indices[24]);
	EXPECT_EQ(35U, indices[25]);
	EXPECT_EQ(5U, indices[26]);

	EXPECT_EQ(33U, indices[27]);
	EXPECT_EQ(34U, indices[28]);
	EXPECT_EQ(5U, indices[29]);

	EXPECT_EQ(6U, indices[30]);
	EXPECT_EQ(33U, indices[31]);
	EXPECT_EQ(5U, indices[32]);

	// Fourth loop
	EXPECT_EQ(26U, indices[33]);
	EXPECT_EQ(28U, indices[34]);
	EXPECT_EQ(6U, indices[35]);

	EXPECT_EQ(7U, indices[36]);
	EXPECT_EQ(26U, indices[37]);
	EXPECT_EQ(6U, indices[38]);

	EXPECT_EQ(25U, indices[39]);
	EXPECT_EQ(26U, indices[40]);
	EXPECT_EQ(7U, indices[41]);

	EXPECT_EQ(25U, indices[42]);
	EXPECT_EQ(7U, indices[43]);
	EXPECT_EQ(8U, indices[44]);

	EXPECT_EQ(11U, indices[45]);
	EXPECT_EQ(25U, indices[46]);
	EXPECT_EQ(8U, indices[47]);

	EXPECT_EQ(9U, indices[48]);
	EXPECT_EQ(11U, indices[49]);
	EXPECT_EQ(8U, indices[50]);

	EXPECT_EQ(10U, indices[51]);
	EXPECT_EQ(11U, indices[52]);
	EXPECT_EQ(9U, indices[53]);

	// Fifth loop
	EXPECT_EQ(24U, indices[54]);
	EXPECT_EQ(25U, indices[55]);
	EXPECT_EQ(11U, indices[56]);

	EXPECT_EQ(18U, indices[57]);
	EXPECT_EQ(24U, indices[58]);
	EXPECT_EQ(11U, indices[59]);

	EXPECT_EQ(12U, indices[60]);
	EXPECT_EQ(18U, indices[61]);
	EXPECT_EQ(11U, indices[62]);

	EXPECT_EQ(16U, indices[63]);
	EXPECT_EQ(12U, indices[64]);
	EXPECT_EQ(14U, indices[65]);

	EXPECT_EQ(17U, indices[66]);
	EXPECT_EQ(12U, indices[67]);
	EXPECT_EQ(16U, indices[68]);

	EXPECT_EQ(17U, indices[69]);
	EXPECT_EQ(18U, indices[70]);
	EXPECT_EQ(12U, indices[71]);

	// Sixth loop
	EXPECT_EQ(14U, indices[72]);
	EXPECT_EQ(12U, indices[73]);
	EXPECT_EQ(13U, indices[74]);

	// Seventh loop
	EXPECT_EQ(15U, indices[75]);
	EXPECT_EQ(16U, indices[76]);
	EXPECT_EQ(14U, indices[77]);

	// Eighth loop
	EXPECT_EQ(18U, indices[78]);
	EXPECT_EQ(23U, indices[79]);
	EXPECT_EQ(24U, indices[80]);

	EXPECT_EQ(22U, indices[81]);
	EXPECT_EQ(23U, indices[82]);
	EXPECT_EQ(18U, indices[83]);

	EXPECT_EQ(19U, indices[84]);
	EXPECT_EQ(22U, indices[85]);
	EXPECT_EQ(18U, indices[86]);

	EXPECT_EQ(20U, indices[87]);
	EXPECT_EQ(22U, indices[88]);
	EXPECT_EQ(19U, indices[89]);

	EXPECT_EQ(21U, indices[90]);
	EXPECT_EQ(22U, indices[91]);
	EXPECT_EQ(20U, indices[92]);

	// Ninth loop
	EXPECT_EQ(27U, indices[93]);
	EXPECT_EQ(28U, indices[94]);
	EXPECT_EQ(26U, indices[95]);

	// Tenth loop
	EXPECT_EQ(28U, indices[96]);
	EXPECT_EQ(33U, indices[97]);
	EXPECT_EQ(6U, indices[98]);

	EXPECT_EQ(28U, indices[99]);
	EXPECT_EQ(32U, indices[100]);
	EXPECT_EQ(33U, indices[101]);

	EXPECT_EQ(31U, indices[102]);
	EXPECT_EQ(32U, indices[103]);
	EXPECT_EQ(28U, indices[104]);

	EXPECT_EQ(29U, indices[105]);
	EXPECT_EQ(31U, indices[106]);
	EXPECT_EQ(28U, indices[107]);

	EXPECT_EQ(30U, indices[108]);
	EXPECT_EQ(31U, indices[109]);
	EXPECT_EQ(29U, indices[110]);

	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsVectorImage_destroy(image));
	dsVectorScratchData_destroy(scratchData);
}

TEST_F(TriangulateTest, SawtoothRightCW)
{
	// Test a combination of vertices that do and don't line up exactly.
	dsVectorMaterialSet* materialSet = dsVectorMaterialSet_create((dsAllocator*)&allocator,
		resourceManager, NULL, 1);
	dsVectorMaterial material;
	dsColor color = {{255, 255, 255, 255}};
	ASSERT_TRUE(dsVectorMaterial_setColor(&material, color));
	ASSERT_TRUE(dsVectorMaterialSet_addMaterial(materialSet, "fill", &material, true));

	dsVectorScratchData* scratchData = dsVectorScratchData_create((dsAllocator*)&allocator);
	dsVectorCommand commands[22];
	commands[0].commandType = dsVectorCommandType_StartPath;
	dsMatrix33_identity(commands[0].startPath.transform);
	commands[1].commandType = dsVectorCommandType_Move;
	commands[1].move.position.x = 0.0f;
	commands[1].move.position.y = 0.0f;
	commands[2].commandType = dsVectorCommandType_Line;
	commands[2].line.end.x = 10.0f;
	commands[2].line.end.y = 0.0f;
	commands[3].commandType = dsVectorCommandType_Line;
	commands[3].line.end.x = 11.0f;
	commands[3].line.end.y = 1.0f;
	commands[4].commandType = dsVectorCommandType_Line;
	commands[4].line.end.x = 10.0f;
	commands[4].line.end.y = 2.0f;
	commands[5].commandType = dsVectorCommandType_Line;
	commands[5].line.end.x = 11.0f;
	commands[5].line.end.y = 3.0f;
	commands[6].commandType = dsVectorCommandType_Line;
	commands[6].line.end.x = 10.0f;
	commands[6].line.end.y = 4.0f;
	commands[7].commandType = dsVectorCommandType_Line;
	commands[7].line.end.x = 11.0f;
	commands[7].line.end.y = 5.0f;
	commands[8].commandType = dsVectorCommandType_Line;
	commands[8].line.end.x = 9.5f;
	commands[8].line.end.y = 6.0f;
	commands[9].commandType = dsVectorCommandType_Line;
	commands[9].line.end.x = 11.0f;
	commands[9].line.end.y = 7.0f;
	commands[10].commandType = dsVectorCommandType_Line;
	commands[10].line.end.x = 10.0f;
	commands[10].line.end.y = 8.0f;
	commands[11].commandType = dsVectorCommandType_Line;
	commands[11].line.end.x = 11.0f;
	commands[11].line.end.y = 9.0f;
	commands[12].commandType = dsVectorCommandType_Line;
	commands[12].line.end.x = 10.0f;
	commands[12].line.end.y = 10.0f;
	commands[13].commandType = dsVectorCommandType_Line;
	commands[13].line.end.x = 11.0f;
	commands[13].line.end.y = 11.0f;
	commands[14].commandType = dsVectorCommandType_Line;
	commands[14].line.end.x = 10.5f;
	commands[14].line.end.y = 12.0f;
	commands[15].commandType = dsVectorCommandType_Line;
	commands[15].line.end.x = 11.0f;
	commands[15].line.end.y = 13.0f;
	commands[16].commandType = dsVectorCommandType_Line;
	commands[16].line.end.x = 10.0f;
	commands[16].line.end.y = 14.0f;
	commands[17].commandType = dsVectorCommandType_Line;
	commands[17].line.end.x = 11.0f;
	commands[17].line.end.y = 15.0f;
	commands[18].commandType = dsVectorCommandType_Line;
	commands[18].line.end.x = 10.0f;
	commands[18].line.end.y = 16.0f;
	commands[19].commandType = dsVectorCommandType_Line;
	commands[19].line.end.x = 0.0f;
	commands[19].line.end.y = 16.0f;
	commands[20].commandType = dsVectorCommandType_ClosePath;
	commands[21].commandType = dsVectorCommandType_FillPath;
	commands[21].fillPath.material = "fill";
	commands[21].fillPath.opacity = 1.0f;

	dsVector2f size = {{11.0f, 16.0f}};
	dsVectorImage* image = dsVectorImage_create((dsAllocator*)&allocator, scratchData,
		resourceManager, NULL, commands, DS_ARRAY_SIZE(commands), materialSet, true,
		NULL, &size, 0.1f);
	ASSERT_TRUE(image);

	dsGfxBuffer* buffer = dsVectorImage_getBuffer(image);
	ASSERT_TRUE(buffer);
	ASSERT_EQ(sizeof(ShapeVertex)*19 + sizeof(uint16_t)*51, buffer->size);

	const void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, buffer->size);
	ASSERT_TRUE(data);

	const ShapeVertex* vertices = (const ShapeVertex*)data;
	EXPECT_EQ(0.0f, vertices[0].position.x);
	EXPECT_EQ(0.0f, vertices[0].position.y);
	EXPECT_EQ(10.0f, vertices[1].position.x);
	EXPECT_EQ(0.0f, vertices[1].position.y);
	EXPECT_EQ(11.0f, vertices[2].position.x);
	EXPECT_EQ(1.0f, vertices[2].position.y);
	EXPECT_EQ(10.0f, vertices[3].position.x);
	EXPECT_EQ(2.0f, vertices[3].position.y);
	EXPECT_EQ(11.0f, vertices[4].position.x);
	EXPECT_EQ(3.0f, vertices[4].position.y);
	EXPECT_EQ(10.0f, vertices[5].position.x);
	EXPECT_EQ(4.0f, vertices[5].position.y);
	EXPECT_EQ(11.0f, vertices[6].position.x);
	EXPECT_EQ(5.0f, vertices[6].position.y);
	EXPECT_EQ(9.5f, vertices[7].position.x);
	EXPECT_EQ(6.0f, vertices[7].position.y);
	EXPECT_EQ(11.0f, vertices[8].position.x);
	EXPECT_EQ(7.0f, vertices[8].position.y);
	EXPECT_EQ(10.0f, vertices[9].position.x);
	EXPECT_EQ(8.0f, vertices[9].position.y);
	EXPECT_EQ(11.0f, vertices[10].position.x);
	EXPECT_EQ(9.0f, vertices[10].position.y);
	EXPECT_EQ(10.0f, vertices[11].position.x);
	EXPECT_EQ(10.0f, vertices[11].position.y);
	EXPECT_EQ(11.0f, vertices[12].position.x);
	EXPECT_EQ(11.0f, vertices[12].position.y);
	EXPECT_EQ(10.5f, vertices[13].position.x);
	EXPECT_EQ(12.0f, vertices[13].position.y);
	EXPECT_EQ(11.0f, vertices[14].position.x);
	EXPECT_EQ(13.0f, vertices[14].position.y);
	EXPECT_EQ(10.0f, vertices[15].position.x);
	EXPECT_EQ(14.0f, vertices[15].position.y);
	EXPECT_EQ(11.0f, vertices[16].position.x);
	EXPECT_EQ(15.0f, vertices[16].position.y);
	EXPECT_EQ(10.0f, vertices[17].position.x);
	EXPECT_EQ(16.0f, vertices[17].position.y);
	EXPECT_EQ(0.0f, vertices[18].position.x);
	EXPECT_EQ(16.0f, vertices[18].position.y);

	const uint16_t* indices = (const uint16_t*)((const uint8_t*)data + 19*sizeof(ShapeVertex));
	EXPECT_EQ(7U, indices[0]);
	EXPECT_EQ(0U, indices[1]);
	EXPECT_EQ(18U, indices[2]);

	EXPECT_EQ(1U, indices[3]);
	EXPECT_EQ(0U, indices[4]);
	EXPECT_EQ(7U, indices[5]);

	EXPECT_EQ(3U, indices[6]);
	EXPECT_EQ(1U, indices[7]);
	EXPECT_EQ(7U, indices[8]);

	EXPECT_EQ(5U, indices[9]);
	EXPECT_EQ(3U, indices[10]);
	EXPECT_EQ(7U, indices[11]);

	EXPECT_EQ(6U, indices[12]);
	EXPECT_EQ(5U, indices[13]);
	EXPECT_EQ(7U, indices[14]);

	EXPECT_EQ(2U, indices[15]);
	EXPECT_EQ(1U, indices[16]);
	EXPECT_EQ(3U, indices[17]);

	EXPECT_EQ(4U, indices[18]);
	EXPECT_EQ(3U, indices[19]);
	EXPECT_EQ(5U, indices[20]);

	EXPECT_EQ(8U, indices[21]);
	EXPECT_EQ(7U, indices[22]);
	EXPECT_EQ(9U, indices[23]);

	EXPECT_EQ(10U, indices[24]);
	EXPECT_EQ(9U, indices[25]);
	EXPECT_EQ(11U, indices[26]);

	EXPECT_EQ(13U, indices[27]);
	EXPECT_EQ(11U, indices[28]);
	EXPECT_EQ(15U, indices[29]);

	EXPECT_EQ(12U, indices[30]);
	EXPECT_EQ(11U, indices[31]);
	EXPECT_EQ(13U, indices[32]);

	EXPECT_EQ(14U, indices[33]);
	EXPECT_EQ(13U, indices[34]);
	EXPECT_EQ(15U, indices[35]);

	EXPECT_EQ(9U, indices[36]);
	EXPECT_EQ(7U, indices[37]);
	EXPECT_EQ(18U, indices[38]);

	EXPECT_EQ(11U, indices[39]);
	EXPECT_EQ(9U, indices[40]);
	EXPECT_EQ(18U, indices[41]);

	EXPECT_EQ(15U, indices[42]);
	EXPECT_EQ(11U, indices[43]);
	EXPECT_EQ(18U, indices[44]);

	EXPECT_EQ(17U, indices[45]);
	EXPECT_EQ(15U, indices[46]);
	EXPECT_EQ(18U, indices[47]);

	EXPECT_EQ(16U, indices[48]);
	EXPECT_EQ(15U, indices[49]);
	EXPECT_EQ(17U, indices[50]);

	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsVectorImage_destroy(image));
	dsVectorScratchData_destroy(scratchData);
}

TEST_F(TriangulateTest, SawtoothRightCCW)
{
	// Test a combination of vertices that do and don't line up exactly.
	dsVectorMaterialSet* materialSet = dsVectorMaterialSet_create((dsAllocator*)&allocator,
		resourceManager, NULL, 1);
	dsVectorMaterial material;
	dsColor color = {{255, 255, 255, 255}};
	ASSERT_TRUE(dsVectorMaterial_setColor(&material, color));
	ASSERT_TRUE(dsVectorMaterialSet_addMaterial(materialSet, "fill", &material, true));

	dsVectorScratchData* scratchData = dsVectorScratchData_create((dsAllocator*)&allocator);
	dsVectorCommand commands[22];
	commands[0].commandType = dsVectorCommandType_StartPath;
	dsMatrix33_identity(commands[0].startPath.transform);
	commands[1].commandType = dsVectorCommandType_Move;
	commands[1].move.position.x = 0.0f;
	commands[1].move.position.y = 0.0f;
	commands[2].commandType = dsVectorCommandType_Line;
	commands[2].line.end.x = 0.0f;
	commands[2].line.end.y = 16.0f;
	commands[3].commandType = dsVectorCommandType_Line;
	commands[3].line.end.x = 10.0f;
	commands[3].line.end.y = 16.0f;
	commands[4].commandType = dsVectorCommandType_Line;
	commands[4].line.end.x = 11.0f;
	commands[4].line.end.y = 15.0f;
	commands[5].commandType = dsVectorCommandType_Line;
	commands[5].line.end.x = 10.0f;
	commands[5].line.end.y = 14.0f;
	commands[6].commandType = dsVectorCommandType_Line;
	commands[6].line.end.x = 11.0f;
	commands[6].line.end.y = 13.0f;
	commands[7].commandType = dsVectorCommandType_Line;
	commands[7].line.end.x = 10.5f;
	commands[7].line.end.y = 12.0f;
	commands[8].commandType = dsVectorCommandType_Line;
	commands[8].line.end.x = 11.0f;
	commands[8].line.end.y = 11.0f;
	commands[9].commandType = dsVectorCommandType_Line;
	commands[9].line.end.x = 10.0f;
	commands[9].line.end.y = 10.0f;
	commands[10].commandType = dsVectorCommandType_Line;
	commands[10].line.end.x = 11.0f;
	commands[10].line.end.y = 9.0f;
	commands[11].commandType = dsVectorCommandType_Line;
	commands[11].line.end.x = 10.0f;
	commands[11].line.end.y = 8.0f;
	commands[12].commandType = dsVectorCommandType_Line;
	commands[12].line.end.x = 11.0f;
	commands[12].line.end.y = 7.0f;
	commands[13].commandType = dsVectorCommandType_Line;
	commands[13].line.end.x = 9.5f;
	commands[13].line.end.y = 6.0f;
	commands[14].commandType = dsVectorCommandType_Line;
	commands[14].line.end.x = 11.0f;
	commands[14].line.end.y = 5.0f;
	commands[15].commandType = dsVectorCommandType_Line;
	commands[15].line.end.x = 10.0f;
	commands[15].line.end.y = 4.0f;
	commands[16].commandType = dsVectorCommandType_Line;
	commands[16].line.end.x = 11.0f;
	commands[16].line.end.y = 3.0f;
	commands[17].commandType = dsVectorCommandType_Line;
	commands[17].line.end.x = 10.0f;
	commands[17].line.end.y = 2.0f;
	commands[18].commandType = dsVectorCommandType_Line;
	commands[18].line.end.x = 11.0f;
	commands[18].line.end.y = 1.0f;
	commands[19].commandType = dsVectorCommandType_Line;
	commands[19].line.end.x = 10.0f;
	commands[19].line.end.y = 0.0f;
	commands[20].commandType = dsVectorCommandType_ClosePath;
	commands[21].commandType = dsVectorCommandType_FillPath;
	commands[21].fillPath.material = "fill";
	commands[21].fillPath.opacity = 1.0f;

	dsVector2f size = {{11.0f, 16.0f}};
	dsVectorImage* image = dsVectorImage_create((dsAllocator*)&allocator, scratchData,
		resourceManager, NULL, commands, DS_ARRAY_SIZE(commands), materialSet, true,
		NULL, &size, 0.1f);
	ASSERT_TRUE(image);

	dsGfxBuffer* buffer = dsVectorImage_getBuffer(image);
	ASSERT_TRUE(buffer);
	ASSERT_EQ(sizeof(ShapeVertex)*19 + sizeof(uint16_t)*51, buffer->size);

	const void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, buffer->size);
	ASSERT_TRUE(data);

	const ShapeVertex* vertices = (const ShapeVertex*)data;
	EXPECT_EQ(0.0f, vertices[0].position.x);
	EXPECT_EQ(0.0f, vertices[0].position.y);
	EXPECT_EQ(0.0f, vertices[1].position.x);
	EXPECT_EQ(16.0f, vertices[1].position.y);
	EXPECT_EQ(10.0f, vertices[2].position.x);
	EXPECT_EQ(16.0f, vertices[2].position.y);
	EXPECT_EQ(11.0f, vertices[3].position.x);
	EXPECT_EQ(15.0f, vertices[3].position.y);
	EXPECT_EQ(10.0f, vertices[4].position.x);
	EXPECT_EQ(14.0f, vertices[4].position.y);
	EXPECT_EQ(11.0f, vertices[5].position.x);
	EXPECT_EQ(13.0f, vertices[5].position.y);
	EXPECT_EQ(10.5f, vertices[6].position.x);
	EXPECT_EQ(12.0f, vertices[6].position.y);
	EXPECT_EQ(11.0f, vertices[7].position.x);
	EXPECT_EQ(11.0f, vertices[7].position.y);
	EXPECT_EQ(10.0f, vertices[8].position.x);
	EXPECT_EQ(10.0f, vertices[8].position.y);
	EXPECT_EQ(11.0f, vertices[9].position.x);
	EXPECT_EQ(9.0f, vertices[9].position.y);
	EXPECT_EQ(10.0f, vertices[10].position.x);
	EXPECT_EQ(8.0f, vertices[10].position.y);
	EXPECT_EQ(11.0f, vertices[11].position.x);
	EXPECT_EQ(7.0f, vertices[11].position.y);
	EXPECT_EQ(9.5f, vertices[12].position.x);
	EXPECT_EQ(6.0f, vertices[12].position.y);
	EXPECT_EQ(11.0f, vertices[13].position.x);
	EXPECT_EQ(5.0f, vertices[13].position.y);
	EXPECT_EQ(10.0f, vertices[14].position.x);
	EXPECT_EQ(4.0f, vertices[14].position.y);
	EXPECT_EQ(11.0f, vertices[15].position.x);
	EXPECT_EQ(3.0f, vertices[15].position.y);
	EXPECT_EQ(10.0f, vertices[16].position.x);
	EXPECT_EQ(2.0f, vertices[16].position.y);
	EXPECT_EQ(11.0f, vertices[17].position.x);
	EXPECT_EQ(1.0f, vertices[17].position.y);
	EXPECT_EQ(10.0f, vertices[18].position.x);
	EXPECT_EQ(0.0f, vertices[18].position.y);

	const uint16_t* indices = (const uint16_t*)((const uint8_t*)data + 19*sizeof(ShapeVertex));
	EXPECT_EQ(12U, indices[0]);
	EXPECT_EQ(0U, indices[1]);
	EXPECT_EQ(1U, indices[2]);

	EXPECT_EQ(18U, indices[3]);
	EXPECT_EQ(0U, indices[4]);
	EXPECT_EQ(12U, indices[5]);

	EXPECT_EQ(16U, indices[6]);
	EXPECT_EQ(18U, indices[7]);
	EXPECT_EQ(12U, indices[8]);

	EXPECT_EQ(14U, indices[9]);
	EXPECT_EQ(16U, indices[10]);
	EXPECT_EQ(12U, indices[11]);

	EXPECT_EQ(13U, indices[12]);
	EXPECT_EQ(14U, indices[13]);
	EXPECT_EQ(12U, indices[14]);

	EXPECT_EQ(10U, indices[15]);
	EXPECT_EQ(12U, indices[16]);
	EXPECT_EQ(1U, indices[17]);

	EXPECT_EQ(8U, indices[18]);
	EXPECT_EQ(10U, indices[19]);
	EXPECT_EQ(1U, indices[20]);

	EXPECT_EQ(4U, indices[21]);
	EXPECT_EQ(8U, indices[22]);
	EXPECT_EQ(1U, indices[23]);

	EXPECT_EQ(2U, indices[24]);
	EXPECT_EQ(4U, indices[25]);
	EXPECT_EQ(1U, indices[26]);

	EXPECT_EQ(3U, indices[27]);
	EXPECT_EQ(4U, indices[28]);
	EXPECT_EQ(2U, indices[29]);

	EXPECT_EQ(5U, indices[30]);
	EXPECT_EQ(6U, indices[31]);
	EXPECT_EQ(4U, indices[32]);

	EXPECT_EQ(6U, indices[33]);
	EXPECT_EQ(8U, indices[34]);
	EXPECT_EQ(4U, indices[35]);

	EXPECT_EQ(7U, indices[36]);
	EXPECT_EQ(8U, indices[37]);
	EXPECT_EQ(6U, indices[38]);

	EXPECT_EQ(9U, indices[39]);
	EXPECT_EQ(10U, indices[40]);
	EXPECT_EQ(8U, indices[41]);

	EXPECT_EQ(11U, indices[42]);
	EXPECT_EQ(12U, indices[43]);
	EXPECT_EQ(10U, indices[44]);

	EXPECT_EQ(15U, indices[45]);
	EXPECT_EQ(16U, indices[46]);
	EXPECT_EQ(14U, indices[47]);

	EXPECT_EQ(17U, indices[48]);
	EXPECT_EQ(18U, indices[49]);
	EXPECT_EQ(16U, indices[50]);

	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsVectorImage_destroy(image));
	dsVectorScratchData_destroy(scratchData);
}

TEST_F(TriangulateTest, SawtoothLeftCW)
{
	// Test a combination of vertices that do and don't line up exactly.
	dsVectorMaterialSet* materialSet = dsVectorMaterialSet_create((dsAllocator*)&allocator,
		resourceManager, NULL, 1);
	dsVectorMaterial material;
	dsColor color = {{255, 255, 255, 255}};
	ASSERT_TRUE(dsVectorMaterial_setColor(&material, color));
	ASSERT_TRUE(dsVectorMaterialSet_addMaterial(materialSet, "fill", &material, true));

	dsVectorScratchData* scratchData = dsVectorScratchData_create((dsAllocator*)&allocator);
	dsVectorCommand commands[22];
	commands[0].commandType = dsVectorCommandType_StartPath;
	dsMatrix33_identity(commands[0].startPath.transform);
	commands[1].commandType = dsVectorCommandType_Move;
	commands[1].move.position.x = 1.0f;
	commands[1].move.position.y = 0.0f;
	commands[2].commandType = dsVectorCommandType_Line;
	commands[2].line.end.x = 10.0f;
	commands[2].line.end.y = 0.0f;
	commands[3].commandType = dsVectorCommandType_Line;
	commands[3].line.end.x = 10.0f;
	commands[3].line.end.y = 16.0f;
	commands[4].commandType = dsVectorCommandType_Line;
	commands[4].line.end.x = 1.0f;
	commands[4].line.end.y = 16.0f;
	commands[5].commandType = dsVectorCommandType_Line;
	commands[5].line.end.x = 0.0f;
	commands[5].line.end.y = 15.0f;
	commands[6].commandType = dsVectorCommandType_Line;
	commands[6].line.end.x = 1.0f;
	commands[6].line.end.y = 14.0f;
	commands[7].commandType = dsVectorCommandType_Line;
	commands[7].line.end.x = 0.0f;
	commands[7].line.end.y = 13.0f;
	commands[8].commandType = dsVectorCommandType_Line;
	commands[8].line.end.x = 0.5f;
	commands[8].line.end.y = 12.0f;
	commands[9].commandType = dsVectorCommandType_Line;
	commands[9].line.end.x = 0.0f;
	commands[9].line.end.y = 11.0f;
	commands[10].commandType = dsVectorCommandType_Line;
	commands[10].line.end.x = 1.0f;
	commands[10].line.end.y = 10.0f;
	commands[11].commandType = dsVectorCommandType_Line;
	commands[11].line.end.x = 0.0f;
	commands[11].line.end.y = 9.0f;
	commands[12].commandType = dsVectorCommandType_Line;
	commands[12].line.end.x = 1.0f;
	commands[12].line.end.y = 8.0f;
	commands[13].commandType = dsVectorCommandType_Line;
	commands[13].line.end.x = 0.0f;
	commands[13].line.end.y = 7.0f;
	commands[14].commandType = dsVectorCommandType_Line;
	commands[14].line.end.x = 1.5f;
	commands[14].line.end.y = 6.0f;
	commands[15].commandType = dsVectorCommandType_Line;
	commands[15].line.end.x = 0.0f;
	commands[15].line.end.y = 5.0f;
	commands[16].commandType = dsVectorCommandType_Line;
	commands[16].line.end.x = 1.0f;
	commands[16].line.end.y = 4.0f;
	commands[17].commandType = dsVectorCommandType_Line;
	commands[17].line.end.x = 0.0f;
	commands[17].line.end.y = 3.0f;
	commands[18].commandType = dsVectorCommandType_Line;
	commands[18].line.end.x = 1.0f;
	commands[18].line.end.y = 2.0f;
	commands[19].commandType = dsVectorCommandType_Line;
	commands[19].line.end.x = 0.0f;
	commands[19].line.end.y = 1.0f;
	commands[20].commandType = dsVectorCommandType_ClosePath;
	commands[21].commandType = dsVectorCommandType_FillPath;
	commands[21].fillPath.material = "fill";
	commands[21].fillPath.opacity = 1.0f;

	dsVector2f size = {{10.0f, 16.0f}};
	dsVectorImage* image = dsVectorImage_create((dsAllocator*)&allocator, scratchData,
		resourceManager, NULL, commands, DS_ARRAY_SIZE(commands), materialSet, true,
		NULL, &size, 0.1f);
	ASSERT_TRUE(image);

	dsGfxBuffer* buffer = dsVectorImage_getBuffer(image);
	ASSERT_TRUE(buffer);
	ASSERT_EQ(sizeof(ShapeVertex)*19 + sizeof(uint16_t)*51, buffer->size);

	const void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, buffer->size);
	ASSERT_TRUE(data);

	const ShapeVertex* vertices = (const ShapeVertex*)data;
	EXPECT_EQ(1.0f, vertices[0].position.x);
	EXPECT_EQ(0.0f, vertices[0].position.y);
	EXPECT_EQ(10.0f, vertices[1].position.x);
	EXPECT_EQ(0.0f, vertices[1].position.y);
	EXPECT_EQ(10.0f, vertices[2].position.x);
	EXPECT_EQ(16.0f, vertices[2].position.y);
	EXPECT_EQ(1.0f, vertices[3].position.x);
	EXPECT_EQ(16.0f, vertices[3].position.y);
	EXPECT_EQ(0.0f, vertices[4].position.x);
	EXPECT_EQ(15.0f, vertices[4].position.y);
	EXPECT_EQ(1.0f, vertices[5].position.x);
	EXPECT_EQ(14.0f, vertices[5].position.y);
	EXPECT_EQ(0.0f, vertices[6].position.x);
	EXPECT_EQ(13.0f, vertices[6].position.y);
	EXPECT_EQ(0.5f, vertices[7].position.x);
	EXPECT_EQ(12.0f, vertices[7].position.y);
	EXPECT_EQ(0.0f, vertices[8].position.x);
	EXPECT_EQ(11.0f, vertices[8].position.y);
	EXPECT_EQ(1.0f, vertices[9].position.x);
	EXPECT_EQ(10.0f, vertices[9].position.y);
	EXPECT_EQ(0.0f, vertices[10].position.x);
	EXPECT_EQ(9.0f, vertices[10].position.y);
	EXPECT_EQ(1.0f, vertices[11].position.x);
	EXPECT_EQ(8.0f, vertices[11].position.y);
	EXPECT_EQ(0.0f, vertices[12].position.x);
	EXPECT_EQ(7.0f, vertices[12].position.y);
	EXPECT_EQ(1.5f, vertices[13].position.x);
	EXPECT_EQ(6.0f, vertices[13].position.y);
	EXPECT_EQ(0.0f, vertices[14].position.x);
	EXPECT_EQ(5.0f, vertices[14].position.y);
	EXPECT_EQ(1.0f, vertices[15].position.x);
	EXPECT_EQ(4.0f, vertices[15].position.y);
	EXPECT_EQ(0.0f, vertices[16].position.x);
	EXPECT_EQ(3.0f, vertices[16].position.y);
	EXPECT_EQ(1.0f, vertices[17].position.x);
	EXPECT_EQ(2.0f, vertices[17].position.y);
	EXPECT_EQ(0.0f, vertices[18].position.x);
	EXPECT_EQ(1.0f, vertices[18].position.y);

	const uint16_t* indices = (const uint16_t*)((const uint8_t*)data + 19*sizeof(ShapeVertex));
	EXPECT_EQ(17U, indices[0]);
	EXPECT_EQ(0U, indices[1]);
	EXPECT_EQ(18U, indices[2]);

	EXPECT_EQ(13U, indices[3]);
	EXPECT_EQ(17U, indices[4]);
	EXPECT_EQ(15U, indices[5]);

	EXPECT_EQ(13U, indices[6]);
	EXPECT_EQ(0U, indices[7]);
	EXPECT_EQ(17U, indices[8]);

	EXPECT_EQ(1U, indices[9]);
	EXPECT_EQ(0U, indices[10]);
	EXPECT_EQ(13U, indices[11]);

	EXPECT_EQ(13U, indices[12]);
	EXPECT_EQ(12U, indices[13]);
	EXPECT_EQ(11U, indices[14]);

	EXPECT_EQ(13U, indices[15]);
	EXPECT_EQ(11U, indices[16]);
	EXPECT_EQ(9U, indices[17]);

	EXPECT_EQ(13U, indices[18]);
	EXPECT_EQ(9U, indices[19]);
	EXPECT_EQ(5U, indices[20]);

	EXPECT_EQ(13U, indices[21]);
	EXPECT_EQ(5U, indices[22]);
	EXPECT_EQ(3U, indices[23]);

	EXPECT_EQ(1U, indices[24]);
	EXPECT_EQ(13U, indices[25]);
	EXPECT_EQ(3U, indices[26]);

	EXPECT_EQ(2U, indices[27]);
	EXPECT_EQ(1U, indices[28]);
	EXPECT_EQ(3U, indices[29]);

	EXPECT_EQ(3U, indices[30]);
	EXPECT_EQ(5U, indices[31]);
	EXPECT_EQ(4U, indices[32]);

	EXPECT_EQ(5U, indices[33]);
	EXPECT_EQ(9U, indices[34]);
	EXPECT_EQ(7U, indices[35]);

	EXPECT_EQ(5U, indices[36]);
	EXPECT_EQ(7U, indices[37]);
	EXPECT_EQ(6U, indices[38]);

	EXPECT_EQ(9U, indices[39]);
	EXPECT_EQ(8U, indices[40]);
	EXPECT_EQ(7U, indices[41]);

	EXPECT_EQ(9U, indices[42]);
	EXPECT_EQ(11U, indices[43]);
	EXPECT_EQ(10U, indices[44]);

	EXPECT_EQ(13U, indices[45]);
	EXPECT_EQ(15U, indices[46]);
	EXPECT_EQ(14U, indices[47]);

	EXPECT_EQ(15U, indices[48]);
	EXPECT_EQ(17U, indices[49]);
	EXPECT_EQ(16U, indices[50]);

	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsVectorImage_destroy(image));
	dsVectorScratchData_destroy(scratchData);
}

TEST_F(TriangulateTest, SawtoothLeftCCW)
{
	// Test a combination of vertices that do and don't line up exactly.
	dsVectorMaterialSet* materialSet = dsVectorMaterialSet_create((dsAllocator*)&allocator,
		resourceManager, NULL, 1);
	dsVectorMaterial material;
	dsColor color = {{255, 255, 255, 255}};
	ASSERT_TRUE(dsVectorMaterial_setColor(&material, color));
	ASSERT_TRUE(dsVectorMaterialSet_addMaterial(materialSet, "fill", &material, true));

	dsVectorScratchData* scratchData = dsVectorScratchData_create((dsAllocator*)&allocator);
	dsVectorCommand commands[22];
	commands[0].commandType = dsVectorCommandType_StartPath;
	dsMatrix33_identity(commands[0].startPath.transform);
	commands[1].commandType = dsVectorCommandType_Move;
	commands[1].move.position.x = 1.0f;
	commands[1].move.position.y = 0.0f;
	commands[2].commandType = dsVectorCommandType_Line;
	commands[2].line.end.x = 0.0f;
	commands[2].line.end.y = 1.0f;
	commands[3].commandType = dsVectorCommandType_Line;
	commands[3].line.end.x = 1.0f;
	commands[3].line.end.y = 2.0f;
	commands[4].commandType = dsVectorCommandType_Line;
	commands[4].line.end.x = 0.0f;
	commands[4].line.end.y = 3.0f;
	commands[5].commandType = dsVectorCommandType_Line;
	commands[5].line.end.x = 1.0f;
	commands[5].line.end.y = 4.0f;
	commands[6].commandType = dsVectorCommandType_Line;
	commands[6].line.end.x = 0.0f;
	commands[6].line.end.y = 5.0f;
	commands[7].commandType = dsVectorCommandType_Line;
	commands[7].line.end.x = 1.5f;
	commands[7].line.end.y = 6.0f;
	commands[8].commandType = dsVectorCommandType_Line;
	commands[8].line.end.x = 0.0f;
	commands[8].line.end.y = 7.0f;
	commands[9].commandType = dsVectorCommandType_Line;
	commands[9].line.end.x = 1.0f;
	commands[9].line.end.y = 8.0f;
	commands[10].commandType = dsVectorCommandType_Line;
	commands[10].line.end.x = 0.0f;
	commands[10].line.end.y = 9.0f;
	commands[11].commandType = dsVectorCommandType_Line;
	commands[11].line.end.x = 1.0f;
	commands[11].line.end.y = 10.0f;
	commands[12].commandType = dsVectorCommandType_Line;
	commands[12].line.end.x = 0.0f;
	commands[12].line.end.y = 11.0f;
	commands[13].commandType = dsVectorCommandType_Line;
	commands[13].line.end.x = 0.5f;
	commands[13].line.end.y = 12.0f;
	commands[14].commandType = dsVectorCommandType_Line;
	commands[14].line.end.x = 0.0f;
	commands[14].line.end.y = 13.0f;
	commands[15].commandType = dsVectorCommandType_Line;
	commands[15].line.end.x = 1.0f;
	commands[15].line.end.y = 14.0f;
	commands[16].commandType = dsVectorCommandType_Line;
	commands[16].line.end.x = 0.0f;
	commands[16].line.end.y = 15.0f;
	commands[17].commandType = dsVectorCommandType_Line;
	commands[17].line.end.x = 1.0f;
	commands[17].line.end.y = 16.0f;
	commands[18].commandType = dsVectorCommandType_Line;
	commands[18].line.end.x = 10.0f;
	commands[18].line.end.y = 16.0f;
	commands[19].commandType = dsVectorCommandType_Line;
	commands[19].line.end.x = 10.0f;
	commands[19].line.end.y = 0.0f;
	commands[20].commandType = dsVectorCommandType_ClosePath;
	commands[21].commandType = dsVectorCommandType_FillPath;
	commands[21].fillPath.material = "fill";
	commands[21].fillPath.opacity = 1.0f;

	dsVector2f size = {{10.0f, 16.0f}};
	dsVectorImage* image = dsVectorImage_create((dsAllocator*)&allocator, scratchData,
		resourceManager, NULL, commands, DS_ARRAY_SIZE(commands), materialSet, true,
		NULL, &size, 0.1f);
	ASSERT_TRUE(image);

	dsGfxBuffer* buffer = dsVectorImage_getBuffer(image);
	ASSERT_TRUE(buffer);
	ASSERT_EQ(sizeof(ShapeVertex)*19 + sizeof(uint16_t)*51, buffer->size);

	const void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, buffer->size);
	ASSERT_TRUE(data);

	const ShapeVertex* vertices = (const ShapeVertex*)data;
	EXPECT_EQ(1.0f, vertices[0].position.x);
	EXPECT_EQ(0.0f, vertices[0].position.y);
	EXPECT_EQ(0.0f, vertices[1].position.x);
	EXPECT_EQ(1.0f, vertices[1].position.y);
	EXPECT_EQ(1.0f, vertices[2].position.x);
	EXPECT_EQ(2.0f, vertices[2].position.y);
	EXPECT_EQ(0.0f, vertices[3].position.x);
	EXPECT_EQ(3.0f, vertices[3].position.y);
	EXPECT_EQ(1.0f, vertices[4].position.x);
	EXPECT_EQ(4.0f, vertices[4].position.y);
	EXPECT_EQ(0.0f, vertices[5].position.x);
	EXPECT_EQ(5.0f, vertices[5].position.y);
	EXPECT_EQ(1.5f, vertices[6].position.x);
	EXPECT_EQ(6.0f, vertices[6].position.y);
	EXPECT_EQ(0.0f, vertices[7].position.x);
	EXPECT_EQ(7.0f, vertices[7].position.y);
	EXPECT_EQ(1.0f, vertices[8].position.x);
	EXPECT_EQ(8.0f, vertices[8].position.y);
	EXPECT_EQ(0.0f, vertices[9].position.x);
	EXPECT_EQ(9.0f, vertices[9].position.y);
	EXPECT_EQ(1.0f, vertices[10].position.x);
	EXPECT_EQ(10.0f, vertices[10].position.y);
	EXPECT_EQ(0.0f, vertices[11].position.x);
	EXPECT_EQ(11.0f, vertices[11].position.y);
	EXPECT_EQ(0.5f, vertices[12].position.x);
	EXPECT_EQ(12.0f, vertices[12].position.y);
	EXPECT_EQ(0.0f, vertices[13].position.x);
	EXPECT_EQ(13.0f, vertices[13].position.y);
	EXPECT_EQ(1.0f, vertices[14].position.x);
	EXPECT_EQ(14.0f, vertices[14].position.y);
	EXPECT_EQ(0.0f, vertices[15].position.x);
	EXPECT_EQ(15.0f, vertices[15].position.y);
	EXPECT_EQ(1.0f, vertices[16].position.x);
	EXPECT_EQ(16.0f, vertices[16].position.y);
	EXPECT_EQ(10.0f, vertices[17].position.x);
	EXPECT_EQ(16.0f, vertices[17].position.y);
	EXPECT_EQ(10.0f, vertices[18].position.x);
	EXPECT_EQ(0.0f, vertices[18].position.y);

	const uint16_t* indices = (const uint16_t*)((const uint8_t*)data + 19*sizeof(ShapeVertex));
	EXPECT_EQ(2U, indices[0]);
	EXPECT_EQ(0U, indices[1]);
	EXPECT_EQ(1U, indices[2]);

	EXPECT_EQ(6U, indices[3]);
	EXPECT_EQ(2U, indices[4]);
	EXPECT_EQ(4U, indices[5]);

	EXPECT_EQ(6U, indices[6]);
	EXPECT_EQ(0U, indices[7]);
	EXPECT_EQ(2U, indices[8]);

	EXPECT_EQ(18U, indices[9]);
	EXPECT_EQ(0U, indices[10]);
	EXPECT_EQ(6U, indices[11]);

	EXPECT_EQ(4U, indices[12]);
	EXPECT_EQ(2U, indices[13]);
	EXPECT_EQ(3U, indices[14]);

	EXPECT_EQ(6U, indices[15]);
	EXPECT_EQ(4U, indices[16]);
	EXPECT_EQ(5U, indices[17]);

	EXPECT_EQ(6U, indices[18]);
	EXPECT_EQ(7U, indices[19]);
	EXPECT_EQ(8U, indices[20]);

	EXPECT_EQ(6U, indices[21]);
	EXPECT_EQ(8U, indices[22]);
	EXPECT_EQ(10U, indices[23]);

	EXPECT_EQ(6U, indices[24]);
	EXPECT_EQ(10U, indices[25]);
	EXPECT_EQ(14U, indices[26]);

	EXPECT_EQ(6U, indices[27]);
	EXPECT_EQ(14U, indices[28]);
	EXPECT_EQ(16U, indices[29]);

	EXPECT_EQ(18U, indices[30]);
	EXPECT_EQ(6U, indices[31]);
	EXPECT_EQ(16U, indices[32]);

	EXPECT_EQ(17U, indices[33]);
	EXPECT_EQ(18U, indices[34]);
	EXPECT_EQ(16U, indices[35]);

	EXPECT_EQ(10U, indices[36]);
	EXPECT_EQ(8U, indices[37]);
	EXPECT_EQ(9U, indices[38]);

	EXPECT_EQ(10U, indices[39]);
	EXPECT_EQ(11U, indices[40]);
	EXPECT_EQ(12U, indices[41]);

	EXPECT_EQ(14U, indices[42]);
	EXPECT_EQ(10U, indices[43]);
	EXPECT_EQ(12U, indices[44]);

	EXPECT_EQ(14U, indices[45]);
	EXPECT_EQ(12U, indices[46]);
	EXPECT_EQ(13U, indices[47]);

	EXPECT_EQ(16U, indices[48]);
	EXPECT_EQ(14U, indices[49]);
	EXPECT_EQ(15U, indices[50]);

	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsVectorImage_destroy(image));
	dsVectorScratchData_destroy(scratchData);
}

TEST_F(TriangulateTest, HoleCW)
{
	dsVectorMaterialSet* materialSet = dsVectorMaterialSet_create((dsAllocator*)&allocator,
		resourceManager, NULL, 1);
	dsVectorMaterial material;
	dsColor color = {{255, 255, 255, 255}};
	ASSERT_TRUE(dsVectorMaterial_setColor(&material, color));
	ASSERT_TRUE(dsVectorMaterialSet_addMaterial(materialSet, "fill", &material, true));

	dsVectorScratchData* scratchData = dsVectorScratchData_create((dsAllocator*)&allocator);
	dsVectorCommand commands[15];
	commands[0].commandType = dsVectorCommandType_StartPath;
	dsMatrix33_identity(commands[0].startPath.transform);
	commands[1].commandType = dsVectorCommandType_Move;
	commands[1].move.position.x = 5.0f;
	commands[1].move.position.y = 3.0f;
	commands[2].commandType = dsVectorCommandType_Line;
	commands[2].line.end.x = 5.0f;
	commands[2].line.end.y = 5.0f;
	commands[3].commandType = dsVectorCommandType_Line;
	commands[3].line.end.x = 0.0f;
	commands[3].line.end.y = 5.0f;
	commands[4].commandType = dsVectorCommandType_Line;
	commands[4].line.end.x = 0.0f;
	commands[4].line.end.y = 0.0f;
	commands[5].commandType = dsVectorCommandType_Line;
	commands[5].line.end.x = 10.0f;
	commands[5].line.end.y = 0.0f;
	commands[6].commandType = dsVectorCommandType_Line;
	commands[6].line.end.x = 10.0f;
	commands[6].line.end.y = 5.0f;
	commands[7].commandType = dsVectorCommandType_Line;
	commands[7].line.end.x = 5.0f;
	commands[7].line.end.y = 5.0f;
	commands[8].commandType = dsVectorCommandType_Line;
	commands[8].line.end.x = 5.0f;
	commands[8].line.end.y = 3.0f;
	commands[9].commandType = dsVectorCommandType_Line;
	commands[9].line.end.x = 6.0f;
	commands[9].line.end.y = 3.0f;
	commands[10].commandType = dsVectorCommandType_Line;
	commands[10].line.end.x = 6.0f;
	commands[10].line.end.y = 2.0f;
	commands[11].commandType = dsVectorCommandType_Line;
	commands[11].line.end.x = 4.0f;
	commands[11].line.end.y = 2.0f;
	commands[12].commandType = dsVectorCommandType_Line;
	commands[12].line.end.x = 4.0f;
	commands[12].line.end.y = 3.0f;
	commands[13].commandType = dsVectorCommandType_ClosePath;
	commands[14].commandType = dsVectorCommandType_FillPath;
	commands[14].fillPath.material = "fill";
	commands[14].fillPath.opacity = 1.0f;

	dsVector2f size = {{10.0f, 5.0f}};
	dsVectorImage* image = dsVectorImage_create((dsAllocator*)&allocator, scratchData,
		resourceManager, NULL, commands, DS_ARRAY_SIZE(commands), materialSet, true,
		NULL, &size, 0.1f);
	ASSERT_TRUE(image);

	dsGfxBuffer* buffer = dsVectorImage_getBuffer(image);
	ASSERT_TRUE(buffer);
	ASSERT_EQ(sizeof(ShapeVertex)*12 + sizeof(uint16_t)*30, buffer->size);

	const void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, buffer->size);
	ASSERT_TRUE(data);

	const ShapeVertex* vertices = (const ShapeVertex*)data;
	EXPECT_EQ(5.0f, vertices[0].position.x);
	EXPECT_EQ(3.0f, vertices[0].position.y);
	EXPECT_EQ(5.0f, vertices[1].position.x);
	EXPECT_EQ(5.0f, vertices[1].position.y);
	EXPECT_EQ(0.0f, vertices[2].position.x);
	EXPECT_EQ(5.0f, vertices[2].position.y);
	EXPECT_EQ(0.0f, vertices[3].position.x);
	EXPECT_EQ(0.0f, vertices[3].position.y);
	EXPECT_EQ(10.0f, vertices[4].position.x);
	EXPECT_EQ(0.0f, vertices[4].position.y);
	EXPECT_EQ(10.0f, vertices[5].position.x);
	EXPECT_EQ(5.0f, vertices[5].position.y);
	EXPECT_EQ(5.0f, vertices[6].position.x);
	EXPECT_EQ(5.0f, vertices[6].position.y);
	EXPECT_EQ(5.0f, vertices[7].position.x);
	EXPECT_EQ(3.0f, vertices[7].position.y);
	EXPECT_EQ(6.0f, vertices[8].position.x);
	EXPECT_EQ(3.0f, vertices[8].position.y);
	EXPECT_EQ(6.0f, vertices[9].position.x);
	EXPECT_EQ(2.0f, vertices[9].position.y);
	EXPECT_EQ(4.0f, vertices[10].position.x);
	EXPECT_EQ(2.0f, vertices[10].position.y);
	EXPECT_EQ(4.0f, vertices[11].position.x);
	EXPECT_EQ(3.0f, vertices[11].position.y);

	const uint16_t* indices = (const uint16_t*)((const uint8_t*)data + 12*sizeof(ShapeVertex));
	EXPECT_EQ(11U, indices[0]);
	EXPECT_EQ(10U, indices[1]);
	EXPECT_EQ(2U, indices[2]);

	EXPECT_EQ(0U, indices[3]);
	EXPECT_EQ(11U, indices[4]);
	EXPECT_EQ(2U, indices[5]);

	EXPECT_EQ(1U, indices[6]);
	EXPECT_EQ(0U, indices[7]);
	EXPECT_EQ(2U, indices[8]);

	EXPECT_EQ(10U, indices[9]);
	EXPECT_EQ(3U, indices[10]);
	EXPECT_EQ(2U, indices[11]);

	EXPECT_EQ(9U, indices[12]);
	EXPECT_EQ(3U, indices[13]);
	EXPECT_EQ(10U, indices[14]);

	EXPECT_EQ(4U, indices[15]);
	EXPECT_EQ(9U, indices[16]);
	EXPECT_EQ(8U, indices[17]);

	EXPECT_EQ(4U, indices[18]);
	EXPECT_EQ(3U, indices[19]);
	EXPECT_EQ(9U, indices[20]);

	EXPECT_EQ(8U, indices[21]);
	EXPECT_EQ(7U, indices[22]);
	EXPECT_EQ(6U, indices[23]);

	EXPECT_EQ(4U, indices[24]);
	EXPECT_EQ(8U, indices[25]);
	EXPECT_EQ(6U, indices[26]);

	EXPECT_EQ(5U, indices[27]);
	EXPECT_EQ(4U, indices[28]);
	EXPECT_EQ(6U, indices[29]);

	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsVectorImage_destroy(image));
	dsVectorScratchData_destroy(scratchData);
}

TEST_F(TriangulateTest, HoleCCW)
{
	dsVectorMaterialSet* materialSet = dsVectorMaterialSet_create((dsAllocator*)&allocator,
		resourceManager, NULL, 1);
	dsVectorMaterial material;
	dsColor color = {{255, 255, 255, 255}};
	ASSERT_TRUE(dsVectorMaterial_setColor(&material, color));
	ASSERT_TRUE(dsVectorMaterialSet_addMaterial(materialSet, "fill", &material, true));

	dsVectorScratchData* scratchData = dsVectorScratchData_create((dsAllocator*)&allocator);
	dsVectorCommand commands[15];
	commands[0].commandType = dsVectorCommandType_StartPath;
	dsMatrix33_identity(commands[0].startPath.transform);
	commands[1].commandType = dsVectorCommandType_Move;
	commands[1].move.position.x = 5.0f;
	commands[1].move.position.y = 3.0f;
	commands[2].commandType = dsVectorCommandType_Line;
	commands[2].line.end.x = 4.0f;
	commands[2].line.end.y = 3.0f;
	commands[3].commandType = dsVectorCommandType_Line;
	commands[3].line.end.x = 4.0f;
	commands[3].line.end.y = 2.0f;
	commands[4].commandType = dsVectorCommandType_Line;
	commands[4].line.end.x = 6.0f;
	commands[4].line.end.y = 2.0f;
	commands[5].commandType = dsVectorCommandType_Line;
	commands[5].line.end.x = 6.0f;
	commands[5].line.end.y = 3.0f;
	commands[6].commandType = dsVectorCommandType_Line;
	commands[6].line.end.x = 5.0f;
	commands[6].line.end.y = 3.0f;
	commands[7].commandType = dsVectorCommandType_Line;
	commands[7].line.end.x = 5.0f;
	commands[7].line.end.y = 5.0f;
	commands[8].commandType = dsVectorCommandType_Line;
	commands[8].line.end.x = 10.0f;
	commands[8].line.end.y = 5.0f;
	commands[9].commandType = dsVectorCommandType_Line;
	commands[9].line.end.x = 10.0f;
	commands[9].line.end.y = 0.0f;
	commands[10].commandType = dsVectorCommandType_Line;
	commands[10].line.end.x = 0.0f;
	commands[10].line.end.y = 0.0f;
	commands[11].commandType = dsVectorCommandType_Line;
	commands[11].line.end.x = 0.0f;
	commands[11].line.end.y = 5.0f;
	commands[12].commandType = dsVectorCommandType_Line;
	commands[12].line.end.x = 5.0f;
	commands[12].line.end.y = 5.0f;
	commands[13].commandType = dsVectorCommandType_ClosePath;
	commands[14].commandType = dsVectorCommandType_FillPath;
	commands[14].fillPath.material = "fill";
	commands[14].fillPath.opacity = 1.0f;

	dsVector2f size = {{10.0f, 5.0f}};
	dsVectorImage* image = dsVectorImage_create((dsAllocator*)&allocator, scratchData,
		resourceManager, NULL, commands, DS_ARRAY_SIZE(commands), materialSet, true,
		NULL, &size, 0.1f);
	ASSERT_TRUE(image);

	dsGfxBuffer* buffer = dsVectorImage_getBuffer(image);
	ASSERT_TRUE(buffer);
	ASSERT_EQ(sizeof(ShapeVertex)*12 + sizeof(uint16_t)*30, buffer->size);

	const void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, buffer->size);
	ASSERT_TRUE(data);

	const ShapeVertex* vertices = (const ShapeVertex*)data;
	EXPECT_EQ(5.0f, vertices[0].position.x);
	EXPECT_EQ(3.0f, vertices[0].position.y);
	EXPECT_EQ(4.0f, vertices[1].position.x);
	EXPECT_EQ(3.0f, vertices[1].position.y);
	EXPECT_EQ(4.0f, vertices[2].position.x);
	EXPECT_EQ(2.0f, vertices[2].position.y);
	EXPECT_EQ(6.0f, vertices[3].position.x);
	EXPECT_EQ(2.0f, vertices[3].position.y);
	EXPECT_EQ(6.0f, vertices[4].position.x);
	EXPECT_EQ(3.0f, vertices[4].position.y);
	EXPECT_EQ(5.0f, vertices[5].position.x);
	EXPECT_EQ(3.0f, vertices[5].position.y);
	EXPECT_EQ(5.0f, vertices[6].position.x);
	EXPECT_EQ(5.0f, vertices[6].position.y);
	EXPECT_EQ(10.0f, vertices[7].position.x);
	EXPECT_EQ(5.0f, vertices[7].position.y);
	EXPECT_EQ(10.0f, vertices[8].position.x);
	EXPECT_EQ(0.0f, vertices[8].position.y);
	EXPECT_EQ(0.0f, vertices[9].position.x);
	EXPECT_EQ(0.0f, vertices[9].position.y);
	EXPECT_EQ(0.0f, vertices[10].position.x);
	EXPECT_EQ(5.0f, vertices[10].position.y);
	EXPECT_EQ(5.0f, vertices[11].position.x);
	EXPECT_EQ(5.0f, vertices[11].position.y);

	const uint16_t* indices = (const uint16_t*)((const uint8_t*)data + 12*sizeof(ShapeVertex));
	EXPECT_EQ(1U, indices[0]);
	EXPECT_EQ(2U, indices[1]);
	EXPECT_EQ(10U, indices[2]);

	EXPECT_EQ(0U, indices[3]);
	EXPECT_EQ(1U, indices[4]);
	EXPECT_EQ(10U, indices[5]);

	EXPECT_EQ(11U, indices[6]);
	EXPECT_EQ(0U, indices[7]);
	EXPECT_EQ(10U, indices[8]);

	EXPECT_EQ(2U, indices[9]);
	EXPECT_EQ(9U, indices[10]);
	EXPECT_EQ(10U, indices[11]);

	EXPECT_EQ(3U, indices[12]);
	EXPECT_EQ(9U, indices[13]);
	EXPECT_EQ(2U, indices[14]);

	EXPECT_EQ(8U, indices[15]);
	EXPECT_EQ(3U, indices[16]);
	EXPECT_EQ(4U, indices[17]);

	EXPECT_EQ(8U, indices[18]);
	EXPECT_EQ(9U, indices[19]);
	EXPECT_EQ(3U, indices[20]);

	EXPECT_EQ(4U, indices[21]);
	EXPECT_EQ(5U, indices[22]);
	EXPECT_EQ(6U, indices[23]);

	EXPECT_EQ(8U, indices[24]);
	EXPECT_EQ(4U, indices[25]);
	EXPECT_EQ(6U, indices[26]);

	EXPECT_EQ(7U, indices[27]);
	EXPECT_EQ(8U, indices[28]);
	EXPECT_EQ(6U, indices[29]);

	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsVectorImage_destroy(image));
	dsVectorScratchData_destroy(scratchData);
}

TEST_F(TriangulateTest, TriangleNoClose)
{
	dsVectorMaterialSet* materialSet = dsVectorMaterialSet_create((dsAllocator*)&allocator,
		resourceManager, NULL, 1);
	dsVectorMaterial material;
	dsColor color = {{255, 255, 255, 255}};
	ASSERT_TRUE(dsVectorMaterial_setColor(&material, color));
	ASSERT_TRUE(dsVectorMaterialSet_addMaterial(materialSet, "fill", &material, true));

	dsVectorScratchData* scratchData = dsVectorScratchData_create((dsAllocator*)&allocator);
	dsVectorCommand commands[5];
	commands[0].commandType = dsVectorCommandType_StartPath;
	dsMatrix33_identity(commands[0].startPath.transform);
	commands[1].commandType = dsVectorCommandType_Move;
	commands[1].move.position.x = 0.0f;
	commands[1].move.position.y = 0.0f;
	commands[2].commandType = dsVectorCommandType_Line;
	commands[2].line.end.x = 1.0f;
	commands[2].line.end.y = 1.2f;
	commands[3].commandType = dsVectorCommandType_Line;
	commands[3].line.end.x = 2.0f;
	commands[3].line.end.y = 0.4f;
	commands[4].commandType = dsVectorCommandType_FillPath;
	commands[4].fillPath.material = "fill";
	commands[4].fillPath.opacity = 1.0f;

	dsVector2f size = {{2.0f, 2.0f}};
	dsVectorImage* image = dsVectorImage_create((dsAllocator*)&allocator, scratchData,
		resourceManager, NULL, commands, DS_ARRAY_SIZE(commands), materialSet, true,
		NULL, &size, 0.1f);
	ASSERT_TRUE(image);

	dsGfxBuffer* buffer = dsVectorImage_getBuffer(image);
	ASSERT_TRUE(buffer);
	ASSERT_EQ(sizeof(ShapeVertex)*3 + sizeof(uint16_t)*3, buffer->size);

	const void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, buffer->size);
	ASSERT_TRUE(data);

	const ShapeVertex* vertices = (const ShapeVertex*)data;
	EXPECT_EQ(0.0f, vertices[0].position.x);
	EXPECT_EQ(0.0f, vertices[0].position.y);
	EXPECT_EQ(1.0f, vertices[1].position.x);
	EXPECT_EQ(1.2f, vertices[1].position.y);
	EXPECT_EQ(2.0f, vertices[2].position.x);
	EXPECT_EQ(0.4f, vertices[2].position.y);

	const uint16_t* indices = (const uint16_t*)((const uint8_t*)data + 3*sizeof(ShapeVertex));
	EXPECT_EQ(2U, indices[0]);
	EXPECT_EQ(0U, indices[1]);
	EXPECT_EQ(1U, indices[2]);

	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsVectorImage_destroy(image));
	dsVectorScratchData_destroy(scratchData);
}

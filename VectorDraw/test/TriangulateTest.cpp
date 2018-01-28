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

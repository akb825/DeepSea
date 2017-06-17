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

#include "FixtureBase.h"
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <gtest/gtest.h>
#include <string.h>

struct TestStruct
{
	float vec3Mem[3];
	float vec2Mem[2];
	float floatMem;
	int intMem;
	unsigned int uintMem;
	double doubleMem;
	float matrix3x4Mem[3][4];
	double doubleMatrix2x3Mem[2][3];
	float floatArrayMem[5];
};

struct TestGfxBufferStruct
{
	float vec3Mem[3];
	float padding1;
	float vec2Mem[2];
	float floatMem;
	int intMem;
	unsigned int uintMem;
	float padding2;
	double doubleMem;
	float matrix3x4Mem[3][4];
	double doubleMatrix2x3Mem[2][4];
	float floatArrayMem[5][4];
};

class ShaderVariableGroupTest : public FixtureBase
{
public:
	dsShaderVariableGroupDesc* createDesc()
	{
		dsShaderVariableElement elements[] =
		{
			{"vec3Mem", dsMaterialType_Vec3, 0},
			{"vec2Mem", dsMaterialType_Vec2, 0},
			{"floatMem", dsMaterialType_Float, 0},
			{"intMem", dsMaterialType_Int, 0},
			{"uintMem", dsMaterialType_UInt, 0},
			{"doubleMem", dsMaterialType_Double, 0},
			{"matrix3x4Mem", dsMaterialType_Mat3x4, 0},
			{"doubleMatrix2x3Mem", dsMaterialType_DMat2x3, 0},
			{"floatArrayMem", dsMaterialType_Float, 5}
		};

		return dsShaderVariableGroupDesc_create(resourceManager, NULL, elements,
			(uint32_t)(DS_ARRAY_SIZE(elements)));
	}

	TestStruct createTestValues()
	{
		TestStruct testValues;
		testValues.vec3Mem[0] = 0.1f;
		testValues.vec3Mem[1] = 0.2f;
		testValues.vec3Mem[2] = 0.3f;
		testValues.vec2Mem[0] = 0.4f;
		testValues.vec2Mem[1] = 0.5f;
		testValues.floatMem = 0.6f;
		testValues.intMem = -7;
		testValues.uintMem = 8;
		testValues.doubleMem = 0.9;
		testValues.matrix3x4Mem[0][0] = 1.0f;
		testValues.matrix3x4Mem[0][1] = 1.1f;
		testValues.matrix3x4Mem[0][2] = 1.2f;
		testValues.matrix3x4Mem[0][3] = 1.3f;
		testValues.matrix3x4Mem[1][0] = 1.4f;
		testValues.matrix3x4Mem[1][1] = 1.5f;
		testValues.matrix3x4Mem[1][2] = 1.6f;
		testValues.matrix3x4Mem[1][3] = 1.7f;
		testValues.matrix3x4Mem[2][0] = 1.8f;
		testValues.matrix3x4Mem[2][1] = 1.9f;
		testValues.matrix3x4Mem[2][2] = 2.0f;
		testValues.matrix3x4Mem[2][3] = 2.1f;
		testValues.doubleMatrix2x3Mem[0][0] = 2.2;
		testValues.doubleMatrix2x3Mem[0][1] = 2.3;
		testValues.doubleMatrix2x3Mem[0][2] = 2.4;
		testValues.doubleMatrix2x3Mem[1][0] = 2.5;
		testValues.doubleMatrix2x3Mem[1][1] = 2.6;
		testValues.doubleMatrix2x3Mem[1][2] = 2.7;
		testValues.floatArrayMem[0] = 2.8f;
		testValues.floatArrayMem[1] = 2.9f;
		testValues.floatArrayMem[2] = 3.0f;
		testValues.floatArrayMem[3] = 3.1f;
		testValues.floatArrayMem[4] = 3.2f;
		return testValues;
	}
};

TEST_F(ShaderVariableGroupTest, GfxBuffer)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsShaderVariableGroupDesc* desc = createDesc();
	ASSERT_TRUE(desc);
	dsShaderVariableGroup* group = dsShaderVariableGroup_create(resourceManager, NULL, NULL, desc);
	ASSERT_TRUE(group);
	EXPECT_EQ(desc, dsShaderVariableGroup_getDescription(group));

	dsGfxBuffer* buffer = dsShaderVariableGroup_getGfxBuffer(group);
	ASSERT_TRUE(buffer);

	TestGfxBufferStruct* gfxBufferValues = (TestGfxBufferStruct*)dsGfxBuffer_map(buffer,
		dsGfxBufferMap_Read | dsGfxBufferMap_Write, 0, DS_MAP_FULL_BUFFER);
	ASSERT_TRUE(gfxBufferValues);
	memset(gfxBufferValues, 0, sizeof(TestGfxBufferStruct));

	TestStruct testValues = createTestValues();
	EXPECT_FALSE(dsShaderVariableGroup_setElementData(group, 0, &testValues.vec3Mem,
		dsMaterialType_Float, 0, 1));
	EXPECT_FALSE(dsShaderVariableGroup_setElementData(group, 0, &testValues.vec3Mem,
		dsMaterialType_Vec3, 1, 1));

	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 0, &testValues.vec3Mem,
		dsMaterialType_Vec3, 0, 1));
	EXPECT_EQ(0.0f, gfxBufferValues->vec3Mem[0]);
	EXPECT_EQ(0.0f, gfxBufferValues->vec3Mem[1]);
	EXPECT_EQ(0.0f, gfxBufferValues->vec3Mem[2]);

	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 1, &testValues.vec2Mem,
		dsMaterialType_Vec2, 0, 1));
	EXPECT_EQ(0.0f, gfxBufferValues->vec2Mem[0]);
	EXPECT_EQ(0.0f, gfxBufferValues->vec2Mem[1]);

	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 2, &testValues.floatMem,
		dsMaterialType_Float, 0, 1));
	EXPECT_EQ(0.0f, gfxBufferValues->floatMem);

	EXPECT_TRUE(dsShaderVariableGroup_commit(commandBuffer, group));
	EXPECT_EQ(0.1f, gfxBufferValues->vec3Mem[0]);
	EXPECT_EQ(0.2f, gfxBufferValues->vec3Mem[1]);
	EXPECT_EQ(0.3f, gfxBufferValues->vec3Mem[2]);
	EXPECT_EQ(0.4f, gfxBufferValues->vec2Mem[0]);
	EXPECT_EQ(0.5f, gfxBufferValues->vec2Mem[1]);
	EXPECT_EQ(0.6f, gfxBufferValues->floatMem);

	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 3, &testValues.intMem,
		dsMaterialType_Int, 0, 1));
	EXPECT_EQ(0, gfxBufferValues->intMem);

	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 4, &testValues.uintMem,
		dsMaterialType_UInt, 0, 1));
	EXPECT_EQ(0U, gfxBufferValues->uintMem);

	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 5, &testValues.doubleMem,
		dsMaterialType_Double, 0, 1));
	EXPECT_EQ(0.0, gfxBufferValues->doubleMem);

	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 6, &testValues.matrix3x4Mem,
		dsMaterialType_Mat3x4, 0, 1));
	EXPECT_EQ(0.0f, gfxBufferValues->matrix3x4Mem[0][0]);
	EXPECT_EQ(0.0f, gfxBufferValues->matrix3x4Mem[0][1]);
	EXPECT_EQ(0.0f, gfxBufferValues->matrix3x4Mem[0][2]);
	EXPECT_EQ(0.0f, gfxBufferValues->matrix3x4Mem[0][3]);
	EXPECT_EQ(0.0f, gfxBufferValues->matrix3x4Mem[1][0]);
	EXPECT_EQ(0.0f, gfxBufferValues->matrix3x4Mem[1][1]);
	EXPECT_EQ(0.0f, gfxBufferValues->matrix3x4Mem[1][2]);
	EXPECT_EQ(0.0f, gfxBufferValues->matrix3x4Mem[1][3]);
	EXPECT_EQ(0.0f, gfxBufferValues->matrix3x4Mem[2][0]);
	EXPECT_EQ(0.0f, gfxBufferValues->matrix3x4Mem[2][1]);
	EXPECT_EQ(0.0f, gfxBufferValues->matrix3x4Mem[2][2]);
	EXPECT_EQ(0.0f, gfxBufferValues->matrix3x4Mem[2][3]);

	EXPECT_TRUE(dsShaderVariableGroup_commit(commandBuffer, group));
	EXPECT_EQ(-7, gfxBufferValues->intMem);
	EXPECT_EQ(8U, gfxBufferValues->uintMem);
	EXPECT_EQ(0.9, gfxBufferValues->doubleMem);
	EXPECT_EQ(1.0f, gfxBufferValues->matrix3x4Mem[0][0]);
	EXPECT_EQ(1.1f, gfxBufferValues->matrix3x4Mem[0][1]);
	EXPECT_EQ(1.2f, gfxBufferValues->matrix3x4Mem[0][2]);
	EXPECT_EQ(1.3f, gfxBufferValues->matrix3x4Mem[0][3]);
	EXPECT_EQ(1.4f, gfxBufferValues->matrix3x4Mem[1][0]);
	EXPECT_EQ(1.5f, gfxBufferValues->matrix3x4Mem[1][1]);
	EXPECT_EQ(1.6f, gfxBufferValues->matrix3x4Mem[1][2]);
	EXPECT_EQ(1.7f, gfxBufferValues->matrix3x4Mem[1][3]);
	EXPECT_EQ(1.8f, gfxBufferValues->matrix3x4Mem[2][0]);
	EXPECT_EQ(1.9f, gfxBufferValues->matrix3x4Mem[2][1]);
	EXPECT_EQ(2.0f, gfxBufferValues->matrix3x4Mem[2][2]);
	EXPECT_EQ(2.1f, gfxBufferValues->matrix3x4Mem[2][3]);

	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 7, &testValues.doubleMatrix2x3Mem,
		dsMaterialType_DMat2x3, 0, 1));
	EXPECT_EQ(0.0, gfxBufferValues->doubleMatrix2x3Mem[0][0]);
	EXPECT_EQ(0.0, gfxBufferValues->doubleMatrix2x3Mem[0][1]);
	EXPECT_EQ(0.0, gfxBufferValues->doubleMatrix2x3Mem[0][2]);
	EXPECT_EQ(0.0, gfxBufferValues->doubleMatrix2x3Mem[1][0]);
	EXPECT_EQ(0.0, gfxBufferValues->doubleMatrix2x3Mem[1][1]);
	EXPECT_EQ(0.0, gfxBufferValues->doubleMatrix2x3Mem[1][2]);

	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 8, testValues.floatArrayMem,
		dsMaterialType_Float, 0, 2));
	EXPECT_EQ(0.0f, gfxBufferValues->floatArrayMem[0][0]);
	EXPECT_EQ(0.0f, gfxBufferValues->floatArrayMem[1][0]);

	EXPECT_TRUE(dsShaderVariableGroup_commit(commandBuffer, group));
	EXPECT_EQ(2.2, gfxBufferValues->doubleMatrix2x3Mem[0][0]);
	EXPECT_EQ(2.3, gfxBufferValues->doubleMatrix2x3Mem[0][1]);
	EXPECT_EQ(2.4, gfxBufferValues->doubleMatrix2x3Mem[0][2]);
	EXPECT_EQ(2.5, gfxBufferValues->doubleMatrix2x3Mem[1][0]);
	EXPECT_EQ(2.6, gfxBufferValues->doubleMatrix2x3Mem[1][1]);
	EXPECT_EQ(2.7, gfxBufferValues->doubleMatrix2x3Mem[1][2]);
	EXPECT_EQ(2.8f, gfxBufferValues->floatArrayMem[0][0]);
	EXPECT_EQ(2.9f, gfxBufferValues->floatArrayMem[1][0]);

	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 8, testValues.floatArrayMem + 2,
		dsMaterialType_Float, 2, 3));
	EXPECT_EQ(0.0f, gfxBufferValues->floatArrayMem[2][0]);
	EXPECT_EQ(0.0f, gfxBufferValues->floatArrayMem[3][0]);
	EXPECT_EQ(0.0f, gfxBufferValues->floatArrayMem[4][0]);

	EXPECT_TRUE(dsShaderVariableGroup_commit(commandBuffer, group));
	EXPECT_EQ(0.1f, gfxBufferValues->vec3Mem[0]);
	EXPECT_EQ(0.2f, gfxBufferValues->vec3Mem[1]);
	EXPECT_EQ(0.3f, gfxBufferValues->vec3Mem[2]);
	EXPECT_EQ(0.4f, gfxBufferValues->vec2Mem[0]);
	EXPECT_EQ(0.5f, gfxBufferValues->vec2Mem[1]);
	EXPECT_EQ(0.6f, gfxBufferValues->floatMem);
	EXPECT_EQ(-7, gfxBufferValues->intMem);
	EXPECT_EQ(8U, gfxBufferValues->uintMem);
	EXPECT_EQ(0.9, gfxBufferValues->doubleMem);
	EXPECT_EQ(1.0f, gfxBufferValues->matrix3x4Mem[0][0]);
	EXPECT_EQ(1.1f, gfxBufferValues->matrix3x4Mem[0][1]);
	EXPECT_EQ(1.2f, gfxBufferValues->matrix3x4Mem[0][2]);
	EXPECT_EQ(1.3f, gfxBufferValues->matrix3x4Mem[0][3]);
	EXPECT_EQ(1.4f, gfxBufferValues->matrix3x4Mem[1][0]);
	EXPECT_EQ(1.5f, gfxBufferValues->matrix3x4Mem[1][1]);
	EXPECT_EQ(1.6f, gfxBufferValues->matrix3x4Mem[1][2]);
	EXPECT_EQ(1.7f, gfxBufferValues->matrix3x4Mem[1][3]);
	EXPECT_EQ(1.8f, gfxBufferValues->matrix3x4Mem[2][0]);
	EXPECT_EQ(1.9f, gfxBufferValues->matrix3x4Mem[2][1]);
	EXPECT_EQ(2.0f, gfxBufferValues->matrix3x4Mem[2][2]);
	EXPECT_EQ(2.1f, gfxBufferValues->matrix3x4Mem[2][3]);
	EXPECT_EQ(-7, gfxBufferValues->intMem);
	EXPECT_EQ(8U, gfxBufferValues->uintMem);
	EXPECT_EQ(0.9, gfxBufferValues->doubleMem);
	EXPECT_EQ(1.0f, gfxBufferValues->matrix3x4Mem[0][0]);
	EXPECT_EQ(1.1f, gfxBufferValues->matrix3x4Mem[0][1]);
	EXPECT_EQ(1.2f, gfxBufferValues->matrix3x4Mem[0][2]);
	EXPECT_EQ(1.3f, gfxBufferValues->matrix3x4Mem[0][3]);
	EXPECT_EQ(1.4f, gfxBufferValues->matrix3x4Mem[1][0]);
	EXPECT_EQ(1.5f, gfxBufferValues->matrix3x4Mem[1][1]);
	EXPECT_EQ(1.6f, gfxBufferValues->matrix3x4Mem[1][2]);
	EXPECT_EQ(1.7f, gfxBufferValues->matrix3x4Mem[1][3]);
	EXPECT_EQ(1.8f, gfxBufferValues->matrix3x4Mem[2][0]);
	EXPECT_EQ(1.9f, gfxBufferValues->matrix3x4Mem[2][1]);
	EXPECT_EQ(2.0f, gfxBufferValues->matrix3x4Mem[2][2]);
	EXPECT_EQ(2.1f, gfxBufferValues->matrix3x4Mem[2][3]);
	EXPECT_EQ(2.2, gfxBufferValues->doubleMatrix2x3Mem[0][0]);
	EXPECT_EQ(2.3, gfxBufferValues->doubleMatrix2x3Mem[0][1]);
	EXPECT_EQ(2.4, gfxBufferValues->doubleMatrix2x3Mem[0][2]);
	EXPECT_EQ(2.5, gfxBufferValues->doubleMatrix2x3Mem[1][0]);
	EXPECT_EQ(2.6, gfxBufferValues->doubleMatrix2x3Mem[1][1]);
	EXPECT_EQ(2.7, gfxBufferValues->doubleMatrix2x3Mem[1][2]);
	EXPECT_EQ(2.8f, gfxBufferValues->floatArrayMem[0][0]);
	EXPECT_EQ(2.9f, gfxBufferValues->floatArrayMem[1][0]);
	EXPECT_EQ(3.0f, gfxBufferValues->floatArrayMem[2][0]);
	EXPECT_EQ(3.1f, gfxBufferValues->floatArrayMem[3][0]);
	EXPECT_EQ(3.2f, gfxBufferValues->floatArrayMem[4][0]);

	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));
	EXPECT_TRUE(dsShaderVariableGroup_destroy(group));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(desc));
}

TEST_F(ShaderVariableGroupTest, NoGfxBuffer)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;
	resourceManager->supportedBuffers = (dsGfxBufferUsage)(resourceManager->supportedBuffers &
		~dsGfxBufferUsage_UniformBlock);

	dsShaderVariableGroupDesc* desc = createDesc();
	ASSERT_TRUE(desc);
	dsShaderVariableGroup* group = dsShaderVariableGroup_create(resourceManager, NULL, NULL, desc);
	ASSERT_TRUE(group);
	EXPECT_EQ(desc, dsShaderVariableGroup_getDescription(group));

	EXPECT_FALSE(dsShaderVariableGroup_getGfxBuffer(group));

	TestStruct testValues = createTestValues();
	EXPECT_FALSE(dsShaderVariableGroup_setElementData(group, 0, &testValues.vec3Mem,
		dsMaterialType_Float, 0, 1));
	EXPECT_FALSE(dsShaderVariableGroup_setElementData(group, 0, &testValues.vec3Mem,
		dsMaterialType_Vec3, 1, 1));

	for (uint32_t i = 0; i < desc->elementCount; ++i)
		EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, i));

	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 0, &testValues.vec3Mem,
		dsMaterialType_Vec3, 0, 1));
	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 1, &testValues.vec2Mem,
		dsMaterialType_Vec2, 0, 1));
	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 2, &testValues.floatMem,
		dsMaterialType_Float, 0, 1));

	EXPECT_TRUE(dsShaderVariableGroup_isElementDirty(group, 0));
	EXPECT_TRUE(dsShaderVariableGroup_isElementDirty(group, 1));
	EXPECT_TRUE(dsShaderVariableGroup_isElementDirty(group, 2));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 3));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 4));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 5));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 6));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 7));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 8));

	EXPECT_TRUE(dsShaderVariableGroup_commit(commandBuffer, group));
	const void* elementPtr = dsShaderVariableGroup_getRawElementData(group, 0);
	ASSERT_TRUE(elementPtr);
	EXPECT_EQ(0, memcmp(&testValues.vec3Mem, elementPtr, sizeof(testValues.vec3Mem)));
	elementPtr = dsShaderVariableGroup_getRawElementData(group, 1);
	ASSERT_TRUE(elementPtr);
	EXPECT_EQ(0, memcmp(&testValues.vec2Mem, elementPtr, sizeof(testValues.vec2Mem)));
	elementPtr = dsShaderVariableGroup_getRawElementData(group, 2);
	ASSERT_TRUE(elementPtr);
	EXPECT_EQ(0, memcmp(&testValues.floatMem, elementPtr, sizeof(testValues.floatMem)));

	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 3, &testValues.intMem,
		dsMaterialType_Int, 0, 1));
	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 4, &testValues.uintMem,
		dsMaterialType_UInt, 0, 1));
	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 5, &testValues.doubleMem,
		dsMaterialType_Double, 0, 1));
	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 6, &testValues.matrix3x4Mem,
		dsMaterialType_Mat3x4, 0, 1));

	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 0));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 1));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 2));
	EXPECT_TRUE(dsShaderVariableGroup_isElementDirty(group, 3));
	EXPECT_TRUE(dsShaderVariableGroup_isElementDirty(group, 4));
	EXPECT_TRUE(dsShaderVariableGroup_isElementDirty(group, 5));
	EXPECT_TRUE(dsShaderVariableGroup_isElementDirty(group, 6));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 7));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 8));

	EXPECT_TRUE(dsShaderVariableGroup_commit(commandBuffer, group));
	elementPtr = dsShaderVariableGroup_getRawElementData(group, 3);
	ASSERT_TRUE(elementPtr);
	EXPECT_EQ(0, memcmp(&testValues.intMem, elementPtr, sizeof(testValues.intMem)));
	elementPtr = dsShaderVariableGroup_getRawElementData(group, 4);
	ASSERT_TRUE(elementPtr);
	EXPECT_EQ(0, memcmp(&testValues.uintMem, elementPtr, sizeof(testValues.uintMem)));
	elementPtr = dsShaderVariableGroup_getRawElementData(group, 5);
	ASSERT_TRUE(elementPtr);
	EXPECT_EQ(0, memcmp(&testValues.doubleMem, elementPtr, sizeof(testValues.doubleMem)));
	elementPtr = dsShaderVariableGroup_getRawElementData(group, 6);
	ASSERT_TRUE(elementPtr);
	EXPECT_EQ(0, memcmp(&testValues.matrix3x4Mem, elementPtr, sizeof(testValues.matrix3x4Mem)));

	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 7, &testValues.doubleMatrix2x3Mem,
		dsMaterialType_DMat2x3, 0, 1));
	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 8, testValues.floatArrayMem,
		dsMaterialType_Float, 0, 2));

	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 0));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 1));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 2));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 3));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 4));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 5));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 6));
	EXPECT_TRUE(dsShaderVariableGroup_isElementDirty(group, 7));
	EXPECT_TRUE(dsShaderVariableGroup_isElementDirty(group, 8));

	EXPECT_TRUE(dsShaderVariableGroup_commit(commandBuffer, group));
	elementPtr = dsShaderVariableGroup_getRawElementData(group, 7);
	ASSERT_TRUE(elementPtr);
	EXPECT_EQ(0, memcmp(&testValues.doubleMatrix2x3Mem, elementPtr,
		sizeof(testValues.doubleMatrix2x3Mem)));
	elementPtr = dsShaderVariableGroup_getRawElementData(group, 8);
	ASSERT_TRUE(elementPtr);
	EXPECT_EQ(0, memcmp(&testValues.floatArrayMem, elementPtr, sizeof(float)*2));

	EXPECT_TRUE(dsShaderVariableGroup_setElementData(group, 8, testValues.floatArrayMem + 2,
		dsMaterialType_Float, 2, 3));

	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 0));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 1));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 2));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 3));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 4));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 5));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 6));
	EXPECT_FALSE(dsShaderVariableGroup_isElementDirty(group, 7));
	EXPECT_TRUE(dsShaderVariableGroup_isElementDirty(group, 8));

	EXPECT_TRUE(dsShaderVariableGroup_commit(commandBuffer, group));
	elementPtr = dsShaderVariableGroup_getRawElementData(group, 0);
	ASSERT_TRUE(elementPtr);
	EXPECT_EQ(0, memcmp(&testValues.vec3Mem, elementPtr, sizeof(testValues.vec3Mem)));
	elementPtr = dsShaderVariableGroup_getRawElementData(group, 1);
	ASSERT_TRUE(elementPtr);
	EXPECT_EQ(0, memcmp(&testValues.vec2Mem, elementPtr, sizeof(testValues.vec2Mem)));
	elementPtr = dsShaderVariableGroup_getRawElementData(group, 2);
	ASSERT_TRUE(elementPtr);
	EXPECT_EQ(0, memcmp(&testValues.floatMem, elementPtr, sizeof(testValues.floatMem)));
	elementPtr = dsShaderVariableGroup_getRawElementData(group, 3);
	ASSERT_TRUE(elementPtr);
	EXPECT_EQ(0, memcmp(&testValues.intMem, elementPtr, sizeof(testValues.intMem)));
	elementPtr = dsShaderVariableGroup_getRawElementData(group, 4);
	ASSERT_TRUE(elementPtr);
	EXPECT_EQ(0, memcmp(&testValues.uintMem, elementPtr, sizeof(testValues.uintMem)));
	elementPtr = dsShaderVariableGroup_getRawElementData(group, 5);
	ASSERT_TRUE(elementPtr);
	EXPECT_EQ(0, memcmp(&testValues.doubleMem, elementPtr, sizeof(testValues.doubleMem)));
	elementPtr = dsShaderVariableGroup_getRawElementData(group, 6);
	ASSERT_TRUE(elementPtr);
	EXPECT_EQ(0, memcmp(&testValues.matrix3x4Mem, elementPtr, sizeof(testValues.matrix3x4Mem)));
	elementPtr = dsShaderVariableGroup_getRawElementData(group, 7);
	ASSERT_TRUE(elementPtr);
	EXPECT_EQ(0, memcmp(&testValues.doubleMatrix2x3Mem, elementPtr,
		sizeof(testValues.doubleMatrix2x3Mem)));
	elementPtr = dsShaderVariableGroup_getRawElementData(group, 8);
	ASSERT_TRUE(elementPtr);
	EXPECT_EQ(0, memcmp(&testValues.floatArrayMem, elementPtr, sizeof(testValues.floatArrayMem)));

	EXPECT_TRUE(dsShaderVariableGroup_destroy(group));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(desc));
}

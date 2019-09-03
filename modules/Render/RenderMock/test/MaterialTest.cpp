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

#include "Fixtures/FixtureBase.h"
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <gtest/gtest.h>

class MaterialTest : public FixtureBase
{
};

TEST_F(MaterialTest, PrimitivesVectorsMatrices)
{
	dsShaderVariableElement groupElements[] =
	{
		{"testValue", dsMaterialType_Float, 0}
	};

	dsShaderVariableGroupDesc* groupDesc = dsShaderVariableGroupDesc_create(resourceManager, NULL,
		groupElements, DS_ARRAY_SIZE(groupElements));

	dsMaterialElement elements[] =
	{
		{"vec3Mem", dsMaterialType_Vec3, 0, NULL, dsMaterialBinding_Material, 0},
		{"vec2Mem", dsMaterialType_Vec2, 0, NULL, dsMaterialBinding_Material, 0},
		{"floatMem", dsMaterialType_Float, 0, NULL, dsMaterialBinding_Material, 0},
		{"intMem", dsMaterialType_Int, 0, NULL, dsMaterialBinding_Material, 0},
		{"texture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0},
		{"uintMem", dsMaterialType_UInt, 0, NULL, dsMaterialBinding_Material, 0},
		{"doubleMem", dsMaterialType_Double, 0, NULL, dsMaterialBinding_Material, 0},
		{"variableGroup", dsMaterialType_VariableGroup, 0, groupDesc, dsMaterialBinding_Material,
			0},
		{"matrix3x4Mem", dsMaterialType_Mat3x4, 0, NULL, dsMaterialBinding_Material, 0},
		{"doubleMatrix2x3Mem", dsMaterialType_DMat2x3, 0, NULL, dsMaterialBinding_Material, 0},
		{"buffer", dsMaterialType_UniformBlock, 0, NULL, dsMaterialBinding_Material, 0},
		{"floatArrayMem", dsMaterialType_Float, 5, NULL, dsMaterialBinding_Material, 0}
	};

	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		DS_ARRAY_SIZE(elements));
	ASSERT_TRUE(materialDesc);

	EXPECT_FALSE(dsMaterial_create(NULL, (dsAllocator*)&allocator, materialDesc));
	EXPECT_FALSE(dsMaterial_create(resourceManager, (dsAllocator*)&allocator, NULL));

	dsMaterial* material = dsMaterial_create(resourceManager, (dsAllocator*)&allocator,
		materialDesc);
	ASSERT_TRUE(material);

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
	} testValues;

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

	EXPECT_FALSE(dsMaterial_setElementData(material, 0, &testValues.vec3Mem, dsMaterialType_Float,
		0, 1));
	EXPECT_FALSE(dsMaterial_setElementData(material, 0, &testValues.vec3Mem, dsMaterialType_Vec3,
		1, 1));
	EXPECT_FALSE(dsMaterial_setElementData(material, 4, &testValues.vec3Mem, dsMaterialType_Vec3,
		0, 1));
	EXPECT_FALSE(dsMaterial_setElementData(material, 7, &testValues.vec3Mem, dsMaterialType_Vec3,
		0, 1));
	EXPECT_FALSE(dsMaterial_setElementData(material, 10, &testValues.vec3Mem, dsMaterialType_Vec3,
		0, 1));

	EXPECT_TRUE(dsMaterial_setElementData(material, 0, &testValues.vec3Mem, dsMaterialType_Vec3, 0,
		1));
	EXPECT_TRUE(dsMaterial_setElementData(material, 1, &testValues.vec2Mem, dsMaterialType_Vec2, 0,
		1));
	EXPECT_TRUE(dsMaterial_setElementData(material, 2, &testValues.floatMem, dsMaterialType_Float,
		0, 1));
	EXPECT_TRUE(dsMaterial_setElementData(material, 3, &testValues.intMem, dsMaterialType_Int, 0,
		1));
	EXPECT_TRUE(dsMaterial_setElementData(material, 5, &testValues.uintMem, dsMaterialType_UInt, 0,
		1));
	EXPECT_TRUE(dsMaterial_setElementData(material, 6, &testValues.doubleMem, dsMaterialType_Double,
		0, 1));
	EXPECT_TRUE(dsMaterial_setElementData(material, 8, &testValues.matrix3x4Mem,
		dsMaterialType_Mat3x4, 0, 1));
	EXPECT_TRUE(dsMaterial_setElementData(material, 9, &testValues.doubleMatrix2x3Mem,
		dsMaterialType_DMat2x3, 0, 1));
	EXPECT_TRUE(dsMaterial_setElementData(material, 11, testValues.floatArrayMem,
		dsMaterialType_Float, 0, 2));
	EXPECT_TRUE(dsMaterial_setElementData(material, 11, testValues.floatArrayMem + 2,
		dsMaterialType_Float, 2, 3));

	TestStruct readValues;
	EXPECT_FALSE(dsMaterial_getElementData(&readValues.vec3Mem, material, 0, dsMaterialType_Float,
		0, 1));
	EXPECT_FALSE(dsMaterial_getElementData(&readValues.vec3Mem, material, 0, dsMaterialType_Vec3,
		1, 1));
	EXPECT_FALSE(dsMaterial_getElementData(&readValues.vec3Mem, material, 4, dsMaterialType_Vec3,
		0, 1));
	EXPECT_FALSE(dsMaterial_getElementData(&readValues.vec3Mem, material, 7, dsMaterialType_Vec3,
		0, 1));
	EXPECT_FALSE(dsMaterial_getElementData(&readValues.vec3Mem, material, 10, dsMaterialType_Vec3,
		0, 1));

	EXPECT_TRUE(dsMaterial_getElementData(&readValues.vec3Mem, material, 0, dsMaterialType_Vec3, 0,
		1));
	EXPECT_EQ(0.1f, readValues.vec3Mem[0]);
	EXPECT_EQ(0.2f, readValues.vec3Mem[1]);
	EXPECT_EQ(0.3f, readValues.vec3Mem[2]);

	EXPECT_TRUE(dsMaterial_getElementData(&readValues.vec2Mem, material, 1, dsMaterialType_Vec2, 0,
		1));
	EXPECT_EQ(0.4f, readValues.vec2Mem[0]);
	EXPECT_EQ(0.5f, readValues.vec2Mem[1]);

	EXPECT_TRUE(dsMaterial_getElementData(&readValues.floatMem, material, 2, dsMaterialType_Float,
		0, 1));
	EXPECT_EQ(0.6f, readValues.floatMem);

	EXPECT_TRUE(dsMaterial_getElementData(&readValues.intMem, material, 3, dsMaterialType_Int, 0,
		1));
	EXPECT_EQ(-7, readValues.intMem);

	EXPECT_TRUE(dsMaterial_getElementData(&readValues.uintMem, material, 5, dsMaterialType_UInt, 0,
		1));
	EXPECT_EQ(8U, readValues.uintMem);

	EXPECT_TRUE(dsMaterial_getElementData(&readValues.doubleMem, material, 6, dsMaterialType_Double,
		0, 1));
	EXPECT_EQ(0.9, readValues.doubleMem);

	EXPECT_TRUE(dsMaterial_getElementData(&readValues.matrix3x4Mem, material, 8,
		dsMaterialType_Mat3x4, 0, 1));
	EXPECT_EQ(1.0f, readValues.matrix3x4Mem[0][0]);
	EXPECT_EQ(1.1f, readValues.matrix3x4Mem[0][1]);
	EXPECT_EQ(1.2f, readValues.matrix3x4Mem[0][2]);
	EXPECT_EQ(1.3f, readValues.matrix3x4Mem[0][3]);
	EXPECT_EQ(1.4f, readValues.matrix3x4Mem[1][0]);
	EXPECT_EQ(1.5f, readValues.matrix3x4Mem[1][1]);
	EXPECT_EQ(1.6f, readValues.matrix3x4Mem[1][2]);
	EXPECT_EQ(1.7f, readValues.matrix3x4Mem[1][3]);
	EXPECT_EQ(1.8f, readValues.matrix3x4Mem[2][0]);
	EXPECT_EQ(1.9f, readValues.matrix3x4Mem[2][1]);
	EXPECT_EQ(2.0f, readValues.matrix3x4Mem[2][2]);
	EXPECT_EQ(2.1f, readValues.matrix3x4Mem[2][3]);

	EXPECT_TRUE(dsMaterial_getElementData(&readValues.doubleMatrix2x3Mem, material, 9,
		dsMaterialType_DMat2x3, 0, 1));
	EXPECT_EQ(2.2, readValues.doubleMatrix2x3Mem[0][0]);
	EXPECT_EQ(2.3, readValues.doubleMatrix2x3Mem[0][1]);
	EXPECT_EQ(2.4, readValues.doubleMatrix2x3Mem[0][2]);
	EXPECT_EQ(2.5, readValues.doubleMatrix2x3Mem[1][0]);
	EXPECT_EQ(2.6, readValues.doubleMatrix2x3Mem[1][1]);
	EXPECT_EQ(2.7, readValues.doubleMatrix2x3Mem[1][2]);

	EXPECT_TRUE(dsMaterial_getElementData(readValues.floatArrayMem, material, 11,
		dsMaterialType_Float, 0, 2));
	EXPECT_EQ(2.8f, readValues.floatArrayMem[0]);
	EXPECT_EQ(2.9f, readValues.floatArrayMem[1]);

	EXPECT_TRUE(dsMaterial_getElementData(readValues.floatArrayMem + 2, material, 11,
		dsMaterialType_Float, 2, 3));
	EXPECT_EQ(3.0f, readValues.floatArrayMem[2]);
	EXPECT_EQ(3.1f, readValues.floatArrayMem[3]);
	EXPECT_EQ(3.2f, readValues.floatArrayMem[4]);

	const uint32_t* rawUIntData = (const uint32_t*)dsMaterial_getRawElementData(material, 5);
	ASSERT_TRUE(rawUIntData);
	EXPECT_EQ(8U, *rawUIntData);
	EXPECT_FALSE(dsMaterial_getRawElementData(material, 4));
	EXPECT_FALSE(dsMaterial_getRawElementData(material, 7));
	EXPECT_FALSE(dsMaterial_getRawElementData(material, 10));

	dsMaterial_destroy(material);
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(groupDesc));
}

TEST_F(MaterialTest, Textures)
{
	dsShaderVariableElement groupElements[] =
	{
		{"testValue", dsMaterialType_Float, 0}
	};

	dsShaderVariableGroupDesc* groupDesc = dsShaderVariableGroupDesc_create(resourceManager, NULL,
		groupElements, DS_ARRAY_SIZE(groupElements));

	dsMaterialElement elements[] =
	{
		{"float", dsMaterialType_Float, 0, NULL, dsMaterialBinding_Material, 0},
		{"texture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0},
		{"variableGroup", dsMaterialType_VariableGroup, 0, groupDesc, dsMaterialBinding_Material,
			0},
		{"image", dsMaterialType_Image, 0, NULL, dsMaterialBinding_Material, 0},
		{"buffer", dsMaterialType_UniformBlock, 0, NULL, dsMaterialBinding_Material, 0},
		{"subpassInput", dsMaterialType_SubpassInput, 0, NULL, dsMaterialBinding_Material, 0},
		{"sharedTexture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Global, 0}
	};

	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		DS_ARRAY_SIZE(elements));
	ASSERT_TRUE(materialDesc);

	dsMaterial* material = dsMaterial_create(resourceManager, (dsAllocator*)&allocator,
		materialDesc);
	ASSERT_TRUE(material);

	dsTextureInfo texInfo = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		dsTextureDim_2D, 16, 16, 0, DS_ALL_MIP_LEVELS, 1};
	dsTexture* texture1 = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo, dsGfxMemory_Static, &texInfo, NULL, 0);
	ASSERT_TRUE(texture1);

	dsTexture* texture2 = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Image | dsTextureUsage_CopyTo, dsGfxMemory_Static, &texInfo, NULL, 0);
	ASSERT_TRUE(texture2);

	dsTexture* texture3 = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_SubpassInput | dsTextureUsage_CopyTo, dsGfxMemory_Static, &texInfo, NULL, 0);
	ASSERT_TRUE(texture3);

	EXPECT_FALSE(dsMaterial_setTexture(material, 0, texture1));
	EXPECT_FALSE(dsMaterial_setTexture(material, 1, texture2));
	EXPECT_FALSE(dsMaterial_setTexture(material, 1, texture3));
	EXPECT_FALSE(dsMaterial_setTexture(material, 2, texture1));
	EXPECT_FALSE(dsMaterial_setTexture(material, 3, texture1));
	EXPECT_FALSE(dsMaterial_setTexture(material, 3, texture3));
	EXPECT_FALSE(dsMaterial_setTexture(material, 4, texture1));
	EXPECT_FALSE(dsMaterial_setTexture(material, 5, texture1));
	EXPECT_FALSE(dsMaterial_setTexture(material, 5, texture2));
	EXPECT_FALSE(dsMaterial_setTexture(material, 6, texture1));

	EXPECT_TRUE(dsMaterial_setTexture(material, 1, texture1));
	EXPECT_TRUE(dsMaterial_setTexture(material, 3, texture2));
	EXPECT_TRUE(dsMaterial_setTexture(material, 5, texture3));

	EXPECT_FALSE(dsMaterial_getTexture(material, 0));
	EXPECT_EQ(texture1, dsMaterial_getTexture(material, 1));
	EXPECT_FALSE(dsMaterial_getTextureBuffer(NULL, NULL, NULL, material, 1));
	EXPECT_FALSE(dsMaterial_getTexture(material, 2));
	EXPECT_EQ(texture2, dsMaterial_getTexture(material, 3));
	EXPECT_FALSE(dsMaterial_getTexture(material, 4));
	EXPECT_EQ(texture3, dsMaterial_getTexture(material, 5));

	dsMaterial_destroy(material);
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(groupDesc));
	EXPECT_TRUE(dsTexture_destroy(texture1));
	EXPECT_TRUE(dsTexture_destroy(texture2));
	EXPECT_TRUE(dsTexture_destroy(texture3));
}

TEST_F(MaterialTest, TextureBuffers)
{
	dsShaderVariableElement groupElements[] =
	{
		{"testValue", dsMaterialType_Float, 0}
	};

	dsShaderVariableGroupDesc* groupDesc = dsShaderVariableGroupDesc_create(resourceManager, NULL,
		groupElements, DS_ARRAY_SIZE(groupElements));

	dsMaterialElement elements[] =
	{
		{"float", dsMaterialType_Float, 0, NULL, dsMaterialBinding_Material, 0},
		{"texture", dsMaterialType_TextureBuffer, 0, NULL, dsMaterialBinding_Material, 0},
		{"variableGroup", dsMaterialType_VariableGroup, 0, groupDesc, dsMaterialBinding_Material,
			0},
		{"mutableTexture", dsMaterialType_ImageBuffer, 0, NULL, dsMaterialBinding_Material, 0},
		{"buffer", dsMaterialType_UniformBlock, 0, NULL, dsMaterialBinding_Material, 0},
		{"subpassInput", dsMaterialType_SubpassInput, 0, NULL, dsMaterialBinding_Material, 0},
		{"sharedTexture", dsMaterialType_TextureBuffer, 0, NULL, dsMaterialBinding_Global, 0}
	};

	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		DS_ARRAY_SIZE(elements));
	ASSERT_TRUE(materialDesc);

	dsMaterial* material = dsMaterial_create(resourceManager, (dsAllocator*)&allocator,
		materialDesc);
	ASSERT_TRUE(material);

	dsGfxBuffer* buffer1 = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_Texture,
		dsGfxMemory_Dynamic, NULL, 1024);
	ASSERT_TRUE(buffer1);

	dsGfxBuffer* buffer2 = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_Image,
		dsGfxMemory_Dynamic, NULL, 1024);
	ASSERT_TRUE(buffer2);

	dsGfxBuffer* buffer3 = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_Vertex,
		dsGfxMemory_Dynamic, NULL, 1024);
	ASSERT_TRUE(buffer3);

	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	EXPECT_FALSE(dsMaterial_setTextureBuffer(material, 0, buffer1, format, 0, 256));
	EXPECT_FALSE(dsMaterial_setTextureBuffer(material, 1, buffer1, format, 0, 1024));
	EXPECT_FALSE(dsMaterial_setTextureBuffer(material, 1, buffer1, format, 24, 256));
	EXPECT_FALSE(dsMaterial_setTextureBuffer(material, 1, buffer3, format, 0, 256));
	EXPECT_FALSE(dsMaterial_setTextureBuffer(material, 2, buffer1, format, 0, 256));
	EXPECT_FALSE(dsMaterial_setTextureBuffer(material, 3, buffer2, dsGfxFormat_BC1_RGB, 0, 256));
	EXPECT_FALSE(dsMaterial_setTextureBuffer(material, 4, buffer1, format, 0, 256));
	EXPECT_FALSE(dsMaterial_setTextureBuffer(material, 5, buffer1, format, 0, 256));
	EXPECT_FALSE(dsMaterial_setTextureBuffer(material, 5, buffer1, format, 0, 256));
	EXPECT_FALSE(dsMaterial_setTextureBuffer(material, 6, buffer1, format, 0, 256));

	EXPECT_TRUE(dsMaterial_setTextureBuffer(material, 1, buffer1, format, 0, 256));
	EXPECT_FALSE(dsMaterial_setTextureBuffer(material, 3, buffer2, format, 24, 20));
	EXPECT_TRUE(dsMaterial_setTextureBuffer(material, 3, buffer2, format, 32, 20));

	dsGfxFormat storedFormat;
	size_t offset, count;
	EXPECT_FALSE(dsMaterial_getTextureBuffer(NULL, NULL, NULL, material, 0));
	EXPECT_EQ(buffer1, dsMaterial_getTextureBuffer(NULL, NULL, NULL, material, 1));
	EXPECT_EQ(buffer1, dsMaterial_getTextureBuffer(&storedFormat, &offset, &count, material, 1));
	EXPECT_EQ(format, storedFormat);
	EXPECT_EQ(0U, offset);
	EXPECT_EQ(256U, count);
	EXPECT_FALSE(dsMaterial_getTexture(material, 1));
	EXPECT_FALSE(dsMaterial_getTextureBuffer(NULL, NULL, NULL, material, 2));
	EXPECT_EQ(buffer2, dsMaterial_getTextureBuffer(&storedFormat, &offset, &count, material, 3));
	EXPECT_EQ(format, storedFormat);
	EXPECT_EQ(32U, offset);
	EXPECT_EQ(20U, count);
	EXPECT_FALSE(dsMaterial_getTextureBuffer(NULL, NULL, NULL, material, 4));
	EXPECT_FALSE(dsMaterial_getTextureBuffer(NULL, NULL, NULL, material, 5));

	resourceManager->maxTextureBufferElements = 128;
	EXPECT_FALSE(dsMaterial_setTextureBuffer(material, 1, buffer1, format, 0, 256));

	resourceManager->maxTextureBufferElements = 16*1024*1024;
	resourceManager->hasTextureBufferSubrange = false;
	EXPECT_FALSE(dsMaterial_setTextureBuffer(material, 1, buffer1, format, 4, 255));
	EXPECT_FALSE(dsMaterial_setTextureBuffer(material, 1, buffer1, format, 0, 255));
	EXPECT_TRUE(dsMaterial_setTextureBuffer(material, 1, buffer1, format, 0, 256));

	dsMaterial_destroy(material);
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(groupDesc));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer1));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer2));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer3));
}

TEST_F(MaterialTest, ShaderVariableGroups)
{
	dsShaderVariableElement groupElements[] =
	{
		{"testValue", dsMaterialType_Float, 0}
	};

	dsShaderVariableGroupDesc* groupDesc1 = dsShaderVariableGroupDesc_create(resourceManager, NULL,
		groupElements, DS_ARRAY_SIZE(groupElements));
	ASSERT_TRUE(groupDesc1);

	dsShaderVariableGroupDesc* groupDesc2 = dsShaderVariableGroupDesc_create(resourceManager, NULL,
		groupElements, DS_ARRAY_SIZE(groupElements));
	ASSERT_TRUE(groupDesc2);

	dsMaterialElement elements[] =
	{
		{"float", dsMaterialType_Float, 0, NULL, dsMaterialBinding_Material, 0},
		{"variableGroup", dsMaterialType_VariableGroup, 0, groupDesc1, dsMaterialBinding_Material,
			0},
		{"texture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0},
		{"sharedVariableGroup", dsMaterialType_VariableGroup, 0, groupDesc2,
			dsMaterialBinding_Global, 0},
		{"buffer", dsMaterialType_UniformBlock, 0, NULL, dsMaterialBinding_Material, 0}
	};

	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		DS_ARRAY_SIZE(elements));
	ASSERT_TRUE(materialDesc);

	dsMaterial* material = dsMaterial_create(resourceManager, (dsAllocator*)&allocator,
		materialDesc);
	ASSERT_TRUE(material);

	dsShaderVariableGroup* variableGroup1 = dsShaderVariableGroup_create(resourceManager, NULL,
		NULL, groupDesc1);
	ASSERT_TRUE(variableGroup1);

	dsShaderVariableGroup* variableGroup2 = dsShaderVariableGroup_create(resourceManager, NULL,
		NULL, groupDesc2);
	ASSERT_TRUE(variableGroup2);

	EXPECT_FALSE(dsMaterial_setVariableGroup(material, 0, variableGroup1));
	EXPECT_TRUE(dsMaterial_setVariableGroup(material, 1, variableGroup1));
	EXPECT_FALSE(dsMaterial_setVariableGroup(material, 1, variableGroup2));
	EXPECT_FALSE(dsMaterial_setVariableGroup(material, 2, variableGroup1));
	EXPECT_FALSE(dsMaterial_setVariableGroup(material, 3, variableGroup1));
	EXPECT_FALSE(dsMaterial_setVariableGroup(material, 4, variableGroup1));

	EXPECT_FALSE(dsMaterial_getVariableGroup(material, 0));
	EXPECT_EQ(variableGroup1, dsMaterial_getVariableGroup(material, 1));
	EXPECT_FALSE(dsMaterial_getVariableGroup(material, 2));
	EXPECT_FALSE(dsMaterial_getVariableGroup(material, 3));
	EXPECT_FALSE(dsMaterial_getVariableGroup(material, 4));

	dsMaterial_destroy(material);
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	EXPECT_TRUE(dsShaderVariableGroup_destroy(variableGroup1));
	EXPECT_TRUE(dsShaderVariableGroup_destroy(variableGroup2));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(groupDesc1));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(groupDesc2));
}

TEST_F(MaterialTest, Buffers)
{
	dsShaderVariableElement groupElements[] =
	{
		{"testValue", dsMaterialType_Float, 0}
	};

	dsShaderVariableGroupDesc* groupDesc = dsShaderVariableGroupDesc_create(resourceManager, NULL,
		groupElements, DS_ARRAY_SIZE(groupElements));

	dsMaterialElement elements[] =
	{
		{"float", dsMaterialType_Float, 0, NULL, dsMaterialBinding_Material, 0},
		{"uniformBlock", dsMaterialType_UniformBlock, 0, NULL, dsMaterialBinding_Material, 0},
		{"texture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0},
		{"uniformBuffer", dsMaterialType_UniformBuffer, 0, NULL, dsMaterialBinding_Material, 0},
		{"variableGroup", dsMaterialType_VariableGroup, 0, groupDesc, dsMaterialBinding_Material,
			0},
		{"sharedBuffer", dsMaterialType_UniformBlock, 0, NULL, dsMaterialBinding_Global, 0}
	};

	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		DS_ARRAY_SIZE(elements));
	ASSERT_TRUE(materialDesc);

	dsMaterial* material = dsMaterial_create(resourceManager, (dsAllocator*)&allocator,
		materialDesc);
	ASSERT_TRUE(material);

	dsGfxBuffer* buffer1 = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static, NULL, 128);
	ASSERT_TRUE(buffer1);

	dsGfxBuffer* buffer2 = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_UniformBuffer | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static, NULL, 128);
	ASSERT_TRUE(buffer2);

	EXPECT_FALSE(dsMaterial_setBuffer(material, 0, buffer1, 0, 128));
	EXPECT_FALSE(dsMaterial_setBuffer(material, 1, buffer2, 0, 128));
	EXPECT_FALSE(dsMaterial_setBuffer(material, 2, buffer1, 0, 128));
	EXPECT_FALSE(dsMaterial_setBuffer(material, 3, buffer1, 0, 128));
	EXPECT_FALSE(dsMaterial_setBuffer(material, 4, buffer1, 0, 128));
	EXPECT_FALSE(dsMaterial_setBuffer(material, 5, buffer1, 0, 128));

	EXPECT_FALSE(dsMaterial_setBuffer(material, 1, buffer1, 128, 64));
	EXPECT_TRUE(dsMaterial_setBuffer(material, 1, buffer1, 0, 128));
	EXPECT_TRUE(dsMaterial_setBuffer(material, 3, buffer2, 32, 96));

	size_t offset, size;
	EXPECT_FALSE(dsMaterial_getBuffer(&offset, &size, material, 0));

	EXPECT_EQ(buffer1, dsMaterial_getBuffer(&offset, &size, material, 1));
	EXPECT_EQ(0U, offset);
	EXPECT_EQ(128U, size);

	EXPECT_FALSE(dsMaterial_getBuffer(&offset, &size, material, 2));

	EXPECT_EQ(buffer2, dsMaterial_getBuffer(&offset, &size, material, 3));
	EXPECT_EQ(32U, offset);
	EXPECT_EQ(96U, size);

	EXPECT_FALSE(dsMaterial_getBuffer(&offset, &size, material, 4));
	EXPECT_FALSE(dsMaterial_getBuffer(&offset, &size, material, 5));

	resourceManager->maxUniformBlockSize = 64;
	EXPECT_FALSE(dsMaterial_setBuffer(material, 1, buffer1, 0, 128));

	dsMaterial_destroy(material);
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(groupDesc));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer1));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer2));
}

TEST_F(MaterialTest, MixedTypes)
{
	dsShaderVariableElement groupElements[] =
	{
		{"testValue", dsMaterialType_Float, 0}
	};

	dsShaderVariableGroupDesc* groupDesc = dsShaderVariableGroupDesc_create(resourceManager, NULL,
		groupElements, DS_ARRAY_SIZE(groupElements));

	dsMaterialElement elements[] =
	{
		{"float", dsMaterialType_Float, 0, NULL, dsMaterialBinding_Material, 0},
		{"sharedTexture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Global, 0},
		{"texture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0},
		{"uniformBlock", dsMaterialType_UniformBlock, 0, NULL, dsMaterialBinding_Material, 0},
		{"variableGroup", dsMaterialType_VariableGroup, 0, groupDesc, dsMaterialBinding_Material,
			0}
	};

	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		DS_ARRAY_SIZE(elements));
	ASSERT_TRUE(materialDesc);

	dsMaterial* material = dsMaterial_create(resourceManager, (dsAllocator*)&allocator,
		materialDesc);
	ASSERT_TRUE(material);

	dsTextureInfo texInfo = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		dsTextureDim_2D, 16, 16, 0, DS_ALL_MIP_LEVELS, 1};
	dsTexture* texture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo, dsGfxMemory_Static, &texInfo, NULL, 0);
	ASSERT_TRUE(texture);

	dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static, NULL, 128);
	ASSERT_TRUE(buffer);

	dsShaderVariableGroup* variableGroup = dsShaderVariableGroup_create(resourceManager, NULL, NULL,
		groupDesc);
	ASSERT_TRUE(variableGroup);

	float floatVal = 1.2f;
	EXPECT_TRUE(dsMaterial_setElementData(material, 0, &floatVal, dsMaterialType_Float, 0, 1));
	EXPECT_TRUE(dsMaterial_setTexture(material, 2, texture));
	EXPECT_TRUE(dsMaterial_setBuffer(material, 3, buffer, 0, 128));
	EXPECT_TRUE(dsMaterial_setVariableGroup(material, 4, variableGroup));

	floatVal = 0;
	EXPECT_TRUE(dsMaterial_getElementData(&floatVal, material, 0, dsMaterialType_Float, 0, 1));
	EXPECT_EQ(texture, dsMaterial_getTexture(material, 2));

	size_t offset, size;
	EXPECT_EQ(buffer, dsMaterial_getBuffer(&offset, &size, material, 3));
	EXPECT_EQ(0U, offset);
	EXPECT_EQ(128U, size);

	EXPECT_EQ(variableGroup, dsMaterial_getVariableGroup(material, 4));

	dsMaterial_destroy(material);
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));
	EXPECT_TRUE(dsShaderVariableGroup_destroy(variableGroup));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(groupDesc));
}

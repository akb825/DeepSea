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

#include "AssetFixtureBase.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/VolatileMaterialValues.h>
#include <gtest/gtest.h>

class ShaderTest : public AssetFixtureBase
{
public:
	ShaderTest()
		: AssetFixtureBase("shaders")
	{
	}
};

TEST_F(ShaderTest, Create)
{
	dsShaderVariableElement transformElements[] =
	{
		{"modelViewProjection", dsMaterialType_Mat4, 0},
		{"normalMat", dsMaterialType_Mat3, 0}
	};
	unsigned int transformElementCount = (unsigned int)DS_ARRAY_SIZE(transformElements);
	dsShaderVariableGroupDesc* transformDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, transformElements, transformElementCount);
	ASSERT_TRUE(transformDesc);

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, false, 0},
		{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, false, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, false, 0},
		{"Transform", dsMaterialType_VariableGroup, 0, transformDesc, true, 0},
		{"extraVar", dsMaterialType_Int, 0, NULL, false, 0}
	};
	unsigned int elementCount = (unsigned int)DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadFile(resourceManager, NULL,
		getPath("test.mslb"));
	ASSERT_TRUE(shaderModule);

	EXPECT_FALSE(dsShader_createName(NULL, NULL, shaderModule, "Test", materialDesc,
		dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, NULL, "Test", materialDesc,
		dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, shaderModule, NULL, materialDesc,
		dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, shaderModule, "asdf", materialDesc,
		dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, shaderModule, "Test", NULL,
		dsPrimitiveType_TriangleList));
	dsShader* shader = dsShader_createName(resourceManager, NULL, shaderModule, "Test",
		materialDesc, dsPrimitiveType_TriangleList);
	ASSERT_TRUE(shader);

	EXPECT_TRUE(dsShader_destroy(shader));
	EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(transformDesc));
}

TEST_F(ShaderTest, CreateNoBuffers)
{
	resourceManager->supportedBuffers =
		(dsGfxBufferUsage)(resourceManager->supportedBuffers & ~dsGfxBufferUsage_UniformBlock);

	dsShaderVariableElement transformElements[] =
	{
		{"modelViewProjection", dsMaterialType_Mat4, 0},
		{"normalMat", dsMaterialType_Mat3, 0}
	};
	unsigned int transformElementCount = (unsigned int)DS_ARRAY_SIZE(transformElements);
	dsShaderVariableGroupDesc* transformDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, transformElements, transformElementCount);
	ASSERT_TRUE(transformDesc);

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, false, 0},
		{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, false, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, false, 0},
		{"Transform", dsMaterialType_VariableGroup, 0, transformDesc, true, 0},
		{"extraVar", dsMaterialType_Int, 0, NULL, false, 0}
	};
	unsigned int elementCount = (unsigned int)DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadFile(resourceManager, NULL,
		getPath("test-nobuffers.mslb"));
	ASSERT_TRUE(shaderModule);

	dsShader* shader = dsShader_createName(resourceManager, NULL, shaderModule, "Test",
		materialDesc, dsPrimitiveType_TriangleList);
	ASSERT_TRUE(shader);

	EXPECT_TRUE(dsShader_destroy(shader));
	EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(transformDesc));
}

TEST_F(ShaderTest, CreateNoBuffersDuplicateElements)
{
	resourceManager->supportedBuffers =
		(dsGfxBufferUsage)(resourceManager->supportedBuffers & ~dsGfxBufferUsage_UniformBlock);

	dsShaderVariableElement transformElements[] =
	{
		{"modelViewProjection", dsMaterialType_Mat4, 0},
		{"normalMat", dsMaterialType_Mat3, 0}
	};
	unsigned int transformElementCount = (unsigned int)DS_ARRAY_SIZE(transformElements);
	dsShaderVariableGroupDesc* transformDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, transformElements, transformElementCount);
	ASSERT_TRUE(transformDesc);

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, false, 0},
		{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, false, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, false, 0},
		{"Transform", dsMaterialType_VariableGroup, 0, transformDesc, true, 0},
		{"OtherTransform", dsMaterialType_VariableGroup, 0, transformDesc, true, 0},
		{"extraVar", dsMaterialType_Int, 0, NULL, false, 0}
	};
	unsigned int elementCount = (unsigned int)DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadFile(resourceManager, NULL,
		getPath("test-nobuffers.mslb"));
	ASSERT_TRUE(shaderModule);

	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, shaderModule, "Test", materialDesc,
		dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(transformDesc));
}

TEST_F(ShaderTest, CreateTypeMismatch)
{
	dsShaderVariableElement transformElements[] =
	{
		{"modelViewProjection", dsMaterialType_Mat4, 0},
		{"normalMat", dsMaterialType_Mat3, 0}
	};
	unsigned int transformElementCount = (unsigned int)DS_ARRAY_SIZE(transformElements);
	dsShaderVariableGroupDesc* transformDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, transformElements, transformElementCount);
	ASSERT_TRUE(transformDesc);

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, false, 0},
		{"colorMultiplier", dsMaterialType_Vec3, 0, NULL, false, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, false, 0},
		{"Transform", dsMaterialType_VariableGroup, 0, transformDesc, true, 0},
	};
	unsigned int elementCount = (unsigned int)DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadFile(resourceManager, NULL,
		getPath("test.mslb"));
	ASSERT_TRUE(shaderModule);

	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, shaderModule, "Test", materialDesc,
		dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(transformDesc));
}

TEST_F(ShaderTest, CreateMissingVariable)
{
	dsShaderVariableElement transformElements[] =
	{
		{"modelViewProjection", dsMaterialType_Mat4, 0},
		{"normalMat", dsMaterialType_Mat3, 0}
	};
	unsigned int transformElementCount = (unsigned int)DS_ARRAY_SIZE(transformElements);
	dsShaderVariableGroupDesc* transformDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, transformElements, transformElementCount);
	ASSERT_TRUE(transformDesc);

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, false, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, false, 0},
		{"Transform", dsMaterialType_VariableGroup, 0, transformDesc, true, 0},
	};
	unsigned int elementCount = (unsigned int)DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadFile(resourceManager, NULL,
		getPath("test.mslb"));
	ASSERT_TRUE(shaderModule);

	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, shaderModule, "Test", materialDesc,
		dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(transformDesc));
}

TEST_F(ShaderTest, CreateVariableGroupTypeMismatch)
{
	dsShaderVariableElement transformElements[] =
	{
		{"modelViewProjection", dsMaterialType_Mat4, 0},
		{"normalMat", dsMaterialType_Mat4, 0}
	};
	unsigned int transformElementCount = (unsigned int)DS_ARRAY_SIZE(transformElements);
	dsShaderVariableGroupDesc* transformDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, transformElements, transformElementCount);
	ASSERT_TRUE(transformDesc);

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, false, 0},
		{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, false, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, false, 0},
		{"Transform", dsMaterialType_VariableGroup, 0, transformDesc, true, 0},
	};
	unsigned int elementCount = (unsigned int)DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadFile(resourceManager, NULL,
		getPath("test.mslb"));
	ASSERT_TRUE(shaderModule);

	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, shaderModule, "Test", materialDesc,
		dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(transformDesc));
}

TEST_F(ShaderTest, CreateVariableGroupElementMismatch)
{
	dsShaderVariableElement transformElements[] =
	{
		{"modelViewProjection", dsMaterialType_Mat4, 0},
		{"integer", dsMaterialType_Int, 0},
		{"normalMat", dsMaterialType_Mat4, 0}
	};
	unsigned int transformElementCount = (unsigned int)DS_ARRAY_SIZE(transformElements);
	dsShaderVariableGroupDesc* transformDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, transformElements, transformElementCount);
	ASSERT_TRUE(transformDesc);

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, false, 0},
		{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, false, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, false, 0},
		{"Transform", dsMaterialType_VariableGroup, 0, transformDesc, true, 0},
	};
	unsigned int elementCount = (unsigned int)DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadFile(resourceManager, NULL,
		getPath("test.mslb"));
	ASSERT_TRUE(shaderModule);

	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, shaderModule, "Test", materialDesc,
		dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(transformDesc));
}

TEST_F(ShaderTest, BindAndUpdate)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsShaderVariableElement transformElements[] =
	{
		{"modelViewProjection", dsMaterialType_Mat4, 0},
		{"normalMat", dsMaterialType_Mat3, 0}
	};
	unsigned int transformElementCount = (unsigned int)DS_ARRAY_SIZE(transformElements);
	dsShaderVariableGroupDesc* transformDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, transformElements, transformElementCount);
	ASSERT_TRUE(transformDesc);

	dsShaderVariableElement groupElements[] =
	{
		{"testValue", dsMaterialType_Float, 0}
	};
	unsigned int groupElementCount = (unsigned int)DS_ARRAY_SIZE(groupElements);
	dsShaderVariableGroupDesc* groupDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, groupElements, groupElementCount);
	ASSERT_TRUE(groupDesc);

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, true, 0},
		{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, false, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, false, 0},
		{"Transform", dsMaterialType_VariableGroup, 0, transformDesc, true, 0},
	};
	unsigned int elementCount = (unsigned int)DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadFile(resourceManager, NULL,
		getPath("test.mslb"));
	ASSERT_TRUE(shaderModule);

	dsShader* shader = dsShader_createName(resourceManager, NULL, shaderModule, "Test",
		materialDesc, dsPrimitiveType_TriangleList);
	ASSERT_TRUE(shader);

	dsMaterial* material = dsMaterial_create((dsAllocator*)&allocator, materialDesc);
	ASSERT_TRUE(material);

	dsShaderVariableGroup* transformGroup = dsShaderVariableGroup_create(resourceManager, NULL,
		NULL, transformDesc);
	ASSERT_TRUE(transformGroup);

	dsShaderVariableGroup* group = dsShaderVariableGroup_create(resourceManager, NULL, NULL,
		groupDesc);
	ASSERT_TRUE(group);

	dsTexture* texture1 = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo, dsGfxMemory_Static,
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), dsTextureDim_2D, 16, 16, 0,
		DS_ALL_MIP_LEVELS, NULL, 0);
	ASSERT_TRUE(texture1);

	dsTexture* texture2 = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Image | dsTextureUsage_CopyTo, dsGfxMemory_Static,
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), dsTextureDim_2D, 16, 16, 0,
		DS_ALL_MIP_LEVELS, NULL, 0);
	ASSERT_TRUE(texture2);

	dsVolatileMaterialValues* volatileValues = dsVolatileMaterialValues_create(
		(dsAllocator*)&allocator, DS_DEFAULT_MAX_VOLATILE_MATERIAL_VALUES);
	ASSERT_TRUE(volatileValues);

	EXPECT_TRUE(dsVolatileMaterialValues_setTextureName(volatileValues, "diffuseTexture",
		texture1));
	EXPECT_TRUE(dsVolatileMaterialValues_setVariableGroupName(volatileValues, "Transform",
		transformGroup));

	EXPECT_FALSE(dsShader_bind(NULL, shader, material, volatileValues, NULL));
	EXPECT_FALSE(dsShader_bind(commandBuffer, NULL, material, volatileValues, NULL));
	EXPECT_FALSE(dsShader_bind(commandBuffer, shader, NULL, volatileValues, NULL));

	EXPECT_TRUE(dsVolatileMaterialValues_removeValueName(volatileValues, "diffuseTexture"));
	EXPECT_FALSE(dsShader_bind(commandBuffer, shader, material, volatileValues, NULL));

	EXPECT_TRUE(dsVolatileMaterialValues_setTextureName(volatileValues, "diffuseTexture",
		texture2));
	EXPECT_FALSE(dsShader_bind(commandBuffer, shader, material, volatileValues, NULL));

	EXPECT_TRUE(dsVolatileMaterialValues_setTextureName(volatileValues, "diffuseTexture",
		texture1));
	EXPECT_TRUE(dsVolatileMaterialValues_removeValueName(volatileValues, "Transform"));
	EXPECT_FALSE(dsShader_bind(commandBuffer, shader, material, volatileValues, NULL));

	EXPECT_TRUE(dsVolatileMaterialValues_setVariableGroupName(volatileValues, "Transform",
		group));
	EXPECT_FALSE(dsShader_bind(commandBuffer, shader, material, volatileValues, NULL));

	EXPECT_TRUE(dsVolatileMaterialValues_setVariableGroupName(volatileValues, "Transform",
		transformGroup));
	EXPECT_TRUE(dsShader_bind(commandBuffer, shader, material, volatileValues, NULL));

	EXPECT_FALSE(dsShader_updateVolatileValues(commandBuffer, shader, NULL));

	EXPECT_TRUE(dsVolatileMaterialValues_removeValueName(volatileValues, "diffuseTexture"));
	EXPECT_FALSE(dsShader_updateVolatileValues(commandBuffer, shader, volatileValues));

	EXPECT_TRUE(dsVolatileMaterialValues_setTextureName(volatileValues, "diffuseTexture",
		texture2));
	EXPECT_FALSE(dsShader_updateVolatileValues(commandBuffer, shader, volatileValues));

	EXPECT_TRUE(dsVolatileMaterialValues_setTextureName(volatileValues, "diffuseTexture",
		texture1));
	EXPECT_TRUE(dsVolatileMaterialValues_removeValueName(volatileValues, "Transform"));
	EXPECT_FALSE(dsShader_updateVolatileValues(commandBuffer, shader, volatileValues));

	EXPECT_TRUE(dsVolatileMaterialValues_setVariableGroupName(volatileValues, "Transform",
		group));
	EXPECT_FALSE(dsShader_updateVolatileValues(commandBuffer, shader, volatileValues));

	EXPECT_TRUE(dsVolatileMaterialValues_setVariableGroupName(volatileValues, "Transform",
		transformGroup));
	EXPECT_TRUE(dsShader_updateVolatileValues(commandBuffer, shader, volatileValues));

	EXPECT_FALSE(dsShader_unbind(NULL, shader));
	EXPECT_FALSE(dsShader_unbind(commandBuffer, NULL));

	EXPECT_TRUE(dsShader_unbind(commandBuffer, shader));

	EXPECT_TRUE(dsShader_destroy(shader));
	EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
	dsVolatileMaterialValues_destroy(volatileValues);
	EXPECT_TRUE(dsShaderVariableGroup_destroy(group));
	EXPECT_TRUE(dsShaderVariableGroup_destroy(transformGroup));
	EXPECT_TRUE(dsTexture_destroy(texture1));
	EXPECT_TRUE(dsTexture_destroy(texture2));
	dsMaterial_destroy(material);
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(groupDesc));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(transformDesc));
}

TEST_F(ShaderTest, BindAndUpdateBuffer)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, false, 0},
		{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, false, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, false, 0},
		{"Transform", dsMaterialType_UniformBlock, 0, NULL, true, 0},
	};
	unsigned int elementCount = (unsigned int)DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadFile(resourceManager, NULL,
		getPath("test.mslb"));
	ASSERT_TRUE(shaderModule);

	dsShader* shader = dsShader_createName(resourceManager, NULL, shaderModule, "Test",
		materialDesc, dsPrimitiveType_TriangleList);
	ASSERT_TRUE(shader);

	dsMaterial* material = dsMaterial_create((dsAllocator*)&allocator, materialDesc);
	ASSERT_TRUE(material);

	dsGfxBuffer* buffer1 = dsGfxBuffer_create(resourceManager, (dsAllocator*)&allocator,
		dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static, NULL,
		sizeof(float)*28);
	ASSERT_TRUE(buffer1);

	dsGfxBuffer* buffer2 = dsGfxBuffer_create(resourceManager, (dsAllocator*)&allocator,
		dsGfxBufferUsage_UniformBuffer | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static, NULL,
		sizeof(float)*28);
	ASSERT_TRUE(buffer2);

	dsVolatileMaterialValues* volatileValues = dsVolatileMaterialValues_create(
		(dsAllocator*)&allocator, DS_DEFAULT_MAX_VOLATILE_MATERIAL_VALUES);
	ASSERT_TRUE(volatileValues);

	EXPECT_FALSE(dsShader_bind(commandBuffer, shader, material, volatileValues, NULL));

	EXPECT_TRUE(dsVolatileMaterialValues_setBufferName(volatileValues, "Transform", buffer2, 0,
		buffer2->size));
	EXPECT_FALSE(dsShader_bind(commandBuffer, shader, material, volatileValues, NULL));

	EXPECT_TRUE(dsVolatileMaterialValues_setBufferName(volatileValues, "Transform", buffer1, 0,
		buffer1->size));
	EXPECT_TRUE(dsShader_bind(commandBuffer, shader, material, volatileValues, NULL));

	EXPECT_TRUE(dsVolatileMaterialValues_removeValueName(volatileValues, "Transform"));
	EXPECT_FALSE(dsShader_updateVolatileValues(commandBuffer, shader, volatileValues));

	EXPECT_TRUE(dsVolatileMaterialValues_setBufferName(volatileValues, "Transform", buffer2, 0,
		buffer2->size));
	EXPECT_FALSE(dsShader_updateVolatileValues(commandBuffer, shader, volatileValues));

	EXPECT_TRUE(dsVolatileMaterialValues_setBufferName(volatileValues, "Transform", buffer1, 0,
		buffer1->size));
	EXPECT_TRUE(dsShader_updateVolatileValues(commandBuffer, shader, volatileValues));

	EXPECT_FALSE(dsShader_unbind(NULL, shader));
	EXPECT_FALSE(dsShader_unbind(commandBuffer, NULL));

	EXPECT_TRUE(dsShader_unbind(commandBuffer, shader));

	EXPECT_TRUE(dsShader_destroy(shader));
	EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
	dsVolatileMaterialValues_destroy(volatileValues);
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer1));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer2));
	dsMaterial_destroy(material);
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
}

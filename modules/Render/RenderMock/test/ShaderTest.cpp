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

#include "Fixtures/AssetFixtureBase.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Render/Resources/Framebuffer.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/RenderPass.h>
#include <DeepSea/Render/RenderSurface.h>
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
	unsigned int transformElementCount = DS_ARRAY_SIZE(transformElements);
	dsShaderVariableGroupDesc* transformDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, transformElements, transformElementCount);
	ASSERT_TRUE(transformDesc);

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0},
		{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, dsMaterialBinding_Material, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, dsMaterialBinding_Material, 0},
		{"Transform", dsMaterialType_VariableGroup, 0, transformDesc, dsMaterialBinding_Global, 0},
		{"extraVar", dsMaterialType_Int, 0, NULL, dsMaterialBinding_Material, 0}
	};
	unsigned int elementCount = DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadResource(resourceManager, NULL,
		dsFileResourceType_Embedded, getRelativePath("test.mslb"), "test");
	ASSERT_TRUE(shaderModule);

	EXPECT_FALSE(dsShader_createName(NULL, NULL, shaderModule, "Test", materialDesc));
	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, NULL, "Test", materialDesc));
	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, shaderModule, NULL, materialDesc));
	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, shaderModule, "asdf", materialDesc));
	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, shaderModule, "Test", NULL));
	dsShader* shader = dsShader_createName(resourceManager, NULL, shaderModule, "Test",
		materialDesc);
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
	unsigned int transformElementCount = DS_ARRAY_SIZE(transformElements);
	dsShaderVariableGroupDesc* transformDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, transformElements, transformElementCount);
	ASSERT_TRUE(transformDesc);

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0},
		{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, dsMaterialBinding_Material, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, dsMaterialBinding_Material, 0},
		{"Transform", dsMaterialType_VariableGroup, 0, transformDesc, dsMaterialBinding_Global, 0},
		{"extraVar", dsMaterialType_Int, 0, NULL, dsMaterialBinding_Material, 0}
	};
	unsigned int elementCount = DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadResource(resourceManager, NULL,
		dsFileResourceType_Embedded, getRelativePath("test-nobuffers.mslb"), "test");
	ASSERT_TRUE(shaderModule);

	dsShader* shader = dsShader_createName(resourceManager, NULL, shaderModule, "Test",
		materialDesc);
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
	unsigned int transformElementCount = DS_ARRAY_SIZE(transformElements);
	dsShaderVariableGroupDesc* transformDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, transformElements, transformElementCount);
	ASSERT_TRUE(transformDesc);

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0},
		{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, dsMaterialBinding_Material, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, dsMaterialBinding_Material, 0},
		{"Transform", dsMaterialType_VariableGroup, 0, transformDesc, dsMaterialBinding_Global, 0},
		{"OtherTransform", dsMaterialType_VariableGroup, 0, transformDesc, dsMaterialBinding_Global,
			0},
		{"extraVar", dsMaterialType_Int, 0, NULL, dsMaterialBinding_Material, 0}
	};
	unsigned int elementCount = DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadResource(resourceManager, NULL,
		dsFileResourceType_Embedded, getRelativePath("test-nobuffers.mslb"), "test");
	ASSERT_TRUE(shaderModule);

	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, shaderModule, "Test", materialDesc));

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
	unsigned int transformElementCount = DS_ARRAY_SIZE(transformElements);
	dsShaderVariableGroupDesc* transformDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, transformElements, transformElementCount);
	ASSERT_TRUE(transformDesc);

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0},
		{"colorMultiplier", dsMaterialType_Vec3, 0, NULL, dsMaterialBinding_Material, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, dsMaterialBinding_Material, 0},
		{"Transform", dsMaterialType_VariableGroup, 0, transformDesc, dsMaterialBinding_Global, 0},
	};
	unsigned int elementCount = DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadResource(resourceManager, NULL,
		dsFileResourceType_Embedded, getRelativePath("test.mslb"), "test");
	ASSERT_TRUE(shaderModule);

	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, shaderModule, "Test", materialDesc));

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
	unsigned int transformElementCount = DS_ARRAY_SIZE(transformElements);
	dsShaderVariableGroupDesc* transformDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, transformElements, transformElementCount);
	ASSERT_TRUE(transformDesc);

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, dsMaterialBinding_Material, 0},
		{"Transform", dsMaterialType_VariableGroup, 0, transformDesc, dsMaterialBinding_Global, 0},
	};
	unsigned int elementCount = DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadResource(resourceManager, NULL,
		dsFileResourceType_Embedded, getRelativePath("test.mslb"), "test");
	ASSERT_TRUE(shaderModule);

	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, shaderModule, "Test", materialDesc));

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
	unsigned int transformElementCount = DS_ARRAY_SIZE(transformElements);
	dsShaderVariableGroupDesc* transformDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, transformElements, transformElementCount);
	ASSERT_TRUE(transformDesc);

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0},
		{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, dsMaterialBinding_Material, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, dsMaterialBinding_Material, 0},
		{"Transform", dsMaterialType_VariableGroup, 0, transformDesc, dsMaterialBinding_Global, 0},
	};
	unsigned int elementCount = DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadResource(resourceManager, NULL,
		dsFileResourceType_Embedded, getRelativePath("test.mslb"), "test");
	ASSERT_TRUE(shaderModule);

	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, shaderModule, "Test", materialDesc));

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
	unsigned int transformElementCount = DS_ARRAY_SIZE(transformElements);
	dsShaderVariableGroupDesc* transformDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, transformElements, transformElementCount);
	ASSERT_TRUE(transformDesc);

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0},
		{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, dsMaterialBinding_Material, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, dsMaterialBinding_Material, 0},
		{"Transform", dsMaterialType_VariableGroup, 0, transformDesc, dsMaterialBinding_Global, 0},
	};
	unsigned int elementCount = DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadResource(resourceManager, NULL,
		dsFileResourceType_Embedded, getRelativePath("test.mslb"), "test");
	ASSERT_TRUE(shaderModule);

	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, shaderModule, "Test", materialDesc));

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
	unsigned int transformElementCount = DS_ARRAY_SIZE(transformElements);
	dsShaderVariableGroupDesc* transformDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, transformElements, transformElementCount);
	ASSERT_TRUE(transformDesc);

	dsShaderVariableElement groupElements[] =
	{
		{"testValue", dsMaterialType_Float, 0}
	};
	unsigned int groupElementCount = DS_ARRAY_SIZE(groupElements);
	dsShaderVariableGroupDesc* groupDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, groupElements, groupElementCount);
	ASSERT_TRUE(groupDesc);

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Global, 0},
		{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, dsMaterialBinding_Material, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, dsMaterialBinding_Material, 0},
		{"Transform", dsMaterialType_VariableGroup, 0, transformDesc, dsMaterialBinding_Instance,
			0},
	};
	unsigned int elementCount = DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadResource(resourceManager, NULL,
		dsFileResourceType_Embedded, getRelativePath("test.mslb"), "test");
	ASSERT_TRUE(shaderModule);

	dsShader* shader = dsShader_createName(resourceManager, NULL, shaderModule, "Test",
		materialDesc);
	ASSERT_TRUE(shader);

	dsMaterial* material = dsMaterial_create(resourceManager, (dsAllocator*)&allocator,
		materialDesc);
	ASSERT_TRUE(material);

	dsShaderVariableGroup* transformGroup = dsShaderVariableGroup_create(resourceManager, NULL,
		NULL, transformDesc);
	ASSERT_TRUE(transformGroup);

	dsShaderVariableGroup* group = dsShaderVariableGroup_create(resourceManager, NULL, NULL,
		groupDesc);
	ASSERT_TRUE(group);

	dsTextureInfo texInfo = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		dsTextureDim_2D, 16, 16, 0, DS_ALL_MIP_LEVELS, 1};
	dsTexture* texture1 = dsTexture_create(resourceManager, NULL,
		(dsTextureUsage)(dsTextureUsage_Texture | dsTextureUsage_CopyTo), dsGfxMemory_Static,
		&texInfo, NULL, 0);
	ASSERT_TRUE(texture1);

	dsTexture* texture2 = dsTexture_create(resourceManager, NULL,
		(dsTextureUsage)(dsTextureUsage_Image | dsTextureUsage_CopyTo), dsGfxMemory_Static,
		&texInfo, NULL, 0);
	ASSERT_TRUE(texture2);

	dsSharedMaterialValues* globalValues = dsSharedMaterialValues_create(
		(dsAllocator*)&allocator, DS_DEFAULT_MAX_SHARED_MATERIAL_VALUES);
	ASSERT_TRUE(globalValues);

	dsSharedMaterialValues* instanceValues = dsSharedMaterialValues_create(
		(dsAllocator*)&allocator, DS_DEFAULT_MAX_SHARED_MATERIAL_VALUES);
	ASSERT_TRUE(instanceValues);

	EXPECT_TRUE(dsSharedMaterialValues_setTextureName(globalValues, "diffuseTexture",
		texture1));
	EXPECT_TRUE(dsSharedMaterialValues_setVariableGroupName(instanceValues, "Transform",
		transformGroup));

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0));

	EXPECT_FALSE(dsShader_bind(shader, NULL, material, globalValues, NULL));
	EXPECT_FALSE(dsShader_bind(NULL, commandBuffer, material, globalValues, NULL));
	EXPECT_FALSE(dsShader_bind(shader, commandBuffer, NULL, globalValues, NULL));

	EXPECT_TRUE(dsSharedMaterialValues_removeValueName(globalValues, "diffuseTexture"));
	EXPECT_FALSE(dsShader_bind(shader, commandBuffer, material, globalValues, NULL));

	EXPECT_TRUE(dsSharedMaterialValues_setTextureName(globalValues, "diffuseTexture",
		texture2));
	EXPECT_FALSE(dsShader_bind(shader, commandBuffer, material, globalValues, NULL));

	EXPECT_TRUE(dsSharedMaterialValues_setTextureName(globalValues, "diffuseTexture",
		texture1));
	EXPECT_TRUE(dsShader_bind(shader, commandBuffer, material, globalValues, NULL));

	EXPECT_FALSE(dsShader_updateInstanceValues(shader, commandBuffer, NULL));

	EXPECT_TRUE(dsSharedMaterialValues_removeValueName(instanceValues, "Transform"));
	EXPECT_FALSE(dsShader_updateInstanceValues(shader, commandBuffer, instanceValues));

	EXPECT_TRUE(dsSharedMaterialValues_setVariableGroupName(instanceValues, "Transform", group));
	EXPECT_FALSE(dsShader_updateInstanceValues(shader, commandBuffer, instanceValues));

	EXPECT_TRUE(dsSharedMaterialValues_setVariableGroupName(instanceValues, "Transform",
		transformGroup));
	EXPECT_TRUE(dsShader_updateInstanceValues(shader, commandBuffer, instanceValues));

	EXPECT_FALSE(dsShader_unbind(shader, NULL));
	EXPECT_FALSE(dsShader_unbind(NULL, commandBuffer));

	EXPECT_FALSE(dsRenderPass_end(renderPass, commandBuffer));
	EXPECT_TRUE(dsShader_unbind(shader, commandBuffer));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	EXPECT_FALSE(dsShader_bind(shader, commandBuffer, material, globalValues, NULL));

	EXPECT_TRUE(dsShader_destroy(shader));
	EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
	dsSharedMaterialValues_destroy(globalValues);
	dsSharedMaterialValues_destroy(instanceValues);
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
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0},
		{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, dsMaterialBinding_Material, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, dsMaterialBinding_Material, 0},
		{"Transform", dsMaterialType_UniformBlock, 0, NULL, dsMaterialBinding_Global, 0},
	};
	unsigned int elementCount = DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadResource(resourceManager, NULL,
		dsFileResourceType_Embedded, getRelativePath("test.mslb"), "test");
	ASSERT_TRUE(shaderModule);

	dsShader* shader = dsShader_createName(resourceManager, NULL, shaderModule, "Test",
		materialDesc);
	ASSERT_TRUE(shader);

	dsMaterial* material = dsMaterial_create(resourceManager, (dsAllocator*)&allocator,
		materialDesc);
	ASSERT_TRUE(material);

	dsGfxBuffer* buffer1 = dsGfxBuffer_create(resourceManager, (dsAllocator*)&allocator,
		(dsGfxBufferUsage)(dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_CopyTo),
		dsGfxMemory_Static, NULL, sizeof(float)*28);
	ASSERT_TRUE(buffer1);

	dsGfxBuffer* buffer2 = dsGfxBuffer_create(resourceManager, (dsAllocator*)&allocator,
		(dsGfxBufferUsage)(dsGfxBufferUsage_UniformBuffer | dsGfxBufferUsage_CopyTo),
		dsGfxMemory_Static, NULL, sizeof(float)*28);
	ASSERT_TRUE(buffer2);

	dsSharedMaterialValues* globalValues = dsSharedMaterialValues_create(
		(dsAllocator*)&allocator, DS_DEFAULT_MAX_SHARED_MATERIAL_VALUES);
	ASSERT_TRUE(globalValues);

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0));

	EXPECT_FALSE(dsShader_bind(shader, commandBuffer, material, globalValues, NULL));

	EXPECT_TRUE(dsSharedMaterialValues_setBufferName(globalValues, "Transform", buffer2, 0,
		buffer2->size));
	EXPECT_FALSE(dsShader_bind(shader, commandBuffer, material, globalValues, NULL));

	EXPECT_TRUE(dsSharedMaterialValues_setBufferName(globalValues, "Transform", buffer1, 0,
		buffer1->size));
	EXPECT_TRUE(dsShader_bind(shader, commandBuffer, material, globalValues, NULL));

	EXPECT_FALSE(dsShader_unbind(shader, NULL));
	EXPECT_FALSE(dsShader_unbind(NULL, commandBuffer));

	EXPECT_FALSE(dsRenderPass_end(renderPass, commandBuffer));
	EXPECT_TRUE(dsShader_unbind(shader, commandBuffer));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	EXPECT_FALSE(dsShader_bind(shader, commandBuffer, material, globalValues, NULL));

	EXPECT_TRUE(dsShader_destroy(shader));
	EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
	dsSharedMaterialValues_destroy(globalValues);
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer1));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer2));
	dsMaterial_destroy(material);
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
}

TEST_F(ShaderTest, BindAndUpdateInstanceBuffer)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0},
		{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, dsMaterialBinding_Material, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, dsMaterialBinding_Material, 0},
		{"Transform", dsMaterialType_UniformBlock, 0, NULL, dsMaterialBinding_Instance, 0},
	};
	unsigned int elementCount = DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadResource(resourceManager, NULL,
		dsFileResourceType_Embedded, getRelativePath("test.mslb"), "test");
	ASSERT_TRUE(shaderModule);

	dsShader* shader = dsShader_createName(resourceManager, NULL, shaderModule, "Test",
		materialDesc);
	ASSERT_TRUE(shader);

	dsMaterial* material = dsMaterial_create(resourceManager, (dsAllocator*)&allocator,
		materialDesc);
	ASSERT_TRUE(material);

	dsGfxBuffer* buffer1 = dsGfxBuffer_create(resourceManager, (dsAllocator*)&allocator,
		(dsGfxBufferUsage)(dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_CopyTo),
		dsGfxMemory_Static, NULL, sizeof(float)*28);
	ASSERT_TRUE(buffer1);

	dsGfxBuffer* buffer2 = dsGfxBuffer_create(resourceManager, (dsAllocator*)&allocator,
		(dsGfxBufferUsage)(dsGfxBufferUsage_UniformBuffer | dsGfxBufferUsage_CopyTo),
		dsGfxMemory_Static, NULL, sizeof(float)*28);
	ASSERT_TRUE(buffer2);

	dsSharedMaterialValues* instanceValues = dsSharedMaterialValues_create(
		(dsAllocator*)&allocator, DS_DEFAULT_MAX_SHARED_MATERIAL_VALUES);
	ASSERT_TRUE(instanceValues);

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0));
	EXPECT_TRUE(dsShader_bind(shader, commandBuffer, material, NULL, NULL));

	EXPECT_FALSE(dsShader_updateInstanceValues(shader, commandBuffer, instanceValues));

	EXPECT_TRUE(dsSharedMaterialValues_setBufferName(instanceValues, "Transform", buffer2, 0,
		buffer2->size));
	EXPECT_FALSE(dsShader_updateInstanceValues(shader, commandBuffer, instanceValues));

	EXPECT_TRUE(dsSharedMaterialValues_setBufferName(instanceValues, "Transform", buffer1, 0,
		buffer1->size));
	EXPECT_TRUE(dsShader_updateInstanceValues(shader, commandBuffer, instanceValues));

	EXPECT_FALSE(dsShader_unbind(shader, NULL));
	EXPECT_FALSE(dsShader_unbind(NULL, commandBuffer));

	EXPECT_FALSE(dsRenderPass_end(renderPass, commandBuffer));
	EXPECT_TRUE(dsShader_unbind(shader, commandBuffer));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	EXPECT_FALSE(dsShader_bind(shader, commandBuffer, material, NULL, NULL));

	EXPECT_TRUE(dsShader_destroy(shader));
	EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
	dsSharedMaterialValues_destroy(instanceValues);
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer1));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer2));
	dsMaterial_destroy(material);
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
}

TEST_F(ShaderTest, BindAndUpdateCompute)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsShaderVariableElement transformElements[] =
	{
		{"modelViewProjection", dsMaterialType_Mat4, 0},
		{"normalMat", dsMaterialType_Mat3, 0}
	};
	unsigned int transformElementCount = DS_ARRAY_SIZE(transformElements);
	dsShaderVariableGroupDesc* transformDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, transformElements, transformElementCount);
	ASSERT_TRUE(transformDesc);

	dsShaderVariableElement groupElements[] =
	{
		{"testValue", dsMaterialType_Float, 0}
	};
	unsigned int groupElementCount = DS_ARRAY_SIZE(groupElements);
	dsShaderVariableGroupDesc* groupDesc = dsShaderVariableGroupDesc_create(resourceManager,
		NULL, groupElements, groupElementCount);
	ASSERT_TRUE(groupDesc);

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Global, 0},
		{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, dsMaterialBinding_Material, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, dsMaterialBinding_Material, 0},
		{"Transform", dsMaterialType_VariableGroup, 0, transformDesc, dsMaterialBinding_Instance,
			0},
	};
	unsigned int elementCount = DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadResource(resourceManager, NULL,
		dsFileResourceType_Embedded, getRelativePath("test.mslb"), "test");
	ASSERT_TRUE(shaderModule);

	dsShader* shader = dsShader_createName(resourceManager, NULL, shaderModule, "Test",
		materialDesc);
	ASSERT_TRUE(shader);

	dsMaterial* material = dsMaterial_create(resourceManager, (dsAllocator*)&allocator,
		materialDesc);
	ASSERT_TRUE(material);

	dsShaderVariableGroup* transformGroup = dsShaderVariableGroup_create(resourceManager, NULL,
		NULL, transformDesc);
	ASSERT_TRUE(transformGroup);

	dsShaderVariableGroup* group = dsShaderVariableGroup_create(resourceManager, NULL, NULL,
		groupDesc);
	ASSERT_TRUE(group);

	dsTextureInfo texInfo = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		dsTextureDim_2D, 16, 16, 0, DS_ALL_MIP_LEVELS, 1};
	dsTexture* texture1 = dsTexture_create(resourceManager, NULL,
		(dsTextureUsage)(dsTextureUsage_Texture | dsTextureUsage_CopyTo), dsGfxMemory_Static,
		&texInfo, NULL, 0);
	ASSERT_TRUE(texture1);

	dsTexture* texture2 = dsTexture_create(resourceManager, NULL,
		(dsTextureUsage)(dsTextureUsage_Image | dsTextureUsage_CopyTo), dsGfxMemory_Static,
		&texInfo, NULL, 0);
	ASSERT_TRUE(texture2);

	dsSharedMaterialValues* globalValues = dsSharedMaterialValues_create(
		(dsAllocator*)&allocator, DS_DEFAULT_MAX_SHARED_MATERIAL_VALUES);
	ASSERT_TRUE(globalValues);

	dsSharedMaterialValues* instanceValues = dsSharedMaterialValues_create(
		(dsAllocator*)&allocator, DS_DEFAULT_MAX_SHARED_MATERIAL_VALUES);
	ASSERT_TRUE(instanceValues);

	EXPECT_TRUE(dsSharedMaterialValues_setTextureName(globalValues, "diffuseTexture",
		texture1));
	EXPECT_TRUE(dsSharedMaterialValues_setVariableGroupName(instanceValues, "Transform",
		transformGroup));

	EXPECT_FALSE(dsShader_bindCompute(shader, NULL, material, globalValues));
	EXPECT_FALSE(dsShader_bindCompute(NULL, commandBuffer, material, globalValues));
	EXPECT_FALSE(dsShader_bindCompute(shader, commandBuffer, NULL, globalValues));

	EXPECT_TRUE(dsSharedMaterialValues_removeValueName(globalValues, "diffuseTexture"));
	EXPECT_FALSE(dsShader_bindCompute(shader, commandBuffer, material, globalValues));

	EXPECT_TRUE(dsSharedMaterialValues_setTextureName(globalValues, "diffuseTexture",
		texture2));
	EXPECT_FALSE(dsShader_bindCompute(shader, commandBuffer, material, globalValues));

	EXPECT_TRUE(dsSharedMaterialValues_setTextureName(globalValues, "diffuseTexture",
		texture1));
	EXPECT_TRUE(dsShader_bindCompute(shader, commandBuffer, material, globalValues));

	EXPECT_FALSE(dsShader_updateComputeInstanceValues(shader, commandBuffer, NULL));

	EXPECT_TRUE(dsSharedMaterialValues_removeValueName(instanceValues, "Transform"));
	EXPECT_FALSE(dsShader_updateComputeInstanceValues(shader, commandBuffer, instanceValues));

	EXPECT_TRUE(dsSharedMaterialValues_setVariableGroupName(instanceValues, "Transform", group));
	EXPECT_FALSE(dsShader_updateComputeInstanceValues(shader, commandBuffer, instanceValues));

	EXPECT_TRUE(dsSharedMaterialValues_setVariableGroupName(instanceValues, "Transform",
		transformGroup));
	EXPECT_TRUE(dsShader_updateComputeInstanceValues(shader, commandBuffer, instanceValues));

	EXPECT_FALSE(dsShader_unbindCompute(shader, NULL));
	EXPECT_FALSE(dsShader_unbindCompute(NULL, commandBuffer));

	EXPECT_FALSE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0));
	EXPECT_TRUE(dsShader_unbindCompute(shader, commandBuffer));

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0));
	EXPECT_FALSE(dsShader_bindCompute(shader, commandBuffer, material, globalValues));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	EXPECT_TRUE(dsShader_destroy(shader));
	EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
	dsSharedMaterialValues_destroy(globalValues);
	dsSharedMaterialValues_destroy(instanceValues);
	EXPECT_TRUE(dsShaderVariableGroup_destroy(group));
	EXPECT_TRUE(dsShaderVariableGroup_destroy(transformGroup));
	EXPECT_TRUE(dsTexture_destroy(texture1));
	EXPECT_TRUE(dsTexture_destroy(texture2));
	dsMaterial_destroy(material);
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(groupDesc));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(transformDesc));
}

TEST_F(ShaderTest, BindAndUpdateComputeBuffer)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0},
		{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, dsMaterialBinding_Material, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, dsMaterialBinding_Material, 0},
		{"Transform", dsMaterialType_UniformBlock, 0, NULL, dsMaterialBinding_Global, 0},
	};
	unsigned int elementCount = DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadResource(resourceManager, NULL,
		dsFileResourceType_Embedded, getRelativePath("test.mslb"), "test");
	ASSERT_TRUE(shaderModule);

	dsShader* shader = dsShader_createName(resourceManager, NULL, shaderModule, "Test",
		materialDesc);
	ASSERT_TRUE(shader);

	dsMaterial* material = dsMaterial_create(resourceManager, (dsAllocator*)&allocator,
		materialDesc);
	ASSERT_TRUE(material);

	dsGfxBuffer* buffer1 = dsGfxBuffer_create(resourceManager, (dsAllocator*)&allocator,
		(dsGfxBufferUsage)(dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_CopyTo),
		dsGfxMemory_Static, NULL, sizeof(float)*28);
	ASSERT_TRUE(buffer1);

	dsGfxBuffer* buffer2 = dsGfxBuffer_create(resourceManager, (dsAllocator*)&allocator,
		(dsGfxBufferUsage)(dsGfxBufferUsage_UniformBuffer | dsGfxBufferUsage_CopyTo),
		dsGfxMemory_Static, NULL, sizeof(float)*28);
	ASSERT_TRUE(buffer2);

	dsSharedMaterialValues* globalValues = dsSharedMaterialValues_create(
		(dsAllocator*)&allocator, DS_DEFAULT_MAX_SHARED_MATERIAL_VALUES);
	ASSERT_TRUE(globalValues);

	EXPECT_FALSE(dsShader_bindCompute(shader, commandBuffer, material, globalValues));

	EXPECT_TRUE(dsSharedMaterialValues_setBufferName(globalValues, "Transform", buffer2, 0,
		buffer2->size));
	EXPECT_FALSE(dsShader_bindCompute(shader, commandBuffer, material, globalValues));

	EXPECT_TRUE(dsSharedMaterialValues_setBufferName(globalValues, "Transform", buffer1, 0,
		buffer1->size));
	EXPECT_TRUE(dsShader_bindCompute(shader, commandBuffer, material, globalValues));

	EXPECT_FALSE(dsShader_unbindCompute(shader, NULL));
	EXPECT_FALSE(dsShader_unbindCompute(NULL, commandBuffer));

	EXPECT_FALSE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0));
	EXPECT_TRUE(dsShader_unbindCompute(shader, commandBuffer));

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0));
	EXPECT_FALSE(dsShader_unbindCompute(shader, commandBuffer));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	EXPECT_TRUE(dsShader_destroy(shader));
	EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
	dsSharedMaterialValues_destroy(globalValues);
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer1));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer2));
	dsMaterial_destroy(material);
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
}

TEST_F(ShaderTest, BindAndUpdateComputeInstanceBuffer)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsMaterialElement elements[] =
	{
		{"diffuseTexture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0},
		{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, dsMaterialBinding_Material, 0},
		{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, dsMaterialBinding_Material, 0},
		{"Transform", dsMaterialType_UniformBlock, 0, NULL, dsMaterialBinding_Instance, 0},
	};
	unsigned int elementCount = DS_ARRAY_SIZE(elements);
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	dsShaderModule* shaderModule = dsShaderModule_loadResource(resourceManager, NULL,
		dsFileResourceType_Embedded, getRelativePath("test.mslb"), "test");
	ASSERT_TRUE(shaderModule);

	dsShader* shader = dsShader_createName(resourceManager, NULL, shaderModule, "Test",
		materialDesc);
	ASSERT_TRUE(shader);

	dsMaterial* material = dsMaterial_create(resourceManager, (dsAllocator*)&allocator,
		materialDesc);
	ASSERT_TRUE(material);

	dsGfxBuffer* buffer1 = dsGfxBuffer_create(resourceManager, (dsAllocator*)&allocator,
		(dsGfxBufferUsage)(dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_CopyTo),
		dsGfxMemory_Static, NULL, sizeof(float)*28);
	ASSERT_TRUE(buffer1);

	dsGfxBuffer* buffer2 = dsGfxBuffer_create(resourceManager, (dsAllocator*)&allocator,
		(dsGfxBufferUsage)(dsGfxBufferUsage_UniformBuffer | dsGfxBufferUsage_CopyTo),
		dsGfxMemory_Static, NULL, sizeof(float)*28);
	ASSERT_TRUE(buffer2);

	dsSharedMaterialValues* instanceValues = dsSharedMaterialValues_create(
		(dsAllocator*)&allocator, DS_DEFAULT_MAX_SHARED_MATERIAL_VALUES);
	ASSERT_TRUE(instanceValues);

	EXPECT_TRUE(dsShader_bindCompute(shader, commandBuffer, material, NULL));

	EXPECT_FALSE(dsShader_updateInstanceValues(shader, commandBuffer, instanceValues));

	EXPECT_TRUE(dsSharedMaterialValues_setBufferName(instanceValues, "Transform", buffer2, 0,
		buffer2->size));
	EXPECT_FALSE(dsShader_updateComputeInstanceValues(shader, commandBuffer, instanceValues));

	EXPECT_TRUE(dsSharedMaterialValues_setBufferName(instanceValues, "Transform", buffer1, 0,
		buffer1->size));
	EXPECT_TRUE(dsShader_updateComputeInstanceValues(shader, commandBuffer, instanceValues));

	EXPECT_FALSE(dsShader_unbindCompute(shader, NULL));
	EXPECT_FALSE(dsShader_unbindCompute(NULL, commandBuffer));

	EXPECT_FALSE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0));
	EXPECT_TRUE(dsShader_unbindCompute(shader, commandBuffer));

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0));
	EXPECT_FALSE(dsShader_unbindCompute(shader, commandBuffer));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	EXPECT_TRUE(dsShader_destroy(shader));
	EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
	dsSharedMaterialValues_destroy(instanceValues);
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer1));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer2));
	dsMaterial_destroy(material);
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
}

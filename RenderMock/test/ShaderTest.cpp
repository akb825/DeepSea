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
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
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

	EXPECT_FALSE(dsShader_createName(resourceManager, NULL, shaderModule, "Test", materialDesc));

	EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(transformDesc));
}

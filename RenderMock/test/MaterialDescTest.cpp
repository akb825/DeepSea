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
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <gtest/gtest.h>

class MaterialDescTest : public FixtureBase
{
};

TEST_F(MaterialDescTest, Create)
{
	dsMaterialElement elements[] =
	{
		{"singleVec4", dsMaterialType_Vec4, 0, NULL, false, 0},
		{"matrixArray", dsMaterialType_Mat4, 3, NULL, false, 0},
		{"integer", dsMaterialType_Int, 0, NULL, false, 0},
		{"texture", dsMaterialType_Texture, 0, NULL, true, 0}
	};
	unsigned int elementCount = (unsigned int)DS_ARRAY_SIZE(elements);

	EXPECT_FALSE(dsMaterialDesc_create(NULL, NULL, elements, elementCount));
	EXPECT_FALSE(dsMaterialDesc_create(resourceManager, NULL, NULL, elementCount));

	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL,
		elements, elementCount);
	ASSERT_TRUE(materialDesc);
	EXPECT_EQ(1U, resourceManager->materialDescCount);

	EXPECT_EQ(0U, dsMaterialDesc_findElement(materialDesc, "singleVec4"));
	EXPECT_EQ(1U, dsMaterialDesc_findElement(materialDesc, "matrixArray"));
	EXPECT_EQ(2U, dsMaterialDesc_findElement(materialDesc, "integer"));
	EXPECT_EQ(DS_UNKNOWN, dsMaterialDesc_findElement(materialDesc, "asdf"));

	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	EXPECT_EQ(0U, resourceManager->materialDescCount);
}

TEST_F(MaterialDescTest, CreateDuplicateName)
{
	dsMaterialElement elements[] =
	{
		{"integer", dsMaterialType_Int, 0, NULL, false, 0},
		{"singleVec4", dsMaterialType_Vec4, 0, NULL, false, 0},
		{"matrixArray", dsMaterialType_Mat4, 3, NULL, false, 0},
		{"integer", dsMaterialType_Int, 3, NULL, false, 0},
		{"texture", dsMaterialType_Texture, 0, NULL, true, 0}
	};
	unsigned int elementCount = (unsigned int)DS_ARRAY_SIZE(elements);

	EXPECT_FALSE(dsMaterialDesc_create(resourceManager, NULL, elements, elementCount));
}

TEST_F(MaterialDescTest, CreateVolatilePrimitive)
{
	dsMaterialElement elements[] =
	{
		{"singleVec4", dsMaterialType_Vec4, 0, NULL, true, 0},
		{"matrixArray", dsMaterialType_Mat4, 3, NULL, false, 0},
		{"integer", dsMaterialType_Int, 0, NULL, false, 0},
		{"texture", dsMaterialType_Texture, 0, NULL, true, 0}
	};
	unsigned int elementCount = (unsigned int)DS_ARRAY_SIZE(elements);

	EXPECT_FALSE(dsMaterialDesc_create(resourceManager, NULL, elements, elementCount));
}

TEST_F(MaterialDescTest, CreateOpaqueArray)
{
	dsMaterialElement elements[] =
	{
		{"singleVec4", dsMaterialType_Vec4, 0, NULL, false, 0},
		{"matrixArray", dsMaterialType_Mat4, 3, NULL, false, 0},
		{"integer", dsMaterialType_Int, 0, NULL, false, 0},
		{"texture", dsMaterialType_Texture, 2, NULL, true, 0}
	};
	unsigned int elementCount = (unsigned int)DS_ARRAY_SIZE(elements);

	EXPECT_FALSE(dsMaterialDesc_create(resourceManager, NULL, elements, elementCount));
}

TEST_F(MaterialDescTest, CreateShaderVariableGroup)
{
	dsMaterialElement elements[] =
	{
		{"singleVec4", dsMaterialType_Vec4, 0, NULL, false, 0},
		{"matrixArray", dsMaterialType_Mat4, 3, NULL, false, 0},
		{"integer", dsMaterialType_Int, 0, NULL, false, 0},
		{"texture", dsMaterialType_Texture, 0, NULL, true, 0},
		{"variableGroup", dsMaterialType_VariableGroup, 0, NULL, false, 0}
	};
	unsigned int elementCount = (unsigned int)DS_ARRAY_SIZE(elements);

	EXPECT_FALSE(dsMaterialDesc_create(resourceManager, NULL, elements, elementCount));

	dsShaderVariableElement groupElements[] =
	{
		{"float", dsMaterialType_Float, 0},
		{"vec3Array", dsMaterialType_Vec3, 2}
	};
	unsigned int groupElementCount = DS_ARRAY_SIZE(groupElements);
	dsShaderVariableGroupDesc* groupDesc = dsShaderVariableGroupDesc_create(resourceManager, NULL,
		groupElements, groupElementCount);
	ASSERT_TRUE(groupDesc);

	elements[4].shaderVariableGroupDesc = groupDesc;
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(groupDesc));
}

TEST_F(MaterialDescTest, CreateUniformBlock)
{
	dsMaterialElement elements[] =
	{
		{"singleVec4", dsMaterialType_Vec4, 0, NULL, false, 0},
		{"matrixArray", dsMaterialType_Mat4, 3, NULL, false, 0},
		{"integer", dsMaterialType_Int, 0, NULL, false, 0},
		{"texture", dsMaterialType_Texture, 0, NULL, true, 0},
		{"uniformBlock", dsMaterialType_UniformBlock, 0, NULL, false, 0}
	};
	unsigned int elementCount = (unsigned int)DS_ARRAY_SIZE(elements);

	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));

	resourceManager->supportedBuffers =
		(dsGfxBufferUsage)(resourceManager->supportedBuffers & ~dsGfxBufferUsage_UniformBlock);
	EXPECT_FALSE(dsMaterialDesc_create(resourceManager, NULL, elements, elementCount));
}

TEST_F(MaterialDescTest, CreateUniformBuffer)
{
	dsMaterialElement elements[] =
	{
		{"singleVec4", dsMaterialType_Vec4, 0, NULL, false, 0},
		{"matrixArray", dsMaterialType_Mat4, 3, NULL, false, 0},
		{"integer", dsMaterialType_Int, 0, NULL, false, 0},
		{"texture", dsMaterialType_Texture, 0, NULL, true, 0},
		{"uniformBuffer", dsMaterialType_UniformBuffer, 0, NULL, false, 0}
	};
	unsigned int elementCount = (unsigned int)DS_ARRAY_SIZE(elements);

	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements,
		elementCount);
	ASSERT_TRUE(materialDesc);

	EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));

	resourceManager->supportedBuffers =
		(dsGfxBufferUsage)(resourceManager->supportedBuffers & ~dsGfxBufferUsage_UniformBuffer);
	EXPECT_FALSE(dsMaterialDesc_create(resourceManager, NULL, elements, elementCount));
}

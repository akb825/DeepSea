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
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <gtest/gtest.h>

class ShaderVariableGroupDescTest : public FixtureBase
{
};

TEST_F(ShaderVariableGroupDescTest, Create)
{
	dsShaderVariableElement elements[] =
	{
		{"singleVec4", dsMaterialType_Vec4, 0},
		{"matrixArray", dsMaterialType_Mat4, 3},
		{"integer", dsMaterialType_Int, 0}
	};
	unsigned int elementCount = DS_ARRAY_SIZE(elements);

	EXPECT_FALSE(dsShaderVariableGroupDesc_create(NULL, NULL, elements, elementCount));
	EXPECT_FALSE(dsShaderVariableGroupDesc_create(resourceManager, NULL, NULL, elementCount));
	EXPECT_FALSE(dsShaderVariableGroupDesc_create(resourceManager, NULL, elements, 0));

	dsShaderVariableGroupDesc* groupDesc = dsShaderVariableGroupDesc_create(resourceManager, NULL,
		elements, elementCount);
	ASSERT_TRUE(groupDesc);
	EXPECT_EQ(1U, resourceManager->shaderVariableGroupDescCount);

	EXPECT_EQ(0U, dsShaderVariableGroupDesc_findElement(groupDesc, "singleVec4"));
	EXPECT_EQ(1U, dsShaderVariableGroupDesc_findElement(groupDesc, "matrixArray"));
	EXPECT_EQ(2U, dsShaderVariableGroupDesc_findElement(groupDesc, "integer"));
	EXPECT_EQ(DS_MATERIAL_UNKNOWN, dsShaderVariableGroupDesc_findElement(groupDesc, "asdf"));

	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(groupDesc));
	EXPECT_EQ(0U, resourceManager->shaderVariableGroupDescCount);
}

TEST_F(ShaderVariableGroupDescTest, CreateDuplicateName)
{
	dsShaderVariableElement elements[] =
	{
		{"integer", dsMaterialType_Int, 0},
		{"singleVec4", dsMaterialType_Vec4, 0},
		{"matrixArray", dsMaterialType_Mat4, 3},
		{"integer", dsMaterialType_Int, 3}
	};
	unsigned int elementCount = DS_ARRAY_SIZE(elements);

	EXPECT_FALSE(dsShaderVariableGroupDesc_create(resourceManager, NULL, elements, elementCount));
}

TEST_F(ShaderVariableGroupDescTest, CreateOpaqueType)
{
	dsShaderVariableElement elements[] =
	{
		{"singleVec4", dsMaterialType_Vec4, 0},
		{"matrixArray", dsMaterialType_Mat4, 3},
		{"integer", dsMaterialType_Int, 0},
		{"texture", dsMaterialType_Texture, 0}
	};
	unsigned int elementCount = DS_ARRAY_SIZE(elements);

	EXPECT_FALSE(dsShaderVariableGroupDesc_create(resourceManager, NULL, elements, elementCount));
}

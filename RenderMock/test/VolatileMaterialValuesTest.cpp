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
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/VolatileMaterialValues.h>

class VolatileMaterialValuesTest : public FixtureBase
{
};

TEST_F(VolatileMaterialValuesTest, Create)
{
	EXPECT_FALSE(dsVolatileMaterialValues_create(NULL, DS_DEFAULT_MAX_VOLATILE_MATERIAL_VALUES));
	EXPECT_FALSE(dsVolatileMaterialValues_create((dsAllocator*)&allocator, 0));
	dsVolatileMaterialValues* values = dsVolatileMaterialValues_create((dsAllocator*)&allocator,
		DS_DEFAULT_MAX_VOLATILE_MATERIAL_VALUES);
	EXPECT_TRUE(values);

	EXPECT_EQ(DS_DEFAULT_MAX_VOLATILE_MATERIAL_VALUES,
		dsVolatileMaterialValues_getMaxValueCount(values));
	EXPECT_EQ(0U, dsVolatileMaterialValues_getValueCount(values));

	dsVolatileMaterialValues_destroy(values);
}

TEST_F(VolatileMaterialValuesTest, Textures)
{
	dsVolatileMaterialValues* values = dsVolatileMaterialValues_create((dsAllocator*)&allocator,
		DS_DEFAULT_MAX_VOLATILE_MATERIAL_VALUES);
	ASSERT_TRUE(values);

	dsTexture* texture1 = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo, dsGfxMemory_Static,
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), dsTextureDim_2D, 16, 16, 0,
		DS_ALL_MIP_LEVELS, NULL, 0);
	ASSERT_TRUE(texture1);

	dsTexture* texture2 = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo, dsGfxMemory_Static,
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), dsTextureDim_2D, 16, 16, 0,
		DS_ALL_MIP_LEVELS, NULL, 0);
	ASSERT_TRUE(texture2);

	EXPECT_TRUE(dsVolatileMaterialValues_setTextureName(values, "test1", texture1));
	EXPECT_TRUE(dsVolatileMaterialValues_setTextureId(values, dsHashString("test2"), texture2));

	EXPECT_EQ(2U, dsVolatileMaterialValues_getValueCount(values));

	EXPECT_EQ(texture1, dsVolatileMaterialValues_getTextureId(values, dsHashString("test1")));
	EXPECT_EQ(texture2, dsVolatileMaterialValues_getTextureName(values, "test2"));

	EXPECT_FALSE(dsVolatileMaterialValues_getTextureName(values, "asdf"));
	EXPECT_FALSE(dsVolatileMaterialValues_getTextureId(values, dsHashString("asdf")));
	EXPECT_FALSE(dsVolatileMaterialValues_getVariableGroupName(values, "test1"));
	EXPECT_FALSE(dsVolatileMaterialValues_getBufferName(values, "test1"));

	EXPECT_TRUE(dsVolatileMaterialValues_setTextureName(values, "test1", texture2));
	EXPECT_TRUE(dsVolatileMaterialValues_setTextureName(values, "test2", texture1));
	EXPECT_EQ(texture2, dsVolatileMaterialValues_getTextureName(values, "test1"));
	EXPECT_EQ(texture1, dsVolatileMaterialValues_getTextureName(values, "test2"));

	EXPECT_TRUE(dsVolatileMaterialValues_removeValueName(values, "test1"));
	EXPECT_FALSE(dsVolatileMaterialValues_removeValueName(values, "test1"));
	EXPECT_TRUE(dsVolatileMaterialValues_removeValueId(values, dsHashString("test2")));
	EXPECT_FALSE(dsVolatileMaterialValues_removeValueId(values, dsHashString("test2")));

	EXPECT_EQ(0U, dsVolatileMaterialValues_getValueCount(values));
	EXPECT_FALSE(dsVolatileMaterialValues_getTextureName(values, "test1"));
	EXPECT_FALSE(dsVolatileMaterialValues_getTextureName(values, "test2"));

	dsVolatileMaterialValues_destroy(values);
	EXPECT_TRUE(dsTexture_destroy(texture1));
	EXPECT_TRUE(dsTexture_destroy(texture2));
}

TEST_F(VolatileMaterialValuesTest, VariableGroups)
{
	dsVolatileMaterialValues* values = dsVolatileMaterialValues_create((dsAllocator*)&allocator,
		DS_DEFAULT_MAX_VOLATILE_MATERIAL_VALUES);
	ASSERT_TRUE(values);

	dsShaderVariableElement elements[] =
	{
		{"test", dsMaterialType_Vec4, 0}
	};

	dsShaderVariableGroupDesc* desc = dsShaderVariableGroupDesc_create(resourceManager, NULL,
		elements, (uint32_t)DS_ARRAY_SIZE(elements));
	ASSERT_TRUE(desc);

	dsShaderVariableGroup* variableGroup1 = dsShaderVariableGroup_create(resourceManager, NULL, NULL,
		desc, dsShaderCommitType_Immediate);
	ASSERT_TRUE(variableGroup1);

	dsShaderVariableGroup* variableGroup2 = dsShaderVariableGroup_create(resourceManager, NULL, NULL,
	desc, dsShaderCommitType_Immediate);
	ASSERT_TRUE(variableGroup2);

	EXPECT_TRUE(dsVolatileMaterialValues_setVariableGroupName(values, "test1", variableGroup1));
	EXPECT_TRUE(dsVolatileMaterialValues_setVariableGroupId(values, dsHashString("test2"),
		variableGroup2));

	EXPECT_EQ(2U, dsVolatileMaterialValues_getValueCount(values));

	EXPECT_EQ(variableGroup1, dsVolatileMaterialValues_getVariableGroupId(values, dsHashString("test1")));
	EXPECT_EQ(variableGroup2, dsVolatileMaterialValues_getVariableGroupName(values, "test2"));

	EXPECT_FALSE(dsVolatileMaterialValues_getVariableGroupName(values, "asdf"));
	EXPECT_FALSE(dsVolatileMaterialValues_getVariableGroupId(values, dsHashString("asdf")));
	EXPECT_FALSE(dsVolatileMaterialValues_getTextureName(values, "test1"));
	EXPECT_FALSE(dsVolatileMaterialValues_getBufferName(values, "test1"));

	EXPECT_TRUE(dsVolatileMaterialValues_setVariableGroupName(values, "test1", variableGroup2));
	EXPECT_TRUE(dsVolatileMaterialValues_setVariableGroupName(values, "test2", variableGroup1));
	EXPECT_EQ(variableGroup2, dsVolatileMaterialValues_getVariableGroupName(values, "test1"));
	EXPECT_EQ(variableGroup1, dsVolatileMaterialValues_getVariableGroupName(values, "test2"));

	EXPECT_TRUE(dsVolatileMaterialValues_removeValueName(values, "test1"));
	EXPECT_FALSE(dsVolatileMaterialValues_removeValueName(values, "test1"));
	EXPECT_TRUE(dsVolatileMaterialValues_removeValueId(values, dsHashString("test2")));
	EXPECT_FALSE(dsVolatileMaterialValues_removeValueId(values, dsHashString("test2")));

	EXPECT_EQ(0U, dsVolatileMaterialValues_getValueCount(values));
	EXPECT_FALSE(dsVolatileMaterialValues_getVariableGroupName(values, "test1"));
	EXPECT_FALSE(dsVolatileMaterialValues_getVariableGroupName(values, "test2"));

	dsVolatileMaterialValues_destroy(values);
	EXPECT_TRUE(dsShaderVariableGroup_destroy(variableGroup1));
	EXPECT_TRUE(dsShaderVariableGroup_destroy(variableGroup2));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(desc));
}

TEST_F(VolatileMaterialValuesTest, Buffers)
{
	dsVolatileMaterialValues* values = dsVolatileMaterialValues_create((dsAllocator*)&allocator,
		DS_DEFAULT_MAX_VOLATILE_MATERIAL_VALUES);
	ASSERT_TRUE(values);

	dsGfxBuffer* buffer1 = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Vertex | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static, NULL, 128);
	ASSERT_TRUE(buffer1);

	dsGfxBuffer* buffer2 = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Vertex | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static, NULL, 128);
	ASSERT_TRUE(buffer2);

	EXPECT_TRUE(dsVolatileMaterialValues_setBufferName(values, "test1", buffer1));
	EXPECT_TRUE(dsVolatileMaterialValues_setBufferId(values, dsHashString("test2"), buffer2));

	EXPECT_EQ(2U, dsVolatileMaterialValues_getValueCount(values));

	EXPECT_EQ(buffer1, dsVolatileMaterialValues_getBufferId(values, dsHashString("test1")));
	EXPECT_EQ(buffer2, dsVolatileMaterialValues_getBufferName(values, "test2"));

	EXPECT_FALSE(dsVolatileMaterialValues_getBufferName(values, "asdf"));
	EXPECT_FALSE(dsVolatileMaterialValues_getBufferId(values, dsHashString("asdf")));
	EXPECT_FALSE(dsVolatileMaterialValues_getTextureName(values, "test1"));
	EXPECT_FALSE(dsVolatileMaterialValues_getVariableGroupName(values, "test1"));

	EXPECT_TRUE(dsVolatileMaterialValues_setBufferName(values, "test1", buffer2));
	EXPECT_TRUE(dsVolatileMaterialValues_setBufferName(values, "test2", buffer1));
	EXPECT_EQ(buffer2, dsVolatileMaterialValues_getBufferName(values, "test1"));
	EXPECT_EQ(buffer1, dsVolatileMaterialValues_getBufferName(values, "test2"));

	EXPECT_TRUE(dsVolatileMaterialValues_removeValueName(values, "test1"));
	EXPECT_FALSE(dsVolatileMaterialValues_removeValueName(values, "test1"));
	EXPECT_TRUE(dsVolatileMaterialValues_removeValueId(values, dsHashString("test2")));
	EXPECT_FALSE(dsVolatileMaterialValues_removeValueId(values, dsHashString("test2")));

	EXPECT_EQ(0U, dsVolatileMaterialValues_getValueCount(values));
	EXPECT_FALSE(dsVolatileMaterialValues_getBufferName(values, "test1"));
	EXPECT_FALSE(dsVolatileMaterialValues_getBufferName(values, "test2"));

	dsVolatileMaterialValues_destroy(values);
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer1));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer2));
}

TEST_F(VolatileMaterialValuesTest, BuffersMixedTypes)
{
	dsVolatileMaterialValues* values = dsVolatileMaterialValues_create((dsAllocator*)&allocator,
		DS_DEFAULT_MAX_VOLATILE_MATERIAL_VALUES);
	ASSERT_TRUE(values);

	dsTexture* texture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo, dsGfxMemory_Static,
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), dsTextureDim_2D, 16, 16, 0,
		DS_ALL_MIP_LEVELS, NULL, 0);
	ASSERT_TRUE(texture);

	dsShaderVariableElement elements[] =
	{
		{"test", dsMaterialType_Vec4, 0}
	};

	dsShaderVariableGroupDesc* desc = dsShaderVariableGroupDesc_create(resourceManager, NULL,
		elements, (uint32_t)DS_ARRAY_SIZE(elements));
	ASSERT_TRUE(desc);

	dsShaderVariableGroup* variableGroup = dsShaderVariableGroup_create(resourceManager, NULL, NULL,
		desc, dsShaderCommitType_Immediate);
	ASSERT_TRUE(variableGroup);

	dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Vertex | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static, NULL, 128);
	ASSERT_TRUE(buffer);

	EXPECT_TRUE(dsVolatileMaterialValues_setTextureName(values, "texture", texture));
	EXPECT_TRUE(dsVolatileMaterialValues_setVariableGroupName(values, "variable group",
		variableGroup));
	EXPECT_TRUE(dsVolatileMaterialValues_setBufferName(values, "buffer", buffer));
	EXPECT_EQ(3U, dsVolatileMaterialValues_getValueCount(values));

	EXPECT_EQ(texture, dsVolatileMaterialValues_getTextureName(values, "texture"));
	EXPECT_EQ(variableGroup, dsVolatileMaterialValues_getVariableGroupName(values,
		"variable group"));
	EXPECT_EQ(buffer, dsVolatileMaterialValues_getBufferName(values, "buffer"));

	EXPECT_FALSE(dsVolatileMaterialValues_setTextureName(values, "buffer", texture));
	EXPECT_FALSE(dsVolatileMaterialValues_setVariableGroupName(values, "buffer", variableGroup));
	EXPECT_FALSE(dsVolatileMaterialValues_setBufferName(values, "texture", buffer));

	dsVolatileMaterialValues_destroy(values);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_TRUE(dsShaderVariableGroup_destroy(variableGroup));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(desc));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));
}

TEST_F(VolatileMaterialValuesTest, Limit)
{
	dsVolatileMaterialValues* values = dsVolatileMaterialValues_create((dsAllocator*)&allocator, 2);
	ASSERT_TRUE(values);

	dsTexture* texture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo, dsGfxMemory_Static,
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm), dsTextureDim_2D, 16, 16, 0,
		DS_ALL_MIP_LEVELS, NULL, 0);
	ASSERT_TRUE(texture);

	dsShaderVariableElement elements[] =
	{
		{"test", dsMaterialType_Vec4, 0}
	};

	dsShaderVariableGroupDesc* desc = dsShaderVariableGroupDesc_create(resourceManager, NULL,
		elements, (uint32_t)DS_ARRAY_SIZE(elements));
	ASSERT_TRUE(desc);

	dsShaderVariableGroup* variableGroup = dsShaderVariableGroup_create(resourceManager, NULL, NULL,
		desc, dsShaderCommitType_Immediate);
	ASSERT_TRUE(variableGroup);

	dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Vertex | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static, NULL, 128);
	ASSERT_TRUE(buffer);

	EXPECT_TRUE(dsVolatileMaterialValues_setTextureName(values, "texture", texture));
	EXPECT_TRUE(dsVolatileMaterialValues_setVariableGroupName(values, "variable group",
		variableGroup));
	EXPECT_FALSE(dsVolatileMaterialValues_setBufferName(values, "buffer", buffer));
	EXPECT_EQ(2U, dsVolatileMaterialValues_getValueCount(values));

	EXPECT_TRUE(dsVolatileMaterialValues_removeValueName(values, "texture"));
	EXPECT_TRUE(dsVolatileMaterialValues_setBufferName(values, "buffer", buffer));
	EXPECT_FALSE(dsVolatileMaterialValues_setTextureName(values, "texture", texture));
	EXPECT_EQ(2U, dsVolatileMaterialValues_getValueCount(values));

	EXPECT_TRUE(dsVolatileMaterialValues_removeValueName(values, "variable group"));
	EXPECT_TRUE(dsVolatileMaterialValues_setTextureName(values, "texture", texture));
	EXPECT_FALSE(dsVolatileMaterialValues_setVariableGroupName(values, "variable group",
		variableGroup));
	EXPECT_EQ(2U, dsVolatileMaterialValues_getValueCount(values));

	EXPECT_TRUE(dsVolatileMaterialValues_removeValueName(values, "buffer"));
	EXPECT_TRUE(dsVolatileMaterialValues_setVariableGroupName(values, "variable group",
		variableGroup));
	EXPECT_FALSE(dsVolatileMaterialValues_setBufferName(values, "buffer", buffer));
	EXPECT_EQ(2U, dsVolatileMaterialValues_getValueCount(values));

	EXPECT_TRUE(dsVolatileMaterialValues_setTextureName(values, "texture", NULL));
	EXPECT_EQ(2U, dsVolatileMaterialValues_getValueCount(values));
	EXPECT_EQ(NULL, dsVolatileMaterialValues_getTextureName(values, "texture"));
	EXPECT_TRUE(dsVolatileMaterialValues_setTextureName(values, "texture", texture));
	EXPECT_EQ(2U, dsVolatileMaterialValues_getValueCount(values));
	EXPECT_EQ(texture, dsVolatileMaterialValues_getTextureName(values, "texture"));

	dsVolatileMaterialValues_destroy(values);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_TRUE(dsShaderVariableGroup_destroy(variableGroup));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(desc));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));
}

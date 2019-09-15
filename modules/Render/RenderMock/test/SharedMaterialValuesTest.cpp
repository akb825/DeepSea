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
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>

class SharedMaterialValuesTest : public FixtureBase
{
};

TEST_F(SharedMaterialValuesTest, Create)
{
	EXPECT_FALSE(dsSharedMaterialValues_create(NULL, DS_DEFAULT_MAX_SHARED_MATERIAL_VALUES));
	EXPECT_FALSE(dsSharedMaterialValues_create((dsAllocator*)&allocator, 0));
	dsSharedMaterialValues* values = dsSharedMaterialValues_create((dsAllocator*)&allocator,
		DS_DEFAULT_MAX_SHARED_MATERIAL_VALUES);
	EXPECT_TRUE(values);

	EXPECT_EQ(DS_DEFAULT_MAX_SHARED_MATERIAL_VALUES,
		dsSharedMaterialValues_getMaxValueCount(values));
	EXPECT_EQ(0U, dsSharedMaterialValues_getValueCount(values));

	dsSharedMaterialValues_destroy(values);
}

TEST_F(SharedMaterialValuesTest, Textures)
{
	dsSharedMaterialValues* values = dsSharedMaterialValues_create((dsAllocator*)&allocator,
		DS_DEFAULT_MAX_SHARED_MATERIAL_VALUES);
	ASSERT_TRUE(values);

	dsTextureInfo texInfo = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		dsTextureDim_2D, 16, 16, 0, DS_ALL_MIP_LEVELS, 1};
	dsTexture* texture1 = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo, dsGfxMemory_Static, &texInfo, NULL, 0);
	ASSERT_TRUE(texture1);

	dsTexture* texture2 = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo, dsGfxMemory_Static, &texInfo, NULL, 0);
	ASSERT_TRUE(texture2);

	EXPECT_TRUE(dsSharedMaterialValues_setTextureName(values, "test1", texture1));
	EXPECT_TRUE(dsSharedMaterialValues_setTextureID(values, dsHashString("test2"), texture2));

	EXPECT_EQ(2U, dsSharedMaterialValues_getValueCount(values));

	EXPECT_EQ(texture1, dsSharedMaterialValues_getTextureID(values, dsHashString("test1")));
	EXPECT_EQ(texture2, dsSharedMaterialValues_getTextureName(values, "test2"));

	EXPECT_FALSE(dsSharedMaterialValues_getTextureName(values, "asdf"));
	EXPECT_FALSE(dsSharedMaterialValues_getTextureID(values, dsHashString("asdf")));
	EXPECT_FALSE(dsSharedMaterialValues_getVariableGroupName(values, "test1"));
	EXPECT_FALSE(dsSharedMaterialValues_getBufferName(NULL, NULL, values, "test1"));

	EXPECT_TRUE(dsSharedMaterialValues_setTextureName(values, "test1", texture2));
	EXPECT_TRUE(dsSharedMaterialValues_setTextureName(values, "test2", texture1));
	EXPECT_EQ(texture2, dsSharedMaterialValues_getTextureName(values, "test1"));
	EXPECT_EQ(texture1, dsSharedMaterialValues_getTextureName(values, "test2"));

	EXPECT_TRUE(dsSharedMaterialValues_removeValueName(values, "test1"));
	EXPECT_FALSE(dsSharedMaterialValues_removeValueName(values, "test1"));
	EXPECT_TRUE(dsSharedMaterialValues_removeValueID(values, dsHashString("test2")));
	EXPECT_FALSE(dsSharedMaterialValues_removeValueID(values, dsHashString("test2")));

	EXPECT_EQ(0U, dsSharedMaterialValues_getValueCount(values));
	EXPECT_FALSE(dsSharedMaterialValues_getTextureName(values, "test1"));
	EXPECT_FALSE(dsSharedMaterialValues_getTextureName(values, "test2"));

	dsSharedMaterialValues_destroy(values);
	EXPECT_TRUE(dsTexture_destroy(texture1));
	EXPECT_TRUE(dsTexture_destroy(texture2));
}

TEST_F(SharedMaterialValuesTest, TextureBuffers)
{
	dsSharedMaterialValues* values = dsSharedMaterialValues_create((dsAllocator*)&allocator,
		DS_DEFAULT_MAX_SHARED_MATERIAL_VALUES);
	ASSERT_TRUE(values);

	dsGfxBuffer* buffer1 = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Texture | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static, NULL, 1024);
	ASSERT_TRUE(buffer1);

	dsGfxBuffer* buffer2 = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Image| dsGfxBufferUsage_CopyTo, dsGfxMemory_Static, NULL, 1024);
	ASSERT_TRUE(buffer2);

	dsGfxBuffer* buffer3 = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Vertex | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static, NULL, 1024);
	ASSERT_TRUE(buffer3);

	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	EXPECT_FALSE(dsSharedMaterialValues_setTextureBufferName(values, "test1", buffer1, format, 24,
		256));
	EXPECT_FALSE(dsSharedMaterialValues_setTextureBufferName(values, "test1", buffer1,
		dsGfxFormat_BC1_RGB, 0, 256));
	EXPECT_FALSE(dsSharedMaterialValues_setTextureBufferName(values, "test1", buffer3,
		format, 0, 256));
	EXPECT_TRUE(dsSharedMaterialValues_setTextureBufferName(values, "test1", buffer1, format,
		0, 256));
	EXPECT_FALSE(dsSharedMaterialValues_setTextureBufferID(values, dsHashString("test2"), buffer2,
		format, 24, 20));
	EXPECT_TRUE(dsSharedMaterialValues_setTextureBufferID(values, dsHashString("test2"), buffer2,
		format, 32, 20));

	EXPECT_EQ(2U, dsSharedMaterialValues_getValueCount(values));

	dsGfxFormat storedFormat;
	size_t offset, count;
	EXPECT_EQ(buffer1, dsSharedMaterialValues_getTextureBufferID(&storedFormat, &offset, &count,
		values, dsHashString("test1")));
	EXPECT_EQ(format, storedFormat);
	EXPECT_EQ(0U, offset);
	EXPECT_EQ(256U, count);

	EXPECT_EQ(buffer2, dsSharedMaterialValues_getTextureBufferName(&storedFormat, &offset, &count,
		values, "test2"));
	EXPECT_EQ(format, storedFormat);
	EXPECT_EQ(32U, offset);
	EXPECT_EQ(20U, count);

	EXPECT_FALSE(dsSharedMaterialValues_getTextureBufferName(&storedFormat, &offset, &count,
		values, "asdf"));
	EXPECT_FALSE(dsSharedMaterialValues_getTextureBufferID(&storedFormat, &offset, &count, values,
		dsHashString("asdf")));
	EXPECT_FALSE(dsSharedMaterialValues_getTextureName(values, "test1"));
	EXPECT_FALSE(dsSharedMaterialValues_getVariableGroupName(values, "test1"));

	EXPECT_TRUE(dsSharedMaterialValues_setTextureBufferName(values, "test1", buffer2, format, 32,
		96));
	EXPECT_TRUE(dsSharedMaterialValues_setTextureBufferName(values, "test2", buffer1, format, 0,
		128));

	EXPECT_EQ(buffer2, dsSharedMaterialValues_getTextureBufferName(&storedFormat, &offset, &count,
		values, "test1"));
	EXPECT_EQ(format, storedFormat);
	EXPECT_EQ(32U, offset);
	EXPECT_EQ(96U, count);

	EXPECT_EQ(buffer1, dsSharedMaterialValues_getTextureBufferName(&storedFormat, &offset, &count,
		values, "test2"));
	EXPECT_EQ(format, storedFormat);
	EXPECT_EQ(0U, offset);
	EXPECT_EQ(128U, count);

	EXPECT_TRUE(dsSharedMaterialValues_removeValueName(values, "test1"));
	EXPECT_FALSE(dsSharedMaterialValues_removeValueName(values, "test1"));
	EXPECT_TRUE(dsSharedMaterialValues_removeValueID(values, dsHashString("test2")));
	EXPECT_FALSE(dsSharedMaterialValues_removeValueID(values, dsHashString("test2")));

	EXPECT_EQ(0U, dsSharedMaterialValues_getValueCount(values));
	EXPECT_FALSE(dsSharedMaterialValues_getTextureBufferName(NULL, NULL, NULL, values, "test1"));
	EXPECT_FALSE(dsSharedMaterialValues_getTextureBufferName(NULL, NULL, NULL, values, "test2"));

	resourceManager->maxTextureBufferElements = 128;
	EXPECT_FALSE(dsSharedMaterialValues_setTextureBufferName(values, "test1", buffer1, format, 0,
		256));

	resourceManager->maxTextureBufferElements = 16*1024*1024;
	resourceManager->hasTextureBufferSubrange = false;
	EXPECT_FALSE(dsSharedMaterialValues_setTextureBufferName(values, "test1", buffer1, format, 4,
		255));
	EXPECT_FALSE(dsSharedMaterialValues_setTextureBufferName(values, "test1", buffer1, format, 0,
		255));
	EXPECT_TRUE(dsSharedMaterialValues_setTextureBufferName(values, "test", buffer1, format, 0,
		256));

	dsSharedMaterialValues_destroy(values);
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer1));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer2));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer3));
}

TEST_F(SharedMaterialValuesTest, VariableGroups)
{
	dsSharedMaterialValues* values = dsSharedMaterialValues_create((dsAllocator*)&allocator,
		DS_DEFAULT_MAX_SHARED_MATERIAL_VALUES);
	ASSERT_TRUE(values);

	dsShaderVariableElement elements[] =
	{
		{"test", dsMaterialType_Vec4, 0}
	};

	dsShaderVariableGroupDesc* desc = dsShaderVariableGroupDesc_create(resourceManager, NULL,
		elements, DS_ARRAY_SIZE(elements));
	ASSERT_TRUE(desc);

	dsShaderVariableGroup* variableGroup1 = dsShaderVariableGroup_create(resourceManager, NULL,
		NULL, desc);
	ASSERT_TRUE(variableGroup1);

	dsShaderVariableGroup* variableGroup2 = dsShaderVariableGroup_create(resourceManager, NULL,
		NULL, desc);
	ASSERT_TRUE(variableGroup2);

	EXPECT_TRUE(dsSharedMaterialValues_setVariableGroupName(values, "test1", variableGroup1));
	EXPECT_TRUE(dsSharedMaterialValues_setVariableGroupID(values, dsHashString("test2"),
		variableGroup2));

	EXPECT_EQ(2U, dsSharedMaterialValues_getValueCount(values));

	EXPECT_EQ(variableGroup1, dsSharedMaterialValues_getVariableGroupID(values,
		dsHashString("test1")));
	EXPECT_EQ(variableGroup2, dsSharedMaterialValues_getVariableGroupName(values, "test2"));

	EXPECT_FALSE(dsSharedMaterialValues_getVariableGroupName(values, "asdf"));
	EXPECT_FALSE(dsSharedMaterialValues_getVariableGroupID(values, dsHashString("asdf")));
	EXPECT_FALSE(dsSharedMaterialValues_getTextureName(values, "test1"));
	EXPECT_TRUE(dsSharedMaterialValues_getBufferName(NULL, NULL, values, "test1"));

	EXPECT_TRUE(dsSharedMaterialValues_setVariableGroupName(values, "test1", variableGroup2));
	EXPECT_TRUE(dsSharedMaterialValues_setVariableGroupName(values, "test2", variableGroup1));
	EXPECT_EQ(variableGroup2, dsSharedMaterialValues_getVariableGroupName(values, "test1"));
	EXPECT_EQ(variableGroup1, dsSharedMaterialValues_getVariableGroupName(values, "test2"));

	EXPECT_TRUE(dsSharedMaterialValues_removeValueName(values, "test1"));
	EXPECT_FALSE(dsSharedMaterialValues_removeValueName(values, "test1"));
	EXPECT_TRUE(dsSharedMaterialValues_removeValueID(values, dsHashString("test2")));
	EXPECT_FALSE(dsSharedMaterialValues_removeValueID(values, dsHashString("test2")));

	EXPECT_EQ(0U, dsSharedMaterialValues_getValueCount(values));
	EXPECT_FALSE(dsSharedMaterialValues_getVariableGroupName(values, "test1"));
	EXPECT_FALSE(dsSharedMaterialValues_getVariableGroupName(values, "test2"));

	dsSharedMaterialValues_destroy(values);
	EXPECT_TRUE(dsShaderVariableGroup_destroy(variableGroup1));
	EXPECT_TRUE(dsShaderVariableGroup_destroy(variableGroup2));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(desc));
}

TEST_F(SharedMaterialValuesTest, Buffers)
{
	dsSharedMaterialValues* values = dsSharedMaterialValues_create((dsAllocator*)&allocator,
		DS_DEFAULT_MAX_SHARED_MATERIAL_VALUES);
	ASSERT_TRUE(values);

	dsGfxBuffer* buffer1 = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static, NULL, 128);
	ASSERT_TRUE(buffer1);

	dsGfxBuffer* buffer2 = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static, NULL, 128);
	ASSERT_TRUE(buffer2);

	dsGfxBuffer* buffer3 = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Vertex | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static, NULL, 128);
	ASSERT_TRUE(buffer3);

	EXPECT_FALSE(dsSharedMaterialValues_setBufferName(values, "test1", buffer1, 64, 128));
	EXPECT_FALSE(dsSharedMaterialValues_setBufferName(values, "test1", buffer3, 0, 128));
	EXPECT_TRUE(dsSharedMaterialValues_setBufferName(values, "test1", buffer1, 0, 128));
	EXPECT_TRUE(dsSharedMaterialValues_setBufferID(values, dsHashString("test2"), buffer2, 64,
		64));

	EXPECT_EQ(2U, dsSharedMaterialValues_getValueCount(values));

	size_t offset, size;
	EXPECT_EQ(buffer1, dsSharedMaterialValues_getBufferID(&offset, &size, values,
		dsHashString("test1")));
	EXPECT_EQ(0U, offset);
	EXPECT_EQ(128U, size);

	EXPECT_EQ(buffer2, dsSharedMaterialValues_getBufferName(&offset, &size, values, "test2"));
	EXPECT_EQ(64U, offset);
	EXPECT_EQ(64U, size);

	EXPECT_FALSE(dsSharedMaterialValues_getBufferName(&offset, &size, values, "asdf"));
	EXPECT_FALSE(dsSharedMaterialValues_getBufferID(&offset, &size, values,
		dsHashString("asdf")));
	EXPECT_FALSE(dsSharedMaterialValues_getTextureName(values, "test1"));
	EXPECT_FALSE(dsSharedMaterialValues_getVariableGroupName(values, "test1"));

	EXPECT_TRUE(dsSharedMaterialValues_setBufferName(values, "test1", buffer2, 32, 96));
	EXPECT_TRUE(dsSharedMaterialValues_setBufferName(values, "test2", buffer1, 0, 128));

	EXPECT_EQ(buffer2, dsSharedMaterialValues_getBufferName(&offset, &size, values, "test1"));
	EXPECT_EQ(32U, offset);
	EXPECT_EQ(96U, size);

	EXPECT_EQ(buffer1, dsSharedMaterialValues_getBufferName(&offset, &size, values, "test2"));
	EXPECT_EQ(0U, offset);
	EXPECT_EQ(128U, size);

	EXPECT_TRUE(dsSharedMaterialValues_removeValueName(values, "test1"));
	EXPECT_FALSE(dsSharedMaterialValues_removeValueName(values, "test1"));
	EXPECT_TRUE(dsSharedMaterialValues_removeValueID(values, dsHashString("test2")));
	EXPECT_FALSE(dsSharedMaterialValues_removeValueID(values, dsHashString("test2")));

	EXPECT_EQ(0U, dsSharedMaterialValues_getValueCount(values));
	EXPECT_FALSE(dsSharedMaterialValues_getBufferName(&offset, &size, values, "test1"));
	EXPECT_FALSE(dsSharedMaterialValues_getBufferName(&offset, &size, values, "test2"));

	resourceManager->maxUniformBlockSize = 64;
	EXPECT_FALSE(dsSharedMaterialValues_setBufferName(values, "test1", buffer1, 0, 128));

	dsSharedMaterialValues_destroy(values);
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer1));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer2));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer3));
}

TEST_F(SharedMaterialValuesTest, MixedTypes)
{
	dsSharedMaterialValues* values = dsSharedMaterialValues_create((dsAllocator*)&allocator,
		DS_DEFAULT_MAX_SHARED_MATERIAL_VALUES);
	ASSERT_TRUE(values);

	dsTextureInfo texInfo = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		dsTextureDim_2D, 16, 16, 0, DS_ALL_MIP_LEVELS, 1};
	dsTexture* texture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo, dsGfxMemory_Static, &texInfo, NULL, 0);
	ASSERT_TRUE(texture);

	dsGfxBuffer* textureBuffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Texture | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static, NULL, 1024);
	ASSERT_TRUE(textureBuffer);

	dsShaderVariableElement elements[] =
	{
		{"test", dsMaterialType_Vec4, 0}
	};

	dsShaderVariableGroupDesc* desc = dsShaderVariableGroupDesc_create(resourceManager, NULL,
		elements, DS_ARRAY_SIZE(elements));
	ASSERT_TRUE(desc);

	dsShaderVariableGroup* variableGroup = dsShaderVariableGroup_create(resourceManager, NULL, NULL,
		desc);
	ASSERT_TRUE(variableGroup);

	dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static, NULL, 128);
	ASSERT_TRUE(buffer);

	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	EXPECT_TRUE(dsSharedMaterialValues_setTextureName(values, "texture", texture));
	EXPECT_TRUE(dsSharedMaterialValues_setTextureBufferName(values, "texture buffer",
		textureBuffer, format, 0, 256));
	EXPECT_TRUE(dsSharedMaterialValues_setVariableGroupName(values, "variable group",
		variableGroup));
	EXPECT_TRUE(dsSharedMaterialValues_setBufferName(values, "buffer", buffer, 0, 128));
	EXPECT_EQ(4U, dsSharedMaterialValues_getValueCount(values));

	dsGfxFormat storedFormat;
	size_t offset, size;
	EXPECT_EQ(texture, dsSharedMaterialValues_getTextureName(values, "texture"));
	EXPECT_EQ(textureBuffer, dsSharedMaterialValues_getTextureBufferName(&storedFormat, &offset,
		&size, values, "texture buffer"));
	EXPECT_EQ(format, storedFormat);
	EXPECT_EQ(0U, offset);
	EXPECT_EQ(256U, size);
	EXPECT_EQ(variableGroup, dsSharedMaterialValues_getVariableGroupName(values,
		"variable group"));
	EXPECT_EQ(buffer, dsSharedMaterialValues_getBufferName(&offset, &size, values, "buffer"));
	EXPECT_EQ(0U, offset);
	EXPECT_EQ(128U, size);

	EXPECT_FALSE(dsSharedMaterialValues_setTextureName(values, "buffer", texture));
	EXPECT_FALSE(dsSharedMaterialValues_setTextureBufferName(values, "texture", textureBuffer,
		format, 0, 128));
	EXPECT_FALSE(dsSharedMaterialValues_setVariableGroupName(values, "buffer", variableGroup));
	EXPECT_FALSE(dsSharedMaterialValues_setBufferName(values, "texture", buffer, 0, 128));

	dsSharedMaterialValues_destroy(values);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_TRUE(dsGfxBuffer_destroy(textureBuffer));
	EXPECT_TRUE(dsShaderVariableGroup_destroy(variableGroup));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(desc));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));
}

TEST_F(SharedMaterialValuesTest, Limit)
{
	dsSharedMaterialValues* values = dsSharedMaterialValues_create((dsAllocator*)&allocator, 2);
	ASSERT_TRUE(values);

	dsTextureInfo texInfo = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		dsTextureDim_2D, 16, 16, 0, DS_ALL_MIP_LEVELS, 1};
	dsTexture* texture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo, dsGfxMemory_Static, &texInfo, NULL, 0);
	ASSERT_TRUE(texture);

	dsShaderVariableElement elements[] =
	{
		{"test", dsMaterialType_Vec4, 0}
	};

	dsShaderVariableGroupDesc* desc = dsShaderVariableGroupDesc_create(resourceManager, NULL,
		elements, DS_ARRAY_SIZE(elements));
	ASSERT_TRUE(desc);

	dsShaderVariableGroup* variableGroup = dsShaderVariableGroup_create(resourceManager, NULL, NULL,
		desc);
	ASSERT_TRUE(variableGroup);

	dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static, NULL, 128);
	ASSERT_TRUE(buffer);

	EXPECT_TRUE(dsSharedMaterialValues_setTextureName(values, "texture", texture));
	EXPECT_TRUE(dsSharedMaterialValues_setVariableGroupName(values, "variable group",
		variableGroup));
	EXPECT_FALSE(dsSharedMaterialValues_setBufferName(values, "buffer", buffer, 0, 128));
	EXPECT_EQ(2U, dsSharedMaterialValues_getValueCount(values));

	EXPECT_TRUE(dsSharedMaterialValues_removeValueName(values, "texture"));
	EXPECT_TRUE(dsSharedMaterialValues_setBufferName(values, "buffer", buffer, 0, 128));
	EXPECT_FALSE(dsSharedMaterialValues_setTextureName(values, "texture", texture));
	EXPECT_EQ(2U, dsSharedMaterialValues_getValueCount(values));

	EXPECT_TRUE(dsSharedMaterialValues_removeValueName(values, "variable group"));
	EXPECT_TRUE(dsSharedMaterialValues_setTextureName(values, "texture", texture));
	EXPECT_FALSE(dsSharedMaterialValues_setVariableGroupName(values, "variable group",
		variableGroup));
	EXPECT_EQ(2U, dsSharedMaterialValues_getValueCount(values));

	EXPECT_TRUE(dsSharedMaterialValues_removeValueName(values, "buffer"));
	EXPECT_TRUE(dsSharedMaterialValues_setVariableGroupName(values, "variable group",
		variableGroup));
	EXPECT_FALSE(dsSharedMaterialValues_setBufferName(values, "buffer", buffer, 0, 128));
	EXPECT_EQ(2U, dsSharedMaterialValues_getValueCount(values));

	EXPECT_TRUE(dsSharedMaterialValues_setTextureName(values, "texture", NULL));
	EXPECT_EQ(2U, dsSharedMaterialValues_getValueCount(values));
	EXPECT_EQ(NULL, dsSharedMaterialValues_getTextureName(values, "texture"));
	EXPECT_TRUE(dsSharedMaterialValues_setTextureName(values, "texture", texture));
	EXPECT_EQ(2U, dsSharedMaterialValues_getValueCount(values));
	EXPECT_EQ(texture, dsSharedMaterialValues_getTextureName(values, "texture"));

	dsSharedMaterialValues_destroy(values);
	EXPECT_TRUE(dsTexture_destroy(texture));
	EXPECT_TRUE(dsShaderVariableGroup_destroy(variableGroup));
	EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(desc));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));
}

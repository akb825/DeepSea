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
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <gtest/gtest.h>

class ShaderModuleTest : public AssetFixtureBase
{
public:
	ShaderModuleTest()
		: AssetFixtureBase("shaders")
	{
	}
};

TEST_F(ShaderModuleTest, LoadFile)
{
	EXPECT_FALSE(dsShaderModule_loadFile(NULL, NULL, getPath("test.mslb")));
	EXPECT_FALSE(dsShaderModule_loadFile(resourceManager, NULL, NULL));
	EXPECT_FALSE(dsShaderModule_loadFile(resourceManager, NULL, "asdf"));
	dsShaderModule* module = dsShaderModule_loadFile(resourceManager, NULL, getPath("test.mslb"));
	ASSERT_TRUE(module);
	EXPECT_EQ(1U, resourceManager->shaderModuleCount);

	EXPECT_EQ(0U, dsShaderModule_shaderCount(NULL));
	ASSERT_EQ(1U, dsShaderModule_shaderCount(module));
	EXPECT_STREQ("Test", dsShaderModule_shaderName(module, 0));
	EXPECT_FALSE(dsShaderModule_shaderName(module, 1));

	EXPECT_TRUE(dsShaderModule_destroy(module));
	EXPECT_EQ(0U, resourceManager->shaderModuleCount);
}

TEST_F(ShaderModuleTest, LoadStream)
{
	dsFileStream fileStream;
	ASSERT_TRUE(dsFileStream_openPath(&fileStream, getPath("test.mslb"), "rb"));
	EXPECT_FALSE(dsShaderModule_loadStream(NULL, NULL, (dsStream*)&fileStream));
	EXPECT_FALSE(dsShaderModule_loadStream(resourceManager, NULL, NULL));

	dsShaderModule* module = dsShaderModule_loadStream(resourceManager, NULL,
		(dsStream*)&fileStream);
	EXPECT_TRUE(dsStream_close((dsStream*)&fileStream));
	ASSERT_TRUE(module);
	EXPECT_EQ(1U, resourceManager->shaderModuleCount);

	EXPECT_EQ(0U, dsShaderModule_shaderCount(NULL));
	ASSERT_EQ(1U, dsShaderModule_shaderCount(module));
	EXPECT_STREQ("Test", dsShaderModule_shaderName(module, 0));
	EXPECT_FALSE(dsShaderModule_shaderName(module, 1));

	EXPECT_TRUE(dsShaderModule_destroy(module));
	EXPECT_EQ(0U, resourceManager->shaderModuleCount);
}

TEST_F(ShaderModuleTest, LoadDta)
{
	dsFileStream fileStream;
	ASSERT_TRUE(dsFileStream_openPath(&fileStream, getPath("test.mslb"), "rb"));
	ASSERT_TRUE(dsStream_seek((dsStream*)&fileStream, 0, dsStreamSeekWay_End));
	uint64_t size = dsStream_tell((dsStream*)&fileStream);
	ASSERT_NE(DS_STREAM_INVALID_POS, size);
	ASSERT_TRUE(dsStream_seek((dsStream*)&fileStream, 0, dsStreamSeekWay_Beginning));

	void* data = dsAllocator_alloc((dsAllocator*)&allocator, (size_t)size);
	ASSERT_TRUE(data);
	ASSERT_EQ(size, dsStream_read((dsStream*)&fileStream, data, (size_t)size));
	ASSERT_TRUE(dsStream_close((dsStream*)&fileStream));

	EXPECT_FALSE(dsShaderModule_loadData(NULL, NULL, data, (size_t)size));
	EXPECT_FALSE(dsShaderModule_loadData(resourceManager, NULL, NULL, (size_t)size));
	EXPECT_FALSE(dsShaderModule_loadData(resourceManager, NULL, data, (size_t)size - 10));

	dsShaderModule* module = dsShaderModule_loadData(resourceManager, NULL, data, (size_t)size);
	ASSERT_TRUE(module);
	EXPECT_EQ(1U, resourceManager->shaderModuleCount);
	dsAllocator_free((dsAllocator*)&allocator, data);

	EXPECT_EQ(0U, dsShaderModule_shaderCount(NULL));
	ASSERT_EQ(1U, dsShaderModule_shaderCount(module));
	EXPECT_STREQ("Test", dsShaderModule_shaderName(module, 0));
	EXPECT_FALSE(dsShaderModule_shaderName(module, 1));

	EXPECT_TRUE(dsShaderModule_destroy(module));
	EXPECT_EQ(0U, resourceManager->shaderModuleCount);
}

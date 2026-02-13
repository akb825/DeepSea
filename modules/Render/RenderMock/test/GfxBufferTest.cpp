/*
 * Copyright 2016-2026 Aaron Barany
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

#include "Fixtures/RenderPassFixtureBase.h"
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/RenderPass.h>
#include <gtest/gtest.h>
#include <string.h>

namespace
{

struct TestData
{
	float f;
	int i;
};

} // namespace

class GfxBufferTest : public RenderPassFixtureBase
{
};

TEST_F(GfxBufferTest, Create)
{
	EXPECT_FALSE(dsGfxBuffer_create(resourceManager, nullptr, (dsGfxBufferUsage)0,
		dsGfxMemory_Static, nullptr, 100));
	EXPECT_FALSE(dsGfxBuffer_create(resourceManager, nullptr, dsGfxBufferUsage_Vertex,
		(dsGfxMemory)0, nullptr, 100));

	dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, nullptr, dsGfxBufferUsage_Vertex,
		dsGfxMemory_Static | dsGfxMemory_Draw, nullptr, 100);
	EXPECT_TRUE(buffer);
	EXPECT_EQ(1U, resourceManager->bufferCount);
	EXPECT_EQ(100U, resourceManager->bufferMemorySize);
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));
	EXPECT_EQ(0U, resourceManager->bufferCount);
	EXPECT_EQ(0U, resourceManager->bufferMemorySize);
}

TEST_F(GfxBufferTest, Map)
{
	TestData testData = {1.2f, 3};

	dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, nullptr, dsGfxBufferUsage_Vertex,
		dsGfxMemory_Static | dsGfxMemory_Draw, &testData, sizeof(testData));
	ASSERT_TRUE(buffer);
	EXPECT_FALSE(dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, DS_MAP_FULL_BUFFER));
	EXPECT_TRUE(dsGfxBuffer_map(buffer, dsGfxBufferMap_Write, 0, DS_MAP_FULL_BUFFER));
	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));

	buffer = dsGfxBuffer_create(resourceManager, nullptr, dsGfxBufferUsage_Vertex,
		dsGfxMemory_Static | dsGfxMemory_Read | dsGfxMemory_GPUOnly, &testData, sizeof(testData));
	ASSERT_TRUE(buffer);
	EXPECT_FALSE(dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, DS_MAP_FULL_BUFFER));
	EXPECT_FALSE(dsGfxBuffer_map(buffer, dsGfxBufferMap_Write, 0, DS_MAP_FULL_BUFFER));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));

	buffer = dsGfxBuffer_create(resourceManager, nullptr, dsGfxBufferUsage_Vertex,
		dsGfxMemory_Static | dsGfxMemory_Draw | dsGfxMemory_Read | dsGfxMemory_Persistent,
		&testData, sizeof(testData));
	ASSERT_TRUE(buffer);
	EXPECT_FALSE(dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, sizeof(TestData) + 10));
	void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, DS_MAP_FULL_BUFFER);
	ASSERT_TRUE(data);
	EXPECT_EQ(0, memcmp(&testData, data, sizeof(testData)));
	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	data = dsGfxBuffer_map(buffer,
		dsGfxBufferMap_Read | dsGfxBufferMap_Write | dsGfxBufferMap_Persistent, 4, 4);
	ASSERT_TRUE(data);
	EXPECT_EQ(3, *(int*)data);
	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));
}

TEST_F(GfxBufferTest, FlushInvalidate)
{
	TestData testData = {1.2f, 3};

	dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, nullptr, dsGfxBufferUsage_Vertex,
		dsGfxMemory_Static | dsGfxMemory_Draw, &testData, sizeof(testData));
	ASSERT_TRUE(buffer);

	EXPECT_TRUE(dsGfxBuffer_flush(buffer, 0, sizeof(testData)));
	EXPECT_TRUE(dsGfxBuffer_invalidate(buffer, 0, sizeof(testData)));

	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));

	buffer = dsGfxBuffer_create(resourceManager, nullptr, dsGfxBufferUsage_Vertex,
		dsGfxMemory_Static | dsGfxMemory_Draw | dsGfxMemory_Coherent, &testData,
		sizeof(testData));
	ASSERT_TRUE(buffer);

	EXPECT_TRUE(dsGfxBuffer_flush(buffer, 0, sizeof(testData)));
	EXPECT_TRUE(dsGfxBuffer_invalidate(buffer, 0, sizeof(testData)));

	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));
}

TEST_F(GfxBufferTest, CopyData)
{
	TestData testData = {1.2f, 3};
	TestData copyData = {3.4f, 5};
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, nullptr, dsGfxBufferUsage_Vertex,
		dsGfxMemory_Static | dsGfxMemory_Read, &testData, sizeof(testData));
	ASSERT_TRUE(buffer);
	EXPECT_FALSE(dsGfxBuffer_copyData(buffer, commandBuffer, 0, &copyData, sizeof(copyData)));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));

	buffer = dsGfxBuffer_create(resourceManager, nullptr,
		dsGfxBufferUsage_Vertex | dsGfxBufferUsage_CopyTo, dsGfxMemory_Static | dsGfxMemory_Read,
		&testData, sizeof(testData));
	ASSERT_TRUE(buffer);
	EXPECT_FALSE(dsGfxBuffer_copyData(buffer, commandBuffer, 4, &copyData, sizeof(copyData)));
	EXPECT_TRUE(dsGfxBuffer_copyData(buffer, commandBuffer, 0, &copyData, sizeof(copyData)));

	EXPECT_TRUE(dsRenderPass_begin(
		renderPass, commandBuffer, framebuffer, nullptr, nullptr, nullptr, 0, false));
	EXPECT_FALSE(dsGfxBuffer_copyData(buffer, commandBuffer, 0, &copyData, sizeof(copyData)));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	void* data = dsGfxBuffer_map(buffer, dsGfxBufferMap_Read, 0, sizeof(copyData));
	ASSERT_TRUE(data);
	EXPECT_EQ(0, memcmp(&copyData, data, sizeof(copyData)));
	EXPECT_TRUE(dsGfxBuffer_unmap(buffer));

	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));
}

TEST_F(GfxBufferTest, Copy)
{
	TestData testData = {1.2f, 3};
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsGfxBuffer* fromBuffer = dsGfxBuffer_create(resourceManager, nullptr, dsGfxBufferUsage_Vertex,
		dsGfxMemory_GPUOnly, &testData, sizeof(testData));
	ASSERT_TRUE(fromBuffer);
	dsGfxBuffer* toBuffer = dsGfxBuffer_create(resourceManager, nullptr, dsGfxBufferUsage_CopyTo,
		dsGfxMemory_Static | dsGfxMemory_Read, nullptr, sizeof(testData));
	ASSERT_TRUE(toBuffer);
	EXPECT_FALSE(dsGfxBuffer_copy(commandBuffer, fromBuffer, 0, toBuffer, 0, sizeof(testData)));

	EXPECT_TRUE(dsGfxBuffer_destroy(fromBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(toBuffer));

	fromBuffer = dsGfxBuffer_create(resourceManager, nullptr, dsGfxBufferUsage_CopyFrom,
		dsGfxMemory_GPUOnly, &testData, sizeof(testData));
	ASSERT_TRUE(fromBuffer);
	toBuffer = dsGfxBuffer_create(resourceManager, nullptr, dsGfxBufferUsage_Vertex,
		dsGfxMemory_Static | dsGfxMemory_Read, nullptr, sizeof(testData));
	ASSERT_TRUE(toBuffer);
	EXPECT_FALSE(dsGfxBuffer_copy(commandBuffer, fromBuffer, 0, toBuffer, 0, sizeof(testData)));

	EXPECT_TRUE(dsGfxBuffer_destroy(fromBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(toBuffer));

	fromBuffer = dsGfxBuffer_create(resourceManager, nullptr, dsGfxBufferUsage_CopyFrom,
		dsGfxMemory_GPUOnly, &testData, sizeof(testData));
	ASSERT_TRUE(fromBuffer);
	toBuffer = dsGfxBuffer_create(resourceManager, nullptr, dsGfxBufferUsage_CopyTo,
		dsGfxMemory_Static | dsGfxMemory_Read, nullptr, sizeof(testData));
	ASSERT_TRUE(toBuffer);
	EXPECT_FALSE(dsGfxBuffer_copy(commandBuffer, fromBuffer, 4, toBuffer, 0, sizeof(testData)));
	EXPECT_FALSE(dsGfxBuffer_copy(commandBuffer, fromBuffer, 0, toBuffer, 4, sizeof(testData)));
	EXPECT_TRUE(dsGfxBuffer_copy(commandBuffer, fromBuffer, 0, toBuffer, 0, sizeof(testData)));

	EXPECT_TRUE(dsRenderPass_begin(
		renderPass, commandBuffer, framebuffer, nullptr, nullptr, nullptr, 0, false));
	EXPECT_FALSE(dsGfxBuffer_copy(commandBuffer, fromBuffer, 0, toBuffer, 0, sizeof(testData)));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	void* data = dsGfxBuffer_map(toBuffer, dsGfxBufferMap_Read, 0, sizeof(testData));
	ASSERT_TRUE(data);
	EXPECT_EQ(0, memcmp(&testData, data, sizeof(testData)));
	EXPECT_TRUE(dsGfxBuffer_unmap(toBuffer));

	EXPECT_TRUE(dsGfxBuffer_destroy(fromBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(toBuffer));
}

TEST_F(GfxBufferTest, CopyToTexture)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	// 32 x 16 texture, 3 levels, 4 arrays.
	dsColor textureData[(32*16 + 16*8 + 8*4)*4];
	for (unsigned int level = 0, index = 0; level < 3; ++level)
	{
		unsigned int width = 32 >> level;
		unsigned int height = 16 >> level;
		for (unsigned int depth = 0; depth < 4; ++depth)
		{
			for (unsigned int y = 0; y < height; ++y)
			{
				for (unsigned int x = 0; x < width; ++x, ++index)
				{
					textureData[index].r = (uint8_t)x;
					textureData[index].g = (uint8_t)y;
					textureData[index].b = (uint8_t)level;
					textureData[index].a = (uint8_t)depth;
				}
			}
		}
	}

	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	dsTextureInfo fromInfo = {format, dsTextureDim_2D, 32, 16, 4, 3, 1};
	dsGfxBuffer* fromBuffer = dsGfxBuffer_create(resourceManager, nullptr, dsGfxBufferUsage_Vertex,
		dsGfxMemory_GPUOnly, textureData, sizeof(textureData));
	ASSERT_TRUE(fromBuffer);

	dsTextureInfo toInfo = {format, dsTextureDim_2D, 16, 32, 5, 2, 1};
	dsTexture* toTexture = dsTexture_create(resourceManager, nullptr,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo | dsTextureUsage_CopyFrom, dsGfxMemory_Read,
		&toInfo, nullptr, 0);
	ASSERT_TRUE(toTexture);

	// array index 2, mip level 1 (16 x 8), position (1, 2)
	size_t bufferOffset = dsTexture_layerOffset(&fromInfo, 2, 1) + (2*16 + 1)*sizeof(dsColor);
	dsGfxBufferTextureCopyRegion copyRegion =
	{
		bufferOffset, 16, 8,
		{dsCubeFace_None, 3, 4, 1, 0},
		8, 4, 2
	};

	EXPECT_FALSE(dsGfxBuffer_copyToTexture(commandBuffer, fromBuffer, toTexture, &copyRegion, 1));
	EXPECT_TRUE(dsGfxBuffer_destroy(fromBuffer));
	EXPECT_TRUE(dsTexture_destroy(toTexture));

	fromBuffer = dsGfxBuffer_create(resourceManager, nullptr, dsGfxBufferUsage_CopyFrom,
		dsGfxMemory_GPUOnly, textureData, sizeof(textureData));
	ASSERT_TRUE(fromBuffer);

	toTexture = dsTexture_create(resourceManager, nullptr, dsTextureUsage_Texture, dsGfxMemory_Read,
		&toInfo, nullptr, 0);
	ASSERT_TRUE(toTexture);

	EXPECT_FALSE(dsGfxBuffer_copyToTexture(commandBuffer, fromBuffer, toTexture, &copyRegion, 1));
	EXPECT_TRUE(dsGfxBuffer_destroy(fromBuffer));
	EXPECT_TRUE(dsTexture_destroy(toTexture));

	fromBuffer = dsGfxBuffer_create(resourceManager, nullptr, dsGfxBufferUsage_CopyFrom,
		dsGfxMemory_GPUOnly, textureData, sizeof(textureData));
	ASSERT_TRUE(fromBuffer);

	toTexture = dsTexture_create(resourceManager, nullptr,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo | dsTextureUsage_CopyFrom, dsGfxMemory_Read,
		&toInfo, nullptr, 0);
	ASSERT_TRUE(toTexture);

	EXPECT_TRUE(dsRenderPass_begin(
		renderPass, commandBuffer, framebuffer, nullptr, nullptr, nullptr, 0, false));
	EXPECT_FALSE(dsGfxBuffer_copyToTexture(commandBuffer, fromBuffer, toTexture, &copyRegion, 1));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	EXPECT_TRUE(dsGfxBuffer_copyToTexture(commandBuffer, fromBuffer, toTexture, &copyRegion, 1));

	dsColor readTextureData[8*4];
	EXPECT_TRUE(dsTexture_getData(readTextureData, sizeof(readTextureData), toTexture,
		&copyRegion.texturePosition, 8, 4));
	for (unsigned int y = 0, index = 0; y < 4; ++y)
	{
		for (unsigned int x = 0; x < 8; ++x, ++index)
		{
			EXPECT_EQ(x + 1, readTextureData[index].r);
			EXPECT_EQ(y + 2, readTextureData[index].g);
			EXPECT_EQ(1U, readTextureData[index].b);
			EXPECT_EQ(2U, readTextureData[index].a);
		}
	}

	copyRegion.texturePosition.depth = 2;
	EXPECT_TRUE(dsTexture_getData(readTextureData, sizeof(readTextureData), toTexture,
		&copyRegion.texturePosition, 8, 4));
	for (unsigned int y = 0, index = 0; y < 4; ++y)
	{
		for (unsigned int x = 0; x < 8; ++x, ++index)
		{
			EXPECT_EQ(x + 1, readTextureData[index].r);
			EXPECT_EQ(y + 2, readTextureData[index].g);
			EXPECT_EQ(1U, readTextureData[index].b);
			EXPECT_EQ(3U, readTextureData[index].a);
		}
	}

	copyRegion.bufferWidth = 1;
	EXPECT_FALSE(dsGfxBuffer_copyToTexture(commandBuffer, fromBuffer, toTexture, &copyRegion, 1));

	copyRegion.bufferWidth = 16;
	copyRegion.bufferHeight = 1;
	EXPECT_FALSE(dsGfxBuffer_copyToTexture(commandBuffer, fromBuffer, toTexture, &copyRegion, 1));

	copyRegion.bufferHeight = 8;
	copyRegion.bufferOffset = dsTexture_layerOffset(&fromInfo, 3, 2);
	EXPECT_FALSE(dsGfxBuffer_copyToTexture(commandBuffer, fromBuffer, toTexture, &copyRegion, 1));

	copyRegion.bufferOffset = bufferOffset;
	copyRegion.texturePosition.x = 17;
	EXPECT_FALSE(dsGfxBuffer_copyToTexture(commandBuffer, fromBuffer, toTexture, &copyRegion, 1));

	copyRegion.texturePosition.x = 3;
	copyRegion.texturePosition.y = 29;
	EXPECT_FALSE(dsGfxBuffer_copyToTexture(commandBuffer, fromBuffer, toTexture, &copyRegion, 1));

	copyRegion.texturePosition.y = 4;
	copyRegion.texturePosition.mipLevel = 3;
	EXPECT_FALSE(dsGfxBuffer_copyToTexture(commandBuffer, fromBuffer, toTexture, &copyRegion, 1));

	copyRegion.texturePosition.mipLevel = 0;
	copyRegion.texturePosition.depth = 4;
	EXPECT_FALSE(dsGfxBuffer_copyToTexture(commandBuffer, fromBuffer, toTexture, &copyRegion, 1));

	EXPECT_TRUE(dsGfxBuffer_destroy(fromBuffer));
	EXPECT_TRUE(dsTexture_destroy(toTexture));
}

/*
 * Copyright 2019 Aaron Barany
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
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Renderer.h>

class ResourceCopyTest : public FixtureBase
{
};

INSTANTIATE_TEST_CASE_P(RendererFunctional, ResourceCopyTest, FixtureBase::getRendererTypes());

TEST_P(ResourceCopyTest, CopyBuffers)
{
	if (!resourceManager->canCopyBuffers)
	{
		DS_LOG_INFO("ResourceCopyTest", "buffer copying not supported: skipping test.");
		return;
	}

	struct TestData
	{
		float f;
		int i;
	};
	TestData testData = {1.2f, 3};
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsGfxBuffer* fromBuffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_CopyFrom,
		dsGfxMemory_GPUOnly, &testData, sizeof(testData));
	ASSERT_TRUE(fromBuffer);
	dsGfxBuffer* toBuffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_CopyTo,
		(dsGfxMemory)(dsGfxMemory_Read | dsGfxMemory_Synchronize), NULL, sizeof(testData) + 4);
	ASSERT_TRUE(toBuffer);
	EXPECT_TRUE(dsGfxBuffer_copy(commandBuffer, fromBuffer, 0, toBuffer, 4, sizeof(testData)));
	EXPECT_TRUE(dsRenderer_flush(renderer));

	void* data = dsGfxBuffer_map(toBuffer, dsGfxBufferMap_Read, 4, sizeof(testData));
	ASSERT_TRUE(data);
	EXPECT_EQ(0, memcmp(&testData, data, sizeof(testData)));
	EXPECT_TRUE(dsGfxBuffer_unmap(toBuffer));

	EXPECT_TRUE(dsGfxBuffer_destroy(fromBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(toBuffer));
}

TEST_P(ResourceCopyTest, CopyTextures)
{
	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	if (!dsGfxFormat_textureCopySupported(resourceManager, format, format))
	{
		DS_LOG_INFO("ResourceCopyTest", "texture copying not supported: skipping test.");
		return;
	}

	if (!dsGfxFormat_copyBufferToTextureSupported(resourceManager, format))
	{
		DS_LOG_INFO("ResourceCopyTest",
			"texture to buffer copying not supported: skipping test.");
		return;
	}

	if (!resourceManager->hasArbitraryMipmapping)
	{
		DS_LOG_INFO("ResourceCopyTest", "arbitrary mipmapping not supported: skipping test.");
		return;
	}

	if (resourceManager->maxTextureArrayLevels < 5)
	{
		DS_LOG_INFO("ResourceCopyTest", "texture arrays not supported: skipping test.");
		return;
	}

	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

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

	dsTextureInfo fromInfo = {format, dsTextureDim_2D, 32, 16, 4, 3, 1};
	dsTexture* fromTexture = dsTexture_create(resourceManager, NULL, dsTextureUsage_CopyFrom,
		dsGfxMemory_GPUOnly, &fromInfo, textureData, sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	dsTextureInfo toInfo = {format, dsTextureDim_2D, 16, 32, 5, 2, 1};
	dsTexture* toTexture = dsTexture_create(resourceManager, NULL,
		(dsTextureUsage)(dsTextureUsage_CopyTo | dsTextureUsage_CopyFrom), dsGfxMemory_GPUOnly,
		&toInfo, NULL, 0);
	ASSERT_TRUE(toTexture);

	dsGfxBuffer* readBuffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_CopyTo,
		(dsGfxMemory)(dsGfxMemory_Read | dsGfxMemory_Synchronize), NULL,
		8*4*dsGfxFormat_size(format)*2);
	ASSERT_TRUE(readBuffer);

	dsTextureCopyRegion copyRegion =
	{
		{dsCubeFace_None, 1, 2, 2, 1},
		{dsCubeFace_None, 3, 4, 1, 0},
		8, 4, 2
	};
	EXPECT_TRUE(dsTexture_copy(commandBuffer, fromTexture, toTexture, &copyRegion, 1));

	dsGfxBufferTextureCopyRegion readRegion =
	{
		0, 0, 0,
		{dsCubeFace_None, 3, 4, 1, 0},
		8, 4, 2
	};
	EXPECT_TRUE(dsTexture_copyToBuffer(commandBuffer, toTexture, readBuffer, &readRegion, 1));
	EXPECT_TRUE(dsRenderer_flush(renderer));

	dsColor* readTextureData = (dsColor*)dsGfxBuffer_map(readBuffer, dsGfxBufferMap_Read, 0,
		DS_MAP_FULL_BUFFER);
	ASSERT_TRUE(readTextureData);
	unsigned int index = 0;
	for (unsigned int y = 0; y < 4; ++y)
	{
		for (unsigned int x = 0; x < 8; ++x, ++index)
		{
			EXPECT_EQ(x + 1, readTextureData[index].r);
			EXPECT_EQ(y + 2, readTextureData[index].g);
			EXPECT_EQ(1U, readTextureData[index].b);
			EXPECT_EQ(2U, readTextureData[index].a);
		}
	}

	for (unsigned int y = 0; y < 4; ++y)
	{
		for (unsigned int x = 0; x < 8; ++x, ++index)
		{
			EXPECT_EQ(x + 1, readTextureData[index].r);
			EXPECT_EQ(y + 2, readTextureData[index].g);
			EXPECT_EQ(1U, readTextureData[index].b);
			EXPECT_EQ(3U, readTextureData[index].a);
		}
	}
	EXPECT_TRUE(dsGfxBuffer_unmap(readBuffer));

	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsTexture_destroy(toTexture));
	EXPECT_TRUE(dsGfxBuffer_destroy(readBuffer));
}

TEST_P(ResourceCopyTest, CopyBufferToTexture)
{
	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	if (!dsGfxFormat_copyBufferToTextureSupported(resourceManager, format) ||
		!dsGfxFormat_copyTextureToBufferSupported(resourceManager, format))
	{
		DS_LOG_INFO("ResourceCopyTest",
			"copying between buffers and textures not supported: skipping test.");
		return;
	}

	if (!resourceManager->hasArbitraryMipmapping)
	{
		DS_LOG_INFO("ResourceCopyTest", "arbitrary mipmapping not supported: skipping test.");
		return;
	}

	if (resourceManager->maxTextureArrayLevels < 5)
	{
		DS_LOG_INFO("ResourceCopyTest", "texture arrays not supported: skipping test.");
		return;
	}

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

	dsTextureInfo fromInfo = {format, dsTextureDim_2D, 32, 16, 4, 3, 1};
	dsGfxBuffer* fromBuffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_CopyFrom,
		dsGfxMemory_GPUOnly, textureData, sizeof(textureData));
	ASSERT_TRUE(fromBuffer);

	dsTextureInfo toInfo = {format, dsTextureDim_2D, 16, 32, 5, 2, 1};
	dsTexture* toTexture = dsTexture_create(resourceManager, NULL,
		(dsTextureUsage)(dsTextureUsage_CopyTo | dsTextureUsage_CopyFrom), dsGfxMemory_GPUOnly,
		&toInfo, NULL, 0);
	ASSERT_TRUE(toTexture);

	dsGfxBuffer* readBuffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_CopyTo,
		(dsGfxMemory)(dsGfxMemory_Read | dsGfxMemory_Synchronize), NULL,
		8*4*dsGfxFormat_size(format)*2);
	ASSERT_TRUE(fromBuffer);

	// array index 2, mip level 1 (16 x 8), position (1, 2)
	size_t bufferOffset = dsTexture_layerOffset(&fromInfo, 2, 1) + (2*16 + 1)*sizeof(dsColor);
	dsGfxBufferTextureCopyRegion copyRegion =
	{
		bufferOffset, 16, 8,
		{dsCubeFace_None, 3, 4, 1, 0},
		8, 4, 2
	};
	EXPECT_TRUE(dsGfxBuffer_copyToTexture(commandBuffer, fromBuffer, toTexture, &copyRegion, 1));

	dsGfxBufferTextureCopyRegion readRegion =
	{
		0, 0, 0,
		{dsCubeFace_None, 3, 4, 1, 0},
		8, 4, 2
	};
	EXPECT_TRUE(dsTexture_copyToBuffer(commandBuffer, toTexture, readBuffer, &readRegion, 1));
	EXPECT_TRUE(dsRenderer_flush(renderer));

	dsColor* readTextureData = (dsColor*)dsGfxBuffer_map(readBuffer, dsGfxBufferMap_Read, 0,
		DS_MAP_FULL_BUFFER);
	ASSERT_TRUE(readTextureData);
	unsigned int index = 0;
	for (unsigned int y = 0; y < 4; ++y)
	{
		for (unsigned int x = 0; x < 8; ++x, ++index)
		{
			EXPECT_EQ(x + 1, readTextureData[index].r);
			EXPECT_EQ(y + 2, readTextureData[index].g);
			EXPECT_EQ(1U, readTextureData[index].b);
			EXPECT_EQ(2U, readTextureData[index].a);
		}
	}

	for (unsigned int y = 0; y < 4; ++y)
	{
		for (unsigned int x = 0; x < 8; ++x, ++index)
		{
			EXPECT_EQ(x + 1, readTextureData[index].r);
			EXPECT_EQ(y + 2, readTextureData[index].g);
			EXPECT_EQ(1U, readTextureData[index].b);
			EXPECT_EQ(3U, readTextureData[index].a);
		}
	}
	EXPECT_TRUE(dsGfxBuffer_unmap(readBuffer));

	EXPECT_TRUE(dsGfxBuffer_destroy(fromBuffer));
	EXPECT_TRUE(dsTexture_destroy(toTexture));
	EXPECT_TRUE(dsGfxBuffer_destroy(readBuffer));
}

TEST_P(ResourceCopyTest, CopyTextureToBuffer)
{
	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	if (!dsGfxFormat_copyTextureToBufferSupported(resourceManager, format))
	{
		DS_LOG_INFO("ResourceCopyTest",
			"texture to buffer copying not supported: skipping test.");
		return;
	}

	if (!resourceManager->hasArbitraryMipmapping)
	{
		DS_LOG_INFO("ResourceCopyTest", "arbitrary mipmapping not supported: skipping test.");
		return;
	}

	if (resourceManager->maxTextureArrayLevels < 5)
	{
		DS_LOG_INFO("ResourceCopyTest", "texture arrays not supported: skipping test.");
		return;
	}

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

	dsTextureInfo fromInfo = {format, dsTextureDim_2D, 32, 16, 4, 3, 1};
	dsTexture* fromTexture = dsTexture_create(resourceManager, NULL, dsTextureUsage_CopyFrom,
		dsGfxMemory_GPUOnly, &fromInfo, textureData,
		sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	dsTextureInfo toInfo = {format, dsTextureDim_2D, 16, 32, 5, 2, 1};
	size_t toSize = dsTexture_size(&toInfo);
	dsGfxBuffer* toBuffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_CopyTo,
		(dsGfxMemory)(dsGfxMemory_Read | dsGfxMemory_Synchronize), NULL, toSize);
	ASSERT_TRUE(toBuffer);

	// array index 1, mip level 0, position (3, 4)
	size_t bufferOffset = dsTexture_layerOffset(&fromInfo, 1, 0) + (4*16 + 3)*sizeof(dsColor);
	dsGfxBufferTextureCopyRegion copyRegion =
	{
		bufferOffset, 16, 32,
		{dsCubeFace_None, 1, 2, 2, 1},
		8, 4, 2
	};

	EXPECT_TRUE(dsTexture_copyToBuffer(commandBuffer, fromTexture, toBuffer, &copyRegion, 1));
	EXPECT_TRUE(dsRenderer_flush(renderer));

	const uint8_t* bufferData =
		(const uint8_t*)dsGfxBuffer_map(toBuffer, dsGfxBufferMap_Read, 0, DS_MAP_FULL_BUFFER);
	ASSERT_TRUE(bufferData);
	const dsColor* readTextureData = (const dsColor*)(bufferData + bufferOffset);
	for (unsigned int y = 0, index = 0; y < 4; ++y)
	{
		for (unsigned int x = 0; x < 8; ++x, ++index)
		{
			EXPECT_EQ(x + 1, readTextureData[index].r);
			EXPECT_EQ(y + 2, readTextureData[index].g);
			EXPECT_EQ(1U, readTextureData[index].b);
			EXPECT_EQ(2U, readTextureData[index].a);
		}
		index += copyRegion.bufferWidth - copyRegion.textureWidth;
	}

	size_t nextBufferOffset = dsTexture_layerOffset(&fromInfo, 2, 0) + (4*16 + 3)*sizeof(dsColor);
	readTextureData = (const dsColor*)(bufferData + nextBufferOffset);
	for (unsigned int y = 0, index = 0; y < 4; ++y)
	{
		for (unsigned int x = 0; x < 8; ++x, ++index)
		{
			EXPECT_EQ(x + 1, readTextureData[index].r);
			EXPECT_EQ(y + 2, readTextureData[index].g);
			EXPECT_EQ(1U, readTextureData[index].b);
			EXPECT_EQ(3U, readTextureData[index].a);
		}
		index += copyRegion.bufferWidth - copyRegion.textureWidth;
	}
	EXPECT_TRUE(dsGfxBuffer_unmap(toBuffer));

	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsGfxBuffer_destroy(toBuffer));
}

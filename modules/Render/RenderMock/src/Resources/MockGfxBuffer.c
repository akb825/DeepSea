/*
 * Copyright 2016-2019 Aaron Barany
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

#include "Resources/MockGfxBuffer.h"

#include "MockTypes.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <string.h>

dsGfxBuffer* dsMockGfxBuffer_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsGfxBufferUsage usage, dsGfxMemory memoryHints, const void* data, size_t size)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);
	dsMockGfxBuffer* buffer = (dsMockGfxBuffer*)dsAllocator_alloc(allocator,
		sizeof(dsMockGfxBuffer) + size);
	if (!buffer)
		return NULL;

	buffer->buffer.resourceManager = resourceManager;
	buffer->buffer.allocator = dsAllocator_keepPointer(allocator);
	buffer->buffer.usage = usage;
	buffer->buffer.memoryHints = memoryHints;
	buffer->buffer.size = size;
	if (data)
		memcpy(buffer->data, data, size);

	return &buffer->buffer;
}

void* dsMockGfxBuffer_map(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	dsGfxBufferMap flags, size_t offset, size_t size)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(flags);
	DS_ASSERT(buffer);
	if (size == DS_MAP_FULL_BUFFER)
		size = buffer->size;
	DS_ASSERT(offset + size <= buffer->size);
	return ((dsMockGfxBuffer*)buffer)->data + offset;
}

bool dsMockGfxBuffer_unmap(dsResourceManager* resourceManager, dsGfxBuffer* buffer)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(buffer);
	return true;
}

bool dsMockGfxBuffer_flush(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	size_t offset, size_t size)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(buffer);
	DS_UNUSED(offset);
	DS_UNUSED(size);
	return true;
}

bool dsMockGfxBuffer_invalidate(dsResourceManager* resourceManager, dsGfxBuffer* buffer,
	size_t offset, size_t size)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(buffer);
	DS_UNUSED(offset);
	DS_UNUSED(size);
	return true;
}

bool dsMockGfxBuffer_copyData(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxBuffer* buffer, size_t offset, const void* data, size_t size)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(buffer);
	DS_ASSERT(offset + size <= buffer->size);
	DS_ASSERT(data);
	memcpy(((dsMockGfxBuffer*)buffer)->data + offset, data, size);
	return true;
}

bool dsMockGfxBuffer_copy(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxBuffer* srcBuffer, size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset,
	size_t size)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(srcBuffer);
	DS_ASSERT(srcOffset + size <= srcBuffer->size);
	DS_ASSERT(dstBuffer);
	DS_ASSERT(dstOffset + size <= dstBuffer->size);
	memcpy(((dsMockGfxBuffer*)dstBuffer)->data + dstOffset,
		((dsMockGfxBuffer*)srcBuffer)->data + srcOffset, size);
	return true;
}

bool dsMockGfxBuffer_copyToTexture(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer, dsTexture* dstTexture,
	const dsGfxBufferTextureCopyRegion* regions, uint32_t regionCount)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(srcBuffer);
	DS_ASSERT(dstTexture);
	DS_ASSERT(regions || regionCount == 0);

	dsMockGfxBuffer* mockSrcBuffer = (dsMockGfxBuffer*)srcBuffer;
	dsMockTexture* mockDstTexture = (dsMockTexture*)dstTexture;

	const dsTextureInfo* info = &dstTexture->info;
	unsigned int formatSize = dsGfxFormat_size(info->format);
	unsigned int blockX, blockY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, info->format));
	for (uint32_t i = 0; i < regionCount; ++i)
	{
		const dsGfxBufferTextureCopyRegion* region = regions + i;
		const dsTexturePosition* position = &region->texturePosition;
		if (region->textureWidth == 0 || region->textureHeight == 0 || region->layers == 0)
			continue;

		uint32_t layerOffset = position->depth;
		if (info->dimension == dsTextureDim_Cube)
			layerOffset = layerOffset*6 + position->face;

		uint32_t bufferWidth = region->bufferWidth;
		uint32_t bufferHeight = region->bufferHeight;
		if (bufferWidth == 0)
			bufferWidth = region->textureWidth;
		if (bufferHeight == 0)
			bufferHeight = region->textureHeight;

		size_t textureXBlocks = (region->textureWidth + blockX- 1)/blockX;
		size_t rowSize = textureXBlocks*formatSize;
		size_t mipWidth = dsMax(1U, info->width >> position->mipLevel);
		size_t mipXBlocks = (mipWidth + blockX - 1)/blockX;
		size_t textureStride = mipXBlocks*formatSize;
		size_t texturePosOffset = (position->y/blockY*mipXBlocks + position->x/blockX)*formatSize;

		size_t bufferXBlocks = (bufferWidth + blockX - 1)/blockX;
		size_t bufferYBlocks = (bufferHeight + blockY - 1)/blockY;
		size_t bufferStride = bufferXBlocks*formatSize;

		size_t bufferLayerStride = bufferXBlocks*bufferYBlocks*formatSize;
		for (uint32_t i = 0; i < region->layers; ++i)
		{
			uint8_t* textureData = mockDstTexture->data +
				dsTexture_layerOffset(info, layerOffset + i, position->mipLevel) + texturePosOffset;
			const uint8_t* bufferData = mockSrcBuffer->data + region->bufferOffset +
				bufferLayerStride*i;
			for (uint32_t y = 0; y < bufferYBlocks;
				++y, textureData += textureStride, bufferData += bufferStride)
			{
				memcpy(textureData, bufferData, rowSize);
			}
		}
	}

	return true;
}

bool dsMockGfxBuffer_destroy(dsResourceManager* resourceManager, dsGfxBuffer* buffer)
{
	DS_UNUSED(resourceManager);
	if (buffer->allocator)
		return dsAllocator_free(buffer->allocator, buffer);
	return true;
}

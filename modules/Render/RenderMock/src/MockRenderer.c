/*
 * Copyright 2016 Aaron Barany
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

#include <DeepSea/RenderMock/MockRenderer.h>

#include "Resources/MockResourceManager.h"
#include "Resources/MockTexture.h"
#include "MockCommandBuffer.h"
#include "MockCommandBufferPool.h"
#include "MockRenderPass.h"
#include "MockRenderSurface.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Renderer.h>

#include <string.h>

bool dsMockRenderer_destroy(dsRenderer* renderer)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);

	DS_VERIFY(dsRenderer_shutdownResources(renderer));

	dsMockResourceManager_destroy(renderer->resourceManager);
	dsRenderer_shutdown(renderer);
	if (renderer->allocator)
		return dsAllocator_free(renderer->allocator, renderer);
	return true;
}

bool dsMockRenderer_beginFrame(dsRenderer* renderer)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);

	return true;
}

bool dsMockRenderer_endFrame(dsRenderer* renderer)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);

	return true;
}

bool dsMockRenderer_setSurfaceSamples(dsRenderer* renderer, uint32_t samples)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_UNUSED(samples);

	return true;
}

bool dsMockRenderer_setVsync(dsRenderer* renderer, bool vsync)
{
	DS_ASSERT(renderer);

	renderer->vsync = vsync;
	return true;
}

bool dsMockRenderer_setDefaultAnisotropy(dsRenderer* renderer, float anisotropy)
{
	DS_ASSERT(renderer);

	renderer->defaultAnisotropy = anisotropy;
	return true;
}

bool dsMockRenderer_clearColorSurface(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsFramebufferSurface* surface, const dsSurfaceColorValue* colorValue)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(surface);
	DS_UNUSED(surface);
	DS_ASSERT(colorValue);
	DS_UNUSED(colorValue);

	return true;
}

bool dsMockRenderer_clearDepthStencilSurface(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsFramebufferSurface* surface, dsClearDepthStencil surfaceParts,
	const dsDepthStencilValue* depthStencilValue)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(surface);
	DS_UNUSED(surface);
	DS_UNUSED(surfaceParts);
	DS_ASSERT(depthStencilValue);
	DS_UNUSED(depthStencilValue);

	return true;
}

bool dsMockRenderer_draw(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawRange* drawRange)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(geometry);
	DS_UNUSED(geometry);
	DS_ASSERT(drawRange);
	DS_UNUSED(drawRange);

	return true;
}

bool dsMockRenderer_drawIndexed(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawIndexedRange* drawRange)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(geometry);
	DS_UNUSED(geometry);
	DS_ASSERT(drawRange);
	DS_UNUSED(drawRange);

	return true;
}

bool dsMockRenderer_drawIndirect(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(geometry);
	DS_UNUSED(geometry);
	DS_ASSERT(indirectBuffer);
	DS_UNUSED(indirectBuffer);
	DS_UNUSED(offset);
	DS_UNUSED(count);
	DS_UNUSED(stride);

	return true;
}

bool dsMockRenderer_drawIndexedIndirect(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(geometry);
	DS_UNUSED(geometry);
	DS_ASSERT(indirectBuffer);
	DS_UNUSED(indirectBuffer);
	DS_UNUSED(offset);
	DS_UNUSED(count);
	DS_UNUSED(stride);

	return true;
}

bool dsMockRenderer_dispatchCompute(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	uint32_t x, uint32_t y, uint32_t z)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(x);
	DS_UNUSED(y);
	DS_UNUSED(z);

	return true;
}

bool dsMockRenderer_dispatchComputeIndirect(dsRenderer* renderer,
	dsCommandBuffer* commandBuffer, const dsGfxBuffer* indirectBuffer, size_t offset)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	DS_ASSERT(commandBuffer);
	DS_UNUSED(commandBuffer);
	DS_ASSERT(indirectBuffer);
	DS_UNUSED(indirectBuffer);
	DS_UNUSED(offset);

	return true;
}

bool dsMockRenderer_blitSurface(dsRenderer* renderer, dsCommandBuffer* commandBuffer,
	dsGfxSurfaceType srcSurfaceType, void* srcSurface, dsGfxSurfaceType dstSurfaceType,
	void* dstSurface, const dsSurfaceBlitRegion* regions, size_t regionCount, dsBlitFilter filter)
{
	DS_ASSERT(renderer);
	DS_ASSERT(srcSurface);
	DS_ASSERT(dstSurface);
	DS_ASSERT(regions);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(filter);

	if (srcSurfaceType != dsGfxSurfaceType_Texture || dstSurfaceType != dsGfxSurfaceType_Texture)
	{
		errno = EPERM;
		DS_LOG_ERROR("render-mock",
			"Mock render implementation requires blitted surfaces to be textures.");
		return false;
	}

	dsTexture* srcTexture = (dsTexture*)srcSurface;
	dsTexture* dstTexture = (dsTexture*)dstSurface;
	if (srcTexture->info.format != dstTexture->info.format)
	{
		errno = EPERM;
		DS_LOG_ERROR("render-mock", "Mock render implementation requires textures to have the same "
			"format when blitting.");
		return false;
	}

	for (size_t i = 0; i < regionCount; ++i)
	{
		if (regions[i].srcWidth != regions[i].dstWidth ||
			regions[i].srcHeight != regions[i].dstHeight)
		{
			errno = EPERM;
			DS_LOG_ERROR("render-mock", "Mock render implementation requires texture regions to "
				" have the same source and destination dimensions when blitting.");
			return false;
		}
	}

	DS_ASSERT(srcTexture->info.format == dstTexture->info.format);
	unsigned int blockX, blockY, minX, minY;
	DS_VERIFY(dsGfxFormat_blockDimensions(&blockX, &blockY, srcTexture->info.format));
	DS_VERIFY(dsGfxFormat_minDimensions(&minX, &minY, srcTexture->info.format));
	unsigned int blockSize = dsGfxFormat_size(srcTexture->info.format);
	DS_ASSERT(blockSize > 0);

	for (size_t i = 0; i < regionCount; ++i)
	{
		DS_ASSERT(regions[i].srcPosition.x % blockX == 0 && regions[i].srcPosition.y % blockY == 0);
		uint32_t srcPosBlockX = regions[i].srcPosition.x/blockX;
		uint32_t srcPosBlockY = regions[i].srcPosition.y/blockY;
		uint32_t srcPosLayer = regions[i].srcPosition.depth;
		if (srcTexture->info.dimension == dsTextureDim_Cube)
			srcPosLayer = srcPosLayer*6 + regions[i].srcPosition.face;
		uint32_t srcMipWidth = srcTexture->info.width >> regions[i].srcPosition.mipLevel;
		uint32_t srcPitch = (dsMax(srcMipWidth, minX) + blockX - 1)/blockX*blockSize;

		DS_ASSERT(regions[i].dstPosition.x % blockX == 0 && regions[i].dstPosition.y % blockY == 0);
		uint32_t dstPosBlockX = regions[i].dstPosition.x/blockX;
		uint32_t dstPosBlockY = regions[i].dstPosition.y/blockY;
		uint32_t dstPosLayer = regions[i].dstPosition.depth;
		if (srcTexture->info.dimension == dsTextureDim_Cube)
			dstPosLayer = dstPosLayer*6 + regions[i].dstPosition.face;
		uint32_t dstMipWidth = dstTexture->info.width >> regions[i].dstPosition.mipLevel;
		uint32_t dstPitch = (dsMax(dstMipWidth, minX) + blockX - 1)/blockX*blockSize;

		uint32_t copySize = (regions[i].srcWidth + blockX - 1)/blockX*blockSize;
		uint32_t blockHeight = (regions[i].srcHeight + blockY - 1)/blockY;
		for (uint32_t j = 0; j < regions[i].layers; ++j)
		{
			size_t srcOffset = dsTexture_layerOffset(&srcTexture->info, srcPosLayer + j,
				regions[i].srcPosition.mipLevel);
			srcOffset += srcPosBlockY*srcPitch + srcPosBlockX*blockSize;

			size_t dstOffset = dsTexture_layerOffset(&dstTexture->info, dstPosLayer + j,
				regions[i].dstPosition.mipLevel);
			dstOffset += dstPosBlockY*dstPitch + dstPosBlockX*blockSize;

			for (uint32_t y = 0; y < blockHeight; ++y, srcOffset += srcPitch, dstOffset += dstPitch)
			{
				DS_ASSERT(srcOffset + copySize <= ((dsMockTexture*)srcTexture)->dataSize);
				DS_ASSERT(dstOffset + copySize <= ((dsMockTexture*)dstTexture)->dataSize);
				memcpy(((dsMockTexture*)dstTexture)->data + dstOffset,
					((dsMockTexture*)srcTexture)->data + srcOffset, copySize);
			}
		}
	}

	return true;
}

bool dsMockRenderer_waitUntilIdle(dsRenderer* renderer)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	return true;
}

bool dsMockRenderer_restoreGlobalState(dsRenderer* renderer)
{
	DS_ASSERT(renderer);
	DS_UNUSED(renderer);
	return true;
}

bool dsMockRenderer_isSupported(void)
{
	return true;
}

bool dsMockRenderer_queryDevices(dsRenderDeviceInfo* outDevices, uint32_t* outDeviceCount)
{
	DS_UNUSED(outDevices);
	if (!outDeviceCount)
	{
		errno = EINVAL;
		return false;
	}

	*outDeviceCount = 0;
	return true;
}

dsRenderer* dsMockRenderer_create(dsAllocator* allocator)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t totalSize = DS_ALIGNED_SIZE(sizeof(dsRenderer)) +
		DS_ALIGNED_SIZE(sizeof(dsCommandBuffer));
	void* buffer = dsAllocator_alloc(allocator, totalSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAllocator;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAllocator, buffer, totalSize));
	dsRenderer* renderer = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAllocator, dsRenderer);
	DS_ASSERT(renderer);

	if (!dsRenderer_initialize(renderer))
	{
		if (allocator->freeFunc)
			dsAllocator_free(allocator, renderer);
		return NULL;
	}

	dsResourceManager* resourceManager = dsMockResourceManager_create(renderer, allocator);
	if (!resourceManager)
	{
		if (allocator->freeFunc)
			dsAllocator_free(allocator, renderer);
		return NULL;
	}

	renderer->allocator = dsAllocator_keepPointer(allocator);
	renderer->resourceManager = resourceManager;
	renderer->rendererID = DS_MOCK_RENDERER_ID;
	renderer->platformID = DS_MOCK_RENDERER_ID;
	renderer->name = "Mock";
	renderer->shaderLanguage = "spirv";
	renderer->shaderVersion = DS_ENCODE_VERSION(1, 0, 0);
	renderer->driverName = "None";

	renderer->mainCommandBuffer = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAllocator,
		dsCommandBuffer);
	DS_ASSERT(renderer->mainCommandBuffer);
	renderer->mainCommandBuffer->renderer = renderer;
	renderer->mainCommandBuffer->usage = dsCommandBufferUsage_Standard;
	renderer->mainCommandBuffer->frameActive = false;
	renderer->mainCommandBuffer->boundSurface = NULL;
	renderer->mainCommandBuffer->boundFramebuffer = NULL;
	renderer->mainCommandBuffer->boundRenderPass = NULL;
	renderer->mainCommandBuffer->activeRenderSubpass = 0;
	renderer->mainCommandBuffer->indirectCommands = false;
	renderer->mainCommandBuffer->boundShader = NULL;
	renderer->mainCommandBuffer->boundComputeShader = NULL;

	renderer->maxColorAttachments = 4;
	renderer->maxSurfaceSamples = 16;
	renderer->maxAnisotropy = 16;

	renderer->surfaceColorFormat = dsGfxFormat_decorate(dsGfxFormat_R8G8B8, dsGfxFormat_UNorm);
	renderer->surfaceDepthStencilFormat = dsGfxFormat_D24S8;
	renderer->surfaceSamples = 4;
	renderer->doubleBuffer = true;
	renderer->stereoscopic = false;
	renderer->vsync = true;
	renderer->clipHalfDepth = true;
	renderer->clipInvertY = false;
	renderer->hasGeometryShaders = true;
	renderer->hasTessellationShaders = true;
	renderer->hasComputeShaders = true;
	renderer->hasNativeMultidraw = true;
	renderer->supportsInstancedDrawing = true;
	renderer->supportsStartInstance = true;

	renderer->destroyFunc = &dsMockRenderer_destroy;

	renderer->createRenderSurfaceFunc = &dsMockRenderSurface_create;
	renderer->destroyRenderSurfaceFunc = &dsMockRenderSurface_destroy;
	renderer->updateRenderSurfaceFunc = &dsMockRenderSurface_update;
	renderer->beginRenderSurfaceFunc = &dsMockRenderSurface_beginDraw;
	renderer->endRenderSurfaceFunc = &dsMockRenderSurface_endDraw;
	renderer->swapRenderSurfaceBuffersFunc = &dsMockRenderSurface_swapBuffers;

	renderer->createCommandBufferPoolFunc = &dsMockCommandBufferPool_create;
	renderer->resetCommandBufferPoolFunc = &dsMockCommandBufferPool_reset;
	renderer->destroyCommandBufferPoolFunc = &dsMockCommandBufferPool_destroy;

	renderer->beginCommandBufferFunc = &dsMockCommandBuffer_begin;
	renderer->endCommandBufferFunc = &dsMockCommandBuffer_end;
	renderer->submitCommandBufferFunc = &dsMockCommandBuffer_submit;

	renderer->createRenderPassFunc = &dsMockRenderPass_create;
	renderer->destroyRenderPassFunc = &dsMockRenderPass_destroy;
	renderer->beginRenderPassFunc = &dsMockRenderPass_begin;
	renderer->nextRenderSubpassFunc = &dsMockRenderPass_nextSubpass;
	renderer->endRenderPassFunc = &dsMockRenderPass_end;

	renderer->beginFrameFunc = &dsMockRenderer_beginFrame;
	renderer->endFrameFunc = &dsMockRenderer_endFrame;
	renderer->setSurfaceSamplesFunc = &dsMockRenderer_setSurfaceSamples;
	renderer->setVsyncFunc = &dsMockRenderer_setVsync;
	renderer->setDefaultAnisotropyFunc = &dsMockRenderer_setDefaultAnisotropy;
	renderer->clearColorSurfaceFunc = &dsMockRenderer_clearColorSurface;
	renderer->clearDepthStencilSurfaceFunc = &dsMockRenderer_clearDepthStencilSurface;
	renderer->drawFunc = &dsMockRenderer_draw;
	renderer->drawIndexedFunc = &dsMockRenderer_drawIndexed;
	renderer->drawIndirectFunc = &dsMockRenderer_drawIndirect;
	renderer->drawIndexedIndirectFunc = &dsMockRenderer_drawIndexedIndirect;
	renderer->dispatchComputeFunc = &dsMockRenderer_dispatchCompute;
	renderer->dispatchComputeIndirectFunc = &dsMockRenderer_dispatchComputeIndirect;
	renderer->blitSurfaceFunc = &dsMockRenderer_blitSurface;
	renderer->waitUntilIdleFunc = &dsMockRenderer_waitUntilIdle;
	renderer->restoreGlobalStateFunc = &dsMockRenderer_restoreGlobalState;

	DS_VERIFY(dsRenderer_initializeResources(renderer));

	return renderer;
}

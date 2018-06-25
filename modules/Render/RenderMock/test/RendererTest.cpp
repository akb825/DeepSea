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
#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Renderbuffer.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Render/RenderSurface.h>
#include <gtest/gtest.h>

class RendererTest : public FixtureBase
{
};

TEST_F(RendererTest, BeginEndFrame)
{
	EXPECT_FALSE(dsRenderer_beginFrame(nullptr));
	EXPECT_TRUE(dsRenderer_beginFrame(renderer));
	EXPECT_FALSE(dsRenderer_endFrame(nullptr));
	EXPECT_TRUE(dsRenderer_endFrame(renderer));
}

TEST_F(RendererTest, SetSurfaceSamples)
{
	EXPECT_FALSE(dsRenderer_setSurfaceSamples(NULL, 1));
	EXPECT_FALSE(dsRenderer_setSurfaceSamples(renderer, renderer->maxSurfaceSamples + 1));
	EXPECT_TRUE(dsRenderer_setSurfaceSamples(renderer, renderer->maxSurfaceSamples));
}

TEST_F(RendererTest, SetVsync)
{
	EXPECT_FALSE(dsRenderer_setVsync(NULL, false));
	EXPECT_TRUE(dsRenderer_setVsync(renderer, false));
}

TEST_F(RendererTest, SetDefaultAnisotropy)
{
	EXPECT_FALSE(dsRenderer_setDefaultAnisotropy(NULL, 4.0f));
	EXPECT_FALSE(dsRenderer_setDefaultAnisotropy(renderer, renderer->maxAnisotropy + 1));
	EXPECT_TRUE(dsRenderer_setDefaultAnisotropy(renderer, renderer->maxAnisotropy));
}

TEST_F(RendererTest, ClearColorSurface)
{
	dsTextureInfo colorInfo = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
		dsGfxFormat_UNorm), dsTextureDim_2D, 1920, 1080, 0, 1, 4};
	dsOffscreen* offscreen1 = dsTexture_createOffscreen(resourceManager, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Static, &colorInfo, true);
	ASSERT_TRUE(offscreen1);

	dsTextureInfo depthInfo = {dsGfxFormat_D24S8, dsTextureDim_2D, 1920, 1080, 0, 1, 4};
	dsOffscreen* offscreen2 = dsTexture_createOffscreen(resourceManager, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Static, &depthInfo, true);
	ASSERT_TRUE(offscreen2);

	dsRenderbuffer* colorBuffer = dsRenderbuffer_create(resourceManager, NULL,
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		1920, 1080, 4);
	ASSERT_TRUE(colorBuffer);

	dsRenderbuffer* depthBuffer = dsRenderbuffer_create(resourceManager, NULL, dsGfxFormat_D24S8,
		1920, 1080, 4);
	ASSERT_TRUE(depthBuffer);

	dsRenderSurface* renderSurface = dsRenderSurface_create(renderer, NULL, NULL,
		dsRenderSurfaceType_Direct);
	ASSERT_TRUE(renderSurface);

	dsSurfaceColorValue colorValue;
	colorValue.floatValue.r = 0.0f;
	colorValue.floatValue.g = 0.0f;
	colorValue.floatValue.b = 0.0f;
	colorValue.floatValue.a = 1.0f;

	dsFramebufferSurface surface = {dsGfxSurfaceType_Texture, dsCubeFace_None, 0, 0,
		offscreen1};
	EXPECT_FALSE(dsRenderer_clearColorSurface(NULL, renderer->mainCommandBuffer, &surface,
		&colorValue));
	EXPECT_FALSE(dsRenderer_clearColorSurface(renderer, NULL, &surface, &colorValue));
	EXPECT_FALSE(dsRenderer_clearColorSurface(renderer, renderer->mainCommandBuffer, NULL,
		&colorValue));
	EXPECT_FALSE(dsRenderer_clearColorSurface(renderer, renderer->mainCommandBuffer, &surface,
		NULL));

	surface.layer = 2;
	EXPECT_FALSE(dsRenderer_clearColorSurface(renderer, renderer->mainCommandBuffer, &surface,
		&colorValue));

	surface.layer = 0;
	surface.mipLevel = 2;
	EXPECT_FALSE(dsRenderer_clearColorSurface(renderer, renderer->mainCommandBuffer, &surface,
		&colorValue));

	surface.mipLevel = 0;
	EXPECT_TRUE(dsRenderer_clearColorSurface(renderer, renderer->mainCommandBuffer, &surface,
		&colorValue));

	surface.surface = offscreen2;
	EXPECT_FALSE(dsRenderer_clearColorSurface(renderer, renderer->mainCommandBuffer, &surface,
		&colorValue));

	surface.surfaceType = dsGfxSurfaceType_Renderbuffer;
	surface.surface = colorBuffer;
	EXPECT_TRUE(dsRenderer_clearColorSurface(renderer, renderer->mainCommandBuffer, &surface,
		&colorValue));

	surface.surface = depthBuffer;
	EXPECT_FALSE(dsRenderer_clearColorSurface(renderer, renderer->mainCommandBuffer, &surface,
		&colorValue));

	surface.surfaceType = dsGfxSurfaceType_ColorRenderSurface;
	surface.surface = renderSurface;
	EXPECT_TRUE(dsRenderer_clearColorSurface(renderer, renderer->mainCommandBuffer, &surface,
		&colorValue));

	surface.surfaceType = dsGfxSurfaceType_DepthRenderSurface;
	EXPECT_FALSE(dsRenderer_clearColorSurface(renderer, renderer->mainCommandBuffer, &surface,
		&colorValue));

	EXPECT_TRUE(dsRenderSurface_destroy(renderSurface));
	EXPECT_TRUE(dsRenderbuffer_destroy(depthBuffer));
	EXPECT_TRUE(dsRenderbuffer_destroy(colorBuffer));
	EXPECT_TRUE(dsTexture_destroy(offscreen1));
	EXPECT_TRUE(dsTexture_destroy(offscreen2));
}

TEST_F(RendererTest, ClearDepthStencilSurface)
{
	dsTextureInfo depthInfo = {dsGfxFormat_D24S8, dsTextureDim_2D, 1920, 1080, 0, 1, 4};
	dsOffscreen* offscreen1 = dsTexture_createOffscreen(resourceManager, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Static, &depthInfo, true);
	ASSERT_TRUE(offscreen1);

	dsTextureInfo colorInfo = {dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		dsTextureDim_2D, 1920, 1080, 0, 1, 4};
	dsOffscreen* offscreen2 = dsTexture_createOffscreen(resourceManager, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Static, &colorInfo, true);
	ASSERT_TRUE(offscreen2);

	dsRenderbuffer* colorBuffer = dsRenderbuffer_create(resourceManager, NULL,
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm),
		1920, 1080, 4);
	ASSERT_TRUE(colorBuffer);

	dsRenderbuffer* depthBuffer = dsRenderbuffer_create(resourceManager, NULL, dsGfxFormat_D24S8,
		1920, 1080, 4);
	ASSERT_TRUE(depthBuffer);

	dsRenderSurface* renderSurface = dsRenderSurface_create(renderer, NULL, NULL,
		dsRenderSurfaceType_Direct);
	ASSERT_TRUE(renderSurface);

	dsDepthStencilValue depthStencilValue = {1.0f, 0};
	dsFramebufferSurface surface = {dsGfxSurfaceType_Texture, dsCubeFace_None, 0, 0, offscreen1};
	EXPECT_FALSE(dsRenderer_clearDepthStencilSurface(NULL, renderer->mainCommandBuffer, &surface,
		dsClearDepthStencil_Both, &depthStencilValue));
	EXPECT_FALSE(dsRenderer_clearDepthStencilSurface(renderer, NULL, &surface,
		dsClearDepthStencil_Both, &depthStencilValue));
	EXPECT_FALSE(dsRenderer_clearDepthStencilSurface(renderer, renderer->mainCommandBuffer, NULL,
		dsClearDepthStencil_Both, &depthStencilValue));
	EXPECT_FALSE(dsRenderer_clearDepthStencilSurface(renderer, renderer->mainCommandBuffer,
		&surface, dsClearDepthStencil_Both, NULL));

	surface.layer = 2;
	EXPECT_FALSE(dsRenderer_clearDepthStencilSurface(renderer, renderer->mainCommandBuffer,
		&surface, dsClearDepthStencil_Both, &depthStencilValue));

	surface.layer = 0;
	surface.mipLevel = 2;
	EXPECT_FALSE(dsRenderer_clearDepthStencilSurface(renderer, renderer->mainCommandBuffer,
		&surface, dsClearDepthStencil_Both, &depthStencilValue));

	surface.mipLevel = 0;
	EXPECT_TRUE(dsRenderer_clearDepthStencilSurface(renderer, renderer->mainCommandBuffer, &surface,
		dsClearDepthStencil_Both, &depthStencilValue));

	surface.surface = offscreen2;
	EXPECT_FALSE(dsRenderer_clearDepthStencilSurface(renderer, renderer->mainCommandBuffer,
		&surface, dsClearDepthStencil_Both, &depthStencilValue));

	surface.surfaceType = dsGfxSurfaceType_Renderbuffer;
	surface.surface = colorBuffer;
	EXPECT_FALSE(dsRenderer_clearDepthStencilSurface(renderer, renderer->mainCommandBuffer,
		&surface, dsClearDepthStencil_Both, &depthStencilValue));

	surface.surface = depthBuffer;
	EXPECT_TRUE(dsRenderer_clearDepthStencilSurface(renderer, renderer->mainCommandBuffer, &surface,
		dsClearDepthStencil_Both, &depthStencilValue));

	surface.surfaceType = dsGfxSurfaceType_ColorRenderSurface;
	surface.surface = renderSurface;
	EXPECT_FALSE(dsRenderer_clearDepthStencilSurface(renderer, renderer->mainCommandBuffer,
		&surface, dsClearDepthStencil_Both, &depthStencilValue));

	surface.surfaceType = dsGfxSurfaceType_DepthRenderSurface;
	EXPECT_TRUE(dsRenderer_clearDepthStencilSurface(renderer, renderer->mainCommandBuffer, &surface,
		dsClearDepthStencil_Both, &depthStencilValue));

	EXPECT_TRUE(dsRenderSurface_destroy(renderSurface));
	EXPECT_TRUE(dsRenderbuffer_destroy(depthBuffer));
	EXPECT_TRUE(dsRenderbuffer_destroy(colorBuffer));
	EXPECT_TRUE(dsTexture_destroy(offscreen1));
	EXPECT_TRUE(dsTexture_destroy(offscreen2));
}

TEST_F(RendererTest, Draw)
{
	dsGfxBuffer* vertexGfxBuffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Vertex, dsGfxMemory_Static | dsGfxMemory_Draw, NULL, 1024);
	ASSERT_TRUE(vertexGfxBuffer);

	dsVertexBuffer vertexBuffer = {};
	vertexBuffer.buffer = vertexGfxBuffer;
	vertexBuffer.offset = 0;
	vertexBuffer.count = 10;

	EXPECT_TRUE(dsVertexFormat_setAttribEnabled(&vertexBuffer.format, dsVertexAttrib_Position,
		true));
	vertexBuffer.format.elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
	EXPECT_TRUE(dsVertexFormat_computeOffsetsAndSize(&vertexBuffer.format));

	dsVertexBuffer* vertexBufferArray[DS_MAX_GEOMETRY_VERTEX_BUFFERS] = {};
	vertexBufferArray[0] = &vertexBuffer;

	dsDrawGeometry* geometry = dsDrawGeometry_create(resourceManager, NULL, vertexBufferArray,
		NULL);
	ASSERT_TRUE(geometry);

	dsDrawRange drawRange = {10, 1, 0, 0};
	EXPECT_FALSE(dsRenderer_draw(NULL, renderer->mainCommandBuffer, geometry, &drawRange));
	EXPECT_FALSE(dsRenderer_draw(renderer, NULL, geometry, &drawRange));
	EXPECT_FALSE(dsRenderer_draw(renderer, renderer->mainCommandBuffer, NULL, &drawRange));
	EXPECT_FALSE(dsRenderer_draw(renderer, renderer->mainCommandBuffer, geometry, NULL));

	EXPECT_TRUE(dsRenderer_draw(renderer, renderer->mainCommandBuffer, geometry, &drawRange));

	drawRange.firstVertex = 4;
	EXPECT_FALSE(dsRenderer_draw(renderer, renderer->mainCommandBuffer, geometry, &drawRange));

	drawRange.firstVertex = 0;
	drawRange.instanceCount = 10;
	EXPECT_TRUE(dsRenderer_draw(renderer, renderer->mainCommandBuffer, geometry, &drawRange));

	renderer->supportsInstancedDrawing = false;
	EXPECT_FALSE(dsRenderer_draw(renderer, renderer->mainCommandBuffer, geometry, &drawRange));

	EXPECT_TRUE(dsDrawGeometry_destroy(geometry));
	EXPECT_TRUE(dsGfxBuffer_destroy(vertexGfxBuffer));
}

TEST_F(RendererTest, DrawIndexed)
{
	dsGfxBuffer* vertexGfxBuffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Vertex, dsGfxMemory_Static | dsGfxMemory_Draw, NULL, 1024);
	ASSERT_TRUE(vertexGfxBuffer);

	dsGfxBuffer* indexGfxBuffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Index, dsGfxMemory_Static | dsGfxMemory_Draw, NULL, 1024);
	ASSERT_TRUE(indexGfxBuffer);

	dsVertexBuffer vertexBuffer = {};
	vertexBuffer.buffer = vertexGfxBuffer;
	vertexBuffer.offset = 0;
	vertexBuffer.count = 10;

	EXPECT_TRUE(dsVertexFormat_setAttribEnabled(&vertexBuffer.format, dsVertexAttrib_Position,
		true));
	vertexBuffer.format.elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
	EXPECT_TRUE(dsVertexFormat_computeOffsetsAndSize(&vertexBuffer.format));

	dsIndexBuffer indexBuffer = {indexGfxBuffer, 0, 16, (uint32_t)sizeof(uint16_t)};

	dsVertexBuffer* vertexBufferArray[DS_MAX_GEOMETRY_VERTEX_BUFFERS] = {};
	vertexBufferArray[0] = &vertexBuffer;

	dsDrawGeometry* geometry1 = dsDrawGeometry_create(resourceManager, NULL, vertexBufferArray,
		&indexBuffer);
	ASSERT_TRUE(geometry1);

	dsDrawGeometry* geometry2 = dsDrawGeometry_create(resourceManager, NULL, vertexBufferArray,
		NULL);
	ASSERT_TRUE(geometry2);

	dsDrawIndexedRange drawRange = {16, 1, 0, 0, 0};
	EXPECT_FALSE(dsRenderer_drawIndexed(NULL, renderer->mainCommandBuffer, geometry1, &drawRange));
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer, NULL, geometry1, &drawRange));
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer, renderer->mainCommandBuffer, NULL, &drawRange));
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer, renderer->mainCommandBuffer, geometry1, NULL));

	EXPECT_TRUE(dsRenderer_drawIndexed(renderer, renderer->mainCommandBuffer, geometry1,
		&drawRange));
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer, renderer->mainCommandBuffer, geometry2,
		&drawRange));

	drawRange.firstIndex = 4;
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer, renderer->mainCommandBuffer, geometry1,
		&drawRange));

	drawRange.firstIndex = 0;
	drawRange.instanceCount = 10;
	EXPECT_TRUE(dsRenderer_drawIndexed(renderer, renderer->mainCommandBuffer, geometry1,
		&drawRange));

	renderer->supportsInstancedDrawing = false;
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer, renderer->mainCommandBuffer, geometry1,
		&drawRange));

	EXPECT_TRUE(dsDrawGeometry_destroy(geometry1));
	EXPECT_TRUE(dsDrawGeometry_destroy(geometry2));
	EXPECT_TRUE(dsGfxBuffer_destroy(vertexGfxBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(indexGfxBuffer));
}

TEST_F(RendererTest, DrawIndirect)
{
	dsGfxBuffer* vertexGfxBuffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Vertex, dsGfxMemory_Static | dsGfxMemory_Draw, NULL, 1024);
	ASSERT_TRUE(vertexGfxBuffer);

	dsGfxBuffer* indirectBuffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_IndirectDraw, dsGfxMemory_Static | dsGfxMemory_Draw, NULL,
		sizeof(dsDrawRange)*4);
	ASSERT_TRUE(indirectBuffer);

	dsVertexBuffer vertexBuffer = {};
	vertexBuffer.buffer = vertexGfxBuffer;
	vertexBuffer.offset = 0;
	vertexBuffer.count = 10;

	EXPECT_TRUE(dsVertexFormat_setAttribEnabled(&vertexBuffer.format, dsVertexAttrib_Position,
		true));
	vertexBuffer.format.elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
	EXPECT_TRUE(dsVertexFormat_computeOffsetsAndSize(&vertexBuffer.format));

	dsVertexBuffer* vertexBufferArray[DS_MAX_GEOMETRY_VERTEX_BUFFERS] = {};
	vertexBufferArray[0] = &vertexBuffer;

	dsDrawGeometry* geometry = dsDrawGeometry_create(resourceManager, NULL, vertexBufferArray,
		NULL);
	ASSERT_TRUE(geometry);

	EXPECT_FALSE(dsRenderer_drawIndirect(NULL, renderer->mainCommandBuffer, geometry,
		indirectBuffer, 0, 4, sizeof(dsDrawRange)));
	EXPECT_FALSE(dsRenderer_drawIndirect(renderer, NULL, geometry, indirectBuffer, 0, 4,
		sizeof(dsDrawRange)));
	EXPECT_FALSE(dsRenderer_drawIndirect(renderer, renderer->mainCommandBuffer, NULL,
		indirectBuffer, 0, 4, sizeof(dsDrawRange)));
	EXPECT_FALSE(dsRenderer_drawIndirect(renderer, renderer->mainCommandBuffer, geometry, NULL, 0,
		4, sizeof(dsDrawRange)));
	EXPECT_FALSE(dsRenderer_drawIndirect(renderer, renderer->mainCommandBuffer, geometry,
		indirectBuffer, 1, 3, sizeof(dsDrawRange)));
	EXPECT_FALSE(dsRenderer_drawIndirect(renderer, renderer->mainCommandBuffer, geometry,
		indirectBuffer, 0, 5, sizeof(dsDrawRange)));
	EXPECT_FALSE(dsRenderer_drawIndirect(renderer, renderer->mainCommandBuffer, geometry,
		indirectBuffer, 0, 4, 1));

	EXPECT_TRUE(dsRenderer_drawIndirect(renderer, renderer->mainCommandBuffer, geometry,
		indirectBuffer, 0, 4, sizeof(dsDrawRange)));

	EXPECT_TRUE(dsDrawGeometry_destroy(geometry));
	EXPECT_TRUE(dsGfxBuffer_destroy(vertexGfxBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(indirectBuffer));
}

TEST_F(RendererTest, DrawIndexedIndirect)
{
	dsGfxBuffer* vertexGfxBuffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Vertex, dsGfxMemory_Static | dsGfxMemory_Draw, NULL, 1024);
	ASSERT_TRUE(vertexGfxBuffer);

	dsGfxBuffer* indexGfxBuffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Index, dsGfxMemory_Static | dsGfxMemory_Draw, NULL, 1024);
	ASSERT_TRUE(indexGfxBuffer);

	dsGfxBuffer* indirectBuffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_IndirectDraw, dsGfxMemory_Static | dsGfxMemory_Draw, NULL,
		sizeof(dsDrawIndexedRange)*4);
	ASSERT_TRUE(indirectBuffer);

	dsVertexBuffer vertexBuffer = {};
	vertexBuffer.buffer = vertexGfxBuffer;
	vertexBuffer.offset = 0;
	vertexBuffer.count = 10;

	EXPECT_TRUE(dsVertexFormat_setAttribEnabled(&vertexBuffer.format, dsVertexAttrib_Position,
		true));
	vertexBuffer.format.elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
	EXPECT_TRUE(dsVertexFormat_computeOffsetsAndSize(&vertexBuffer.format));

	dsIndexBuffer indexBuffer = {indexGfxBuffer, 0, 16, (uint32_t)sizeof(uint16_t)};

	dsVertexBuffer* vertexBufferArray[DS_MAX_GEOMETRY_VERTEX_BUFFERS] = {};
	vertexBufferArray[0] = &vertexBuffer;

	dsDrawGeometry* geometry1 = dsDrawGeometry_create(resourceManager, NULL, vertexBufferArray,
		&indexBuffer);
	ASSERT_TRUE(geometry1);

	dsDrawGeometry* geometry2 = dsDrawGeometry_create(resourceManager, NULL, vertexBufferArray,
		NULL);
	ASSERT_TRUE(geometry2);

	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(NULL, renderer->mainCommandBuffer, geometry1,
		indirectBuffer, 0, 4, sizeof(dsDrawRange)));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer, NULL, geometry1, indirectBuffer, 0, 4,
		sizeof(dsDrawIndexedRange)));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer, renderer->mainCommandBuffer, NULL,
		indirectBuffer, 0, 4, sizeof(dsDrawIndexedRange)));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer, renderer->mainCommandBuffer, geometry1,
		NULL, 0, 4, sizeof(dsDrawIndexedRange)));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer, renderer->mainCommandBuffer, geometry1,
		indirectBuffer, 1, 3, sizeof(dsDrawIndexedRange)));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer, renderer->mainCommandBuffer, geometry1,
		indirectBuffer, 0, 5, sizeof(dsDrawIndexedRange)));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer, renderer->mainCommandBuffer, geometry1,
		indirectBuffer, 0, 4, 1));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer, renderer->mainCommandBuffer, geometry2,
		indirectBuffer, 0, 4, sizeof(dsDrawIndexedRange)));

	EXPECT_TRUE(dsRenderer_drawIndexedIndirect(renderer, renderer->mainCommandBuffer, geometry1,
		indirectBuffer, 0, 4, sizeof(dsDrawIndexedRange)));

	EXPECT_TRUE(dsDrawGeometry_destroy(geometry1));
	EXPECT_TRUE(dsDrawGeometry_destroy(geometry2));
	EXPECT_TRUE(dsGfxBuffer_destroy(vertexGfxBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(indexGfxBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(indirectBuffer));
}

TEST_F(RendererTest, DispatchCompute)
{
	EXPECT_FALSE(dsRenderer_dispatchCompute(NULL, renderer->mainCommandBuffer, 1, 1, 1));
	EXPECT_FALSE(dsRenderer_dispatchCompute(renderer, NULL, 1, 1, 1));

	EXPECT_TRUE(dsRenderer_dispatchCompute(renderer, renderer->mainCommandBuffer, 1, 1, 1));
	renderer->hasComputeShaders = false;
	EXPECT_FALSE(dsRenderer_dispatchCompute(renderer, renderer->mainCommandBuffer, 1, 1, 1));
}

TEST_F(RendererTest, DispatchComputeIndirect)
{
	dsGfxBuffer* vertexGfxBuffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Vertex, dsGfxMemory_Static | dsGfxMemory_Draw, NULL, 1024);
	ASSERT_TRUE(vertexGfxBuffer);

	dsGfxBuffer* indirectBuffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_IndirectDispatch, dsGfxMemory_Static | dsGfxMemory_Draw, NULL,
		sizeof(uint32_t)*4);
	ASSERT_TRUE(indirectBuffer);

	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(NULL, renderer->mainCommandBuffer,
		indirectBuffer, sizeof(uint32_t)));
	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer, NULL, indirectBuffer,
		sizeof(uint32_t)));
	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer, renderer->mainCommandBuffer,
		NULL, sizeof(uint32_t)));
	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer, renderer->mainCommandBuffer,
		vertexGfxBuffer, sizeof(uint32_t)));
	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer, renderer->mainCommandBuffer,
		indirectBuffer, 1));
	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer, renderer->mainCommandBuffer,
		indirectBuffer, 2*sizeof(uint32_t)));

	EXPECT_TRUE(dsRenderer_dispatchComputeIndirect(renderer, renderer->mainCommandBuffer,
		indirectBuffer, sizeof(uint32_t)));
	renderer->hasComputeShaders = false;
	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer, renderer->mainCommandBuffer,
		indirectBuffer, sizeof(uint32_t)));

	EXPECT_TRUE(dsGfxBuffer_destroy(vertexGfxBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(indirectBuffer));
}

TEST_F(RendererTest, Blit)
{
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
					textureData[index].a = (uint8_t)(depth + 1);
				}
			}
		}
	}

	dsGfxFormat format = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	dsTextureInfo fromInfo = {format, dsTextureDim_2D, 32, 16, 4, 3, 1};
	dsTexture* fromTexture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture,
		dsGfxMemory_Static, &fromInfo, textureData, sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	dsTextureInfo toInfo = {format, dsTextureDim_2D, 16, 32, 5, 2, 1};
	dsTexture* toTexture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo | dsTextureUsage_CopyFrom,
		dsGfxMemory_Static, &toInfo, NULL, 0);
	ASSERT_TRUE(toTexture);

	dsSurfaceBlitRegion blitRegion =
	{
		{dsCubeFace_None, 1, 2, 2, 1},
		{dsCubeFace_None, 3, 4, 1, 0},
		8, 4, 8, 4, 2
	};

	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Texture,
		fromTexture, dsGfxSurfaceType_Texture, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));
	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsTexture_destroy(toTexture));

	fromTexture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyFrom, dsGfxMemory_Static, &fromInfo,
		textureData, sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	toTexture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture, dsGfxMemory_Static,
		&toInfo, NULL, 0);
	ASSERT_TRUE(toTexture);

	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Texture,
		fromTexture, dsGfxSurfaceType_Texture, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));
	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsTexture_destroy(toTexture));

	fromTexture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyFrom, dsGfxMemory_Static, &fromInfo,
		textureData, sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	toTexture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo | dsTextureUsage_CopyFrom,
		dsGfxMemory_Static, &toInfo, NULL, 0);
	ASSERT_TRUE(toTexture);

	EXPECT_TRUE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Texture,
		fromTexture, dsGfxSurfaceType_Texture, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));

	dsColor readTextureData[8*4];
	EXPECT_TRUE(dsTexture_getData(readTextureData, sizeof(readTextureData), toTexture,
		&blitRegion.dstPosition, 8, 4));
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

	blitRegion.dstPosition.depth = 2;
	EXPECT_TRUE(dsTexture_getData(readTextureData, sizeof(readTextureData), toTexture,
		&blitRegion.dstPosition, 8, 4));
	for (unsigned int y = 0, index = 0; y < 4; ++y)
	{
		for (unsigned int x = 0; x < 8; ++x, ++index)
		{
			EXPECT_EQ(x + 1, readTextureData[index].r);
			EXPECT_EQ(y + 2, readTextureData[index].g);
			EXPECT_EQ(1U, readTextureData[index].b);
			EXPECT_EQ(4U, readTextureData[index].a);
		}
	}

	blitRegion.srcPosition.x = 25;
	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Texture,
		fromTexture, dsGfxSurfaceType_Texture, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));

	blitRegion.srcPosition.x = 1;
	blitRegion.srcPosition.y = 13;
	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Texture,
		fromTexture, dsGfxSurfaceType_Texture, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));

	blitRegion.srcPosition.x = 0;
	blitRegion.srcPosition.y = 0;
	blitRegion.srcPosition.mipLevel = 5;
	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Texture,
		fromTexture, dsGfxSurfaceType_Texture, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));

	blitRegion.srcPosition.mipLevel = 0;
	blitRegion.srcPosition.depth = 3;
	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Texture,
		fromTexture, dsGfxSurfaceType_Texture, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));

	blitRegion.srcPosition.depth = 0;
	blitRegion.dstPosition.x = 17;
	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Texture,
		fromTexture, dsGfxSurfaceType_Texture, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));

	blitRegion.dstPosition.x = 3;
	blitRegion.dstPosition.y = 29;
	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Texture,
		fromTexture, dsGfxSurfaceType_Texture, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));

	blitRegion.dstPosition.y = 4;
	blitRegion.dstPosition.mipLevel = 3;
	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Texture,
		fromTexture, dsGfxSurfaceType_Texture, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));

	blitRegion.dstPosition.mipLevel = 0;
	blitRegion.dstPosition.depth = 4;
	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Texture,
		fromTexture, dsGfxSurfaceType_Texture, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));

	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsTexture_destroy(toTexture));
}

TEST_F(RendererTest, WaitUntilIdle)
{
	EXPECT_FALSE(dsRenderer_waitUntilIdle(NULL));
	EXPECT_TRUE(dsRenderer_waitUntilIdle(renderer));
}
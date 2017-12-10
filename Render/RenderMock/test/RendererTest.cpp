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
	dsOffscreen* offscreen1 = dsTexture_createOffscreen(resourceManager, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Static, dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
		dsGfxFormat_UNorm), dsTextureDim_2D, 1920, 1080, 0, 1, 4, true);
	ASSERT_TRUE(offscreen1);

	dsOffscreen* offscreen2 = dsTexture_createOffscreen(resourceManager, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Static, dsGfxFormat_D24S8, dsTextureDim_2D, 1920, 1080,
		0, 1, 4, true);
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

	dsFramebufferSurface surface = {dsFramebufferSurfaceType_Offscreen, dsCubeFace_None, 0, 0,
		offscreen1};
	EXPECT_FALSE(dsRenderer_clearColorSurface(renderer->mainCommandBuffer, NULL, &surface,
		&colorValue));
	EXPECT_FALSE(dsRenderer_clearColorSurface(NULL, renderer, &surface, &colorValue));
	EXPECT_FALSE(dsRenderer_clearColorSurface(renderer->mainCommandBuffer, renderer, NULL,
		&colorValue));
	EXPECT_FALSE(dsRenderer_clearColorSurface(renderer->mainCommandBuffer, renderer, &surface,
		NULL));

	surface.layer = 2;
	EXPECT_FALSE(dsRenderer_clearColorSurface(renderer->mainCommandBuffer, renderer, &surface,
		&colorValue));

	surface.layer = 0;
	surface.mipLevel = 2;
	EXPECT_FALSE(dsRenderer_clearColorSurface(renderer->mainCommandBuffer, renderer, &surface,
		&colorValue));

	surface.mipLevel = 0;
	EXPECT_TRUE(dsRenderer_clearColorSurface(renderer->mainCommandBuffer, renderer, &surface,
		&colorValue));

	surface.surface = offscreen2;
	EXPECT_FALSE(dsRenderer_clearColorSurface(renderer->mainCommandBuffer, renderer, &surface,
		&colorValue));

	surface.surfaceType = dsFramebufferSurfaceType_Renderbuffer;
	surface.surface = colorBuffer;
	EXPECT_TRUE(dsRenderer_clearColorSurface(renderer->mainCommandBuffer, renderer, &surface,
		&colorValue));

	surface.surface = depthBuffer;
	EXPECT_FALSE(dsRenderer_clearColorSurface(renderer->mainCommandBuffer, renderer, &surface,
		&colorValue));

	surface.surfaceType = dsFramebufferSurfaceType_ColorRenderSurface;
	surface.surface = renderSurface;
	EXPECT_TRUE(dsRenderer_clearColorSurface(renderer->mainCommandBuffer, renderer, &surface,
		&colorValue));

	surface.surfaceType = dsFramebufferSurfaceType_DepthRenderSurface;
	EXPECT_FALSE(dsRenderer_clearColorSurface(renderer->mainCommandBuffer, renderer, &surface,
		&colorValue));

	EXPECT_TRUE(dsRenderSurface_destroy(renderSurface));
	EXPECT_TRUE(dsRenderbuffer_destroy(depthBuffer));
	EXPECT_TRUE(dsRenderbuffer_destroy(colorBuffer));
	EXPECT_TRUE(dsTexture_destroy(offscreen1));
	EXPECT_TRUE(dsTexture_destroy(offscreen2));
}

TEST_F(RendererTest, ClearDepthStencilSurface)
{
	dsOffscreen* offscreen1 = dsTexture_createOffscreen(resourceManager, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Static, dsGfxFormat_D24S8, dsTextureDim_2D, 1920, 1080,
		0, 1, 4, true);
	ASSERT_TRUE(offscreen1);

	dsOffscreen* offscreen2 = dsTexture_createOffscreen(resourceManager, NULL,
		dsTextureUsage_Texture, dsGfxMemory_Static, dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8,
		dsGfxFormat_UNorm), dsTextureDim_2D, 1920, 1080,
		0, 1, 4, true);
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
	dsFramebufferSurface surface = {dsFramebufferSurfaceType_Offscreen, dsCubeFace_None, 0, 0,
		offscreen1};
	EXPECT_FALSE(dsRenderer_clearDepthStencilSurface(renderer->mainCommandBuffer, NULL, &surface,
		dsClearDepthStencil_Both, &depthStencilValue));
	EXPECT_FALSE(dsRenderer_clearDepthStencilSurface(NULL, renderer, &surface,
		dsClearDepthStencil_Both, &depthStencilValue));
	EXPECT_FALSE(dsRenderer_clearDepthStencilSurface(renderer->mainCommandBuffer, renderer, NULL,
		dsClearDepthStencil_Both, &depthStencilValue));
	EXPECT_FALSE(dsRenderer_clearDepthStencilSurface(renderer->mainCommandBuffer, renderer,
		&surface, dsClearDepthStencil_Both, NULL));

	surface.layer = 2;
	EXPECT_FALSE(dsRenderer_clearDepthStencilSurface(renderer->mainCommandBuffer, renderer,
		&surface, dsClearDepthStencil_Both, &depthStencilValue));

	surface.layer = 0;
	surface.mipLevel = 2;
	EXPECT_FALSE(dsRenderer_clearDepthStencilSurface(renderer->mainCommandBuffer, renderer,
		&surface, dsClearDepthStencil_Both, &depthStencilValue));

	surface.mipLevel = 0;
	EXPECT_TRUE(dsRenderer_clearDepthStencilSurface(renderer->mainCommandBuffer, renderer, &surface,
		dsClearDepthStencil_Both, &depthStencilValue));

	surface.surface = offscreen2;
	EXPECT_FALSE(dsRenderer_clearDepthStencilSurface(renderer->mainCommandBuffer, renderer,
		&surface, dsClearDepthStencil_Both, &depthStencilValue));

	surface.surfaceType = dsFramebufferSurfaceType_Renderbuffer;
	surface.surface = colorBuffer;
	EXPECT_FALSE(dsRenderer_clearDepthStencilSurface(renderer->mainCommandBuffer, renderer,
		&surface, dsClearDepthStencil_Both, &depthStencilValue));

	surface.surface = depthBuffer;
	EXPECT_TRUE(dsRenderer_clearDepthStencilSurface(renderer->mainCommandBuffer, renderer, &surface,
		dsClearDepthStencil_Both, &depthStencilValue));

	surface.surfaceType = dsFramebufferSurfaceType_ColorRenderSurface;
	surface.surface = renderSurface;
	EXPECT_FALSE(dsRenderer_clearDepthStencilSurface(renderer->mainCommandBuffer, renderer,
		&surface, dsClearDepthStencil_Both, &depthStencilValue));

	surface.surfaceType = dsFramebufferSurfaceType_DepthRenderSurface;
	EXPECT_TRUE(dsRenderer_clearDepthStencilSurface(renderer->mainCommandBuffer, renderer, &surface,
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
	EXPECT_FALSE(dsRenderer_draw(renderer->mainCommandBuffer, NULL, geometry, &drawRange));
	EXPECT_FALSE(dsRenderer_draw(NULL, renderer, geometry, &drawRange));
	EXPECT_FALSE(dsRenderer_draw(renderer->mainCommandBuffer, renderer, NULL, &drawRange));
	EXPECT_FALSE(dsRenderer_draw(renderer->mainCommandBuffer, renderer, geometry, NULL));

	EXPECT_TRUE(dsRenderer_draw(renderer->mainCommandBuffer, renderer, geometry, &drawRange));

	drawRange.firstVertex = 4;
	EXPECT_FALSE(dsRenderer_draw(renderer->mainCommandBuffer, renderer, geometry, &drawRange));

	drawRange.firstVertex = 0;
	drawRange.instanceCount = 10;
	EXPECT_TRUE(dsRenderer_draw(renderer->mainCommandBuffer, renderer, geometry, &drawRange));

	renderer->supportsInstancedDrawing = false;
	EXPECT_FALSE(dsRenderer_draw(renderer->mainCommandBuffer, renderer, geometry, &drawRange));

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
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer->mainCommandBuffer, NULL, geometry1, &drawRange));
	EXPECT_FALSE(dsRenderer_drawIndexed(NULL, renderer, geometry1, &drawRange));
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer->mainCommandBuffer, renderer, NULL, &drawRange));
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer->mainCommandBuffer, renderer, geometry1, NULL));

	EXPECT_TRUE(dsRenderer_drawIndexed(renderer->mainCommandBuffer, renderer, geometry1,
		&drawRange));
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer->mainCommandBuffer, renderer, geometry2,
		&drawRange));

	drawRange.firstIndex = 4;
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer->mainCommandBuffer, renderer, geometry1,
		&drawRange));

	drawRange.firstIndex = 0;
	drawRange.instanceCount = 10;
	EXPECT_TRUE(dsRenderer_drawIndexed(renderer->mainCommandBuffer, renderer, geometry1,
		&drawRange));

	renderer->supportsInstancedDrawing = false;
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer->mainCommandBuffer, renderer, geometry1,
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

	EXPECT_FALSE(dsRenderer_drawIndirect(renderer->mainCommandBuffer, NULL, geometry,
		indirectBuffer, 0, 4, sizeof(dsDrawRange)));
	EXPECT_FALSE(dsRenderer_drawIndirect(NULL, renderer, geometry, indirectBuffer, 0, 4,
		sizeof(dsDrawRange)));
	EXPECT_FALSE(dsRenderer_drawIndirect(renderer->mainCommandBuffer, renderer, NULL,
		indirectBuffer, 0, 4, sizeof(dsDrawRange)));
	EXPECT_FALSE(dsRenderer_drawIndirect(renderer->mainCommandBuffer, renderer, geometry, NULL, 0,
		4, sizeof(dsDrawRange)));
	EXPECT_FALSE(dsRenderer_drawIndirect(renderer->mainCommandBuffer, renderer, geometry,
		indirectBuffer, 1, 3, sizeof(dsDrawRange)));
	EXPECT_FALSE(dsRenderer_drawIndirect(renderer->mainCommandBuffer, renderer, geometry,
		indirectBuffer, 0, 5, sizeof(dsDrawRange)));
	EXPECT_FALSE(dsRenderer_drawIndirect(renderer->mainCommandBuffer, renderer, geometry,
		indirectBuffer, 0, 4, 1));

	EXPECT_TRUE(dsRenderer_drawIndirect(renderer->mainCommandBuffer, renderer, geometry,
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

	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer->mainCommandBuffer, NULL, geometry1,
		indirectBuffer, 0, 4, sizeof(dsDrawRange)));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(NULL, renderer, geometry1, indirectBuffer, 0, 4,
		sizeof(dsDrawIndexedRange)));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer->mainCommandBuffer, renderer, NULL,
		indirectBuffer, 0, 4, sizeof(dsDrawIndexedRange)));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer->mainCommandBuffer, renderer, geometry1,
		NULL, 0, 4, sizeof(dsDrawIndexedRange)));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer->mainCommandBuffer, renderer, geometry1,
		indirectBuffer, 1, 3, sizeof(dsDrawIndexedRange)));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer->mainCommandBuffer, renderer, geometry1,
		indirectBuffer, 0, 5, sizeof(dsDrawIndexedRange)));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer->mainCommandBuffer, renderer, geometry1,
		indirectBuffer, 0, 4, 1));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer->mainCommandBuffer, renderer, geometry2,
		indirectBuffer, 0, 4, sizeof(dsDrawIndexedRange)));

	EXPECT_TRUE(dsRenderer_drawIndexedIndirect(renderer->mainCommandBuffer, renderer, geometry1,
		indirectBuffer, 0, 4, sizeof(dsDrawIndexedRange)));

	EXPECT_TRUE(dsDrawGeometry_destroy(geometry1));
	EXPECT_TRUE(dsDrawGeometry_destroy(geometry2));
	EXPECT_TRUE(dsGfxBuffer_destroy(vertexGfxBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(indexGfxBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(indirectBuffer));
}

TEST_F(RendererTest, DispatchCompute)
{
	EXPECT_FALSE(dsRenderer_dispatchCompute(renderer->mainCommandBuffer, NULL, 1, 1, 1));
	EXPECT_FALSE(dsRenderer_dispatchCompute(NULL, renderer, 1, 1, 1));

	EXPECT_TRUE(dsRenderer_dispatchCompute(renderer->mainCommandBuffer, renderer, 1, 1, 1));
	renderer->hasComputeShaders = false;
	EXPECT_FALSE(dsRenderer_dispatchCompute(renderer->mainCommandBuffer, renderer, 1, 1, 1));
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

	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer->mainCommandBuffer, NULL,
		indirectBuffer, sizeof(uint32_t)));
	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(NULL, renderer, indirectBuffer,
		sizeof(uint32_t)));
	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer->mainCommandBuffer, renderer,
		NULL, sizeof(uint32_t)));
	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer->mainCommandBuffer, renderer,
		vertexGfxBuffer, sizeof(uint32_t)));
	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer->mainCommandBuffer, renderer,
		indirectBuffer, 1));
	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer->mainCommandBuffer, renderer,
		indirectBuffer, 2*sizeof(uint32_t)));

	EXPECT_TRUE(dsRenderer_dispatchComputeIndirect(renderer->mainCommandBuffer, renderer,
		indirectBuffer, sizeof(uint32_t)));
	renderer->hasComputeShaders = false;
	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer->mainCommandBuffer, renderer,
		indirectBuffer, sizeof(uint32_t)));

	EXPECT_TRUE(dsGfxBuffer_destroy(vertexGfxBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(indirectBuffer));
}

TEST_F(RendererTest, WaitUntilIdle)
{
	EXPECT_FALSE(dsRenderer_waitUntilIdle(NULL));
	EXPECT_TRUE(dsRenderer_waitUntilIdle(renderer));
}

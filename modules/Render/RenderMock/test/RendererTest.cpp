/*
 * Copyright 2017-2023 Aaron Barany
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

#include "Fixtures/AssetFixtureBase.h"
#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/Renderbuffer.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Render/RenderPass.h>
#include <DeepSea/Render/RenderSurface.h>
#include <gtest/gtest.h>

class RendererTest : public AssetFixtureBase
{
public:
	RendererTest()
		: AssetFixtureBase("shaders")
	{
	}

	void SetUp() override
	{
		RenderPassFixtureBase::SetUp();

		dsShaderVariableElement transformElements[] =
		{
			{"modelViewProjection", dsMaterialType_Mat4, 0},
			{"normalMat", dsMaterialType_Mat3, 0}
		};
		unsigned int transformElementCount = DS_ARRAY_SIZE(transformElements);
		transformDesc = dsShaderVariableGroupDesc_create(resourceManager, NULL, transformElements,
			transformElementCount);
		ASSERT_TRUE(transformDesc);

		dsMaterialElement elements[] =
		{
			{"diffuseTexture", dsMaterialType_Texture, 0, NULL, dsMaterialBinding_Material, 0},
			{"colorMultiplier", dsMaterialType_Vec4, 0, NULL, dsMaterialBinding_Material, 0},
			{"textureScaleOffset", dsMaterialType_Vec2, 2, NULL, dsMaterialBinding_Material, 0},
			{"Transform", dsMaterialType_VariableGroup, 0, transformDesc,
				dsMaterialBinding_Material, 0},
			{"extraVar", dsMaterialType_Int, 0, NULL, dsMaterialBinding_Material, 0}
		};
		unsigned int elementCount = DS_ARRAY_SIZE(elements);
		materialDesc = dsMaterialDesc_create(resourceManager, NULL, elements, elementCount);
		ASSERT_TRUE(materialDesc);

		shaderModule = dsShaderModule_loadResource(resourceManager, NULL,
			dsFileResourceType_Embedded, getRelativePath("test.mslb"), "test");
		ASSERT_TRUE(shaderModule);

		shader = dsShader_createName(resourceManager, NULL, shaderModule, "Test", materialDesc);
		ASSERT_TRUE(shader);

		transformGroup = dsShaderVariableGroup_create(resourceManager, NULL, NULL, transformDesc);
		ASSERT_TRUE(transformGroup);

		material = dsMaterial_create(resourceManager, (dsAllocator*)&allocator, materialDesc);
		ASSERT_TRUE(material);
	}

	void TearDown() override
	{
		dsMaterial_destroy(material);
		EXPECT_TRUE(dsShaderVariableGroup_destroy(transformGroup));
		EXPECT_TRUE(dsShader_destroy(shader));
		EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
		EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
		EXPECT_TRUE(dsShaderVariableGroupDesc_destroy(transformDesc));
		RenderPassFixtureBase::TearDown();
	}

	dsShaderVariableGroupDesc* transformDesc;
	dsMaterialDesc* materialDesc;
	dsShaderModule* shaderModule;
	dsShader* shader;
	dsShaderVariableGroup* transformGroup;
	dsMaterial* material;
};

TEST_F(RendererTest, BeginEndFrame)
{
	// NOTE: frame was already begun in fixture, so end frame first for this test.
	EXPECT_FALSE(dsRenderer_endFrame(nullptr));
	EXPECT_TRUE(dsRenderer_endFrame(renderer));

	EXPECT_FALSE(dsRenderer_beginFrame(nullptr));
	EXPECT_TRUE(dsRenderer_beginFrame(renderer));
}

TEST_F(RendererTest, SetSurfaceSamples)
{
	EXPECT_FALSE(dsRenderer_setSurfaceSamples(NULL, 1));
	EXPECT_FALSE(dsRenderer_setSurfaceSamples(renderer, renderer->maxSurfaceSamples + 1));
	EXPECT_TRUE(dsRenderer_setSurfaceSamples(renderer, renderer->maxSurfaceSamples));
}

TEST_F(RendererTest, SetDefaultSamples)
{
	EXPECT_FALSE(dsRenderer_setDefaultSamples(NULL, 1));
	EXPECT_FALSE(dsRenderer_setDefaultSamples(renderer, renderer->maxSurfaceSamples + 1));
	EXPECT_TRUE(dsRenderer_setDefaultSamples(renderer, renderer->maxSurfaceSamples));
}

TEST_F(RendererTest, SetSamples)
{
	EXPECT_FALSE(dsRenderer_setSamples(NULL, 1));
	EXPECT_FALSE(dsRenderer_setSamples(renderer, renderer->maxSurfaceSamples + 1));
	EXPECT_TRUE(dsRenderer_setSamples(renderer, renderer->maxSurfaceSamples));
}

TEST_F(RendererTest, SetVSync)
{
	EXPECT_FALSE(dsRenderer_setVSync(NULL, dsVSync_Disabled));
	EXPECT_TRUE(dsRenderer_setVSync(renderer, dsVSync_Disabled));
}

TEST_F(RendererTest, SetDefaultAnisotropy)
{
	EXPECT_FALSE(dsRenderer_setDefaultAnisotropy(NULL, 4.0f));
	EXPECT_FALSE(dsRenderer_setDefaultAnisotropy(renderer, renderer->maxAnisotropy + 1));
	EXPECT_TRUE(dsRenderer_setDefaultAnisotropy(renderer, renderer->maxAnisotropy));
}

TEST_F(RendererTest, Draw)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;
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

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0, false));
	EXPECT_TRUE(dsShader_bind(shader, commandBuffer, material, NULL, NULL));

	dsDrawRange drawRange = {10, 1, 0, 0};
	EXPECT_FALSE(dsRenderer_draw(NULL, commandBuffer, geometry, &drawRange,
		dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_draw(renderer, NULL, geometry, &drawRange,
		dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_draw(renderer, commandBuffer, NULL, &drawRange,
		dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_draw(renderer, commandBuffer, geometry, NULL,
		dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsRenderer_draw(renderer, commandBuffer, geometry, &drawRange,
		dsPrimitiveType_TriangleList));

	drawRange.firstVertex = 4;
	EXPECT_FALSE(dsRenderer_draw(renderer, commandBuffer, geometry, &drawRange,
		dsPrimitiveType_TriangleList));

	drawRange.firstVertex = 0;
	drawRange.instanceCount = 10;
	EXPECT_TRUE(dsRenderer_draw(renderer, commandBuffer, geometry, &drawRange,
		dsPrimitiveType_TriangleList));

	renderer->hasInstancedDrawing = false;
	EXPECT_FALSE(dsRenderer_draw(renderer, commandBuffer, geometry, &drawRange,
		dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsShader_unbind(shader, commandBuffer));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	renderer->hasInstancedDrawing = true;
	EXPECT_FALSE(dsRenderer_draw(renderer, commandBuffer, geometry, &drawRange,
		dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsDrawGeometry_destroy(geometry));
	EXPECT_TRUE(dsGfxBuffer_destroy(vertexGfxBuffer));
}

TEST_F(RendererTest, DrawIndexed)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;
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

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0, false));
	EXPECT_TRUE(dsShader_bind(shader, commandBuffer, material, NULL, NULL));

	dsDrawIndexedRange drawRange = {16, 1, 0, 0, 0};
	EXPECT_FALSE(dsRenderer_drawIndexed(NULL, commandBuffer, geometry1, &drawRange,
		dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer, NULL, geometry1, &drawRange,
		dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer, commandBuffer, NULL, &drawRange,
		dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer, commandBuffer, geometry1, NULL,
		dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsRenderer_drawIndexed(renderer, commandBuffer, geometry1, &drawRange,
		dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer, commandBuffer, geometry2, &drawRange,
		dsPrimitiveType_TriangleList));

	drawRange.firstIndex = 4;
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer, commandBuffer, geometry1, &drawRange,
		dsPrimitiveType_TriangleList));

	drawRange.firstIndex = 0;
	drawRange.instanceCount = 10;
	EXPECT_TRUE(dsRenderer_drawIndexed(renderer, commandBuffer, geometry1, &drawRange,
		dsPrimitiveType_TriangleList));

	renderer->hasInstancedDrawing = false;
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer, commandBuffer, geometry1, &drawRange,
		dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsShader_unbind(shader, commandBuffer));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	renderer->hasInstancedDrawing = true;
	EXPECT_FALSE(dsRenderer_drawIndexed(renderer, commandBuffer, geometry1, &drawRange,
		dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsDrawGeometry_destroy(geometry1));
	EXPECT_TRUE(dsDrawGeometry_destroy(geometry2));
	EXPECT_TRUE(dsGfxBuffer_destroy(vertexGfxBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(indexGfxBuffer));
}

TEST_F(RendererTest, DrawIndirect)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;
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

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0, false));
	EXPECT_TRUE(dsShader_bind(shader, commandBuffer, material, NULL, NULL));

	EXPECT_FALSE(dsRenderer_drawIndirect(NULL, commandBuffer, geometry,
		indirectBuffer, 0, 4, sizeof(dsDrawRange), dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_drawIndirect(renderer, NULL, geometry, indirectBuffer, 0, 4,
		sizeof(dsDrawRange), dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_drawIndirect(renderer, commandBuffer, NULL, indirectBuffer, 0, 4,
		sizeof(dsDrawRange), dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_drawIndirect(renderer, commandBuffer, geometry, NULL, 0, 4,
		sizeof(dsDrawRange), dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_drawIndirect(renderer, commandBuffer, geometry, indirectBuffer, 1, 3,
		sizeof(dsDrawRange), dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_drawIndirect(renderer, commandBuffer, geometry, indirectBuffer, 0, 5,
		sizeof(dsDrawRange), dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_drawIndirect(renderer, commandBuffer, geometry, indirectBuffer, 0, 4,
		1, dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsRenderer_drawIndirect(renderer, commandBuffer, geometry, indirectBuffer, 0, 4,
		sizeof(dsDrawRange), dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsShader_unbind(shader, commandBuffer));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	EXPECT_FALSE(dsRenderer_drawIndirect(renderer, commandBuffer, geometry, indirectBuffer, 0, 4,
		sizeof(dsDrawRange), dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsDrawGeometry_destroy(geometry));
	EXPECT_TRUE(dsGfxBuffer_destroy(vertexGfxBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(indirectBuffer));
}

TEST_F(RendererTest, DrawIndexedIndirect)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;
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

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0, false));
	EXPECT_TRUE(dsShader_bind(shader, commandBuffer, material, NULL, NULL));

	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(NULL, commandBuffer, geometry1, indirectBuffer, 0,
		4, sizeof(dsDrawRange), dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer, NULL, geometry1, indirectBuffer, 0, 4,
		sizeof(dsDrawIndexedRange), dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer, commandBuffer, NULL, indirectBuffer, 0, 4,
		sizeof(dsDrawIndexedRange), dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer, commandBuffer, geometry1, NULL, 0, 4,
		sizeof(dsDrawIndexedRange), dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer, commandBuffer, geometry1, indirectBuffer,
		1, 3, sizeof(dsDrawIndexedRange), dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer, commandBuffer, geometry1, indirectBuffer,
		0, 5, sizeof(dsDrawIndexedRange), dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer, commandBuffer, geometry1, indirectBuffer,
		0, 4, 1, dsPrimitiveType_TriangleList));
	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer, commandBuffer, geometry2, indirectBuffer,
		0, 4, sizeof(dsDrawIndexedRange), dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsRenderer_drawIndexedIndirect(renderer, commandBuffer, geometry1, indirectBuffer,
		0, 4, sizeof(dsDrawIndexedRange), dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsShader_unbind(shader, commandBuffer));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	EXPECT_FALSE(dsRenderer_drawIndexedIndirect(renderer, commandBuffer, geometry1, indirectBuffer,
		0, 4, sizeof(dsDrawIndexedRange), dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsDrawGeometry_destroy(geometry1));
	EXPECT_TRUE(dsDrawGeometry_destroy(geometry2));
	EXPECT_TRUE(dsGfxBuffer_destroy(vertexGfxBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(indexGfxBuffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(indirectBuffer));
}

TEST_F(RendererTest, SetViewport)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;
	dsAlignedBox3f viewport = {{{0.0f, 0.0f, 0.0f}}, {{1024.0f, 768.0f, 1.0f}}};
	EXPECT_FALSE(dsRenderer_setViewport(renderer, commandBuffer, &viewport));

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0, false));
	EXPECT_TRUE(dsRenderer_setViewport(renderer, commandBuffer, &viewport));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));
}

TEST_F(RendererTest, ClearAttachments)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;
	dsClearAttachment clearAttachments[2];
	clearAttachments[0].colorAttachment = 0;
	clearAttachments[0].clearValue.colorValue.floatValue.r = 0.0f;
	clearAttachments[0].clearValue.colorValue.floatValue.g = 0.0f;
	clearAttachments[0].clearValue.colorValue.floatValue.b = 0.0f;
	clearAttachments[0].clearValue.colorValue.floatValue.a = 1.0f;
	clearAttachments[1].colorAttachment = DS_NO_ATTACHMENT;
	clearAttachments[1].clearDepthStencil = dsClearDepthStencil_Both;
	clearAttachments[1].clearValue.depthStencil.depth = 1.0f;
	clearAttachments[1].clearValue.depthStencil.stencil = 0;

	dsAttachmentClearRegion clearRegion =
	{
		0,
		0,
		framebuffer->width,
		framebuffer->height,
		0,
		1
	};

	EXPECT_FALSE(dsRenderer_clearAttachments(renderer, commandBuffer, clearAttachments,
		DS_ARRAY_SIZE(clearAttachments), &clearRegion, 1));

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0, false));

	EXPECT_FALSE(dsRenderer_clearAttachments(NULL, commandBuffer, clearAttachments,
		DS_ARRAY_SIZE(clearAttachments), &clearRegion, 1));
	EXPECT_FALSE(dsRenderer_clearAttachments(renderer, NULL, clearAttachments,
		DS_ARRAY_SIZE(clearAttachments), &clearRegion, 1));
	EXPECT_FALSE(dsRenderer_clearAttachments(renderer, commandBuffer, NULL,
		DS_ARRAY_SIZE(clearAttachments), &clearRegion, 1));
	EXPECT_FALSE(dsRenderer_clearAttachments(renderer, commandBuffer, clearAttachments,
		DS_ARRAY_SIZE(clearAttachments), NULL, 1));
	EXPECT_TRUE(dsRenderer_clearAttachments(renderer, commandBuffer, clearAttachments,
		DS_ARRAY_SIZE(clearAttachments), &clearRegion, 1));

	clearAttachments[0].colorAttachment = 1;
	EXPECT_FALSE(dsRenderer_clearAttachments(renderer, commandBuffer, clearAttachments,
		DS_ARRAY_SIZE(clearAttachments), &clearRegion, 1));

	clearAttachments[0].colorAttachment = 0;
	clearRegion.x = 1;
	EXPECT_FALSE(dsRenderer_clearAttachments(renderer, commandBuffer, clearAttachments,
		DS_ARRAY_SIZE(clearAttachments), &clearRegion, 1));

	clearRegion.x = 0;
	clearRegion.y = 1;
	EXPECT_FALSE(dsRenderer_clearAttachments(renderer, commandBuffer, clearAttachments,
		DS_ARRAY_SIZE(clearAttachments), &clearRegion, 1));

	clearRegion.y = 0;
	clearRegion.layer = 1;
	EXPECT_FALSE(dsRenderer_clearAttachments(renderer, commandBuffer, clearAttachments,
		DS_ARRAY_SIZE(clearAttachments), &clearRegion, 1));

	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));
}

TEST_F(RendererTest, DispatchCompute)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;
	EXPECT_FALSE(dsRenderer_dispatchCompute(NULL, commandBuffer, 1, 1, 1));
	EXPECT_FALSE(dsRenderer_dispatchCompute(renderer, NULL, 1, 1, 1));

	EXPECT_TRUE(dsShader_bindCompute(shader, commandBuffer, material, NULL));

	EXPECT_FALSE(dsRenderer_dispatchCompute(renderer, commandBuffer, 512, 512, 512));
	EXPECT_TRUE(dsRenderer_dispatchCompute(renderer, commandBuffer, 1, 1, 1));
	renderer->maxComputeWorkGroupSize[0] = 0;
	EXPECT_FALSE(dsRenderer_dispatchCompute(renderer, commandBuffer, 1, 1, 1));

	renderer->maxComputeWorkGroupSize[0] = 256;
	EXPECT_TRUE(dsShader_unbindCompute(shader, commandBuffer));

	EXPECT_FALSE(dsRenderer_dispatchCompute(renderer, commandBuffer, 1, 1, 1));
}

TEST_F(RendererTest, DispatchComputeIndirect)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;
	dsGfxBuffer* vertexGfxBuffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_Vertex, dsGfxMemory_Static | dsGfxMemory_Draw, NULL, 1024);
	ASSERT_TRUE(vertexGfxBuffer);

	dsGfxBuffer* indirectBuffer = dsGfxBuffer_create(resourceManager, NULL,
		dsGfxBufferUsage_IndirectDispatch, dsGfxMemory_Static | dsGfxMemory_Draw, NULL,
		sizeof(uint32_t)*4);
	ASSERT_TRUE(indirectBuffer);

	EXPECT_TRUE(dsShader_bindCompute(shader, commandBuffer, material, NULL));

	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(NULL, commandBuffer,
		indirectBuffer, sizeof(uint32_t)));
	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer, NULL, indirectBuffer,
		sizeof(uint32_t)));
	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer, commandBuffer,
		NULL, sizeof(uint32_t)));
	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer, commandBuffer,
		vertexGfxBuffer, sizeof(uint32_t)));
	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer, commandBuffer,
		indirectBuffer, 1));
	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer, commandBuffer,
		indirectBuffer, 2*sizeof(uint32_t)));

	EXPECT_TRUE(dsRenderer_dispatchComputeIndirect(renderer, commandBuffer,
		indirectBuffer, sizeof(uint32_t)));
	renderer->maxComputeWorkGroupSize[0] = 0;
	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer, commandBuffer,
		indirectBuffer, sizeof(uint32_t)));

	renderer->maxComputeWorkGroupSize[0] = 256;
	EXPECT_TRUE(dsShader_unbindCompute(shader, commandBuffer));

	EXPECT_FALSE(dsRenderer_dispatchComputeIndirect(renderer, commandBuffer,
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

	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Offscreen,
		fromTexture, dsGfxSurfaceType_Offscreen, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));
	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsTexture_destroy(toTexture));

	fromTexture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyFrom, dsGfxMemory_Static, &fromInfo,
		textureData, sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	toTexture = dsTexture_create(resourceManager, NULL, dsTextureUsage_Texture, dsGfxMemory_Static,
		&toInfo, NULL, 0);
	ASSERT_TRUE(toTexture);

	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Offscreen,
		fromTexture, dsGfxSurfaceType_Offscreen, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));
	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsTexture_destroy(toTexture));

	fromTexture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyFrom, dsGfxMemory_Static, &fromInfo,
		textureData, sizeof(textureData));
	ASSERT_TRUE(fromTexture);

	toTexture = dsTexture_create(resourceManager, NULL,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo | dsTextureUsage_CopyFrom, dsGfxMemory_Read,
		&toInfo, NULL, 0);
	ASSERT_TRUE(toTexture);

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0, false));
	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Offscreen,
		fromTexture, dsGfxSurfaceType_Offscreen, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	EXPECT_TRUE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Offscreen,
		fromTexture, dsGfxSurfaceType_Offscreen, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));

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
	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Offscreen,
		fromTexture, dsGfxSurfaceType_Offscreen, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));

	blitRegion.srcPosition.x = 1;
	blitRegion.srcPosition.y = 13;
	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Offscreen,
		fromTexture, dsGfxSurfaceType_Offscreen, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));

	blitRegion.srcPosition.x = 0;
	blitRegion.srcPosition.y = 0;
	blitRegion.srcPosition.mipLevel = 5;
	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Offscreen,
		fromTexture, dsGfxSurfaceType_Offscreen, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));

	blitRegion.srcPosition.mipLevel = 0;
	blitRegion.srcPosition.depth = 3;
	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Offscreen,
		fromTexture, dsGfxSurfaceType_Offscreen, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));

	blitRegion.srcPosition.depth = 0;
	blitRegion.dstPosition.x = 17;
	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Offscreen,
		fromTexture, dsGfxSurfaceType_Offscreen, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));

	blitRegion.dstPosition.x = 3;
	blitRegion.dstPosition.y = 29;
	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Offscreen,
		fromTexture, dsGfxSurfaceType_Offscreen, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));

	blitRegion.dstPosition.y = 4;
	blitRegion.dstPosition.mipLevel = 3;
	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Offscreen,
		fromTexture, dsGfxSurfaceType_Offscreen, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));

	blitRegion.dstPosition.mipLevel = 0;
	blitRegion.dstPosition.depth = 4;
	EXPECT_FALSE(dsRenderer_blitSurface(renderer, commandBuffer, dsGfxSurfaceType_Offscreen,
		fromTexture, dsGfxSurfaceType_Offscreen, toTexture, &blitRegion, 1, dsBlitFilter_Nearest));

	EXPECT_TRUE(dsTexture_destroy(fromTexture));
	EXPECT_TRUE(dsTexture_destroy(toTexture));
}

TEST_F(RendererTest, MemoryBarrier)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;
	dsGfxMemoryBarrier barriers[] = {{dsGfxAccess_UniformBufferWrite, dsGfxAccess_IndexRead},
		{dsGfxAccess_HostWrite, dsGfxAccess_VertexAttributeRead}};
	EXPECT_FALSE(dsRenderer_memoryBarrier(NULL, commandBuffer,
		dsGfxPipelineStage_ComputeShader | dsGfxPipelineStage_HostAccess,
		dsGfxPipelineStage_VertexInput, barriers, DS_ARRAY_SIZE(barriers)));
	EXPECT_FALSE(dsRenderer_memoryBarrier(renderer, NULL,
		dsGfxPipelineStage_ComputeShader | dsGfxPipelineStage_HostAccess,
		dsGfxPipelineStage_VertexInput, barriers, DS_ARRAY_SIZE(barriers)));
	EXPECT_FALSE(dsRenderer_memoryBarrier(renderer, commandBuffer, (dsGfxPipelineStage)0,
		dsGfxPipelineStage_VertexInput, NULL, DS_ARRAY_SIZE(barriers)));
	EXPECT_FALSE(dsRenderer_memoryBarrier(renderer, commandBuffer,
		dsGfxPipelineStage_ComputeShader | dsGfxPipelineStage_HostAccess, (dsGfxPipelineStage)0,
		barriers, DS_ARRAY_SIZE(barriers)));
	EXPECT_TRUE(dsRenderer_memoryBarrier(renderer, commandBuffer,
		dsGfxPipelineStage_ComputeShader | dsGfxPipelineStage_HostAccess,
		dsGfxPipelineStage_VertexInput, barriers, DS_ARRAY_SIZE(barriers)));
	EXPECT_TRUE(dsRenderer_memoryBarrier(renderer, commandBuffer,
		dsGfxPipelineStage_ComputeShader | dsGfxPipelineStage_HostAccess,
		dsGfxPipelineStage_VertexInput, NULL, 0));
}

TEST_F(RendererTest, WaitUntilIdle)
{
	EXPECT_FALSE(dsRenderer_waitUntilIdle(NULL));
	EXPECT_TRUE(dsRenderer_waitUntilIdle(renderer));
}

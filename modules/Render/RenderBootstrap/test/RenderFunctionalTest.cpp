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
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Render/Resources/Framebuffer.h>
#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/RenderPass.h>
#include <DeepSea/Render/Renderer.h>

namespace
{
class WriteOffscreenInfo
{
public:
	explicit WriteOffscreenInfo(const FixtureBase& fixture)
	{
		initialize(fixture, 2, 2, 1);
	}

	WriteOffscreenInfo(const FixtureBase& fixture, uint32_t width, uint32_t height,
		uint32_t mipLevels)
	{
		initialize(fixture, width, height, mipLevels);
	}

	~WriteOffscreenInfo()
	{
		EXPECT_TRUE(dsRenderPass_destroy(renderPass));
		EXPECT_TRUE(dsFramebuffer_destroy(framebuffer));
		EXPECT_TRUE(dsTexture_destroy(offscreen));
		EXPECT_TRUE(dsShader_destroy(shader));
		EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
		dsMaterial_destroy(material);
		EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));
	}

	dsMaterialDesc* materialDesc = nullptr;
	dsMaterial* material = nullptr;
	dsShaderModule* shaderModule = nullptr;
	dsShader* shader = nullptr;
	dsOffscreen* offscreen = nullptr;
	dsFramebuffer* framebuffer = nullptr;
	dsRenderPass* renderPass = nullptr;

private:
	void initialize(const FixtureBase& fixture, uint32_t width, uint32_t height, uint32_t mipLevels)
	{
		dsAllocator* allocator = (dsAllocator*)&fixture.allocator;
		dsRenderer* renderer = fixture.renderer;
		dsResourceManager* resourceManager = fixture.resourceManager;

		dsMaterialElement materialElements[] =
		{
			{"projection", dsMaterialType_Mat4, 0, NULL, false, 0}
		};

		materialDesc = dsMaterialDesc_create(resourceManager, allocator, materialElements,
			DS_ARRAY_SIZE(materialElements));
		ASSERT_TRUE(materialDesc);

		material = dsMaterial_create(resourceManager, allocator, materialDesc);
		ASSERT_TRUE(material);

		dsMatrix44f projection;
		ASSERT_TRUE(dsRenderer_makeOrtho(&projection, renderer, -0.25f, 1.25f, -0.25f, 1.25f, 0.0f,
			1.0f));
		uint32_t projectionIdx = dsMaterialDesc_findElement(materialDesc, "projection");
		ASSERT_NE(DS_MATERIAL_UNKNOWN, projectionIdx);
		ASSERT_TRUE(dsMaterial_setElementData(material, projectionIdx, &projection,
			dsMaterialType_Mat4, 0, 1));

		shaderModule = dsShaderModule_loadResource(resourceManager, allocator,
			dsFileResourceType_Embedded, fixture.getShaderPath("WriteOffscreen.mslb"),
			"WriteOffscreen");
		ASSERT_TRUE(shaderModule);

		shader = dsShader_createName(resourceManager, allocator, shaderModule, "WriteOffscreen",
			materialDesc);
		ASSERT_TRUE(shader);

		dsGfxFormat surfaceFormat = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
		dsTextureInfo offscreenInfo = {surfaceFormat, dsTextureDim_2D, width, height, 0, mipLevels,
			1};
		auto usageFlags = (dsTextureUsage)(dsTextureUsage_Texture | dsTextureUsage_CopyFrom |
			dsTextureUsage_CopyTo);
		offscreen = dsTexture_createOffscreen(resourceManager, allocator, usageFlags,
			dsGfxMemory_Read, &offscreenInfo, true);
		ASSERT_TRUE(offscreen);

		dsFramebufferSurface surface = {dsGfxSurfaceType_Texture, dsCubeFace_None, 0, 0, offscreen};
		framebuffer = dsFramebuffer_create(resourceManager, allocator, "WriteOffscreen", &surface,
			1, width, height, 1);
		ASSERT_TRUE(framebuffer);

		dsAttachmentInfo attachment =
		{
			(dsAttachmentUsage)(dsAttachmentUsage_Clear | dsAttachmentUsage_KeepAfter),
			surfaceFormat, 1
		};
		dsColorAttachmentRef attachmentRef = {0, true};
		dsRenderSubpassInfo subpass = {"WriteOffscreen", NULL, &attachmentRef, 0, 1,
			DS_NO_ATTACHMENT};
		renderPass = dsRenderPass_create(renderer, allocator, &attachment, 1, &subpass, 1, NULL,
			DS_DEFAULT_SUBPASS_DEPENDENCIES);
		ASSERT_TRUE(renderPass);
	}
};

struct Vertex
{
	dsVector2f position;
	dsColor color;
};
} // namespace

class RendererFunctionalTest : public FixtureBase
{
};

INSTANTIATE_TEST_CASE_P(RendererFunctional, RendererFunctionalTest,
	FixtureBase::getRendererTypes());

TEST_P(RendererFunctionalTest, ReadFromOffscreen)
{
	WriteOffscreenInfo info(*this);

	Vertex vertices[] =
	{
		{{{0.0f, 0.0f}}, {{0, 0, 0, 255}}},
		{{{1.0f, 0.0f}}, {{255, 0, 0, 255}}},
		{{{1.0f, 1.0f}}, {{0, 0, 255, 255}}},

		{{{1.0f, 1.0f}}, {{0, 0, 255, 255}}},
		{{{0.0f, 1.0f}}, {{0, 255, 0, 255}}},
		{{{0.0f, 0.0f}}, {{0, 0, 0, 255}}}
	};
	dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, (dsAllocator*)&allocator,
		dsGfxBufferUsage_Vertex, (dsGfxMemory)(dsGfxMemory_Static | dsGfxMemory_Draw), vertices,
		sizeof(vertices));
	ASSERT_TRUE(buffer);

	Vertex otherVertices[] =
	{
		{{{0.0f, 0.0f}}, {{255, 255, 255, 255}}},
		{{{1.0f, 0.0f}}, {{0, 255, 255, 255}}},
		{{{1.0f, 1.0f}}, {{255, 255, 0, 255}}},

		{{{1.0f, 1.0f}}, {{255, 255, 0, 255}}},
		{{{0.0f, 1.0f}}, {{255, 0, 255, 255}}},
		{{{0.0f, 0.0f}}, {{255, 255, 255, 255}}}
	};
	dsGfxBuffer* otherBuffer = dsGfxBuffer_create(resourceManager, (dsAllocator*)&allocator,
		dsGfxBufferUsage_Vertex, (dsGfxMemory)(dsGfxMemory_Static | dsGfxMemory_Draw),
		otherVertices, sizeof(otherVertices));
	ASSERT_TRUE(otherBuffer);

	dsVertexFormat format;
	EXPECT_TRUE(dsVertexFormat_initialize(&format));
	EXPECT_TRUE(dsVertexFormat_setAttribEnabled(&format, dsVertexAttrib_Position, true));
	EXPECT_TRUE(dsVertexFormat_setAttribEnabled(&format, dsVertexAttrib_Color, true));
	format.elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32, dsGfxFormat_Float);
	format.elements[dsVertexAttrib_Color].format =
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	ASSERT_TRUE(dsVertexFormat_computeOffsetsAndSize(&format));

	ASSERT_EQ(sizeof(Vertex), format.size);
	ASSERT_EQ(offsetof(Vertex, position), format.elements[dsVertexAttrib_Position].offset);
	ASSERT_EQ(offsetof(Vertex, color), format.elements[dsVertexAttrib_Color].offset);

	dsDrawGeometry* drawGeometry;
	{
		dsVertexBuffer vertexBuffer = {buffer, 0, 6, format};
		dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS] = {&vertexBuffer, nullptr,
			nullptr, nullptr};
		drawGeometry = dsDrawGeometry_create(resourceManager, (dsAllocator*)&allocator,
			vertexBuffers, nullptr);
		ASSERT_TRUE(drawGeometry);
	}

	dsDrawGeometry* otherDrawGeometry;
	{
		dsVertexBuffer vertexBuffer = {otherBuffer, 0, 6, format};
		dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS] = {&vertexBuffer, nullptr,
			nullptr, nullptr};
		otherDrawGeometry = dsDrawGeometry_create(resourceManager, (dsAllocator*)&allocator,
			vertexBuffers, nullptr);
		ASSERT_TRUE(otherDrawGeometry);
	}

	dsSurfaceClearValue clearValue;
	clearValue.colorValue.floatValue.r = 1.0f;
	clearValue.colorValue.floatValue.g = 1.0f;
	clearValue.colorValue.floatValue.b = 1.0f;
	clearValue.colorValue.floatValue.a = 1.0f;
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;
	ASSERT_TRUE(dsRenderPass_begin(info.renderPass, commandBuffer, info.framebuffer, NULL,
		&clearValue, 1));
	ASSERT_TRUE(dsShader_bind(info.shader, commandBuffer, info.material, NULL, NULL));

	dsDrawRange drawRange = {6, 1, 0, 0};
	ASSERT_TRUE(dsRenderer_draw(renderer, commandBuffer, drawGeometry, &drawRange,
		dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsShader_unbind(info.shader, commandBuffer));
	EXPECT_TRUE(dsRenderPass_end(info.renderPass, commandBuffer));

	EXPECT_TRUE(dsRenderer_flush(renderer));

	dsColor colors[4];
	dsTexturePosition position = {dsCubeFace_None, 0, 0, 0, 0};
	ASSERT_TRUE(dsTexture_getData(colors, sizeof(colors), info.offscreen, &position, 2, 2));
	EXPECT_EQ(0, colors[0].r);
	EXPECT_EQ(255, colors[0].g);
	EXPECT_EQ(0, colors[0].b);
	EXPECT_EQ(255, colors[0].a);

	EXPECT_EQ(0, colors[1].r);
	EXPECT_EQ(0, colors[1].g);
	EXPECT_EQ(255, colors[1].b);
	EXPECT_EQ(255, colors[1].a);

	EXPECT_EQ(0, colors[2].r);
	EXPECT_EQ(0, colors[2].g);
	EXPECT_EQ(0, colors[2].b);
	EXPECT_EQ(255, colors[2].a);

	EXPECT_EQ(255, colors[3].r);
	EXPECT_EQ(0, colors[3].g);
	EXPECT_EQ(0, colors[3].b);
	EXPECT_EQ(255, colors[3].a);

	ASSERT_TRUE(dsRenderPass_begin(info.renderPass, commandBuffer, info.framebuffer, NULL,
		&clearValue, 1));
	ASSERT_TRUE(dsShader_bind(info.shader, commandBuffer, info.material, NULL, NULL));

	ASSERT_TRUE(dsRenderer_draw(renderer, commandBuffer, otherDrawGeometry, &drawRange,
		dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsShader_unbind(info.shader, commandBuffer));
	EXPECT_TRUE(dsRenderPass_end(info.renderPass, commandBuffer));

	EXPECT_TRUE(dsRenderer_flush(renderer));

	ASSERT_TRUE(dsTexture_getData(colors, sizeof(colors), info.offscreen, &position, 2, 2));
	EXPECT_EQ(255, colors[0].r);
	EXPECT_EQ(0, colors[0].g);
	EXPECT_EQ(255, colors[0].b);
	EXPECT_EQ(255, colors[0].a);

	EXPECT_EQ(255, colors[1].r);
	EXPECT_EQ(255, colors[1].g);
	EXPECT_EQ(0, colors[1].b);
	EXPECT_EQ(255, colors[1].a);

	EXPECT_EQ(255, colors[2].r);
	EXPECT_EQ(255, colors[2].g);
	EXPECT_EQ(255, colors[2].b);
	EXPECT_EQ(255, colors[2].a);

	EXPECT_EQ(0, colors[3].r);
	EXPECT_EQ(255, colors[3].g);
	EXPECT_EQ(255, colors[3].b);
	EXPECT_EQ(255, colors[3].a);

	EXPECT_TRUE(dsDrawGeometry_destroy(drawGeometry));
	EXPECT_TRUE(dsDrawGeometry_destroy(otherDrawGeometry));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));
	EXPECT_TRUE(dsGfxBuffer_destroy(otherBuffer));
}

TEST_P(RendererFunctionalTest, GenerateMipmaps)
{
	WriteOffscreenInfo info(*this, 7, 9, 3);

	Vertex vertices[] =
	{
		{{{0.0f, 0.0f}}, {{0, 0, 0, 255}}},
		{{{1.0f, 0.0f}}, {{255, 0, 0, 255}}},
		{{{1.0f, 1.0f}}, {{0, 0, 255, 255}}},

		{{{1.0f, 1.0f}}, {{0, 0, 255, 255}}},
		{{{0.0f, 1.0f}}, {{0, 255, 0, 255}}},
		{{{0.0f, 0.0f}}, {{0, 0, 0, 255}}}
	};
	dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, (dsAllocator*)&allocator,
		dsGfxBufferUsage_Vertex, (dsGfxMemory)(dsGfxMemory_Static | dsGfxMemory_Draw), vertices,
		sizeof(vertices));
	ASSERT_TRUE(buffer);

	dsVertexFormat format;
	EXPECT_TRUE(dsVertexFormat_initialize(&format));
	EXPECT_TRUE(dsVertexFormat_setAttribEnabled(&format, dsVertexAttrib_Position, true));
	EXPECT_TRUE(dsVertexFormat_setAttribEnabled(&format, dsVertexAttrib_Color, true));
	format.elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32, dsGfxFormat_Float);
	format.elements[dsVertexAttrib_Color].format =
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
	ASSERT_TRUE(dsVertexFormat_computeOffsetsAndSize(&format));

	ASSERT_EQ(sizeof(Vertex), format.size);
	ASSERT_EQ(offsetof(Vertex, position), format.elements[dsVertexAttrib_Position].offset);
	ASSERT_EQ(offsetof(Vertex, color), format.elements[dsVertexAttrib_Color].offset);

	dsDrawGeometry* drawGeometry;
	dsVertexBuffer vertexBuffer = {buffer, 0, 6, format};
	dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS] = {&vertexBuffer, nullptr,
		nullptr, nullptr};
	drawGeometry = dsDrawGeometry_create(resourceManager, (dsAllocator*)&allocator,
		vertexBuffers, nullptr);
	ASSERT_TRUE(drawGeometry);

	dsSurfaceClearValue clearValue;
	clearValue.colorValue.floatValue.r = 1.0f;
	clearValue.colorValue.floatValue.g = 1.0f;
	clearValue.colorValue.floatValue.b = 1.0f;
	clearValue.colorValue.floatValue.a = 1.0f;
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;
	ASSERT_TRUE(dsRenderPass_begin(info.renderPass, commandBuffer, info.framebuffer, NULL,
		&clearValue, 1));
	ASSERT_TRUE(dsShader_bind(info.shader, commandBuffer, info.material, NULL, NULL));

	dsDrawRange drawRange = {6, 1, 0, 0};
	ASSERT_TRUE(dsRenderer_draw(renderer, commandBuffer, drawGeometry, &drawRange,
		dsPrimitiveType_TriangleList));

	EXPECT_TRUE(dsShader_unbind(info.shader, commandBuffer));
	EXPECT_TRUE(dsRenderPass_end(info.renderPass, commandBuffer));

	EXPECT_TRUE(dsTexture_generateMipmaps(info.offscreen, commandBuffer));
	EXPECT_TRUE(dsRenderer_flush(renderer));

	dsColor color;
	dsTexturePosition position = {dsCubeFace_None, 0, 1, 0, 2};
	ASSERT_TRUE(dsTexture_getData(&color, sizeof(color), info.offscreen, &position, 1, 1));
	EXPECT_LT(120, color.r);
	EXPECT_GT(180, color.r);
	EXPECT_LT(120, color.g);
	EXPECT_GT(180, color.g);
	EXPECT_LT(120, color.b);
	EXPECT_GT(180, color.b);
	EXPECT_EQ(255, color.a);

	EXPECT_TRUE(dsDrawGeometry_destroy(drawGeometry));
	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));
}

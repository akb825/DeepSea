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
#include <DeepSea/Render/Resources/GfxFence.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/CommandBuffer.h>
#include <DeepSea/Render/CommandBufferPool.h>
#include <DeepSea/Render/RenderPass.h>
#include <DeepSea/Render/Renderer.h>

#include <algorithm>
#include <thread>
#include <vector>

namespace
{
struct Vertex
{
	dsVector2f position;
	dsColor color;
};

struct RenderInfo
{
	void load(const FixtureBase& fixture)
	{
		dsRenderer* renderer = fixture.renderer;
		dsResourceManager* resourceManager = fixture.resourceManager;
		dsAllocator* allocator = (dsAllocator*)&fixture.allocator;

		if (resourceManager->maxResourceContexts > 0)
		{
			ASSERT_TRUE(dsResourceManager_createResourceContext(resourceManager));
		}

		const uint32_t width = 4, height = 2;

		dsMaterialElement materialElements[] =
		{
			{"Transform", dsMaterialType_UniformBlock, 0, NULL, dsMaterialBinding_Instance, 0}
		};

		materialDesc = dsMaterialDesc_create(resourceManager, allocator, materialElements,
			DS_ARRAY_SIZE(materialElements));
		ASSERT_TRUE(materialDesc);

		uint32_t transformIdx = dsMaterialDesc_findElement(materialDesc, "Transform");
		ASSERT_NE(DS_MATERIAL_UNKNOWN, transformIdx);
		transformId = materialDesc->elements[transformIdx].nameID;

		material = dsMaterial_create(resourceManager, allocator, materialDesc);
		ASSERT_TRUE(material);

		shaderModule = dsShaderModule_loadResource(resourceManager, allocator,
			dsFileResourceType_Embedded, fixture.getShaderPath("WriteOffscreenTransform.mslb"),
			"WriteOffscreenTransform");
		ASSERT_TRUE(shaderModule);

		shader = dsShader_createName(resourceManager, allocator, shaderModule,
			"WriteOffscreen", materialDesc);
		ASSERT_TRUE(shader);

		dsGfxFormat surfaceFormat = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
		dsTextureInfo offscreenInfo = {surfaceFormat, dsTextureDim_2D, width, height, 0, 1, 1};
		auto usageFlags = (dsTextureUsage)(dsTextureUsage_Texture | dsTextureUsage_CopyFrom |
			dsTextureUsage_CopyTo);
		offscreen = dsTexture_createOffscreen(resourceManager, allocator, usageFlags,
			dsGfxMemory_Read, &offscreenInfo, true);
		ASSERT_TRUE(offscreen);

		dsFramebufferSurface surface = {dsGfxSurfaceType_Offscreen, dsCubeFace_None, 0, 0,
			offscreen};
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

		Vertex vertices[2][6] =
		{
			{
				{{{0.0f, 0.0f}}, {{0, 0, 0, 255}}},
				{{{1.0f, 0.0f}}, {{255, 0, 0, 255}}},
				{{{1.0f, 1.0f}}, {{0, 0, 255, 255}}},

				{{{1.0f, 1.0f}}, {{0, 0, 255, 255}}},
				{{{0.0f, 1.0f}}, {{0, 255, 0, 255}}},
				{{{0.0f, 0.0f}}, {{0, 0, 0, 255}}}
			},
			{
				{{{0.0f, 0.0f}}, {{255, 255, 255, 255}}},
				{{{1.0f, 0.0f}}, {{0, 255, 255, 255}}},
				{{{1.0f, 1.0f}}, {{255, 255, 0, 255}}},

				{{{1.0f, 1.0f}}, {{255, 255, 0, 255}}},
				{{{0.0f, 1.0f}}, {{255, 0, 255, 255}}},
				{{{0.0f, 0.0f}}, {{255, 255, 255, 255}}}
			}
		};

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

		vertexBuffer = dsGfxBuffer_create(resourceManager, allocator,
			dsGfxBufferUsage_Vertex,
			(dsGfxMemory)(dsGfxMemory_Static | dsGfxMemory_Draw | dsGfxMemory_GPUOnly),
			vertices, sizeof(vertices));
		ASSERT_TRUE(vertexBuffer);

		for (uint32_t i = 0; i < 2; ++i)
		{
			dsVertexBuffer vertexBufferRef = {vertexBuffer, (uint32_t)(sizeof(*vertices)*i), 6,
				format};
			dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS] = {&vertexBufferRef,
				nullptr, nullptr, nullptr};
			drawGeometry[i] = dsDrawGeometry_create(resourceManager, allocator,
				vertexBuffers, nullptr);
			ASSERT_TRUE(drawGeometry[i]);

			instanceValues[i] = dsSharedMaterialValues_create(allocator, 1);
		}

		dsMatrix44f leftMatrix =
		{{
			{0.5f, 0.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f},
			{0.0f, 0.0f, 0.0f, 1.0f}
		}};

		dsMatrix44f rightMatrix =
		{{
			{0.5f, 0.0f, 0.0f, 0.0f},
			{0.0f, 1.0f, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f},
			{0.55f, 0.0f, 0.0f, 1.0f}
		}};

		uint32_t transformSize = std::max(static_cast<uint32_t>(sizeof(dsMatrix44f)),
			resourceManager->minUniformBlockAlignment);
		std::vector<uint8_t> transformData(transformSize*2);

		dsMatrix44f projection;
		ASSERT_TRUE(dsRenderer_makeOrtho(&projection, renderer, -0.1f, 1.1f, -0.1f, 1.1f, 0.0f,
			1.0f));

		dsMatrix44f* transforms[2] = {reinterpret_cast<dsMatrix44f*>(transformData.data()),
			reinterpret_cast<dsMatrix44f*>(transformData.data() + transformSize)};
		dsMatrix44_mul(*transforms[0], projection, leftMatrix);
		dsMatrix44_mul(*transforms[1], projection, rightMatrix);
		transformBuffer = dsGfxBuffer_create(resourceManager, allocator,
			dsGfxBufferUsage_UniformBlock, (dsGfxMemory)(dsGfxMemory_Static | dsGfxMemory_GPUOnly),
			transformData.data(), transformData.size());
		ASSERT_TRUE(transformBuffer);

		primaryCommands = dsCommandBufferPool_create(renderer, allocator,
			dsCommandBufferUsage_Standard, 1);
		ASSERT_TRUE(primaryCommands);

		secondaryCommands = dsCommandBufferPool_create(renderer, allocator,
			dsCommandBufferUsage_Secondary, 2);
		ASSERT_TRUE(secondaryCommands);

		if (resourceManager->maxResourceContexts > 0)
		{
			ASSERT_TRUE(dsResourceManager_destroyResourceContext(resourceManager));
		}
	}

	void destroy(const FixtureBase& fixture)
	{
		dsResourceManager* resourceManager = fixture.resourceManager;
		if (resourceManager->maxResourceContexts > 0)
		{
			ASSERT_TRUE(dsResourceManager_createResourceContext(resourceManager));
		}

		EXPECT_TRUE(dsCommandBufferPool_destroy(secondaryCommands));
		EXPECT_TRUE(dsCommandBufferPool_destroy(primaryCommands));
		EXPECT_TRUE(dsGfxBuffer_destroy(transformBuffer));
		for (uint32_t i = 0; i < 2; ++i)
		{
			dsSharedMaterialValues_destroy(instanceValues[i]);
			EXPECT_TRUE(dsDrawGeometry_destroy(drawGeometry[i]));
		}
		EXPECT_TRUE(dsGfxBuffer_destroy(vertexBuffer));
		EXPECT_TRUE(dsRenderPass_destroy(renderPass));
		EXPECT_TRUE(dsFramebuffer_destroy(framebuffer));
		EXPECT_TRUE(dsTexture_destroy(offscreen));
		EXPECT_TRUE(dsShader_destroy(shader));
		EXPECT_TRUE(dsShaderModule_destroy(shaderModule));
		dsMaterial_destroy(material);
		EXPECT_TRUE(dsMaterialDesc_destroy(materialDesc));

		if (resourceManager->maxResourceContexts > 0)
		{
			ASSERT_TRUE(dsResourceManager_destroyResourceContext(resourceManager));
		}
	}

	dsMaterialDesc* materialDesc = nullptr;
	dsMaterial* material = nullptr;
	dsShaderModule* shaderModule = nullptr;
	dsShader* shader = nullptr;
	dsOffscreen* offscreen = nullptr;
	dsFramebuffer* framebuffer = nullptr;
	dsRenderPass* renderPass = nullptr;
	dsGfxBuffer* vertexBuffer = nullptr;
	dsDrawGeometry* drawGeometry[2] = {nullptr, nullptr};
	dsSharedMaterialValues* instanceValues[2] = {nullptr, nullptr};
	dsGfxBuffer* transformBuffer = nullptr;
	dsCommandBufferPool* primaryCommands = nullptr;
	dsCommandBufferPool* secondaryCommands = nullptr;
	uint32_t transformId = 0;
};
} // namespace

class ThreadedFunctionalTest : public FixtureBase
{
public:
	void adjustRendererOptions(dsRendererOptions& options) override
	{
		options.maxResourceThreads = 1;
	}
};

INSTANTIATE_TEST_CASE_P(ThreadedFunctional, ThreadedFunctionalTest,
	FixtureBase::getRendererTypes());

TEST_P(ThreadedFunctionalTest, RenderMultithreaded)
{
	if (!(resourceManager->supportedBuffers & dsGfxBufferUsage_UniformBlock))
	{
		DS_LOG_INFO("RenderFunctionalTest", "Uniform blocks not supported: skipping test.");
		return;
	}

	RenderInfo info;
	if (resourceManager->maxResourceContexts > 0)
	{
		std::thread thread([&]()
			{
				info.load(*this);
			});
		thread.join();
	}
	else
		info.load(*this);

	uint32_t transformSize = std::max(static_cast<uint32_t>(sizeof(dsMatrix44f)),
		resourceManager->minUniformBlockAlignment);
	dsSurfaceClearValue clearValue;
	clearValue.colorValue.floatValue.r = 1.0f;
	clearValue.colorValue.floatValue.g = 1.0f;
	clearValue.colorValue.floatValue.b = 1.0f;
	clearValue.colorValue.floatValue.a = 1.0f;
	dsDrawRange drawRange = {6, 1, 0, 0};
	dsCommandBuffer* primaryCommands = info.primaryCommands->currentBuffers[0];
	std::thread renderThread([&]()
		{
			dsCommandBuffer* secondaryCommands = info.secondaryCommands->currentBuffers[0];
			std::thread drawThread([&]()
				{
					ASSERT_TRUE(dsCommandBuffer_beginSecondary(secondaryCommands, info.framebuffer,
						info.renderPass, 0, NULL));
					ASSERT_TRUE(dsSharedMaterialValues_setBufferId(info.instanceValues[0],
						info.transformId, info.transformBuffer, 0, sizeof(dsMatrix44f)));

					ASSERT_TRUE(dsShader_bind(info.shader, secondaryCommands, info.material, NULL,
						NULL));
					ASSERT_TRUE(dsShader_updateInstanceValues(info.shader, secondaryCommands,
						info.instanceValues[0]));
					ASSERT_TRUE(dsRenderer_draw(renderer, secondaryCommands, info.drawGeometry[0],
						&drawRange, dsPrimitiveType_TriangleList));

					ASSERT_TRUE(dsSharedMaterialValues_setBufferId(info.instanceValues[0],
						info.transformId, info.transformBuffer, transformSize,
						sizeof(dsMatrix44f)));
					ASSERT_TRUE(dsShader_updateInstanceValues(info.shader, secondaryCommands,
						info.instanceValues[0]));
					ASSERT_TRUE(dsRenderer_draw(renderer, secondaryCommands, info.drawGeometry[1],
						&drawRange, dsPrimitiveType_TriangleList));

					EXPECT_TRUE(dsShader_unbind(info.shader, secondaryCommands));
					EXPECT_TRUE(dsCommandBuffer_end(secondaryCommands));
				});

			ASSERT_TRUE(dsRenderPass_begin(info.renderPass, primaryCommands, info.framebuffer, NULL,
				&clearValue, 1));

			drawThread.join();
			ASSERT_TRUE(dsCommandBuffer_submit(primaryCommands, secondaryCommands));
			EXPECT_TRUE(dsRenderPass_end(info.renderPass, primaryCommands));
			EXPECT_TRUE(dsCommandBuffer_end(primaryCommands));
		});
	renderThread.join();

	ASSERT_TRUE(dsCommandBuffer_submit(renderer->mainCommandBuffer, primaryCommands));
	EXPECT_TRUE(dsRenderer_flush(renderer));

	dsColor colors[8];
	dsTexturePosition position = {dsCubeFace_None, 0, 0, 0, 0};
	ASSERT_TRUE(dsTexture_getData(colors, sizeof(colors), info.offscreen, &position, 4, 2));
	EXPECT_EQ(0, colors[0].r);
	EXPECT_EQ(255, colors[0].g);
	EXPECT_EQ(0, colors[0].b);
	EXPECT_EQ(255, colors[0].a);

	EXPECT_EQ(0, colors[1].r);
	EXPECT_EQ(0, colors[1].g);
	EXPECT_EQ(255, colors[1].b);
	EXPECT_EQ(255, colors[1].a);

	EXPECT_EQ(255, colors[2].r);
	EXPECT_EQ(0, colors[2].g);
	EXPECT_EQ(255, colors[2].b);
	EXPECT_EQ(255, colors[2].a);

	EXPECT_EQ(255, colors[3].r);
	EXPECT_EQ(255, colors[3].g);
	EXPECT_EQ(0, colors[3].b);
	EXPECT_EQ(255, colors[3].a);

	EXPECT_EQ(0, colors[4].r);
	EXPECT_EQ(0, colors[4].g);
	EXPECT_EQ(0, colors[4].b);
	EXPECT_EQ(255, colors[4].a);

	EXPECT_EQ(255, colors[5].r);
	EXPECT_EQ(0, colors[5].g);
	EXPECT_EQ(0, colors[5].b);
	EXPECT_EQ(255, colors[5].a);

	EXPECT_EQ(255, colors[6].r);
	EXPECT_EQ(255, colors[6].g);
	EXPECT_EQ(255, colors[6].b);
	EXPECT_EQ(255, colors[6].a);

	EXPECT_EQ(0, colors[7].r);
	EXPECT_EQ(255, colors[7].g);
	EXPECT_EQ(255, colors[7].b);
	EXPECT_EQ(255, colors[7].a);

	ASSERT_TRUE(dsCommandBufferPool_reset(info.primaryCommands));
	ASSERT_TRUE(dsCommandBufferPool_reset(info.secondaryCommands));
	primaryCommands = info.primaryCommands->currentBuffers[0];
	renderThread = std::thread([&]()
		{
			dsCommandBuffer* secondaryCommands0 = info.secondaryCommands->currentBuffers[0];
			dsCommandBuffer* secondaryCommands1 = info.secondaryCommands->currentBuffers[1];
			std::thread drawThread([&]()
				{
					ASSERT_TRUE(dsCommandBuffer_beginSecondary(secondaryCommands0, info.framebuffer,
						info.renderPass, 0, NULL));
					ASSERT_TRUE(dsSharedMaterialValues_setBufferId(info.instanceValues[0],
						info.transformId, info.transformBuffer, transformSize,
						sizeof(dsMatrix44f)));

					ASSERT_TRUE(dsShader_bind(info.shader, secondaryCommands0, info.material, NULL
						, NULL));
					ASSERT_TRUE(dsShader_updateInstanceValues(info.shader, secondaryCommands0,
						info.instanceValues[0]));
					ASSERT_TRUE(dsRenderer_draw(renderer, secondaryCommands0, info.drawGeometry[0],
						&drawRange, dsPrimitiveType_TriangleList));

					EXPECT_TRUE(dsShader_unbind(info.shader, secondaryCommands0));
					EXPECT_TRUE(dsCommandBuffer_end(secondaryCommands0));
				});

			ASSERT_TRUE(dsRenderPass_begin(info.renderPass, primaryCommands, info.framebuffer, NULL,
				&clearValue, 1));

			drawThread.join();
			ASSERT_TRUE(dsCommandBuffer_submit(primaryCommands, secondaryCommands0));

			drawThread = std::thread([&]()
				{
					ASSERT_TRUE(dsCommandBuffer_beginSecondary(secondaryCommands1, info.framebuffer,
						info.renderPass, 0, NULL));
					ASSERT_TRUE(dsSharedMaterialValues_setBufferId(info.instanceValues[0],
						info.transformId, info.transformBuffer, 0, sizeof(dsMatrix44f)));

					ASSERT_TRUE(dsShader_bind(info.shader, secondaryCommands1, info.material, NULL,
						NULL));
					ASSERT_TRUE(dsShader_updateInstanceValues(info.shader, secondaryCommands1,
						info.instanceValues[0]));
					ASSERT_TRUE(dsRenderer_draw(renderer, secondaryCommands1, info.drawGeometry[1],
						&drawRange, dsPrimitiveType_TriangleList));

					EXPECT_TRUE(dsShader_unbind(info.shader, secondaryCommands1));
					EXPECT_TRUE(dsCommandBuffer_end(secondaryCommands1));
				});

			drawThread.join();
			ASSERT_TRUE(dsCommandBuffer_submit(primaryCommands, secondaryCommands1));

			EXPECT_TRUE(dsRenderPass_end(info.renderPass, primaryCommands));
			EXPECT_TRUE(dsCommandBuffer_end(primaryCommands));
		});
	renderThread.join();

	ASSERT_TRUE(dsCommandBuffer_submit(renderer->mainCommandBuffer, primaryCommands));
	EXPECT_TRUE(dsRenderer_flush(renderer));

	ASSERT_TRUE(dsTexture_getData(colors, sizeof(colors), info.offscreen, &position, 4, 2));
	EXPECT_EQ(255, colors[0].r);
	EXPECT_EQ(0, colors[0].g);
	EXPECT_EQ(255, colors[0].b);
	EXPECT_EQ(255, colors[0].a);

	EXPECT_EQ(255, colors[1].r);
	EXPECT_EQ(255, colors[1].g);
	EXPECT_EQ(0, colors[1].b);
	EXPECT_EQ(255, colors[1].a);

	EXPECT_EQ(0, colors[2].r);
	EXPECT_EQ(255, colors[2].g);
	EXPECT_EQ(0, colors[2].b);
	EXPECT_EQ(255, colors[2].a);

	EXPECT_EQ(0, colors[3].r);
	EXPECT_EQ(0, colors[3].g);
	EXPECT_EQ(255, colors[3].b);
	EXPECT_EQ(255, colors[3].a);

	EXPECT_EQ(255, colors[4].r);
	EXPECT_EQ(255, colors[4].g);
	EXPECT_EQ(255, colors[4].b);
	EXPECT_EQ(255, colors[4].a);

	EXPECT_EQ(0, colors[5].r);
	EXPECT_EQ(255, colors[5].g);
	EXPECT_EQ(255, colors[5].b);
	EXPECT_EQ(255, colors[5].a);

	EXPECT_EQ(0, colors[6].r);
	EXPECT_EQ(0, colors[6].g);
	EXPECT_EQ(0, colors[6].b);
	EXPECT_EQ(255, colors[6].a);

	EXPECT_EQ(255, colors[7].r);
	EXPECT_EQ(0, colors[7].g);
	EXPECT_EQ(0, colors[7].b);
	EXPECT_EQ(255, colors[7].a);

	if (resourceManager->maxResourceContexts > 0)
	{
		std::thread thread([&]()
			{
				info.destroy(*this);
			});
		thread.join();
	}
	else
		info.destroy(*this);
}

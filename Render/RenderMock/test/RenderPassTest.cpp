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
#include <DeepSea/Render/Resources/Framebuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/RenderPass.h>
#include <DeepSea/Render/RenderSurface.h>
#include <gtest/gtest.h>

class RenderPassTest : public FixtureBase
{
};

TEST_F(RenderPassTest, Create)
{
	dsAttachmentInfo attachments[] =
	{
		{(dsAttachmentUsage)(dsAttachmentUsage_Clear | dsAttachmentUsage_KeepAfter),
			renderer->surfaceDepthStencilFormat, renderer->surfaceSamples},
		{(dsAttachmentUsage)(dsAttachmentUsage_Clear | dsAttachmentUsage_KeepAfter),
			renderer->surfaceColorFormat, renderer->surfaceSamples},
		{dsAttachmentUsage_Clear, renderer->surfaceColorFormat, renderer->surfaceSamples},
		{dsAttachmentUsage_Clear, renderer->surfaceColorFormat, renderer->surfaceSamples}
	};
	uint32_t attachmentCount = (uint32_t)DS_ARRAY_SIZE(attachments);

	dsColorAttachmentRef pass0ColorAttachments[] = {{2, true}};
	dsColorAttachmentRef pass1ColorAttachments[] = {{3, true}};
	uint32_t pass2InputAttachments[] = {2, 3};
	dsColorAttachmentRef pass2ColorAttachments[] = {{1, false}};
	dsRenderSubpassInfo subpasses[] =
	{
		{"test1", NULL, pass0ColorAttachments, 0, DS_ARRAY_SIZE(pass0ColorAttachments),
			DS_NO_ATTACHMENT},
		{"test2", NULL, pass1ColorAttachments, 0, DS_ARRAY_SIZE(pass1ColorAttachments),
			DS_NO_ATTACHMENT},
		{"combine", pass2InputAttachments, pass2ColorAttachments,
			DS_ARRAY_SIZE(pass2InputAttachments), DS_ARRAY_SIZE(pass2ColorAttachments), 0}
	};
	uint32_t subpassCount = (uint32_t)DS_ARRAY_SIZE(subpasses);

	dsSubpassDependency dependencies[] =
	{
		{0, dsSubpassDependencyStage_Fragment, 2, dsSubpassDependencyStage_Fragment, true},
		{1, dsSubpassDependencyStage_Fragment, 2, dsSubpassDependencyStage_Fragment, true}
	};
	uint32_t dependencyCount = (uint32_t)DS_ARRAY_SIZE(dependencies);

	EXPECT_FALSE(dsRenderPass_create(NULL, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount));
	EXPECT_FALSE(dsRenderPass_create(renderer, NULL, NULL, attachmentCount, subpasses, subpassCount,
		dependencies, dependencyCount));
	EXPECT_FALSE(dsRenderPass_create(renderer, NULL, attachments, attachmentCount, NULL,
		subpassCount, dependencies, dependencyCount));
	EXPECT_FALSE(dsRenderPass_create(renderer, NULL, attachments, attachmentCount, subpasses,
		0, dependencies, dependencyCount));
	EXPECT_FALSE(dsRenderPass_create(renderer, NULL, attachments, attachmentCount, NULL,
		subpassCount, NULL, dependencyCount));

	((uint32_t*)subpasses[2].inputAttachments)[0] = 4;
	EXPECT_FALSE(dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount));
	((uint32_t*)subpasses[2].inputAttachments)[0] = 2;

	((uint32_t*)subpasses[2].colorAttachments)[0] = 4;
	EXPECT_FALSE(dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount));
	((uint32_t*)subpasses[2].colorAttachments)[0] = 1;

	((uint32_t*)subpasses[2].colorAttachments)[0] = 0;
	EXPECT_FALSE(dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount));
	((uint32_t*)subpasses[2].colorAttachments)[0] = 1;

	subpasses[2].depthStencilAttachment = 4;
	EXPECT_FALSE(dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount));
	subpasses[2].depthStencilAttachment = 0;

	subpasses[2].depthStencilAttachment = 1;
	EXPECT_FALSE(dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount));
	subpasses[2].depthStencilAttachment = 0;

	dependencies[0].srcSubpass = 4;
	EXPECT_FALSE(dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount));
	dependencies[0].srcSubpass = 0;

	dependencies[0].dstSubpass = 4;
	EXPECT_FALSE(dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount));
	dependencies[0].dstSubpass = 2;

	dsRenderPass* renderPass = dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount);
	ASSERT_TRUE(renderPass);
	EXPECT_TRUE(dsRenderPass_destroy(renderPass));

	renderPass = dsRenderPass_create(renderer, NULL, attachments, attachmentCount, subpasses,
		subpassCount, NULL, 0);
	ASSERT_TRUE(renderPass);
	EXPECT_TRUE(dsRenderPass_destroy(renderPass));
}

TEST_F(RenderPassTest, BeginNextEnd)
{
	dsAttachmentInfo attachments[] =
	{
		{(dsAttachmentUsage)(dsAttachmentUsage_Clear | dsAttachmentUsage_KeepAfter),
			renderer->surfaceDepthStencilFormat, renderer->surfaceSamples},
		{(dsAttachmentUsage)(dsAttachmentUsage_Clear | dsAttachmentUsage_KeepAfter),
			renderer->surfaceColorFormat, renderer->surfaceSamples},
		{dsAttachmentUsage_Clear, renderer->surfaceColorFormat, renderer->surfaceSamples},
		{dsAttachmentUsage_Clear, renderer->surfaceColorFormat, renderer->surfaceSamples}
	};
	uint32_t attachmentCount = (uint32_t)DS_ARRAY_SIZE(attachments);

	dsColorAttachmentRef pass0ColorAttachments[] = {{2, true}};
	dsColorAttachmentRef pass1ColorAttachments[] = {{3, true}};
	uint32_t pass2InputAttachments[] = {2, 3};
	dsColorAttachmentRef pass2ColorAttachments[] = {{1, false}};
	dsRenderSubpassInfo subpasses[] =
	{
		{"test1", NULL, pass0ColorAttachments, 0, DS_ARRAY_SIZE(pass0ColorAttachments),
			DS_NO_ATTACHMENT},
		{"test2", NULL, pass1ColorAttachments, 0, DS_ARRAY_SIZE(pass1ColorAttachments),
			DS_NO_ATTACHMENT},
		{"combine", pass2InputAttachments, pass2ColorAttachments,
			DS_ARRAY_SIZE(pass2InputAttachments), DS_ARRAY_SIZE(pass2ColorAttachments), 0}
	};
	uint32_t subpassCount = (uint32_t)DS_ARRAY_SIZE(subpasses);

	dsSubpassDependency dependencies[] =
	{
		{0, dsSubpassDependencyStage_Fragment, 2, dsSubpassDependencyStage_Fragment, true},
		{1, dsSubpassDependencyStage_Fragment, 2, dsSubpassDependencyStage_Fragment, true}
	};
	uint32_t dependencyCount = (uint32_t)DS_ARRAY_SIZE(dependencies);

	dsRenderPass* renderPass = dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount);
	ASSERT_TRUE(renderPass);

	dsRenderSurface* renderSurface = dsRenderSurface_create(renderer, NULL, NULL,
		dsRenderSurfaceType_Unknown);
	ASSERT_TRUE(renderSurface);

	dsOffscreen* offscreen1 = dsTexture_createOffscreen(resourceManager, 0,
		dsTextureUsage_SubpassInput, dsGfxMemory_GpuOnly, renderer->surfaceColorFormat,
		dsTextureDim_2D, renderSurface->width, renderSurface->height, 0, 0,
		renderer->surfaceSamples, true);
	ASSERT_TRUE(offscreen1);

	dsOffscreen* offscreen2 = dsTexture_createOffscreen(resourceManager, 0,
		dsTextureUsage_SubpassInput, dsGfxMemory_GpuOnly, renderer->surfaceColorFormat,
		dsTextureDim_2D, renderSurface->width, renderSurface->height, 0, 0,
		renderer->surfaceSamples, true);
	ASSERT_TRUE(offscreen2);

	dsOffscreen* offscreen3 = dsTexture_createOffscreen(resourceManager, 0,
		dsTextureUsage_SubpassInput, dsGfxMemory_GpuOnly, renderer->surfaceDepthStencilFormat,
		dsTextureDim_2D, renderSurface->width, renderSurface->height, 0, 0,
		renderer->surfaceSamples, true);
	ASSERT_TRUE(offscreen3);

	dsFramebufferSurface surfaces1[] =
	{
		{dsFramebufferSurfaceType_DepthRenderSurface, dsCubeFace_PosX, 0, 0, renderSurface},
		{dsFramebufferSurfaceType_ColorRenderSurface, dsCubeFace_PosX, 0, 0, renderSurface},
		{dsFramebufferSurfaceType_Offscreen, dsCubeFace_PosX, 0, 0, offscreen1},
		{dsFramebufferSurfaceType_Offscreen, dsCubeFace_PosX, 0, 0, offscreen2}
	};
	uint32_t surface1Count = (uint32_t)DS_ARRAY_SIZE(surfaces1);

	dsFramebufferSurface surfaces2[] =
	{
		{dsFramebufferSurfaceType_DepthRenderSurface, dsCubeFace_PosX, 0, 0, renderSurface},
		{dsFramebufferSurfaceType_Offscreen, dsCubeFace_PosX, 0, 0, offscreen1},
		{dsFramebufferSurfaceType_ColorRenderSurface, dsCubeFace_PosX, 0, 0, renderSurface},
		{dsFramebufferSurfaceType_Offscreen, dsCubeFace_PosX, 0, 0, offscreen2}
	};
	uint32_t surface2Count = (uint32_t)DS_ARRAY_SIZE(surfaces2);

	dsFramebuffer* framebuffer1 = dsFramebuffer_create(resourceManager, NULL, surfaces1,
		surface1Count, renderSurface->width, renderSurface->height, 1);
	ASSERT_TRUE(framebuffer1);

	dsFramebuffer* framebuffer2 = dsFramebuffer_create(resourceManager, NULL, surfaces1, 2,
		renderSurface->width, renderSurface->height, 1);
	ASSERT_TRUE(framebuffer2);

	surfaces1[3].surface = offscreen3;
	dsFramebuffer* framebuffer3 = dsFramebuffer_create(resourceManager, NULL, surfaces1,
		surface1Count, renderSurface->width, renderSurface->height, 1);
	ASSERT_TRUE(framebuffer3);

	dsFramebuffer* framebuffer4 = dsFramebuffer_create(resourceManager, NULL, surfaces2,
		surface2Count, renderSurface->width, renderSurface->height, 1);
	ASSERT_TRUE(framebuffer4);

	dsSurfaceClearValue clearValues[4];
	clearValues[0].depthStencil.depth = 1.0f;
	clearValues[0].depthStencil.stencil = 0;
	clearValues[1].colorValue.floatValue.r = 0.0f;
	clearValues[1].colorValue.floatValue.g = 0.0f;
	clearValues[1].colorValue.floatValue.b = 0.0f;
	clearValues[1].colorValue.floatValue.a = 1.0f;
	uint32_t clearValueCount = DS_ARRAY_SIZE(clearValues);

	dsAlignedBox3f validViewport =
	{
		{{0.0f, 0.0f, 0.0f}},
		{{(float)renderSurface->width, (float)renderSurface->height, 0.0f}},
	};

	dsAlignedBox3f invalidViewport =
	{
		{{0.0f, 0.0f, 0.0f}},
		{{(float)renderSurface->width + 10, (float)renderSurface->height, 0.0f}},
	};

	EXPECT_FALSE(dsRenderPass_begin(NULL, renderPass, framebuffer1, NULL, clearValues,
		clearValueCount, false));
	EXPECT_FALSE(dsRenderPass_begin(renderer->mainCommandBuffer, NULL, framebuffer1, NULL,
		clearValues, clearValueCount, false));
	EXPECT_FALSE(dsRenderPass_begin(renderer->mainCommandBuffer, renderPass, NULL, NULL,
		clearValues, clearValueCount, false));
	EXPECT_FALSE(dsRenderPass_begin(renderer->mainCommandBuffer, renderPass, framebuffer1, NULL,
		NULL, 0, false));
	EXPECT_FALSE(dsRenderPass_begin(renderer->mainCommandBuffer, renderPass, framebuffer1, NULL,
		clearValues, 2, false));
	EXPECT_FALSE(dsRenderPass_begin(renderer->mainCommandBuffer, renderPass, framebuffer2, NULL,
		clearValues, clearValueCount, false));
	EXPECT_FALSE(dsRenderPass_begin(renderer->mainCommandBuffer, renderPass, framebuffer3, NULL,
		clearValues, clearValueCount, false));
	EXPECT_FALSE(dsRenderPass_begin(renderer->mainCommandBuffer, renderPass, framebuffer1,
		&invalidViewport, clearValues, clearValueCount, false));

	EXPECT_TRUE(dsRenderPass_begin(renderer->mainCommandBuffer, renderPass, framebuffer1, NULL,
		clearValues, clearValueCount, false));
	EXPECT_FALSE(dsRenderPass_nextSubpass(NULL, renderPass, false));
	EXPECT_TRUE(dsRenderPass_nextSubpass(renderer->mainCommandBuffer, renderPass, false));
	EXPECT_TRUE(dsRenderPass_nextSubpass(renderer->mainCommandBuffer, renderPass, false));
	EXPECT_FALSE(dsRenderPass_end(NULL, renderPass));
	EXPECT_FALSE(dsRenderPass_end(renderer->mainCommandBuffer, NULL));
	EXPECT_TRUE(dsRenderPass_end(renderer->mainCommandBuffer, renderPass));

	EXPECT_TRUE(dsRenderPass_begin(renderer->mainCommandBuffer, renderPass, framebuffer1,
		&validViewport, clearValues, clearValueCount, false));
	EXPECT_TRUE(dsRenderPass_nextSubpass(renderer->mainCommandBuffer, renderPass, false));
	EXPECT_TRUE(dsRenderPass_nextSubpass(renderer->mainCommandBuffer, renderPass, false));
	EXPECT_TRUE(dsRenderPass_end(renderer->mainCommandBuffer, renderPass));

	EXPECT_TRUE(dsRenderPass_begin(renderer->mainCommandBuffer, renderPass, framebuffer4,
		&validViewport, clearValues, clearValueCount, false));
	EXPECT_TRUE(dsRenderPass_nextSubpass(renderer->mainCommandBuffer, renderPass, false));
	EXPECT_TRUE(dsRenderPass_nextSubpass(renderer->mainCommandBuffer, renderPass, false));
	EXPECT_TRUE(dsRenderPass_end(renderer->mainCommandBuffer, renderPass));

	resourceManager->canMixWithRenderSurface = false;
	EXPECT_FALSE(dsRenderPass_begin(renderer->mainCommandBuffer, renderPass, framebuffer4,
		&validViewport, clearValues, clearValueCount, false));

	EXPECT_TRUE(dsRenderPass_destroy(renderPass));
	EXPECT_TRUE(dsFramebuffer_destroy(framebuffer1));
	EXPECT_TRUE(dsFramebuffer_destroy(framebuffer2));
	EXPECT_TRUE(dsFramebuffer_destroy(framebuffer3));
	EXPECT_TRUE(dsFramebuffer_destroy(framebuffer4));
	EXPECT_TRUE(dsTexture_destroy(offscreen1));
	EXPECT_TRUE(dsTexture_destroy(offscreen2));
	EXPECT_TRUE(dsTexture_destroy(offscreen3));
	EXPECT_TRUE(dsRenderSurface_destroy(renderSurface));
}

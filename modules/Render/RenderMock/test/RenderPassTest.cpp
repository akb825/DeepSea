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

#include "Fixtures/FixtureBase.h"
#include <DeepSea/Render/Resources/Framebuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/RenderPass.h>
#include <DeepSea/Render/RenderSurface.h>
#include <gtest/gtest.h>

class RenderPassTest : public FixtureBase
{
};

TEST_F(RenderPassTest, DefaultDependencies)
{
	// uint32_t input0[] = {};
	dsAttachmentRef color0[] = {{0, false}, {1, false}};
	dsAttachmentRef depthStencil0 = {2, false};
	// uint32_t input1[] = {};
	dsAttachmentRef color1[] = {{3, false}, {4, false}};
	dsAttachmentRef depthStencil1 = {DS_NO_ATTACHMENT, false};
	uint32_t input2[] = {1};
	dsAttachmentRef color2[] = {{0, false}};
	dsAttachmentRef depthStencil2 = {2, false};
	uint32_t input3[] = {1};
	dsAttachmentRef color3[] = {{4, false}};
	dsAttachmentRef depthStencil3 = {DS_NO_ATTACHMENT, false};
	// uint32_t input4[] = {};
	dsAttachmentRef color4[] = {{5, true}};
	dsAttachmentRef depthStencil4 = {6, false};
	uint32_t input5[] = {0, 2, 4};
	dsAttachmentRef color5[] = {{7, true}};
	dsAttachmentRef depthStencil5 = {DS_NO_ATTACHMENT, false};

	dsRenderSubpassInfo subpasses[] =
	{
		{
			"0",
			nullptr, //input0,
			color0,
			depthStencil0,
			0, //DS_ARRAY_SIZE(input0),
			DS_ARRAY_SIZE(color0)
		},
		{
			"1",
			nullptr, //input1,
			color1,
			depthStencil1,
			0, //DS_ARRAY_SIZE(input1),
			DS_ARRAY_SIZE(color1)
		},
		{
			"2",
			input2,
			color2,
			depthStencil2,
			DS_ARRAY_SIZE(input2),
			DS_ARRAY_SIZE(color2)
		},
		{
			"3",
			input3,
			color3,
			depthStencil3,
			DS_ARRAY_SIZE(input3),
			DS_ARRAY_SIZE(color3)
		},
		{
			"4",
			nullptr, // input4,
			color4,
			depthStencil4,
			0, // DS_ARRAY_SIZE(input4),
			DS_ARRAY_SIZE(color4)
		},
		{
			"5",
			input5,
			color5,
			depthStencil5,
			DS_ARRAY_SIZE(input5),
			DS_ARRAY_SIZE(color5)
		}
	};
	uint32_t subpassCount = DS_ARRAY_SIZE(subpasses);

	constexpr uint32_t dependencyCount = 12;
	ASSERT_EQ(dependencyCount, dsRenderPass_countDefaultDependencies(subpasses, subpassCount));

	// One higher to guarantee no out of bounds access when testing limit check.
	dsSubpassDependency dependencies[dependencyCount + 1];
	EXPECT_FALSE(dsRenderPass_setDefaultDependencies(NULL, dependencyCount, subpasses,
		subpassCount));
	EXPECT_FALSE(dsRenderPass_setDefaultDependencies(dependencies, dependencyCount, NULL,
		subpassCount));
	EXPECT_FALSE(dsRenderPass_setDefaultDependencies(dependencies, dependencyCount - 1, subpasses,
		subpassCount));
	EXPECT_FALSE(dsRenderPass_setDefaultDependencies(dependencies, dependencyCount + 1, subpasses,
		subpassCount));
	EXPECT_TRUE(dsRenderPass_setDefaultDependencies(dependencies, dependencyCount, subpasses,
		subpassCount));

	dsSubpassDependency referenceFirstDependency;
	memset(&referenceFirstDependency, 0, sizeof(dsSubpassDependency));
	referenceFirstDependency.srcSubpass = DS_EXTERNAL_SUBPASS;
	referenceFirstDependency.dstSubpass = 0;
	ASSERT_TRUE(dsRenderPass_addFirstSubpassDependencyFlags(&referenceFirstDependency));

	dsSubpassDependency referenceLastDependency;
	memset(&referenceLastDependency, 0, sizeof(dsSubpassDependency));
	referenceLastDependency.srcSubpass = 0;
	referenceLastDependency.dstSubpass = DS_EXTERNAL_SUBPASS;
	ASSERT_TRUE(dsRenderPass_addLastSubpassDependencyFlags(&referenceLastDependency));

	dsSubpassDependency* dependency = dependencies;
	// Render subpass 0 external dependency.
	EXPECT_EQ(DS_EXTERNAL_SUBPASS, dependency->srcSubpass);
	EXPECT_EQ(referenceFirstDependency.srcStages, dependency->srcStages);
	EXPECT_EQ(referenceFirstDependency.srcAccess, dependency->srcAccess);
	EXPECT_EQ(0U, dependency->dstSubpass);
	EXPECT_EQ(referenceFirstDependency.dstStages, dependency->dstStages);
	EXPECT_EQ(referenceFirstDependency.dstAccess, dependency->dstAccess);
	EXPECT_FALSE(dependency->regionDependency);
	++dependency;

	// Render subpass 1 external dependency.
	EXPECT_EQ(DS_EXTERNAL_SUBPASS, dependency->srcSubpass);
	EXPECT_EQ(referenceFirstDependency.srcStages, dependency->srcStages);
	EXPECT_EQ(referenceFirstDependency.srcAccess, dependency->srcAccess);
	EXPECT_EQ(1U, dependency->dstSubpass);
	EXPECT_EQ(referenceFirstDependency.dstStages, dependency->dstStages);
	EXPECT_EQ(referenceFirstDependency.dstAccess, dependency->dstAccess);
	EXPECT_FALSE(dependency->regionDependency);
	++dependency;

	// Input, color, and depth/stencil dependencies for render subpass 0 to 2.
	EXPECT_EQ(0U, dependency->srcSubpass);
	EXPECT_EQ(dsGfxPipelineStage_ColorOutput | dsGfxPipelineStage_PreFragmentShaderTests |
			dsGfxPipelineStage_PostFragmentShaderTests,
		dependency->srcStages);
	EXPECT_EQ(dsGfxAccess_ColorAttachmentRead | dsGfxAccess_ColorAttachmentWrite |
			dsGfxAccess_DepthStencilAttachmentRead | dsGfxAccess_DepthStencilAttachmentWrite,
		dependency->srcAccess);
	EXPECT_EQ(2U, dependency->dstSubpass);
	EXPECT_EQ(dsGfxPipelineStage_FragmentShader | dsGfxPipelineStage_ColorOutput |
			dsGfxPipelineStage_PreFragmentShaderTests | dsGfxPipelineStage_PostFragmentShaderTests,
		dependency->dstStages);
	EXPECT_EQ(dsGfxAccess_InputAttachmentRead | dsGfxAccess_ColorAttachmentRead |
			dsGfxAccess_ColorAttachmentWrite | dsGfxAccess_DepthStencilAttachmentRead |
			dsGfxAccess_DepthStencilAttachmentWrite,
		dependency->dstAccess);
	EXPECT_TRUE(dependency->regionDependency);
	++dependency;

	// Input dependencies for render subpass 0 to 3.
	EXPECT_EQ(0U, dependency->srcSubpass);
	EXPECT_EQ(dsGfxPipelineStage_ColorOutput, dependency->srcStages);
	EXPECT_EQ(dsGfxAccess_ColorAttachmentWrite, dependency->srcAccess);
	EXPECT_EQ(3U, dependency->dstSubpass);
	EXPECT_EQ(dsGfxPipelineStage_FragmentShader, dependency->dstStages);
	EXPECT_EQ(dsGfxAccess_InputAttachmentRead, dependency->dstAccess);
	EXPECT_TRUE(dependency->regionDependency);
	++dependency;

	// Color dependency for render subpass 1 to 3.
	EXPECT_EQ(1U, dependency->srcSubpass);
	EXPECT_EQ(dsGfxPipelineStage_ColorOutput, dependency->srcStages);
	EXPECT_EQ(dsGfxAccess_ColorAttachmentRead | dsGfxAccess_ColorAttachmentWrite,
		dependency->srcAccess);
	EXPECT_EQ(3U, dependency->dstSubpass);
	EXPECT_EQ(dsGfxPipelineStage_ColorOutput, dependency->dstStages);
	EXPECT_EQ(dsGfxAccess_ColorAttachmentRead | dsGfxAccess_ColorAttachmentWrite,
		dependency->dstAccess);
	EXPECT_TRUE(dependency->regionDependency);
	++dependency;

	// Render subpass 4 external dependency.
	EXPECT_EQ(DS_EXTERNAL_SUBPASS, dependency->srcSubpass);
	EXPECT_EQ(referenceFirstDependency.srcStages, dependency->srcStages);
	EXPECT_EQ(referenceFirstDependency.srcAccess, dependency->srcAccess);
	EXPECT_EQ(4U, dependency->dstSubpass);
	EXPECT_EQ(referenceFirstDependency.dstStages, dependency->dstStages);
	EXPECT_EQ(referenceFirstDependency.dstAccess, dependency->dstAccess);
	EXPECT_FALSE(dependency->regionDependency);
	++dependency;

	// Render subpass 4 external dependency.
	EXPECT_EQ(4U, dependency->srcSubpass);
	EXPECT_EQ(referenceLastDependency.srcStages, dependency->srcStages);
	EXPECT_EQ(referenceLastDependency.srcAccess, dependency->srcAccess);
	EXPECT_EQ(DS_EXTERNAL_SUBPASS, dependency->dstSubpass);
	EXPECT_EQ(referenceLastDependency.dstStages, dependency->dstStages);
	EXPECT_EQ(referenceLastDependency.dstAccess, dependency->dstAccess);
	EXPECT_FALSE(dependency->regionDependency);
	++dependency;

	// Input dependency for render subpass 0 to 5.
	EXPECT_EQ(0U, dependency->srcSubpass);
	EXPECT_EQ(dsGfxPipelineStage_ColorOutput | dsGfxPipelineStage_PreFragmentShaderTests |
		dsGfxPipelineStage_PostFragmentShaderTests, dependency->srcStages);
	EXPECT_EQ(dsGfxAccess_ColorAttachmentWrite | dsGfxAccess_DepthStencilAttachmentWrite,
		dependency->srcAccess);
	EXPECT_EQ(5U, dependency->dstSubpass);
	EXPECT_EQ(dsGfxPipelineStage_FragmentShader, dependency->dstStages);
	EXPECT_EQ(dsGfxAccess_InputAttachmentRead, dependency->dstAccess);
	EXPECT_TRUE(dependency->regionDependency);
	++dependency;

	// Input dependency for render subpass 1 to 5.
	EXPECT_EQ(1U, dependency->srcSubpass);
	EXPECT_EQ(dsGfxPipelineStage_ColorOutput, dependency->srcStages);
	EXPECT_EQ(dsGfxAccess_ColorAttachmentWrite, dependency->srcAccess);
	EXPECT_EQ(5U, dependency->dstSubpass);
	EXPECT_EQ(dsGfxPipelineStage_FragmentShader, dependency->dstStages);
	EXPECT_EQ(dsGfxAccess_InputAttachmentRead, dependency->dstAccess);
	EXPECT_TRUE(dependency->regionDependency);
	++dependency;

	// Input dependency for render subpass 2 to 5.
	EXPECT_EQ(2U, dependency->srcSubpass);
	EXPECT_EQ(dsGfxPipelineStage_ColorOutput | dsGfxPipelineStage_PreFragmentShaderTests |
		dsGfxPipelineStage_PostFragmentShaderTests, dependency->srcStages);
	EXPECT_EQ(dsGfxAccess_ColorAttachmentWrite | dsGfxAccess_DepthStencilAttachmentWrite,
		dependency->srcAccess);
	EXPECT_EQ(5U, dependency->dstSubpass);
	EXPECT_EQ(dsGfxPipelineStage_FragmentShader, dependency->dstStages);
	EXPECT_EQ(dsGfxAccess_InputAttachmentRead, dependency->dstAccess);
	EXPECT_TRUE(dependency->regionDependency);
	++dependency;

	// Input dependency for render subpass 3 to 5.
	EXPECT_EQ(3U, dependency->srcSubpass);
	EXPECT_EQ(dsGfxPipelineStage_ColorOutput, dependency->srcStages);
	EXPECT_EQ(dsGfxAccess_ColorAttachmentWrite, dependency->srcAccess);
	EXPECT_EQ(5U, dependency->dstSubpass);
	EXPECT_EQ(dsGfxPipelineStage_FragmentShader, dependency->dstStages);
	EXPECT_EQ(dsGfxAccess_InputAttachmentRead, dependency->dstAccess);
	EXPECT_TRUE(dependency->regionDependency);
	++dependency;

	// Render subpass 4 external dependency.
	EXPECT_EQ(5U, dependency->srcSubpass);
	EXPECT_EQ(referenceLastDependency.srcStages, dependency->srcStages);
	EXPECT_EQ(referenceLastDependency.srcAccess, dependency->srcAccess);
	EXPECT_EQ(DS_EXTERNAL_SUBPASS, dependency->dstSubpass);
	EXPECT_EQ(referenceLastDependency.dstStages, dependency->dstStages);
	EXPECT_EQ(referenceLastDependency.dstAccess, dependency->dstAccess);
	EXPECT_FALSE(dependency->regionDependency);
	++dependency;

	EXPECT_EQ(dependencies + dependencyCount, dependency);
}

TEST_F(RenderPassTest, Create)
{
	dsAttachmentInfo attachments[] =
	{
		{dsAttachmentUsage_Clear | dsAttachmentUsage_KeepAfter, renderer->surfaceDepthStencilFormat,
			DS_DEFAULT_ANTIALIAS_SAMPLES},
		{dsAttachmentUsage_Clear | dsAttachmentUsage_KeepAfter, renderer->surfaceColorFormat,
			DS_DEFAULT_ANTIALIAS_SAMPLES},
		{dsAttachmentUsage_Clear, renderer->surfaceColorFormat, DS_DEFAULT_ANTIALIAS_SAMPLES},
		{dsAttachmentUsage_Clear, renderer->surfaceColorFormat, DS_DEFAULT_ANTIALIAS_SAMPLES}
	};
	uint32_t attachmentCount = DS_ARRAY_SIZE(attachments);

	dsAttachmentRef pass0ColorAttachments[] = {{2, true}};
	dsAttachmentRef pass1ColorAttachments[] = {{3, true}};
	uint32_t pass2InputAttachments[] = {2, 3};
	dsAttachmentRef pass2ColorAttachments[] = {{1, false}};
	dsRenderSubpassInfo subpasses[] =
	{
		{"test1", NULL, pass0ColorAttachments, {DS_NO_ATTACHMENT, false}, 0,
			DS_ARRAY_SIZE(pass0ColorAttachments)},
		{"test2", NULL, pass1ColorAttachments, {DS_NO_ATTACHMENT, false}, 0,
			DS_ARRAY_SIZE(pass1ColorAttachments)},
		{"combine", pass2InputAttachments, pass2ColorAttachments, {0, true},
			DS_ARRAY_SIZE(pass2InputAttachments), DS_ARRAY_SIZE(pass2ColorAttachments)}
	};
	uint32_t subpassCount = DS_ARRAY_SIZE(subpasses);

	dsSubpassDependency dependencies[] =
	{
		{DS_EXTERNAL_SUBPASS, (dsGfxPipelineStage)0, dsGfxAccess_None, 0, (dsGfxPipelineStage)0,
			dsGfxAccess_None, false},
		{0, dsGfxPipelineStage_ColorOutput | dsGfxPipelineStage_PostFragmentShaderTests,
			dsGfxAccess_ColorAttachmentWrite | dsGfxAccess_DepthStencilAttachmentWrite, 2,
			dsGfxPipelineStage_FragmentShader, dsGfxAccess_InputAttachmentRead, true},
		{1, dsGfxPipelineStage_ColorOutput | dsGfxPipelineStage_PostFragmentShaderTests,
			dsGfxAccess_ColorAttachmentWrite | dsGfxAccess_DepthStencilAttachmentWrite, 2,
			dsGfxPipelineStage_FragmentShader, dsGfxAccess_InputAttachmentRead, true}
	};
	EXPECT_TRUE(dsRenderPass_addFirstSubpassDependencyFlags(dependencies + 0));
	uint32_t dependencyCount = DS_ARRAY_SIZE(dependencies);

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

	subpasses[2].depthStencilAttachment.attachmentIndex = 4;
	EXPECT_FALSE(dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount));
	subpasses[2].depthStencilAttachment.attachmentIndex = 0;

	subpasses[2].depthStencilAttachment.attachmentIndex = 1;
	EXPECT_FALSE(dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount));
	subpasses[2].depthStencilAttachment.attachmentIndex = 0;

	renderer->hasDepthStencilMultisampleResolve = false;
	EXPECT_FALSE(dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount));
	renderer->hasDepthStencilMultisampleResolve = true;;

	dependencies[1].srcSubpass = 4;
	EXPECT_FALSE(dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount));
	dependencies[1].srcSubpass = 0;

	dependencies[1].srcSubpass = 3;
	EXPECT_FALSE(dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount));
	dependencies[1].srcSubpass = 2;

	dependencies[1].dstSubpass = 4;
	EXPECT_FALSE(dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount));
	dependencies[1].dstSubpass = 2;

	attachments[0].samples = 2;
	EXPECT_FALSE(dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount));
	attachments[0].samples = DS_DEFAULT_ANTIALIAS_SAMPLES;

	dsRenderPass* renderPass = dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount);
	ASSERT_TRUE(renderPass);
	EXPECT_TRUE(dsRenderPass_destroy(renderPass));

	renderPass = dsRenderPass_create(renderer, NULL, attachments, attachmentCount, subpasses,
		subpassCount, NULL, 0);
	ASSERT_TRUE(renderPass);
	EXPECT_EQ(0U, renderPass->subpassDependencyCount);
	EXPECT_TRUE(dsRenderPass_destroy(renderPass));

	renderPass = dsRenderPass_create(renderer, NULL, attachments, attachmentCount, subpasses,
		subpassCount, NULL, DS_DEFAULT_SUBPASS_DEPENDENCIES);
	ASSERT_TRUE(renderPass);
	EXPECT_EQ(dsRenderPass_countDefaultDependencies(subpasses, subpassCount),
		renderPass->subpassDependencyCount);
	EXPECT_TRUE(dsRenderPass_destroy(renderPass));
}

TEST_F(RenderPassTest, BeginNextEnd)
{
	dsAttachmentInfo attachments[] =
	{
		{dsAttachmentUsage_Clear, renderer->surfaceDepthStencilFormat,
			renderer->surfaceSamples},
		{dsAttachmentUsage_Clear | dsAttachmentUsage_KeepAfter, renderer->surfaceColorFormat,
			renderer->surfaceSamples},
		{dsAttachmentUsage_Clear, renderer->surfaceColorFormat, renderer->surfaceSamples},
		{dsAttachmentUsage_Clear, renderer->surfaceColorFormat, renderer->surfaceSamples}
	};
	uint32_t attachmentCount = DS_ARRAY_SIZE(attachments);

	dsAttachmentRef pass0ColorAttachments[] = {{2, true}};
	dsAttachmentRef pass1ColorAttachments[] = {{3, true}};
	uint32_t pass2InputAttachments[] = {2, 3};
	dsAttachmentRef pass2ColorAttachments[] = {{1, false}};
	dsRenderSubpassInfo subpasses[] =
	{
		{"test1", NULL, pass0ColorAttachments, {DS_NO_ATTACHMENT, false}, 0,
			DS_ARRAY_SIZE(pass0ColorAttachments)},
		{"test2", NULL, pass1ColorAttachments, {DS_NO_ATTACHMENT, false}, 0,
			DS_ARRAY_SIZE(pass1ColorAttachments)},
		{"combine", pass2InputAttachments, pass2ColorAttachments, {0, false},
			DS_ARRAY_SIZE(pass2InputAttachments), DS_ARRAY_SIZE(pass2ColorAttachments)}
	};
	uint32_t subpassCount = DS_ARRAY_SIZE(subpasses);

	dsSubpassDependency dependencies[] =
	{
		{0, dsGfxPipelineStage_ColorOutput | dsGfxPipelineStage_PostFragmentShaderTests,
			dsGfxAccess_ColorAttachmentWrite | dsGfxAccess_DepthStencilAttachmentWrite, 2,
			dsGfxPipelineStage_FragmentShader, dsGfxAccess_InputAttachmentRead, true},
		{1, dsGfxPipelineStage_ColorOutput | dsGfxPipelineStage_PostFragmentShaderTests,
			dsGfxAccess_ColorAttachmentWrite | dsGfxAccess_DepthStencilAttachmentWrite, 2,
			dsGfxPipelineStage_FragmentShader, dsGfxAccess_InputAttachmentRead, true}
	};
	uint32_t dependencyCount = DS_ARRAY_SIZE(dependencies);

	dsRenderPass* renderPass = dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, dependencies, dependencyCount);
	ASSERT_TRUE(renderPass);

	dsRenderSurface* renderSurface = dsRenderSurface_create(renderer, NULL, "test", NULL,
		dsRenderSurfaceType_Direct, dsRenderSurfaceUsage_Standard);
	ASSERT_TRUE(renderSurface);

	dsTextureInfo colorInfo = {renderer->surfaceColorFormat, dsTextureDim_2D, renderSurface->width,
		renderSurface->height, 0, 0, renderer->surfaceSamples};
	dsOffscreen* offscreen1 = dsTexture_createOffscreen(resourceManager, 0,
		dsTextureUsage_SubpassInput, dsGfxMemory_GPUOnly, &colorInfo, true);
	ASSERT_TRUE(offscreen1);

	dsOffscreen* offscreen2 = dsTexture_createOffscreen(resourceManager, 0,
		dsTextureUsage_SubpassInput, dsGfxMemory_GPUOnly, &colorInfo, true);
	ASSERT_TRUE(offscreen2);

	dsTextureInfo depthInfo = {renderer->surfaceDepthStencilFormat, dsTextureDim_2D,
		renderSurface->width, renderSurface->height, 0, 0, renderer->surfaceSamples};
	dsOffscreen* offscreen3 = dsTexture_createOffscreen(resourceManager, 0,
		dsTextureUsage_SubpassInput, dsGfxMemory_GPUOnly, &depthInfo, true);
	ASSERT_TRUE(offscreen3);

	dsFramebufferSurface surfaces1[] =
	{
		{dsGfxSurfaceType_DepthRenderSurface, dsCubeFace_None, 0, 0, renderSurface},
		{dsGfxSurfaceType_ColorRenderSurface, dsCubeFace_None, 0, 0, renderSurface},
		{dsGfxSurfaceType_Offscreen, dsCubeFace_None, 0, 0, offscreen1},
		{dsGfxSurfaceType_Offscreen, dsCubeFace_None, 0, 0, offscreen2}
	};
	uint32_t surface1Count = DS_ARRAY_SIZE(surfaces1);

	dsFramebufferSurface surfaces2[] =
	{
		{dsGfxSurfaceType_Offscreen, dsCubeFace_None, 0, 0, offscreen3},
		{dsGfxSurfaceType_ColorRenderSurface, dsCubeFace_None, 0, 0, renderSurface},
		{dsGfxSurfaceType_Offscreen, dsCubeFace_None, 0, 0, offscreen1},
		{dsGfxSurfaceType_Offscreen, dsCubeFace_None, 0, 0, offscreen2}
	};
	uint32_t surface2Count = DS_ARRAY_SIZE(surfaces2);

	dsFramebuffer* framebuffer1 = dsFramebuffer_create(resourceManager, NULL, "test", surfaces1,
		surface1Count, renderSurface->width, renderSurface->height, 1);
	ASSERT_TRUE(framebuffer1);

	dsFramebuffer* framebuffer2 = dsFramebuffer_create(resourceManager, NULL, "test", surfaces1, 2,
		renderSurface->width, renderSurface->height, 1);
	ASSERT_TRUE(framebuffer2);

	surfaces1[3].surface = offscreen3;
	dsFramebuffer* framebuffer3 = dsFramebuffer_create(resourceManager, NULL, "test", surfaces1,
		surface1Count, renderSurface->width, renderSurface->height, 1);
	ASSERT_TRUE(framebuffer3);

	dsFramebuffer* framebuffer4 = dsFramebuffer_create(resourceManager, NULL, "test", surfaces2,
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

	EXPECT_FALSE(dsRenderPass_begin(renderPass, NULL, framebuffer1, NULL, clearValues,
		clearValueCount, false));
	EXPECT_FALSE(dsRenderPass_begin(NULL, renderer->mainCommandBuffer, framebuffer1, NULL,
		clearValues, clearValueCount, false));
	EXPECT_FALSE(dsRenderPass_begin(renderPass, renderer->mainCommandBuffer, NULL, NULL,
		clearValues, clearValueCount, false));
	EXPECT_FALSE(dsRenderPass_begin(renderPass, renderer->mainCommandBuffer, framebuffer1, NULL,
		NULL, 0, false));
	EXPECT_FALSE(dsRenderPass_begin(renderPass, renderer->mainCommandBuffer, framebuffer1, NULL,
		clearValues, 2, false));
	EXPECT_FALSE(dsRenderPass_begin(renderPass, renderer->mainCommandBuffer, framebuffer2, NULL,
		clearValues, clearValueCount, false));
	EXPECT_FALSE(dsRenderPass_begin(renderPass, renderer->mainCommandBuffer, framebuffer3, NULL,
		clearValues, clearValueCount, false));
	EXPECT_FALSE(dsRenderPass_begin(renderPass, renderer->mainCommandBuffer, framebuffer1,
		&invalidViewport, clearValues, clearValueCount, false));

	EXPECT_TRUE(dsRenderPass_begin(renderPass, renderer->mainCommandBuffer, framebuffer1, NULL,
		clearValues, clearValueCount, false));
	EXPECT_FALSE(dsRenderPass_nextSubpass(renderPass, NULL, false));
	EXPECT_TRUE(dsRenderPass_nextSubpass(renderPass, renderer->mainCommandBuffer, false));
	EXPECT_TRUE(dsRenderPass_nextSubpass(renderPass, renderer->mainCommandBuffer, false));
	EXPECT_FALSE(dsRenderPass_end(renderPass, NULL));
	EXPECT_FALSE(dsRenderPass_end(NULL, renderer->mainCommandBuffer));
	EXPECT_TRUE(dsRenderPass_end(renderPass, renderer->mainCommandBuffer));

	EXPECT_TRUE(dsRenderPass_begin(renderPass, renderer->mainCommandBuffer, framebuffer1,
		&validViewport, clearValues, clearValueCount, false));
	EXPECT_TRUE(dsRenderPass_nextSubpass(renderPass, renderer->mainCommandBuffer, false));
	EXPECT_TRUE(dsRenderPass_nextSubpass(renderPass, renderer->mainCommandBuffer, false));
	EXPECT_TRUE(dsRenderPass_end(renderPass, renderer->mainCommandBuffer));

	EXPECT_TRUE(dsRenderPass_begin(renderPass, renderer->mainCommandBuffer, framebuffer4,
		&validViewport, clearValues, clearValueCount, false));
	EXPECT_FALSE(dsRenderPass_end(renderPass, renderer->mainCommandBuffer));
	EXPECT_TRUE(dsRenderPass_nextSubpass(renderPass, renderer->mainCommandBuffer, false));
	EXPECT_TRUE(dsRenderPass_nextSubpass(renderPass, renderer->mainCommandBuffer, false));
	EXPECT_FALSE(dsRenderPass_nextSubpass(renderPass, renderer->mainCommandBuffer, false));
	EXPECT_TRUE(dsRenderPass_end(renderPass, renderer->mainCommandBuffer));

	resourceManager->canMixWithRenderSurface = false;
	EXPECT_FALSE(dsRenderPass_begin(renderPass, renderer->mainCommandBuffer, framebuffer4,
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

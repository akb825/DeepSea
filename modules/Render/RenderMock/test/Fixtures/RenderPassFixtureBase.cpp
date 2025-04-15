/*
 * Copyright 2018-2019 Aaron Barany
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

#include "RenderPassFixtureBase.h"
#include <DeepSea/Render/Resources/Framebuffer.h>
#include <DeepSea/Render/RenderPass.h>
#include <DeepSea/Render/RenderSurface.h>

void RenderPassFixtureBase::SetUp()
{
	FixtureBase::SetUp();

	dsAttachmentInfo attachments[] =
	{
		{dsAttachmentUsage_Standard, renderer->surfaceDepthStencilFormat,
			DS_DEFAULT_ANTIALIAS_SAMPLES},
		{dsAttachmentUsage_KeepAfter, renderer->surfaceColorFormat,
			DS_DEFAULT_ANTIALIAS_SAMPLES}
	};
	uint32_t attachmentCount = DS_ARRAY_SIZE(attachments);
	dsAttachmentRef colorAttachments[] = {{1, true}};
	dsRenderSubpassInfo subpasses[] =
	{
		{"test1", NULL, colorAttachments, {0, false}, 0, DS_ARRAY_SIZE(colorAttachments)}
	};
	renderPass = dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, 1, NULL, 0);
	ASSERT_TRUE(renderPass);

	renderSurface = dsRenderSurface_create(renderer, NULL, "test", NULL, NULL,
		dsRenderSurfaceType_Direct, dsRenderSurfaceUsage_Standard, 1920, 1080);
	ASSERT_TRUE(renderSurface);

	dsFramebufferSurface surfaces[] =
	{
		{dsGfxSurfaceType_DepthRenderSurface, dsCubeFace_None, 0, 0, renderSurface},
		{dsGfxSurfaceType_ColorRenderSurface, dsCubeFace_None, 0, 0, renderSurface},
	};
	framebuffer = dsFramebuffer_create(resourceManager, NULL, "test", surfaces,
		DS_ARRAY_SIZE(surfaces), renderSurface->width, renderSurface->height, 1);
	ASSERT_TRUE(framebuffer);
}

void RenderPassFixtureBase::TearDown()
{
	EXPECT_TRUE(dsRenderPass_destroy(renderPass));
	EXPECT_TRUE(dsFramebuffer_destroy(framebuffer));
	EXPECT_TRUE(dsRenderSurface_destroy(renderSurface));
	FixtureBase::TearDown();
}

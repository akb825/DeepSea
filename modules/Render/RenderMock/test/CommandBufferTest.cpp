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
#include <DeepSea/Render/RenderPass.h>
#include <DeepSea/Render/CommandBuffer.h>
#include <DeepSea/Render/CommandBufferPool.h>

class CommandBufferTest : public FixtureBase
{
};

TEST_F(CommandBufferTest, BeginEnd)
{
	dsCommandBufferPool* pool = dsCommandBufferPool_create(renderer, NULL,
		dsCommandBufferUsage_Standard);
	ASSERT_TRUE(pool);

	EXPECT_FALSE(dsCommandBuffer_begin(NULL));
	EXPECT_FALSE(dsCommandBuffer_end(NULL));
	EXPECT_FALSE(dsCommandBuffer_begin(renderer->mainCommandBuffer));
	EXPECT_FALSE(dsCommandBuffer_end(renderer->mainCommandBuffer));

	dsCommandBuffer** commandBuffer = dsCommandBufferPool_createCommandBuffers(pool, 1);
	ASSERT_TRUE(commandBuffer);
	EXPECT_TRUE(dsCommandBuffer_begin(*commandBuffer));
	EXPECT_TRUE(dsCommandBuffer_end(*commandBuffer));

	EXPECT_TRUE(dsCommandBufferPool_destroy(pool));
}

TEST_F(CommandBufferTest, BeginEndSecondary)
{
	dsAttachmentInfo attachments[] =
	{
		{dsAttachmentUsage_Clear | dsAttachmentUsage_KeepAfter, renderer->surfaceColorFormat,
			DS_DEFAULT_ANTIALIAS_SAMPLES}
	};
	uint32_t attachmentCount = DS_ARRAY_SIZE(attachments);

	dsAttachmentRef colorAttachments[] = {{0, true}};
	dsRenderSubpassInfo subpasses[] =
	{
		{"test", NULL, colorAttachments, {DS_NO_ATTACHMENT, false}, 0,
			DS_ARRAY_SIZE(colorAttachments)},
	};
	uint32_t subpassCount = DS_ARRAY_SIZE(subpasses);

	dsRenderPass* renderPass = dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, NULL, DS_DEFAULT_SUBPASS_DEPENDENCIES);
	ASSERT_TRUE(renderPass);

	dsCommandBufferPool* pool = dsCommandBufferPool_create(renderer, NULL,
		dsCommandBufferUsage_Secondary);
	ASSERT_TRUE(pool);

	dsCommandBuffer** commandBuffer = dsCommandBufferPool_createCommandBuffers(pool, 1);
	ASSERT_TRUE(commandBuffer);

	dsAlignedBox3f viewport = {{{0.0f, 0.0f, 0.0f}}, {{10.0f, 15.0f, 1.0f}}};

	EXPECT_FALSE(dsCommandBuffer_begin(*commandBuffer));
	EXPECT_FALSE(dsCommandBuffer_beginSecondary(*commandBuffer, NULL, NULL, 0, &viewport));
	EXPECT_FALSE(dsCommandBuffer_beginSecondary(*commandBuffer, NULL, renderPass, 1, &viewport));
	EXPECT_FALSE(dsCommandBuffer_beginSecondary(*commandBuffer, NULL, renderPass, 1, NULL));

	EXPECT_TRUE(dsCommandBuffer_beginSecondary(*commandBuffer, NULL, renderPass, 0, &viewport));
	EXPECT_TRUE(dsCommandBuffer_end(*commandBuffer));

	EXPECT_TRUE(dsCommandBufferPool_destroy(pool));
	EXPECT_TRUE(dsRenderPass_destroy(renderPass));
}

TEST_F(CommandBufferTest, Submit)
{
	dsCommandBufferPool* pool = dsCommandBufferPool_create(renderer, NULL,
		dsCommandBufferUsage_Standard);
	ASSERT_TRUE(pool);

	dsCommandBuffer** commandBuffer = dsCommandBufferPool_createCommandBuffers(pool, 1);
	ASSERT_TRUE(commandBuffer);

	dsCommandBufferPool* otherPool = dsCommandBufferPool_create(renderer, NULL,
		dsCommandBufferUsage_MultiSubmit);
	ASSERT_TRUE(otherPool);

	dsCommandBuffer** otherCommandBuffer = dsCommandBufferPool_createCommandBuffers(otherPool, 1);
	ASSERT_TRUE(otherCommandBuffer);

	EXPECT_FALSE(dsCommandBuffer_submit(NULL, NULL));
	EXPECT_FALSE(dsCommandBuffer_submit(*commandBuffer, NULL));
	EXPECT_FALSE(dsCommandBuffer_submit(NULL, *commandBuffer));
	EXPECT_FALSE(dsCommandBuffer_submit(*commandBuffer, renderer->mainCommandBuffer));
	EXPECT_TRUE(dsCommandBuffer_submit(renderer->mainCommandBuffer, *commandBuffer));

	EXPECT_TRUE(dsCommandBuffer_submit(*commandBuffer, *otherCommandBuffer));
	EXPECT_FALSE(dsCommandBuffer_submit(*otherCommandBuffer, *commandBuffer));

	EXPECT_TRUE(dsCommandBufferPool_destroy(pool));
	EXPECT_TRUE(dsCommandBufferPool_destroy(otherPool));
}

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
		dsCommandBufferUsage_Standard, 1);
	ASSERT_TRUE(pool);

	EXPECT_FALSE(dsCommandBuffer_begin(NULL));
	EXPECT_FALSE(dsCommandBuffer_end(NULL));
	EXPECT_FALSE(dsCommandBuffer_begin(renderer->mainCommandBuffer));
	EXPECT_FALSE(dsCommandBuffer_end(renderer->mainCommandBuffer));

	EXPECT_TRUE(dsCommandBuffer_begin(pool->currentBuffers[0]));
	EXPECT_TRUE(dsCommandBuffer_end(pool->currentBuffers[0]));

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

	dsColorAttachmentRef colorAttachments[] = {{0, true}};
	dsRenderSubpassInfo subpasses[] =
	{
		{"test", NULL, colorAttachments, 0, DS_ARRAY_SIZE(colorAttachments), DS_NO_ATTACHMENT},
	};
	uint32_t subpassCount = DS_ARRAY_SIZE(subpasses);

	dsRenderPass* renderPass = dsRenderPass_create(renderer, NULL, attachments, attachmentCount,
		subpasses, subpassCount, NULL, DS_DEFAULT_SUBPASS_DEPENDENCIES);
	ASSERT_TRUE(renderPass);

	dsCommandBufferPool* pool = dsCommandBufferPool_create(renderer, NULL,
		dsCommandBufferUsage_Secondary, 1);
	ASSERT_TRUE(pool);

	dsAlignedBox3f viewport = {{{0.0f, 0.0f, 0.0f}}, {{10.0f, 15.0f, 1.0f}}};

	EXPECT_FALSE(dsCommandBuffer_begin(pool->currentBuffers[0]));
	EXPECT_FALSE(dsCommandBuffer_beginSecondary(pool->currentBuffers[0], NULL, NULL, 0, &viewport));
	EXPECT_FALSE(dsCommandBuffer_beginSecondary(pool->currentBuffers[0], NULL, renderPass,
		1, &viewport));
	EXPECT_FALSE(dsCommandBuffer_beginSecondary(pool->currentBuffers[0], NULL, renderPass,
		1, NULL));

	EXPECT_TRUE(dsCommandBuffer_beginSecondary(pool->currentBuffers[0], NULL, renderPass,
		0, &viewport));
	EXPECT_TRUE(dsCommandBuffer_end(pool->currentBuffers[0]));

	EXPECT_TRUE(dsCommandBufferPool_destroy(pool));
	EXPECT_TRUE(dsRenderPass_destroy(renderPass));
}

TEST_F(CommandBufferTest, Submit)
{
	dsCommandBufferPool* pool = dsCommandBufferPool_create(renderer, NULL,
		dsCommandBufferUsage_Standard, 1);
	ASSERT_TRUE(pool);

	dsCommandBufferPool* otherPool = dsCommandBufferPool_create(renderer, NULL,
		dsCommandBufferUsage_MultiSubmit, 1);
	ASSERT_TRUE(otherPool);

	EXPECT_FALSE(dsCommandBuffer_submit(NULL, NULL));
	EXPECT_FALSE(dsCommandBuffer_submit(pool->currentBuffers[0], NULL));
	EXPECT_FALSE(dsCommandBuffer_submit(NULL, pool->currentBuffers[0]));
	EXPECT_FALSE(dsCommandBuffer_submit(pool->currentBuffers[0], renderer->mainCommandBuffer));
	EXPECT_TRUE(dsCommandBuffer_submit(renderer->mainCommandBuffer, pool->currentBuffers[0]));

	EXPECT_TRUE(dsCommandBuffer_submit(pool->currentBuffers[0], otherPool->currentBuffers[0]));
	EXPECT_FALSE(dsCommandBuffer_submit(otherPool->currentBuffers[0], pool->currentBuffers[0]));

	EXPECT_TRUE(dsCommandBufferPool_destroy(pool));
	EXPECT_TRUE(dsCommandBufferPool_destroy(otherPool));
}

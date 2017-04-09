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
#include <DeepSea/Render/CommandBuffer.h>
#include <DeepSea/Render/CommandBufferPool.h>

class CommandBufferTest : public FixtureBase
{
};

TEST_F(CommandBufferTest, BeginEnd)
{
	dsCommandBufferPool* pool = dsCommandBufferPool_create(renderer, NULL, 0, 1);
	ASSERT_TRUE(pool);

	EXPECT_FALSE(dsCommandBuffer_begin(NULL, NULL, 0, NULL));
	EXPECT_FALSE(dsCommandBuffer_end(NULL));
	EXPECT_FALSE(dsCommandBuffer_begin(renderer->mainCommandBuffer, NULL, 0, NULL));
	EXPECT_FALSE(dsCommandBuffer_end(renderer->mainCommandBuffer));

	EXPECT_TRUE(dsCommandBuffer_begin(pool->currentBuffers[0], NULL, 0, NULL));
	EXPECT_TRUE(dsCommandBuffer_end(pool->currentBuffers[0]));

	EXPECT_TRUE(dsCommandBufferPool_destroy(pool));
}

TEST_F(CommandBufferTest, Submit)
{
	dsCommandBufferPool* pool = dsCommandBufferPool_create(renderer, NULL, 0, 1);
	ASSERT_TRUE(pool);

	EXPECT_FALSE(dsCommandBuffer_submit(NULL, NULL));
	EXPECT_FALSE(dsCommandBuffer_submit(pool->currentBuffers[0], NULL));
	EXPECT_FALSE(dsCommandBuffer_submit(NULL, pool->currentBuffers[0]));
	EXPECT_FALSE(dsCommandBuffer_submit(pool->currentBuffers[0], renderer->mainCommandBuffer));
	EXPECT_TRUE(dsCommandBuffer_submit(renderer->mainCommandBuffer, pool->currentBuffers[0]));

	EXPECT_TRUE(dsCommandBufferPool_destroy(pool));
}

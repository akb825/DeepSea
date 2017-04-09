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
#include <DeepSea/Render/CommandBufferPool.h>

class CommandBufferPoolTest : public FixtureBase
{
};

TEST_F(CommandBufferPoolTest, Create)
{
	EXPECT_FALSE(dsCommandBufferPool_create(NULL, NULL, 0, 4));
	EXPECT_FALSE(dsCommandBufferPool_create(renderer, NULL, 0, 0));

	dsCommandBufferPool* pool = dsCommandBufferPool_create(renderer, NULL, 0, 4);
	ASSERT_TRUE(pool);

	EXPECT_TRUE(pool->currentBuffers);
	EXPECT_FALSE(pool->otherBuffers);

	EXPECT_TRUE(dsCommandBufferPool_destroy(pool));

	pool = dsCommandBufferPool_create(renderer, NULL, dsCommandBufferUsage_DoubleBuffer, 4);
	ASSERT_TRUE(pool);

	EXPECT_TRUE(pool->currentBuffers);
	EXPECT_TRUE(pool->otherBuffers);

	EXPECT_TRUE(dsCommandBufferPool_destroy(pool));
}

TEST_F(CommandBufferPoolTest, Reset)
{
	dsCommandBufferPool* pool = dsCommandBufferPool_create(renderer, NULL, 0, 4);
	ASSERT_TRUE(pool);

	EXPECT_FALSE(dsCommandBufferPool_reset(NULL));
	EXPECT_TRUE(dsCommandBufferPool_reset(pool));

	EXPECT_TRUE(pool->currentBuffers);
	EXPECT_FALSE(pool->otherBuffers);

	EXPECT_TRUE(dsCommandBufferPool_destroy(pool));

	pool = dsCommandBufferPool_create(renderer, NULL, dsCommandBufferUsage_DoubleBuffer, 4);
	ASSERT_TRUE(pool);

	dsCommandBuffer** currentBuffers = pool->currentBuffers;
	dsCommandBuffer** otherBuffers = pool->otherBuffers;

	EXPECT_TRUE(dsCommandBufferPool_reset(pool));
	EXPECT_EQ(otherBuffers, pool->currentBuffers);
	EXPECT_EQ(currentBuffers, pool->otherBuffers);

	EXPECT_TRUE(dsCommandBufferPool_destroy(pool));
}

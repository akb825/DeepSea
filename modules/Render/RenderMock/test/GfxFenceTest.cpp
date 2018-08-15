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
#include <DeepSea/Render/Resources/GfxFence.h>
#include <DeepSea/Render/CommandBufferPool.h>
#include <gtest/gtest.h>

class GfxFenceTest : public FixtureBase
{
};

TEST_F(GfxFenceTest, Create)
{
	EXPECT_FALSE(dsGfxFence_create(NULL, NULL));
	dsGfxFence* fence = dsGfxFence_create(resourceManager, NULL);

	EXPECT_TRUE(fence);
	EXPECT_EQ(1U, resourceManager->fenceCount);
	EXPECT_TRUE(dsGfxFence_destroy(fence));
	EXPECT_EQ(0U, resourceManager->fenceCount);

	resourceManager->hasFences = false;
	EXPECT_FALSE(dsGfxFence_create(resourceManager, NULL));
	EXPECT_EQ(0U, resourceManager->fenceCount);
}

TEST_F(GfxFenceTest, Set)
{
	dsCommandBufferPool* pool1 = dsCommandBufferPool_create(renderer, NULL, 0, 1);
	ASSERT_TRUE(pool1);

	dsCommandBufferPool* pool2 = dsCommandBufferPool_create(renderer, NULL,
		dsCommandBufferUsage_MultiSubmit, 1);
	ASSERT_TRUE(pool2);

	dsGfxFence* fence = dsGfxFence_create(resourceManager, NULL);
	ASSERT_TRUE(fence);

	EXPECT_FALSE(dsGfxFence_set(fence, NULL, false));
	EXPECT_FALSE(dsGfxFence_set(NULL, renderer->mainCommandBuffer, false));
	EXPECT_TRUE(dsGfxFence_set(fence, renderer->mainCommandBuffer, false));

	EXPECT_FALSE(dsGfxFence_reset(NULL));
	EXPECT_TRUE(dsGfxFence_reset(fence));
	EXPECT_TRUE(dsGfxFence_set(fence, pool1->currentBuffers[0], false));

	EXPECT_TRUE(dsGfxFence_reset(fence));
	EXPECT_FALSE(dsGfxFence_set(fence, pool2->currentBuffers[0], false));

	EXPECT_TRUE(dsGfxFence_destroy(fence));
	EXPECT_TRUE(dsCommandBufferPool_destroy(pool1));
	EXPECT_TRUE(dsCommandBufferPool_destroy(pool2));
}

TEST_F(GfxFenceTest, SetMultiple)
{
	dsCommandBufferPool* pool1 = dsCommandBufferPool_create(renderer, NULL, 0, 1);
	ASSERT_TRUE(pool1);

	dsCommandBufferPool* pool2 = dsCommandBufferPool_create(renderer, NULL,
		dsCommandBufferUsage_MultiSubmit, 1);
	ASSERT_TRUE(pool2);

	dsGfxFence* fence1 = dsGfxFence_create(resourceManager, NULL);
	ASSERT_TRUE(fence1);
	dsGfxFence* fence2 = dsGfxFence_create(resourceManager, NULL);
	ASSERT_TRUE(fence2);

	dsGfxFence* fences[] = {fence1, fence2};
	uint32_t fenceCount = DS_ARRAY_SIZE(fences);

	EXPECT_FALSE(dsGfxFence_setMultiple(NULL, fences, fenceCount, false));
	EXPECT_FALSE(dsGfxFence_setMultiple(renderer->mainCommandBuffer, NULL, fenceCount, false));
	fences[0] = NULL;
	EXPECT_FALSE(dsGfxFence_setMultiple(renderer->mainCommandBuffer, fences, fenceCount, false));
	fences[0] = fence1;

	EXPECT_TRUE(dsGfxFence_setMultiple(renderer->mainCommandBuffer, NULL, 0, false));
	EXPECT_TRUE(dsGfxFence_setMultiple(renderer->mainCommandBuffer, fences, fenceCount, false));

	EXPECT_TRUE(dsGfxFence_reset(fence1));
	EXPECT_TRUE(dsGfxFence_reset(fence2));
	EXPECT_TRUE(dsGfxFence_setMultiple(pool1->currentBuffers[0], fences, fenceCount, false));

	EXPECT_TRUE(dsGfxFence_reset(fence1));
	EXPECT_TRUE(dsGfxFence_reset(fence2));
	EXPECT_FALSE(dsGfxFence_setMultiple(pool2->currentBuffers[0], fences, fenceCount, false));

	EXPECT_TRUE(dsGfxFence_destroy(fence1));
	EXPECT_TRUE(dsGfxFence_destroy(fence2));
	EXPECT_TRUE(dsCommandBufferPool_destroy(pool1));
	EXPECT_TRUE(dsCommandBufferPool_destroy(pool2));
}

TEST_F(GfxFenceTest, Wait)
{
	dsGfxFence* fence = dsGfxFence_create(resourceManager, NULL);
	ASSERT_TRUE(fence);

	EXPECT_TRUE(dsGfxFence_set(fence, renderer->mainCommandBuffer, false));
	EXPECT_EQ(dsGfxFenceResult_Error, dsGfxFence_wait(NULL, 0));
	EXPECT_EQ(dsGfxFenceResult_Success, dsGfxFence_wait(fence, 0));

	EXPECT_TRUE(dsGfxFence_destroy(fence));
}

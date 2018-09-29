/*
 * Copyright 2018 Aaron Barany
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

#include "Fixtures/RenderPassFixtureBase.h"
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxQueryPool.h>
#include <DeepSea/Render/RenderPass.h>
#include <gtest/gtest.h>

class GfxQueryPoolTest : public RenderPassFixtureBase
{
};

TEST_F(GfxQueryPoolTest, Create)
{
	EXPECT_FALSE(dsGfxQueryPool_create(NULL, NULL, dsGfxQueryType_SamplesPassed, 10));
	EXPECT_FALSE(dsGfxQueryPool_create(resourceManager, NULL, dsGfxQueryType_SamplesPassed, 0));

	dsGfxQueryPool* queries = dsGfxQueryPool_create(resourceManager, NULL,
		dsGfxQueryType_SamplesPassed, 10);
	EXPECT_TRUE(queries);
	EXPECT_TRUE(dsGfxQueryPool_destroy(queries));

	queries = dsGfxQueryPool_create(resourceManager, NULL, dsGfxQueryType_AnySamplesPassed, 10);
	EXPECT_TRUE(queries);
	EXPECT_TRUE(dsGfxQueryPool_destroy(queries));

	queries = dsGfxQueryPool_create(resourceManager, NULL, dsGfxQueryType_Timestamp, 10);
	EXPECT_TRUE(queries);
	EXPECT_TRUE(dsGfxQueryPool_destroy(queries));

	resourceManager->timestampPeriod = 0.0f;
	EXPECT_FALSE(dsGfxQueryPool_create(resourceManager, NULL, dsGfxQueryType_Timestamp, 10));

	resourceManager->timestampPeriod = 1.0f;
	resourceManager->hasQueries = false;
	EXPECT_FALSE(dsGfxQueryPool_create(resourceManager, NULL, dsGfxQueryType_SamplesPassed, 10));
	EXPECT_FALSE(dsGfxQueryPool_create(resourceManager, NULL, dsGfxQueryType_AnySamplesPassed, 10));
	EXPECT_FALSE(dsGfxQueryPool_create(resourceManager, NULL, dsGfxQueryType_Timestamp, 10));
}

TEST_F(GfxQueryPoolTest, Reset)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;
	dsGfxQueryPool* queries = dsGfxQueryPool_create(resourceManager, NULL,
		dsGfxQueryType_SamplesPassed, 10);
	ASSERT_TRUE(queries);

	EXPECT_FALSE(dsGfxQueryPool_reset(NULL, commandBuffer, 0, 10));
	EXPECT_FALSE(dsGfxQueryPool_reset(queries, NULL, 0, 10));
	EXPECT_FALSE(dsGfxQueryPool_reset(queries, commandBuffer, 3, 10));
	EXPECT_TRUE(dsGfxQueryPool_reset(queries, commandBuffer, 0, 10));

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0, false));
	EXPECT_FALSE(dsGfxQueryPool_reset(queries, commandBuffer, 0, 10));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	EXPECT_TRUE(dsGfxQueryPool_destroy(queries));
}

TEST_F(GfxQueryPoolTest, BeginEndQuery)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;
	dsGfxQueryPool* queries = dsGfxQueryPool_create(resourceManager, NULL,
		dsGfxQueryType_SamplesPassed, 1);
	ASSERT_TRUE(queries);

	EXPECT_FALSE(dsGfxQueryPool_beginQuery(NULL, commandBuffer, 0));
	EXPECT_FALSE(dsGfxQueryPool_beginQuery(queries, NULL, 0));
	EXPECT_FALSE(dsGfxQueryPool_beginQuery(queries, commandBuffer, 1));
	EXPECT_TRUE(dsGfxQueryPool_beginQuery(queries, commandBuffer, 0));

	EXPECT_FALSE(dsGfxQueryPool_endQuery(NULL, commandBuffer, 0));
	EXPECT_FALSE(dsGfxQueryPool_endQuery(queries, NULL, 0));
	EXPECT_FALSE(dsGfxQueryPool_endQuery(queries, commandBuffer, 1));
	EXPECT_TRUE(dsGfxQueryPool_endQuery(queries, commandBuffer, 0));

	EXPECT_TRUE(dsGfxQueryPool_destroy(queries));

	queries = dsGfxQueryPool_create(resourceManager, NULL, dsGfxQueryType_Timestamp, 1);
	ASSERT_TRUE(queries);
	EXPECT_FALSE(dsGfxQueryPool_beginQuery(queries, commandBuffer, 0));
	EXPECT_FALSE(dsGfxQueryPool_endQuery(queries, commandBuffer, 0));
	EXPECT_TRUE(dsGfxQueryPool_destroy(queries));
}

TEST_F(GfxQueryPoolTest, QueryTimestamp)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;
	dsGfxQueryPool* queries = dsGfxQueryPool_create(resourceManager, NULL, dsGfxQueryType_Timestamp,
		1);
	ASSERT_TRUE(queries);

	EXPECT_FALSE(dsGfxQueryPool_queryTimestamp(NULL, commandBuffer, 0));
	EXPECT_FALSE(dsGfxQueryPool_queryTimestamp(queries, NULL, 0));
	EXPECT_FALSE(dsGfxQueryPool_queryTimestamp(queries, commandBuffer, 1));
	EXPECT_TRUE(dsGfxQueryPool_queryTimestamp(queries, commandBuffer, 0));

	EXPECT_TRUE(dsGfxQueryPool_destroy(queries));

	queries = dsGfxQueryPool_create(resourceManager, NULL, dsGfxQueryType_SamplesPassed, 1);
	ASSERT_TRUE(queries);
	EXPECT_FALSE(dsGfxQueryPool_queryTimestamp(queries, commandBuffer, 0));
	EXPECT_TRUE(dsGfxQueryPool_destroy(queries));
}

TEST_F(GfxQueryPoolTest, GetValues)
{
	dsGfxQueryPool* queries = dsGfxQueryPool_create(resourceManager, NULL,
		dsGfxQueryType_SamplesPassed, 10);
	ASSERT_TRUE(queries);

	uint64_t data[6];
	const size_t stride = 2*sizeof(uint64_t);

	EXPECT_FALSE(dsGfxQueryPool_getValues(NULL, 2, 3, data, sizeof(data), stride, sizeof(uint32_t),
		false));
	EXPECT_FALSE(dsGfxQueryPool_getValues(queries, 2, 3, NULL, sizeof(data), stride,
		sizeof(uint32_t), false));
	EXPECT_FALSE(dsGfxQueryPool_getValues(queries, 2, 3, data, sizeof(data), 1, sizeof(uint32_t),
		false));
	EXPECT_FALSE(dsGfxQueryPool_getValues(queries, 2, 3, data, sizeof(data), stride - 1,
		sizeof(uint32_t), false));
	EXPECT_FALSE(dsGfxQueryPool_getValues(queries, 2, 3, data, sizeof(data), stride,
		sizeof(uint16_t), false));
	EXPECT_TRUE(dsGfxQueryPool_getValues(queries, 2, 3, data, sizeof(data), stride,
		sizeof(uint32_t), false));
	EXPECT_FALSE(dsGfxQueryPool_getValues(queries, 2, 3, data, sizeof(data), sizeof(uint32_t),
		sizeof(uint32_t), true));
	EXPECT_TRUE(dsGfxQueryPool_getValues(queries, 2, 3, data, sizeof(data), stride,
		sizeof(uint32_t), true));

	EXPECT_TRUE(dsGfxQueryPool_getValues(queries, 2, 3, data, sizeof(data), stride,
		sizeof(uint64_t), false));
	EXPECT_FALSE(dsGfxQueryPool_getValues(queries, 2, 3, data, sizeof(data), sizeof(uint64_t),
		sizeof(uint64_t), true));
	EXPECT_TRUE(dsGfxQueryPool_getValues(queries, 2, 3, data, sizeof(data), stride,
		sizeof(uint64_t), true));

	resourceManager->has64BitQueries = false;
	EXPECT_FALSE(dsGfxQueryPool_getValues(queries, 2, 3, data, sizeof(data), stride,
		sizeof(uint64_t), false));
	EXPECT_FALSE(dsGfxQueryPool_getValues(queries, 2, 3, data, sizeof(data), stride,
		sizeof(uint64_t), true));

	EXPECT_TRUE(dsGfxQueryPool_destroy(queries));
}

TEST_F(GfxQueryPoolTest, CopyValues)
{
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;
	dsGfxQueryPool* queries = dsGfxQueryPool_create(resourceManager, NULL,
		dsGfxQueryType_SamplesPassed, 10);
	ASSERT_TRUE(queries);

	dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, NULL,
		(dsGfxBufferUsage)(dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_CopyTo),
		dsGfxMemory_GPUOnly, NULL, 10*sizeof(uint64_t));
	ASSERT_TRUE(buffer);

	const size_t stride = 2*sizeof(uint64_t);
	EXPECT_FALSE(dsGfxQueryPool_copyValues(NULL, commandBuffer, 2, 3, buffer, 4, stride,
		sizeof(uint32_t), false));
	EXPECT_FALSE(dsGfxQueryPool_copyValues(queries, NULL, 2, 3, buffer, 4, stride,
		sizeof(uint32_t), false));
	EXPECT_FALSE(dsGfxQueryPool_copyValues(queries, commandBuffer, 10, 3, buffer, 4, stride,
		sizeof(uint32_t), false));
	EXPECT_FALSE(dsGfxQueryPool_copyValues(queries, commandBuffer, 2, 3, buffer, 100, stride,
		sizeof(uint32_t), false));
	EXPECT_FALSE(dsGfxQueryPool_copyValues(queries, commandBuffer, 2, 3, buffer, 3, stride,
		sizeof(uint32_t), false));
	EXPECT_FALSE(dsGfxQueryPool_copyValues(queries, commandBuffer, 2, 3, buffer, 4, stride - 1,
		sizeof(uint32_t), false));
	EXPECT_FALSE(dsGfxQueryPool_copyValues(queries, commandBuffer, 2, 3, buffer, 4, stride,
		sizeof(uint16_t), false));
	EXPECT_TRUE(dsGfxQueryPool_copyValues(queries, commandBuffer, 2, 3, buffer, 4, stride,
		sizeof(uint32_t), false));
	EXPECT_FALSE(dsGfxQueryPool_copyValues(queries, commandBuffer, 2, 3, buffer, 4,
		sizeof(uint32_t), sizeof(uint32_t), true));
	EXPECT_TRUE(dsGfxQueryPool_copyValues(queries, commandBuffer, 2, 3, buffer, 4, stride,
		sizeof(uint32_t), true));

	EXPECT_TRUE(dsGfxQueryPool_copyValues(queries, commandBuffer, 2, 3, buffer, 8, stride,
		sizeof(uint64_t), false));
	EXPECT_FALSE(dsGfxQueryPool_copyValues(queries, commandBuffer, 2, 3, buffer, 8,
		sizeof(uint64_t), sizeof(uint64_t), true));
	EXPECT_TRUE(dsGfxQueryPool_copyValues(queries, commandBuffer, 2, 3, buffer, 8, stride,
		sizeof(uint64_t), true));

	resourceManager->has64BitQueries = false;
	EXPECT_FALSE(dsGfxQueryPool_copyValues(queries, commandBuffer, 2, 3, buffer, 8, stride,
		sizeof(uint64_t), false));
	EXPECT_FALSE(dsGfxQueryPool_copyValues(queries, commandBuffer, 2, 3, buffer, 8, stride,
		sizeof(uint64_t), true));

	EXPECT_TRUE(dsRenderPass_begin(renderPass, commandBuffer, framebuffer, NULL, NULL, 0, false));
	EXPECT_FALSE(dsGfxQueryPool_copyValues(queries, commandBuffer, 2, 3, buffer, 4, stride,
		sizeof(uint32_t), false));
	EXPECT_TRUE(dsRenderPass_end(renderPass, commandBuffer));

	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));

	buffer = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_UniformBlock,
		dsGfxMemory_GPUOnly, NULL, 10*sizeof(uint64_t));
	ASSERT_TRUE(buffer);

	EXPECT_FALSE(dsGfxQueryPool_copyValues(queries, commandBuffer, 2, 3, buffer, 4, stride,
		sizeof(uint32_t), false));

	EXPECT_TRUE(dsGfxBuffer_destroy(buffer));
	EXPECT_TRUE(dsGfxQueryPool_destroy(queries));
}

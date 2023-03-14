/*
 * Copyright 2023 Aaron Barany
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

#include "AnimationInternal.h"
#include <DeepSea/Animation/Animation.h>
#include <DeepSea/Animation/AnimationTree.h>
#include <DeepSea/Animation/AnimationNodeMapCache.h>
#include <DeepSea/Animation/DirectAnimation.h>
#include <DeepSea/Animation/KeyframeAnimation.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <gtest/gtest.h>

TEST(AnimationNodeMapCacheTest, AddRemove)
{
	dsSystemAllocator sysAlloc;
	ASSERT_TRUE(dsSystemAllocator_initialize(&sysAlloc, DS_ALLOCATOR_NO_LIMIT));
	auto allocator = reinterpret_cast<dsAllocator*>(&sysAlloc);

	dsDirectAnimationChannel dummyDirectChannel =
	{
		"foo", dsAnimationComponent_Translation, {{0, 0, 0, 0}}
	};
	dsDirectAnimation* direct0 = dsDirectAnimation_create(allocator, &dummyDirectChannel, 1);
	ASSERT_NE(nullptr, direct0);
	dsDirectAnimation* direct1 = dsDirectAnimation_create(allocator, &dummyDirectChannel, 1);
	ASSERT_NE(nullptr, direct1);
	dsDirectAnimation* direct2 = dsDirectAnimation_create(allocator, &dummyDirectChannel, 1);
	ASSERT_NE(nullptr, direct2);

	dsVector4f dummyValue = {{0, 0, 0, 0}};
	dsKeyframeAnimationChannel dummyKeyframeChannel =
	{
		"foo", dsAnimationComponent_Translation, dsAnimationInterpolation_Step, 1, &dummyValue
	};
	float dummyTime = 0;
	dsAnimationKeyframes dummyKeyframes = {1, 1, &dummyTime, &dummyKeyframeChannel};
	dsKeyframeAnimation* keyframe0 = dsKeyframeAnimation_create(allocator, &dummyKeyframes, 1);
	ASSERT_NE(nullptr, keyframe0);
	dsKeyframeAnimation* keyframe1 = dsKeyframeAnimation_create(allocator, &dummyKeyframes, 1);
	ASSERT_NE(nullptr, keyframe1);
	dsKeyframeAnimation* keyframe2 = dsKeyframeAnimation_create(allocator, &dummyKeyframes, 1);
	ASSERT_NE(nullptr, keyframe2);

	dsAnimationBuildNode dummyBuildNode =
	{
		"foo", {{1, 1, 1}}, {{0, 0, 0, 1}}, {{0, 0, 0}}, 0, nullptr
	};
	dsAnimationBuildNode* dummyBuildNodes = &dummyBuildNode;
	dsAnimationTree* tree0 = dsAnimationTree_create(allocator, &dummyBuildNodes, 1);
	ASSERT_NE(nullptr, tree0);
	dsAnimationTree* tree1 = dsAnimationTree_create(allocator, &dummyBuildNodes, 1);
	ASSERT_NE(nullptr, tree1);

	dsAnimationNodeMapCache* cache = dsAnimationNodeMapCache_create(allocator);
	ASSERT_NE(nullptr, cache);

	dsAnimation* animation0 = dsAnimation_create(allocator, cache);
	ASSERT_NE(nullptr, animation0);
	dsAnimation* animation1 = dsAnimation_create(allocator, cache);
	ASSERT_NE(nullptr, animation1);

	ASSERT_TRUE(dsAnimationNodeMapCache_addAnimationTree(cache, tree1));
	EXPECT_EQ(1U, cache->treeMapCount);
	EXPECT_TRUE(dsAnimationNodeMapCache_addAnimationTree(cache, tree1));
	EXPECT_EQ(1U, cache->treeMapCount);
	EXPECT_TRUE(dsAnimationNodeMapCache_removeAnimationTree(cache, tree1));
	ASSERT_EQ(1U, cache->treeMapCount);

	ASSERT_TRUE(dsAnimation_addDirectAnimation(animation0, direct2, 1));
	ASSERT_TRUE(dsAnimation_addDirectAnimation(animation1, direct0, 1));
	ASSERT_TRUE(dsAnimation_addKeyframeAnimation(animation0, keyframe2, 1, 0, 1, false));
	ASSERT_TRUE(dsAnimation_addKeyframeAnimation(animation1, keyframe0, 1, 0, 1, false));

	ASSERT_EQ(2U, cache->directAnimationCount);
	ASSERT_EQ(2U, cache->keyframeAnimationCount);
	ASSERT_EQ(1U, cache->treeMapCount);
	for (uint32_t i = 0; i < cache->treeMapCount; ++i)
	{
		for (uint32_t j = 0; j < cache->directAnimationCount; ++j)
		{
			EXPECT_EQ(cache->directAnimations[j].animation,
				cache->treeMaps[i].directMaps[j]->animation);
		}
		for (uint32_t j = 0; j < cache->keyframeAnimationCount; ++j)
		{
			EXPECT_EQ(cache->keyframeAnimations[j].animation,
				cache->treeMaps[i].keyframeMaps[j]->animation);
		}
	}

	ASSERT_TRUE(dsAnimationNodeMapCache_addAnimationTree(cache, tree0));
	ASSERT_EQ(2U, cache->directAnimationCount);
	ASSERT_EQ(2U, cache->keyframeAnimationCount);
	ASSERT_EQ(2U, cache->treeMapCount);
	for (uint32_t i = 0; i < cache->treeMapCount; ++i)
	{
		for (uint32_t j = 0; j < cache->directAnimationCount; ++j)
		{
			EXPECT_EQ(cache->directAnimations[j].animation,
				cache->treeMaps[i].directMaps[j]->animation);
		}
		for (uint32_t j = 0; j < cache->keyframeAnimationCount; ++j)
		{
			EXPECT_EQ(cache->keyframeAnimations[j].animation,
				cache->treeMaps[i].keyframeMaps[j]->animation);
		}
	}

	ASSERT_TRUE(dsAnimation_addDirectAnimation(animation0, direct1, 1));
	ASSERT_TRUE(dsAnimation_addDirectAnimation(animation1, direct1, 1));
	ASSERT_TRUE(dsAnimation_addKeyframeAnimation(animation0, keyframe1, 1, 0, 1, false));
	ASSERT_TRUE(dsAnimation_addKeyframeAnimation(animation1, keyframe1, 1, 0, 1, false));
	ASSERT_EQ(3U, cache->directAnimationCount);
	ASSERT_EQ(3U, cache->keyframeAnimationCount);
	ASSERT_EQ(2U, cache->treeMapCount);
	for (uint32_t i = 0; i < cache->treeMapCount; ++i)
	{
		for (uint32_t j = 0; j < cache->directAnimationCount; ++j)
		{
			EXPECT_EQ(cache->directAnimations[j].animation,
				cache->treeMaps[i].directMaps[j]->animation);
		}
		for (uint32_t j = 0; j < cache->keyframeAnimationCount; ++j)
		{
			EXPECT_EQ(cache->keyframeAnimations[j].animation,
				cache->treeMaps[i].keyframeMaps[j]->animation);
		}
	}

	ASSERT_TRUE(dsAnimation_removeDirectAnimation(animation0, direct2));
	ASSERT_TRUE(dsAnimation_removeDirectAnimation(animation0, direct1));
	ASSERT_TRUE(dsAnimation_removeDirectAnimation(animation1, direct0));
	ASSERT_TRUE(dsAnimation_removeKeyframeAnimation(animation0, keyframe2));
	ASSERT_TRUE(dsAnimation_removeKeyframeAnimation(animation0, keyframe1));
	ASSERT_TRUE(dsAnimation_removeKeyframeAnimation(animation1, keyframe0));

	ASSERT_EQ(1U, cache->directAnimationCount);
	ASSERT_EQ(1U, cache->keyframeAnimationCount);
	ASSERT_EQ(2U, cache->treeMapCount);
	for (uint32_t i = 0; i < cache->treeMapCount; ++i)
	{
		for (uint32_t j = 0; j < cache->directAnimationCount; ++j)
		{
			EXPECT_EQ(cache->directAnimations[j].animation,
				cache->treeMaps[i].directMaps[j]->animation);
		}
		for (uint32_t j = 0; j < cache->keyframeAnimationCount; ++j)
		{
			EXPECT_EQ(cache->keyframeAnimations[j].animation,
				cache->treeMaps[i].keyframeMaps[j]->animation);
		}
	}

	ASSERT_TRUE(dsAnimationNodeMapCache_removeAnimationTree(cache, tree1));
	ASSERT_EQ(1U, cache->directAnimationCount);
	ASSERT_EQ(1U, cache->keyframeAnimationCount);
	ASSERT_EQ(1U, cache->treeMapCount);
	for (uint32_t i = 0; i < cache->treeMapCount; ++i)
	{
		for (uint32_t j = 0; j < cache->directAnimationCount; ++j)
		{
			EXPECT_EQ(cache->directAnimations[j].animation,
				cache->treeMaps[i].directMaps[j]->animation);
		}
		for (uint32_t j = 0; j < cache->keyframeAnimationCount; ++j)
		{
			EXPECT_EQ(cache->keyframeAnimations[j].animation,
				cache->treeMaps[i].keyframeMaps[j]->animation);
		}
	}

	dsAnimation_destroy(animation0);
	dsAnimation_destroy(animation1);
	// Should be safe to destroy trees first.
	dsAnimationTree_destroy(tree0);
	dsAnimationTree_destroy(tree1);
	dsAnimationNodeMapCache_destroy(cache);
	dsDirectAnimation_destroy(direct0);
	dsDirectAnimation_destroy(direct1);
	dsDirectAnimation_destroy(direct2);
	dsKeyframeAnimation_destroy(keyframe0);
	dsKeyframeAnimation_destroy(keyframe1);
	dsKeyframeAnimation_destroy(keyframe2);
	EXPECT_EQ(0U, allocator->currentAllocations);
	EXPECT_EQ(0U, allocator->size);
}

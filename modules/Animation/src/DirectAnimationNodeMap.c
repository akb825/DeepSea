/*
 * Copyright 2022-2023 Aaron Barany
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

#include "KeyframeAnimationNodeMap.h"

#include <DeepSea/Animation/AnimationTree.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

dsKeyframeAnimationNodeMap* dsKeyframeAnimationNodeMap_create(
	dsAllocator* allocator, const dsKeyframeAnimation* animation, const dsAnimationTree* tree)
{
	if (!allocator || !animation || !tree)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsKeyframeAnimationNodeMap)) +
		DS_ALIGNED_SIZE(sizeof(dsAnimationKeyframesNodeMap)*animation->keyframesCount);
	for (uint32_t i = 0; i < animation->keyframesCount; ++i)
	{
		const dsAnimationKeyframes* curKeyframes = animation->keyframes + i;
		fullSize += DS_ALIGNED_SIZE(sizeof(uint32_t)*curKeyframes->channelCount);
	}

	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsKeyframeAnimationNodeMap* map = DS_ALLOCATE_OBJECT(&bufferAlloc, dsKeyframeAnimationNodeMap);
	DS_ASSERT(map);

	map->allocator = dsAllocator_keepPointer(allocator);
	map->animation = animation;
	map->treeID = tree->id;
	map->keyframesCount = animation->keyframesCount;

	dsAnimationKeyframesNodeMap* keyframesMaps = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc,
		dsAnimationKeyframesNodeMap, animation->keyframesCount);
	DS_ASSERT(keyframesMaps);
	for (uint32_t i = 0; i < animation->keyframesCount; ++i)
	{
		const dsAnimationKeyframes* keyframes = animation->keyframes + i;
		dsAnimationKeyframesNodeMap* keyframesMap = keyframesMaps + i;
		keyframesMap->channelCount = keyframes->channelCount;
		uint32_t* channelNodes =
			DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, uint32_t, keyframes->channelCount);
		DS_ASSERT(channelNodes);
		for (uint32_t j = 0; j < keyframes->channelCount; ++j)
		{
			const dsKeyframeAnimationChannel* channel = keyframes->channels + j;
			channelNodes[j] = dsAnimationTree_findNodeIndexName(tree, channel->node);
		}
		keyframesMap->channelNodes = channelNodes;
	}
	map->keyframesMaps = keyframesMaps;

	return map;
}

void dsKeyframeAnimationNodeMap_destroy(dsKeyframeAnimationNodeMap* map)
{
	if (map && map->allocator)
		DS_VERIFY(dsAllocator_free(map->allocator, map));
}

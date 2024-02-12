/*
 * Copyright 2023-2024 Aaron Barany
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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Animation/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct dsAnimationKeyframesNodeMap
{
	uint32_t channelCount;
	const uint32_t* channelNodes;
} dsAnimationKeyframesNodeMap;

typedef struct dsKeyframeAnimationNodeMap
{
	dsAllocator* allocator;
	const dsKeyframeAnimation* animation;
	uint32_t treeID;
	uint32_t keyframesCount;
	const dsAnimationKeyframesNodeMap* keyframesMaps;
} dsKeyframeAnimationNodeMap;

typedef struct dsDirectAnimationNodeMap
{
	dsAllocator* allocator;
	const dsDirectAnimation* animation;
	uint32_t treeID;
	uint32_t channelCount;
	const uint32_t* channelNodes;
} dsDirectAnimationNodeMap;

typedef struct dsKeyframeAnimationRef
{
	const dsKeyframeAnimation* animation;
	uint32_t refCount;
} dsKeyframeAnimationRef;

typedef struct dsDirectAnimationRef
{
	const dsDirectAnimation* animation;
	uint32_t refCount;
} dsDirectAnimationRef;

typedef struct dsAnimationTreeNodeMap
{
	dsAnimationTree* tree;
	uint32_t refCount;

	dsKeyframeAnimationNodeMap** keyframeMaps;
	dsDirectAnimationNodeMap** directMaps;
	uint32_t maxKeyframeMaps;
	uint32_t maxDirectMaps;
} dsAnimationTreeNodeMap;

struct dsAnimationNodeMapCache
{
	dsAllocator* allocator;

	dsKeyframeAnimationRef* keyframeAnimations;
	uint32_t keyframeAnimationCount;
	uint32_t maxKeyframeAnimations;

	dsDirectAnimationRef* directAnimations;
	uint32_t directAnimationCount;
	uint32_t maxDirectAnimations;

	dsAnimationTreeNodeMap* treeMaps;
	uint32_t treeMapCount;
	uint32_t maxTreeMaps;

	dsReadWriteSpinlock lock;
};

#ifdef __cplusplus
}
#endif

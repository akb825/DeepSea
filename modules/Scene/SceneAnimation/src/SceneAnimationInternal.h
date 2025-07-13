/*
 * Copyright 2023-2025 Aaron Barany
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
#include <DeepSea/SceneAnimation/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct dsSceneAnimationRagdollNodeRef
{
	dsSceneTreeNode* node;
	dsSceneTreeNode* relativeNode;
	const char* nodeName;
	uint32_t animationComponents;
} dsSceneAnimationRagdollNodeRef;

typedef struct dsSceneAnimationRagdollInstance
{
	dsDirectAnimation* animation;
	dsSceneAnimationRagdollNodeRef* nodeRefs;
	uint32_t nodeRefCount;
	uint32_t maxNodeRefs;
	const dsSceneTreeNode** removedNodes;
	uint32_t removedNodeCount;
	uint32_t maxRemovedNodes;
	dsDirectAnimationChannel* tempChannels;
	uint32_t maxTempChannels;
	bool dirty;
	bool sorted;
	float weight;
} dsSceneAnimationRagdollInstance;

typedef struct dsSceneAnimationInstance
{
	dsAllocator* allocator;
	dsAnimation* animation;
	dsSceneAnimationRagdollInstance skeletonRagdoll;
	dsSceneAnimationRagdollInstance additionRagdoll;
} dsSceneAnimationInstance;

typedef struct dsSceneAnimationTreeInstance
{
	dsAllocator* allocator;
	const dsAnimation* animation;
	dsAnimationTree* animationTree;
	bool dirty;
	dsSpinlock lock;
} dsSceneAnimationTreeInstance;

#ifdef __cplusplus
}
#endif

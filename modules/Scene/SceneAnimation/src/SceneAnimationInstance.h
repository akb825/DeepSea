/*
 * Copyright 2025 Aaron Barany
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
#include "SceneAnimationInternal.h"

#ifdef __cplusplus
extern "C"
{
#endif

dsSceneAnimationInstance* dsSceneAnimationInstance_create(dsAllocator* allocator,
	dsAnimationNodeMapCache* nodeMapCache);
bool dsSceneAnimationInstance_addSkeletonRagdollNode(dsSceneAnimationInstance* instance,
	const dsSceneAnimationRagdollNode* ragdollNode, dsSceneTreeNode* treeNode);
bool dsSceneAnimationInstance_addAdditionRagdollNode(dsSceneAnimationInstance* instance,
	const dsSceneAnimationRagdollNode* ragdollNode, dsSceneTreeNode* treeNode);
void dsSceneAnimationInstance_removeSkeletonRagdollNode(
	dsSceneAnimationInstance* instance, const dsSceneTreeNode* nodes);
void dsSceneAnimationInstance_removeAdditionRagdollNode(
	dsSceneAnimationInstance* instance, const dsSceneTreeNode* nodes);
bool dsSceneAnimationInstance_setSkeletonRagdollWeight(
	dsSceneAnimationInstance* instance, float weight);
bool dsSceneAnimationInstance_setAdditionRagdollWeight(
	dsSceneAnimationInstance* instance, float weight);
void dsSceneAnimationInstance_updateRagdolls(dsSceneAnimationInstance* instance);
void dsSceneAnimationInstance_destroy(dsSceneAnimationInstance* instance);

#ifdef __cplusplus
}
#endif

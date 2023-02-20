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

#include <DeepSea/SceneAnimation/SceneAnimation.h>

#include <DeepSea/Animation/Animation.h>
#include <DeepSea/Animation/AnimationTree.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/SceneAnimation/SceneAnimationTree.h>

dsSceneAnimation* dsSceneAnimation_create(dsAllocator* allocator,
	dsSceneAnimationTree* sceneAnimationTree)
{
	if (!allocator || !sceneAnimationTree)
	{
		errno = EINVAL;
		return NULL;
	}

	dsAnimationTree* animationTree = dsAnimationTree_clone(allocator,
		dsSceneAnimationTree_getAnimationTree(sceneAnimationTree));
	if (!animationTree)
		return NULL;

	dsAnimation* animation = dsAnimation_create(allocator, animationTree->id);
	if (!animation)
	{
		dsAnimationTree_destroy(animationTree);
		return NULL;
	}

	dsSceneAnimation* sceneAnimation = DS_ALLOCATE_OBJECT(allocator, dsSceneAnimation);
	if (!sceneAnimation)
	{
		dsAnimationTree_destroy(animationTree);
		dsAnimation_destroy(animation);
	}

	sceneAnimation->allocator = dsAllocator_keepPointer(allocator);
	sceneAnimation->animation = animation;
	sceneAnimation->animationTree = animationTree;
	sceneAnimation->sceneAnimationTree = sceneAnimationTree;
	return sceneAnimation;
}

bool dsSceneAnimation_addKeyframeAnimation(dsSceneAnimation* animation,
	const dsKeyframeAnimation* keyframeAnimation, float weight, double time, double timeScale,
	bool wrap)
{
	if (!animation || !keyframeAnimation)
	{
		errno = EINVAL;
		return false;
	}

	const dsKeyframeAnimationNodeMap* map =
		dsSceneAnimationTree_addKeyframeAnimationNodeMap(animation->sceneAnimationTree,
			keyframeAnimation);
	if (!map)
		return false;

	if (!dsAnimation_addKeyframeAnimation(animation->animation, keyframeAnimation, map, weight,
			time, timeScale, wrap))
	{
		DS_VERIFY(dsSceneAnimationTree_removeKeyframeAnimationNodeMap(animation->sceneAnimationTree,
			keyframeAnimation));
		return false;
	}

	return true;
}

dsKeyframeAnimationEntry* dsSceneAnimation_findKeyframeAnimationEntry(
	dsSceneAnimation* animation, const dsKeyframeAnimation* keyframeAnimation)
{
	if (!animation || !keyframeAnimation)
	{
		errno = EINVAL;
		return NULL;
	}

	return dsAnimation_findKeyframeAnimationEntry(animation->animation, keyframeAnimation);
}

bool dsSceneAnimation_removeKeyframeAnimation(dsSceneAnimation* animation,
	const dsKeyframeAnimation* keyframeAnimation)
{
	if (!animation || !keyframeAnimation)
		return false;

	if (!dsAnimation_removeKeyframeAnimation(animation->animation, keyframeAnimation))
		return false;

	DS_VERIFY(dsSceneAnimationTree_removeKeyframeAnimationNodeMap(animation->sceneAnimationTree,
		keyframeAnimation));
	return true;
}

bool dsSceneAnimation_addDirectAnimation(dsSceneAnimation* animation,
	const dsDirectAnimation* directAnimation, float weight)
{
	if (!animation || !directAnimation)
	{
		errno = EINVAL;
		return false;
	}

	const dsDirectAnimationNodeMap* map =
		dsSceneAnimationTree_addDirectAnimationNodeMap(animation->sceneAnimationTree,
			directAnimation);
	if (!map)
		return false;

	if (!dsAnimation_addDirectAnimation(animation->animation, directAnimation, map, weight))
	{
		DS_VERIFY(dsSceneAnimationTree_removeDirectAnimationNodeMap(animation->sceneAnimationTree,
			directAnimation));
		return false;
	}

	return true;
}

dsDirectAnimationEntry* dsSceneAnimation_findDirectAnimationEntry(
	dsSceneAnimation* animation, const dsDirectAnimation* directAnimation)
{
	if (!animation || !directAnimation)
	{
		errno = EINVAL;
		return NULL;
	}

	return dsAnimation_findDirectAnimationEntry(animation->animation, directAnimation);
}

bool dsSceneAnimation_removeDirectAnimation(dsSceneAnimation* animation,
	const dsDirectAnimation* directAnimation)
{
	if (!animation || !directAnimation)
		return false;

	if (!dsAnimation_removeDirectAnimation(animation->animation, directAnimation))
		return false;

	DS_VERIFY(dsSceneAnimationTree_removeDirectAnimationNodeMap(animation->sceneAnimationTree,
		directAnimation));
	return true;
}

void dsSceneAnimation_destroy(dsSceneAnimation* animation)
{
	if (!animation)
		return;

	for (uint32_t i = 0; i < animation->animation->keyframeEntryCount; ++i)
	{
		dsKeyframeAnimationEntry* entry = animation->animation->keyframeEntries + i;
		DS_VERIFY(dsSceneAnimationTree_removeKeyframeAnimationNodeMap(animation->sceneAnimationTree,
			entry->animation));
	}

	for (uint32_t i = 0; i < animation->animation->directEntryCount; ++i)
	{
		dsDirectAnimationEntry* entry = animation->animation->directEntries + i;
		DS_VERIFY(dsSceneAnimationTree_removeDirectAnimationNodeMap(animation->sceneAnimationTree,
			entry->animation));
	}

	dsAnimation_destroy(animation->animation);
	dsAnimationTree_destroy(animation->animationTree);
	if (animation->allocator)
		DS_VERIFY(dsAllocator_free(animation->allocator, animation));
}

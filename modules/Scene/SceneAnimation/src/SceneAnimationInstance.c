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

#include "SceneAnimationInstance.h"

#include <DeepSea/Animation/Animation.h>
#include <DeepSea/Animation/DirectAnimation.h>

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Bits.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Sort.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Scene/Nodes/SceneTreeNode.h>

#include <stdlib.h>
#include <string.h>

static int ptrCompare(const void* left, const void* right)
{
	return DS_CMP(*(const void**)left, *(const void**)right);
}

static int nodeRefCompare(const void* left, const void* right)
{
	const dsSceneAnimationRagdollNodeRef* leftNodeRef =
		(const dsSceneAnimationRagdollNodeRef*)left;
	const dsSceneAnimationRagdollNodeRef* rightNodeRef =
		(const dsSceneAnimationRagdollNodeRef*)right;
	return DS_CMP(leftNodeRef->node, rightNodeRef->node);
}

static int nodeNodeRefCompare(const void* left, const void* right, void* context)
{
	DS_UNUSED(context);
	const dsSceneTreeNode* node = (const dsSceneTreeNode*)left;
	const dsSceneAnimationRagdollNodeRef* nodeRef = (const dsSceneAnimationRagdollNodeRef*)right;
	return DS_CMP(node, nodeRef->node);
}

static bool dsSceneAnimationRagdollInstance_addNode(dsAllocator* allocator,
	dsSceneAnimationRagdollInstance* instance, const dsSceneAnimationRagdollNode* ragdollNode,
	dsSceneTreeNode* treeNode)
{
	dsSceneTreeNode* relativeNode = treeNode;
	for (unsigned int i = 0; i < ragdollNode->relativeAncestor; ++i)
	{
		relativeNode = relativeNode->parent;
		if (!relativeNode)
		{
			DS_LOG_ERROR_F(DS_SCENE_ANIMATION_LOG_TAG,
				"Relative ancestor for ragdoll node '%s' doesn't exist.",
				ragdollNode->animationNodeName);
			return false;
		}
	}

	uint32_t index = instance->nodeRefCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(
			allocator, instance->nodeRefs, instance->nodeRefCount, instance->maxNodeRefs, 1))
	{
		return false;
	}

	dsSceneAnimationRagdollNodeRef* nodeRef = instance->nodeRefs + index;
	nodeRef->node = treeNode;
	nodeRef->relativeNode = relativeNode;
	nodeRef->nodeName = ragdollNode->animationNodeName;
	nodeRef->animationComponents = ragdollNode->animationComponents;

	instance->dirty = true;
	instance->sorted = false;
	return true;
}

static void dsSceneAnimationRagdollInstance_removeNode(
	dsAllocator* allocator, dsSceneAnimationRagdollInstance* instance, const dsSceneTreeNode* node)
{
	uint32_t index = instance->removedNodeCount;
	if (DS_RESIZEABLE_ARRAY_ADD(allocator, instance->removedNodes, instance->removedNodeCount,
			instance->maxRemovedNodes, 1))
	{
		instance->removedNodes[index] = node;
		return;
	}

	// Fall back to linear removal.
	for (uint32_t i = 0; i < instance->nodeRefCount; ++i)
	{
		dsSceneAnimationRagdollNodeRef* nodeRef = instance->nodeRefs + i;
		if (nodeRef->node != node)
			continue;

		// Swap with end.
		*nodeRef = instance->nodeRefs[instance->nodeRefCount - 1];
		--instance->nodeRefCount;
		instance->sorted = false;
		break;
	}
}

static void dsSceneAnimationRagdollInstance_removeNodeRefs(
	dsSceneAnimationRagdollInstance* instance)
{
	if (instance->removedNodeCount == 0)
		return;

	// Both refs and remove nodes must be sorted.
	if (!instance->sorted)
	{
		qsort(instance->nodeRefs, instance->nodeRefCount, sizeof(dsSceneAnimationRagdollNodeRef),
			&nodeRefCompare);
		instance->sorted = true;
	}
	qsort((void*)instance->removedNodes, instance->removedNodeCount, sizeof(void*), &ptrCompare);

	// Search for the first candidate to remove.
	dsSceneAnimationRagdollNodeRef* curEntry =
		(dsSceneAnimationRagdollNodeRef*)dsBinarySearchLowerBound(instance->removedNodes,
			instance->nodeRefs, instance->nodeRefCount, sizeof(dsSceneAnimationRagdollNodeRef),
			&nodeNodeRefCompare, NULL);
	if (!curEntry)
	{
		instance->removedNodeCount = 0;
		return;
	}

	uint32_t curNodeIndex = 0;
	dsSceneAnimationRagdollNodeRef* target = curEntry;
	dsSceneAnimationRagdollNodeRef* entryEnd = instance->nodeRefs + instance->nodeRefCount;
	for (; curEntry < entryEnd; ++curEntry)
	{
		if (curNodeIndex < instance->removedNodeCount)
		{
			bool remove = curEntry->node == instance->removedNodes[curNodeIndex];
			while (curNodeIndex < instance->removedNodeCount &&
				instance->removedNodes[curNodeIndex] <= curEntry->node)
			{
				++curNodeIndex;
			}

			if (remove)
			{
				--instance->nodeRefCount;
				continue;
			}
		}

		if (target != curEntry)
			memcpy(target, curEntry, sizeof(dsSceneAnimationRagdollNodeRef));
		++target;
	}

	if (target != curEntry)
		instance->dirty = true;
	instance->removedNodeCount = 0;
}

static bool dsSceneAnimationRagdollInstance_setWeight(
	dsSceneAnimationRagdollInstance* instance, dsAnimation* animation, float weight)
{
	if (weight < 0.0f || weight > 1.0f)
	{
		errno = EINVAL;
		return false;
	}
	else if (weight == instance->weight)
		return true;

	instance->weight = weight;
	// If dirty we don't need to update within the animation itself.
	if (!instance->animation || instance->dirty)
		return true;

	dsDirectAnimationEntry* entry = dsAnimation_findDirectAnimationEntry(
		animation, instance->animation);
	if (entry)
		entry->weight = weight;
	return true;
}

static bool dsSceneAnimationRagdollInstance_recreateAnimation(
	dsAllocator* allocator, dsSceneAnimationRagdollInstance* instance, dsAnimation* animation)
{
	if (!instance->dirty)
		return false;

	// After this point even errors return true so it won't try to update the channels.
	if (!instance->sorted)
	{
		qsort(instance->nodeRefs, instance->nodeRefCount, sizeof(dsSceneAnimationRagdollNodeRef),
			&nodeRefCompare);
		instance->sorted = true;
	}
	// Consider not dirty even if failed so it won't retry every frame to encounter the same error.
	instance->dirty = false;

	if (instance->animation)
	{
		dsAnimation_removeDirectAnimation(animation, instance->animation);
		dsDirectAnimation_destroy(instance->animation);
		instance->animation = NULL;
	}

	uint32_t channelCount = 0;
	for (uint32_t i = 0; i < instance->nodeRefCount; ++i)
		channelCount += dsCountBits(instance->nodeRefs[i].animationComponents);
	if (channelCount == 0)
		return true;

	uint32_t dummy = 0;
	if (!DS_CHECK(DS_SCENE_ANIMATION_LOG_TAG, DS_RESIZEABLE_ARRAY_ADD(allocator,
			instance->tempChannels, dummy, instance->maxTempChannels, channelCount)))
	{
		return true;
	}

	dsDirectAnimationChannel* curChannel = instance->tempChannels;
	for (uint32_t i = 0; i < instance->nodeRefCount; ++i)
	{
		const dsSceneAnimationRagdollNodeRef* nodeRef = instance->nodeRefs + i;
		dsMatrix44f relativeTransform;
		dsSceneTreeNode_getCurrentRelativeTransform(
			&relativeTransform, nodeRef->node, nodeRef->relativeNode);
		dsVector3f position, scale;
		dsQuaternion4f orientation;
		dsMatrix44f_decomposeTransform(&position, &orientation, &scale, &relativeTransform);

		if (nodeRef->animationComponents & (1 << dsAnimationComponent_Translation))
		{
			curChannel->node = nodeRef->nodeName;
			curChannel->component = dsAnimationComponent_Translation;
			curChannel->value.x = position.x;
			curChannel->value.y = position.y;
			curChannel->value.z = position.z;
			curChannel->value.w = 0;
			++curChannel;
		}

		if (nodeRef->animationComponents & (1 << dsAnimationComponent_Rotation))
		{
			curChannel->node = nodeRef->nodeName;
			curChannel->component = dsAnimationComponent_Rotation;
			curChannel->value = *(dsVector4f*)&orientation;
			++curChannel;
		}

		if (nodeRef->animationComponents & (1 << dsAnimationComponent_Scale))
		{
			curChannel->node = nodeRef->nodeName;
			curChannel->component = dsAnimationComponent_Translation;
			curChannel->value.x = scale.x;
			curChannel->value.y = scale.y;
			curChannel->value.z = scale.z;
			curChannel->value.w = 0;
			++curChannel;
		}
	}
	DS_ASSERT(curChannel == instance->tempChannels + channelCount);

	instance->animation = dsDirectAnimation_create(allocator, instance->tempChannels, channelCount);
	if (!instance->animation)
		return true;

	return true;
}

static void dsSceneAnimationRagdollInstance_update(dsAllocator* allocator,
	dsSceneAnimationRagdollInstance* instance, dsAnimation* animation)
{
	dsSceneAnimationRagdollInstance_removeNodeRefs(instance);
	bool updated = dsSceneAnimationRagdollInstance_recreateAnimation(
		allocator, instance, animation);
	if (updated || instance->weight == 0.0f || !instance->animation)
		return;

	dsDirectAnimationChannel* curChannel = instance->animation->channels;
	for (uint32_t i = 0; i < instance->nodeRefCount; ++i)
	{
		const dsSceneAnimationRagdollNodeRef* nodeRef = instance->nodeRefs + i;
		dsMatrix44f relativeTransform;
		dsSceneTreeNode_getCurrentRelativeTransform(
			&relativeTransform, nodeRef->node, nodeRef->relativeNode);
		dsVector3f position, scale;
		dsQuaternion4f orientation;
		dsMatrix44f_decomposeTransform(&position, &orientation, &scale, &relativeTransform);

		if (nodeRef->animationComponents & (1 << dsAnimationComponent_Translation))
		{
			DS_ASSERT(curChannel->node == nodeRef->nodeName);
			DS_ASSERT(curChannel->component == dsAnimationComponent_Translation);
			curChannel->value.x = position.x;
			curChannel->value.y = position.y;
			curChannel->value.z = position.z;
			curChannel->value.w = 0;
			++curChannel;
		}

		if (nodeRef->animationComponents & (1 << dsAnimationComponent_Rotation))
		{
			DS_ASSERT(curChannel->node == nodeRef->nodeName);
			DS_ASSERT(curChannel->component == dsAnimationComponent_Rotation);
			curChannel->value = *(dsVector4f*)&orientation;
			++curChannel;
		}

		if (nodeRef->animationComponents & (1 << dsAnimationComponent_Scale))
		{
			DS_ASSERT(curChannel->node == nodeRef->nodeName);
			DS_ASSERT(curChannel->component == dsAnimationComponent_Translation);
			curChannel->value.x = scale.x;
			curChannel->value.y = scale.y;
			curChannel->value.z = scale.z;
			curChannel->value.w = 0;
			++curChannel;
		}
	}
	DS_ASSERT(curChannel == instance->animation->channels + instance->animation->channelCount);
}

static void dsSceneAnimationRagdollInstance_shutdown(dsAllocator* allocator,
	dsSceneAnimationRagdollInstance* instance)
{
	dsDirectAnimation_destroy(instance->animation);
	DS_VERIFY(dsAllocator_free(allocator, instance->nodeRefs));
	DS_VERIFY(dsAllocator_free(allocator, (void*)instance->removedNodes));
	DS_VERIFY(dsAllocator_free(allocator, instance->tempChannels));
}

dsSceneAnimationInstance* dsSceneAnimationInstance_create(dsAllocator* allocator,
	dsAnimationNodeMapCache* nodeMapCache)
{
	dsSceneAnimationInstance* instance = DS_ALLOCATE_OBJECT(allocator, dsSceneAnimationInstance);
	if (!instance)
		return NULL;

	instance->allocator = dsAllocator_keepPointer(allocator);
	instance->animation = dsAnimation_create(allocator, nodeMapCache);
	if (!DS_CHECK_MESSAGE(DS_SCENE_ANIMATION_LOG_TAG, instance->animation != NULL,
			"dsSceneAnimation_create(allocator, nodeMapCache)"))
	{
		if (instance->allocator)
			DS_VERIFY(dsAllocator_free(instance->allocator, instance));
		return NULL;
	}

	memset(&instance->skeletonRagdoll, 0, sizeof(dsSceneAnimationRagdollInstance));
	memset(&instance->additionRagdoll, 0, sizeof(dsSceneAnimationRagdollInstance));
	// Addition ragdolls default to full weight.
	instance->additionRagdoll.weight = 1.0f;
	return instance;
}

bool dsSceneAnimationInstance_addSkeletonRagdollNode(dsSceneAnimationInstance* instance,
	const dsSceneAnimationRagdollNode* ragdollNode, dsSceneTreeNode* treeNode)
{
	return dsSceneAnimationRagdollInstance_addNode(
		instance->allocator, &instance->skeletonRagdoll, ragdollNode, treeNode);;
}

bool dsSceneAnimationInstance_addAdditionRagdollNode(dsSceneAnimationInstance* instance,
	const dsSceneAnimationRagdollNode* ragdollNode, dsSceneTreeNode* treeNode)
{
	return dsSceneAnimationRagdollInstance_addNode(
		instance->allocator, &instance->additionRagdoll, ragdollNode, treeNode);;
}

void dsSceneAnimationInstance_removeSkeletonRagdollNode(
	dsSceneAnimationInstance* instance, const dsSceneTreeNode* node)
{
	dsSceneAnimationRagdollInstance_removeNode(
		instance->allocator, &instance->skeletonRagdoll, node);
}

void dsSceneAnimationInstance_removeAdditionRagdollNode(
	dsSceneAnimationInstance* instance, const dsSceneTreeNode* node)
{
	dsSceneAnimationRagdollInstance_removeNode(
		instance->allocator, &instance->additionRagdoll, node);
}

bool dsSceneAnimationInstance_setSkeletonRagdollWeight(
	dsSceneAnimationInstance* instance, float weight)
{
	return dsSceneAnimationRagdollInstance_setWeight(
		&instance->skeletonRagdoll, instance->animation, weight);
}

bool dsSceneAnimationInstance_setAdditionRagdollWeight(
	dsSceneAnimationInstance* instance, float weight)
{
	return dsSceneAnimationRagdollInstance_setWeight(
		&instance->additionRagdoll, instance->animation, weight);
}

void dsSceneAnimationInstance_updateRagdolls(dsSceneAnimationInstance* instance)
{
	dsSceneAnimationRagdollInstance_update(
		instance->allocator, &instance->skeletonRagdoll, instance->animation);
	dsSceneAnimationRagdollInstance_update(
		instance->allocator, &instance->additionRagdoll, instance->animation);
}

void dsSceneAnimationInstance_destroy(dsSceneAnimationInstance* instance)
{
	if (!instance)
		return;

	dsAnimation_destroy(instance->animation);
	dsSceneAnimationRagdollInstance_shutdown(instance->allocator, &instance->skeletonRagdoll);
	dsSceneAnimationRagdollInstance_shutdown(instance->allocator, &instance->additionRagdoll);
	if (instance->allocator)
		DS_VERIFY(dsAllocator_free(instance->allocator, instance));
}

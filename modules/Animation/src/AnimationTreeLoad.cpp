/*
 * Copyright 2023-2026 Aaron Barany
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

#include "AnimationTreeLoad.h"

#include <DeepSea/Animation/AnimationTree.h>

#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Quaternion.h>

#include <cstring>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/AnimationTree_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

#define DS_MAX_STACK_NODES 1024

static uint32_t countNodes(
	const flatbuffers::Vector<flatbuffers::Offset<DeepSeaAnimation::AnimationTreeNode>>& fbNodes)
{
	uint32_t count = 0;
	for (auto node : fbNodes)
	{
		if (!node)
			continue;

		++count;
		if (node->children())
			count += countNodes(*node->children());
	}

	return count;
}

static dsAnimationBuildNode* createBuildNodesRec(const DeepSeaAnimation::AnimationTreeNode& fbNode,
	dsAnimationBuildNode* buildNodes, uint32_t& buildNodeIndex,
	const dsAnimationBuildNode** buildNodePtrs, uint32_t& buildNodePtrsIndex)
{
	dsAnimationBuildNode& buildNode = buildNodes[buildNodeIndex++];

	auto fbChildren = fbNode.children();
	buildNode.childCount = 0;
	if (fbChildren)
	{
		for (auto child : *fbChildren)
		{
			if (child)
				++buildNode.childCount;
		}
	}

	buildNode.name = fbNode.name()->c_str();

	auto fbScale = fbNode.scale();
	if (fbScale)
	{
		buildNode.transform.scale.x = fbScale->x();
		buildNode.transform.scale.y = fbScale->y();
		buildNode.transform.scale.z = fbScale->z();
		buildNode.transform.scale.w = 1.0f;
	}
	else
	{
		buildNode.transform.scale.x = buildNode.transform.scale.y = buildNode.transform.scale.z =
			buildNode.transform.scale.w = 1.0f;
	}

	auto fbRotation = fbNode.rotation();
	if (fbRotation)
	{
		buildNode.transform.orientation.r = fbRotation->r();
		buildNode.transform.orientation.i = fbRotation->i();
		buildNode.transform.orientation.j = fbRotation->j();
		buildNode.transform.orientation.k = fbRotation->k();
	}
	else
	{
		dsQuaternion4_identityRotation(buildNode.transform.orientation);
	}

	auto fbTranslation = fbNode.translation();
	if (fbTranslation)
	{
		buildNode.transform.position.x = fbTranslation->x();
		buildNode.transform.position.y = fbTranslation->y();
		buildNode.transform.position.z = fbTranslation->z();
		buildNode.transform.position.w = 0.0f;
	}
	else
	{
		buildNode.transform.position.x = buildNode.transform.position.y =
			buildNode.transform.position.z = buildNode.transform.position.w = 0.0f;
	}

	if (buildNode.childCount > 0)
	{
		const dsAnimationBuildNode** children = buildNodePtrs + buildNodePtrsIndex;
		buildNode.children = children;
		buildNodePtrsIndex += buildNode.childCount;
		uint32_t childIndex = 0;
		for (auto child : *fbChildren)
		{
			if (child)
			{
				children[childIndex++] = createBuildNodesRec(*child, buildNodes, buildNodeIndex,
					buildNodePtrs, buildNodePtrsIndex);
			}
		}
	}
	else
		buildNode.children = nullptr;

	return &buildNode;
}

static dsAnimationTree* dsAnimationTree_loadNodes(dsAllocator* allocator,
	dsAllocator* scratchAllocator,
	const flatbuffers::Vector<flatbuffers::Offset<DeepSeaAnimation::AnimationTreeNode>>& fbRootNodes,
	const char* name)
{
	uint32_t nodeCount = countNodes(fbRootNodes);
	if (nodeCount == 0)
	{
		errno = EFORMAT;
		if (name)
			DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG, "Animation tree has zero nodes for '%s'.", name);
		else
			DS_LOG_ERROR(DS_ANIMATION_LOG_TAG, "Animation tree has zero nodes.");
		return nullptr;
	}

	bool heapNodes;
	dsAnimationBuildNode* buildNodes;
	const dsAnimationBuildNode** buildNodePtrs;
	if (nodeCount <= DS_MAX_STACK_NODES)
	{
		heapNodes = false;
		buildNodes = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsAnimationBuildNode, nodeCount);
		buildNodePtrs = DS_ALLOCATE_STACK_OBJECT_ARRAY(const dsAnimationBuildNode*, nodeCount);
	}
	else
	{
		heapNodes = scratchAllocator->freeFunc != nullptr;
		buildNodes = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, dsAnimationBuildNode, nodeCount);
		if (buildNodes)
			return nullptr;

		buildNodePtrs = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, const dsAnimationBuildNode*,
			nodeCount);
		if (!buildNodePtrs)
		{
			if (heapNodes)
				DS_VERIFY(dsAllocator_free(scratchAllocator, buildNodePtrs));
			return nullptr;
		}
	}


	uint32_t rootNodeCount = 0;
	for (auto fbRootNode : fbRootNodes)
	{
		if (fbRootNode)
			++rootNodeCount;
	}

	uint32_t buildNodeIndex = 0;
	uint32_t buildNodePtrsIndex = rootNodeCount;
	const dsAnimationBuildNode** rootNodes = buildNodePtrs;
	uint32_t rootNodeIndex = 0;
	for (auto fbRootNode : fbRootNodes)
	{
		if (fbRootNode)
		{
			rootNodes[rootNodeIndex++] = createBuildNodesRec(*fbRootNode, buildNodes,
				buildNodeIndex, buildNodePtrs, buildNodePtrsIndex);
		}
	}

	dsAnimationTree* tree = dsAnimationTree_create(allocator, rootNodes, rootNodeCount);
	if (heapNodes)
	{
		DS_VERIFY(dsAllocator_free(scratchAllocator, buildNodes));
		DS_VERIFY(dsAllocator_free(scratchAllocator, buildNodePtrs));
	}
	return tree;
}

static dsAnimationTree* dsAnimationTree_loadJointNodes(dsAllocator* allocator,
	dsAllocator* scratchAllocator,
	const flatbuffers::Vector<flatbuffers::Offset<DeepSeaAnimation::AnimationJointTreeNode>>&
		fbNodes, const char* name)
{
	uint32_t nodeCount = fbNodes.size();
	if (nodeCount == 0)
	{
		errno = EFORMAT;
		if (name)
			DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG, "Animation tree has zero nodes for '%s'.", name);
		else
			DS_LOG_ERROR(DS_ANIMATION_LOG_TAG, "Animation tree has zero nodes.");
		return nullptr;
	}

	dsAnimationJointBuildNode* buildNodes;
	bool heapNodes;
	if (nodeCount <= DS_MAX_STACK_NODES)
	{
		buildNodes = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsAnimationJointBuildNode, nodeCount);
		heapNodes = false;
	}
	else
	{
		heapNodes = scratchAllocator->freeFunc != nullptr;
		buildNodes = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, dsAnimationJointBuildNode,
			nodeCount);
		if (buildNodes)
			return nullptr;
	}

	for (uint32_t i = 0; i < nodeCount; ++i)
	{
		const auto& fbNode = fbNodes[i];
		if (!fbNode)
		{
			errno = EFORMAT;
			if (name)
			{
				DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG, "Animation tree joint node unset for '%s'.",
					name);
			}
			else
				DS_LOG_ERROR(DS_ANIMATION_LOG_TAG, "Animation tree joint node unset.");
			if (heapNodes)
				DS_VERIFY(dsAllocator_free(scratchAllocator, buildNodes));
			return nullptr;
		}

		dsAnimationJointBuildNode& buildNode = buildNodes[i];
		buildNode.name = fbNode->name()->c_str();

		auto fbScale = fbNode->scale();
		if (fbScale)
		{
			buildNode.transform.scale.x = fbScale->x();
			buildNode.transform.scale.y = fbScale->y();
			buildNode.transform.scale.z = fbScale->z();
			buildNode.transform.scale.w = 1.0f;
		}
		else
		{
			buildNode.transform.scale.x = buildNode.transform.scale.y = buildNode.transform.scale.z =
				buildNode.transform.scale.w = 1.0f;
		}

		auto fbRotation = fbNode->rotation();
		if (fbRotation)
		{
			buildNode.transform.orientation.r = fbRotation->r();
			buildNode.transform.orientation.i = fbRotation->i();
			buildNode.transform.orientation.j = fbRotation->j();
			buildNode.transform.orientation.k = fbRotation->k();
		}
		else
		{
			dsQuaternion4_identityRotation(buildNode.transform.orientation);
		}

		auto fbTranslation = fbNode->translation();
		if (fbTranslation)
		{
			buildNode.transform.position.x = fbTranslation->x();
			buildNode.transform.position.y = fbTranslation->y();
			buildNode.transform.position.z = fbTranslation->z();
			buildNode.transform.position.w = 0.0f;
		}
		else
		{
			buildNode.transform.position.x = buildNode.transform.position.y =
				buildNode.transform.position.z = buildNode.transform.position.w = 0.0f;
		}

		auto fbLocalSpace = fbNode->toLocalSpace();
		std::memcpy(&buildNode.toNodeLocalSpace, fbLocalSpace, sizeof(dsMatrix44f));

		auto fbChildren = fbNode->children();
		if (fbChildren)
		{
			buildNode.childCount = fbChildren->size();
			buildNode.children = fbChildren->data();
		}
		else
		{
			buildNode.childCount = 0;
			buildNode.children = nullptr;
		}
	}

	dsAnimationTree* tree = dsAnimationTree_createJoints(allocator, buildNodes, nodeCount);
	if (heapNodes)
		DS_VERIFY(dsAllocator_free(scratchAllocator, buildNodes));
	return tree;
}

dsAnimationTree* dsAnimationTree_loadImpl(dsAllocator* allocator, dsAllocator* scratchAllocator,
	const void* data, size_t size, const char* name)
{
	DS_ASSERT(scratchAllocator);
	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), size);
	if (!DeepSeaAnimation::VerifyAnimationTreeBuffer(verifier))
	{
		errno = EFORMAT;
		if (name)
		{
			DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG,
				"Invalid animation tree flatbuffer format for '%s'.", name);
		}
		else
			DS_LOG_ERROR(DS_ANIMATION_LOG_TAG, "Invalid animation tree flatbuffer format.");
		return nullptr;
	}

	auto fbAnimationTree = DeepSeaAnimation::GetAnimationTree(data);
	auto fbRootNodes = fbAnimationTree->rootNodes();
	auto fbJointNodes = fbAnimationTree->jointNodes();
	if (!fbRootNodes && !fbJointNodes)
	{
		errno = EFORMAT;
		if (name)
		{
			DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG,
				"Animation tree must have either rootNodes or jointNodes provided for '%s'.", name);
		}
		else
		{
			DS_LOG_ERROR(DS_ANIMATION_LOG_TAG,
				"Animation tree must have either rootNodes or jointNodes provided.");
		}
		return nullptr;
	}
	else if (fbRootNodes && fbJointNodes)
	{
		errno = EFORMAT;
		if (name)
		{
			DS_LOG_ERROR_F(DS_ANIMATION_LOG_TAG,
				"Animation tree must have only one of rootNodes or jointNodes provided for '%s'.",
				name);
		}
		else
		{
			DS_LOG_ERROR(DS_ANIMATION_LOG_TAG,
				"Animation tree must have only one of rootNodes or jointNodes provided.");
		}
		return nullptr;
	}

	if (fbRootNodes)
		return dsAnimationTree_loadNodes(allocator, scratchAllocator, *fbRootNodes, name);
	DS_ASSERT(fbJointNodes);
	return dsAnimationTree_loadJointNodes(allocator, scratchAllocator, *fbJointNodes, name);
}

#include "AnimationTreeLoad.h"

#include <DeepSea/Animation/AnimationTree.h>

#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Types.h>

#include <cstring>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include "Flatbuffers/AnimationTree_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#endif

#define DS_MAX_STACK_NODES 1024

static uint32_t countNodes(
	const flatbuffers::Vector<flatbuffers::Offset<DeepSeaAnimation::AnimationNode>>& fbNodes)
{
	uint32_t count = 0;
	for (const auto& node : fbNodes)
	{
		if (!node)
			continue;

		++count;
		if (node->children())
			count += countNodes(*node->children());
	}

	return count;
}

static dsAnimationBuildNode* createBuildNodesRec(const DeepSeaAnimation::AnimationNode& fbNode,
	dsAnimationBuildNode* buildNodes, uint32_t& buildNodeIndex,
	const dsAnimationBuildNode** buildNodePtrs, uint32_t& buildNodePtrsIndex)
{
	dsAnimationBuildNode& buildNode = buildNodes[buildNodeIndex++];

	auto fbChildren = fbNode.children();
	buildNode.childCount = 0;
	if (fbChildren)
	{
		for (const auto& child : *fbChildren)
		{
			if (child)
				++buildNode.childCount;
		}
	}

	buildNode.name = fbNode.name()->c_str();

	auto fbScale = fbNode.scale();
	if (fbScale)
	{
		buildNode.scale.x = fbScale->x();
		buildNode.scale.y = fbScale->y();
		buildNode.scale.z = fbScale->z();
	}
	else
	{
		buildNode.scale.x = 0.0f;
		buildNode.scale.y = 0.0f;
		buildNode.scale.z = 0.0f;
	}

	auto fbRotation = fbNode.rotation();
	if (fbRotation)
	{
		buildNode.rotation.r = fbRotation->r();
		buildNode.rotation.i = fbRotation->i();
		buildNode.rotation.j = fbRotation->j();
		buildNode.rotation.k = fbRotation->k();
	}
	else
	{
		buildNode.rotation.r = 1.0f;
		buildNode.rotation.i = 0.0f;
		buildNode.rotation.j = 0.0f;
		buildNode.rotation.k = 0.0f;
	}

	auto fbTranslation = fbNode.translation();
	if (fbTranslation)
	{
		buildNode.translation.x = fbTranslation->x();
		buildNode.translation.y = fbTranslation->y();
		buildNode.translation.z = fbTranslation->z();
	}
	else
	{
		buildNode.translation.x = 0.0f;
		buildNode.translation.y = 0.0f;
		buildNode.translation.z = 0.0f;
	}

	if (buildNode.childCount > 0)
	{
		const dsAnimationBuildNode** children = buildNodePtrs + buildNodePtrsIndex;
		buildNode.children = children;
		buildNodePtrsIndex += buildNode.childCount;
		uint32_t childIndex = 0;
		for (const auto& child : *fbChildren)
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
	const flatbuffers::Vector<flatbuffers::Offset<DeepSeaAnimation::AnimationNode>>& fbRootNodes,
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

	dsAnimationBuildNode* buildNodes;
	const dsAnimationBuildNode** buildNodePtrs;
	bool heapNodes;
	if (nodeCount <= DS_MAX_STACK_NODES)
	{
		buildNodes = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsAnimationBuildNode, nodeCount);
		buildNodePtrs = DS_ALLOCATE_STACK_OBJECT_ARRAY(const dsAnimationBuildNode*, nodeCount);
		heapNodes = false;
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
	for (const auto& fbRootNode : fbRootNodes)
	{
		if (fbRootNode)
			++rootNodeCount;
	}

	uint32_t buildNodeIndex = 0;
	uint32_t buildNodePtrsIndex = rootNodeCount;
	const dsAnimationBuildNode** rootNodes = buildNodePtrs;
	uint32_t rootNodeIndex = 0;
	for (const auto& fbRootNode : fbRootNodes)
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
	const flatbuffers::Vector<flatbuffers::Offset<DeepSeaAnimation::AnimationJointNode>>&
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
			buildNode.scale.x = fbScale->x();
			buildNode.scale.y = fbScale->y();
			buildNode.scale.z = fbScale->z();
		}
		else
		{
			buildNode.scale.x = 0.0f;
			buildNode.scale.y = 0.0f;
			buildNode.scale.z = 0.0f;
		}

		auto fbRotation = fbNode->rotation();
		if (fbRotation)
		{
			buildNode.rotation.r = fbRotation->r();
			buildNode.rotation.i = fbRotation->i();
			buildNode.rotation.j = fbRotation->j();
			buildNode.rotation.k = fbRotation->k();
		}
		else
		{
			buildNode.rotation.r = 1.0f;
			buildNode.rotation.i = 0.0f;
			buildNode.rotation.j = 0.0f;
			buildNode.rotation.k = 0.0f;
		}

		auto fbTranslation = fbNode->translation();
		if (fbTranslation)
		{
			buildNode.translation.x = fbTranslation->x();
			buildNode.translation.y = fbTranslation->y();
			buildNode.translation.z = fbTranslation->z();
		}
		else
		{
			buildNode.translation.x = 0.0f;
			buildNode.translation.y = 0.0f;
			buildNode.translation.z = 0.0f;
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

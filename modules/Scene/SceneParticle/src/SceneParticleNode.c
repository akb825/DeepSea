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

#include <DeepSea/SceneParticle/SceneParticleNode.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Geometry/OrientedBox3.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Particle/ParticleEmitter.h>

#include <DeepSea/Scene/Nodes/SceneCullNode.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <DeepSea/SceneParticle/SceneParticlePrepare.h>

#include <string.h>

struct dsSceneParticleNode
{
	dsSceneCullNode node;
	dsAllocator* emitterAllocator;
	dsCreateSceneParticleNodeEmitterFunction createEmitterFunc;
	dsUpdateSceneParticleNodeEmitterFunction updateEmitterFunc;
	void* createEmitterUserData;
	dsDestroySceneUserDataFunction destroyCreateEmitterUserDataFunc;
};

static bool dsSceneParticleNode_getBounds(dsMatrix44f* outBoxMatrix, const dsSceneCullNode* node,
	const dsSceneTreeNode* treeNode)
{
	DS_UNUSED(node);
	dsParticleEmitter* emitter = dsSceneParticleNode_getEmitterForInstance(treeNode);
	if (!emitter)
		return false;

	dsOrientedBox3f_toMatrix(outBoxMatrix, &emitter->bounds);
	return true;
}

static void dsSceneParticleNode_destroy(dsSceneNode* node)
{
	dsSceneParticleNode* particleNode = (dsSceneParticleNode*)node;
	if (particleNode->destroyCreateEmitterUserDataFunc)
		particleNode->destroyCreateEmitterUserDataFunc(particleNode->createEmitterUserData);
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

const char* const dsSceneParticleNode_typeName = "ParticleNode";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsSceneParticleNode_type(void)
{
	return &nodeType;
}

const dsSceneNodeType* dsSceneParticleNode_setupParentType(dsSceneNodeType* type)
{
	dsSceneNode_setupParentType(&nodeType, dsSceneCullNode_type());
	return dsSceneNode_setupParentType(type, &nodeType);
}

dsSceneParticleNode* dsSceneParticleNode_create(dsAllocator* allocator,
	dsAllocator* emitterAllocator, dsCreateSceneParticleNodeEmitterFunction createEmitterFunc,
	dsUpdateSceneParticleNodeEmitterFunction updateEmitterFunc, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc, const char* const* itemLists,
	uint32_t itemListCount)
{
	if (!allocator || !createEmitterFunc || (!itemLists && itemListCount > 0))
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return NULL;
	}

	size_t itemListsSize = dsSceneNode_itemListsAllocSize(itemLists, itemListCount);
	if (itemListsSize == 0)
		return NULL;

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneParticleNode)) + itemListsSize;
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneParticleNode* particleNode = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneParticleNode);
	DS_ASSERT(particleNode);

	const char* const* itemListsCopy = dsSceneNode_copyItemLists((dsAllocator*)&bufferAlloc,
		itemLists, itemListCount);
	DS_ASSERT(itemListCount == 0 || itemListsCopy);

	dsSceneNode* node = (dsSceneNode*)particleNode;
	if (!dsSceneNode_initialize(node, allocator, dsSceneParticleNode_setupParentType(NULL),
			itemListsCopy, itemListCount, &dsSceneParticleNode_destroy))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, particleNode));
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return NULL;
	}

	particleNode->emitterAllocator = emitterAllocator ? emitterAllocator : allocator;
	particleNode->createEmitterFunc = createEmitterFunc;
	particleNode->updateEmitterFunc = updateEmitterFunc;
	particleNode->createEmitterUserData = userData;
	particleNode->destroyCreateEmitterUserDataFunc = destroyUserDataFunc;

	dsSceneCullNode* cullNode = (dsSceneCullNode*)particleNode;
	cullNode->hasBounds = true;
	dsMatrix44_identity(cullNode->staticLocalBoxMatrix);
	cullNode->getBoundsFunc = &dsSceneParticleNode_getBounds;

	return particleNode;
}

dsParticleEmitter* dsSceneParticleNode_createEmitter(const dsSceneParticleNode* node,
	const dsSceneTreeNode* treeNode)
{
	if (!node || !treeNode)
	{
		errno = EINVAL;
		return NULL;
	}

	if (treeNode->node != (const dsSceneNode*)node)
	{
		errno = EPERM;
		return NULL;
	}

	return node->createEmitterFunc(node, node->emitterAllocator, node->createEmitterUserData,
		treeNode);
}

dsParticleEmitter* dsSceneParticleNode_getEmitterForInstance(const dsSceneTreeNode* treeNode)
{
	if (!treeNode)
		return NULL;

	const dsSceneNodeItemData* itemData = &treeNode->itemData;
	DS_ASSERT(itemData->count == treeNode->node->itemListCount);
	for (uint32_t i = 0; i < itemData->count; ++i)
	{
		const dsSceneItemList* itemList = treeNode->itemLists[i].list;
		if (itemList && itemList->type == dsSceneParticlePrepare_type())
			return (dsParticleEmitter*)itemData->itemData[i].data;
	}

	return NULL;
}

bool dsSceneParticleNode_updateEmitter(const dsSceneParticleNode* node,
	dsParticleEmitter* emitter, const dsSceneTreeNode* treeNode, float time)
{
	if (!node || !emitter || !treeNode)
	{
		errno = EINVAL;
		return false;
	}

	if (treeNode->node != (const dsSceneNode*)node)
	{
		errno = EPERM;
		return false;
	}

	if (node->updateEmitterFunc)
		return node->updateEmitterFunc(node, node->createEmitterUserData, emitter, treeNode, time);
	else
		return dsParticleEmitter_update(emitter, time);
}

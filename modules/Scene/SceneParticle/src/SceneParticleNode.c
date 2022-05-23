/*
 * Copyright 2022 Aaron Barany
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
#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <string.h>

struct dsSceneParticleNode
{
	dsSceneNode node;
	dsAllocator* emitterAllocator;
	dsCreateSceneParticleNodeEmitterFunction createEmitterFunc;
	void* createEmitterUserData;
	dsDestroySceneUserDataFunction destroyCreateEmitterUserDataFunc;
};

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

dsSceneParticleNode* dsSceneParticleNode_create(dsAllocator* allocator,
	dsAllocator* emitterAllocator, dsCreateSceneParticleNodeEmitterFunction createEmitterFunc,
	void* userData, dsDestroySceneUserDataFunction destroyUserDataFunc,
	const char* const* itemLists, uint32_t itemListCount)
{
	if (!allocator || !createEmitterFunc || (!itemLists && itemListCount > 0))
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneParticleNode)) +
		DS_ALIGNED_SIZE(sizeof(const char*)*itemListCount);
	for (uint32_t i = 0; i < itemListCount; ++i)
	{
		if (!itemLists[i])
		{
			if (destroyUserDataFunc)
				destroyUserDataFunc(userData);
			errno = EINVAL;
			return NULL;
		}

		fullSize += DS_ALIGNED_SIZE(strlen(itemLists[i]) + 1);
	}

	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneParticleNode* particleNode = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneParticleNode);
	DS_ASSERT(particleNode);

	const char** itemListsCopy = NULL;
	if (itemListCount > 0)
	{
		itemListsCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, const char*, itemListCount);
		DS_ASSERT(itemListsCopy);
		for (uint32_t i = 0; i < itemListCount; ++i)
		{
			size_t nameLen = strlen(itemLists[i]) + 1;
			char* nameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
			memcpy(nameCopy, itemLists[i], nameLen);
			itemListsCopy[i] = nameCopy;
		}
	}

	dsSceneNode* node = (dsSceneNode*)particleNode;
	if (!dsSceneNode_initialize(node, allocator, dsSceneParticleNode_type(), itemListsCopy,
			itemListCount, &dsSceneParticleNode_destroy))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, particleNode));
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return NULL;
	}

	particleNode->emitterAllocator = emitterAllocator ? emitterAllocator : allocator;
	particleNode->createEmitterFunc = createEmitterFunc;
	particleNode->createEmitterUserData = userData;
	particleNode->destroyCreateEmitterUserDataFunc = destroyUserDataFunc;

	return particleNode;
}

dsParticleEmitter* dsSceneParticleNode_createEmitter(const dsSceneParticleNode* node)
{
	if (!node)
	{
		errno = EINVAL;
		return NULL;
	}

	return node->createEmitterFunc(node, node->emitterAllocator, node->createEmitterUserData);
}

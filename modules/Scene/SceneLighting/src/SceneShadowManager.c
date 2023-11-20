/*
 * Copyright 2021-2023 Aaron Barany
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

#include <DeepSea/SceneLighting/SceneShadowManager.h>

#include "SceneLightShadowsInternal.h"

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/PoolAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/SceneLighting/SceneLightShadows.h>

typedef struct NamedShadowsNode
{
	dsHashTableNode node;
	dsSceneLightShadows* lightShadows;
} NamedShadowsNode;

typedef struct LightShadowsNode
{
	dsHashTableNode node;
	uint32_t key;
	dsSceneLightShadows* lightShadows;
} LightShadowsNode;

struct dsSceneShadowManager
{
	dsAllocator* allocator;
	dsPoolAllocator namedShadowsPool;
	dsPoolAllocator lightShadowsPool;
	dsHashTable* namedShadowsTable;
	dsHashTable* lightShadowsTable;
};

static void destroyLightShadows(dsSceneLightShadows* const* lightShadows, uint32_t lightShadowsCount)
{
	if (!lightShadows)
		return;

	for (uint32_t i = 0; i < lightShadowsCount; ++i)
		dsSceneLightShadows_destroy(lightShadows[i]);
}

const char* const dsSceneShadowManager_typeName = "ShadowManager";

static dsCustomSceneResourceType resourceType;
const dsCustomSceneResourceType* dsSceneShadowManager_type(void)
{
	return &resourceType;
}

dsSceneShadowManager* dsSceneShadowManager_create(dsAllocator* allocator,
	dsSceneLightShadows* const* lightShadows, uint32_t lightShadowsCount)
{
	if (!allocator || !lightShadows || lightShadowsCount == 0)
	{
		destroyLightShadows(lightShadows, lightShadowsCount);
		errno = EINVAL;
		return NULL;
	}

	uint32_t tableSize = dsHashTable_tableSize(lightShadowsCount);
	size_t namedPoolSize = dsPoolAllocator_bufferSize(sizeof(NamedShadowsNode), lightShadowsCount);
	size_t lightPoolSize = dsPoolAllocator_bufferSize(sizeof(LightShadowsNode), lightShadowsCount);
	size_t hashTableSize = dsHashTable_fullAllocSize(tableSize);
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneShadowManager)) + namedPoolSize +
		lightPoolSize + hashTableSize*2;
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		int prevErrno = errno;
		destroyLightShadows(lightShadows, lightShadowsCount);
		errno = prevErrno;
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsSceneShadowManager* shadowManager = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneShadowManager);
	DS_ASSERT(shadowManager);

	shadowManager->allocator = dsAllocator_keepPointer(allocator);
	DS_VERIFY(dsPoolAllocator_initialize(&shadowManager->namedShadowsPool, sizeof(NamedShadowsNode),
		lightShadowsCount, dsAllocator_alloc((dsAllocator*)&bufferAlloc, namedPoolSize),
		namedPoolSize));
	DS_VERIFY(dsPoolAllocator_initialize(&shadowManager->lightShadowsPool, sizeof(LightShadowsNode),
		lightShadowsCount, dsAllocator_alloc((dsAllocator*)&bufferAlloc, lightPoolSize),
		lightPoolSize));

	shadowManager->namedShadowsTable =
		(dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, hashTableSize);
	DS_VERIFY(dsHashTable_initialize(shadowManager->namedShadowsTable, tableSize, &dsHashString,
		&dsHashStringEqual));

	shadowManager->lightShadowsTable =
		(dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, hashTableSize);
	DS_VERIFY(dsHashTable_initialize(shadowManager->lightShadowsTable, tableSize, &dsHashIdentity,
		&dsHash32Equal));

	for (uint32_t i = 0; i < lightShadowsCount; ++i)
	{
		dsSceneLightShadows* curShadows = lightShadows[i];
		if (!curShadows)
		{
			destroyLightShadows(lightShadows, lightShadowsCount);
			if (shadowManager->allocator)
				dsAllocator_free(shadowManager->allocator, shadowManager);
			errno = EINVAL;
			return NULL;
		}

		NamedShadowsNode* namedNode =
			DS_ALLOCATE_OBJECT(&shadowManager->namedShadowsPool, NamedShadowsNode);
		DS_ASSERT(namedNode);

		namedNode->lightShadows = curShadows;
		if (!dsHashTable_insert(shadowManager->namedShadowsTable, curShadows->name,
				(dsHashTableNode*)namedNode, NULL))
		{
			DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "Duplicate scene light shadows '%s'.",
				curShadows->name);
			destroyLightShadows(lightShadows, lightShadowsCount);
			if (shadowManager->allocator)
				dsAllocator_free(shadowManager->allocator, shadowManager);
			errno = EINVAL;
			return NULL;
		}

		uint32_t lightID = dsSceneLightShadows_getLightID(curShadows);
		if (lightID)
		{
			LightShadowsNode* lightNode =
				DS_ALLOCATE_OBJECT(&shadowManager->lightShadowsPool, LightShadowsNode);
			DS_ASSERT(lightNode);
			lightNode->key = lightID;
			lightNode->lightShadows = curShadows;

			if (!dsHashTable_insert(shadowManager->lightShadowsTable, &lightNode->key,
					(dsHashTableNode*)lightNode, NULL))
			{
				DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
					"Duplicate light name for scene light shadows '%s'.", curShadows->name);
				destroyLightShadows(lightShadows, lightShadowsCount);
				if (shadowManager->allocator)
					dsAllocator_free(shadowManager->allocator, shadowManager);
				errno = EINVAL;
				return NULL;
			}
		}
	}

	return shadowManager;
}

uint32_t dsSceneShadowManager_getLightShadowsCount(const dsSceneShadowManager* shadowManager)
{
	return shadowManager ? (uint32_t)shadowManager->lightShadowsPool.chunkCount : 0;
}

dsSceneLightShadows* dsSceneShadowManager_findLightShadows(
	const dsSceneShadowManager* shadowManager, const char* name)
{
	if (!shadowManager || !name)
		return NULL;

	dsHashTableNode* node = dsHashTable_find(shadowManager->namedShadowsTable, name);
	if (!node)
		return NULL;

	return ((NamedShadowsNode*)node)->lightShadows;
}

dsSceneLightShadows* dsSceneShadowManager_findShadowsForLightName(
	const dsSceneShadowManager* shadowManager, const char* lightName)
{
	if (!shadowManager || !lightName)
		return NULL;

	uint32_t key = dsHashString(lightName);
	dsHashTableNode* node = dsHashTable_find(shadowManager->lightShadowsTable, &key);
	if (!node)
		return NULL;

	return ((LightShadowsNode*)node)->lightShadows;
}

dsSceneLightShadows* dsSceneShadowManager_findShadowsForLightID(
	const dsSceneShadowManager* shadowManager, uint32_t lightID)
{
	if (!shadowManager || !lightID)
		return NULL;

	dsHashTableNode* node = dsHashTable_find(shadowManager->lightShadowsTable, &lightID);
	if (!node)
		return NULL;

	return ((LightShadowsNode*)node)->lightShadows;
}

bool dsSceneShadowManager_setShadowsLightName(dsSceneShadowManager* shadowManager,
	dsSceneLightShadows* lightShadows, const char* lightName)
{
	return dsSceneShadowManager_setShadowsLightID(shadowManager, lightShadows,
		lightName ? dsHashString(lightName) : 0);
}

bool dsSceneShadowManager_setShadowsLightID(dsSceneShadowManager* shadowManager,
	dsSceneLightShadows* lightShadows, uint32_t lightID)
{
	if (!shadowManager || !lightShadows)
	{
		errno = EINVAL;
		return false;
	}

	if (dsSceneShadowManager_findLightShadows(shadowManager, lightShadows->name) != lightShadows)
	{
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
			"Light shadows '%s' isn't associated with this shadow manager.", lightShadows->name);
		errno = ENOTFOUND;
		return false;
	}

	if (lightID == lightShadows->lightID)
		return true;

	dsAllocator* allocator = (dsAllocator*)&shadowManager->lightShadowsPool;
	uint32_t prevID = lightShadows->lightID;
	LightShadowsNode* node;
	if (lightShadows->lightID)
		node = (LightShadowsNode*)dsHashTable_remove(shadowManager->lightShadowsTable, &lightID);
	else
		node = DS_ALLOCATE_OBJECT(allocator, LightShadowsNode);

	// Should be guaranteed so long as we check that the shadow manager is associated with this
	// shadow manager.
	DS_ASSERT(node);

	if (lightID)
	{
		node->key = lightID;
		node->lightShadows = lightShadows;
		if (!dsHashTable_insert(shadowManager->lightShadowsTable, &node->key,
				(dsHashTableNode*)node, NULL))
		{
			DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
				"Duplicate light name for scene light shadows '%s'.", lightShadows->name);
			// Restore to previous state.
			if (prevID)
			{
				node->key = lightID;
				DS_VERIFY(dsHashTable_insert(shadowManager->lightShadowsTable, &node->key,
					(dsHashTableNode*)node, NULL));
			}
			else
				DS_VERIFY(dsAllocator_free(allocator, node));
			return false;
		}
	}
	else
		DS_VERIFY(dsAllocator_free(allocator, node));

	return true;
}

bool dsSceneShadowManager_prepare(dsSceneShadowManager* shadowManager, const dsView* view,
	const dsSceneItemList* itemList)
{
	if (!shadowManager || !view || !itemList)
	{
		errno = EINVAL;
		return false;
	}

	// Iterate over all shadows rather than just ones associated with lights to ensure that they are
	// properly marked as invalid.
	for (dsListNode* node = shadowManager->namedShadowsTable->list.head; node; node = node->next)
	{
		dsSceneLightShadows* shadows = ((NamedShadowsNode*)node)->lightShadows;
		if (!dsSceneLightShadows_prepare(shadows, view, itemList))
			return false;
	}

	return true;
}

uint32_t dsSceneShadowManager_globalTransformGroupCount(const dsSceneShadowManager* shadowManager)
{
	if (!shadowManager)
		return 0;

	uint32_t count = 0;
	for (dsListNode* node = shadowManager->namedShadowsTable->list.head; node; node = node->next)
	{
		dsSceneLightShadows* shadows = ((NamedShadowsNode*)node)->lightShadows;
		count += shadows->transformGroupID != 0;
	}

	return count;
}

bool dsSceneShadowManager_destroy(dsSceneShadowManager* shadowManager)
{
	if (!shadowManager)
		return true;

	for (dsListNode* node = shadowManager->namedShadowsTable->list.head; node; node = node->next)
	{
		dsSceneLightShadows* shadows = ((NamedShadowsNode*)node)->lightShadows;
		if (!dsSceneLightShadows_destroy(shadows))
			return false;
	}

	if (shadowManager->allocator)
		DS_VERIFY(dsAllocator_free(shadowManager->allocator, shadowManager));
	return true;
}

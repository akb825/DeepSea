/*
 * Copyright 2019 Aaron Barany
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

#include <DeepSea/Scene/SceneResources.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/PoolAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <DeepSea/Render/Resources/Texture.h>

#include <string.h>

typedef struct ResourceNode
{
	dsHashTableNode node;
	void* resource;
	char name[DS_MAX_SCENE_RESOURCE_NAME_LENGTH];
	bool owned;
} ResourceNode;

struct dsSceneResources
{
	dsAllocator* allocator;
	dsHashTable* textures;
	dsHashTable* buffers;
	dsHashTable* shaderModules;
	dsHashTable* shaders;
	dsHashTable* drawGeometries;
	dsPoolAllocator nodePool;
	uint32_t maxTextures;
	uint32_t maxBuffers;
	uint32_t maxShaderModules;
	uint32_t maxShaders;
	uint32_t maxDrawGeometries;
	uint32_t refCount;
};

static dsHashTable* createHashTable(dsBufferAllocator* allocator, uint32_t maxItems)
{
	uint32_t tableSize = dsHashTable_getTableSize(maxItems);
	dsHashTable* hashTable = (dsHashTable*)dsAllocator_alloc((dsAllocator*)allocator,
		dsHashTable_fullAllocSize(tableSize));
	DS_ASSERT(hashTable);
	DS_VERIFY(dsHashTable_initialize(hashTable, tableSize, dsHashString, dsHashStringEqual));
	return hashTable;
}

static void destroy(dsSceneResources* resources)
{
	if (resources->textures)
	{
		for (dsListNode* node = resources->textures->list.head; node; node = node->next)
		{
			ResourceNode* resourceNode = (ResourceNode*)node;
			if (resourceNode->owned)
				dsTexture_destroy((dsTexture*)resourceNode->resource);
		}
	}

	if (resources->buffers)
	{
		for (dsListNode* node = resources->buffers->list.head; node; node = node->next)
		{
			ResourceNode* resourceNode = (ResourceNode*)node;
			if (resourceNode->owned)
				dsGfxBuffer_destroy((dsGfxBuffer*)resourceNode->resource);
		}
	}

	if (resources->shaders)
	{
		for (dsListNode* node = resources->shaders->list.head; node; node = node->next)
		{
			ResourceNode* resourceNode = (ResourceNode*)node;
			if (resourceNode->owned)
				dsShader_destroy((dsShader*)resourceNode->resource);
		}
	}

	if (resources->shaderModules)
	{
		for (dsListNode* node = resources->shaderModules->list.head; node; node = node->next)
		{
			ResourceNode* resourceNode = (ResourceNode*)node;
			if (resourceNode->owned)
				dsShaderModule_destroy((dsShaderModule*)resourceNode->resource);
		}
	}

	if (resources->drawGeometries)
	{
		for (dsListNode* node = resources->drawGeometries->list.head; node; node = node->next)
		{
			ResourceNode* resourceNode = (ResourceNode*)node;
			if (resourceNode->owned)
				dsDrawGeometry_destroy((dsDrawGeometry*)resourceNode->resource);
		}
	}

	if (resources->allocator)
		DS_VERIFY(dsAllocator_free(resources->allocator, resources));
}

size_t dsSceneResources_sizeof(void)
{
	return sizeof(dsSceneResources);
}

size_t dsSceneResources_fullAllocSize(uint32_t maxTextures, uint32_t maxBuffers,
	uint32_t maxShaderModules, uint32_t maxShaders, uint32_t maxDrawGeometries)
{
	uint32_t totalNodes = maxTextures + maxBuffers + maxShaderModules + maxShaders +
		maxDrawGeometries;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneResources));
	if (maxTextures > 0)
		fullSize += dsHashTable_fullAllocSize(dsHashTable_getTableSize(maxTextures));
	if (maxBuffers > 0)
		fullSize += dsHashTable_fullAllocSize(dsHashTable_getTableSize(maxBuffers));
	if (maxShaderModules > 0)
		fullSize += dsHashTable_fullAllocSize(dsHashTable_getTableSize(maxShaderModules));
	if (maxShaders > 0)
		fullSize += dsHashTable_fullAllocSize(dsHashTable_getTableSize(maxShaders));
	if (maxDrawGeometries > 0)
		fullSize += dsHashTable_fullAllocSize(dsHashTable_getTableSize(maxDrawGeometries));
	return fullSize + dsPoolAllocator_bufferSize(sizeof(ResourceNode), totalNodes);
}

dsSceneResources* dsSceneResources_create(dsAllocator* allocator, uint32_t maxTextures,
	uint32_t maxBuffers, uint32_t maxShaderModules, uint32_t maxShaders, uint32_t maxDrawGeometries)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = dsSceneResources_fullAllocSize(maxTextures, maxBuffers, maxShaderModules,
		maxShaders, maxDrawGeometries);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	dsSceneResources* sceneResources = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneResources);
	DS_ASSERT(sceneResources);

	sceneResources->allocator = dsAllocator_keepPointer(allocator);

	if (maxTextures > 0)
		sceneResources->textures = createHashTable(&bufferAlloc, maxTextures);
	else
		sceneResources->textures = NULL;
	if (maxBuffers > 0)
		sceneResources->buffers = createHashTable(&bufferAlloc, maxBuffers);
	else
		sceneResources->buffers = NULL;
	if (maxShaderModules > 0)
		sceneResources->shaderModules = createHashTable(&bufferAlloc, maxShaderModules);
	else
		sceneResources->shaderModules = NULL;
	if (maxShaders > 0)
		sceneResources->shaders = createHashTable(&bufferAlloc, maxShaders);
	else
		sceneResources->shaders = NULL;
	if (maxDrawGeometries > 0)
		sceneResources->drawGeometries = createHashTable(&bufferAlloc, maxDrawGeometries);
	else
		sceneResources->drawGeometries = NULL;

	uint32_t totalNodes = maxTextures + maxBuffers + maxShaderModules + maxShaders +
		maxDrawGeometries;
	size_t poolSize = dsPoolAllocator_bufferSize(sizeof(ResourceNode), totalNodes);
	void* poolBuffer = dsAllocator_alloc((dsAllocator*)&bufferAlloc, poolSize);
	DS_ASSERT(poolBuffer);
	DS_VERIFY(dsPoolAllocator_initialize(&sceneResources->nodePool, sizeof(ResourceNode),
		totalNodes, poolBuffer, poolSize));

	sceneResources->maxTextures = maxTextures;
	sceneResources->maxBuffers = maxBuffers;
	sceneResources->maxShaderModules = maxShaderModules;
	sceneResources->maxShaders = maxShaders;
	sceneResources->maxDrawGeometries = maxDrawGeometries;
	sceneResources->refCount = 1;
	return sceneResources;
}

uint32_t dsSceneResources_getRemainingTextures(const dsSceneResources* resources)
{
	if (!resources || !resources->textures)
		return 0;

	return resources->maxTextures - (uint32_t)resources->textures->list.length;
}

bool dsSceneResources_addTexture(dsSceneResources* resources, const char* name, dsTexture* texture,
	bool own)
{
	if (!resources || !name || !texture)
	{
		errno = EINVAL;
		return false;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_SCENE_RESOURCE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Texture name '%s' exceeds maximum size of %u.",
			name, DS_MAX_SCENE_RESOURCE_NAME_LENGTH);
		return false;
	}

	dsHashTableNode* foundNode = dsHashTable_find(resources->textures, name);
	if (foundNode)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Texture '%s' has already been added.", name);
		return false;
	}

	if (!resources->textures || resources->textures->list.length >= resources->maxTextures)
	{
		errno = ESIZE;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Maximum number of textures has been exceeded.");
		return false;
	}

	ResourceNode* node = DS_ALLOCATE_OBJECT(&resources->nodePool, ResourceNode);
	DS_ASSERT(node);
	strncpy(node->name, name, nameLength + 1);
	node->resource = texture;
	node->owned = own;
	DS_VERIFY(dsHashTable_insert(resources->textures, node->name, (dsHashTableNode*)node, NULL));
	return true;
}

bool dsSceneResource_removeTexture(dsSceneResources* resources, const char* name,
	bool relinquish)
{
	if (!resources || !resources->textures || !name)
		return false;

	ResourceNode* node = (ResourceNode*)dsHashTable_remove(resources->textures, name);
	if (!node)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Texture '%s' not found.", name);
		return false;
	}

	DS_ASSERT(node->resource);
	if (node->owned && !relinquish)
	{
		if (!dsTexture_destroy((dsTexture*)node->resource))
		{
			// Put back into the hash table.
			DS_VERIFY(dsHashTable_insert(resources->textures, node->name,
				(dsHashTableNode*)node, NULL));
			return false;
		}
	}

	DS_VERIFY(dsAllocator_free((dsAllocator*)&resources->nodePool, node));
	return true;
}

dsTexture* dsSceneResources_findTexture(const dsSceneResources* resources, const char* name)
{
	if (!resources || !name)
		return NULL;

	ResourceNode* node = (ResourceNode*)dsHashTable_find(resources->textures, name);
	if (!node)
		return NULL;

	return (dsTexture*)node->resource;
}

uint32_t dsSceneResources_getRemainingBuffers(const dsSceneResources* resources)
{
	if (!resources || !resources->buffers)
		return 0;

	return resources->maxBuffers - (uint32_t)resources->buffers->list.length;
}

bool dsSceneResources_addBuffer(dsSceneResources* resources, const char* name, dsGfxBuffer* buffer,
	bool own)
{
	if (!resources || !name || !buffer)
	{
		errno = EINVAL;
		return false;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_SCENE_RESOURCE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Buffer name '%s' exceeds maximum size of %u.",
			name, DS_MAX_SCENE_RESOURCE_NAME_LENGTH);
		return false;
	}

	dsHashTableNode* foundNode = dsHashTable_find(resources->buffers, name);
	if (foundNode)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Buffer '%s' has already been added.", name);
		return false;
	}

	if (!resources->buffers || resources->buffers->list.length >= resources->maxBuffers)
	{
		errno = ESIZE;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Maximum number of buffers has been exceeded.");
		return false;
	}

	ResourceNode* node = DS_ALLOCATE_OBJECT(&resources->nodePool, ResourceNode);
	DS_ASSERT(node);
	strncpy(node->name, name, nameLength + 1);
	node->resource = buffer;
	node->owned = own;
	DS_VERIFY(dsHashTable_insert(resources->buffers, node->name, (dsHashTableNode*)node, NULL));
	return true;
}

bool dsSceneResource_removeBuffer(dsSceneResources* resources, const char* name,
	bool relinquish)
{
	if (!resources || !resources->buffers || !name)
		return false;

	ResourceNode* node = (ResourceNode*)dsHashTable_remove(resources->buffers, name);
	if (!node)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Buffer '%s' not found.", name);
		return false;
	}

	DS_ASSERT(node->resource);
	if (node->owned && !relinquish)
	{
		if (!dsGfxBuffer_destroy((dsGfxBuffer*)node->resource))
		{
			// Put back into the hash table.
			DS_VERIFY(dsHashTable_insert(resources->buffers, node->name,
				(dsHashTableNode*)node, NULL));
			return false;
		}
	}

	DS_VERIFY(dsAllocator_free((dsAllocator*)&resources->nodePool, node));
	return true;
}

dsGfxBuffer* dsSceneResources_findBuffer(const dsSceneResources* resources, const char* name)
{
	if (!resources || !name)
		return NULL;

	ResourceNode* node = (ResourceNode*)dsHashTable_find(resources->buffers, name);
	if (!node)
		return NULL;

	return (dsGfxBuffer*)node->resource;
}

uint32_t dsSceneResources_getRemainingShaderModules(const dsSceneResources* resources)
{
	if (!resources || !resources->shaderModules)
		return 0;

	return resources->maxShaderModules - (uint32_t)resources->shaderModules->list.length;
}

bool dsSceneResources_addShaderModule(dsSceneResources* resources, const char* name,
	dsShaderModule* shaderModule, bool own)
{
	if (!resources || !name || !shaderModule)
	{
		errno = EINVAL;
		return false;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_SCENE_RESOURCE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "ShaderModule name '%s' exceeds maximum size of %u.",
			name, DS_MAX_SCENE_RESOURCE_NAME_LENGTH);
		return false;
	}

	dsHashTableNode* foundNode = dsHashTable_find(resources->shaderModules, name);
	if (foundNode)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "ShaderModule '%s' has already been added.", name);
		return false;
	}

	if (!resources->shaderModules || resources->shaderModules->list.length >=
		resources->maxShaderModules)
	{
		errno = ESIZE;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Maximum number of shaderModules has been exceeded.");
		return false;
	}

	ResourceNode* node = DS_ALLOCATE_OBJECT(&resources->nodePool, ResourceNode);
	DS_ASSERT(node);
	strncpy(node->name, name, nameLength + 1);
	node->resource = shaderModule;
	node->owned = own;
	DS_VERIFY(dsHashTable_insert(resources->shaderModules, node->name, (dsHashTableNode*)node,
		NULL));
	return true;
}

bool dsSceneResource_removeShaderModule(dsSceneResources* resources, const char* name,
	bool relinquish)
{
	if (!resources || !resources->shaderModules || !name)
		return false;

	ResourceNode* node = (ResourceNode*)dsHashTable_remove(resources->shaderModules, name);
	if (!node)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "ShaderModule '%s' not found.", name);
		return false;
	}

	DS_ASSERT(node->resource);
	if (node->owned && !relinquish)
	{
		if (!dsShaderModule_destroy((dsShaderModule*)node->resource))
		{
			// Put back into the hash table.
			DS_VERIFY(dsHashTable_insert(resources->shaderModules, node->name,
				(dsHashTableNode*)node, NULL));
			return false;
		}
	}

	DS_VERIFY(dsAllocator_free((dsAllocator*)&resources->nodePool, node));
	return true;
}

dsShaderModule* dsSceneResources_findShaderModule(const dsSceneResources* resources,
	const char* name)
{
	if (!resources || !name)
		return NULL;

	ResourceNode* node = (ResourceNode*)dsHashTable_find(resources->shaderModules, name);
	if (!node)
		return NULL;

	return (dsShaderModule*)node->resource;
}

uint32_t dsSceneResources_getRemainingShaders(const dsSceneResources* resources)
{
	if (!resources || !resources->shaders)
		return 0;

	return resources->maxShaders - (uint32_t)resources->shaders->list.length;
}

bool dsSceneResources_addShader(dsSceneResources* resources, const char* name, dsShader* shader,
	bool own)
{
	if (!resources || !name || !shader)
	{
		errno = EINVAL;
		return false;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_SCENE_RESOURCE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Shader name '%s' exceeds maximum size of %u.",
			name, DS_MAX_SCENE_RESOURCE_NAME_LENGTH);
		return false;
	}

	dsHashTableNode* foundNode = dsHashTable_find(resources->shaders, name);
	if (foundNode)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Shader '%s' has already been added.", name);
		return false;
	}

	if (!resources->shaders || resources->shaders->list.length >= resources->maxShaders)
	{
		errno = ESIZE;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Maximum number of shaders has been exceeded.");
		return false;
	}

	ResourceNode* node = DS_ALLOCATE_OBJECT(&resources->nodePool, ResourceNode);
	DS_ASSERT(node);
	strncpy(node->name, name, nameLength + 1);
	node->resource = shader;
	node->owned = own;
	DS_VERIFY(dsHashTable_insert(resources->shaders, node->name, (dsHashTableNode*)node, NULL));
	return true;
}

bool dsSceneResource_removeShader(dsSceneResources* resources, const char* name,
	bool relinquish)
{
	if (!resources || !resources->shaders || !name)
		return false;

	ResourceNode* node = (ResourceNode*)dsHashTable_remove(resources->shaders, name);
	if (!node)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Shader '%s' not found.", name);
		return false;
	}

	DS_ASSERT(node->resource);
	if (node->owned && !relinquish)
	{
		if (!dsShader_destroy((dsShader*)node->resource))
		{
			// Put back into the hash table.
			DS_VERIFY(dsHashTable_insert(resources->shaders, node->name,
				(dsHashTableNode*)node, NULL));
			return false;
		}
	}

	DS_VERIFY(dsAllocator_free((dsAllocator*)&resources->nodePool, node));
	return true;
}

dsShader* dsSceneResources_findShader(const dsSceneResources* resources, const char* name)
{
	if (!resources || !name)
		return NULL;

	ResourceNode* node = (ResourceNode*)dsHashTable_find(resources->shaders, name);
	if (!node)
		return NULL;

	return (dsShader*)node->resource;
}

uint32_t dsSceneResources_getRemainingDrawGeometries(const dsSceneResources* resources)
{
	if (!resources || !resources->drawGeometries)
		return 0;

	return resources->maxDrawGeometries - (uint32_t)resources->drawGeometries->list.length;
}

bool dsSceneResources_addDrawGeometry(dsSceneResources* resources, const char* name,
	dsDrawGeometry* drawGeometry, bool own)
{
	if (!resources || !name || !drawGeometry)
	{
		errno = EINVAL;
		return false;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_SCENE_RESOURCE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "DrawGeometry name '%s' exceeds maximum size of %u.",
			name, DS_MAX_SCENE_RESOURCE_NAME_LENGTH);
		return false;
	}

	dsHashTableNode* foundNode = dsHashTable_find(resources->drawGeometries, name);
	if (foundNode)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "DrawGeometry '%s' has already been added.", name);
		return false;
	}

	if (!resources->drawGeometries || resources->drawGeometries->list.length >=
			resources->maxDrawGeometries)
	{
		errno = ESIZE;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Maximum number of drawGeometries has been exceeded.");
		return false;
	}

	ResourceNode* node = DS_ALLOCATE_OBJECT(&resources->nodePool, ResourceNode);
	DS_ASSERT(node);
	strncpy(node->name, name, nameLength + 1);
	node->resource = drawGeometry;
	node->owned = own;
	DS_VERIFY(dsHashTable_insert(resources->drawGeometries, node->name, (dsHashTableNode*)node,
		NULL));
	return true;
}

bool dsSceneResource_removeDrawGeometry(dsSceneResources* resources, const char* name,
	bool relinquish)
{
	if (!resources || !resources->drawGeometries || !name)
		return false;

	ResourceNode* node = (ResourceNode*)dsHashTable_remove(resources->drawGeometries, name);
	if (!node)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "DrawGeometry '%s' not found.", name);
		return false;
	}

	DS_ASSERT(node->resource);
	if (node->owned && !relinquish)
	{
		if (!dsDrawGeometry_destroy((dsDrawGeometry*)node->resource))
		{
			// Put back into the hash table.
			DS_VERIFY(dsHashTable_insert(resources->drawGeometries, node->name,
				(dsHashTableNode*)node, NULL));
			return false;
		}
	}

	DS_VERIFY(dsAllocator_free((dsAllocator*)&resources->nodePool, node));
	return true;
}

dsDrawGeometry* dsSceneResources_findDrawGeometry(const dsSceneResources* resources,
	const char* name)
{
	if (!resources || !name)
		return NULL;

	ResourceNode* node = (ResourceNode*)dsHashTable_find(resources->drawGeometries, name);
	if (!node)
		return NULL;

	return (dsDrawGeometry*)node->resource;
}

dsSceneResources* dsSceneResources_addRef(dsSceneResources* resources)
{
	if (!resources)
		return NULL;

	DS_ATOMIC_FETCH_ADD32(&resources->refCount, 1);
	return resources;
}

void dsSceneResources_freeRef(dsSceneResources* resources)
{
	if (!resources)
		return;

	if (DS_ATOMIC_FETCH_ADD32(&resources->refCount, -1) == 1)
		destroy(resources);
}

/*
 * Copyright 2017 Aaron Barany
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

#include <DeepSea/VectorDraw/VectorResources.h>

#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/PoolAllocator.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/TextureData.h>
#include <DeepSea/Text/FaceGroup.h>
#include <DeepSea/Text/Font.h>
#include <string.h>

struct dsVectorResources
{
	dsAllocator* allocator;
	dsHashTable* textureTable;
	dsPoolAllocator texturePool;
	dsHashTable* faceGroupTable;
	dsPoolAllocator faceGroupPool;
	dsHashTable* fontTable;
	dsPoolAllocator fontPool;
};

typedef struct dsTextureNode
{
	dsHashTableNode node;
	char name[DS_MAX_VECTOR_RESOURCE_NAME_LENGTH];
	dsTexture* texture;
	bool owned;
} dsTextureNode;

typedef struct dsFaceGroupNode
{
	dsHashTableNode node;
	char name[DS_MAX_VECTOR_RESOURCE_NAME_LENGTH];
	dsFaceGroup* faceGroup;
	bool owned;
} dsFaceGroupNode;

typedef struct dsFontNode
{
	dsHashTableNode node;
	char name[DS_MAX_VECTOR_RESOURCE_NAME_LENGTH];
	dsFont* font;
	bool owned;
} dsFontNode;

typedef struct dsResourceInfo
{
	dsAllocator* allocator;
	const char* basePath;
	dsFileResourceType type;
} dsResourceInfo;

static const float loadFactor = 0.75f;

static uint32_t tableSize(uint32_t maxSize)
{
	return (uint32_t)roundf((float)maxSize/loadFactor);
}

static dsTexture* loadTextureFile(void* userData, dsResourceManager* resourceManager,
	dsAllocator* allocator, dsAllocator* tempAllocator, const char* path, dsTextureUsage usage,
	dsGfxMemory memoryHints)
{
	const char* basePath = (const char*)userData;
	char finalPath[DS_PATH_MAX];
	if (!dsPath_combine(finalPath, sizeof(finalPath), basePath, path))
	{
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Path '%s%c%s' is too long.", basePath,
			DS_PATH_SEPARATOR, path);
		return NULL;
	}

	return dsTextureData_loadFileToTexture(resourceManager, allocator, tempAllocator, finalPath,
		NULL, usage, memoryHints);
}

static bool loadFontFaceFile(void* userData, dsFaceGroup* faceGroup, const char* path,
	const char* name)
{
	const char* basePath = (const char*)userData;
	char finalPath[DS_PATH_MAX];
	if (!dsPath_combine(finalPath, sizeof(finalPath), basePath, path))
	{
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Path '%s%c%s' is too long.", basePath,
			DS_PATH_SEPARATOR, path);
		return false;
	}

	return dsFaceGroup_loadFaceFile(faceGroup, finalPath, name);
}

static dsTexture* loadTextureResource(void* userData, dsResourceManager* resourceManager,
	dsAllocator* allocator, dsAllocator* tempAllocator, const char* path, dsTextureUsage usage,
	dsGfxMemory memoryHints)
{
	dsResourceInfo* resourceInfo = (dsResourceInfo*)userData;
	char finalPath[DS_PATH_MAX];
	if (!dsPath_combine(finalPath, sizeof(finalPath), resourceInfo->basePath, path))
	{
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Path '%s%c%s' is too long.", resourceInfo->basePath,
			DS_PATH_SEPARATOR, path);
		return NULL;
	}

	return dsTextureData_loadResourceToTexture(resourceManager, allocator, tempAllocator,
		resourceInfo->type, finalPath, NULL, usage, memoryHints);
}

static bool loadFontFaceResource(void* userData, dsFaceGroup* faceGroup, const char* path,
	const char* name)
{
	dsResourceInfo* resourceInfo = (dsResourceInfo*)userData;
	char finalPath[DS_PATH_MAX];
	if (!dsPath_combine(finalPath, sizeof(finalPath), resourceInfo->basePath, path))
	{
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Path '%s%c%s' is too long.", resourceInfo->basePath,
			DS_PATH_SEPARATOR, path);
		return false;
	}

	return dsFaceGroup_loadFaceResource(faceGroup, resourceInfo->allocator, resourceInfo->type,
		finalPath, name);
}

dsVectorResources* dsVectorResources_loadImpl(dsAllocator* allocator, dsAllocator* scratchAllocator,
	dsResourceManager* resourceManager, const void* data, size_t size, void* loadUserData,
	dsLoadVectorResourcesTextureFunction loadTextureFunc,
	dsLoadVectorResourcesFontFaceFunction loadFontFaceFunc, const dsTextQuality* qualityRemap,
	const char* name);

size_t dsVectorResources_fullAllocSize(uint32_t maxTextures, uint32_t maxFaceGroups,
	uint32_t maxFonts)
{
	uint32_t textureTableSize = tableSize(maxTextures);
	uint32_t faceGroupTableSize = tableSize(maxFaceGroups);
	uint32_t fontTableSize = tableSize(maxFonts);

	return DS_ALIGNED_SIZE(sizeof(dsVectorResources)) +
		dsHashTable_fullAllocSize(textureTableSize) +
		dsPoolAllocator_bufferSize(sizeof(dsTextureNode), maxTextures) +
		dsHashTable_fullAllocSize(faceGroupTableSize) +
		dsPoolAllocator_bufferSize(sizeof(dsFaceGroupNode), maxFaceGroups) +
		dsHashTable_fullAllocSize(fontTableSize) +
		dsPoolAllocator_bufferSize(sizeof(dsFontNode), maxFonts);
}

dsVectorResources* dsVectorReosurces_create(dsAllocator* allocator, uint32_t maxTextures,
	uint32_t maxFaceGroups, uint32_t maxFonts)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = dsVectorResources_fullAllocSize(maxTextures, maxFaceGroups, maxFonts);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsVectorResources* resources = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc,
		dsVectorResources);
	DS_ASSERT(resources);

	resources->allocator = dsAllocator_keepPointer(allocator);

	if (maxTextures > 0)
	{
		uint32_t textureTableSize = tableSize(maxTextures);
		resources->textureTable = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
			dsHashTable_fullAllocSize(textureTableSize));
		DS_ASSERT(resources->textureTable);
		DS_VERIFY(dsHashTable_initialize(resources->textureTable, textureTableSize, &dsHashString,
			&dsHashStringEqual));

		size_t poolSize = dsPoolAllocator_bufferSize(sizeof(dsTextureNode), maxTextures);
		void* pool = dsAllocator_alloc((dsAllocator*)&bufferAlloc, poolSize);
		DS_ASSERT(pool);
		DS_VERIFY(dsPoolAllocator_initialize(&resources->texturePool, sizeof(dsTextureNode),
			maxTextures, pool, poolSize));
	}
	else
	{
		resources->textureTable = NULL;
		memset(&resources->textureTable, 0, sizeof(resources->textureTable));
	}

	if (maxFaceGroups > 0)
	{
		uint32_t faceGroupTableSize = tableSize(maxFaceGroups);
		resources->faceGroupTable = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
			dsHashTable_fullAllocSize(faceGroupTableSize));
		DS_ASSERT(resources->faceGroupTable);
		DS_VERIFY(dsHashTable_initialize(resources->faceGroupTable, faceGroupTableSize, &dsHashString,
			&dsHashStringEqual));

		size_t poolSize = dsPoolAllocator_bufferSize(sizeof(dsFaceGroupNode), maxFaceGroups);
		void* pool = dsAllocator_alloc((dsAllocator*)&bufferAlloc, poolSize);
		DS_ASSERT(pool);
		DS_VERIFY(dsPoolAllocator_initialize(&resources->faceGroupPool, sizeof(dsFaceGroupNode),
			maxTextures, pool, poolSize));
	}
	else
	{
		resources->faceGroupTable = NULL;
		memset(&resources->faceGroupTable, 0, sizeof(resources->faceGroupTable));
	}

	if (maxFonts > 0)
	{
		uint32_t fontTableSize = tableSize(maxFonts);
		resources->fontTable = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
			dsHashTable_fullAllocSize(fontTableSize));
		DS_ASSERT(resources->fontTable);
		DS_VERIFY(dsHashTable_initialize(resources->fontTable, fontTableSize, &dsHashString,
			&dsHashStringEqual));

		size_t poolSize = dsPoolAllocator_bufferSize(sizeof(dsFontNode), maxFonts);
		void* pool = dsAllocator_alloc((dsAllocator*)&bufferAlloc, poolSize);
		DS_ASSERT(pool);
		DS_VERIFY(dsPoolAllocator_initialize(&resources->fontPool, sizeof(dsFontNode),
			maxFonts, pool, poolSize));
	}
	else
	{
		resources->fontTable = NULL;
		memset(&resources->fontTable, 0, sizeof(resources->fontTable));
	}

	return resources;
}

dsVectorResources* dsVectorResources_loadFile(dsAllocator* allocator, dsAllocator* scratchAllocator,
	dsResourceManager* resourceManager, const char* filePath, const dsTextQuality* qualityRemap)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !resourceManager || !filePath)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!scratchAllocator)
		scratchAllocator = allocator;

	char baseDirectory[DS_PATH_MAX];
	if (!dsPath_getDirectoryName(baseDirectory, sizeof(baseDirectory), filePath))
	{
		if (errno == EINVAL)
			baseDirectory[0] = 0;
		else
		{
			DS_PROFILE_FUNC_RETURN(NULL);
		}
	}

	dsFileStream stream;
	if (!dsFileStream_openPath(&stream, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open vector resources file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, (dsStream*)&stream, scratchAllocator);
	dsFileStream_close(&stream);
	if (!buffer)
	{
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsVectorResources* resources = dsVectorResources_loadImpl(allocator, scratchAllocator,
		resourceManager, buffer, size, baseDirectory, &loadTextureFile, &loadFontFaceFile,
		qualityRemap, filePath);
	DS_VERIFY(dsAllocator_free(scratchAllocator, buffer));
	DS_PROFILE_FUNC_RETURN(resources);
}

dsVectorResources* dsVectorResources_loadResource(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsResourceManager* resourceManager, dsFileResourceType type,
	const char* filePath, const dsTextQuality* qualityRemap)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !resourceManager || !filePath)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!scratchAllocator)
		scratchAllocator = allocator;

	char baseDirectory[DS_PATH_MAX];
	if (!dsPath_getDirectoryName(baseDirectory, sizeof(baseDirectory), filePath))
	{
		if (errno == EINVAL)
			baseDirectory[0] = 0;
		else
		{
			DS_PROFILE_FUNC_RETURN(NULL);
		}
	}

	dsResourceStream stream;
	if (!dsResourceStream_open(&stream, type, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open vector resources file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, (dsStream*)&stream, scratchAllocator);
	dsStream_close((dsStream*)&stream);
	if (!buffer)
	{
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsResourceInfo resourceInfo = {scratchAllocator, baseDirectory, type};
	dsVectorResources* resources = dsVectorResources_loadImpl(allocator, scratchAllocator,
		resourceManager, buffer, size, &resourceInfo, &loadTextureResource, &loadFontFaceResource,
		qualityRemap, filePath);
	DS_VERIFY(dsAllocator_free(scratchAllocator, buffer));
	DS_PROFILE_FUNC_RETURN(resources);
}

dsVectorResources* dsVectorResources_loadStream(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsResourceManager* resourceManager, dsStream* stream,
	void* loadUserData, dsLoadVectorResourcesTextureFunction loadTextureFunc,
	dsLoadVectorResourcesFontFaceFunction loadFontFaceFunc, const dsTextQuality* qualityRemap)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !resourceManager || !stream || !loadTextureFunc || !loadFontFaceFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!scratchAllocator)
		scratchAllocator = allocator;

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, stream, scratchAllocator);
	if (!buffer)
	{
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsVectorResources* resources = dsVectorResources_loadImpl(allocator, scratchAllocator,
		resourceManager, buffer, size, loadUserData, loadTextureFunc, loadFontFaceFunc,
		qualityRemap, NULL);
	DS_VERIFY(dsAllocator_free(scratchAllocator, buffer));
	DS_PROFILE_FUNC_RETURN(resources);
}

dsVectorResources* dsVectorResources_loadData(dsAllocator* allocator, dsAllocator* scratchAllocator,
	dsResourceManager* resourceManager, const void* data, size_t size, void* loadUserData,
	dsLoadVectorResourcesTextureFunction loadTextureFunc,
	dsLoadVectorResourcesFontFaceFunction loadFontFaceFunc, const dsTextQuality* qualityRemap)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !resourceManager || !data || size == 0 || !loadTextureFunc ||
		!loadFontFaceFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!scratchAllocator)
		scratchAllocator = allocator;

	dsVectorResources* resources = dsVectorResources_loadImpl(allocator, scratchAllocator,
		resourceManager, data, size, loadUserData, loadTextureFunc, loadFontFaceFunc,
		qualityRemap, NULL);
	DS_PROFILE_FUNC_RETURN(resources);
}

uint32_t dsVectorResources_getRemainingTextures(const dsVectorResources* resources)
{
	if (!resources)
		return 0;

	return (uint32_t)resources->texturePool.freeCount;
}

bool dsVectorResources_addTexture(dsVectorResources* resources, const char* name,
	dsTexture* texture, bool own)
{
	if (!resources || !name || !texture)
	{
		errno = EINVAL;
		return false;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_VECTOR_RESOURCE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Texture name '%s' exceeds maximum size of %u.",
			name, DS_MAX_VECTOR_RESOURCE_NAME_LENGTH);
		return false;
	}

	dsHashTableNode* foundNode = dsHashTable_find(resources->textureTable, name);
	if (foundNode)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Texture '%s' has already been added.", name);
		return false;
	}

	if (!resources->textureTable || resources->texturePool.freeCount == 0)
	{
		errno = ESIZE;
		DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG, "Maximum number of textures has been exceeded.");
		return false;
	}

	dsTextureNode* node = DS_ALLOCATE_OBJECT((dsAllocator*)&resources->texturePool, dsTextureNode);
	DS_ASSERT(node);
	strncpy(node->name, name, nameLength + 1);
	node->texture = texture;
	node->owned = own;
	DS_VERIFY(dsHashTable_insert(resources->textureTable, node->name, (dsHashTableNode*)node,
		NULL));
	return true;
}

bool dsVectorResource_removeTexture(dsVectorResources* resources, const char* name,
	bool relinquish)
{
	if (!resources || !name)
		return false;

	dsTextureNode* node = (dsTextureNode*)dsHashTable_remove(resources->textureTable, name);
	if (!node)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Texture '%s' not found.", name);
		return false;
	}

	DS_ASSERT(node->texture);
	if (node->owned && !relinquish)
	{
		if (!dsTexture_destroy(node->texture))
		{
			// Put back into the hash table.
			DS_VERIFY(dsHashTable_insert(resources->textureTable, node->name,
				(dsHashTableNode*)node, NULL));
			return false;
		}
	}

	DS_VERIFY(dsAllocator_free((dsAllocator*)&resources->texturePool, node));
	return true;
}

dsTexture* dsVectorResources_findTexture(const dsVectorResources* resources, const char* name)
{
	if (!resources || !name)
		return NULL;

	dsTextureNode* node = (dsTextureNode*)dsHashTable_find(resources->textureTable, name);
	if (!node)
		return NULL;

	return node->texture;
}

dsFont* dsVectorResources_findFont(const dsVectorResources* resources, const char* name)
{
	if (!resources || !name)
		return NULL;

	dsFontNode* node = (dsFontNode*)dsHashTable_find(resources->fontTable, name);
	if (!node)
		return NULL;

	return node->font;
}

bool dsVectorResources_preloadASCII(dsVectorResources* resources, dsCommandBuffer* commandBuffer)
{
	if (!resources || !commandBuffer)
	{
		errno = EINVAL;
		return false;
	}

	for (dsListNode* node = resources->fontTable->list.head; node; node = node->next)
	{
		dsFont* font = ((dsFontNode*)node)->font;
		if (!dsFont_preloadASCII(font, commandBuffer))
			return false;
	}

	return true;
}

uint32_t dsVectorResources_getRemainingFaceGroups(const dsVectorResources* resources)
{
	if (!resources)
		return 0;

	return (uint32_t)resources->faceGroupPool.freeCount;
}

bool dsVectorResources_addFaceGroup(dsVectorResources* resources, const char* name,
	dsFaceGroup* faceGroup, bool own)
{
	if (!resources || !name || !faceGroup)
	{
		errno = EINVAL;
		return false;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_VECTOR_RESOURCE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Face group name '%s' exceeds maximum size of %u.",
			name, DS_MAX_VECTOR_RESOURCE_NAME_LENGTH);
		return false;
	}

	dsHashTableNode* foundNode = dsHashTable_find(resources->faceGroupTable, name);
	if (foundNode)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Face group '%s' has already been added.", name);
		return false;
	}

	if (!resources->faceGroupTable || resources->faceGroupPool.freeCount == 0)
	{
		errno = ESIZE;
		DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG, "Maximum number of face groups has been exceeded.");
		return false;
	}

	dsFaceGroupNode* node = DS_ALLOCATE_OBJECT((dsAllocator*)&resources->faceGroupPool,
		dsFaceGroupNode);
	DS_ASSERT(node);
	strncpy(node->name, name, nameLength + 1);
	node->faceGroup = faceGroup;
	node->owned = own;
	DS_VERIFY(dsHashTable_insert(resources->faceGroupTable, node->name, (dsHashTableNode*)node,
		NULL));
	return true;
}

bool dsVectorResource_removeFaceGroup(dsVectorResources* resources, const char* name,
	bool relinquish)
{
	if (!resources || !name)
		return false;

	dsFaceGroupNode* node = (dsFaceGroupNode*)dsHashTable_remove(resources->faceGroupTable, name);
	if (!node)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "FaceGroup '%s' not found.", name);
		return false;
	}

	DS_ASSERT(node->faceGroup);
	if (node->owned && !relinquish)
		dsFaceGroup_destroy(node->faceGroup);

	DS_VERIFY(dsAllocator_free((dsAllocator*)&resources->faceGroupPool, node));
	return true;
}

dsFaceGroup* dsVectorResources_findFaceGroup(const dsVectorResources* resources, const char* name)
{
	if (!resources || !name)
		return NULL;

	dsFaceGroupNode* node = (dsFaceGroupNode*)dsHashTable_find(resources->faceGroupTable, name);
	if (!node)
		return NULL;

	return node->faceGroup;
}

uint32_t dsVectorResources_getRemainingFonts(const dsVectorResources* resources)
{
	if (!resources)
		return 0;

	return (uint32_t)resources->fontPool.freeCount;
}

bool dsVectorResources_addFont(dsVectorResources* resources, const char* name,
	dsFont* font, bool own)
{
	if (!resources || !name || !font)
	{
		errno = EINVAL;
		return false;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_VECTOR_RESOURCE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Font name '%s' exceeds maximum size of %u.",
			name, DS_MAX_VECTOR_RESOURCE_NAME_LENGTH);
		return false;
	}

	dsHashTableNode* foundNode = dsHashTable_find(resources->fontTable, name);
	if (foundNode)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Font '%s' has already been added.", name);
		return false;
	}

	if (!resources->fontTable || resources->fontPool.freeCount == 0)
	{
		errno = ESIZE;
		DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG, "Maximum number of fonts has been exceeded.");
		return false;
	}

	dsFontNode* node = DS_ALLOCATE_OBJECT((dsAllocator*)&resources->fontPool, dsFontNode);
	DS_ASSERT(node);
	strncpy(node->name, name, nameLength + 1);
	node->font = font;
	node->owned = own;
	DS_VERIFY(dsHashTable_insert(resources->fontTable, node->name, (dsHashTableNode*)node,
		NULL));
	return true;
}

bool dsVectorResource_removeFont(dsVectorResources* resources, const char* name,
	bool relinquish)
{
	if (!resources || !name)
		return false;

	dsFontNode* node = (dsFontNode*)dsHashTable_remove(resources->fontTable, name);
	if (!node)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Font '%s' not found.", name);
		return false;
	}

	DS_ASSERT(node->font);
	if (node->owned && !relinquish)
	{
		if (!dsFont_destroy(node->font))
		{
			// Put back into the hash table.
			DS_VERIFY(dsHashTable_insert(resources->fontTable, node->name,
				(dsHashTableNode*)node, NULL));
			return false;
		}
	}

	DS_VERIFY(dsAllocator_free((dsAllocator*)&resources->fontPool, node));
	return true;
}

bool dsVectorResources_destroy(dsVectorResources* resources)
{
	if (!resources)
		return true;

	bool firstDestroyed = true;
	DS_UNUSED(firstDestroyed);
	if (resources->textureTable)
	{
		for (dsTextureNode* node = (dsTextureNode*)resources->textureTable->list.head;
			node; node = (dsTextureNode*)node->node.listNode.next)
		{
			if (node->owned)
			{
				if (!dsTexture_destroy(node->texture))
				{
					DS_ASSERT(firstDestroyed);
					return false;
				}
				firstDestroyed = false;
			}
		}
	}

	if (resources->fontTable)
	{
		for (dsFontNode* node = (dsFontNode*)resources->fontTable->list.head;
			node; node = (dsFontNode*)node->node.listNode.next)
		{
			if (node->owned)
			{
				if (!dsFont_destroy(node->font))
				{
					DS_ASSERT(firstDestroyed);
					return false;
				}
				firstDestroyed = false;
			}
		}
	}

	if (resources->faceGroupTable)
	{
		for (dsFaceGroupNode* node = (dsFaceGroupNode*)resources->faceGroupTable->list.head;
			node; node = (dsFaceGroupNode*)node->node.listNode.next)
		{
			if (node->owned)
				dsFaceGroup_destroy(node->faceGroup);
		}
	}

	if (resources->allocator)
		DS_VERIFY(dsAllocator_free(resources->allocator, resources));
	return true;
}

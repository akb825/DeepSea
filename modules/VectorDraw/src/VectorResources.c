/*
 * Copyright 2017-2026 Aaron Barany
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

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/PoolAllocator.h>
#include <DeepSea/Core/Streams/FileArchive.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Streams/RelativePathStream.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Math/Core.h>

#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/TextureData.h>

#include <DeepSea/Text/FaceGroup.h>
#include <DeepSea/Text/Font.h>
#include <DeepSea/Text/TextIcons.h>

#include <DeepSea/VectorDraw/VectorImage.h>

#include <string.h>

typedef struct ResourceNode
{
	dsHashTableNode node;
	char name[DS_MAX_VECTOR_RESOURCE_NAME_LENGTH];
	void* resource;
	dsVectorResourceType type;
	bool owned;
} ResourceNode;

struct dsVectorResources
{
	dsAllocator* allocator;
	dsHashTable* resourceTable;
	dsPoolAllocator resourcePool;
};

static bool destroyResource(dsVectorResourceType type, void* resource)
{
	switch (type)
	{
		case dsVectorResourceType_Texture:
			return dsTexture_destroy((dsTexture*)resource);
		case dsVectorResourceType_VectorImage:
			return dsVectorImage_destroy((dsVectorImage*)resource);
		case dsVectorResourceType_FaceGroup:
			dsFaceGroup_destroy((dsFaceGroup*)resource);
			return true;
		case dsVectorResourceType_TextIcons:
			dsTextIcons_destroy((dsTextIcons*)resource);
			return true;
		case dsVectorResourceType_Font:
			dsFont_destroy((dsFont*)resource);
			return true;
	}
	DS_ASSERT(false);
	// Return true to avoid adding it back to the table.
	return true;
}

dsVectorResources* dsVectorResources_loadImpl(dsAllocator* allocator, dsAllocator* scratchAllocator,
	dsAllocator* resourceAllocator, dsResourceManager* resourceManager, const void* data,
	size_t size, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc,
	const dsTextQuality* qualityRemap, const dsVectorImageInitResources* initResources,
	float pixelSize, const dsVectorShaders* vectorIconShaders, const dsShader* textureIconShader,
	const dsMaterial* textureIconMaterial, const char* name);

size_t dsVectorResources_fullAllocSize(uint32_t maxResources)
{
	size_t tableSize = dsHashTable_tableSize(maxResources);
	return DS_ALIGNED_SIZE(sizeof(dsVectorResources)) + dsHashTable_fullAllocSize(tableSize) +
		dsPoolAllocator_bufferSize(sizeof(ResourceNode), maxResources);
}

dsVectorResources* dsVectorReosurces_create(dsAllocator* allocator, uint32_t maxResources)
{
	if (!allocator || maxResources == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = dsVectorResources_fullAllocSize(maxResources);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsVectorResources* resources = DS_ALLOCATE_OBJECT(&bufferAlloc, dsVectorResources);
	DS_ASSERT(resources);

	resources->allocator = dsAllocator_keepPointer(allocator);

	size_t tableSize = dsHashTable_tableSize(maxResources);
	resources->resourceTable = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		dsHashTable_fullAllocSize(tableSize));
	DS_ASSERT(resources->resourceTable);
	DS_VERIFY(dsHashTable_initialize(
		resources->resourceTable, tableSize, &dsHashString, &dsHashStringEqual));

	size_t poolSize = dsPoolAllocator_bufferSize(sizeof(ResourceNode), maxResources);
	void* pool = dsAllocator_alloc((dsAllocator*)&bufferAlloc, poolSize);
	DS_ASSERT(pool);
	DS_VERIFY(dsPoolAllocator_initialize(
		&resources->resourcePool, sizeof(ResourceNode), maxResources, pool, poolSize));

	return resources;
}

dsVectorResources* dsVectorResources_loadFile(dsAllocator* allocator, dsAllocator* scratchAllocator,
	dsAllocator* resourceAllocator, dsResourceManager* resourceManager, const char* filePath,
	const dsTextQuality* qualityRemap, const dsVectorImageInitResources* initResources,
	float pixelSize, const dsVectorShaders* vectorIconShaders, const dsShader* textureIconShader,
	const dsMaterial* textureIconMaterial)
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
			DS_PROFILE_FUNC_RETURN(NULL);
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
		DS_PROFILE_FUNC_RETURN(NULL);

	dsFileRelativePath fileInfo = {baseDirectory};
	dsVectorResources* resources = dsVectorResources_loadImpl(allocator, scratchAllocator,
		resourceAllocator, resourceManager, buffer, size, &fileInfo, &dsFileRelativePath_open,
		&dsFileRelativePath_close, qualityRemap, initResources, pixelSize,
		vectorIconShaders, textureIconShader, textureIconMaterial, filePath);
	DS_VERIFY(dsAllocator_free(scratchAllocator, buffer));
	DS_PROFILE_FUNC_RETURN(resources);
}

dsVectorResources* dsVectorResources_loadResource(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsAllocator* resourceAllocator,
	dsResourceManager* resourceManager, dsFileResourceType type, const char* filePath,
	const dsTextQuality* qualityRemap, const dsVectorImageInitResources* initResources,
	float pixelSize, const dsVectorShaders* vectorIconShaders, const dsShader* textureIconShader,
	const dsMaterial* textureIconMaterial)
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
			DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsResourceStream stream;
	if (!dsResourceStream_open(&stream, type, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open vector resources file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, (dsStream*)&stream, scratchAllocator);
	dsResourceStream_close(&stream);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsResourceRelativePath resourceInfo = {baseDirectory, type};
	dsVectorResources* resources = dsVectorResources_loadImpl(allocator, scratchAllocator,
		resourceAllocator, resourceManager, buffer, size, &resourceInfo,
		&dsResourceRelativePath_open, &dsResourceRelativePath_close, qualityRemap, initResources,
		pixelSize, vectorIconShaders, textureIconShader, textureIconMaterial, filePath);
	DS_VERIFY(dsAllocator_free(scratchAllocator, buffer));
	DS_PROFILE_FUNC_RETURN(resources);
}

dsVectorResources* dsVectorResources_loadArchive(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsAllocator* resourceAllocator,
	dsResourceManager* resourceManager, const dsFileArchive* archive,
	const char* filePath, const dsTextQuality* qualityRemap,
	const dsVectorImageInitResources* initResources, float pixelSize,
	const dsVectorShaders* vectorIconShaders, const dsShader* textureIconShader,
	const dsMaterial* textureIconMaterial)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !resourceManager || !archive || !filePath)
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
			DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsStream* stream = dsFileArchive_openFile(archive, filePath);
	if (!stream)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open vector resources file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, stream, scratchAllocator);
	dsStream_close(stream);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsArchiveRelativePath archiveInfo = {baseDirectory, archive};
	dsVectorResources* resources = dsVectorResources_loadImpl(allocator, scratchAllocator,
		resourceAllocator, resourceManager, buffer, size, &archiveInfo, &dsArchiveRelativePath_open,
		&dsArchiveRelativePath_close, qualityRemap, initResources, pixelSize, vectorIconShaders,
		textureIconShader, textureIconMaterial, filePath);
	DS_VERIFY(dsAllocator_free(scratchAllocator, buffer));
	DS_PROFILE_FUNC_RETURN(resources);
}

dsVectorResources* dsVectorResources_loadStream(dsAllocator* allocator,
	dsAllocator* scratchAllocator, dsAllocator* resourceAllocator,
	dsResourceManager* resourceManager, dsStream* stream, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc,
	const dsTextQuality* qualityRemap, const dsVectorImageInitResources* initResources,
	float pixelSize, const dsVectorShaders* vectorIconShaders, const dsShader* textureIconShader,
	const dsMaterial* textureIconMaterial)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !resourceManager || !stream || !openRelativePathStreamFunc ||
		!closeRelativePathStreamFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!scratchAllocator)
		scratchAllocator = allocator;

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, stream, scratchAllocator);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsVectorResources* resources = dsVectorResources_loadImpl(allocator, scratchAllocator,
		resourceAllocator, resourceManager, buffer, size, relativePathUserData,
		openRelativePathStreamFunc, closeRelativePathStreamFunc, qualityRemap, initResources,
		pixelSize, vectorIconShaders, textureIconShader, textureIconMaterial, NULL);
	DS_VERIFY(dsAllocator_free(scratchAllocator, buffer));
	DS_PROFILE_FUNC_RETURN(resources);
}

dsVectorResources* dsVectorResources_loadData(dsAllocator* allocator, dsAllocator* scratchAllocator,
	dsAllocator* resourceAllocator, dsResourceManager* resourceManager, const void* data,
	size_t size, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc,
	const dsTextQuality* qualityRemap, const dsVectorImageInitResources* initResources,
	float pixelSize, const dsVectorShaders* vectorIconShaders, const dsShader* textureIconShader,
	const dsMaterial* textureIconMaterial)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !resourceManager || !data || size == 0 || !openRelativePathStreamFunc ||
		!closeRelativePathStreamFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!scratchAllocator)
		scratchAllocator = allocator;

	dsVectorResources* resources = dsVectorResources_loadImpl(allocator, scratchAllocator,
		resourceAllocator, resourceManager, data, size, relativePathUserData,
		openRelativePathStreamFunc, closeRelativePathStreamFunc, qualityRemap, initResources,
		pixelSize, vectorIconShaders, textureIconShader, textureIconMaterial, NULL);
	DS_PROFILE_FUNC_RETURN(resources);
}

uint32_t dsVectorResources_getRemainingResources(const dsVectorResources* resources)
{
	if (!resources)
		return 0;

	return (uint32_t)resources->resourcePool.freeCount;
}

bool dsVectorResources_addResource(dsVectorResources* resources, const char* name,
	dsVectorResourceType type, void* resource, bool own)
{
	if (type < dsVectorResourceType_Texture || type > dsVectorResourceType_Font || !resource)
	{
		errno = EINVAL;
		return false;
	}

	if (!resources || !name)
	{
		errno = EINVAL;
		if (own)
			destroyResource(type, resource);
		return false;
	}

	size_t nameLength = strlen(name) + 1;
	if (nameLength > DS_MAX_VECTOR_RESOURCE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Resource name '%s' exceeds maximum size of %u.",
			name, DS_MAX_VECTOR_RESOURCE_NAME_LENGTH);
		if (own)
			destroyResource(type, resource);
		return false;
	}

	dsHashTableNode* foundNode = dsHashTable_find(resources->resourceTable, name);
	if (foundNode)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Resource '%s' has already been added.", name);
		if (own)
			destroyResource(type, resource);
		return false;
	}

	if (resources->resourcePool.freeCount == 0)
	{
		errno = ESIZE;
		DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG, "Maximum number of resources has been exceeded.");
		if (own)
			destroyResource(type, resource);
		return false;
	}

	ResourceNode* node = DS_ALLOCATE_OBJECT(&resources->resourcePool, ResourceNode);
	DS_ASSERT(node);
	memcpy(node->name, name, nameLength);
	node->resource = resource;
	node->type = type;
	node->owned = own;
	DS_VERIFY(dsHashTable_insert(
		resources->resourceTable, node->name, (dsHashTableNode*)node, NULL));
	return true;
}

bool dsVectorResources_removeResource(
	dsVectorResources* resources, const char* name, bool relinquish)
{
	if (!resources || !name)
		return false;

	ResourceNode* node = (ResourceNode*)dsHashTable_remove(resources->resourceTable, name);
	if (!node)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Resource '%s' not found.", name);
		return false;
	}

	DS_ASSERT(node->resource);
	if (node->owned && !relinquish)
	{
		if (!destroyResource(node->type, node->resource))
		{
			// Put back into the hash table.
			DS_VERIFY(dsHashTable_insert(
				resources->resourceTable, node->name, (dsHashTableNode*)node, NULL));
			return false;
		}
	}

	DS_VERIFY(dsAllocator_free((dsAllocator*)&resources->resourcePool, node));
	return true;
}

bool dsVectorResources_findResource(dsVectorResourceType* outType, void** outResource,
	const dsVectorResources* resources, const char* name)
{
	if (!outType || !outResource || !resources || !name)
		return false;

	ResourceNode* node = (ResourceNode*)dsHashTable_find(resources->resourceTable, name);
	if (!node)
		return false;

	*outType = node->type;
	*outResource = node->resource;
	return true;
}

bool dsVectorResources_preloadASCII(dsVectorResources* resources, dsCommandBuffer* commandBuffer)
{
	if (!resources || !commandBuffer)
	{
		errno = EINVAL;
		return false;
	}

	for (dsListNode* node = resources->resourceTable->list.head; node; node = node->next)
	{
		ResourceNode* resourceNode = (ResourceNode*)node;
		if (resourceNode->type == dsVectorResourceType_Font &&
			!dsFont_preloadASCII((dsFont*)resourceNode->resource, commandBuffer))
		{
			return false;
		}
	}

	return true;
}

bool dsVectorResources_destroy(dsVectorResources* resources)
{
	if (!resources)
		return true;

	// Destroy all resources in reverse order.
	for (dsListNode* node = resources->resourceTable->list.tail; node; node = node->previous)
	{
		ResourceNode* resourceNode = (ResourceNode*)node;
		if (!destroyResource(resourceNode->type, resourceNode->resource))
			return false;
	}

	if (resources->allocator)
		DS_VERIFY(dsAllocator_free(resources->allocator, resources));
	return true;
}

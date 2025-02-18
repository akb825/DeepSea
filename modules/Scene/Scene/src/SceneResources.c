/*
 * Copyright 2019-2025 Aaron Barany
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
#include <DeepSea/Core/Streams/FileArchive.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/Texture.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/CustomSceneResource.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <string.h>

typedef struct dsFileInfo
{
	const char* basePath;
	dsFileStream stream;
} dsFileInfo;

typedef struct dsResourceInfo
{
	const char* basePath;
	dsFileResourceType type;
	dsResourceStream stream;
} dsResourceInfo;

typedef struct dsArchiveInfo
{
	const char* basePath;
	const dsFileArchive* archive;
} dsArchiveInfo;

typedef struct ResourceNode
{
	dsHashTableNode node;
	void* resource;
	char name[DS_MAX_SCENE_NAME_LENGTH];
	dsSceneResourceType type;
	bool owned;
} ResourceNode;

struct dsSceneResources
{
	dsAllocator* allocator;
	dsHashTable* resources;
	dsPoolAllocator nodePool;
	uint32_t refCount;
};

static dsStream* openFileStream(void* userData, const char* path)
{
	dsFileInfo* fileInfo = (dsFileInfo*)userData;
	char finalPath[DS_PATH_MAX];
	if (!dsPath_combine(finalPath, sizeof(finalPath), fileInfo->basePath, path))
	{
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Path '%s%c%s' is too long.", fileInfo->basePath,
			DS_PATH_SEPARATOR, path);
		return NULL;
	}

	if (!dsFileStream_openPath(&fileInfo->stream, finalPath, "rb"))
	{
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Couldn't open file '%s'.", finalPath);
		return NULL;
	}

	return (dsStream*)&fileInfo->stream;
}

static void closeFileStream(void* userData, dsStream* stream)
{
	DS_UNUSED(userData);
	dsFileStream_close((dsFileStream*)stream);
}

static dsStream* openResourceStream(void* userData, const char* path)
{
	dsResourceInfo* resourceInfo = (dsResourceInfo*)userData;
	char finalPath[DS_PATH_MAX];
	if (!dsPath_combine(finalPath, sizeof(finalPath), resourceInfo->basePath, path))
	{
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Path '%s%c%s' is too long.", resourceInfo->basePath,
			DS_PATH_SEPARATOR, path);
		return NULL;
	}

	if (!dsResourceStream_open(&resourceInfo->stream, resourceInfo->type, finalPath, "rb"))
	{
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Couldn't open file '%s'.", finalPath);
		return NULL;
	}

	return (dsStream*)&resourceInfo->stream;
}

static void closeResourceStream(void* userData, dsStream* stream)
{
	DS_UNUSED(userData);
	dsResourceStream_close((dsResourceStream*)stream);
}

static dsStream* openArchiveStream(void* userData, const char* path)
{
	dsArchiveInfo* archiveInfo = (dsArchiveInfo*)userData;
	char finalPath[DS_PATH_MAX];
	if (!dsPath_combine(finalPath, sizeof(finalPath), archiveInfo->basePath, path))
	{
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Path '%s%c%s' is too long.", archiveInfo->basePath,
			DS_PATH_SEPARATOR, path);
		return NULL;
	}

	dsStream* stream = dsFileArchive_openFile(archiveInfo->archive, finalPath);
	if (!stream)
	{
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Couldn't open file '%s'.", finalPath);
		return NULL;
	}

	return stream;
}

static void closeArchiveStream(void* userData, dsStream* stream)
{
	DS_UNUSED(userData);
	dsStream_close(stream);
}

static dsHashTable* createHashTable(dsBufferAllocator* allocator, uint32_t maxItems)
{
	size_t tableSize = dsHashTable_tableSize(maxItems);
	dsHashTable* hashTable = (dsHashTable*)dsAllocator_alloc((dsAllocator*)allocator,
		dsHashTable_fullAllocSize(tableSize));
	DS_ASSERT(hashTable);
	DS_VERIFY(dsHashTable_initialize(hashTable, tableSize, dsHashString, dsHashStringEqual));
	return hashTable;
}

static bool destroyResource(dsSceneResourceType type, void* resource)
{
	switch (type)
	{
		case dsSceneResourceType_Buffer:
			return dsGfxBuffer_destroy((dsGfxBuffer*)resource);
		case dsSceneResourceType_Texture:
			return dsTexture_destroy((dsTexture*)resource);
		case dsSceneResourceType_ShaderVariableGroupDesc:
			return dsShaderVariableGroupDesc_destroy((dsShaderVariableGroupDesc*)resource);
		case dsSceneResourceType_ShaderVariableGroup:
			return dsShaderVariableGroup_destroy((dsShaderVariableGroup*)resource);
		case dsSceneResourceType_MaterialDesc:
			return dsMaterialDesc_destroy((dsMaterialDesc*)resource);
		case dsSceneResourceType_Material:
			dsMaterial_destroy((dsMaterial*)resource);
			return true;
		case dsSceneResourceType_ShaderModule:
			return dsShaderModule_destroy((dsShaderModule*)resource);
		case dsSceneResourceType_Shader:
			return dsShader_destroy((dsShader*)resource);
		case dsSceneResourceType_DrawGeometry:
			return dsDrawGeometry_destroy((dsDrawGeometry*)resource);
		case dsSceneResourceType_Custom:
			return dsCustomSceneResource_destroy((dsCustomSceneResource*)resource);
		default:
			DS_ASSERT(false);
			return false;
	}
}

dsSceneResources* dsSceneResources_loadImpl(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData,
	const void* data, size_t dataSize, const char* fileName, void* relativePathUserData,
	dsOpenSceneResourcesRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseSceneResourcesRelativePathStreamFunction closeRelativePathStreamFunc);

size_t dsSceneResources_sizeof(void)
{
	return sizeof(dsSceneResources);
}

size_t dsSceneResources_fullAllocSize(uint32_t maxResources)
{
	return DS_ALIGNED_SIZE(sizeof(dsSceneResources)) +
		dsHashTable_fullAllocSize(dsHashTable_tableSize(maxResources)) +
		dsPoolAllocator_bufferSize(sizeof(ResourceNode), maxResources);
}

dsSceneResources* dsSceneResources_create(dsAllocator* allocator, uint32_t maxResources)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = dsSceneResources_fullAllocSize(maxResources);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsSceneResources* sceneResources = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneResources);
	DS_ASSERT(sceneResources);

	sceneResources->allocator = dsAllocator_keepPointer(allocator);
	sceneResources->resources = createHashTable(&bufferAlloc, maxResources);

	size_t poolSize = dsPoolAllocator_bufferSize(sizeof(ResourceNode), maxResources);
	void* poolBuffer = dsAllocator_alloc((dsAllocator*)&bufferAlloc, poolSize);
	DS_ASSERT(poolBuffer);
	DS_VERIFY(dsPoolAllocator_initialize(&sceneResources->nodePool, sizeof(ResourceNode),
		maxResources, poolBuffer, poolSize));

	sceneResources->refCount = 1;
	return sceneResources;
}

dsSceneResources* dsSceneResources_loadFile(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData,
	const char* filePath)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !loadContext || !scratchData || !filePath)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsFileStream stream;
	if (!dsFileStream_openPath(&stream, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open scene resources file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsSceneLoadScratchData_readUntilEnd(&size, scratchData, (dsStream*)&stream);
	dsFileStream_close(&stream);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	char baseDirectory[DS_PATH_MAX];
	if (!dsPath_getDirectoryName(baseDirectory, sizeof(baseDirectory), filePath))
	{
		if (errno == EINVAL)
			baseDirectory[0] = 0;
		else
			DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsFileInfo fileInfo = {baseDirectory};
	dsSceneResources* resources = dsSceneResources_loadImpl(allocator, resourceAllocator,
		loadContext, scratchData, buffer, size, filePath, &fileInfo, &openFileStream,
		&closeFileStream);
	DS_VERIFY(dsSceneLoadScratchData_freeReadBuffer(scratchData, buffer));
	DS_PROFILE_FUNC_RETURN(resources);
}

dsSceneResources* dsSceneResources_loadResource(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsFileResourceType type, const char* filePath)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !loadContext || !scratchData || !filePath)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsResourceStream stream;
	if (!dsResourceStream_open(&stream, type, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open scene resources file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsSceneLoadScratchData_readUntilEnd(&size, scratchData, (dsStream*)&stream);
	dsResourceStream_close(&stream);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	char baseDirectory[DS_PATH_MAX];
	if (!dsPath_getDirectoryName(baseDirectory, sizeof(baseDirectory), filePath))
	{
		if (errno == EINVAL)
			baseDirectory[0] = 0;
		else
			DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsResourceInfo resourceInfo = {baseDirectory, type};
	dsSceneResources* resources = dsSceneResources_loadImpl(allocator, resourceAllocator,
		loadContext, scratchData, buffer, size, filePath, &resourceInfo, &openResourceStream,
		&closeResourceStream);
	DS_VERIFY(dsSceneLoadScratchData_freeReadBuffer(scratchData, buffer));
	DS_PROFILE_FUNC_RETURN(resources);
}

dsSceneResources* dsSceneResources_loadArchive(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, const dsFileArchive* archive, const char* filePath)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !loadContext || !scratchData || !archive || !filePath)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsStream* stream = dsFileArchive_openFile(archive, filePath);
	if (!stream)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open scene resources file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsSceneLoadScratchData_readUntilEnd(&size, scratchData, stream);
	dsStream_close(stream);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	char baseDirectory[DS_PATH_MAX];
	if (!dsPath_getDirectoryName(baseDirectory, sizeof(baseDirectory), filePath))
	{
		if (errno == EINVAL)
			baseDirectory[0] = 0;
		else
			DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsArchiveInfo archiveInfo = {baseDirectory, archive};
	dsSceneResources* resources = dsSceneResources_loadImpl(allocator, resourceAllocator,
		loadContext, scratchData, buffer, size, filePath, &archiveInfo, &openArchiveStream,
		&closeArchiveStream);
	DS_VERIFY(dsSceneLoadScratchData_freeReadBuffer(scratchData, buffer));
	DS_PROFILE_FUNC_RETURN(resources);
}

dsSceneResources* dsSceneResources_loadStream(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsStream* stream, 	void* relativePathUserData,
	dsOpenSceneResourcesRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseSceneResourcesRelativePathStreamFunction closeRelativePathStreamFunc)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !loadContext || !scratchData || !stream || !openRelativePathStreamFunc ||
		!closeRelativePathStreamFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsSceneLoadScratchData_readUntilEnd(&size, scratchData, stream);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsSceneResources* resources = dsSceneResources_loadImpl(allocator, resourceAllocator,
		loadContext, scratchData, buffer, size, NULL, relativePathUserData,
		openRelativePathStreamFunc, closeRelativePathStreamFunc);
	DS_VERIFY(dsSceneLoadScratchData_freeReadBuffer(scratchData, buffer));
	DS_PROFILE_FUNC_RETURN(resources);
}

dsSceneResources* dsSceneResources_loadData(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData, const void* data,
	size_t size, void* relativePathUserData,
	dsOpenSceneResourcesRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseSceneResourcesRelativePathStreamFunction closeRelativePathStreamFunc)
{
	DS_PROFILE_FUNC_START();

	if (!allocator || !loadContext || !scratchData || !data || size == 0 ||
		!openRelativePathStreamFunc || !closeRelativePathStreamFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsSceneResources* resources = dsSceneResources_loadImpl(allocator, resourceAllocator,
		loadContext, scratchData, data, size, NULL, relativePathUserData,
		openRelativePathStreamFunc, closeRelativePathStreamFunc);
	DS_PROFILE_FUNC_RETURN(resources);
}

uint32_t dsSceneResources_getRemainingResources(const dsSceneResources* resources)
{
	if (!resources)
		return 0;

	return (uint32_t)resources->nodePool.freeCount;
}

bool dsSceneResources_addResource(dsSceneResources* resources, const char* name,
	dsSceneResourceType type, void* resource, bool own)
{
	if (!resources || !name || type < dsSceneResourceType_Buffer ||
		type > dsSceneResourceType_Custom || !resource)
	{
		errno = EINVAL;
		return false;
	}

	size_t nameLength = strlen(name) + 1;
	if (nameLength > DS_MAX_SCENE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Resource name '%s' exceeds maximum size of %u.",
			name, DS_MAX_SCENE_NAME_LENGTH);
		return false;
	}

	dsHashTableNode* foundNode = dsHashTable_find(resources->resources, name);
	if (foundNode)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Resource '%s' has already been added.", name);
		return false;
	}

	if (resources->nodePool.freeCount == 0)
	{
		errno = ESIZE;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Maximum number of resources has been exceeded.");
		return false;
	}

	ResourceNode* node = DS_ALLOCATE_OBJECT(&resources->nodePool, ResourceNode);
	DS_ASSERT(node);
	memcpy(node->name, name, nameLength);
	node->resource = resource;
	node->type = type;
	node->owned = own;
	DS_VERIFY(dsHashTable_insert(resources->resources, node->name, (dsHashTableNode*)node, NULL));
	if (type == dsSceneResourceType_SceneNode)
		dsSceneNode_addRef((dsSceneNode*)resource);
	return true;
}

bool dsSceneResource_removeResource(dsSceneResources* resources, const char* name,
	bool relinquish)
{
	if (!resources || !resources->resources || !name)
		return false;

	ResourceNode* node = (ResourceNode*)dsHashTable_remove(resources->resources, name);
	if (!node)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Texture '%s' not found.", name);
		return false;
	}

	DS_ASSERT(node->resource);
	if (node->type == dsSceneResourceType_SceneNode)
		dsSceneNode_freeRef((dsSceneNode*)node->resource);
	else if (node->owned && !relinquish)
	{
		if (!destroyResource(node->type, node->resource))
		{
			// Put back into the hash table.
			DS_VERIFY(dsHashTable_insert(resources->resources, node->name, (dsHashTableNode*)node,
				NULL));
			return false;
		}
	}

	DS_VERIFY(dsAllocator_free((dsAllocator*)&resources->nodePool, node));
	return true;
}

bool dsSceneResources_findResource(dsSceneResourceType* outType, void** outResource,
	const dsSceneResources* resources, const char* name)
{
	if (!resources || !name)
		return false;

	ResourceNode* node = (ResourceNode*)dsHashTable_find(resources->resources, name);
	if (!node)
		return false;

	if (outType)
		*outType = node->type;
	if (outResource)
		*outResource = node->resource;
	return true;
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

	if (DS_ATOMIC_FETCH_ADD32(&resources->refCount, -1) != 1)
		return;

	// Destroy all resources in reverse order.
	for (dsListNode* node = resources->resources->list.tail; node; node = node->previous)
	{
		ResourceNode* resourceNode = (ResourceNode*)node;
		if (resourceNode->type == dsSceneResourceType_SceneNode)
			dsSceneNode_freeRef((dsSceneNode*)resourceNode->resource);
		else if (resourceNode->owned)
			destroyResource(resourceNode->type, resourceNode->resource);
	}

	dsPoolAllocator_shutdown(&resources->nodePool);
	if (resources->allocator)
		DS_VERIFY(dsAllocator_free(resources->allocator, resources));
}

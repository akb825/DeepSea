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

#include <DeepSea/Render/Resources/SharedMaterialValues.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/PoolAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Types.h>

typedef enum Type
{
	Type_Texture,
	Type_TextureBuffer,
	Type_ShaderVariableGroup,
	Type_Buffer
} Type;

typedef struct Entry
{
	dsHashTableNode node;
	uint32_t key;
	Type type;
	size_t offset;
	size_t size;
	dsGfxFormat format;
	void* value;
} Entry;

struct dsSharedMaterialValues
{
	dsAllocator* allocator;
	dsPoolAllocator entryPool;
	dsHashTable* hashTable;
};

static uint32_t identityHash(const void* key)
{
	return *(const uint32_t*)key;
}

static unsigned int getTableSize(unsigned int maxValues)
{
	const float loadFactor = 0.73f;
	return (unsigned int)((float)maxValues/loadFactor);
}

static void* getValue(dsGfxFormat* outFormat, size_t* outOffset, size_t* outSize,
	const dsSharedMaterialValues* values, uint32_t nameId, Type type)
{
	Entry* entry = (Entry*)dsHashTable_find(values->hashTable, &nameId);
	if (!entry || entry->type != type)
		return NULL;

	if (outFormat)
		*outFormat = entry->format;
	if (outOffset)
		*outOffset = entry->offset;
	if (outSize)
		*outSize = entry->size;
	return entry->value;
}

static bool setValue(dsSharedMaterialValues* values, uint32_t nameId, Type type, void* value,
	dsGfxFormat format, size_t offset, size_t size)
{
	Entry* entry = (Entry*)dsHashTable_find(values->hashTable, &nameId);
	if (entry)
	{
		if (entry->type != type)
		{
			errno = EINVAL;
			return false;
		}

		entry->value = value;
		entry->format = format;
		entry->offset = offset;
		entry->size = size;
		return true;
	}

	entry = DS_ALLOCATE_OBJECT((dsAllocator*)&values->entryPool, Entry);
	if (!entry)
		return false;

	entry->key = nameId;
	entry->type = type;
	entry->value = value;
	entry->format = format;
	entry->offset = offset;
	entry->size = size;
	DS_VERIFY(dsHashTable_insert(values->hashTable, &entry->key, &entry->node, NULL));
	return true;
}

static bool canUseTextureBuffer(dsGfxBuffer* buffer, dsGfxFormat format, size_t offset,
	size_t count)
{
	if (!buffer)
		return true;

	dsResourceManager* resourceManager = buffer->resourceManager;
	if (!dsGfxFormat_textureBufferSupported(resourceManager, format))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Format not supported for texture buffers.");
		return false;
	}

	if (!(buffer->usage & (dsGfxBufferUsage_Texture | dsGfxBufferUsage_Image)))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Buffer doesn't support being used as a texture.");
		return false;
	}

	unsigned int formatSize = dsGfxFormat_size(format);
	if (!DS_IS_BUFFER_RANGE_VALID(offset, count*formatSize, buffer->size))
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to bind outside of buffer range.");
		return false;
	}

	if (!resourceManager->hasTextureBufferSubrange && (offset != 0 ||
		count*formatSize != buffer->size))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Current target doesn't support using a subrange of a texture buffer.");
		return false;
	}

	if (resourceManager->minTextureBufferAlignment > 0 &&
		(offset % resourceManager->minTextureBufferAlignment) != 0)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Texture buffer offset doesn't match alignment requirements.");
		return false;
	}

	if (count > resourceManager->maxTextureBufferElements)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Texture buffer elements exceeds the maximum for the current target.");
		return false;
	}

	return true;
}

static bool canUseBuffer(dsGfxBuffer* buffer, size_t offset, size_t size)
{
	if (!buffer)
		return true;

	if (!(buffer->usage & (dsGfxBufferUsage_UniformBlock | dsGfxBufferUsage_UniformBuffer)))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Buffer doesn't support uniform blocks or buffers.");
		return false;
	}

	if (!DS_IS_BUFFER_RANGE_VALID(offset, size, buffer->size))
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to bind outside of buffer range.");
		return false;
	}

	dsResourceManager* resourceManager = buffer->resourceManager;
	uint32_t alignment = 0;
	if (buffer->usage & dsGfxBufferUsage_UniformBlock)
		alignment = resourceManager->minUniformBlockAlignment;
	if (buffer->usage & dsGfxBufferUsage_UniformBuffer)
		alignment = dsMax(alignment, resourceManager->minUniformBufferAlignment);
	if (alignment > 0 && (offset % alignment) != 0)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Buffer offset doesn't match alignment requirements.");
		return false;
	}

	if ((buffer->usage & dsGfxBufferUsage_UniformBlock) &&
		size > buffer->resourceManager->maxUniformBlockSize)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG,
			"Buffer size exceeds the maximum uniform block size for the current target.");
		return false;
	}

	return true;
}

size_t dsSharedMaterialValues_sizeof(void)
{
	return sizeof(dsSharedMaterialValues);
}

size_t dsSharedMaterialValues_fullAllocSize(unsigned int maxValues)
{
	return DS_ALIGNED_SIZE(sizeof(dsSharedMaterialValues)) +
		DS_ALIGNED_SIZE(dsPoolAllocator_bufferSize(sizeof(Entry), maxValues)) +
		dsHashTable_fullAllocSize(getTableSize(maxValues));
}

dsSharedMaterialValues* dsSharedMaterialValues_create(dsAllocator* allocator,
	unsigned int maxValues)
{
	if (!allocator || !maxValues)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t bufferSize = dsSharedMaterialValues_fullAllocSize(maxValues);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAllocator;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAllocator, buffer, bufferSize));

	dsSharedMaterialValues* materialValues = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAllocator,
		dsSharedMaterialValues);
	DS_ASSERT(materialValues);
	materialValues->allocator = dsAllocator_keepPointer(allocator);

	size_t poolSize = dsPoolAllocator_bufferSize(sizeof(Entry), maxValues);
	void* poolBuffer = dsAllocator_alloc((dsAllocator*)&bufferAllocator, poolSize);
	DS_ASSERT(poolBuffer);
	DS_VERIFY(dsPoolAllocator_initialize(&materialValues->entryPool, sizeof(Entry), maxValues,
		poolBuffer, poolSize));

	unsigned int tableSize = getTableSize(maxValues);
	materialValues->hashTable = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAllocator,
		dsHashTable_fullAllocSize(tableSize));
	DS_VERIFY(dsHashTable_initialize(materialValues->hashTable, tableSize, &identityHash,
		&dsHash32Equal));
	return materialValues;
}

unsigned int dsSharedMaterialValues_getValueCount(const dsSharedMaterialValues* values)
{
	if (!values)
		return 0;

	return (unsigned int)values->hashTable->list.length;
}

unsigned int dsSharedMaterialValues_getMaxValueCount(const dsSharedMaterialValues* values)
{
	if (!values)
		return 0;

	return (unsigned int)values->entryPool.chunkCount;
}

dsTexture* dsSharedMaterialValues_getTextureName(const dsSharedMaterialValues* values,
	const char* name)
{
	if (!values || !name)
		return NULL;

	return (dsTexture*)getValue(NULL, NULL, NULL, values, dsHashString(name), Type_Texture);
}

dsTexture* dsSharedMaterialValues_getTextureId(const dsSharedMaterialValues* values,
	uint32_t nameId)
{
	if (!values)
		return NULL;

	return (dsTexture*)getValue(NULL, NULL, NULL, values, nameId, Type_Texture);
}

bool dsSharedMaterialValues_setTextureName(dsSharedMaterialValues* values, const char* name,
	dsTexture* texture)
{
	if (!values || !name)
	{
		errno = EINVAL;
		return false;
	}

	return setValue(values, dsHashString(name), Type_Texture, texture, dsGfxFormat_Unknown, 0, 0);
}

bool dsSharedMaterialValues_setTextureId(dsSharedMaterialValues* values, uint32_t nameId,
	dsTexture* texture)
{
	if (!values)
	{
		errno = EINVAL;
		return false;
	}

	return setValue(values, nameId, Type_Texture, texture, dsGfxFormat_Unknown, 0, 0);
}

dsGfxBuffer* dsSharedMaterialValues_getTextureBufferName(dsGfxFormat* outFormat,
	size_t* outOffset, size_t* outCount, const dsSharedMaterialValues* values, const char* name)
{
	if (!values || !name)
		return NULL;

	return (dsGfxBuffer*)getValue(outFormat, outOffset, outCount, values, dsHashString(name),
		Type_TextureBuffer);
}

dsGfxBuffer* dsSharedMaterialValues_getTextureBufferId(dsGfxFormat* outFormat, size_t* outOffset,
	size_t* outCount, const dsSharedMaterialValues* values, uint32_t nameId)
{
	if (!values)
		return NULL;

	return (dsGfxBuffer*)getValue(outFormat, outOffset, outCount, values, nameId,
		Type_TextureBuffer);
}

bool dsSharedMaterialValues_setTextureBufferName(dsSharedMaterialValues* values,
	const char* name, dsGfxBuffer* buffer, dsGfxFormat format, size_t offset, size_t count)
{
	if (!values || !name)
	{
		errno = EINVAL;
		return false;
	}

	if (!canUseTextureBuffer(buffer, format, offset, count))
		return false;

	return setValue(values, dsHashString(name), Type_TextureBuffer, buffer, format, offset, count);
}

bool dsSharedMaterialValues_setTextureBufferId(dsSharedMaterialValues* values, uint32_t nameId,
	dsGfxBuffer* buffer, dsGfxFormat format, size_t offset, size_t count)
{
	if (!values)
	{
		errno = EINVAL;
		return false;
	}

	if (!canUseTextureBuffer(buffer, format, offset, count))
		return false;

	return setValue(values, nameId, Type_TextureBuffer, buffer, format, offset, count);
}

dsShaderVariableGroup* dsSharedMaterialValues_getVariableGroupName(
	const dsSharedMaterialValues* values, const char* name)
{
	if (!values || !name)
		return NULL;

	return (dsShaderVariableGroup*)getValue(NULL, NULL, NULL, values, dsHashString(name),
		Type_ShaderVariableGroup);
}

dsShaderVariableGroup* dsSharedMaterialValues_getVariableGroupId(
	const dsSharedMaterialValues* values, uint32_t nameId)
{
	if (!values)
		return NULL;

	return (dsShaderVariableGroup*)getValue(NULL, NULL, NULL, values, nameId,
		Type_ShaderVariableGroup);
}

bool dsSharedMaterialValues_setVariableGroupName(dsSharedMaterialValues* values,
	const char* name, dsShaderVariableGroup* group)
{
	if (!values || !name)
		return false;

	return setValue(values, dsHashString(name), Type_ShaderVariableGroup, group,
		dsGfxFormat_Unknown, 0, 0);
}

bool dsSharedMaterialValues_setVariableGroupId(dsSharedMaterialValues* values,
	uint32_t nameId, dsShaderVariableGroup* group)
{
	if (!values)
		return false;

	return setValue(values, nameId, Type_ShaderVariableGroup, group, dsGfxFormat_Unknown, 0, 0);
}

dsGfxBuffer* dsSharedMaterialValues_getBufferName(size_t* outOffset, size_t* outSize,
	const dsSharedMaterialValues* values, const char* name)
{
	if (!values || !name)
		return NULL;

	return (dsGfxBuffer*)getValue(NULL, outOffset, outSize, values, dsHashString(name),
		Type_Buffer);
}

dsGfxBuffer* dsSharedMaterialValues_getBufferId(size_t* outOffset, size_t* outSize,
	const dsSharedMaterialValues* values, uint32_t nameId)
{
	if (!values)
		return NULL;

	return (dsGfxBuffer*)getValue(NULL, outOffset, outSize, values, nameId, Type_Buffer);
}

bool dsSharedMaterialValues_setBufferName(dsSharedMaterialValues* values, const char* name,
	dsGfxBuffer* buffer, size_t offset, size_t size)
{
	if (!values || !name)
	{
		errno = EINVAL;
		return false;
	}

	if (!canUseBuffer(buffer, offset, size))
		return false;

	return setValue(values, dsHashString(name), Type_Buffer, buffer, dsGfxFormat_Unknown, offset,
		size);
}

bool dsSharedMaterialValues_setBufferId(dsSharedMaterialValues* values, uint32_t nameId,
	dsGfxBuffer* buffer, size_t offset, size_t size)
{
	if (!values)
	{
		errno = EINVAL;
		return false;
	}

	if (!canUseBuffer(buffer, offset, size))
		return false;

	return setValue(values, nameId, Type_Buffer, buffer, dsGfxFormat_Unknown, offset, size);
}

bool dsSharedMaterialValues_removeValueName(dsSharedMaterialValues* values, const char* name)
{
	if (!values || !name)
		return false;

	return dsSharedMaterialValues_removeValueId(values, dsHashString(name));
}

bool dsSharedMaterialValues_removeValueId(dsSharedMaterialValues* values, uint32_t nameId)
{
	if (!values)
		return false;

	Entry* entry = (Entry*)dsHashTable_remove(values->hashTable, &nameId);
	if (!entry)
		return false;

	DS_VERIFY(dsAllocator_free((dsAllocator*)&values->entryPool, entry));
	return true;
}

bool dsSharedMaterialValues_clear(dsSharedMaterialValues* values)
{
	if (!values)
	{
		errno = EINVAL;
		return false;
	}

	DS_VERIFY(dsHashTable_clear(values->hashTable));
	DS_VERIFY(dsPoolAllocator_reset(&values->entryPool));
	return true;
}

void dsSharedMaterialValues_destroy(dsSharedMaterialValues* values)
{
	if (!values)
		return;

	dsPoolAllocator_shutdown(&values->entryPool);
	if (values->allocator)
		dsAllocator_free(values->allocator, values);
}

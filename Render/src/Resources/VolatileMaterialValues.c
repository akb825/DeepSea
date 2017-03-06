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

#include <DeepSea/Render/Resources/VolatileMaterialValues.h>
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/PoolAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

typedef enum Type
{
	Type_Texture,
	Type_ShaderVariableGroup,
	Type_Buffer
} Type;

typedef struct Entry
{
	dsHashTableNode node;
	uint32_t key;
	Type type;
	void* value;
} Entry;

struct dsVolatileMaterialValues
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
	const float loadFactor = 0.75f;
	return (unsigned int)((float)maxValues/loadFactor);
}

static void* getValue(const dsVolatileMaterialValues* values, uint32_t nameId, Type type)
{
	Entry* entry = (Entry*)dsHashTable_find(values->hashTable, &nameId);
	if (!entry || entry->type != type)
		return NULL;

	return entry->value;
}

static bool setValue(dsVolatileMaterialValues* values, uint32_t nameId, Type type, void* value)
{
	Entry* entry = (Entry*)dsHashTable_find(values->hashTable, &nameId);
	if (entry)
	{
		if (entry->type != type)
		{
			errno = EPERM;
			return false;
		}

		entry->value = value;
		return true;
	}

	entry = dsAllocator_alloc((dsAllocator*)&values->entryPool, sizeof(Entry));
	if (!entry)
		return false;

	entry->key = nameId;
	entry->type = type;
	entry->value = value;
	DS_VERIFY(dsHashTable_insert(values->hashTable, &entry->key, &entry->node, NULL));
	return true;
}

size_t dsVolatileMaterialValues_sizeof(void)
{
	return sizeof(dsVolatileMaterialValues);
}

size_t dsVolatileMaterialValues_fullAllocSize(unsigned int maxValues)
{
	return DS_ALIGNED_SIZE(sizeof(dsVolatileMaterialValues)) +
		DS_ALIGNED_SIZE(dsPoolAllocator_bufferSize(sizeof(Entry), maxValues)) +
		dsHashTable_fullAllocSize(getTableSize(maxValues));
}

dsVolatileMaterialValues* dsVolatileMaterialValues_create(dsAllocator* allocator,
	unsigned int maxValues)
{
	if (!allocator || !maxValues)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t bufferSize = dsVolatileMaterialValues_fullAllocSize(maxValues);
	void* buffer = dsAllocator_alloc(allocator, bufferSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAllocator;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAllocator, buffer, bufferSize));

	dsVolatileMaterialValues* materialValues = dsAllocator_alloc((dsAllocator*)&bufferAllocator,
		sizeof(dsVolatileMaterialValues));
	DS_ASSERT(materialValues);
	if (allocator->freeFunc)
		materialValues->allocator = allocator;
	else
		materialValues->allocator = NULL;

	size_t poolSize = dsPoolAllocator_bufferSize(sizeof(Entry), maxValues);
	void* poolBuffer = dsAllocator_alloc((dsAllocator*)&bufferAllocator, poolSize);
	DS_ASSERT(poolBuffer);
	DS_VERIFY(dsPoolAllocator_initialize(&materialValues->entryPool, sizeof(Entry), maxValues,
		poolBuffer, poolSize));

	unsigned int tableSize = getTableSize(maxValues);
	materialValues->hashTable = dsAllocator_alloc((dsAllocator*)&bufferAllocator,
		dsHashTable_fullAllocSize(tableSize));
	DS_VERIFY(dsHashTable_initialize(materialValues->hashTable, tableSize, &identityHash,
		&dsHash32Equal));
	return materialValues;
}

unsigned int dsVolatileMaterialValues_getValueCount(const dsVolatileMaterialValues* values)
{
	if (!values)
		return 0;

	return (unsigned int)values->hashTable->list.length;
}

unsigned int dsVolatileMaterialValues_getMaxValueCount(const dsVolatileMaterialValues* values)
{
	if (!values)
		return 0;

	return (unsigned int)values->entryPool.chunkCount;
}

dsTexture* dsVolatileMaterialValue_getTextureName(const dsVolatileMaterialValues* values,
	const char* name)
{
	if (!values || !name)
		return NULL;

	return (dsTexture*)getValue(values, dsHashString(name), Type_Texture);
}

dsTexture* dsVolatileMaterialValue_getTextureId(const dsVolatileMaterialValues* values,
	uint32_t nameId)
{
	if (!values)
		return NULL;

	return (dsTexture*)getValue(values, nameId, Type_Texture);
}

bool dsVolatileMaterialValue_setTextureName(dsVolatileMaterialValues* values, const char* name,
	dsTexture* texture)
{
	if (!values || !name)
	{
		errno = EINVAL;
		return false;
	}

	return setValue(values, dsHashString(name), Type_Texture, texture);
}

bool dsVolatileMaterialValue_setTextureId(dsVolatileMaterialValues* values, uint32_t nameId,
	dsTexture* texture)
{
	if (!values)
	{
		errno = EINVAL;
		return false;
	}

	return setValue(values, nameId, Type_Texture, texture);
}

dsShaderVariableGroup* dsVolatileMaterialValue_getVariableGroupName(
	const dsVolatileMaterialValues* values, const char* name)
{
	if (!values || !name)
		return NULL;

	return (dsShaderVariableGroup*)getValue(values, dsHashString(name), Type_ShaderVariableGroup);
}

dsShaderVariableGroup* dsVolatileMaterialValue_getVariableGroupId(
	const dsVolatileMaterialValues* values, uint32_t nameId)
{
	if (!values)
		return NULL;

	return (dsShaderVariableGroup*)getValue(values, nameId, Type_ShaderVariableGroup);
}

bool dsVolatileMaterialValue_setVariableGroupName(dsVolatileMaterialValues* values,
	const char* name, dsShaderVariableGroup* group)
{
	if (!values || !name)
		return false;

	return setValue(values, dsHashString(name), Type_ShaderVariableGroup, group);
}

bool dsVolatileMaterialValue_setVariableGroupId(dsVolatileMaterialValues* values,
	uint32_t nameId, dsShaderVariableGroup* group)
{
	if (!values)
		return false;

	return setValue(values, nameId, Type_ShaderVariableGroup, group);
}

dsGfxBuffer* dsVolatileMaterialValue_getBufferName(const dsVolatileMaterialValues* values,
	const char* name)
{
	if (!values || !name)
		return NULL;

	return (dsGfxBuffer*)getValue(values, dsHashString(name), Type_Buffer);
}

dsGfxBuffer* dsVolatileMaterialValue_getBufferId(const dsVolatileMaterialValues* values,
	uint32_t nameId)
{
	if (!values)
		return NULL;

	return (dsGfxBuffer*)getValue(values, nameId, Type_Buffer);
}

bool dsVolatileMaterialValue_setBufferName(dsVolatileMaterialValues* values, const char* name,
	dsGfxBuffer* buffer)
{
	if (!values || !name)
	{
		errno = EINVAL;
		return false;
	}

	return setValue(values, dsHashString(name), Type_Buffer, buffer);
}

bool dsVolatileMaterialValue_setBufferId(dsVolatileMaterialValues* values, uint32_t nameId,
	dsGfxBuffer* buffer)
{
	if (!values)
	{
		errno = EINVAL;
		return false;
	}

	return setValue(values, nameId, Type_Buffer, buffer);
}

bool dsVolatileMaterialValues_removeValueName(dsVolatileMaterialValues* values, const char* name)
{
	if (!values || !name)
		return false;

	return dsVolatileMaterialValues_removeValueId(values, dsHashString(name));
}

bool dsVolatileMaterialValues_removeValueId(dsVolatileMaterialValues* values, uint32_t nameId)
{
	if (!values)
		return false;

	Entry* entry = (Entry*)dsHashTable_remove(values->hashTable, &nameId);
	if (!entry)
		return false;

	DS_VERIFY(dsAllocator_free((dsAllocator*)&values->entryPool, entry));
	return true;
}

bool dsVolatileMaterialValues_clear(dsVolatileMaterialValues* values)
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

void dsVolatileMaterialValues_destroy(dsVolatileMaterialValues* values)
{
	if (!values || !values->allocator)
		return;

	dsAllocator_free(values->allocator, values);
}

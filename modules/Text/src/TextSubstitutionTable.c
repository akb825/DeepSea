/*
 * Copyright 2020 Aaron Barany
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

#include <DeepSea/Text/TextSubstitutionTable.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/PoolAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Text/TextSubstitutionData.h>

#include <string.h>

typedef struct dsTextSubstitutionData
{
	dsAllocator* allocator;
	char* stringData;
	uint32_t maxStringLen;
} dsTextSubstitutionData;

typedef struct SubstitutionNode
{
	dsHashTableNode node;
	char* stringData;
	uint32_t nameLen;
	uint32_t stringDataLen;
} SubstitutionNode;

struct dsTextSubstitutionTable
{
	dsAllocator* allocator;
	dsHashTable* table;
	dsPoolAllocator nodePool;
};

static inline bool addChar(dsTextSubstitutionData* data, uint32_t* len, char c)
{
	uint32_t index = *len;
	if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->stringData, *len, data->maxStringLen, 1))
		return false;

	data->stringData[index] = c;
	return true;
}

static void adjustRanges(uint32_t start, int32_t lenDiff, dsTextStyle* ranges, uint32_t rangeCount)
{
	for (uint32_t i = 0; i < rangeCount; ++i)
	{
		dsTextStyle* range = ranges + i;
		uint32_t rangeEnd = range->start + range->count;
		if (rangeEnd < start)
			continue;

		if (range->start > start)
			range->start += lenDiff;
		else if (range->start <= start && rangeEnd > start)
			range->count += lenDiff;
	}
}

dsTextSubstitutionData* dsTextSubstitutionData_create(dsAllocator* allocator)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_TEXT_LOG_TAG,
			"Allocator for text substitution data must support freeing memory.");
		return NULL;
	}

	dsTextSubstitutionData* data = DS_ALLOCATE_OBJECT(allocator, dsTextSubstitutionData);
	if (!data)
		return NULL;

	data->allocator = allocator;
	data->stringData = NULL;
	data->maxStringLen = 0;
	return data;
}

void dsTextSubstitutionData_destroy(dsTextSubstitutionData* data)
{
	if (!data)
		return;

	DS_VERIFY(dsAllocator_free(data->allocator, data->stringData));
	DS_VERIFY(dsAllocator_free(data->allocator, data));
}

dsTextSubstitutionTable* dsTextSubstitutionTable_create(dsAllocator* allocator,
	uint32_t maxStrings)
{
	if (!allocator || maxStrings == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_TEXT_LOG_TAG,
			"Allocator for text substitution data must support freeing memory.");
		return NULL;
	}

	size_t tableSize = dsHashTable_tableSize(maxStrings);
	size_t tableAllocSize = dsHashTable_fullAllocSize(tableSize);
	size_t poolSize = dsPoolAllocator_bufferSize(sizeof(SubstitutionNode), maxStrings);

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsTextSubstitutionTable)) + tableAllocSize + poolSize;
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsTextSubstitutionTable* table = DS_ALLOCATE_OBJECT(&bufferAlloc, dsTextSubstitutionTable);
	DS_ASSERT(table);

	table->allocator = allocator;
	table->table = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, tableAllocSize);
	DS_ASSERT(table->table);
	DS_VERIFY(dsHashTable_initialize(table->table, tableSize, &dsHashString, &dsHashStringEqual));

	void* poolBuffer = dsAllocator_alloc((dsAllocator*)&bufferAlloc, poolSize);
	DS_ASSERT(poolBuffer);
	DS_VERIFY(dsPoolAllocator_initialize(&table->nodePool, sizeof(SubstitutionNode), maxStrings,
		poolBuffer, poolSize));

	return table;
}

uint32_t dsTextSubstitutionTable_getRemainingStrings(const dsTextSubstitutionTable* table)
{
	if (!table)
		return 0;

	return (uint32_t)table->nodePool.freeCount;
}

bool dsTextSubstitutionTable_setString(dsTextSubstitutionTable* table, const char* name,
	const char* value)
{
	if (!table || !name || !value)
	{
		errno = EINVAL;
		return false;
	}

	size_t valueLen = strlen(value) + 1;
	SubstitutionNode* node = (SubstitutionNode*)dsHashTable_find(table->table, name);
	if (node)
	{
		size_t combinedSize = node->nameLen + valueLen;
		if (combinedSize > node->stringDataLen)
		{
			// Re-allocate string data if larger. Use only the name length for fallback copy of
			// realloc.
			node->stringData = dsAllocator_reallocWithFallback(table->allocator, node->stringData,
				node->nameLen, combinedSize);
			if (!node->stringData)
				return false;

			// Also need to re-assign the key.
			((dsHashTableNode*)node)->key = node->stringData;
		}

		memcpy(node->stringData + node->nameLen, value, valueLen);
		return true;
	}

	node = DS_ALLOCATE_OBJECT(&table->nodePool, SubstitutionNode);
	if (!node)
	{
		errno = ESIZE;
		DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Maximum number of substitution strings has been exceeded.");
		return false;
	}

	size_t nameLen = strlen(name) + 1;
	size_t combinedSize = nameLen + valueLen;
	char* stringData = dsAllocator_alloc(table->allocator, combinedSize);
	if (!stringData)
		return false;

	memcpy(stringData, name, nameLen);
	memcpy(stringData + nameLen, value, valueLen);
	node->stringData = stringData;
	node->nameLen = (uint32_t)nameLen;
	node->stringDataLen = (uint32_t)combinedSize;
	DS_VERIFY(dsHashTable_insert(table->table, stringData, (dsHashTableNode*)node, NULL));
	return true;
}

const char* dsTextSubstitutionTable_getString(dsTextSubstitutionTable* table, const char* name)
{
	if (!table || !name)
		return NULL;

	SubstitutionNode* node = (SubstitutionNode*)dsHashTable_find(table->table, name);
	if (!node)
		return NULL;

	return node->stringData + node->nameLen;
}

bool dsTextSubstitutionTable_removeString(dsTextSubstitutionTable* table, const char* name)
{
	if (!table || !name)
		return false;

	SubstitutionNode* node = (SubstitutionNode*)dsHashTable_remove(table->table, name);
	if (!node)
		return false;

	DS_VERIFY(dsAllocator_free(table->allocator, node->stringData));
	DS_VERIFY(dsAllocator_free((dsAllocator*)&table->nodePool, node));
	return true;
}

const char* dsTextSubstitutionTable_substitute(const dsTextSubstitutionTable* table,
	dsTextSubstitutionData* data, const char* string, dsTextStyle* ranges, uint32_t rangeCount)
{
	if (!table || !data || !string || (!ranges && rangeCount > 0))
	{
		errno = EINVAL;
		return NULL;
	}

	// Reserve the original string's length to avoid excessive re-allocations.
	{
		uint32_t dummyLen = 0;
		if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->stringData, dummyLen,
				data->maxStringLen, (uint32_t)strlen(string) + 1))
		{
			return NULL;
		}
	}

	uint32_t len = 0;
	uint32_t varStart = 0;
	bool lastIsDollar = false;
	bool insideVar = false;
	for (const char* c = string; *c; ++c)
	{
		if (*c == '$')
		{
			lastIsDollar = true;
			continue;
		}
		else if (*c == '{' && lastIsDollar)
		{
			insideVar = true;
			varStart = len;
			continue;
		}
		else if (*c == '}' && insideVar)
		{
			// Variable + ${}
			uint32_t varLen = len - varStart + 3;
			if (!addChar(data, &len, 0))
				return NULL;

			const char* varName = data->stringData + varStart;
			SubstitutionNode* node = (SubstitutionNode*)dsHashTable_find(table->table, varName);
			if (!node)
			{
				errno = ENOTFOUND;
				DS_LOG_ERROR_F(DS_TEXT_LOG_TAG, "Variable '%s' not found for substitution.",
					varName);
				return NULL;
			}

			const char* substitution = node->stringData + node->nameLen;
			uint32_t substitutionLen = (uint32_t)strlen(substitution);
			uint32_t nextLen = varStart + substitutionLen;
			if (nextLen > len)
			{
				len = varStart;
				if (!DS_RESIZEABLE_ARRAY_ADD(data->allocator, data->stringData, len,
						data->maxStringLen, nextLen - len))
				{
					return NULL;
				}
				DS_ASSERT(len == nextLen);
			}
			else
				len = nextLen;

			memcpy(data->stringData + varStart, substitution, substitutionLen);
			adjustRanges(varStart, substitutionLen - varLen, ranges, rangeCount);
			insideVar = false;
			continue;
		}

		// If we got to this point, add the current character.
		if (!addChar(data, &len, *c))
			return NULL;
	}

	if (insideVar)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Ending '}' not present for variable substitution.");
		return NULL;
	}

	if (!addChar(data, &len, 0))
		return NULL;

	return data->stringData;
}

void dsTextSubstitutionTable_destroy(dsTextSubstitutionTable* table)
{
	if (!table)
		return;

	dsListNode* node = table->table->list.head;
	while (node)
	{
		DS_VERIFY(dsAllocator_free(table->allocator, ((SubstitutionNode*)node)->stringData));
		node = node->next;
	}

	DS_VERIFY(dsAllocator_free(table->allocator, table));
}

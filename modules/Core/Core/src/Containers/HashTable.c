/*
 * Copyright 2016-2022 Aaron Barany
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

#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Containers/List.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Memory.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <string.h>

uint32_t dsHashTable_tableSize(uint32_t maxElements)
{
	const float loadFactor = 0.75f;
	return (uint32_t)((float)maxElements/loadFactor);
}

size_t dsHashTable_sizeof(size_t tableSize)
{
	// Subtract the single element for the table used to satisfy the compiler.
	return sizeof(dsHashTable) - sizeof(dsHashTableNode*) + tableSize*sizeof(dsHashTableNode*);
}

size_t dsHashTable_fullAllocSize(size_t tableSize)
{
	return DS_ALIGNED_SIZE(dsHashTable_sizeof(tableSize));
}

bool dsHashTable_initialize(dsHashTable* hashTable, size_t tableSize, dsHashFunction hashFunc,
	dsKeysEqualFunction keysEqualFunc)
{
	if (!hashTable || !tableSize || !hashFunc || !keysEqualFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (!dsList_initialize(&hashTable->list))
		return false;

	hashTable->hashFunc = hashFunc;
	hashTable->keysEqualFunc = keysEqualFunc;
	hashTable->tableSize = tableSize;
	memset(hashTable->table, 0, sizeof(dsHashTableNode*)*tableSize);
	return true;
}

bool dsHashTable_insert(dsHashTable* hashTable, const void* key, dsHashTableNode* node,
	dsHashTableNode** existingNode)
{
	if (!hashTable || !hashTable->hashFunc || !hashTable->keysEqualFunc || !node)
	{
		if (existingNode)
			*existingNode = NULL;
		errno = EINVAL;
		return false;
	}

	uint32_t hash = hashTable->hashFunc(key);
	size_t index = hash % hashTable->tableSize;

	// Check if it's in the chain for the current chain.
	for (dsHashTableNode* chain = hashTable->table[index]; chain; chain = chain->chainNext)
	{
		if (chain->hash == hash && hashTable->keysEqualFunc(chain->key, key))
		{
			if (existingNode)
				*existingNode = chain;
			errno = EPERM;
			return false;
		}
	}

	// Add the node to the chain and the iterator list.
	node->key = key;
	node->hash = hash;
	node->chainNext = hashTable->table[index];
	hashTable->table[index] = node;
	DS_VERIFY(dsList_append((dsList*)hashTable, (dsListNode*)node));

	if (existingNode)
		*existingNode = NULL;
	return true;
}

dsHashTableNode* dsHashTable_find(const dsHashTable* hashTable, const void* key)
{
	if (!hashTable)
		return NULL;

	uint32_t hash = hashTable->hashFunc(key);
	size_t index = hash % hashTable->tableSize;
	for (dsHashTableNode* chain = hashTable->table[index]; chain; chain = chain->chainNext)
	{
		if (chain->hash == hash && hashTable->keysEqualFunc(chain->key, key))
			return chain;
	}

	return NULL;
}

dsHashTableNode* dsHashTable_remove(dsHashTable* hashTable, const void* key)
{
	if (!hashTable)
		return NULL;

	uint32_t hash = hashTable->hashFunc(key);
	size_t index = hash % hashTable->tableSize;
	dsHashTableNode* chain;
	dsHashTableNode* prev = NULL;
	for (chain = hashTable->table[index]; chain; prev = chain, chain = chain->chainNext)
	{
		if (chain->hash == hash && hashTable->keysEqualFunc(chain->key, key))
			break;
	}

	if (!chain)
		return NULL;

	if (prev)
		prev->chainNext = chain->chainNext;
	else
		hashTable->table[index] = chain->chainNext;

	DS_VERIFY(dsList_remove((dsList*)hashTable, (dsListNode*)chain));
	return chain;
}

bool dsHashTable_clear(dsHashTable* hashTable)
{
	if (!hashTable)
	{
		errno = EINVAL;
		return false;
	}

	DS_VERIFY(dsList_clear((dsList*)hashTable));
	memset(hashTable->table, 0, sizeof(dsHashTableNode*)*hashTable->tableSize);
	return true;
}

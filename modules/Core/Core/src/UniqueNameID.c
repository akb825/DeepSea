/*
 * Copyright 2024 Aaron Barany
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

#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <string.h>

typedef struct UniqueNameNode
{
	dsHashTableNode node;
	uint32_t id;
} UniqueNameNode;

typedef struct dsUniqueNameIDs
{
	dsAllocator* allocator;
	dsHashTable* hashTable;
	dsSpinlock lock;
	uint32_t nameLimit;
	uint32_t nextID;
} dsUniqueNameIDs;

static dsUniqueNameIDs* sUniqueNameIDs;

bool dsUniqueNameID_initialize(dsAllocator* allocator, uint32_t initialNameLimit)
{
	if (!allocator || initialNameLimit == 0)
	{
		errno = EINVAL;
		return false;
	}

	if (!allocator->freeFunc)
	{
		DS_LOG_ERROR(DS_CORE_LOG_TAG, "Unique name ID allocator must support freeing memory.");
		errno = EINVAL;
		return false;
	}

	if (sUniqueNameIDs)
	{
		errno = EPERM;
		return false;
	}

	dsUniqueNameIDs* uniqueNameIDs = DS_ALLOCATE_OBJECT(allocator, dsUniqueNameIDs);
	if (!uniqueNameIDs)
		return false;

	size_t tableSize = dsHashTable_tableSize(initialNameLimit);
	size_t hashTableSize = dsHashTable_sizeof(tableSize);
	uniqueNameIDs->allocator = dsAllocator_keepPointer(allocator);
	uniqueNameIDs->hashTable = (dsHashTable*)dsAllocator_alloc(allocator, hashTableSize);
	if (!uniqueNameIDs)
	{
		dsAllocator_free(allocator, uniqueNameIDs);
		return false;
	}

	DS_VERIFY(dsHashTable_initialize(
		uniqueNameIDs->hashTable, tableSize, dsHashString, dsHashStringEqual));
	DS_VERIFY(dsSpinlock_initialize(&uniqueNameIDs->lock));
	uniqueNameIDs->nameLimit = initialNameLimit;
	uniqueNameIDs->nextID = 1;

	sUniqueNameIDs = uniqueNameIDs;
	return true;
}

bool dsUniqueNameID_isInitialized(void)
{
	return sUniqueNameIDs != NULL;
}

uint32_t dsUniqueNameID_create(const char* name)
{
	if (!sUniqueNameIDs)
	{
		errno = EPERM;
		return 0;
	}

	if (!name)
	{
		errno = EINVAL;
		return 0;
	}

	DS_VERIFY(dsSpinlock_lock(&sUniqueNameIDs->lock));

	UniqueNameNode* nameNode = (UniqueNameNode*)dsHashTable_find(sUniqueNameIDs->hashTable, name);
	uint32_t id;
	if (nameNode)
		id = nameNode->id;
	else
	{
		if (sUniqueNameIDs->hashTable->list.length >= sUniqueNameIDs->nameLimit)
		{
			size_t nextNameLimit = sUniqueNameIDs->nameLimit*2;
			size_t tableSize = dsHashTable_tableSize(nextNameLimit);
			size_t hashTableSize = dsHashTable_sizeof(tableSize);
			dsHashTable* nextHashTable = (dsHashTable*)dsAllocator_alloc(
				sUniqueNameIDs->allocator, hashTableSize);
			if (!nextHashTable)
			{
				DS_VERIFY(dsSpinlock_unlock(&sUniqueNameIDs->lock));
				return 0;
			}

			DS_VERIFY(dsHashTable_rehash(nextHashTable, tableSize, sUniqueNameIDs->hashTable));
			DS_VERIFY(dsAllocator_free(sUniqueNameIDs->allocator, sUniqueNameIDs->hashTable));
			sUniqueNameIDs->hashTable = nextHashTable;
		}

		size_t nameLen = strlen(name) + 1;
		size_t fullSize = DS_ALIGNED_SIZE(sizeof(UniqueNameNode)) + DS_ALIGNED_SIZE(nameLen);
		void* buffer = dsAllocator_alloc(sUniqueNameIDs->allocator, fullSize);
		if (!buffer)
		{
			DS_VERIFY(dsSpinlock_unlock(&sUniqueNameIDs->lock));
			return 0;
		}

		dsBufferAllocator bufferAlloc;
		DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

		nameNode = DS_ALLOCATE_OBJECT(&bufferAlloc, UniqueNameNode);
		DS_ASSERT(nameNode);

		char* nameCopy =  DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
		memcpy(nameCopy, name, nameLen);
		id = nameNode->id = sUniqueNameIDs->nextID++;

		DS_VERIFY(dsHashTable_insert(
			sUniqueNameIDs->hashTable, nameCopy, (dsHashTableNode*)nameNode, NULL));
	}

	DS_VERIFY(dsSpinlock_unlock(&sUniqueNameIDs->lock));
	return id;
}

uint32_t dsUniqueNameID_get(const char* name)
{
	if (!sUniqueNameIDs || !name)
		return 0;

	DS_VERIFY(dsSpinlock_lock(&sUniqueNameIDs->lock));

	uint32_t id = 0;
	UniqueNameNode* nameNode = (UniqueNameNode*)dsHashTable_find(sUniqueNameIDs->hashTable, name);
	if (nameNode)
		id = nameNode->id;

	DS_VERIFY(dsSpinlock_unlock(&sUniqueNameIDs->lock));
	return id;
}

bool dsUniqueNameID_shutdown(void)
{
	if (!sUniqueNameIDs)
	{
		errno = EPERM;
		return false;
	}

	dsListNode* node = sUniqueNameIDs->hashTable->list.head;
	while (node)
	{
		dsListNode* nextNode = node->next;
		DS_VERIFY(dsAllocator_free(sUniqueNameIDs->allocator, node));
		node = nextNode;
	}

	DS_VERIFY(dsAllocator_free(sUniqueNameIDs->allocator, sUniqueNameIDs->hashTable));
	dsSpinlock_shutdown(&sUniqueNameIDs->lock);
	DS_VERIFY(dsAllocator_free(sUniqueNameIDs->allocator, sUniqueNameIDs));
	sUniqueNameIDs = NULL;
	return true;
}

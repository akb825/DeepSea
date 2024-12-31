/*
 * Copyright 2016-2024 Aaron Barany
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

#include "Helpers.h"
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Containers/Hash.h>
#include <gtest/gtest.h>

struct TestNode
{
	dsHashTableNode node;
	unsigned int value;
};

static uint32_t chainHashFunction(const void*)
{
	return 0;
}

TEST(HashTableTest, TableSize)
{
	EXPECT_EQ(133U, dsHashTable_tableSize(100));
	EXPECT_EQ(267U, dsHashTable_tableSize(200));
}

TEST(HashTableTest, Initialize)
{
	const unsigned int size = 101;
	DS_STATIC_HASH_TABLE(size) storage;
	EXPECT_EQ(sizeof(storage), dsHashTable_sizeof(size));

	dsHashTable* hashTable = &storage.hashTable;
	EXPECT_FALSE_ERRNO(EINVAL, dsHashTable_initialize(nullptr, size, &dsHashString,
		&dsHashStringEqual));
	EXPECT_TRUE(dsHashTable_initialize(hashTable, size, &dsHashString, &dsHashStringEqual));
	EXPECT_EQ(sizeof(storage), reinterpret_cast<std::uint8_t*>(hashTable->table + size) -
		reinterpret_cast<std::uint8_t*>(hashTable));
}

TEST(HashTableTest, Insert)
{
	TestNode node1 = {{}, 1};
	TestNode node2 = {{}, 2};
	TestNode node3 = {{}, 3};

	const unsigned int size = 101;
	DS_STATIC_HASH_TABLE(size) storage;
	dsHashTable* hashTable = &storage.hashTable;
	EXPECT_TRUE(dsHashTable_initialize(hashTable, size, &dsHashString, &dsHashStringEqual));

	EXPECT_TRUE(dsHashTable_insert(hashTable, "test1", (dsHashTableNode*)&node1, nullptr));
	EXPECT_TRUE(dsHashTable_insert(hashTable, "test2", (dsHashTableNode*)&node2, nullptr));

	dsHashTableNode* existingNode;
	EXPECT_FALSE_ERRNO(EINVAL, dsHashTable_insert(nullptr, "test2", (dsHashTableNode*)&node3,
		nullptr));
	EXPECT_FALSE_ERRNO(EINVAL, dsHashTable_insert(hashTable, "test2", nullptr,
		nullptr));
	EXPECT_FALSE_ERRNO(EPERM, dsHashTable_insert(hashTable, "test2", (dsHashTableNode*)&node3,
		&existingNode));
	EXPECT_EQ((dsHashTableNode*)&node2, existingNode);

	EXPECT_TRUE(dsHashTable_insert(hashTable, "test3", (dsHashTableNode*)&node3, &existingNode));
	EXPECT_EQ(nullptr, existingNode);

	dsList* list = (dsList*)hashTable;
	EXPECT_EQ(3U, list->length);
	EXPECT_EQ((dsListNode*)&node1, list->head);
	EXPECT_EQ((dsListNode*)&node3, list->tail);

	EXPECT_EQ(nullptr, node1.node.listNode.previous);
	EXPECT_EQ((dsListNode*)&node2, node1.node.listNode.next);

	EXPECT_EQ((dsListNode*)&node1, node2.node.listNode.previous);
	EXPECT_EQ((dsListNode*)&node3, node2.node.listNode.next);

	EXPECT_EQ((dsListNode*)&node2, node3.node.listNode.previous);
	EXPECT_EQ(nullptr, node3.node.listNode.next);
}

TEST(HashTableTest, Find)
{
	TestNode node1 = {{}, 1};
	TestNode node2 = {{}, 2};
	TestNode node3 = {{}, 3};

	const unsigned int size = 101;
	DS_STATIC_HASH_TABLE(size) storage;
	dsHashTable* hashTable = &storage.hashTable;
	EXPECT_TRUE(dsHashTable_initialize(hashTable, size, &dsHashString, &dsHashStringEqual));

	EXPECT_TRUE(dsHashTable_insert(hashTable, "test1", (dsHashTableNode*)&node1, nullptr));
	EXPECT_TRUE(dsHashTable_insert(hashTable, "test2", (dsHashTableNode*)&node2, nullptr));
	EXPECT_TRUE(dsHashTable_insert(hashTable, "test3", (dsHashTableNode*)&node3, nullptr));

	EXPECT_EQ((dsHashTableNode*)&node1, dsHashTable_find(hashTable, "test1"));
	EXPECT_EQ((dsHashTableNode*)&node2, dsHashTable_find(hashTable, "test2"));
	EXPECT_EQ((dsHashTableNode*)&node3, dsHashTable_find(hashTable, "test3"));
	EXPECT_FALSE(dsHashTable_find(hashTable, "test4"));
	EXPECT_FALSE(dsHashTable_find(nullptr, "test1"));
}

TEST(HashTableTest, Remove)
{
	TestNode node1 = {{}, 1};
	TestNode node2 = {{}, 2};
	TestNode node3 = {{}, 3};

	const unsigned int size = 101;
	DS_STATIC_HASH_TABLE(size) storage;
	dsHashTable* hashTable = &storage.hashTable;
	EXPECT_TRUE(dsHashTable_initialize(hashTable, size, &dsHashString, &dsHashStringEqual));

	EXPECT_TRUE(dsHashTable_insert(hashTable, "test1", (dsHashTableNode*)&node1, nullptr));
	EXPECT_TRUE(dsHashTable_insert(hashTable, "test2", (dsHashTableNode*)&node2, nullptr));
	EXPECT_TRUE(dsHashTable_insert(hashTable, "test3", (dsHashTableNode*)&node3, nullptr));

	EXPECT_FALSE(dsHashTable_remove(nullptr, "test2"));
	EXPECT_TRUE(dsHashTable_remove(hashTable, "test2"));
	EXPECT_FALSE(dsHashTable_remove(hashTable, "test2"));

	EXPECT_EQ((dsHashTableNode*)&node1, dsHashTable_find(hashTable, "test1"));
	EXPECT_FALSE(dsHashTable_find(hashTable, "test2"));
	EXPECT_EQ((dsHashTableNode*)&node3, dsHashTable_find(hashTable, "test3"));

	dsList* list = (dsList*)hashTable;
	EXPECT_EQ(2U, list->length);
	EXPECT_EQ((dsListNode*)&node1, list->head);
	EXPECT_EQ((dsListNode*)&node3, list->tail);

	EXPECT_EQ(nullptr, node1.node.listNode.previous);
	EXPECT_EQ((dsListNode*)&node3, node1.node.listNode.next);

	EXPECT_EQ((dsListNode*)&node1, node3.node.listNode.previous);
	EXPECT_EQ(nullptr, node3.node.listNode.next);
}

TEST(HashTableTest, Clear)
{
	TestNode node1 = {{}, 1};
	TestNode node2 = {{}, 2};
	TestNode node3 = {{}, 3};

	const unsigned int size = 101;
	DS_STATIC_HASH_TABLE(size) storage;
	dsHashTable* hashTable = &storage.hashTable;
	EXPECT_TRUE(dsHashTable_initialize(hashTable, size, &dsHashString, &dsHashStringEqual));

	EXPECT_TRUE(dsHashTable_insert(hashTable, "test1", (dsHashTableNode*)&node1, nullptr));
	EXPECT_TRUE(dsHashTable_insert(hashTable, "test2", (dsHashTableNode*)&node2, nullptr));
	EXPECT_TRUE(dsHashTable_insert(hashTable, "test3", (dsHashTableNode*)&node3, nullptr));

	EXPECT_FALSE_ERRNO(EINVAL, dsHashTable_clear(nullptr));
	EXPECT_TRUE(dsHashTable_clear(hashTable));

	EXPECT_FALSE(dsHashTable_find(hashTable, "test1"));
	EXPECT_FALSE(dsHashTable_find(hashTable, "test2"));
	EXPECT_FALSE(dsHashTable_find(hashTable, "test3"));

	EXPECT_EQ(0U, hashTable->list.length);
}

TEST(HashTableTest, Chaining)
{
	TestNode node1 = {{}, 1};
	TestNode node2 = {{}, 2};
	TestNode node3 = {{}, 3};

	const unsigned int size = 101;
	DS_STATIC_HASH_TABLE(size) storage;
	dsHashTable* hashTable = &storage.hashTable;
	EXPECT_TRUE(dsHashTable_initialize(hashTable, size, &chainHashFunction, &dsHashStringEqual));

	EXPECT_TRUE(dsHashTable_insert(hashTable, "test1", (dsHashTableNode*)&node1, nullptr));
	EXPECT_TRUE(dsHashTable_insert(hashTable, "test2", (dsHashTableNode*)&node2, nullptr));
	EXPECT_TRUE(dsHashTable_insert(hashTable, "test3", (dsHashTableNode*)&node3, nullptr));

	EXPECT_EQ((dsHashTableNode*)&node1, dsHashTable_find(hashTable, "test1"));
	EXPECT_EQ((dsHashTableNode*)&node2, dsHashTable_find(hashTable, "test2"));
	EXPECT_EQ((dsHashTableNode*)&node3, dsHashTable_find(hashTable, "test3"));

	dsList* list = (dsList*)hashTable;
	EXPECT_EQ(3U, list->length);
	EXPECT_EQ((dsListNode*)&node1, list->head);
	EXPECT_EQ((dsListNode*)&node3, list->tail);

	EXPECT_EQ(nullptr, node1.node.listNode.previous);
	EXPECT_EQ((dsListNode*)&node2, node1.node.listNode.next);

	EXPECT_EQ((dsListNode*)&node1, node2.node.listNode.previous);
	EXPECT_EQ((dsListNode*)&node3, node2.node.listNode.next);

	EXPECT_EQ((dsListNode*)&node2, node3.node.listNode.previous);
	EXPECT_EQ(nullptr, node3.node.listNode.next);

	EXPECT_TRUE(dsHashTable_remove(hashTable, "test2"));

	EXPECT_EQ((dsHashTableNode*)&node1, dsHashTable_find(hashTable, "test1"));
	EXPECT_FALSE(dsHashTable_find(hashTable, "test2"));
	EXPECT_EQ((dsHashTableNode*)&node3, dsHashTable_find(hashTable, "test3"));

	EXPECT_EQ(2U, list->length);
	EXPECT_EQ((dsListNode*)&node1, list->head);
	EXPECT_EQ((dsListNode*)&node3, list->tail);

	EXPECT_EQ(nullptr, node1.node.listNode.previous);
	EXPECT_EQ((dsListNode*)&node3, node1.node.listNode.next);

	EXPECT_EQ((dsListNode*)&node1, node3.node.listNode.previous);
	EXPECT_EQ(nullptr, node3.node.listNode.next);

	EXPECT_TRUE(dsHashTable_remove(hashTable, "test3"));
	EXPECT_TRUE(dsHashTable_remove(hashTable, "test1"));

	EXPECT_FALSE(dsHashTable_find(hashTable, "test1"));
	EXPECT_FALSE(dsHashTable_find(hashTable, "test2"));
	EXPECT_FALSE(dsHashTable_find(hashTable, "test3"));

	EXPECT_EQ(0U, list->length);
}

TEST(HashTableTest, Rehash)
{
	TestNode node1 = {{}, 1};
	TestNode node2 = {{}, 2};
	TestNode node3 = {{}, 3};

	const unsigned int smallSize = 51;
	DS_STATIC_HASH_TABLE(smallSize) smallStorage;
	dsHashTable* smallHashTable = &smallStorage.hashTable;
	EXPECT_TRUE(dsHashTable_initialize(
		smallHashTable, smallSize, &dsHashString, &dsHashStringEqual));

	EXPECT_TRUE(dsHashTable_insert(smallHashTable, "test1", (dsHashTableNode*)&node1, nullptr));
	EXPECT_TRUE(dsHashTable_insert(smallHashTable, "test2", (dsHashTableNode*)&node2, nullptr));
	EXPECT_TRUE(dsHashTable_insert(smallHashTable, "test3", (dsHashTableNode*)&node3, nullptr));

	const unsigned int largeSize = 101;
	DS_STATIC_HASH_TABLE(largeSize) largeStorage;
	dsHashTable* largeHashTable = &largeStorage.hashTable;
	EXPECT_TRUE(dsHashTable_rehash(largeHashTable, largeSize, smallHashTable));

	EXPECT_EQ(0U, smallHashTable->list.length);
	EXPECT_EQ(3U, largeHashTable->list.length);

	EXPECT_EQ((dsHashTableNode*)&node1, dsHashTable_find(largeHashTable, "test1"));
	EXPECT_EQ((dsHashTableNode*)&node2, dsHashTable_find(largeHashTable, "test2"));
	EXPECT_EQ((dsHashTableNode*)&node3, dsHashTable_find(largeHashTable, "test3"));
}

TEST(HashTableTest, RehashChaining)
{
	TestNode node1 = {{}, 1};
	TestNode node2 = {{}, 2};
	TestNode node3 = {{}, 3};

	const unsigned int smallSize = 51;
	DS_STATIC_HASH_TABLE(smallSize) smallStorage;
	dsHashTable* smallHashTable = &smallStorage.hashTable;
	EXPECT_TRUE(dsHashTable_initialize(
		smallHashTable, smallSize, &chainHashFunction, &dsHashStringEqual));

	EXPECT_TRUE(dsHashTable_insert(smallHashTable, "test1", (dsHashTableNode*)&node1, nullptr));
	EXPECT_TRUE(dsHashTable_insert(smallHashTable, "test2", (dsHashTableNode*)&node2, nullptr));
	EXPECT_TRUE(dsHashTable_insert(smallHashTable, "test3", (dsHashTableNode*)&node3, nullptr));

	const unsigned int largeSize = 101;
	DS_STATIC_HASH_TABLE(largeSize) largeStorage;
	dsHashTable* largeHashTable = &largeStorage.hashTable;
	EXPECT_TRUE(dsHashTable_rehash(largeHashTable, largeSize, smallHashTable));

	EXPECT_EQ(0U, smallHashTable->list.length);
	EXPECT_EQ(3U, largeHashTable->list.length);

	EXPECT_EQ((dsHashTableNode*)&node1, dsHashTable_find(largeHashTable, "test1"));
	EXPECT_EQ((dsHashTableNode*)&node2, dsHashTable_find(largeHashTable, "test2"));
	EXPECT_EQ((dsHashTableNode*)&node3, dsHashTable_find(largeHashTable, "test3"));
}

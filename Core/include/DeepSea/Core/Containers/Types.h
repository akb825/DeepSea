/*
 * Copyright 2016 Aaron Barany
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

#pragma once

#include <DeepSea/Core/Config.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Types used for data structures.
 *
 * Each of the data structures declared here depend on the caller to manage the memory. This allows
 * nodes to be statically allocated, dynamically allocated, allocated in a pool, etc.
 */

/**
 * @brief Structure that defines a list node.
 *
 * Put this as the first element of a structure to be able to store it within a dsList.
 *
 * The previous and next pointers must be set to NULL before inserting into a list. This can be
 * done manually or by calling dsListNode_initialize.
 */
typedef struct dsListNode dsListNode;

/** @copydoc dsListNode */
struct dsListNode
{
	/**
	 * @brief The previous node in the list.
	 */
	dsListNode* previous;

	/**
	 * @brief The next node in the list.
	 */
	dsListNode* next;
};

/**
 * @brief Structure for a linked list.
 */
typedef struct dsList
{
	/**
	 * @brief The number of nodes within the list.
	 */
	size_t length;

	/**
	 * @brief The first node of the list.
	 */
	dsListNode* head;

	/**
	 * @brief The last node of the list.
	 */
	dsListNode* tail;
} dsList;

/**
 * @brief Structure that describes a hash table node.
 *
 * Put this as the first element of a structure to be able to store it within a dsHashTable.
 *
 * The hash table chains collisions in a linked list. This additionally a list node, which can be
 * used to iterate over the nodes in the hash table.
 */
typedef struct dsHashTableNode dsHashTableNode;

/** @copydoc dsHashTableNode */
struct dsHashTableNode
{
	/**
	 * @brief The node used to place in the list of all nodes for iteration.
	 */
	dsListNode listNode;

	/**
	 * @brief The next node in the chain for the current hash value.
	 */
	dsHashTableNode* chainNext;

	/**
	 * @brief The hash for the node.
	 */
	size_t hash;
};

#if DS_WINDOWS
#pragma warning(push)
#pragma warning(disable: 4200)
#endif

/**
 * @brief Structure that holds a hash table.
 *
 * The has table can be cast to a dsList in order to access iterate over the nodes and get the
 * total number of elements. This list should not be modified.
 */
typedef struct dsHashTable
{
	/**
	 * @brief The list of nodes. This should not be modified.
	 */
	dsList list;

	/**
	 * @brief The number of elements stored within the table.
	 */
	size_t tableSize;

	/**
	 * @brief The hash table.
	 */
	dsHashTableNode* table[];
} dsHashTable;

#if DS_WINDOWS
#pragma warning(pop)
#endif

/**
 * @brief Declares a statically-allocated hash table.
 *
 * Use this as the type for a variable declaration. For example: DS_STATIC_HASH_TABLE(101) myTable;
 *
 * The pointer can then be cast to type dsHashTable, or accessed as myTable.hashTable.
 *
 * @param tableSize The size of the table.
 */
#define DS_STATIC_HASH_TABLE(tableSize) \
	union \
	{ \
		dsHashTable hashTable; \
		uint8_t data[sizeof(dsHashTable) + tableSize*sizeof(void*)]; \
	}

#ifdef __cplusplus
}
#endif

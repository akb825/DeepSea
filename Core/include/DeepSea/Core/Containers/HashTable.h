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
#include <DeepSea/Core/Export.h>
#include <DeepSea/Core/Containers/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for managing a hash table.
 *
 * None of these functions allocate or deallocate memory, so it is up to the caller to allocate or
 * free if necessary.
 */

/**
 * @brief Calculates the size of a hash table.
 * @param tableSize The number of hash buckets for the hash table.
 * @return The size of the hash table.
 */
DS_CORE_EXPORT size_t dsHashTable_sizeof(size_t tableSize);

/**
 * @brief Calculates the full allocated size of a hash table, including padding.
 * @param tableSize The number of hash buckets for the hash table.
 * @return The full allocated size of the hash table.
 */
DS_CORE_EXPORT size_t dsHashTable_fullAllocSize(size_t tableSize);

/**
 * @brief Initializes a hash table.
 * @param hashTable The hash table to initialize.
 * @param tableSize The number of hash buckets for the hash table. This must match the size used for
 * dsHashTable_sizeof() or DS_STATIC_HASH_TABLE() when allocating the hash table.
 * @param hashFunc The hashing function.
 * @param keysEqualFunc The function to determine if two keys are equal.
 * @return False if the parameters are invalid.
 */
DS_CORE_EXPORT bool dsHashTable_initialize(dsHashTable* hashTable, size_t tableSize,
	dsHashFunction hashFunc, dsKeysEqualFunction keysEqualFunc);

/**
 * @brief Inserts a node into the hash table.
 * @param hashTable The hash table to insert into.
 * @param key The key for the node.
 * @param node The node to insert.
 * @param existingNode If not NULL, this will be set to the node already in the hash table for
 * the same key. The existing will be set to NULL if there was no existing node.
 * @return False if the node wasn't inserted.
 */
DS_CORE_EXPORT bool dsHashTable_insert(dsHashTable* hashTable, const void* key,
	dsHashTableNode* node, dsHashTableNode** existingNode);

/**
 * @brief Finds the a node within the hash table.
 * @param hashTable The hash table to find the node in.
 * @param key The key to find the node for.
 * @return The found node, or NULL if not found.
 */
DS_CORE_EXPORT dsHashTableNode* dsHashTable_find(const dsHashTable* hashTable, const void* key);

/**
 * @brief Removes a node from the hash table.
 * @param hashTable The hash table to remove the node from.
 * @param key The key for the node.
 * @return True if the node was removed.
 */
DS_CORE_EXPORT bool dsHashTable_remove(dsHashTable* hashTable, const void* key);

/**
 * @brief Clears the contents of the hash table.
 * @param hashTable The hash table to clear.
 * @return False if hashTable is NULL.
 */
DS_CORE_EXPORT bool dsHashTable_clear(dsHashTable* hashTable);

#ifdef __cplusplus
}
#endif

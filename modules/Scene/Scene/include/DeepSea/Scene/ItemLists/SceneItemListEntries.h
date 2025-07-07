/*
 * Copyright 2025 Aaron Barany
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
#include <DeepSea/Scene/Types.h>
#include <DeepSea/Scene/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for manipulating entries within scene item lists.
 *
 * These are helpers for efficient management of scene item list entries. This assumes that the
 * entries are kept in a contiguous array, with new entries appended with an ID that's a simple
 * incrementing uint64_t counter. When removing entries, the node IDs should be pushed onto an array
 * and removed all at once in pre-transform update, update, or commit (the first of these
 * used within the item list) with dsSceneItemListEntries_removeMulti().
 */

/**
 * @brief Finds an entry by node ID.
 * @param entries The array of entries.
 * @param entryCount The number of entries.
 * @param entrySize The size of each entry from sizeof().
 * @param idField The offset of the node ID field within the entry from offsetof().
 * @param nodeID The node ID to find the entry for.
 * @return The entry or NULL if not found.
 */
DS_SCENE_EXPORT const void* dsSceneItemListEntries_findEntry(
	const void* entries, uint32_t entryCount, size_t entrySize, size_t idField, uint64_t nodeID);

/**
 * @brief Removes a single entry.
 *
 * This should only be used when you are guaranteed to only have a single entry to remove, or as a
 * fallback if there's a failure appending the node ID to the list of entries to remove.
 *
 * @param entries The array of entries.
 * @param[inout] entryCount The number of entries. This will be updated for the removed entry.
 * @param entrySize The size of each entry from sizeof().
 * @param idField The offset of the node ID field within the entry from offsetof().
 * @param nodeID The node ID to remove.
 */
DS_SCENE_EXPORT void dsSceneItemListEntries_removeSingle(
	void* entries, uint32_t* entryCount, size_t entrySize, size_t idField, uint64_t nodeID);

/**
 * @brief Removes a single entry by index.
 *
 * This should only be used when you are guaranteed to only have a single entry to remove, or as a
 * fallback if there's a failure appending the node ID to the list of entries to remove.
 *
 * @param entries The array of entries.
 * @param[inout] entryCount The number of entries. This will be updated for the removed entry.
 * @param entrySize The size of each entry from sizeof().
 * @param index The index of the entry to remove.
 */
DS_SCENE_EXPORT void dsSceneItemListEntries_removeSingleIndex(
	void* entries, uint32_t* entryCount, size_t entrySize, size_t index);

/**
 * @brief Removes multiple entries by index.
 * @param entries The array of entries.
 * @param[inout] entryCount The number of entries. This will be updated based on any removed
 *     entries.
 * @param entrySize The size of each entry from sizeof().
 * @param idField The offset of the node ID field within the entry from offsetof().
 * @param nodeIDs The node IDs to remove. This array will be modified.
 * @param nodeIDCount The number of node IDs to remove.
 */
DS_SCENE_EXPORT void dsSceneItemListEntries_removeMulti(void* entries, uint32_t* entryCount,
	size_t entrySize, size_t idField, uint64_t* nodeIDs, uint32_t nodeIDCount);

#ifdef __cplusplus
}
#endif

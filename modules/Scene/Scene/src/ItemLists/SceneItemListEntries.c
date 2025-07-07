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

#include <DeepSea/Scene/ItemLists/SceneItemListEntries.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Sort.h>

#include <stdlib.h>
#include <string.h>

static int idEntryCompare(const void* left, const void* right, void* context)
{
	size_t idField = *(size_t*)context;
	right = (const uint8_t*)right + idField;
	return DS_CMP(*(const uint64_t*)left, *(const uint64_t*)right);
}

static int idCompare(const void* left, const void* right)
{
	return DS_CMP(*(const uint64_t*)left, *(const uint64_t*)right);
}

const void* dsSceneItemListEntries_findEntry(
	const void* entries, uint32_t entryCount, size_t entrySize, size_t idField, uint64_t nodeID)
{
	DS_ASSERT(entries || entryCount == 0);
	DS_ASSERT(entrySize >= sizeof(uint64_t));
	DS_ASSERT(idField <= entrySize - sizeof(uint64_t));
	return dsBinarySearch(&nodeID, entries, entryCount, entrySize, &idEntryCompare, &idField);
}

void dsSceneItemListEntries_removeSingle(
	void* entries, uint32_t* entryCount, size_t entrySize, size_t idField, uint64_t nodeID)
{
	DS_ASSERT(entryCount);
	DS_ASSERT(entries || *entryCount == 0);
	DS_ASSERT(entrySize >= sizeof(uint64_t));
	DS_ASSERT(idField <= entrySize - sizeof(uint64_t));
	uint8_t* entry = (uint8_t*)dsBinarySearch(
		&nodeID, entries, *entryCount, entrySize, &idEntryCompare, &idField);
	if (!entry)
		return;

	uint8_t* nextEntry = entry + entrySize;
	uint8_t* entryEnd = (uint8_t*)entries + (*entryCount)*entrySize;
	memmove(entry, nextEntry, entryEnd - nextEntry);
	--(*entryCount);
}

void dsSceneItemListEntries_removeSingleIndex(
	void* entries, uint32_t* entryCount, size_t entrySize, size_t index)
{
	DS_ASSERT(entries);
	DS_ASSERT(entryCount);
	DS_ASSERT(index < *entryCount);
	uint8_t* entryBytes = (uint8_t*)entries;
	memmove(entryBytes + index*entrySize, entryBytes + (index + 1)*entrySize,
		(*entryCount - index - 1)*entrySize);
	--(*entryCount);
}

void dsSceneItemListEntries_removeMulti(void* entries, uint32_t* entryCount,
	size_t entrySize, size_t idField, uint64_t* nodeIDs, uint32_t nodeIDCount)
{
	DS_ASSERT(entrySize >= sizeof(uint64_t));
	DS_ASSERT(idField <= entrySize - sizeof(uint64_t));
	DS_ASSERT(entryCount);
	if (!entries || *entryCount == 0 || !nodeIDs || nodeIDCount == 0)
		return;

	qsort(nodeIDs, nodeIDCount, sizeof(uint64_t), &idCompare);
	uint32_t curIDIndex = 0;

	// Search for the first candidate to remove.
	uint8_t* curEntry = (uint8_t*)dsBinarySearchLowerBound(
		nodeIDs + curIDIndex, entries, *entryCount, entrySize, &idEntryCompare, &idField);
	if (!curEntry)
		return;

	uint8_t* target = curEntry;
	uint8_t* entryEnd = (uint8_t*)entries + (*entryCount)*entrySize;
	for (; curEntry < entryEnd; curEntry += entrySize)
	{
		if (curIDIndex < nodeIDCount)
		{
			uint64_t thisID = *(uint64_t*)(curEntry + idField);
			bool remove = thisID == nodeIDs[curIDIndex];
			while (curIDIndex < nodeIDCount && nodeIDs[curIDIndex] <= thisID)
				++curIDIndex;

			if (remove)
			{
				--(*entryCount);
				continue;
			}
		}

		if (target != curEntry)
			memcpy(target, curEntry, entrySize);
		target += entrySize;
	}
}

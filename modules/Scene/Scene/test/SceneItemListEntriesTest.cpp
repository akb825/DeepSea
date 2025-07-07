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

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Scene/ItemLists/SceneItemListEntries.h>

#include <gtest/gtest.h>
#include <vector>

struct TestEntry
{
	uint32_t value;
	uint64_t nodeID;
};

class SceneItemListEntriesTest : public testing::Test
{
public:
	SceneItemListEntriesTest()
		: allocator(reinterpret_cast<dsAllocator*>(&systemAllocator))
	{
	}

	void SetUp() override
	{
		ASSERT_TRUE(dsSystemAllocator_initialize(&systemAllocator, DS_ALLOCATOR_NO_LIMIT));
		entries = nullptr;
		entryCount = 0;
		maxEntries = 0;
		nextNodeID = 0;
	}

	void TearDown() override
	{
		EXPECT_TRUE(dsAllocator_free(allocator, entries));
		EXPECT_EQ(0U, allocator->size);
	}

	uint64_t addEntry(uint32_t value)
	{
		uint32_t index = entryCount;
		bool inserted = DS_RESIZEABLE_ARRAY_ADD(allocator, entries, entryCount, maxEntries, 1);
		EXPECT_TRUE(inserted);
		if (!inserted)
			return DS_NO_SCENE_NODE;

		uint64_t nodeID = nextNodeID++;
		entries[index].value = value;
		entries[index].nodeID = nodeID;
		return nodeID;
	}

	dsSystemAllocator systemAllocator;
	dsAllocator* allocator;

	TestEntry* entries;
	uint32_t entryCount;
	uint32_t maxEntries;
	uint64_t nextNodeID;
};

TEST_F(SceneItemListEntriesTest, FindEntry)
{
	uint64_t firstID = addEntry(1);
	uint64_t secondID = addEntry(2);
	uint64_t thirdID = addEntry(3);
	uint64_t fourthID = addEntry(4);

	auto entry = reinterpret_cast<const TestEntry*>(dsSceneItemListEntries_findEntry(entries,
		entryCount, sizeof(TestEntry), offsetof(TestEntry, nodeID), firstID));
	ASSERT_TRUE(entry);
	EXPECT_EQ(1U, entry->value);

	entry = reinterpret_cast<const TestEntry*>(dsSceneItemListEntries_findEntry(entries,
			entryCount, sizeof(TestEntry), offsetof(TestEntry, nodeID), secondID));
	ASSERT_TRUE(entry);
	EXPECT_EQ(2U, entry->value);

	entry = reinterpret_cast<const TestEntry*>(dsSceneItemListEntries_findEntry(entries,
			entryCount, sizeof(TestEntry), offsetof(TestEntry, nodeID), thirdID));
	ASSERT_TRUE(entry);
	EXPECT_EQ(3U, entry->value);

	entry = reinterpret_cast<const TestEntry*>(dsSceneItemListEntries_findEntry(entries,
			entryCount, sizeof(TestEntry), offsetof(TestEntry, nodeID), fourthID));
	ASSERT_TRUE(entry);
	EXPECT_EQ(4U, entry->value);

	EXPECT_FALSE(dsSceneItemListEntries_findEntry(entries, entryCount, sizeof(TestEntry),
		offsetof(TestEntry, nodeID), nextNodeID));
}

TEST_F(SceneItemListEntriesTest, RemoveSingle)
{
	uint64_t firstID = addEntry(1);
	uint64_t secondID = addEntry(2);
	uint64_t thirdID = addEntry(3);
	uint64_t fourthID = addEntry(4);

	dsSceneItemListEntries_removeSingle(entries, &entryCount, sizeof(TestEntry),
		offsetof(TestEntry, nodeID), thirdID);
	EXPECT_EQ(3U, entryCount);
	dsSceneItemListEntries_removeSingle(entries, &entryCount, sizeof(TestEntry),
		offsetof(TestEntry, nodeID), thirdID);
	ASSERT_EQ(3U, entryCount);
	EXPECT_EQ(1U, entries[0].value);
	EXPECT_EQ(2U, entries[1].value);
	EXPECT_EQ(4U, entries[2].value);

	dsSceneItemListEntries_removeSingle(entries, &entryCount, sizeof(TestEntry),
		offsetof(TestEntry, nodeID), firstID);
	ASSERT_EQ(2U, entryCount);
	EXPECT_EQ(2U, entries[0].value);
	EXPECT_EQ(4U, entries[1].value);

	dsSceneItemListEntries_removeSingle(entries, &entryCount, sizeof(TestEntry),
		offsetof(TestEntry, nodeID), fourthID);
	ASSERT_EQ(1U, entryCount);
	EXPECT_EQ(2U, entries[0].value);

	dsSceneItemListEntries_removeSingle(entries, &entryCount, sizeof(TestEntry),
		offsetof(TestEntry, nodeID), secondID);
	ASSERT_EQ(0U, entryCount);
}

TEST_F(SceneItemListEntriesTest, RemoveSingleIndex)
{
	addEntry(1);
	addEntry(2);
	addEntry(3);
	addEntry(4);

	dsSceneItemListEntries_removeSingleIndex(entries, &entryCount, sizeof(TestEntry), 2);
	ASSERT_EQ(3U, entryCount);
	EXPECT_EQ(1U, entries[0].value);
	EXPECT_EQ(2U, entries[1].value);
	EXPECT_EQ(4U, entries[2].value);

	dsSceneItemListEntries_removeSingleIndex(entries, &entryCount, sizeof(TestEntry), 0);
	ASSERT_EQ(2U, entryCount);
	EXPECT_EQ(2U, entries[0].value);
	EXPECT_EQ(4U, entries[1].value);

	dsSceneItemListEntries_removeSingleIndex(entries, &entryCount, sizeof(TestEntry), 1);
	ASSERT_EQ(1U, entryCount);
	EXPECT_EQ(2U, entries[0].value);

	dsSceneItemListEntries_removeSingleIndex(entries, &entryCount, sizeof(TestEntry), 0);
	ASSERT_EQ(0U, entryCount);
}

TEST_F(SceneItemListEntriesTest, RemoveMulti)
{
	std::vector<uint64_t> removeIDs;
	addEntry(1);
	uint64_t earlierID = addEntry(2);
	uint64_t earlierRemovedID = addEntry(3);
	addEntry(4);
	removeIDs.push_back(addEntry(5));
	addEntry(6);

	removeIDs.push_back(earlierRemovedID);

	dsSceneItemListEntries_removeMulti(entries, &entryCount, sizeof(TestEntry),
		offsetof(TestEntry, nodeID), removeIDs.data(), static_cast<uint32_t>(removeIDs.size()));
	EXPECT_EQ(4U, entryCount);
	dsSceneItemListEntries_removeMulti(entries, &entryCount, sizeof(TestEntry),
		offsetof(TestEntry, nodeID), removeIDs.data(), static_cast<uint32_t>(removeIDs.size()));

	ASSERT_EQ(4U, entryCount);
	EXPECT_EQ(1U, entries[0].value);
	EXPECT_EQ(2U, entries[1].value);
	EXPECT_EQ(4U, entries[2].value);
	EXPECT_EQ(6U, entries[3].value);

	removeIDs.clear();
	addEntry(7);
	removeIDs.push_back(addEntry(8));
	addEntry(9);
	removeIDs.push_back(addEntry(9));
	removeIDs.push_back(addEntry(10));
	addEntry(11);
	removeIDs.push_back(addEntry(12));

	removeIDs.push_back(earlierRemovedID);
	removeIDs.push_back(earlierID);

	dsSceneItemListEntries_removeMulti(entries, &entryCount, sizeof(TestEntry),
		offsetof(TestEntry, nodeID), removeIDs.data(), static_cast<uint32_t>(removeIDs.size()));

	ASSERT_EQ(6U, entryCount);
	EXPECT_EQ(1U, entries[0].value);
	EXPECT_EQ(4U, entries[1].value);
	EXPECT_EQ(6U, entries[2].value);
	EXPECT_EQ(7U, entries[3].value);
	EXPECT_EQ(9U, entries[4].value);
	EXPECT_EQ(11U, entries[5].value);

	removeIDs.clear();
	removeIDs.push_back(nextNodeID);
	dsSceneItemListEntries_removeMulti(entries, &entryCount, sizeof(TestEntry),
		offsetof(TestEntry, nodeID), removeIDs.data(), static_cast<uint32_t>(removeIDs.size()));
	ASSERT_EQ(6U, entryCount);
}

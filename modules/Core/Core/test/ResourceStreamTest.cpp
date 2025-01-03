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

#include "Helpers.h"

#include <DeepSea/Core/Streams/FileUtils.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Streams/ResourceStream.h>

#include <gtest/gtest.h>
#include <stdlib.h>
#include <string>
#include <unordered_map>

// TODO: iOS has restrictive filesystem permissions and we don't have the application code available
// here to set the proper directory.
#if !DS_IOS

class ResourceStreamDirectory : public testing::Test
{
public:
	void SetUp() override
	{
		strncpy(dynamicDir, dsResourceStream_getDynamicDirectory(), DS_PATH_MAX - 1);
		dynamicDir[DS_PATH_MAX - 1] = 0;
		ASSERT_TRUE(dsPath_combine(rootDir, sizeof(rootDir), dynamicDir, "DirectoryIteratorTest"));
		dsResourceStream_setDynamicDirectory(rootDir);
		ASSERT_TRUE(dsPath_combine(firstPath, sizeof(firstPath), rootDir, "first"));
		ASSERT_TRUE(dsPath_combine(secondPath, sizeof(secondPath), rootDir, "second"));
		ASSERT_TRUE(dsPath_combine(thirdPath, sizeof(thirdPath), rootDir, "third"));
		ASSERT_TRUE(dsCreateDirectory(rootDir));
	}

	void TearDown() override
	{
		unlink(firstPath);
		unlink(secondPath);
		rmdir(thirdPath);
		rmdir(rootDir);
		dsResourceStream_setDynamicDirectory(dynamicDir);
	}

private:
	char dynamicDir[DS_PATH_MAX];
	char rootDir[DS_PATH_MAX];
	char firstPath[DS_PATH_MAX];
	char secondPath[DS_PATH_MAX];
	char thirdPath[DS_PATH_MAX];
};

TEST_F(ResourceStreamDirectory, DirectoryIterator)
{
	EXPECT_FALSE_ERRNO(EINVAL, dsResourceStream_openDirectory(dsFileResourceType_Dynamic, NULL));
	EXPECT_FALSE_ERRNO(EINVAL, dsResourceStream_openDirectory(dsFileResourceType_Dynamic, ""));

	dsResourceStream stream;
	EXPECT_FALSE(dsResourceStream_openDirectory(dsFileResourceType_Dynamic, "first"));
	EXPECT_TRUE(errno == ENOENT || errno == ENOTDIR);

	ASSERT_TRUE(dsResourceStream_open(&stream, dsFileResourceType_Dynamic, "first", "w"));
	EXPECT_TRUE(dsResourceStream_close(&stream));

	ASSERT_TRUE(dsResourceStream_open(&stream, dsFileResourceType_Dynamic, "second", "w"));
	EXPECT_TRUE(dsResourceStream_close(&stream));

	char thirdPath[DS_PATH_MAX];
	ASSERT_TRUE(dsResourceStream_getPath(
		thirdPath, sizeof(thirdPath), dsFileResourceType_Dynamic, "third"));
	ASSERT_TRUE(dsCreateDirectory(thirdPath));

	dsDirectoryIterator iterator = dsResourceStream_openDirectory(dsFileResourceType_Dynamic, ".");
	ASSERT_TRUE(iterator);

	std::unordered_map<std::string, bool> entries;
	dsDirectoryEntryResult result;
	do
	{
		dsDirectoryEntry entry;
		result = dsResourceStream_nextDirectoryEntry(&entry, iterator);
		if (result == dsDirectoryEntryResult_Success)
			entries.emplace(entry.name, entry.isDirectory);
	} while (result == dsDirectoryEntryResult_Success);
	EXPECT_EQ(dsDirectoryEntryResult_End, result);

	std::unordered_map<std::string, bool> expectedEntries = {{"first", false},
		{"second", false}, {"third", true}};
	EXPECT_EQ(expectedEntries, entries);
}

#endif // !DS_IOS

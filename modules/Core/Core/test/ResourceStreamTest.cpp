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

#include <DeepSea/Core/Streams/FileStream.h>
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
		ASSERT_TRUE(dsFileStream_createDirectory(rootDir));
		dsResourceStream_setDynamicDirectory(rootDir);
	}

	void TearDown() override
	{
		EXPECT_TRUE(dsResourceStream_removeFile(dsFileResourceType_Dynamic, "first"));
		EXPECT_TRUE(dsResourceStream_removeFile(dsFileResourceType_Dynamic, "second"));
		EXPECT_TRUE(dsResourceStream_removeDirectory(dsFileResourceType_Dynamic, "third"));
		EXPECT_TRUE(dsFileStream_removeDirectory(rootDir));
		dsResourceStream_setDynamicDirectory(dynamicDir);
	}

private:
	char dynamicDir[DS_PATH_MAX];
	char rootDir[DS_PATH_MAX];
};

TEST_F(ResourceStreamDirectory, DirectoryIterator)
{
	EXPECT_FALSE_ERRNO(EINVAL, dsResourceStream_openDirectory(dsFileResourceType_Dynamic, NULL));
	EXPECT_FALSE_ERRNO(EINVAL, dsResourceStream_openDirectory(dsFileResourceType_Dynamic, ""));

	EXPECT_EQ(dsPathStatus_Missing,
		dsResourceStream_pathStatus(dsFileResourceType_Dynamic, "first"));
	EXPECT_FALSE(dsResourceStream_openDirectory(dsFileResourceType_Dynamic, "first"));
	EXPECT_TRUE(errno == ENOENT || errno == ENOTDIR);

	dsResourceStream stream;
	ASSERT_TRUE(dsResourceStream_open(&stream, dsFileResourceType_Dynamic, "first", "w"));
	EXPECT_TRUE(dsResourceStream_close(&stream));
	EXPECT_EQ(dsPathStatus_ExistsFile,
		dsResourceStream_pathStatus(dsFileResourceType_Dynamic, "first"));

	ASSERT_TRUE(dsResourceStream_open(&stream, dsFileResourceType_Dynamic, "second", "w"));
	EXPECT_TRUE(dsResourceStream_close(&stream));
	EXPECT_EQ(dsPathStatus_ExistsFile,
		dsResourceStream_pathStatus(dsFileResourceType_Dynamic, "second"));

	ASSERT_TRUE(dsResourceStream_createDirectory(dsFileResourceType_Dynamic, "third"));
	EXPECT_EQ(dsPathStatus_ExistsDirectory,
		dsResourceStream_pathStatus(dsFileResourceType_Dynamic, "third"));

	dsDirectoryIterator iterator = dsResourceStream_openDirectory(dsFileResourceType_Dynamic, ".");
	ASSERT_TRUE(iterator);

	std::unordered_map<std::string, bool> entries;
	dsPathStatus result;
	do
	{
		char entry[DS_FILE_NAME_MAX];
		result = dsFileStream_nextDirectoryEntry(entry, sizeof(entry), iterator);
		if (result > dsPathStatus_Missing)
			entries.emplace(entry, result == dsPathStatus_ExistsDirectory);
	} while (result > dsPathStatus_Missing);
	EXPECT_EQ(dsPathStatus_Missing, result);
	EXPECT_TRUE(dsResourceStream_closeDirectory(iterator));

	std::unordered_map<std::string, bool> expectedEntries = {{"first", false},
		{"second", false}, {"third", true}};
	EXPECT_EQ(expectedEntries, entries);
}

#endif // !DS_IOS

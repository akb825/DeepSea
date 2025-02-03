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

#include <DeepSea/Core/Streams/AndroidArchive.h>

#if DS_ANDROID

#include "Helpers.h"
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Streams/FileArchive.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Streams/Stream.h>

#include <gtest/gtest.h>
#include <cstring>
#include <set>
#include <string>

extern "C"
{

extern JNIEnv* dsUnitTestJNIEnv;
extern jobject dsUnitTestActivity;

}

class AndroidArchiveTest : public testing::Test
{
public:
	AndroidArchiveTest()
		: allocator(reinterpret_cast<dsAllocator*>(&systemAllocator)), assetManager(nullptr)
	{
	}

	void SetUp() override
	{
		EXPECT_TRUE(dsSystemAllocator_initialize(&systemAllocator, DS_ALLOCATOR_NO_LIMIT));

		const jint capacity = 16;
		ASSERT_LE(0, dsUnitTestJNIEnv->PushLocalFrame(capacity));

		jclass activityClass = dsUnitTestJNIEnv->GetObjectClass(dsUnitTestActivity);
		jmethodID activity_getAssets = dsUnitTestJNIEnv->GetMethodID(activityClass, "getAssets",
			"()Landroid/content/res/AssetManager;");
		assetManager = dsUnitTestJNIEnv->NewGlobalRef(
				dsUnitTestJNIEnv->CallObjectMethod(dsUnitTestActivity, activity_getAssets));

		dsUnitTestJNIEnv->PopLocalFrame(nullptr);
	}

	void TearDown() override
	{
		EXPECT_EQ(0U, allocator->size);
		if (assetManager)
			dsUnitTestJNIEnv->DeleteGlobalRef(assetManager);
	}

	dsSystemAllocator systemAllocator;
	dsAllocator* allocator;
	jobject assetManager;
};

static const char* assetDir = "Core-assets";

TEST_F(AndroidArchiveTest, PathStatus)
{
	char path[DS_PATH_MAX];
	dsAndroidArchive* archive = dsAndroidArchive_open(allocator, dsUnitTestJNIEnv, assetManager);
	ASSERT_TRUE(archive);

	EXPECT_EQ(dsPathStatus_ExistsDirectory, dsAndroidArchive_pathStatus(archive, assetDir));

	ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "text.txt"));
	EXPECT_EQ(dsPathStatus_ExistsFile, dsAndroidArchive_pathStatus(archive, path));

	ASSERT_TRUE(dsPath_combine(path, sizeof(path), ".", assetDir));
	ASSERT_TRUE(dsPath_combine(path, sizeof(path), path, "text.txt"));
	EXPECT_EQ(dsPathStatus_ExistsFile, dsAndroidArchive_pathStatus(archive, path));

	// NOTE: As of the time of writing, non-existent paths will appear as existing directories with
	// AAssetManager.
	// ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "asdf"));
	// EXPECT_EQ(dsPathStatus_Missing, dsAndroidArchive_pathStatus(archive, path));

	dsAndroidArchive_close(archive);
}

TEST_F(AndroidArchiveTest, IterateDirectory)
{
	char path[DS_PATH_MAX];
	dsAndroidArchive* archive = dsAndroidArchive_open(allocator, dsUnitTestJNIEnv, assetManager);
	ASSERT_TRUE(archive);

	ASSERT_TRUE(dsPath_combine(path, sizeof(path), ".", assetDir));
	dsDirectoryIterator iter = dsAndroidArchive_openDirectory(archive, path);
	ASSERT_TRUE(iter);

	char name[DS_FILE_NAME_MAX] = {};
	std::set<std::string> foundNames;
	do
	{
		dsPathStatus status =
			dsAndroidArchive_nextDirectoryEntry(name, sizeof(name), archive, iter);
		EXPECT_NE(dsPathStatus_Error, status);
		if (status <= dsPathStatus_Missing)
			break;

		EXPECT_EQ(status, dsPathStatus_ExistsFile);
		foundNames.insert(name);
	} while (true);

	EXPECT_TRUE(dsAndroidArchive_closeDirectory(archive, iter));

	std::set<std::string> expectedNames =
	{
		"comments.zip",
		"empty",
		"files-only.zip",
		"large.zip",
		"prefix.zip",
		"simple.zip",
		"text.txt",
		"uncompressed.zip"
	};
	EXPECT_EQ(expectedNames, foundNames);

	dsAndroidArchive_close(archive);
}

TEST_F(AndroidArchiveTest, OpenFile)
{
	char path[DS_PATH_MAX];
	char buffer[32];
	dsAndroidArchive* archive = dsAndroidArchive_open(allocator, dsUnitTestJNIEnv, assetManager);
	ASSERT_TRUE(archive);

	EXPECT_FALSE_ERRNO(ENOENT, dsAndroidArchive_openFile(archive, "asdf"));

	ASSERT_TRUE(dsPath_combine(path, sizeof(path), ".", assetDir));
	ASSERT_TRUE(dsPath_combine(path, sizeof(path), path, "text.txt"));
	dsStream* stream = dsAndroidArchive_openFile(archive, path);
	ASSERT_TRUE(stream);

	size_t fileSize = 24U;
	EXPECT_EQ(fileSize, dsStream_remainingBytes(stream));

	size_t readSize = 7;
	EXPECT_EQ(readSize, dsStream_read(stream, buffer, readSize));
	buffer[readSize] = 0;
	EXPECT_STREQ("This is", buffer);

	EXPECT_EQ(readSize, dsStream_tell(stream));
	EXPECT_EQ(fileSize - readSize, dsStream_remainingBytes(stream));

	EXPECT_TRUE(dsStream_seek(stream, 5, dsStreamSeekWay_Current));

	readSize = 5;
	EXPECT_EQ(readSize, dsStream_read(stream, buffer, readSize));
	buffer[readSize] = 0;
	EXPECT_STREQ("a zip", buffer);
	EXPECT_EQ(fileSize, dsStream_tell(stream) + dsStream_remainingBytes(stream));

	EXPECT_TRUE(dsStream_close(stream));

	dsAndroidArchive_close(archive);
}

TEST_F(AndroidArchiveTest, FileArchiveFunctions)
{
	char path[DS_PATH_MAX];
	char name[DS_FILE_NAME_MAX] = {};
	char buffer[32] = {};
	auto archive = reinterpret_cast<dsFileArchive*>(
		dsAndroidArchive_open(allocator, dsUnitTestJNIEnv, assetManager));
	ASSERT_TRUE(archive);

	ASSERT_TRUE(dsPath_combine(path, sizeof(path), assetDir, "text.txt"));
	EXPECT_EQ(dsPathStatus_ExistsFile, dsFileArchive_pathStatus(archive, path));

	dsStream* stream = dsFileArchive_openFile(archive, path);
	ASSERT_TRUE(stream);

	EXPECT_EQ(24U, dsStream_read(stream, buffer, sizeof(buffer)));
	EXPECT_STREQ("This is not a zip file.\n", buffer);

	EXPECT_TRUE(dsStream_close(stream));

	ASSERT_TRUE(dsPath_combine(path, sizeof(path), ".", assetDir));
	dsDirectoryIterator iter = dsFileArchive_openDirectory(archive, path);
	ASSERT_TRUE(iter);
	EXPECT_EQ(dsPathStatus_ExistsFile,
		dsFileArchive_nextDirectoryEntry(name, sizeof(name), archive, iter));
	EXPECT_LT(0U, std::strlen(name));
	EXPECT_TRUE(dsFileArchive_closeDirectory(archive, iter));

	dsFileArchive_close(archive);
}

#endif

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

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <string.h>

typedef struct dsAndroidAssetStream
{
	dsStream stream;
	dsAllocator* allocator;
	AAsset* asset;
} dsAndroidAssetStream;

struct dsAndroidArchive
{
	dsFileArchive archive;
	dsAllocator* allocator;
	JNIEnv* env;
	jobject assetManagerRef;
	AAssetManager* assetManager;
};

static const char* removeLeadingDotDir(const char* path)
{
	if (path[0] != '.')
		return path;

	unsigned int offset = 0;
	if (path[1] == 0)
		offset = 1;
	else if (path[1] == DS_PATH_SEPARATOR)
		offset = 2;

	return path + offset;
}

static size_t dsAndroidAssetStream_read(dsStream* stream, void* data, size_t size)
{
	dsAndroidAssetStream* assetStream = (dsAndroidAssetStream*)stream;
	if (!assetStream || !assetStream->asset || !data)
	{
		errno = EINVAL;
		return 0;
	}

	int readSize = AAsset_read(assetStream->asset, data, size);
	if (readSize < 0)
	{
		errno = EIO;
		return 0;
	}

	return readSize;
}

static bool dsAndroidAssetStream_seek(dsStream* stream, int64_t offset, dsStreamSeekWay way)
{
	dsAndroidAssetStream* assetStream = (dsAndroidAssetStream*)stream;
	if (!assetStream || !assetStream->asset)
	{
		errno = EINVAL;
		return 0;
	}

	int whence;
	switch (way)
	{
		case dsStreamSeekWay_Beginning:
			whence = SEEK_SET;
			break;
		case dsStreamSeekWay_Current:
			whence = SEEK_CUR;
			break;
		case dsStreamSeekWay_End:
			whence = SEEK_END;
			break;
		default:
			DS_ASSERT(false);
			errno = EINVAL;
			return false;
	}

	if (AAsset_seek64(assetStream->asset, offset, whence) == -1)
	{
		errno = EIO;
		return false;
	}

	return true;
}

static uint64_t dsAndroidAssetStream_tell(dsStream* stream)
{
	dsAndroidAssetStream* assetStream = (dsAndroidAssetStream*)stream;
	if (!assetStream || !assetStream->asset)
	{
		errno = EINVAL;
		return 0;
	}

	off64_t position = AAsset_seek64(assetStream->asset, 0, SEEK_CUR);
	if (position == -1)
	{
		errno = EIO;
		return DS_STREAM_INVALID_POS;
	}
	return position;
}

static uint64_t dsAndroidAssetStream_remainingBytes(dsStream* stream)
{
	dsAndroidAssetStream* assetStream = (dsAndroidAssetStream*)stream;
	if (!assetStream || !assetStream->asset)
	{
		errno = EINVAL;
		return 0;
	}

	off64_t size = AAsset_getRemainingLength64(assetStream->asset);
	if (size == -1)
	{
		errno = EIO;
		return DS_STREAM_INVALID_POS;
	}

	return size;
}

static bool dsAndroidAssetStream_close(dsStream* stream)
{
	dsAndroidAssetStream* assetStream = (dsAndroidAssetStream*)stream;
	if (!assetStream || !assetStream->asset)
	{
		errno = EINVAL;
		return false;
	}

	AAsset_close(assetStream->asset);
	return dsAllocator_free(assetStream->allocator, assetStream);
}

dsAndroidArchive* dsAndroidArchive_open(dsAllocator* allocator, JNIEnv* env, jobject assetManager)
{
	if (!allocator || !env || !assetManager)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		DS_LOG_ERROR(DS_CORE_LOG_TAG, "Android archive allocator must support freeing memory.");
		errno = EINVAL;
		return NULL;
	}

	AAssetManager* nativeAssetManager = AAssetManager_fromJava(env, assetManager);
	if (!nativeAssetManager)
	{
		errno = EINVAL;
		return NULL;
	}

	dsAndroidArchive* archive = DS_ALLOCATE_OBJECT(allocator, dsAndroidArchive);
	if (!archive)
		return NULL;

	archive->allocator = dsAllocator_keepPointer(allocator);
	archive->env = env;
	archive->assetManagerRef = (*env)->NewGlobalRef(env, assetManager);
	archive->assetManager = nativeAssetManager;

	dsFileArchive* baseArchive = (dsFileArchive*)archive;
	baseArchive->getPathStatusFunc =
		(dsGetFileArchivePathStatusFunction)&dsAndroidArchive_pathStatus;
	baseArchive->openDirectoryFunc =
		(dsOpenFileArchiveDirectoryFunction)&dsAndroidArchive_openDirectory;
	baseArchive->nextDirectoryEntryFunc =
		(dsNextFileArchiveDirectoryEntryFunction)&dsAndroidArchive_nextDirectoryEntry;
	baseArchive->closeDirectoryFunc =
		(dsCloseFileArchiveDirectoryFunction)&dsAndroidArchive_closeDirectory;
	baseArchive->openFileFunc = (dsOpenFileArchiveFileFunction)&dsAndroidArchive_openFile;
	baseArchive->closeFunc = (dsCloseFileArchiveFunction)&dsAndroidArchive_close;

	return archive;
}

dsPathStatus dsAndroidArchive_pathStatus(const dsAndroidArchive* archive, const char* path)
{
	if (!archive || !path || *path == 0)
	{
		errno = EINVAL;
		return dsPathStatus_Error;
	}

	path = removeLeadingDotDir(path);

	AAsset* asset = AAssetManager_open(archive->assetManager, path, AASSET_MODE_STREAMING);
	if (asset)
	{
		AAsset_close(asset);
		return dsPathStatus_ExistsFile;
	}

	AAssetDir* assetDir = AAssetManager_openDir(archive->assetManager, path);
	if (assetDir)
	{
		AAssetDir_close(assetDir);
		return dsPathStatus_ExistsDirectory;
	}

	return dsPathStatus_Missing;
}

dsDirectoryIterator dsAndroidArchive_openDirectory(
	const dsAndroidArchive* archive, const char* path)
{
	if (!archive || !path || *path == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	path = removeLeadingDotDir(path);
	AAssetDir* dir = AAssetManager_openDir(archive->assetManager, path);
	if (!dir)
	{
		errno = ENOENT;
		return NULL;
	}

	return dir;
}

dsPathStatus dsAndroidArchive_nextDirectoryEntry(
	char* result, size_t resultSize, const dsAndroidArchive* archive, dsDirectoryIterator iterator)
{
	if (!result || resultSize == 0 || !archive || !iterator)
	{
		errno = EINVAL;
		return dsPathStatus_Error;
	}

	const char* entryName = AAssetDir_getNextFileName((AAssetDir*)iterator);
	if (!entryName)
		return dsPathStatus_Missing;

	// NDK unable to provide directory entries at this time.
	size_t nameLen = strlen(entryName) + 1;
	if (nameLen > resultSize)
	{
		errno = ESIZE;
		return dsPathStatus_Error;
	}
	else
		memcpy(result, entryName, nameLen);
	return dsPathStatus_ExistsFile;
}

bool dsAndroidArchive_closeDirectory(const dsAndroidArchive* archive, dsDirectoryIterator iterator)
{
	if (!archive || !iterator)
	{
		errno = EINVAL;
		return false;
	}

	AAssetDir_close((AAssetDir*)iterator);
	return true;
}

dsStream* dsAndroidArchive_openFile(const dsAndroidArchive* archive, const char* path)
{
	if (!archive || !path || *path == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	path = removeLeadingDotDir(path);
	AAsset* asset = AAssetManager_open(archive->assetManager, path, AASSET_MODE_RANDOM);
	if (!asset)
	{
		errno = ENOENT;
		return NULL;
	}

	dsAndroidAssetStream* stream = DS_ALLOCATE_OBJECT(archive->allocator, dsAndroidAssetStream);
	if (!stream)
	{
		AAsset_close(asset);
		return NULL;
	}

	stream->allocator = archive->allocator;
	stream->asset = asset;

	dsStream* baseStream = (dsStream*)stream;
	baseStream->readFunc = &dsAndroidAssetStream_read;
	baseStream->writeFunc = NULL;
	baseStream->seekFunc = &dsAndroidAssetStream_seek;
	baseStream->tellFunc = &dsAndroidAssetStream_tell;
	baseStream->remainingBytesFunc = &dsAndroidAssetStream_remainingBytes;
	baseStream->remainingBytesFunc = NULL;
	baseStream->flushFunc = NULL;
	baseStream->closeFunc = &dsAndroidAssetStream_close;

	return baseStream;
}

void dsAndroidArchive_close(dsAndroidArchive* archive)
{
	if (!archive)
		return;

	(*archive->env)->DeleteGlobalRef(archive->env, archive->assetManagerRef);
	DS_VERIFY(dsAllocator_free(archive->allocator, archive));
}

#endif

/*
 * Copyright 2018 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <DeepSea/Core/Streams/ResourceStream.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <string.h>

#if DS_ANDROID
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <jni.h>
#endif

static char gEmbeddedDir[DS_PATH_MAX];
static char gLocalDir[DS_PATH_MAX];
static char gDynamicDir[DS_PATH_MAX];

#if DS_ANDROID
static AAssetManager* gAssetManager;

static size_t assetRead(dsGenericStream* stream, void* data, size_t size)
{
	if (!stream || !stream->userData || !data)
	{
		errno = EINVAL;
		return 0;
	}

	int readSize = AAsset_read((AAsset*)stream->userData, data, size);
	if (readSize < 0)
	{
		errno = EIO;
		return 0;
	}

	return readSize;
}

static bool assetSeek(dsGenericStream* stream, int64_t offset, dsStreamSeekWay way)
{
	if (!stream || !stream->userData)
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

	if (AAsset_seek((AAsset*)stream->userData, (off_t)offset, whence) == (off_t)-1)
	{
		errno = EIO;
		return false;
	}

	return true;
}

static uint64_t assetTell(dsGenericStream* stream)
{
	if (!stream || !stream->userData)
	{
		errno = EINVAL;
		return 0;
	}

	off_t position = AAsset_seek((AAsset*)stream->userData, 0, SEEK_CUR);
	if (position == (off_t)-1)
	{
		errno = EIO;
		return DS_STREAM_INVALID_POS;
	}
	return position;
}

static bool assetClose(dsGenericStream* stream)
{
	if (!stream || !stream->userData)
	{
		errno = EINVAL;
		return false;
	}

	AAsset_close((AAsset*)stream->userData);
	stream->userData = NULL;
	return true;
}
#endif

bool dsResourceStream_setContext(void* globalContext, void* applicationContext,
	const char* embeddedDir, const char* localDir, const char* dynamicDir)
{
#if DS_ANDROID
	if (!gAssetManager)
	{
		if (!globalContext || !applicationContext)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_CORE_LOG_TAG, "No contexts provided to get the AAssetManager.");
			return false;
		}

		JNIEnv* env = (JNIEnv*)globalContext;
		jobject activity = (jobject)applicationContext;

		jclass activityClass = (*env)->GetObjectClass(env, activity);

		jmethodID activity_getAssets = (*env)->GetMethodID(env, activityClass, "getAssets",
			"()Landroid/content/res/AssetManager;");
		jobject assetManager = (*env)->CallObjectMethod(env, activity, activity_getAssets);
		jobject assetManagerRef = (*env)->NewGlobalRef(env, assetManager);
		gAssetManager = AAssetManager_fromJava(env, assetManagerRef);
	}
#endif

	if (embeddedDir)
		strncpy(gEmbeddedDir, embeddedDir, sizeof(gEmbeddedDir) - 1);
	if (localDir)
		strncpy(gLocalDir, localDir, sizeof(gLocalDir) - 1);
	if (dynamicDir)
		strncpy(gDynamicDir, dynamicDir, sizeof(gDynamicDir) - 1);

	return true;
}

bool dsResourceStream_isFile(dsFileResourceType type)
{
#if DS_ANDROID
	return type != dsFileResourceType_Embedded;
#else
	DS_UNUSED(type);
	return true;
#endif
}

const char* dsResourceStream_getDirectory(dsFileResourceType type)
{
	switch (type)
	{
		case dsFileResourceType_Embedded:
			return gEmbeddedDir;
		case dsFileResourceType_Installed:
			return gLocalDir;
		case dsFileResourceType_Dynamic:
			return gDynamicDir;
		default:
			return NULL;
	}
}

bool dsResourceStream_open(dsResourceStream* stream, dsFileResourceType type, const char* path,
	const char* mode)
{
	if (!stream || !path || !mode)
	{
		errno = EINVAL;
		return false;
	}

	char finalPath[DS_PATH_MAX];
	switch (type)
	{
		case dsFileResourceType_Embedded:
			if (strcmp(mode, "r") != 0 && strcmp(mode, "rb") != 0)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_CORE_LOG_TAG, "Embedded assets cannot be written to.");
				return false;
			}

			dsPath_combine(finalPath, sizeof(finalPath), gEmbeddedDir, path);
#if DS_ANDROID
			if (!gAssetManager)
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_CORE_LOG_TAG, "Android asset manager not set. Call "
					"dsResourceStream_setContext() to get the asset manager.");
				return false;
			}

			AAsset* asset = AAssetManager_open(gAssetManager, path, AASSET_MODE_RANDOM);
			if (!asset)
			{
				errno = ENOTFOUND;
				return false;
			}

			((dsStream*)stream)->readFunc = (dsStreamReadFunction)&assetRead;
			((dsStream*)stream)->writeFunc = NULL;
			((dsStream*)stream)->seekFunc = (dsStreamSeekFunction)&assetSeek;
			((dsStream*)stream)->tellFunc = (dsStreamTellFunction)&assetTell;
			((dsStream*)stream)->flushFunc = NULL;
			((dsStream*)stream)->closeFunc = (dsStreamCloseFunction)&assetClose;
			((dsGenericStream*)stream)->userData = asset;
			stream->isFile = false;
			return true;
#else
			break;
#endif
		case dsFileResourceType_Installed:
			if (strcmp(mode, "r") != 0 && strcmp(mode, "rb") != 0)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_CORE_LOG_TAG, "Installed assets cannot be written to.");
				return false;
			}
			dsPath_combine(finalPath, sizeof(finalPath), gLocalDir, path);
			break;
		case dsFileResourceType_Dynamic:
			dsPath_combine(finalPath, sizeof(finalPath), gDynamicDir, path);
			break;
	}

	stream->isFile = true;
	return dsFileStream_openPath((dsFileStream*)stream, finalPath, mode);
}

size_t dsResourceStream_read(dsResourceStream* stream, void* data, size_t size);
size_t dsResourceStream_write(dsResourceStream* stream, const void* data, size_t size);
bool dsResourceStream_seek(dsResourceStream* stream, int64_t offset, dsStreamSeekWay way);
uint64_t dsResourceStream_tell(dsFileStream* stream);
void dsResourceStream_flush(dsFileStream* stream);
bool dsResourceStream_close(dsResourceStream* stream);

/*
 * Copyright 2018-2025 Aaron Barany
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
#include <malloc.h>
#endif

static char gEmbeddedDir[DS_PATH_MAX];
static char gLocalDir[DS_PATH_MAX];
static char gDynamicDir[DS_PATH_MAX];

#if DS_ANDROID
static AAssetManager* gAssetManager;

typedef struct dsDirectoryIteratorInfo
{
	AAssetDir* assetDir;
	dsDirectoryIterator filesystemIter;
} dsDirectoryIteratorInfo;

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

	if (AAsset_seek64((AAsset*)stream->userData, offset, whence) == -1)
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

	off64_t position = AAsset_seek64((AAsset*)stream->userData, 0, SEEK_CUR);
	if (position == -1)
	{
		errno = EIO;
		return DS_STREAM_INVALID_POS;
	}
	return position;
}

static uint64_t assetRemainingBytes(dsGenericStream* stream)
{
	if (!stream || !stream->userData)
	{
		errno = EINVAL;
		return 0;
	}

	off64_t size = AAsset_getRemainingLength64((AAsset*)stream->userData);
	if (size == -1)
	{
		errno = EIO;
		return DS_STREAM_INVALID_POS;
	}

	return size;
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
	DS_UNUSED(globalContext);
	DS_UNUSED(applicationContext);

#if DS_ANDROID
	if (!globalContext || !applicationContext)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_CORE_LOG_TAG, "No contexts provided to get the AAssetManager.");
		return false;
	}

	JNIEnv* env = (JNIEnv*)globalContext;
	jobject activity = (jobject)applicationContext;

	const jint capacity = 16;
	if ((*env)->PushLocalFrame(env, capacity) < 0)
	{
		errno = ENOMEM;
		return false;
	}

	jclass activityClass = (*env)->GetObjectClass(env, activity);

	if (!gAssetManager)
	{
		jmethodID activity_getAssets = (*env)->GetMethodID(env, activityClass, "getAssets",
			"()Landroid/content/res/AssetManager;");
		jobject assetManager = (*env)->CallObjectMethod(env, activity, activity_getAssets);
		jobject assetManagerRef = (*env)->NewGlobalRef(env, assetManager);
		gAssetManager = AAssetManager_fromJava(env, assetManagerRef);
	}

	jstring localDirString = NULL;
	if (!localDir)
	{
		// fileObj = context.getFilesDir();
		jmethodID activity_getFilesDir = (*env)->GetMethodID(env, activityClass, "getFilesDir",
			"()Ljava/io/File;");
		jobject fileObject = (*env)->CallObjectMethod(env, activity, activity_getFilesDir);
		DS_ASSERT(fileObject);

		// path = fileObject.getCanonicalPath();
		jmethodID file_getCononicalPath = (*env)->GetMethodID(env, (*env)->GetObjectClass(env,
			fileObject), "getCanonicalPath", "()Ljava/lang/String;");
		localDirString = (jstring)(*env)->CallObjectMethod(env, fileObject,
			file_getCononicalPath);
		localDir = (*env)->GetStringUTFChars(env, localDirString, NULL);
	}

	jstring dynamicDirString = NULL;
	if (!dynamicDir)
	{
		// fileObj = context.getExternalFilesDir();
		jmethodID activity_getExternalFilesDir = (*env)->GetMethodID(env, activityClass,
			"getExternalFilesDir", "(Ljava/lang/String;)Ljava/io/File;");
		jobject fileObject = (*env)->CallObjectMethod(env, activity, activity_getExternalFilesDir,
			NULL);
		DS_ASSERT(fileObject);

		// path = fileObject.getCanonicalPath();
		jmethodID file_getCononicalPath = (*env)->GetMethodID(env, (*env)->GetObjectClass(env,
			fileObject), "getCanonicalPath", "()Ljava/lang/String;");
		dynamicDirString = (jstring)(*env)->CallObjectMethod(env, fileObject,
			file_getCononicalPath);
		dynamicDir = (*env)->GetStringUTFChars(env, dynamicDirString, NULL);
	}
#endif

	if (embeddedDir)
		strncpy(gEmbeddedDir, embeddedDir, sizeof(gEmbeddedDir) - 1);
	if (localDir)
		strncpy(gLocalDir, localDir, sizeof(gLocalDir) - 1);
	if (dynamicDir)
		strncpy(gDynamicDir, dynamicDir, sizeof(gDynamicDir) - 1);

#if DS_ANDROID
	if (localDirString)
		(*env)->ReleaseStringUTFChars(env, localDirString, localDir);
	if (dynamicDirString)
		(*env)->ReleaseStringUTFChars(env, dynamicDirString, dynamicDir);
	(*env)->PopLocalFrame(env, NULL);
#endif

	return true;
}

const char* dsResourceStream_getEmbeddedDirectory(void)
{
	return gEmbeddedDir;
}

void dsResourceStream_setEmbeddedDirectory(const char* dir)
{
	if (dir)
		strncpy(gEmbeddedDir, dir, sizeof(gEmbeddedDir) - 1);
}

const char* dsResourceStream_getLocalDirectory(void)
{
	return gLocalDir;
}

void dsResourceStream_setLocalDirectory(const char* dir)
{
	if (dir)
		strncpy(gLocalDir, dir, sizeof(gLocalDir) - 1);
}

const char* dsResourceStream_getDynamicDirectory(void)
{
	return gDynamicDir;
}

void dsResourceStream_setDynamicDirectory(const char* dir)
{
	if (dir)
		strncpy(gDynamicDir, dir, sizeof(gDynamicDir) - 1);
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

bool dsResourceStream_getPath(char* outResult, size_t resultSize, dsFileResourceType type,
	const char* path)
{
	if (!outResult || !path)
	{
		errno = EINVAL;
		return false;
	}

	if (dsPath_isAbsolute(path) && type != dsFileResourceType_External)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_CORE_LOG_TAG,
			"Only resources of type dsFileResourceType_External may be absolute paths.");
		return false;
	}

	switch (type)
	{
		case dsFileResourceType_Embedded:
			return dsPath_combine(outResult, resultSize, gEmbeddedDir, path);
		case dsFileResourceType_Installed:
			return dsPath_combine(outResult, resultSize, gLocalDir, path);
		case dsFileResourceType_Dynamic:
			return dsPath_combine(outResult, resultSize, gDynamicDir, path);
		case dsFileResourceType_External:
		{
			size_t pathLen = strlen(path);
			if (pathLen >= resultSize)
			{
				errno = ESIZE;
				return false;
			}

			memcpy(outResult, path, resultSize + 1);
			return true;
		}
		default:
			errno = EINVAL;
			return false;
	}
}

dsDirectoryIterator dsResourceStream_openDirectory(dsFileResourceType type, const char* path)
{
	if (!path || *path == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	char finalPath[DS_PATH_MAX];
	if (!dsResourceStream_getPath(finalPath, sizeof(finalPath), type, path))
		return NULL;

#if DS_ANDROID
	dsDirectoryIteratorInfo* info = malloc(sizeof(dsDirectoryIteratorInfo));
	if (!info)
		return NULL;

	if (dsResourceStream_isFile(type))
	{
		info->assetDir = NULL;
		info->filesystemIter = dsFileStream_openDirectory(finalPath);
		if (!info->filesystemIter)
		{
			free(info);
			return NULL;
		}
	}
	else
	{
		info->filesystemIter = NULL;
		info->assetDir = AAssetManager_openDir(gAssetManager, finalPath);
		if (!info->assetDir)
		{
			free(info);
			// Most common error type, since specific error unknown.
			errno = ENOTFOUND;
			return NULL;
		}
	}

	return info;
#else
	return dsFileStream_openDirectory(finalPath);
#endif
}

dsDirectoryEntryResult dsResourceStream_nextDirectoryEntry(
	dsDirectoryEntry* outEntry, dsDirectoryIterator iterator)
{
#if DS_ANDROID
	if (!outEntry || !iterator)
	{
		errno = EINVAL;
		return dsDirectoryEntryResult_Error;
	}

	dsDirectoryIteratorInfo* info = (dsDirectoryIteratorInfo*)iterator;
	if (info->assetDir)
	{
		const char* entryName = AAssetDir_getNextFileName(info->assetDir);
		if (!entryName)
			return dsDirectoryEntryResult_End;

		// NDK unable to provide directory entries at this time.
		outEntry->isDirectory = false;
		size_t nameLen = strlen(entryName) + 1;
		if (nameLen > DS_FILE_NAME_MAX)
		{
			errno = ESIZE;
			return dsDirectoryEntryResult_Error;
		}
		else
			memcpy(outEntry->name, entryName, nameLen);
		return dsDirectoryEntryResult_Success;
	}
#endif
	return dsFileStream_nextDirectoryEntry(outEntry, iterator);
}

bool dsResourceStream_closeDirectory(dsDirectoryIterator iterator)
{
#if DS_ANDROID
	if (!iterator)
	{
		errno = EINVAL;
		return false;
	}

	dsDirectoryIteratorInfo* info = (dsDirectoryIteratorInfo*)iterator;
	bool success;
	if (info->assetDir)
	{
		AAssetDir_close(info->assetDir);
		success = true;
	}
	else
		success = dsFileStream_closeDirectory(info->filesystemIter);
	if (success)
		free(info);
	return success;
#else
	return dsFileStream_closeDirectory(iterator);
#endif
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
	if (!dsResourceStream_getPath(finalPath, sizeof(finalPath), type, path))
		return false;

	switch (type)
	{
		case dsFileResourceType_Embedded:
			if (strcmp(mode, "r") != 0 && strcmp(mode, "rb") != 0)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_CORE_LOG_TAG, "Embedded assets cannot be written to.");
				return false;
			}

#if DS_ANDROID
			if (!gAssetManager)
			{
				errno = EPERM;
				DS_LOG_ERROR(DS_CORE_LOG_TAG, "Android asset manager not set. Call "
					"dsResourceStream_setContext() to get the asset manager.");
				return false;
			}

			AAsset* asset = AAssetManager_open(gAssetManager, finalPath, AASSET_MODE_RANDOM);
			if (!asset)
			{
				errno = ENOTFOUND;
				return false;
			}

			dsStream* baseStream = (dsStream*)stream;
			baseStream->readFunc = (dsStreamReadFunction)&assetRead;
			baseStream->writeFunc = NULL;
			baseStream->seekFunc = (dsStreamSeekFunction)&assetSeek;
			baseStream->tellFunc = (dsStreamTellFunction)&assetTell;
			baseStream->remainingBytesFunc = (dsStreamTellFunction)&assetRemainingBytes;
			baseStream->restartFunc = NULL;
			baseStream->flushFunc = NULL;
			baseStream->closeFunc = (dsStreamCloseFunction)&assetClose;
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
			break;
		default:
			break;
	}

	stream->isFile = true;
	return dsFileStream_openPath((dsFileStream*)stream, finalPath, mode);
}

size_t dsResourceStream_read(dsResourceStream* stream, void* data, size_t size);
size_t dsResourceStream_write(dsResourceStream* stream, const void* data, size_t size);
bool dsResourceStream_seek(dsResourceStream* stream, int64_t offset, dsStreamSeekWay way);
uint64_t dsResourceStream_tell(dsResourceStream* stream);
uint64_t dsResourceStream_remainingBytes(dsResourceStream* stream);
void dsResourceStream_flush(dsFileStream* stream);
bool dsResourceStream_close(dsResourceStream* stream);

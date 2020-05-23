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

const char* dsResourceStream_getEmbeddedDir(void)
{
	return gEmbeddedDir;
}

void dsResourceStream_setEmbeddedDir(const char* dir)
{
	if (dir)
		strncpy(gEmbeddedDir, dir, sizeof(gEmbeddedDir) - 1);
}

const char* dsResourceStream_getLocalDir(void)
{
	return gLocalDir;
}

void dsResourceStream_setLocalDir(const char* dir)
{
	if (dir)
		strncpy(gLocalDir, dir, sizeof(gLocalDir) - 1);
}

const char* dsResourceStream_getDynamicDir(void)
{
	return gDynamicDir;
}

void dsResourceStream_setDynamicDir(const char* dir)
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

			strncpy(outResult, path, resultSize);
			return true;
		}
		default:
			errno = EINVAL;
			return false;
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
uint64_t dsResourceStream_tell(dsFileStream* stream);
void dsResourceStream_flush(dsFileStream* stream);
bool dsResourceStream_close(dsResourceStream* stream);

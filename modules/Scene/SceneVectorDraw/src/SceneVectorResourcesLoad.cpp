/*
 * Copyright 2020-2025 Aaron Barany
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

#include "SceneVectorResourcesLoad.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Render/Resources/TextureData.h>

#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <DeepSea/Text/FaceGroup.h>

#include <DeepSea/VectorDraw/VectorResources.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/SceneVectorResources_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

typedef struct dsRelativePathInfo
{
	dsAllocator* allocator;
	const char* basePath;
	void* relativePathUserData;
	dsOpenSceneResourcesRelativePathStreamFunction openRelativePathStreamFunc;
	dsCloseSceneResourcesRelativePathStreamFunction closeRelativePathStreamFunc;
} dsRelativePathInfo;


static dsTexture* loadTexture(void* userData, dsResourceManager* resourceManager,
	dsAllocator* allocator, dsAllocator* tempAllocator, const char* path, dsTextureUsage usage,
	dsGfxMemory memoryHints)
{
	dsRelativePathInfo* pathInfo = (dsRelativePathInfo*)userData;
	char finalPath[DS_PATH_MAX];
	if (!dsPath_combine(finalPath, sizeof(finalPath), pathInfo->basePath, path))
	{
		DS_LOG_ERROR_F(DS_SCENE_VECTOR_DRAW_LOG_TAG, "Path '%s%c%s' is too long.",
			pathInfo->basePath, DS_PATH_SEPARATOR, path);
		return NULL;
	}

	dsStream* stream = pathInfo->openRelativePathStreamFunc(
		pathInfo->relativePathUserData, finalPath);
	if (!stream)
		return NULL;

	dsTexture* texture = dsTextureData_loadStreamToTexture(resourceManager, allocator,
		tempAllocator, stream, NULL, usage, memoryHints);
	pathInfo->closeRelativePathStreamFunc(userData, stream);
	return texture;
}

static bool loadFontFace(void* userData, dsFaceGroup* faceGroup, const char* path, const char* name)
{
	dsRelativePathInfo* pathInfo = (dsRelativePathInfo*)userData;
	char finalPath[DS_PATH_MAX];
	if (!dsPath_combine(finalPath, sizeof(finalPath), pathInfo->basePath, path))
	{
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Path '%s%c%s' is too long.", pathInfo->basePath,
			DS_PATH_SEPARATOR, path);
		return false;
	}

	dsStream* stream = pathInfo->openRelativePathStreamFunc(
		pathInfo->relativePathUserData, finalPath);
	if (!stream)
		return false;

	bool retVal = dsFaceGroup_loadFaceStream(faceGroup, pathInfo->allocator, stream, name);
	pathInfo->closeRelativePathStreamFunc(userData, stream);
	return retVal;
}

void* dsVectorSceneResources_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*, void* userData,
	const uint8_t* data, size_t dataSize, void* relativePathUserData,
	dsOpenSceneResourcesRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseSceneResourcesRelativePathStreamFunction closeRelativePathStreamFunc)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneVectorDraw::VerifyVectorResourcesBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_VECTOR_DRAW_LOG_TAG, "Invalid vector resources flatbuffer format.");
		return nullptr;
	}

	auto vectorResourcesUserData = reinterpret_cast<VectorResourcesUserData*>(userData);
	const dsTextQuality* textQualityRemap =
		vectorResourcesUserData ? vectorResourcesUserData->qualityRemap : nullptr;

	auto fbVectorResources = DeepSeaSceneVectorDraw::GetVectorResources(data);
	dsAllocator* scratchAllocator = dsSceneLoadScratchData_getAllocator(scratchData);
	dsResourceManager* resourceManager =
		dsSceneLoadContext_getRenderer(loadContext)->resourceManager;
	dsVectorResources* resources;
	if (auto fbFileRef = fbVectorResources->resources_as_FileReference())
	{
		resources = dsVectorResources_loadResource(allocator, scratchAllocator, resourceManager,
			DeepSeaScene::convert(fbFileRef->type()), fbFileRef->path()->c_str(), textQualityRemap);
	}
	else if (auto fbRelativePathRef = fbVectorResources->resources_as_RelativePathReference())
	{
		char baseDirectory[DS_PATH_MAX];
		if (!dsPath_getDirectoryName(baseDirectory, sizeof(baseDirectory),
				fbRelativePathRef->path()->c_str()))
		{
			if (errno == EINVAL)
				baseDirectory[0] = 0;
			else
				return nullptr;
		}

		dsStream* stream = openRelativePathStreamFunc(
			relativePathUserData, fbRelativePathRef->path()->c_str());
		if (!stream)
			return nullptr;

		// Read into memory first rather than letting loadStream() do so to ensure only one stream
		// is open at a time.
		size_t size;
		void* buffer = dsStream_readUntilEnd(&size, stream, scratchAllocator);
		closeRelativePathStreamFunc(relativePathUserData, stream);
		if (!buffer)
			return nullptr;

		dsRelativePathInfo pathInfo = {scratchAllocator, baseDirectory, relativePathUserData,
			openRelativePathStreamFunc, closeRelativePathStreamFunc};
		resources = dsVectorResources_loadData(allocator, scratchAllocator, resourceManager, buffer,
			size, &pathInfo, &loadTexture, &loadFontFace, textQualityRemap);
		DS_VERIFY(dsAllocator_free(scratchAllocator, buffer));
	}
	else if (auto fbRawData = fbVectorResources->resources_as_RawData())
	{
		dsRelativePathInfo pathInfo = {scratchAllocator, "", relativePathUserData,
			openRelativePathStreamFunc, closeRelativePathStreamFunc};
		auto fbData = fbRawData->data();
		resources = dsVectorResources_loadData(allocator, scratchAllocator, resourceManager,
			fbData->data(), fbData->size(), &pathInfo, &loadTexture, &loadFontFace,
			textQualityRemap);
	}
	else
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_VECTOR_DRAW_LOG_TAG, "Vector resources flatbuffer data not set.");
		return nullptr;
	}

	return resources;
}

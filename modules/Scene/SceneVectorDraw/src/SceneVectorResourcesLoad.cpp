/*
 * Copyright 2020-2026 Aaron Barany
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

#include <DeepSea/SceneVectorDraw/SceneVectorMaterialSet.h>
#include <DeepSea/SceneVectorDraw/SceneVectorResources.h>

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

typedef struct RelativePathWrapper
{
	const char* basePath;
	void* relativePathUserData;
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc;
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc;
} RelativePathWrapper;

dsStream* openRelativePathStream(void* userData, const char* path, const char* mode)
{
	RelativePathWrapper* pathInfo = (RelativePathWrapper*)userData;
	char finalPath[DS_PATH_MAX];
	if (!dsPath_combine(finalPath, sizeof(finalPath), pathInfo->basePath, path))
	{
		DS_LOG_ERROR_F(DS_SCENE_VECTOR_DRAW_LOG_TAG, "Path '%s%c%s' is too long.",
			pathInfo->basePath, DS_PATH_SEPARATOR, path);
		return NULL;
	}

	return pathInfo->openRelativePathStreamFunc(pathInfo->relativePathUserData, finalPath, mode);
}

void closeRelativePathStream(void* userData, dsStream* stream)
{
	RelativePathWrapper* pathInfo = (RelativePathWrapper*)userData;
	pathInfo->closeRelativePathStreamFunc(pathInfo->relativePathUserData, stream);
}

void* dsVectorSceneResources_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc)
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
		vectorResourcesUserData->hasQualityRemap ? vectorResourcesUserData->qualityRemap : nullptr;

	auto fbVectorResources = DeepSeaSceneVectorDraw::GetVectorResources(data);
	dsAllocator* scratchAllocator = dsSceneLoadScratchData_getAllocator(scratchData);
	dsResourceManager* resourceManager =
		dsSceneLoadContext_getRenderer(loadContext)->resourceManager;

	dsSceneResourceType resourceType;
	auto fbSharedMaterials = fbVectorResources->sharedMaterials();
	dsVectorMaterialSet* sharedMaterials = nullptr;
	if (fbSharedMaterials)
	{
		dsCustomSceneResource* resource;
		if (!dsSceneLoadScratchData_findResource(&resourceType, reinterpret_cast<void**>(&resource),
				scratchData, fbSharedMaterials->c_str()) ||
			resourceType != dsSceneResourceType_Custom ||
			resource->type != dsSceneVectorMaterialSet_type())
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR_F(DS_SCENE_VECTOR_DRAW_LOG_TAG,
				"Couldn't find vector scene material set '%s'.", fbSharedMaterials->c_str());
			return nullptr;
		}

		sharedMaterials = reinterpret_cast<dsVectorMaterialSet*>(resource->resource);
	}

	dsVectorShaders* vectorShaders = nullptr;
	auto fbVectorShaders = fbVectorResources->vectorShaders();
	if (fbVectorShaders)
	{
		dsCustomSceneResource* customResource;
		if (!dsSceneLoadScratchData_findResource(&resourceType,
				reinterpret_cast<void**>(&customResource), scratchData,
				fbVectorShaders->c_str()) ||
			resourceType != dsSceneResourceType_Custom ||
			customResource->type != dsSceneVectorResources_type())
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR_F(DS_SCENE_VECTOR_DRAW_LOG_TAG, "Couldn't find vector shaders '%s'.",
				fbVectorShaders->c_str());
			return nullptr;
		}
		vectorShaders = reinterpret_cast<dsVectorShaders*>(customResource->resource);
	}

	dsShader* textureIconShader = nullptr;
	auto fbTextureIconShader = fbVectorResources->textureIconShader();
	if (fbTextureIconShader)
	{
		if (!dsSceneLoadScratchData_findResource(&resourceType,
				reinterpret_cast<void**>(&textureIconShader), scratchData,
				fbTextureIconShader->c_str()) ||
			resourceType != dsSceneResourceType_Shader)
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR_F(DS_SCENE_VECTOR_DRAW_LOG_TAG, "Couldn't find shader '%s'.",
				fbTextureIconShader->c_str());
			return nullptr;
		}
	}

	dsMaterial* textureIconMaterial = nullptr;
	auto fbTextureIconMaterial = fbVectorResources->textureIconMaterial();
	if (fbTextureIconMaterial)
	{
		if (!dsSceneLoadScratchData_findResource(&resourceType,
				reinterpret_cast<void**>(&textureIconShader), scratchData,
				fbTextureIconMaterial->c_str()) ||
			resourceType != dsSceneResourceType_Material)
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR_F(DS_SCENE_VECTOR_DRAW_LOG_TAG, "Couldn't find material '%s'.",
				fbTextureIconMaterial->c_str());
			return nullptr;
		}
	}

	dsVectorImageInitResources initResources;
	bool hasInitResources = vectorResourcesUserData->commandBuffer && vectorShaders;
	if (hasInitResources)
	{
		initResources.resourceManager = resourceManager;
		initResources.commandBuffer = vectorResourcesUserData->commandBuffer;
		initResources.scratchData = vectorResourcesUserData->scratchData;
		initResources.sharedMaterials = sharedMaterials;
		initResources.shaderModule = vectorShaders->shaderModule;
		initResources.textShaderName = vectorShaders->shaders[dsVectorShaderType_TextColor]->name;
		initResources.resources = nullptr;
		initResources.resourceCount = 0;
		initResources.srgb = fbVectorResources->srgb();
	}

	dsVectorResources* resources;
	if (auto fbFileRef = fbVectorResources->resources_as_FileReference())
	{
		resources = dsVectorResources_loadResource(allocator, scratchAllocator, resourceAllocator,
			resourceManager, DeepSeaScene::convert(fbFileRef->type()), fbFileRef->path()->c_str(),
			textQualityRemap, hasInitResources ? &initResources : nullptr,
			vectorResourcesUserData->pixelSize, vectorShaders, textureIconShader,
			textureIconMaterial);
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
			relativePathUserData, fbRelativePathRef->path()->c_str(), "rb");
		if (!stream)
			return nullptr;

		// Read into memory first rather than letting loadStream() do so to ensure only one stream
		// is open at a time.
		size_t size;
		void* buffer = dsStream_readUntilEnd(&size, stream, scratchAllocator);
		closeRelativePathStreamFunc(relativePathUserData, stream);
		if (!buffer)
			return nullptr;

		RelativePathWrapper pathInfo = {baseDirectory, relativePathUserData,
			openRelativePathStreamFunc, closeRelativePathStreamFunc};
		resources = dsVectorResources_loadData(allocator, scratchAllocator, resourceAllocator,
			resourceManager, buffer, size, &pathInfo, &openRelativePathStream,
			&closeRelativePathStream, textQualityRemap, hasInitResources ? &initResources : nullptr,
			vectorResourcesUserData->pixelSize, vectorShaders, textureIconShader,
			textureIconMaterial);
		DS_VERIFY(dsAllocator_free(scratchAllocator, buffer));
	}
	else if (auto fbRawData = fbVectorResources->resources_as_RawData())
	{
		auto fbData = fbRawData->data();
		resources = dsVectorResources_loadData(allocator, scratchAllocator, resourceAllocator,
			resourceManager, fbData->data(), fbData->size(), relativePathUserData,
			openRelativePathStreamFunc, closeRelativePathStreamFunc, textQualityRemap,
			hasInitResources ? &initResources : nullptr, vectorResourcesUserData->pixelSize,
			vectorShaders, textureIconShader, textureIconMaterial);
	}
	else
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_VECTOR_DRAW_LOG_TAG, "Vector resources flatbuffer data not set.");
		return nullptr;
	}

	return resources;
}

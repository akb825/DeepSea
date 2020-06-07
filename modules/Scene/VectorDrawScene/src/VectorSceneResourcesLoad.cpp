/*
 * Copyright 2020 Aaron Barany
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

#include "VectorSceneResourcesLoad.h"

#include "Flatbuffers/VectorSceneResources_generated.h"
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/VectorDraw/VectorResources.h>

static dsTexture* loadTexture(void*, dsResourceManager*, dsAllocator*, dsAllocator*, const char*,
	dsTextureUsage, dsGfxMemory)
{
	errno = EFORMAT;
	DS_LOG_ERROR(DS_VECTOR_DRAW_SCENE_LOG_TAG,
		"Cannot load textures from file from embedded vector draw resources.");
	return nullptr;
}

static bool loadFontFace(void*, dsFaceGroup*, const char*, const char*)
{
	errno = EFORMAT;
	DS_LOG_ERROR(DS_VECTOR_DRAW_SCENE_LOG_TAG,
		"Cannot load font faces from file from embedded vector draw resources.");
	return false;
}

void* dsVectorSceneResources_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*, void* userData,
	const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaVectorDrawScene::VerifyVectorResourcesBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_VECTOR_DRAW_SCENE_LOG_TAG, "Invalid vector resources flatbuffer format.");
		return nullptr;
	}

	auto vectorResourcesUserData = reinterpret_cast<VectorResourcesUserData*>(userData);
	const dsTextQuality* textQualityRemap =
		vectorResourcesUserData ? vectorResourcesUserData->qualityRemap : nullptr;

	auto fbVectorResources = DeepSeaVectorDrawScene::GetVectorResources(data);
	dsAllocator* scratchAllocator = dsSceneLoadScratchData_getAllocator(scratchData);
	dsResourceManager* resourceManager =
		dsSceneLoadContext_getRenderer(loadContext)->resourceManager;
	dsVectorResources* resources;
	if (auto fbFileRef = fbVectorResources->resources_as_FileReference())
	{
		resources = dsVectorResources_loadResource(allocator, scratchAllocator, resourceManager,
			DeepSeaScene::convert(fbFileRef->type()), fbFileRef->path()->c_str(), textQualityRemap);
	}
	else if (auto fbRawData = fbVectorResources->resources_as_RawData())
	{
		auto fbData = fbRawData->data();
		resources = dsVectorResources_loadData(allocator, scratchAllocator, resourceManager,
			fbData->data(), fbData->size(), nullptr, &loadTexture, &loadFontFace, textQualityRemap);
	}
	else
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Vector resources flatbuffer data not set.");
		return nullptr;
	}

	return resources;
}

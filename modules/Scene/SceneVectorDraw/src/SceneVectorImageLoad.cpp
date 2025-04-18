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

#include "SceneVectorImageLoad.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <DeepSea/SceneVectorDraw/SceneVectorMaterialSet.h>
#include <DeepSea/SceneVectorDraw/SceneVectorResources.h>
#include <DeepSea/SceneVectorDraw/SceneVectorShaders.h>

#include <DeepSea/VectorDraw/VectorImage.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/SceneVectorImage_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

void* dsSceneVectorImage_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize, void* relativePathUserData,
	dsOpenSceneResourcesRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseSceneResourcesRelativePathStreamFunction closeRelativePathStreamFunc)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneVectorDraw::VerifyVectorImageBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_VECTOR_DRAW_LOG_TAG, "Invalid scene vector image flatbuffer format.");
		return nullptr;
	}

	dsResourceManager* resourceManager =
		dsSceneLoadContext_getRenderer(loadContext)->resourceManager;
	auto vectorImageUserData = reinterpret_cast<SceneVectorImageUserData*>(userData);
	auto fbVectorImage = DeepSeaSceneVectorDraw::GetVectorImage(data);

	auto fbSize = fbVectorImage->targetSize();
	dsVector2f size;
	bool hasSize = false;
	if (fbSize)
	{
		size = DeepSeaScene::convert(*fbSize);
		hasSize = true;
	}

	auto fbSharedMaterials = fbVectorImage->sharedMaterials();
	dsVectorMaterialSet* sharedMaterials = nullptr;
	if (fbSharedMaterials)
	{
		dsCustomSceneResource* resource;
		dsSceneResourceType resourceType;
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

	auto fbResources = fbVectorImage->resources();
	dsVectorResources** resources = nullptr;
	uint32_t resourceCount = 0;
	if (fbResources)
	{
		resourceCount = fbResources->size();
		resources = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsVectorResources*, resourceCount);
		for (uint32_t i = 0; i < resourceCount; ++i)
		{
			auto fbResource = (*fbResources)[i];
			if (!fbResource)
			{
				errno = EFORMAT;
				DS_LOG_ERROR(DS_SCENE_VECTOR_DRAW_LOG_TAG, "Vector scene resource is unset.");
				return nullptr;
			}

			dsCustomSceneResource* resource;
			dsSceneResourceType resourceType;
			if (!dsSceneLoadScratchData_findResource(&resourceType,
					reinterpret_cast<void**>(&resource), scratchData, fbResource->c_str()) ||
				resourceType != dsSceneResourceType_Custom ||
				resource->type != dsSceneVectorResources_type())
			{
				errno = ENOTFOUND;
				DS_LOG_ERROR_F(DS_SCENE_VECTOR_DRAW_LOG_TAG,
					"Couldn't find vector scene resource '%s'.", fbResource->c_str());
				return nullptr;
			}

			resources[i] = reinterpret_cast<dsVectorResources*>(resource->resource);
		}
	}

	auto fbShaders = fbVectorImage->vectorShaders();
	dsVectorShaders* shaders;
	{
		dsCustomSceneResource* resource;
		dsSceneResourceType resourceType;
		if (!dsSceneLoadScratchData_findResource(&resourceType,
				reinterpret_cast<void**>(&resource), scratchData, fbShaders->c_str()) ||
			resourceType != dsSceneResourceType_Custom ||
			resource->type != dsSceneVectorShaders_type())
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR_F(DS_SCENE_VECTOR_DRAW_LOG_TAG,
				"Couldn't find vector scene shaders '%s'.", fbShaders->c_str());
			return nullptr;
		}

		shaders = reinterpret_cast<dsVectorShaders*>(resource->resource);
	}

	dsVectorImageInitResources initResources =
	{
		resourceManager,
		vectorImageUserData->commandBuffer,
		vectorImageUserData->scratchData,
		sharedMaterials,
		shaders->shaderModule,
		shaders->shaders[dsVectorShaderType_TextColor]->name,
		resources,
		resourceCount,
		fbVectorImage->srgb()
	};

	dsVectorImage* vectorImage;
	if (auto fileRef = fbVectorImage->image_as_FileReference())
	{
		vectorImage = dsVectorImage_loadResource(allocator, resourceAllocator, &initResources,
			DeepSeaScene::convert(fileRef->type()), fileRef->path()->c_str(),
			vectorImageUserData->pixelSize, hasSize ? &size : nullptr);
	}
	else if (auto fbRelativePathRef = fbVectorImage->image_as_RelativePathReference())
	{
		dsStream* stream = openRelativePathStreamFunc(
			relativePathUserData, fbRelativePathRef->path()->c_str());
		if (!stream)
			return nullptr;

		vectorImage = dsVectorImage_loadStream(allocator, resourceAllocator, &initResources,
			stream, vectorImageUserData->pixelSize, hasSize ? &size : nullptr);
		closeRelativePathStreamFunc(relativePathUserData, stream);
	}
	else if (auto rawData = fbVectorImage->image_as_RawData())
	{
		auto data = rawData->data();
		vectorImage = dsVectorImage_loadData(allocator, resourceAllocator, &initResources,
			data->data(), data->size(), vectorImageUserData->pixelSize, hasSize ? &size : nullptr);
	}
	else
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_VECTOR_DRAW_LOG_TAG, "No data provided for vector image");
		return nullptr;
	}

	return vectorImage;
}

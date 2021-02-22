/*
 * Copyright 2021 Aaron Barany
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

#include "DeferredLightResolveLoad.h"
#include "Flatbuffers/DeferredLightResolve_generated.h"
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/Shader.h>

#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/SceneLighting/SceneLightSet.h>
#include <DeepSea/SceneLighting/DeferredLightResolve.h>

static dsShader* findShader(dsSceneLoadScratchData* scratchData, const char* name)
{
	dsSceneResourceType type;
	dsShader* shader;
	if (!dsSceneLoadScratchData_findResource(&type, (void**)&shader, scratchData, name) ||
		type != dsSceneResourceType_Shader)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "Couldn't find shader '%s'.", name);
		return nullptr;
	}

	return shader;
}

static dsMaterial* findMaterial(dsSceneLoadScratchData* scratchData, const char* name)
{
	dsSceneResourceType type;
	dsMaterial* material;
	if (!dsSceneLoadScratchData_findResource(&type, (void**)&material, scratchData, name) ||
		type != dsSceneResourceType_Material)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "Couldn't find material '%s'.", name);
		return nullptr;
	}

	return material;
}

extern "C"
dsSceneItemList* dsDeferredLightResolve_load(const dsSceneLoadContext*,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void*, const char* name, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneLighting::VerifyDeferredLightResolveBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG,
			"Invalid deferred light resolve flatbuffer format.");
		return nullptr;
	}

	auto fbResolve = DeepSeaSceneLighting::GetDeferredLightResolve(data);

	const char* lightSetName = fbResolve->lightSet()->c_str();
	dsSceneResourceType type;
	dsCustomSceneResource* resource;
	if (!dsSceneLoadScratchData_findResource(&type, (void**)&resource, scratchData,
			lightSetName) || type != dsSceneResourceType_Custom ||
		resource->type != dsSceneLightSet_type())
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "Couldn't find light set '%s'.",
			lightSetName);
		return nullptr;
	}

	dsSceneLightSet* lightSet = reinterpret_cast<dsSceneLightSet*>(resource->resource);

	auto fbAmbientShader = fbResolve->ambientShader();
	dsShader* ambientShader = NULL;
	dsMaterial* ambientMaterial = NULL;
	if (fbAmbientShader)
	{
		ambientShader = findShader(scratchData, fbAmbientShader->c_str());
		auto fbAmbientMaterial = fbResolve->ambientMaterial();
		if (fbAmbientMaterial)
			ambientMaterial = findMaterial(scratchData, fbAmbientMaterial->c_str());
		else
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG,
				"No deferred light resolve ambient material set.");
		}

		if (!ambientShader || !ambientMaterial)
			return nullptr;
	}

	auto fbDirectionalShader = fbResolve->directionalShader();
	dsShader* directionalShader = NULL;
	dsMaterial* directionalMaterial = NULL;
	if (fbDirectionalShader)
	{
		directionalShader = findShader(scratchData, fbDirectionalShader->c_str());
		auto fbDirectionalMaterial = fbResolve->directionalMaterial();
		if (fbDirectionalMaterial)
			directionalMaterial = findMaterial(scratchData, fbDirectionalMaterial->c_str());
		else
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG,
				"No deferred light resolve directional material set.");
		}

		if (!directionalShader || !directionalMaterial)
			return nullptr;
	}

	auto fbPointShader = fbResolve->pointShader();
	dsShader* pointShader = NULL;
	dsMaterial* pointMaterial = NULL;
	if (fbPointShader)
	{
		pointShader = findShader(scratchData, fbPointShader->c_str());
		auto fbPointMaterial = fbResolve->pointMaterial();
		if (fbPointMaterial)
			pointMaterial = findMaterial(scratchData, fbPointMaterial->c_str());
		else
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG,
				"No deferred light resolve point material set.");
		}

		if (!pointShader || !pointMaterial)
			return nullptr;
	}

	auto fbSpotShader = fbResolve->spotShader();
	dsShader* spotShader = NULL;
	dsMaterial* spotMaterial = NULL;
	if (fbSpotShader)
	{
		spotShader = findShader(scratchData, fbSpotShader->c_str());
		auto fbSpotMaterial = fbResolve->spotMaterial();
		if (fbSpotMaterial)
			spotMaterial = findMaterial(scratchData, fbSpotMaterial->c_str());
		else
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG, "No deferred light resolve spot material.");
		}

		if (!spotShader || !spotMaterial)
			return nullptr;
	}

	float intensityThreshold = fbResolve->intensityThreshold();
	if (intensityThreshold <= 0)
		intensityThreshold = DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD;

	return reinterpret_cast<dsSceneItemList*>(dsDeferredLightResolve_create(allocator,
		resourceAllocator, name, lightSet, ambientShader, ambientMaterial, directionalShader,
		directionalMaterial, pointShader, pointMaterial, spotShader, spotMaterial,
		intensityThreshold));
}

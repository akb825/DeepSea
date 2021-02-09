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

	dsShader* ambientShader = findShader(scratchData, fbResolve->ambientShader()->c_str());
	if (!ambientShader)
		return nullptr;

	dsMaterial* ambientMaterial = findMaterial(scratchData, fbResolve->ambientMaterial()->c_str());
	if (!ambientMaterial)
		return nullptr;

	dsShader* directionalShader = findShader(scratchData, fbResolve->directionalShader()->c_str());
	if (!directionalShader)
		return nullptr;

	dsMaterial* directionalMaterial =
		findMaterial(scratchData, fbResolve->directionalMaterial()->c_str());
	if (!directionalMaterial)
		return nullptr;

	dsShader* pointShader = findShader(scratchData, fbResolve->pointShader()->c_str());
	if (!pointShader)
		return nullptr;

	dsMaterial* pointMaterial = findMaterial(scratchData, fbResolve->pointMaterial()->c_str());
	if (!pointMaterial)
		return nullptr;

	dsShader* spotShader = findShader(scratchData, fbResolve->spotShader()->c_str());
	if (!spotShader)
		return nullptr;

	dsMaterial* spotMaterial = findMaterial(scratchData, fbResolve->spotMaterial()->c_str());
	if (!spotMaterial)
		return nullptr;

	float intensityThreshold = fbResolve->intensityThreshold();
	if (intensityThreshold <= 0)
		intensityThreshold = DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD;

	return reinterpret_cast<dsSceneItemList*>(dsDeferredLightResolve_create(allocator,
		resourceAllocator, name, lightSet, ambientShader, ambientMaterial, directionalShader,
		directionalMaterial, pointShader, pointMaterial, spotShader, spotMaterial,
		intensityThreshold));
}

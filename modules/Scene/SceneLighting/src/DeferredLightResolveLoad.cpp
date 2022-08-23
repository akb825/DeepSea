/*
 * Copyright 2021-2022 Aaron Barany
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
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/Shader.h>

#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/SceneLighting/DeferredLightResolve.h>
#include <DeepSea/SceneLighting/SceneLightSet.h>
#include <DeepSea/SceneLighting/SceneShadowManager.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include "Flatbuffers/DeferredLightResolve_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#endif

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

	auto fbShadowManager = fbResolve->shadowManager();
	dsSceneShadowManager* shadowManager = NULL;
	if (fbShadowManager)
	{
		const char* shadowManagerName = fbShadowManager->c_str();
		if (!dsSceneLoadScratchData_findResource(&type, (void**)&resource, scratchData,
				shadowManagerName) || type != dsSceneResourceType_Custom ||
			resource->type != dsSceneShadowManager_type())
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "Couldn't find shadow manager '%s'.",
				shadowManagerName);
			return nullptr;
		}

		shadowManager = reinterpret_cast<dsSceneShadowManager*>(resource->resource);
	}

	dsDeferredLightDrawInfo ambientInfo = {NULL, NULL};
	auto fbAmbient = fbResolve->ambient();
	if (fbAmbient)
	{
		ambientInfo.shader = findShader(scratchData, fbAmbient->shader()->c_str());
		ambientInfo.material = findMaterial(scratchData, fbAmbient->material()->c_str());
		if (!ambientInfo.shader || !ambientInfo.material)
			return nullptr;
	}

	dsDeferredLightDrawInfo lightInfos[dsSceneLightType_Count] =
	{
		{NULL, NULL}, {NULL, NULL}, {NULL, NULL}
	};

	dsDeferredShadowLightDrawInfo shadowLightInfos[dsSceneLightType_Count] =
	{
		{NULL, NULL, NULL}, {NULL, NULL, NULL}, {NULL, NULL, NULL}
	};

	dsSceneLightType lightType = dsSceneLightType_Directional;
	auto fbDirectional = fbResolve->directional();
	if (fbDirectional)
	{
		dsDeferredLightDrawInfo* lightInfo = lightInfos + lightType;
		lightInfo->shader = findShader(scratchData, fbDirectional->shader()->c_str());
		lightInfo->material = findMaterial(scratchData, fbDirectional->material()->c_str());

		if (!lightInfo->shader || !lightInfo->material)
			return nullptr;
	}

	auto fbShadowDirectional = fbResolve->shadowDirectional();
	if (fbShadowDirectional)
	{
		dsDeferredShadowLightDrawInfo* lightInfo = shadowLightInfos + lightType;
		lightInfo->shader = findShader(scratchData, fbShadowDirectional->shader()->c_str());
		lightInfo->material = findMaterial(scratchData, fbShadowDirectional->material()->c_str());
		lightInfo->transformGroupName = fbShadowDirectional->transformGroup()->c_str();
		lightInfo->shadowTextureName = fbShadowDirectional->shadowTexture()->c_str();

		if (!lightInfo->shader || !lightInfo->material)
			return nullptr;
	}

	lightType = dsSceneLightType_Point;
	auto fbPoint = fbResolve->point();
	if (fbPoint)
	{
		dsDeferredLightDrawInfo* lightInfo = lightInfos + lightType;
		lightInfo->shader = findShader(scratchData, fbPoint->shader()->c_str());
		lightInfo->material = findMaterial(scratchData, fbPoint->material()->c_str());

		if (!lightInfo->shader || !lightInfo->material)
			return nullptr;
	}

	auto fbShadowPoint = fbResolve->shadowPoint();
	if (fbShadowPoint)
	{
		dsDeferredShadowLightDrawInfo* lightInfo = shadowLightInfos + lightType;
		lightInfo->shader = findShader(scratchData, fbShadowPoint->shader()->c_str());
		lightInfo->material = findMaterial(scratchData, fbShadowPoint->material()->c_str());
		lightInfo->transformGroupName = fbShadowPoint->transformGroup()->c_str();
		lightInfo->shadowTextureName = fbShadowPoint->shadowTexture()->c_str();

		if (!lightInfo->shader || !lightInfo->material)
			return nullptr;
	}

	lightType = dsSceneLightType_Spot;
	auto fbSpot = fbResolve->spot();
	if (fbSpot)
	{
		dsDeferredLightDrawInfo* lightInfo = lightInfos + lightType;
		lightInfo->shader = findShader(scratchData, fbSpot->shader()->c_str());
		lightInfo->material = findMaterial(scratchData, fbSpot->material()->c_str());

		if (!lightInfo->shader || !lightInfo->material)
			return nullptr;
	}

	auto fbShadowSpot = fbResolve->shadowSpot();
	if (fbShadowSpot)
	{
		dsDeferredShadowLightDrawInfo* lightInfo = shadowLightInfos + lightType;
		lightInfo->shader = findShader(scratchData, fbShadowSpot->shader()->c_str());
		lightInfo->material = findMaterial(scratchData, fbShadowSpot->material()->c_str());
		lightInfo->transformGroupName = fbShadowSpot->transformGroup()->c_str();
		lightInfo->shadowTextureName = fbShadowSpot->shadowTexture()->c_str();

		if (!lightInfo->shader || !lightInfo->material)
			return nullptr;
	}

	float intensityThreshold = fbResolve->intensityThreshold();
	if (intensityThreshold <= 0)
		intensityThreshold = DS_DEFAULT_SCENE_LIGHT_INTENSITY_THRESHOLD;

	return reinterpret_cast<dsSceneItemList*>(dsDeferredLightResolve_create(allocator,
		resourceAllocator, name, lightSet, shadowManager, &ambientInfo, lightInfos,
		shadowLightInfos, intensityThreshold));
}

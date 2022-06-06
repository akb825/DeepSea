/*
 * Copyright 2022 Aaron Barany
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

#include "SceneStandardParticleEmitterFactoryLoad.h"

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Timer.h>

#include <DeepSea/Math/Random.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <DeepSea/SceneParticle/SceneStandardParticleEmitterFactory.h>

#include <time.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

#include "Flatbuffers/StandardParticleEmitterFactory_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#endif

void* dsSceneStandardParticleEmitterFactory_load(const dsSceneLoadContext*,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*, void*,
	const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneParticle::VerifyStandardParticleEmitterFactoryBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_PARTICLE_LOG_TAG,
			"Invalid standard particle emitter factory flatbuffer format.");
		return nullptr;
	}

	auto fbFactory = DeepSeaSceneParticle::GetStandardParticleEmitterFactory(data);

	auto fbParams = fbFactory->params();
	dsParticleEmitterParams params;
	params.maxParticles = fbParams->maxParticles();

	dsSceneResourceType resourceType;
	const char* shaderName = fbParams->shader()->c_str();
	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&params.shader), scratchData, shaderName) ||
		resourceType != dsSceneResourceType_Shader)
	{
		errno = ENOTFOUND;
		DS_LOG_INFO_F(DS_SCENE_PARTICLE_LOG_TAG, "Couldn't find particle shader '%s'.", shaderName);
		return nullptr;
	}

	const char* materialName = fbParams->material()->c_str();
	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&params.material), scratchData, materialName) ||
		resourceType != dsSceneResourceType_Material)
	{
		errno = ENOTFOUND;
		DS_LOG_INFO_F(DS_SCENE_PARTICLE_LOG_TAG, "Couldn't find particle material '%s'.",
			materialName);
		return nullptr;
	}

	auto fbRelativeNode = fbFactory->relativeNode();
	dsSceneNode* relativeNode = nullptr;
	if (fbRelativeNode && (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&relativeNode), scratchData, fbRelativeNode->c_str()) ||
		resourceType != dsSceneResourceType_SceneNode))
	{
		errno = ENOTFOUND;
		DS_LOG_INFO_F(DS_SCENE_PARTICLE_LOG_TAG, "Couldn't find particle relative node '%s'.",
			fbRelativeNode->c_str());
		return nullptr;
	}

	params.instanceValueCount = fbParams->instanceValueCount();
	params.populateInstanceValuesFunc = nullptr;
	params.populateInstanceValuesUserData = nullptr;

	dsStandardParticleEmitterOptions options;
	switch (fbFactory->spawnVolume_type())
	{
		case DeepSeaSceneParticle::ParticleVolume::ParticleBox:
		{
			auto fbBox = fbFactory->spawnVolume_as_ParticleBox();
			options.spawnVolume.type = dsParticleVolumeType_Box;
			options.spawnVolume.box.min = DeepSeaScene::convert(*fbBox->min());
			options.spawnVolume.box.max = DeepSeaScene::convert(*fbBox->max());
			break;
		}
		case DeepSeaSceneParticle::ParticleVolume::ParticleSphere:
		{
			auto fbSphere = fbFactory->spawnVolume_as_ParticleSphere();
			options.spawnVolume.type = dsParticleVolumeType_Sphere;
			options.spawnVolume.sphere.center = DeepSeaScene::convert(*fbSphere->center());
			options.spawnVolume.sphere.radius = fbSphere->radius();
			break;
		}
		case DeepSeaSceneParticle::ParticleVolume::ParticleCylinder:
		{
			auto fbCylinder = fbFactory->spawnVolume_as_ParticleCylinder();
			options.spawnVolume.type = dsParticleVolumeType_Cylinder;
			options.spawnVolume.cylinder.center = DeepSeaScene::convert(*fbCylinder->center());
			options.spawnVolume.cylinder.radius = fbCylinder->radius();
			options.spawnVolume.cylinder.height = fbCylinder->height();
			break;
		}
		case DeepSeaSceneParticle::ParticleVolume::NONE:
			DS_ASSERT(false);
			break;
	}
	options.volumeMatrix = DeepSeaScene::convert(*fbFactory->volumeMatrix());
	options.widthRange = DeepSeaScene::convert(*fbFactory->widthRange());
	auto fbHeightRange = fbFactory->heightRange();
	if (fbHeightRange)
		options.heightRange = DeepSeaScene::convert(*fbHeightRange);
	else
		options.heightRange.x = options.heightRange.y = -1;
	options.baseDirection = DeepSeaScene::convert(*fbFactory->baseDirection());
	dsVector3f_normalize(&options.baseDirection, &options.baseDirection);
	options.directionSpread = fbFactory->directionSpread();
	options.spawnTimeRange = DeepSeaScene::convert(*fbFactory->spawnTimeRange());
	options.activeTimeRange = DeepSeaScene::convert(*fbFactory->activeTimeRange());
	options.speedRange = DeepSeaScene::convert(*fbFactory->speedRange());
	options.rotationRange = DeepSeaScene::convert(*fbFactory->rotationRange());
	auto fbTextureRange = fbFactory->textureRange();
	options.textureRange.x = fbTextureRange->x();
	options.textureRange.y = fbTextureRange->y();
	options.colorHueRange = DeepSeaScene::convert(*fbFactory->colorHueRange());
	options.colorSaturationRange = DeepSeaScene::convert(*fbFactory->colorSaturationRange());
	options.colorValueRange = DeepSeaScene::convert(*fbFactory->colorValueRange());
	options.intensityRange = DeepSeaScene::convert(*fbFactory->intensityRange());

	uint32_t seed = fbFactory->seed();
	if (seed == 0)
		seed = dsRandomSeed();
	return dsSceneStandardParticleEmitterFactory_create(allocator, &params, seed, &options,
		fbFactory->enabled(), fbFactory->startTime(), relativeNode);
}

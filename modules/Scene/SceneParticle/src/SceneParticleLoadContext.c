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

#include <DeepSea/SceneParticle/SceneParticleLoadContext.h>

#include <DeepSea/Core/Error.h>

#include <DeepSea/SceneParticle/ParticleTransformData.h>
#include <DeepSea/SceneParticle/SceneParticleEmitterFactory.h>
#include <DeepSea/SceneParticle/SceneParticleNode.h>
#include <DeepSea/SceneParticle/SceneParticleDrawList.h>
#include <DeepSea/SceneParticle/SceneParticlePrepareList.h>
#include <DeepSea/SceneParticle/SceneStandardParticleEmitterFactory.h>

#include <DeepSea/Scene/SceneLoadContext.h>

#include "ParticleTransformDataLoad.h"
#include "SceneParticleNodeLoad.h"
#include "SceneParticleDrawListLoad.h"
#include "SceneParticlePrepareListLoad.h"
#include "SceneStandardParticleEmitterFactoryLoad.h"

static bool destroyParticleEmitterFactory(void* resource)
{
	dsSceneParticleEmitterFactory_destroy((dsSceneParticleEmitterFactory*)resource);
	return true;
}

bool dsSceneParticleLoadConext_registerTypes(dsSceneLoadContext* loadContext)
{
	if (!loadContext)
	{
		errno = EINVAL;
		return false;
	}

	if (!dsSceneLoadContext_registerCustomResourceType(loadContext,
			dsSceneStandardParticleEmitterFactory_typeName, dsSceneParticleEmitterFactory_type(),
			&dsSceneStandardParticleEmitterFactory_load, &destroyParticleEmitterFactory,
			NULL, NULL, 0))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerNodeType(loadContext, dsSceneParticleNode_typeName,
			&dsSceneParticleNode_load, NULL, NULL))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerItemListType(loadContext, dsSceneParticleDrawList_typeName,
			&dsSceneParticleDrawList_load, NULL, NULL))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerItemListType(loadContext, dsSceneParticlePrepareList_typeName,
			&dsSceneParticlePrepareList_load, NULL, NULL))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerInstanceDataType(loadContext, dsParticleTransformData_typeName,
			&dsParticleTransformData_load, NULL, NULL))
	{
		return false;
	}

	return true;
}

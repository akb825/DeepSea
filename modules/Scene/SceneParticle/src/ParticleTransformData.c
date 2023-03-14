/*
 * Copyright 2022-2023 Aaron Barany
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

#include <DeepSea/SceneParticle/ParticleTransformData.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>

#include <DeepSea/Scene/ItemLists/SceneInstanceVariables.h>
#include <DeepSea/Scene/Types.h>

#include <DeepSea/SceneParticle/SceneParticleNode.h>

#include <string.h>

static dsShaderVariableElement elements[] =
{
	{"world", dsMaterialType_Mat4, 0},
	{"worldView", dsMaterialType_Mat4, 0},
	{"localWorldOrientation", dsMaterialType_Mat3, 0},
	{"localViewOrientation", dsMaterialType_Mat3, 0},
	{"worldViewProj", dsMaterialType_Mat4, 0}
};

typedef struct ParticleTransform
{
	dsMatrix44f world;
	dsMatrix44f worldView;
	dsVector4f localWorldOrientation[3];
	dsVector4f localViewOrientation[3];
	dsMatrix44f worldViewProj;
} ParticleTransform;

static inline void toMatrix33Vectors(dsVector4f outVectors[3], const dsMatrix33f* inMatrix)
{
	for (unsigned int i = 0; i < 3; ++i)
		*(dsVector3f*)(outVectors + i) = inMatrix->columns[i];
}

#if DS_HAS_SIMD
DS_SIMD_START_FLOAT4()
static void dsParticleTransformData_populateDataSIMD(void* userData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount,
	const dsShaderVariableGroupDesc* dataDesc, uint8_t* data, uint32_t stride)
{
	DS_PROFILE_FUNC_START();

	DS_UNUSED(userData);
	DS_UNUSED(dataDesc);
	DS_ASSERT(stride >= sizeof(ParticleTransform));
	for (uint32_t i = 0; i < instanceCount; ++i, data += stride)
	{
		const dsParticleEmitter* emitter =
			dsSceneParticleNode_getEmitterForInstance(instances[i]);
		ParticleTransform* transform = (ParticleTransform*)(data);
		// Store intermeidates on the stack to avoid reading back from GPU memory.
		dsMatrix44f world;
		dsMatrix44f worldView;
		if (emitter)
			world = emitter->transform;
		else
			dsMatrix44_identity(world);
		transform->world = world;
		dsMatrix44f_affineMulSIMD(&worldView, &view->viewMatrix, &world);
		transform->worldView = worldView;
		dsMatrix44f_affineInvert33SIMD(transform->localWorldOrientation, &world);
		dsMatrix44f_affineInvert33SIMD(transform->localViewOrientation, &worldView);
		dsMatrix44f_mulSIMD(&transform->worldViewProj, &view->projectionMatrix, &worldView);
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}
DS_SIMD_END()

DS_SIMD_START_FMA()
static void dsParticleTransformData_populateDataFMA(void* userData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount,
	const dsShaderVariableGroupDesc* dataDesc, uint8_t* data, uint32_t stride)
{
	DS_PROFILE_FUNC_START();

	DS_UNUSED(userData);
	DS_UNUSED(dataDesc);
	DS_ASSERT(stride >= sizeof(ParticleTransform));
	for (uint32_t i = 0; i < instanceCount; ++i, data += stride)
	{
		const dsParticleEmitter* emitter =
			dsSceneParticleNode_getEmitterForInstance(instances[i]);
		ParticleTransform* transform = (ParticleTransform*)(data);
		// Store intermeidates on the stack to avoid reading back from GPU memory.
		dsMatrix44f world;
		dsMatrix44f worldView;
		if (emitter)
			world = emitter->transform;
		else
			dsMatrix44_identity(world);
		transform->world = world;
		dsMatrix44f_affineMulFMA(&worldView, &view->viewMatrix, &world);
		transform->worldView = worldView;
		dsMatrix44f_affineInvert33FMA(transform->localWorldOrientation, &world);
		dsMatrix44f_affineInvert33FMA(transform->localViewOrientation, &worldView);
		dsMatrix44f_mulSIMD(&transform->worldViewProj, &view->projectionMatrix, &worldView);
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}
DS_SIMD_END()
#endif

static void dsParticleTransformData_populateData(void* userData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount,
	const dsShaderVariableGroupDesc* dataDesc, uint8_t* data, uint32_t stride)
{
	DS_PROFILE_FUNC_START();

	DS_UNUSED(userData);
	DS_UNUSED(dataDesc);
	DS_ASSERT(stride >= sizeof(ParticleTransform));
	for (uint32_t i = 0; i < instanceCount; ++i, data += stride)
	{
		const dsParticleEmitter* emitter =
			dsSceneParticleNode_getEmitterForInstance(instances[i]);
		ParticleTransform* transform = (ParticleTransform*)(data);
		// Store intermeidates on the stack to avoid reading back from GPU memory.
		dsMatrix44f world;
		dsMatrix44f worldView;
		if (emitter)
			world = emitter->transform;
		else
			dsMatrix44_identity(world);
		transform->world = world;
		dsMatrix44f_affineMul(&worldView, &view->viewMatrix, &world);
		transform->worldView = worldView;

		dsMatrix33f tempMatrix33Inv;
		dsMatrix44f_affineInvert33(&tempMatrix33Inv, &world);
		toMatrix33Vectors(transform->localWorldOrientation, &tempMatrix33Inv);

		dsMatrix44f_affineInvert33(&tempMatrix33Inv, &worldView);
		toMatrix33Vectors(transform->localViewOrientation, &tempMatrix33Inv);

		dsMatrix44f_mul(&transform->worldViewProj, &view->projectionMatrix,
			&transform->worldView);
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}

const char* const dsParticleTransformData_typeName = "ParticleTransformData";

dsShaderVariableGroupDesc* dsParticleTransformData_createShaderVariableGroupDesc(
	dsResourceManager* resourceManager, dsAllocator* allocator)
{
	if (!resourceManager)
	{
		errno = EINVAL;
		return NULL;
	}

	return dsShaderVariableGroupDesc_create(resourceManager, allocator, elements,
		DS_ARRAY_SIZE(elements));
}

bool dsParticleTransformData_isShaderVariableGroupCompatible(
	const dsShaderVariableGroupDesc* transformDesc)
{
	return transformDesc &&
		dsShaderVariableGroup_areElementsEqual(elements, DS_ARRAY_SIZE(elements),
			transformDesc->elements, transformDesc->elementCount);
}

dsSceneInstanceData* dsParticleTransformData_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsShaderVariableGroupDesc* transformDesc)
{
	if (!allocator || !transformDesc)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!dsParticleTransformData_isShaderVariableGroupCompatible(transformDesc))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_PARTICLE_LOG_TAG,
			"Particle transform data's shader variable group description must have been created "
			"with dsParticleTransformData_createShaderVariableGroupDesc().");
		return NULL;
	}

	dsPopulateSceneInstanceVariablesFunction populateFunc;
#if DS_HAS_SIMD
	if (DS_SIMD_ALWAYS_FMA || (dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		populateFunc = &dsParticleTransformData_populateDataFMA;
	else if (DS_SIMD_ALWAYS_FLOAT4 || (dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		populateFunc = &dsParticleTransformData_populateDataSIMD;
	else
#endif
		populateFunc = &dsParticleTransformData_populateData;
	return dsSceneInstanceVariables_create(allocator, resourceManager, transformDesc,
		dsHashString(dsParticleTransformData_typeName), populateFunc, NULL, NULL);
}

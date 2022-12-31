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

#include <DeepSea/SceneParticle/ParticleTransformData.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Math/SIMD/Matrix44x4.h>
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>

#include <DeepSea/Scene/ItemLists/SceneInstanceVariables.h>
#include <DeepSea/Scene/Nodes/SceneTreeNode.h>
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

#if DS_HAS_SIMD
DS_SIMD_START_FLOAT4()
static void computeTransformsSIMD(ParticleTransform transforms[4], const dsMatrix44x4f *viewMatrix,
	const dsMatrix44x4f* projectionMatrix, const dsMatrix44f* world0,
	const dsMatrix44f* world1, const dsMatrix44f* world2, const dsMatrix44f* world3)
{
	transforms[0].world = *world0;
	transforms[1].world = *world1;
	transforms[2].world = *world2;
	transforms[3].world = *world3;

	dsMatrix44x4f world;
	dsMatrix44x4f_load(&world, world0, world1, world2, world3);

	dsMatrix44x4f worldView;
	dsMatrix44x4f_affineMul(&worldView, viewMatrix, &world);
	dsMatrix44x4f_store(&transforms[0].worldView, &transforms[1].worldView,
		&transforms[2].worldView, &transforms[3].worldView, &worldView);

	dsMatrix44x4f inverse;
	dsMatrix44x4f_invert33(&inverse, &world);
	dsMatrix44x4f_store33(transforms[0].localWorldOrientation, transforms[1].localWorldOrientation,
		transforms[2].localWorldOrientation, transforms[3].localWorldOrientation, &inverse);

	dsMatrix44x4f_invert33(&inverse, &worldView);
	dsMatrix44x4f_store33(transforms[0].localViewOrientation, transforms[1].localViewOrientation,
		transforms[2].localViewOrientation, transforms[3].localViewOrientation, &inverse);

	dsMatrix44x4f worldViewProj;
	dsMatrix44x4f_mul(&worldViewProj, projectionMatrix, &worldView);
	dsMatrix44x4f_store(&transforms[0].worldViewProj, &transforms[1].worldViewProj,
		&transforms[2].worldViewProj, &transforms[3].worldViewProj, &worldViewProj);
}

static void dsInstanceTransformData_populateDataSIMD(const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount, uint8_t* data, uint32_t stride)
{
	dsMatrix44x4f viewMatrix;
	dsMatrix44x4f_load(&viewMatrix, &view->viewMatrix, &view->viewMatrix, &view->viewMatrix,
		&view->viewMatrix);

	dsMatrix44x4f projectionMatrix;
	dsMatrix44x4f_load(&projectionMatrix, &view->projectionMatrix, &view->projectionMatrix,
		&view->projectionMatrix, &view->projectionMatrix);

	dsMatrix44f identity;
	dsMatrix44_identity(identity);

	// First process all full sets of 4 transforms.
	uint32_t i = 0;
	while (i + 4 <= instanceCount)
	{
		const dsParticleEmitter* emitter =
			dsSceneParticleNode_getEmitterForInstance(instances[i++]);
		const dsMatrix44f* world0 = emitter ? &emitter->transform : &identity;
		emitter = dsSceneParticleNode_getEmitterForInstance(instances[i++]);
		const dsMatrix44f* world1 = emitter ? &emitter->transform : &identity;
		emitter = dsSceneParticleNode_getEmitterForInstance(instances[i++]);
		const dsMatrix44f* world2 = emitter ? &emitter->transform : &identity;
		emitter = dsSceneParticleNode_getEmitterForInstance(instances[i++]);
		const dsMatrix44f* world3 = emitter ? &emitter->transform : &identity;

		ParticleTransform transforms[4];
		computeTransformsSIMD(
			transforms, &viewMatrix, &projectionMatrix, world0, world1, world2, world3);

		*(ParticleTransform*)(data) = transforms[0];
		data += stride;
		*(ParticleTransform*)(data) = transforms[1];
		data += stride;
		*(ParticleTransform*)(data) = transforms[2];
		data += stride;
		*(ParticleTransform*)(data) = transforms[3];
		data += stride;
	}

	// Then process any remainder.
	uint32_t rem = instanceCount - i;
	if (rem > 0)
	{
		const dsParticleEmitter* emitter =
			dsSceneParticleNode_getEmitterForInstance(instances[i++]);
		const dsMatrix44f* world0 = emitter ? &emitter->transform : &identity;
		emitter =
			i < instanceCount ? dsSceneParticleNode_getEmitterForInstance(instances[i++]) : NULL;
		const dsMatrix44f* world1 = emitter ? &emitter->transform : &identity;
		emitter =
			i < instanceCount ? dsSceneParticleNode_getEmitterForInstance(instances[i++]) : NULL;
		const dsMatrix44f* world2 = emitter ? &emitter->transform : &identity;
		emitter =
			i < instanceCount ? dsSceneParticleNode_getEmitterForInstance(instances[i++]) : NULL;
		const dsMatrix44f* world3 = emitter ? &emitter->transform : &identity;

		ParticleTransform transforms[4];
		computeTransformsSIMD(
			transforms, &viewMatrix, &projectionMatrix, world0, world1, world2, world3);
		for (uint32_t j = 0; j < rem; ++j, data += stride)
			*(ParticleTransform*)(data) = transforms[j];
	}
}
DS_SIMD_END()

DS_SIMD_START_FMA()
static void computeTransformsFMA(ParticleTransform transforms[4], const dsMatrix44x4f *viewMatrix,
	const dsMatrix44x4f* projectionMatrix, const dsMatrix44f* world0,
	const dsMatrix44f* world1, const dsMatrix44f* world2, const dsMatrix44f* world3)
{
	transforms[0].world = *world0;
	transforms[1].world = *world1;
	transforms[2].world = *world2;
	transforms[3].world = *world3;

	dsMatrix44x4f world;
	dsMatrix44x4f_load(&world, world0, world1, world2, world3);

	dsMatrix44x4f worldView;
	dsMatrix44x4f_affineMulFMA(&worldView, viewMatrix, &world);
	dsMatrix44x4f_store(&transforms[0].worldView, &transforms[1].worldView,
		&transforms[2].worldView, &transforms[3].worldView, &worldView);

	dsMatrix44x4f inverse;
	dsMatrix44x4f_invert33FMA(&inverse, &world);
	dsMatrix44x4f_store33(transforms[0].localWorldOrientation, transforms[1].localWorldOrientation,
		transforms[2].localWorldOrientation, transforms[3].localWorldOrientation, &inverse);

	dsMatrix44x4f_invert33FMA(&inverse, &worldView);
	dsMatrix44x4f_store33(transforms[0].localViewOrientation, transforms[1].localViewOrientation,
		transforms[2].localViewOrientation, transforms[3].localViewOrientation, &inverse);

	dsMatrix44x4f worldViewProj;
	dsMatrix44x4f_mulFMA(&worldViewProj, projectionMatrix, &worldView);
	dsMatrix44x4f_store(&transforms[0].worldViewProj, &transforms[1].worldViewProj,
		&transforms[2].worldViewProj, &transforms[3].worldViewProj, &worldViewProj);
}

static void dsInstanceTransformData_populateDataFMA(const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount, uint8_t* data, uint32_t stride)
{
	dsMatrix44x4f viewMatrix;
	dsMatrix44x4f_load(&viewMatrix, &view->viewMatrix, &view->viewMatrix, &view->viewMatrix,
		&view->viewMatrix);

	dsMatrix44x4f projectionMatrix;
	dsMatrix44x4f_load(&projectionMatrix, &view->projectionMatrix, &view->projectionMatrix,
		&view->projectionMatrix, &view->projectionMatrix);

	dsMatrix44f identity;
	dsMatrix44_identity(identity);

	// First process all full sets of 4 transforms.
	uint32_t i = 0;
	while (i + 4 <= instanceCount)
	{
		const dsParticleEmitter* emitter =
			dsSceneParticleNode_getEmitterForInstance(instances[i++]);
		const dsMatrix44f* world0 = emitter ? &emitter->transform : &identity;
		emitter = dsSceneParticleNode_getEmitterForInstance(instances[i++]);
		const dsMatrix44f* world1 = emitter ? &emitter->transform : &identity;
		emitter = dsSceneParticleNode_getEmitterForInstance(instances[i++]);
		const dsMatrix44f* world2 = emitter ? &emitter->transform : &identity;
		emitter = dsSceneParticleNode_getEmitterForInstance(instances[i++]);
		const dsMatrix44f* world3 = emitter ? &emitter->transform : &identity;

		ParticleTransform transforms[4];
		computeTransformsFMA(
			transforms, &viewMatrix, &projectionMatrix, world0, world1, world2, world3);

		*(ParticleTransform*)(data) = transforms[0];
		data += stride;
		*(ParticleTransform*)(data) = transforms[1];
		data += stride;
		*(ParticleTransform*)(data) = transforms[2];
		data += stride;
		*(ParticleTransform*)(data) = transforms[3];
		data += stride;
	}

	// Then process any remainder.
	uint32_t rem = instanceCount - i;
	if (rem > 0)
	{
		const dsParticleEmitter* emitter =
			dsSceneParticleNode_getEmitterForInstance(instances[i++]);
		const dsMatrix44f* world0 = emitter ? &emitter->transform : &identity;
		emitter =
			i < instanceCount ? dsSceneParticleNode_getEmitterForInstance(instances[i++]) : NULL;
		const dsMatrix44f* world1 = emitter ? &emitter->transform : &identity;
		emitter =
			i < instanceCount ? dsSceneParticleNode_getEmitterForInstance(instances[i++]) : NULL;
		const dsMatrix44f* world2 = emitter ? &emitter->transform : &identity;
		emitter =
			i < instanceCount ? dsSceneParticleNode_getEmitterForInstance(instances[i++]) : NULL;
		const dsMatrix44f* world3 = emitter ? &emitter->transform : &identity;

		ParticleTransform transforms[4];
		computeTransformsFMA(
			transforms, &viewMatrix, &projectionMatrix, world0, world1, world2, world3);
		for (uint32_t j = 0; j < rem; ++j, data += stride)
			*(ParticleTransform*)(data) = transforms[j];
	}
}
DS_SIMD_END()
#endif

static inline void toMatrix33Vectors(dsVector4f outVectors[3], const dsMatrix33f* inMatrix)
{
	for (unsigned int i = 0; i < 3; ++i)
		*(dsVector3f*)(outVectors + i) = inMatrix->columns[i];
}

static void dsParticleTransformData_populateData(void* userData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount,
	const dsShaderVariableGroupDesc* dataDesc, uint8_t* data, uint32_t stride)
{
	DS_PROFILE_FUNC_START();

	DS_UNUSED(userData);
	DS_UNUSED(dataDesc);
	DS_ASSERT(stride >= sizeof(ParticleTransform));
#if DS_HAS_SIMD
	if (DS_SIMD_ALWAYS_FMA || (dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		dsInstanceTransformData_populateDataFMA(view, instances, instanceCount, data, stride);
	else if (DS_SIMD_ALWAYS_FLOAT4 || (dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		dsInstanceTransformData_populateDataSIMD(view, instances, instanceCount, data, stride);
	else
#endif
	{
		for (uint32_t i = 0; i < instanceCount; ++i, data += stride)
		{
			const dsParticleEmitter* emitter =
				dsSceneParticleNode_getEmitterForInstance(instances[i]);
			// The GPU memory can have some bad properties when accessing from the CPU, so first do
			// all work on CPU memory and copy as one to the GPU buffer.
			ParticleTransform transform;
			if (emitter)
				transform.world = emitter->transform;
			else
				dsMatrix44_identity(transform.world);
			dsMatrix44_affineMul(transform.worldView, view->viewMatrix, transform.world);

			dsMatrix33f tempMatrix33, tempMatrix33Inv;
			dsMatrix33_copy(tempMatrix33, transform.world);
			dsMatrix33f_invert(&tempMatrix33Inv, &tempMatrix33);
			toMatrix33Vectors(transform.localWorldOrientation, &tempMatrix33Inv);

			dsMatrix33_copy(tempMatrix33, transform.worldView);
			dsMatrix33f_invert(&tempMatrix33Inv, &tempMatrix33);
			toMatrix33Vectors(transform.localViewOrientation, &tempMatrix33Inv);

			dsMatrix44_mul(transform.worldViewProj, view->projectionMatrix, transform.worldView);

			*(ParticleTransform*)(data) = transform;
		}
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
		DS_LOG_ERROR(DS_SCENE_LOG_TAG,
			"Particle transform data's shader variable group description must have been created "
			"with dsParticleTransformData_createShaderVariableGroupDesc().");
		return NULL;
	}

	return dsSceneInstanceVariables_create(allocator, resourceManager, transformDesc,
		dsHashString(dsParticleTransformData_typeName),
		&dsParticleTransformData_populateData, NULL, NULL);
}

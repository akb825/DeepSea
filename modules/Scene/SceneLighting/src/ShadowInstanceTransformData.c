/*
 * Copyright 2021-2023 Aaron Barany
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

#include <DeepSea/SceneLighting/ShadowInstanceTransformData.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Scene/ItemLists/InstanceTransformData.h>
#include <DeepSea/Scene/ItemLists/SceneInstanceVariables.h>

#include <DeepSea/SceneLighting/SceneLightShadows.h>

typedef struct ShadowUserData
{
	dsAllocator* allocator;
	const dsSceneLightShadows* shadows;
	uint32_t surface;
} ShadowUserData;

typedef struct InstanceTransform
{
	dsMatrix44f world;
	dsMatrix44f worldView;
	dsVector4f worldViewInvTrans[3];
	dsMatrix44f worldViewProj;
} InstanceTransform;

static void ShadowUserData_destroy(void* userData)
{
	ShadowUserData* shadowData = (ShadowUserData*)userData;
	if (shadowData->allocator)
		DS_VERIFY(dsAllocator_free(shadowData->allocator, shadowData));
}

static void dummyTransformData(uint32_t instanceCount, uint8_t* data, uint32_t stride)
{
	dsMatrix44f identity;
	dsMatrix44_identity(identity);
	dsVector4f identity0 = {{1.0f, 0.0f, 0.0f, 0.0f}};
	dsVector4f identity1 = {{0.0f, 1.0f, 0.0f, 0.0f}};
	dsVector4f identity2 = {{0.0f, 0.0f, 1.0f, 0.0f}};
	for (uint32_t i = 0; i < instanceCount; ++i, data += stride)
	{
		InstanceTransform* transform = (InstanceTransform*)(data);
		transform->world = identity;
		transform->worldView = identity;
		transform->worldViewInvTrans[0] = identity0;
		transform->worldViewInvTrans[1] = identity1;
		transform->worldViewInvTrans[2] = identity2;
		transform->worldViewProj = identity;
	}
}

#if DS_HAS_SIMD
DS_SIMD_START(DS_SIMD_FLOAT4)
static void dsShadowInstanceTransformData_populateDataSIMD(void* userData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount,
	const dsShaderVariableGroupDesc* dataDesc, uint8_t* data, uint32_t stride)
{
	DS_PROFILE_FUNC_START();

	DS_UNUSED(dataDesc);
	DS_ASSERT(stride >= sizeof(InstanceTransform));

	ShadowUserData* shadowData = (ShadowUserData*)userData;
	if (shadowData->surface >= dsSceneLightShadows_getSurfaceCount(shadowData->shadows))
	{
		dummyTransformData(instanceCount, data, stride);
		DS_PROFILE_FUNC_RETURN_VOID();
	}

	const dsMatrix44f* projection = dsSceneLightShadows_getSurfaceProjection(shadowData->shadows,
		shadowData->surface);
	if (!DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, projection))
	{
		dummyTransformData(instanceCount, data, stride);
		DS_PROFILE_FUNC_RETURN_VOID();
	}

	for (uint32_t i = 0; i < instanceCount; ++i, data += stride)
	{
		const dsMatrix44f* world = &instances[i]->transform;
		InstanceTransform* transform = (InstanceTransform*)(data);
		transform->world = *world;
		// Store intermeidate on the stack to avoid reading back from GPU memory.
		dsMatrix44f worldView;
		dsMatrix44f_affineMulSIMD(&worldView, &view->viewMatrix, world);
		transform->worldView = worldView;
		dsMatrix44f_inverseTransposeSIMD(transform->worldViewInvTrans, &worldView);
		dsMatrix44f_mulSIMD(&transform->worldViewProj, projection, &worldView);
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}
DS_SIMD_END()

DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_FMA)
static void dsShadowInstanceTransformData_populateDataFMA(void* userData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount,
	const dsShaderVariableGroupDesc* dataDesc, uint8_t* data, uint32_t stride)
{
	DS_PROFILE_FUNC_START();

	DS_UNUSED(dataDesc);
	DS_ASSERT(stride >= sizeof(InstanceTransform));

	ShadowUserData* shadowData = (ShadowUserData*)userData;
	if (shadowData->surface >= dsSceneLightShadows_getSurfaceCount(shadowData->shadows))
	{
		dummyTransformData(instanceCount, data, stride);
		DS_PROFILE_FUNC_RETURN_VOID();
	}

	const dsMatrix44f* projection = dsSceneLightShadows_getSurfaceProjection(shadowData->shadows,
		shadowData->surface);
	if (!DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, projection))
	{
		dummyTransformData(instanceCount, data, stride);
		DS_PROFILE_FUNC_RETURN_VOID();
	}

	for (uint32_t i = 0; i < instanceCount; ++i, data += stride)
	{
		const dsMatrix44f* world = &instances[i]->transform;
		InstanceTransform* transform = (InstanceTransform*)(data);
		transform->world = *world;
		// Store intermeidate on the stack to avoid reading back from GPU memory.
		dsMatrix44f worldView;
		dsMatrix44f_affineMulFMA(&worldView, &view->viewMatrix, world);
		transform->worldView = worldView;
		dsMatrix44f_inverseTransposeFMA(transform->worldViewInvTrans, &worldView);
		dsMatrix44f_mulFMA(&transform->worldViewProj, projection, &worldView);
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}
DS_SIMD_END()
#endif

static void dsShadowInstanceTransformData_populateData(void* userData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount,
	const dsShaderVariableGroupDesc* dataDesc, uint8_t* data, uint32_t stride)
{
	DS_PROFILE_FUNC_START();

	DS_UNUSED(dataDesc);
	DS_ASSERT(stride >= sizeof(InstanceTransform));

	ShadowUserData* shadowData = (ShadowUserData*)userData;
	if (shadowData->surface >= dsSceneLightShadows_getSurfaceCount(shadowData->shadows))
	{
		dummyTransformData(instanceCount, data, stride);
		DS_PROFILE_FUNC_RETURN_VOID();
	}

	const dsMatrix44f* projection = dsSceneLightShadows_getSurfaceProjection(shadowData->shadows,
		shadowData->surface);
	if (!DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, projection))
	{
		dummyTransformData(instanceCount, data, stride);
		DS_PROFILE_FUNC_RETURN_VOID();
	}

	for (uint32_t i = 0; i < instanceCount; ++i, data += stride)
	{
		const dsMatrix44f* world = &instances[i]->transform;
		InstanceTransform* transform = (InstanceTransform*)(data);
		transform->world = *world;
		// Store intermeidate on the stack to avoid reading back from GPU memory.
		dsMatrix44f worldView;
		dsMatrix44f_affineMul(&worldView, &view->viewMatrix, world);
		transform->worldView = worldView;

		dsMatrix33f worldViewInvTrans;
		dsMatrix44f_inverseTranspose(&worldViewInvTrans, &worldView);
		for (unsigned int i = 0; i < 3; ++i)
		{
			*(dsVector3f*)(transform->worldViewInvTrans + i) = worldViewInvTrans.columns[i];
			transform->worldViewInvTrans[i].w = 0;
		}

		dsMatrix44f_mul(&transform->worldViewProj, projection, &worldView);
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}

const char* const dsShadowInstanceTransformData_typeName = "ShadowInstanceTransformData";

dsSceneInstanceData* dsShadowInstanceTransformData_create(dsAllocator* allocator,
	dsAllocator* resourceAllocator, dsResourceManager* resourceManager,
	const dsSceneLightShadows* shadows, uint32_t surface,
	const dsShaderVariableGroupDesc* transformDesc)
{
	if (!allocator || !shadows || surface > DS_MAX_SCENE_LIGHT_SHADOWS_SURFACES || !transformDesc)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!dsInstanceTransformData_isShaderVariableGroupCompatible(transformDesc))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG,
			"Shadow instance transform data's shader variable group description must have been "
			"created with dsInstanceTransformData_createShaderVariableGroupDesc().");
		return NULL;
	}

	dsPopulateSceneInstanceVariablesFunction populateFunc;
#if DS_HAS_SIMD
	if (DS_SIMD_ALWAYS_FMA || (dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		populateFunc = &dsShadowInstanceTransformData_populateDataFMA;
	else if (DS_SIMD_ALWAYS_FLOAT4 || (dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		populateFunc = &dsShadowInstanceTransformData_populateDataSIMD;
	else
#endif
		populateFunc = &dsShadowInstanceTransformData_populateData;

	ShadowUserData* userData = DS_ALLOCATE_OBJECT(allocator, ShadowUserData);
	if (!userData)
		return NULL;

	userData->allocator = dsAllocator_keepPointer(allocator);
	userData->shadows = shadows;
	userData->surface = surface;
	return dsSceneInstanceVariables_create(allocator, resourceAllocator, resourceManager,
		transformDesc, dsHashString(dsInstanceTransformData_typeName), populateFunc, userData,
		&ShadowUserData_destroy);
}

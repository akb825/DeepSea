/*
 * Copyright 2019-2024 Aaron Barany
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

#include <DeepSea/Scene/ItemLists/InstanceTransformData.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>

#include <DeepSea/Scene/ItemLists/SceneInstanceVariables.h>
#include <DeepSea/Scene/Types.h>

#include <string.h>

static dsShaderVariableElement elements[] =
{
	{"world", dsMaterialType_Mat4, 0},
	{"worldView", dsMaterialType_Mat4, 0},
	{"worldViewInvTrans", dsMaterialType_Mat3, 0},
	{"worldViewProj", dsMaterialType_Mat4, 0}
};

typedef struct InstanceTransform
{
	dsMatrix44f world;
	dsMatrix44f worldView;
	dsVector4f worldViewInvTrans[3];
	dsMatrix44f worldViewProj;
} InstanceTransform;

#if DS_HAS_SIMD
DS_SIMD_START(DS_SIMD_FLOAT4)
static void dsInstanceTransformData_populateDataSIMD(void* userData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount,
	const dsShaderVariableGroupDesc* dataDesc, uint8_t* data, uint32_t stride)
{
	DS_PROFILE_FUNC_START();

	DS_UNUSED(userData);
	DS_UNUSED(dataDesc);
	DS_ASSERT(stride >= sizeof(InstanceTransform));
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
		dsMatrix44f_mulSIMD(&transform->worldViewProj, &view->projectionMatrix, &worldView);
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}
DS_SIMD_END()

DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_FMA)
static void dsInstanceTransformData_populateDataFMA(void* userData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount,
	const dsShaderVariableGroupDesc* dataDesc, uint8_t* data, uint32_t stride)
{
	DS_PROFILE_FUNC_START();

	DS_UNUSED(userData);
	DS_UNUSED(dataDesc);
	DS_ASSERT(stride >= sizeof(InstanceTransform));
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
		dsMatrix44f_mulFMA(&transform->worldViewProj, &view->projectionMatrix, &worldView);
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}
DS_SIMD_END()
#endif

static void dsInstanceTransformData_populateData(void* userData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount,
	const dsShaderVariableGroupDesc* dataDesc, uint8_t* data, uint32_t stride)
{
	DS_PROFILE_FUNC_START();

	DS_UNUSED(userData);
	DS_UNUSED(dataDesc);
	DS_ASSERT(stride >= sizeof(InstanceTransform));
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

		dsMatrix44f_mul(&transform->worldViewProj, &view->projectionMatrix, &worldView);
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}

#if DS_HAS_SIMD
static dsSceneInstanceVariablesType instanceVariablesTypeSIMD =
{
	&dsInstanceTransformData_populateDataSIMD
};

static dsSceneInstanceVariablesType instanceVariablesTypeFMA =
{
	&dsInstanceTransformData_populateDataFMA
};
#endif

static dsSceneInstanceVariablesType instanceVariablesType =
{
	&dsInstanceTransformData_populateData
};

const char* const dsInstanceTransformData_typeName = "InstanceTransformData";

dsShaderVariableGroupDesc* dsInstanceTransformData_createShaderVariableGroupDesc(
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

bool dsInstanceTransformData_isShaderVariableGroupCompatible(
	const dsShaderVariableGroupDesc* transformDesc)
{
	return transformDesc &&
		dsShaderVariableGroup_areElementsEqual(elements, DS_ARRAY_SIZE(elements),
			transformDesc->elements, transformDesc->elementCount);
}

dsSceneInstanceData* dsInstanceTransformData_create(dsAllocator* allocator,
	dsAllocator* resourceAllocator, dsResourceManager* resourceManager,
	const dsShaderVariableGroupDesc* transformDesc)
{
	if (!allocator || !transformDesc)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!dsInstanceTransformData_isShaderVariableGroupCompatible(transformDesc))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG,
			"Instance transform data's shader variable group description must have been created "
			"with dsInstanceTransformData_createShaderVariableGroupDesc().");
		return NULL;
	}

	const dsSceneInstanceVariablesType* type;
#if DS_HAS_SIMD
	if (DS_SIMD_ALWAYS_FMA || (dsHostSIMDFeatures & dsSIMDFeatures_FMA))
		type = &instanceVariablesTypeFMA;
	else if (DS_SIMD_ALWAYS_FLOAT4 || (dsHostSIMDFeatures & dsSIMDFeatures_Float4))
		type = &instanceVariablesTypeSIMD;
	else
#endif
		type = &instanceVariablesType;
	return dsSceneInstanceVariables_create(allocator, resourceAllocator, resourceManager,
		transformDesc, dsUniqueNameID_create(dsInstanceTransformData_typeName), type, NULL);
}

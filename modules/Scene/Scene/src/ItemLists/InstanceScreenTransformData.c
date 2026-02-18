/*
 * Copyright 2026 Aaron Barany
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

#include <DeepSea/Scene/ItemLists/InstanceScreenTransformData.h>

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
	{"worldProj", dsMaterialType_Mat4, 0}
};

typedef struct InstanceScreenTransform
{
	dsMatrix44f world;
	dsMatrix44f worldProj;
} InstanceScreenTransform;

#if DS_HAS_SIMD
DS_SIMD_START(DS_SIMD_FLOAT4)
static void dsInstanceScreenTransformData_populateDataSIMD(void* userData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount,
	const dsShaderVariableGroupDesc* dataDesc, uint8_t* data, uint32_t stride)
{
	DS_PROFILE_FUNC_START();

	DS_UNUSED(userData);
	DS_UNUSED(dataDesc);
	DS_ASSERT(stride >= sizeof(InstanceScreenTransform));
	for (uint32_t i = 0; i < instanceCount; ++i, data += stride)
	{
		const dsMatrix44f* world = &instances[i]->transform;
		InstanceScreenTransform* transform = (InstanceScreenTransform*)(data);
		transform->world = *world;
		dsMatrix44f_mulSIMD(&transform->worldProj, &view->screenProjectionMatrix, world);
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}
DS_SIMD_END()

DS_SIMD_START(DS_SIMD_FLOAT4,DS_SIMD_FMA)
static void dsInstanceScreenTransformData_populateDataFMA(void* userData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount,
	const dsShaderVariableGroupDesc* dataDesc, uint8_t* data, uint32_t stride)
{
	DS_PROFILE_FUNC_START();

	DS_UNUSED(userData);
	DS_UNUSED(dataDesc);
	DS_ASSERT(stride >= sizeof(InstanceScreenTransform));
	for (uint32_t i = 0; i < instanceCount; ++i, data += stride)
	{
		const dsMatrix44f* world = &instances[i]->transform;
		InstanceScreenTransform* transform = (InstanceScreenTransform*)(data);
		transform->world = *world;
		dsMatrix44f_mulFMA(&transform->worldProj, &view->screenProjectionMatrix, world);
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}
DS_SIMD_END()
#endif

static void dsInstanceScreenTransformData_populateData(void* userData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount,
	const dsShaderVariableGroupDesc* dataDesc, uint8_t* data, uint32_t stride)
{
	DS_PROFILE_FUNC_START();

	DS_UNUSED(userData);
	DS_UNUSED(dataDesc);
	DS_ASSERT(stride >= sizeof(InstanceScreenTransform));
	for (uint32_t i = 0; i < instanceCount; ++i, data += stride)
	{
		const dsMatrix44f* world = &instances[i]->transform;
		InstanceScreenTransform* transform = (InstanceScreenTransform*)(data);
		transform->world = *world;
		dsMatrix44f_mul(&transform->worldProj, &view->screenProjectionMatrix, world);
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}

#if DS_HAS_SIMD
static dsSceneInstanceVariablesType instanceVariablesTypeSIMD =
{
	&dsInstanceScreenTransformData_populateDataSIMD
};

static dsSceneInstanceVariablesType instanceVariablesTypeFMA =
{
	&dsInstanceScreenTransformData_populateDataFMA
};
#endif

static dsSceneInstanceVariablesType instanceVariablesType =
{
	&dsInstanceScreenTransformData_populateData
};

const char* const dsInstanceScreenTransformData_typeName = "InstanceScreenTransformData";
const char* const dsInstanceScreenTransformData_uniformName = "dsInstanceScreenTransformData";

dsShaderVariableGroupDesc* dsInstanceScreenTransformData_createShaderVariableGroupDesc(
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

bool dsInstanceScreenTransformData_isShaderVariableGroupCompatible(
	const dsShaderVariableGroupDesc* transformDesc)
{
	return transformDesc &&
		dsShaderVariableGroup_areElementsEqual(elements, DS_ARRAY_SIZE(elements),
			transformDesc->elements, transformDesc->elementCount);
}

dsSceneInstanceData* dsInstanceScreenTransformData_create(dsAllocator* allocator,
	dsAllocator* resourceAllocator, dsResourceManager* resourceManager,
	const dsShaderVariableGroupDesc* transformDesc)
{
	if (!allocator || !transformDesc)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!dsInstanceScreenTransformData_isShaderVariableGroupCompatible(transformDesc))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG,
			"Instance screen transform data's shader variable group description must have been "
			"created with dsInstanceScreenTransformData_createShaderVariableGroupDesc().");
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
		transformDesc, dsUniqueNameID_create(dsInstanceScreenTransformData_uniformName), type,
		NULL);
}

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

#include <DeepSea/SceneVectorDraw/InstanceDiscardBoundsData.h>

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

#include <DeepSea/SceneVectorDraw/SceneDiscardBoundsNode.h>

#include <float.h>

static dsShaderVariableElement elements[] =
{
	{"discardWorldProjInv", dsMaterialType_Mat4, 0},
	{"discardBounds", dsMaterialType_Vec4, 0}
};

typedef struct InstanceDiscardBounds
{
	dsMatrix44f discardWorldProjInv;
	dsVector4f discardBounds;
} InstanceDiscardBounds;

// NOTE: Expect that providing guaranteed SIMD and FMA versions isn't worth it given the comparative
// cost of getting the bounds and transform for the instance in the scene graph sub-tree.
static void dsInstanceDiscardBoundsData_populateData(void* userData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount,
	const dsShaderVariableGroupDesc* dataDesc, uint8_t* data, uint32_t stride)
{
	DS_PROFILE_FUNC_START();

	DS_UNUSED(userData);
	DS_UNUSED(dataDesc);
	DS_ASSERT(stride >= sizeof(InstanceDiscardBounds));
	for (uint32_t i = 0; i < instanceCount; ++i, data += stride)
	{
		dsMatrix44f worldTransform;
		const dsAlignedBox2f* discardBounds = dsSceneDiscardBoundsNode_getDiscardBoundsForInstance(
			&worldTransform, instances[i]);

		InstanceDiscardBounds* instanceData = (InstanceDiscardBounds*)(data);
		if (discardBounds)
		{
			dsMatrix44f worldProj;
			dsMatrix44f_mul(&worldProj, &view->screenProjectionMatrix,  &worldTransform);
			// Screen projection is orthographic, so still an affine transform.
			dsMatrix44f_affineInvert(&instanceData->discardWorldProjInv, &worldProj);
			instanceData->discardBounds.x = discardBounds->min.x;
			instanceData->discardBounds.y = discardBounds->min.y;
			instanceData->discardBounds.z = discardBounds->max.x;
			instanceData->discardBounds.w = discardBounds->max.y;
		}
		else
		{
			dsMatrix44_identity(instanceData->discardWorldProjInv);
			instanceData->discardBounds.x = -FLT_MAX;
			instanceData->discardBounds.y = -FLT_MAX;
			instanceData->discardBounds.z = FLT_MAX;
			instanceData->discardBounds.w = FLT_MAX;
		}
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}

static dsSceneInstanceVariablesType instanceVariablesType =
{
	&dsInstanceDiscardBoundsData_populateData
};

const char* const dsInstanceDiscardBoundsData_typeName = "InstanceDiscardBoundsData";
const char* const dsInstanceDiscardBoundsData_uniformName = "dsInstanceDiscardBoundsData";

dsShaderVariableGroupDesc* dsInstanceDiscardBoundsData_createShaderVariableGroupDesc(
	dsResourceManager* resourceManager, dsAllocator* allocator)
{
	if (!resourceManager)
	{
		errno = EINVAL;
		return NULL;
	}

	return dsShaderVariableGroupDesc_create(
		resourceManager, allocator, elements, DS_ARRAY_SIZE(elements));
}

bool dsInstanceDiscardBoundsData_isShaderVariableGroupCompatible(
	const dsShaderVariableGroupDesc* discardBoundsDesc)
{
	return discardBoundsDesc &&
		dsShaderVariableGroup_areElementsEqual(elements, DS_ARRAY_SIZE(elements),
			discardBoundsDesc->elements, discardBoundsDesc->elementCount);
}

dsSceneInstanceData* dsInstanceDiscardBoundsData_create(dsAllocator* allocator,
	dsAllocator* resourceAllocator, dsResourceManager* resourceManager,
	const dsShaderVariableGroupDesc* transformDesc)
{
	if (!allocator || !transformDesc)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!dsInstanceDiscardBoundsData_isShaderVariableGroupCompatible(transformDesc))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG,
			"Instance discard bounds data's shader variable group description must have been "
			"created with dsInstanceDiscardBoundsData_createShaderVariableGroupDesc().");
		return NULL;
	}

	return dsSceneInstanceVariables_create(allocator, resourceAllocator, resourceManager,
		transformDesc, dsUniqueNameID_create(dsInstanceDiscardBoundsData_uniformName),
		&instanceVariablesType, NULL);
}

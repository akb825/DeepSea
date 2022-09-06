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

#include <DeepSea/SceneLighting/ShadowInstanceTransformData.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Scene/ItemLists/InstanceTransformData.h>
#include <DeepSea/Scene/ItemLists/SceneInstanceVariables.h>
#include <DeepSea/Scene/Nodes//SceneTreeNode.h>

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
	dsMatrix44f worldViewInvTrans;
	dsMatrix44f worldViewProj;
} InstanceTransform;

static void ShadowUserData_destroy(void* userData)
{
	ShadowUserData* shadowData = (ShadowUserData*)userData;
	if (shadowData->allocator)
		DS_VERIFY(dsAllocator_free(shadowData->allocator, shadowData));
}

void dsShadowInstanceTransformData_populateData(void* userData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount,
	const dsShaderVariableGroupDesc* dataDesc, uint8_t* data, uint32_t stride)
{
	DS_UNUSED(dataDesc);
	DS_ASSERT(stride >= sizeof(InstanceTransform));

	ShadowUserData* shadowData = (ShadowUserData*)userData;
	if (shadowData->surface >= dsSceneLightShadows_getSurfaceCount(shadowData->shadows))
		return;

	const dsMatrix44f* projection = dsSceneLightShadows_getSurfaceProjection(shadowData->shadows,
		shadowData->surface);
	if (!DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, projection))
		return;

	for (uint32_t i = 0; i < instanceCount; ++i, data += stride)
	{
		const dsMatrix44f* nodeTransform = dsSceneTreeNode_getTransform(instances[i]);
		DS_ASSERT(nodeTransform);
		InstanceTransform transform;
		transform.world = *nodeTransform;
		dsMatrix44_affineMul(transform.worldView, view->viewMatrix, *nodeTransform);
		dsMatrix44f_inverseTranspose(&transform.worldViewInvTrans, &transform.worldView);
		dsMatrix44_mul(transform.worldViewProj, *projection, transform.worldView);
		*(InstanceTransform*)(data) = transform;
	}
}

const char* const dsShadowInstanceTransformData_typeName = "ShadowInstanceTransformData";

dsSceneInstanceData* dsShadowInstanceTransformData_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsSceneLightShadows* shadows, uint32_t surface,
	const dsShaderVariableGroupDesc* transformDesc)
{
	if (!allocator || !shadows || surface > DS_MAX_SCENE_LIGHT_SHADOWS_SURFACES ||
		!transformDesc)
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

	ShadowUserData* userData = DS_ALLOCATE_OBJECT(allocator, ShadowUserData);
	if (!userData)
		return NULL;

	userData->allocator = dsAllocator_keepPointer(allocator);
	userData->shadows = shadows;
	userData->surface = surface;
	return dsSceneInstanceVariables_create(allocator, resourceManager, transformDesc,
		dsHashString(dsInstanceTransformData_typeName), &dsShadowInstanceTransformData_populateData,
		userData, &ShadowUserData_destroy);
}

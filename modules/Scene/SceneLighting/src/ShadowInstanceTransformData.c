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
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Math/SIMD/Matrix44x4.h>
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
	dsMatrix44fSIMD world;
	dsMatrix44fSIMD worldView;
	dsVector4fSIMD worldViewInvTrans[3];
	dsMatrix44fSIMD worldViewProj;
} InstanceTransform;

static void ShadowUserData_destroy(void* userData)
{
	ShadowUserData* shadowData = (ShadowUserData*)userData;
	if (shadowData->allocator)
		DS_VERIFY(dsAllocator_free(shadowData->allocator, shadowData));
}

#if DS_HAS_SIMD
DS_SIMD_START_FLOAT4()
static void computeTransformsSIMD(InstanceTransform transforms[4], const dsMatrix44x4f *viewMatrix,
	const dsMatrix44x4f* projectionMatrix, const dsMatrix44fSIMD* world0,
	const dsMatrix44fSIMD* world1, const dsMatrix44fSIMD* world2, const dsMatrix44fSIMD* world3)
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

	dsMatrix44x4f worldViewInvTrans;
	dsMatrix44x4f_inverseTranspose(&worldViewInvTrans, &worldView);
	dsMatrix44x4f_store33(transforms[0].worldViewInvTrans, transforms[1].worldViewInvTrans,
		transforms[2].worldViewInvTrans, transforms[3].worldViewInvTrans, &worldViewInvTrans);

	dsMatrix44x4f worldViewProj;
	dsMatrix44x4f_mul(&worldViewProj, projectionMatrix, &worldView);
	dsMatrix44x4f_store(&transforms[0].worldViewProj, &transforms[1].worldViewProj,
		&transforms[2].worldViewProj, &transforms[3].worldViewProj, &worldViewProj);
}

static void dsInstanceTransformData_populateDataSIMD(const dsMatrix44fSIMD* viewMatrix,
	const dsMatrix44fSIMD* projectionMatrix, const dsSceneTreeNode* const* instances,
	uint32_t instanceCount, uint8_t* data, uint32_t stride)
{
	dsMatrix44x4f viewMatrix4;
	dsMatrix44x4f_load(&viewMatrix4, viewMatrix, viewMatrix, viewMatrix, viewMatrix);

	dsMatrix44x4f projectionMatrix4;
	dsMatrix44x4f_load(&projectionMatrix4, projectionMatrix, projectionMatrix,
		projectionMatrix, projectionMatrix);

	// First process all full sets of 4 transforms.
	uint32_t i = 0;
	while (i + 4 <= instanceCount)
	{
		const dsMatrix44fSIMD* world0 = dsSceneTreeNode_getTransform(instances[i++]);
		DS_ASSERT(world0);
		const dsMatrix44fSIMD* world1 = dsSceneTreeNode_getTransform(instances[i++]);
		DS_ASSERT(world1);
		const dsMatrix44fSIMD* world2 = dsSceneTreeNode_getTransform(instances[i++]);
		DS_ASSERT(world2);
		const dsMatrix44fSIMD* world3 = dsSceneTreeNode_getTransform(instances[i++]);
		DS_ASSERT(world3);

		InstanceTransform transforms[4];
		computeTransformsSIMD(
			transforms, &viewMatrix4, &projectionMatrix4, world0, world1, world2, world3);

		*(InstanceTransform*)(data) = transforms[0];
		data += stride;
		*(InstanceTransform*)(data) = transforms[1];
		data += stride;
		*(InstanceTransform*)(data) = transforms[2];
		data += stride;
		*(InstanceTransform*)(data) = transforms[3];
		data += stride;
	}

	// Then process any remainder.
	uint32_t rem = instanceCount - i;
	if (rem > 0)
	{
		const dsMatrix44fSIMD* world0 = dsSceneTreeNode_getTransform(instances[i++]);
		DS_ASSERT(world0);
		const dsMatrix44fSIMD* world1 =
			i < instanceCount ? dsSceneTreeNode_getTransform(instances[i++]) : world0;
		DS_ASSERT(world1);
		const dsMatrix44fSIMD* world2 =
			i < instanceCount ? dsSceneTreeNode_getTransform(instances[i++]) : world0;
		DS_ASSERT(world2);
		DS_ASSERT(i == instanceCount);
		const dsMatrix44fSIMD* world3 = world0;
		DS_ASSERT(world3);

		InstanceTransform transforms[4];
		computeTransformsSIMD(
			transforms, &viewMatrix4, &projectionMatrix4, world0, world1, world2, world3);
		for (uint32_t j = 0; j < rem; ++j, data += stride)
			*(InstanceTransform*)(data) = transforms[j];
	}
}
DS_SIMD_END()

DS_SIMD_START_FMA()
static void computeTransformsFMA(InstanceTransform transforms[4], const dsMatrix44x4f *viewMatrix,
	const dsMatrix44x4f* projectionMatrix, const dsMatrix44fSIMD* world0,
	const dsMatrix44fSIMD* world1, const dsMatrix44fSIMD* world2, const dsMatrix44fSIMD* world3)
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

	dsMatrix44x4f worldViewInvTrans;
	dsMatrix44x4f_inverseTransposeFMA(&worldViewInvTrans, &worldView);
	dsMatrix44x4f_store33(transforms[0].worldViewInvTrans, transforms[1].worldViewInvTrans,
		transforms[2].worldViewInvTrans, transforms[3].worldViewInvTrans, &worldViewInvTrans);

	dsMatrix44x4f worldViewProj;
	dsMatrix44x4f_mulFMA(&worldViewProj, projectionMatrix, &worldView);
	dsMatrix44x4f_store(&transforms[0].worldViewProj, &transforms[1].worldViewProj,
		&transforms[2].worldViewProj, &transforms[3].worldViewProj, &worldViewProj);
}

static void dsInstanceTransformData_populateDataFMA(const dsMatrix44fSIMD* viewMatrix,
	const dsMatrix44fSIMD* projectionMatrix,  const dsSceneTreeNode* const* instances,
	uint32_t instanceCount, uint8_t* data, uint32_t stride)
{
	dsMatrix44x4f viewMatrix4;
	dsMatrix44x4f_load(&viewMatrix4, viewMatrix, viewMatrix, viewMatrix, viewMatrix);

	dsMatrix44x4f projectionMatrix4;
	dsMatrix44x4f_load(&projectionMatrix4, projectionMatrix, projectionMatrix, projectionMatrix,
		projectionMatrix);

	// First process all full sets of 4 transforms.
	uint32_t i = 0;
	while (i + 4 <= instanceCount)
	{
		const dsMatrix44fSIMD* world0 = dsSceneTreeNode_getTransform(instances[i++]);
		DS_ASSERT(world0);
		const dsMatrix44fSIMD* world1 = dsSceneTreeNode_getTransform(instances[i++]);
		DS_ASSERT(world1);
		const dsMatrix44fSIMD* world2 = dsSceneTreeNode_getTransform(instances[i++]);
		DS_ASSERT(world2);
		const dsMatrix44fSIMD* world3 = dsSceneTreeNode_getTransform(instances[i++]);
		DS_ASSERT(world3);

		InstanceTransform transforms[4];
		computeTransformsFMA(
			transforms, &viewMatrix4, &projectionMatrix4, world0, world1, world2, world3);

		*(InstanceTransform*)(data) = transforms[0];
		data += stride;
		*(InstanceTransform*)(data) = transforms[1];
		data += stride;
		*(InstanceTransform*)(data) = transforms[2];
		data += stride;
		*(InstanceTransform*)(data) = transforms[3];
		data += stride;
	}

	// Then process any remainder.
	uint32_t rem = instanceCount - i;
	if (rem > 0)
	{
		const dsMatrix44fSIMD* world0 = dsSceneTreeNode_getTransform(instances[i++]);
		DS_ASSERT(world0);
		const dsMatrix44fSIMD* world1 =
			i < instanceCount ? dsSceneTreeNode_getTransform(instances[i++]) : world0;
		DS_ASSERT(world1);
		const dsMatrix44fSIMD* world2 =
			i < instanceCount ? dsSceneTreeNode_getTransform(instances[i++]) : world0;
		DS_ASSERT(world2);
		DS_ASSERT(i == instanceCount);
		const dsMatrix44fSIMD* world3 = world0;
		DS_ASSERT(world3);

		InstanceTransform transforms[4];
		computeTransformsFMA(
			transforms, &viewMatrix4, &projectionMatrix4, world0, world1, world2, world3);
		for (uint32_t j = 0; j < rem; ++j, data += stride)
			*(InstanceTransform*)(data) = transforms[j];
	}
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
		return;

	const dsMatrix44fSIMD* projection = dsSceneLightShadows_getSurfaceProjection(shadowData->shadows,
		shadowData->surface);
	if (!DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, projection))
		return;

#if DS_HAS_SIMD
	if (DS_SIMD_ALWAYS_FMA || (dsHostSIMDFeatures & dsSIMDFeatures_FMA))
	{
		dsInstanceTransformData_populateDataFMA(&view->viewMatrix, projection, instances,
			instanceCount, data, stride);
	}
	else if (DS_SIMD_ALWAYS_FLOAT4 || (dsHostSIMDFeatures & dsSIMDFeatures_Float4))
	{
		dsInstanceTransformData_populateDataSIMD(&view->viewMatrix, projection, instances,
			instanceCount, data, stride);
	}
	else
#endif
	{
		for (uint32_t i = 0; i < instanceCount; ++i, data += stride)
		{
			const dsMatrix44f* nodeTransform = dsSceneTreeNode_getTransform(instances[i]);
			DS_ASSERT(nodeTransform);
			InstanceTransform transform;
			transform.world = *nodeTransform;
			dsMatrix44_affineMul(transform.worldView, view->viewMatrix, *nodeTransform);

			dsMatrix33f worldViewInvTrans;
			dsMatrix44f_inverseTranspose(&worldViewInvTrans, &transform.worldView);
			for (unsigned int i = 0; i < 3; ++i)
			{
				*(dsVector3f*)(transform.worldViewInvTrans + i) = worldViewInvTrans.columns[i];
				transform.worldViewInvTrans[i].w = 0;
			}

			dsMatrix44_mul(transform.worldViewProj, *projection, transform.worldView);
			*(InstanceTransform*)(data) = transform;
		}
	}

	DS_PROFILE_FUNC_RETURN_VOID();
}

const char* const dsShadowInstanceTransformData_typeName = "ShadowInstanceTransformData";

dsSceneInstanceData* dsShadowInstanceTransformData_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsSceneLightShadows* shadows, uint32_t surface,
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

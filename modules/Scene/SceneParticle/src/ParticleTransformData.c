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

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>

#include <DeepSea/Scene/ItemLists/InstanceTransformData.h>
#include <DeepSea/Scene/ItemLists/SceneInstanceVariables.h>
#include <DeepSea/Scene/Nodes/SceneTreeNode.h>
#include <DeepSea/Scene/Types.h>

#include <DeepSea/SceneParticle/SceneParticleNode.h>

#include <string.h>

typedef struct InstanceTransform
{
	dsMatrix44f world;
	dsMatrix44f worldView;
	dsMatrix44f worldViewInvTrans;
	dsMatrix44f worldViewProj;
} InstanceTransform;

static void dsParticleTransformData_populateData(void* userData, const dsView* view,
	const dsSceneTreeNode* const* instances, uint32_t instanceCount,
	const dsShaderVariableGroupDesc* dataDesc, uint8_t* data, uint32_t stride)
{
	DS_UNUSED(userData);
	DS_UNUSED(dataDesc);
	DS_ASSERT(stride >= sizeof(InstanceTransform));
	for (uint32_t i = 0; i < instanceCount; ++i, data += stride)
	{
		const dsParticleEmitter* emitter = dsSceneParticleNode_getEmitterForInstance(instances[i]);
		// The GPU memory can have some bad properties when accessing from the CPU, so first do all
		// work on CPU memory and copy as one to the GPU buffer.
		InstanceTransform transform;
		if (emitter)
			transform.world = emitter->transform;
		else
			dsMatrix44_identity(transform.world);
		dsMatrix44_affineMul(transform.worldView, view->viewMatrix, transform.world);

		dsMatrix44f inverseWorldView;
		dsMatrix44f_affineInvert(&inverseWorldView, &transform.worldView);
		dsMatrix44_transpose(transform.worldViewInvTrans, inverseWorldView);

		dsMatrix44_mul(transform.worldViewProj, view->projectionMatrix, transform.worldView);

		*(InstanceTransform*)(data) = transform;
	}
}

const char* const dsParticleTransformData_typeName = "InstanceTransformData";

dsSceneInstanceData* dsParticleTransformData_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsShaderVariableGroupDesc* transformDesc)
{
	if (!allocator || !transformDesc)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!dsSceneTransformData_isShaderVariableGroupCompatible(transformDesc))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG,
			"Particle transform data's shader variable group description must have been created "
			"with dsInstanceTransformData_createShaderVariableGroupDesc().");
		return NULL;
	}

	return dsSceneInstanceVariables_create(allocator, resourceManager, transformDesc,
		dsHashString(dsInstanceTransformData_typeName),
		&dsParticleTransformData_populateData, NULL, NULL);
}

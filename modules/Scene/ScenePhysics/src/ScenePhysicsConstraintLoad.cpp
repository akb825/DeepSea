/*
 * Copyright 2024 Aaron Barany
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

#include "ScenePhysicsConstraintLoad.h"

#include "ScenePhysicsTypes.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>
#include <DeepSea/Physics/Flatbuffers/PhysicsFlatbufferHelpers.h>

#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <DeepSea/ScenePhysics/ScenePhysicsConstraint.h>
#include <DeepSea/ScenePhysics/SceneRigidBody.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/ScenePhysicsConstraint_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif


static dsPhysicsActor* findActor(dsPhysicsEngine* engine, void* userData, const char* name)
{
	DS_UNUSED(engine);

	dsSceneLoadScratchData* scratchData = (dsSceneLoadScratchData*)userData;
	dsSceneResourceType type;
	dsCustomSceneResource* resource;
	if (!dsSceneLoadScratchData_findResource(&type, (void**)&resource, scratchData, name) ||
		type != dsSceneResourceType_Custom || resource->type != dsSceneRigidBody_type())
	{
		DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG, "Couldn't find physics actor '%s'.", name);
		errno = ENOTFOUND;
		return NULL;
	}

	return (dsPhysicsActor*)resource->resource;
}

static dsPhysicsConstraint* findConstraint(
	dsPhysicsEngine* engine, void* userData, const char* name)
{
	DS_UNUSED(engine);

	dsSceneLoadScratchData* scratchData = (dsSceneLoadScratchData*)userData;
	dsSceneResourceType type;
	dsCustomSceneResource* resource;
	if (!dsSceneLoadScratchData_findResource(&type, (void**)&resource, scratchData, name) ||
		type != dsSceneResourceType_Custom || resource->type != dsScenePhysicsConstraint_type())
	{
		DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG, "Couldn't find physics constraint '%s'.", name);
		errno = ENOTFOUND;
		return NULL;
	}

	dsScenePhysicsConstraint* constraint = (dsScenePhysicsConstraint*)resource->resource;
	return constraint->constraint;
}

void* dsScenePhysicsConstraint_load(const dsSceneLoadContext*, dsSceneLoadScratchData* scratchData,
	dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScenePhysics::VerifyScenePhysicsConstraintBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG,
			"Invalid scene physics constraint flatbuffer format.");
		return nullptr;
	}

	dsScenePhysicsLoadData* loadData = (dsScenePhysicsLoadData*)userData;
	auto fbSceneConstraint = DeepSeaScenePhysics::GetScenePhysicsConstraint(data);

	auto fbConstraint = fbSceneConstraint->constraint();
	dsPhysicsConstraint* constraint = dsPhysicsConstraint_loadData(loadData->engine,
		loadData->allocator, &findActor, scratchData, &findConstraint, scratchData,
		fbConstraint->data(), fbConstraint->size());
	if (!constraint)
		return nullptr;

	return dsScenePhysicsConstraint_create(allocator, constraint,
		fbSceneConstraint->firstRigidBodyInstance()->c_str(),
		fbSceneConstraint->firstConnectedConstraintInstance()->c_str(),
		fbSceneConstraint->secondRigidBodyInstance()->c_str(),
		fbSceneConstraint->secondConnectedConstraintInstance()->c_str());
}

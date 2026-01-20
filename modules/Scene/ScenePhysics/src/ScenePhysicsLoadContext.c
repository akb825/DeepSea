/*
 * Copyright 2024-2026 Aaron Barany
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

#include <DeepSea/ScenePhysics/ScenePhysicsLoadContext.h>

#include "ScenePhysicsConstraintLoad.h"
#include "ScenePhysicsConstraintNodeLoad.h"
#include "ScenePhysicsListLoad.h"
#include "ScenePhysicsTypes.h"
#include "SceneRigidBodyGroupNodeLoad.h"
#include "SceneRigidBodyNodeLoad.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>
#include <DeepSea/Physics/Shapes/PhysicsShape.h>
#include <DeepSea/Physics/PhysicsEngine.h>
#include <DeepSea/Physics/RigidBody.h>
#include <DeepSea/Physics/RigidBodyTemplate.h>

#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <DeepSea/ScenePhysics/ScenePhysicsConstraint.h>
#include <DeepSea/ScenePhysics/ScenePhysicsConstraintNode.h>
#include <DeepSea/ScenePhysics/ScenePhysicsList.h>
#include <DeepSea/ScenePhysics/ScenePhysicsShape.h>
#include <DeepSea/ScenePhysics/SceneRigidBody.h>
#include <DeepSea/ScenePhysics/SceneRigidBodyGroupNode.h>
#include <DeepSea/ScenePhysics/SceneRigidBodyNode.h>
#include <DeepSea/ScenePhysics/SceneRigidBodyTemplate.h>

static dsPhysicsShape* findShape(dsPhysicsEngine* engine, void* userData, const char* name)
{
	DS_UNUSED(engine);

	dsSceneLoadScratchData* scratchData = (dsSceneLoadScratchData*)userData;
	dsSceneResourceType type;
	dsCustomSceneResource* resource;
	if (!dsSceneLoadScratchData_findResource(&type, (void**)&resource, scratchData, name) ||
		type != dsSceneResourceType_Custom || resource->type != dsScenePhysicsShape_type())
	{
		DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG, "Couldn't find physics shape '%s'.", name);
		errno = ENOTFOUND;
		return NULL;
	}

	return dsPhysicsShape_addRef((dsPhysicsShape*)resource->resource);
}

static void dsScenePhysicsLoadData_destroy(void* userData)
{
	dsScenePhysicsLoadData* loadData = (dsScenePhysicsLoadData*)userData;
	if (loadData->ownsEngine)
		dsPhysicsEngine_destroy(loadData->engine);
	DS_VERIFY(dsAllocator_free(loadData->allocator, loadData));
}

static void* dsScenePhysicsShape_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc)
{
	DS_UNUSED(loadContext);
	DS_UNUSED(allocator);
	DS_UNUSED(resourceAllocator);
	DS_UNUSED(relativePathUserData);
	DS_UNUSED(openRelativePathStreamFunc);
	DS_UNUSED(closeRelativePathStreamFunc);

	dsScenePhysicsLoadData* loadData = (dsScenePhysicsLoadData*)userData;
	return dsPhysicsShape_loadData(loadData->engine, loadData->allocator, &findShape, scratchData,
		data, dataSize);
}

static bool dsScenePhysicsShape_destroyResource(void* resource)
{
	dsPhysicsShape* shape = (dsPhysicsShape*)resource;
	dsPhysicsShape_freeRef(shape);
	return true;
}

static void* dsSceneRigidBody_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc)
{
	DS_UNUSED(loadContext);
	DS_UNUSED(allocator);
	DS_UNUSED(resourceAllocator);
	DS_UNUSED(relativePathUserData);
	DS_UNUSED(openRelativePathStreamFunc);
	DS_UNUSED(closeRelativePathStreamFunc);

	dsScenePhysicsLoadData* loadData = (dsScenePhysicsLoadData*)userData;
	// NOTE: No support for rigid body groups for individual rigid bodies.
	return dsRigidBody_loadData(loadData->engine, loadData->allocator, NULL, NULL,
		loadData->canCollisionGroupsCollideFunc, NULL, NULL, &findShape, scratchData, data,
		dataSize);
}

static void* dsSceneRigidBodyTemplate_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc)
{
	DS_UNUSED(loadContext);
	DS_UNUSED(allocator);
	DS_UNUSED(resourceAllocator);
	DS_UNUSED(relativePathUserData);
	DS_UNUSED(openRelativePathStreamFunc);
	DS_UNUSED(closeRelativePathStreamFunc);

	dsScenePhysicsLoadData* loadData = (dsScenePhysicsLoadData*)userData;
	// NOTE: No support for rigid body groups for individual rigid bodies.
	return dsRigidBodyTemplate_loadData(loadData->engine, loadData->allocator,
		loadData->canCollisionGroupsCollideFunc, &findShape, scratchData, data, dataSize);
}

static bool dsSceneRigidBodyTemplate_destroyResource(void* resource)
{
	dsRigidBodyTemplate* rigidBodyTemplate = (dsRigidBodyTemplate*)resource;
	dsRigidBodyTemplate_destroy(rigidBodyTemplate);
	return true;
}

bool dsScenePhysicsLoadConext_registerTypes(dsSceneLoadContext* loadContext, dsAllocator* allocator,
	dsPhysicsEngine* physicsEngine, bool takeOwnership, dsThreadPool* threadPool,
	dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc)
{
	if (!loadContext || !physicsEngine)
	{
		if (takeOwnership)
			dsPhysicsEngine_destroy(physicsEngine);
		errno = EINVAL;
		return false;
	}

	if (!allocator)
		allocator = physicsEngine->allocator;

	dsScenePhysicsLoadData* loadData = DS_ALLOCATE_OBJECT(allocator, dsScenePhysicsLoadData);
	if (!loadData)
	{
		if (takeOwnership)
			dsPhysicsEngine_destroy(physicsEngine);
		return false;
	}

	loadData->allocator = dsAllocator_keepPointer(allocator);
	loadData->engine = physicsEngine;
	loadData->threadPool = threadPool;
	loadData->canCollisionGroupsCollideFunc = canCollisionGroupsCollideFunc;
	loadData->ownsEngine = takeOwnership;
	// Item list type is responsible for cleaning up load data.
	if (!dsSceneLoadContext_registerItemListType(loadContext, dsScenePhysicsList_typeName,
			&dsScenePhysicsList_load, physicsEngine, &dsScenePhysicsLoadData_destroy))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerCustomResourceType(loadContext, dsScenePhysicsShape_typeName,
			dsScenePhysicsShape_type(), &dsScenePhysicsShape_load,
			&dsScenePhysicsShape_destroyResource, loadData, NULL, 0))
	{
		return false;
	}

	// Destroy function is directly compatible.
	if (!dsSceneLoadContext_registerCustomResourceType(loadContext, dsSceneRigidBody_typeName,
			dsSceneRigidBody_type(), &dsSceneRigidBody_load,
			(dsDestroyCustomSceneResourceFunction)&dsRigidBody_destroy, loadData, NULL, 0))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerCustomResourceType(loadContext,
			dsSceneRigidBodyTemplate_typeName, dsSceneRigidBodyTemplate_type(),
			&dsSceneRigidBodyTemplate_load, &dsSceneRigidBodyTemplate_destroyResource, loadData,
			NULL, 0))
	{
		return false;
	}

	// Destroy function is directly compatible.
	if (!dsSceneLoadContext_registerCustomResourceType(loadContext,
			dsScenePhysicsConstraint_typeName, dsScenePhysicsConstraint_type(),
			&dsScenePhysicsConstraint_load,
			(dsDestroyCustomSceneResourceFunction)&dsPhysicsConstraint_destroy, loadData, NULL, 0))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerNodeType(loadContext, dsSceneRigidBodyGroupNode_typeName,
			&dsSceneRigidBodyGroupNode_load, NULL, NULL))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerNodeType(loadContext, dsSceneRigidBodyNode_typeName,
			&dsSceneRigidBodyNode_load, NULL, NULL))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerNodeType(loadContext, "UniqueRigidBodyNode",
			&dsSceneRigidBodyNode_loadUnique, NULL, NULL))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerNodeType(loadContext, "RigidBodyTemplateNode",
			&dsSceneRigidBodyNode_loadTemplate, NULL, NULL))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerNodeType(loadContext, dsScenePhysicsConstraintNode_typeName,
			&dsScenePhysicsConstraintNode_load, NULL, NULL))
	{
		return false;
	}

	return true;
}

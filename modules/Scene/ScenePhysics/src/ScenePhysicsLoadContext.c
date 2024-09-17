#include <DeepSea/ScenePhysics/ScenePhysicsLoadContext.h>

#include "ScenePhysicsListLoad.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Physics/PhysicsEngine.h>

#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <DeepSea/ScenePhysics/ScenePhysicsList.h>

static void dsScenePhysicsListData_destroy(void* userData)
{
	dsScenePhysicsListData* listData = (dsScenePhysicsListData*)userData;
	if (listData->ownsEngine)
		dsPhysicsEngine_destroy(listData->engine);
	DS_VERIFY(dsAllocator_free(listData->allocator, listData));
}

bool dsScenePhysicsLoadConext_registerTypes(dsSceneLoadContext* loadContext, dsAllocator* allocator,
	dsPhysicsEngine* physicsEngine, bool takeOwnership, dsThreadPool* threadPool)
{
	if (!loadContext || !allocator || !physicsEngine)
	{
		if (takeOwnership)
			dsPhysicsEngine_destroy(physicsEngine);
		errno = EINVAL;
		return false;
	}

	dsScenePhysicsListData* listData = DS_ALLOCATE_OBJECT(allocator, dsScenePhysicsListData);
	if (!listData)
	{
		if (takeOwnership)
			dsPhysicsEngine_destroy(physicsEngine);
		return false;
	}

	listData->allocator = dsAllocator_keepPointer(allocator);
	listData->engine = physicsEngine;
	listData->threadPool = threadPool;
	listData->ownsEngine = takeOwnership;
	if (!dsSceneLoadContext_registerItemListType(loadContext, dsScenePhysicsList_typeName,
			&dsScenePhysicsList_load, physicsEngine, &dsScenePhysicsListData_destroy))
	{
		return false;
	}

	return true;
}

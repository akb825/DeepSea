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

#include "ScenePhysicsListLoad.h"

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/PhysicsScene.h>

#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/ItemLists/SceneInstanceData.h>
#include <DeepSea/Scene/SceneLoadContext.h>

#include <DeepSea/ScenePhysics/ScenePhysicsList.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/ScenePhysicsList_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

dsSceneItemList* dsScenePhysicsList_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const char* name, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScenePhysics::VerifyPhysicsListBuffer(verifier))
	{
		DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG, "Invalid scene physics list flatbuffer format.");
		errno = EFORMAT;
		return nullptr;
	}

	auto fbPhysicsList = DeepSeaScenePhysics::GetPhysicsList(data);

	dsPhysicsSceneSettings settings;
	settings.maxStaticBodies = fbPhysicsList->maxStaticBodoes();
	settings.maxDynamicBodies = fbPhysicsList->maxDynamicBodies();
	settings.maxConstrainedBodyGroups = fbPhysicsList->maxConstrainedBodyGroups();
	settings.maxStaticShapes = fbPhysicsList->maxStaticShapes();
	settings.maxDynamicShapes = fbPhysicsList->maxDynamicShapes();
	settings.maxConstraints = fbPhysicsList->maxConstraints();
	settings.maxBodyCollisionPairs = fbPhysicsList->maxBodyCollisionPairs();
	settings.maxContactPoints = fbPhysicsList->maxContactPoints();
	settings.gravity = DeepSeaScene::convert(*fbPhysicsList->gravity());
	settings.multiThreadedModifications = fbPhysicsList->multiThreadedModifications();

	dsScenePhysicsListData* listData = (dsScenePhysicsListData*)userData;
	dsPhysicsScene* physicsScene = dsPhysicsScene_create(
		listData->engine, allocator, &settings, listData->threadPool);
	if (!physicsScene)
		return nullptr;

	return dsScenePhysicsList_create(allocator, name, physicsScene, true,
		fbPhysicsList->targetStepTime());
}

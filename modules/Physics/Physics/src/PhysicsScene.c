/*
 * Copyright 2023-2024 Aaron Barany
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

#include <DeepSea/Physics/PhysicsScene.h>

#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

dsPhysicsScene* dsPhysicsScene_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	const dsPhysicsSceneLimits* limits, dsThreadPool* threadPool)
{
	if (!engine || (!allocator && !engine->allocator) || !limits || !engine->createSceneFunc ||
		!engine->destroySceneFunc)
	{
		errno = EINVAL;
		return NULL;
	}

	return engine->createSceneFunc(engine, allocator, limits, threadPool);
}

bool dsPhysicsScene_setCombineFrictionFunction(dsPhysicsScene* scene,
	dsCombineFrictionFunction combineFunc)
{
	if (!scene || !scene->engine || !scene->engine->setSceneCombineFrictionFunc || !combineFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = scene->engine;
	return engine->setSceneCombineFrictionFunc(engine, scene, combineFunc);
}

bool dsPhysicsScene_setCombineRestitutionFunction(dsPhysicsScene* scene,
	dsCombineRestitutionFunction combineFunc)
{
	if (!scene || !scene->engine || !scene->engine->setSceneCombineRestitutionFunc || !combineFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = scene->engine;
	return engine->setSceneCombineRestitutionFunc(engine, scene, combineFunc);
}

bool dsPhysicsScene_addRigidBodies(dsPhysicsScene* scene,
	dsRigidBody* const* rigidBodies, uint32_t rigidBodyCount, bool activate)
{
	DS_PROFILE_FUNC_START();
	if (!scene || !scene->engine || !scene->engine->addSceneRigidBodiesFunc ||
		(!rigidBodies && rigidBodyCount > 0))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (rigidBodyCount == 0)
		DS_PROFILE_FUNC_RETURN(true);

	for (uint32_t i = 0; i < rigidBodyCount; ++i)
	{
		dsRigidBody* rigidBody = rigidBodies[i];
		if (!rigidBody)
		{
			errno = EINVAL;
			DS_PROFILE_FUNC_RETURN(false);
		}

		if (rigidBody->group)
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
				"Cannot add a rigid body to a scene when associated with a group.");
			errno = EPERM;
			DS_PROFILE_FUNC_RETURN(false);
		}

		// Assume that the rigid bodywon't be added/removed across threads for this sanity check.
		if (((dsPhysicsActor*)rigidBody)->scene)
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
				"Cannot add a rigid body to a scene when already associated with a scene.");
			errno = EPERM;
			DS_PROFILE_FUNC_RETURN(false);
		}
	}

	dsPhysicsEngine* engine = scene->engine;
	bool success = engine->addSceneRigidBodiesFunc(
		engine, scene, rigidBodies, rigidBodyCount, activate);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsPhysicsScene_removeRigidBodies(dsPhysicsScene* scene, dsRigidBody* const* rigidBodies,
	uint32_t rigidBodyCount)
{
	DS_PROFILE_FUNC_START();
	if (!scene || !scene->engine || !scene->engine->removeSceneRigidBodiesFunc ||
		(!rigidBodies && rigidBodyCount > 0))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (rigidBodyCount == 0)
		DS_PROFILE_FUNC_RETURN(true);

	for (uint32_t i = 0; i < rigidBodyCount; ++i)
	{
		dsRigidBody* rigidBody = rigidBodies[i];
		if (!rigidBody)
		{
			errno = EINVAL;
			DS_PROFILE_FUNC_RETURN(false);
		}

		if (rigidBody->group)
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
				"Cannot remove a rigid body from a scene when associated with a group.");
			errno = EPERM;
			DS_PROFILE_FUNC_RETURN(false);
		}

		// Assume that the rigid bodywon't be added/removed across threads for this sanity check.
		if (((dsPhysicsActor*)rigidBody)->scene != scene)
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
				"Cannot remove a rigid body from a scene it's not associated with.");
			errno = EPERM;
			DS_PROFILE_FUNC_RETURN(false);
		}
	}

	dsPhysicsEngine* engine = scene->engine;
	bool success = engine->removeSceneRigidBodiesFunc(engine, scene, rigidBodies, rigidBodyCount);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsPhysicsScene_addRigidBodyGroup(dsPhysicsScene* scene, dsRigidBodyGroup* group, bool activate)
{
	DS_PROFILE_FUNC_START();
	if (!scene || !scene->engine || !scene->engine->addSceneRigidBodyGroupFunc || !group)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsPhysicsScene* groupScene;
	DS_ATOMIC_LOAD_PTR(&group->scene, &groupScene);
	if (groupScene)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Cannot add a rigid body group to a scene when already associated with a scene.");
		errno = EPERM;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsPhysicsEngine* engine = scene->engine;
	bool success = engine->addSceneRigidBodyGroupFunc(engine, scene, group, activate);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsPhysicsScene_removeRigidBodyGroup(dsPhysicsScene* scene, dsRigidBodyGroup* group)
{
	DS_PROFILE_FUNC_START();
	if (!scene || !scene->engine || !scene->engine->removeSceneRigidBodyGroupFunc || !group)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsPhysicsScene* groupScene;
	DS_ATOMIC_LOAD_PTR(&group->scene, &groupScene);
	if (groupScene != scene)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Cannot remove a rigid body group from a scene it's not associated with.");
		errno = EPERM;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsPhysicsEngine* engine = scene->engine;
	bool success = engine->removeSceneRigidBodyGroupFunc(engine, scene, group);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsPhysicsScene_setUpdateContactSettingsFunction(dsPhysicsScene* scene,
	dsUpdatePhysicsActorContactPropertiesFunction function, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc)
{
	if (!scene || !scene->engine || !scene->engine->setSceneUpdateContactPropertiesFunc ||
		!function)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = scene->engine;
	if (!engine->setSceneUpdateContactPropertiesFunc(engine, scene, function, userData,
			destroyUserDataFunc))
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return false;
	}

	return true;
}

bool dsPhysicsScene_setContactManifoldAddedFunction(dsPhysicsScene* scene,
	dsPhysicsActorContactManifoldFunction function, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc)
{
	if (!scene || !scene->engine || !scene->engine->setSceneContactManifoldAddedFunc ||
		!function)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = scene->engine;
	if (!engine->setSceneContactManifoldAddedFunc(engine, scene, function, userData,
			destroyUserDataFunc))
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return false;
	}

	return true;
}

bool dsPhysicsScene_setContactManifoldUpdatedFunction(dsPhysicsScene* scene,
	dsPhysicsActorContactManifoldFunction function, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc)
{
	if (!scene || !scene->engine || !scene->engine->setSceneContactManifoldUpdatedFunc ||
		!function)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = scene->engine;
	if (!engine->setSceneContactManifoldUpdatedFunc(engine, scene, function, userData,
			destroyUserDataFunc))
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return false;
	}

	return true;
}

bool dsPhysicsScene_setContactManifoldRemovedFunction(dsPhysicsScene* scene,
	dsPhysicsActorContactManifoldFunction function, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc)
{
	if (!scene || !scene->engine || !scene->engine->setSceneContactManifoldRemovedFunc ||
		!function)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = scene->engine;
	if (!engine->setSceneContactManifoldAddedFunc(engine, scene, function, userData,
			destroyUserDataFunc))
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return false;
	}

	return true;
}

uint32_t dsPhysicsScene_addStepListener(dsPhysicsScene* scene,
	dsOnPhysicsSceneStepFunction function, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc)
{
	if (!scene || !scene->engine || !scene->engine->addSceneStepListenerFunc || !function)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return DS_INVALID_PHYSICS_ID;
	}

	dsPhysicsEngine* engine = scene->engine;
	uint32_t listenerID = engine->addSceneStepListenerFunc(
		engine, scene, function, userData, destroyUserDataFunc);
	if (listenerID == DS_INVALID_PHYSICS_ID && destroyUserDataFunc)
		destroyUserDataFunc(userData);
	return listenerID;
}

bool dsPhysicsScene_removeStepListener(dsPhysicsScene* scene, uint32_t listenerID)
{
	if (!scene || !scene->engine || !scene->engine->removeSceneStepListenerFunc ||
		listenerID == DS_INVALID_PHYSICS_ID)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = scene->engine;
	return engine->removeSceneStepListenerFunc(engine, scene, listenerID);
}

bool dsPhysicsScene_setGravity(dsPhysicsScene* scene, const dsVector3f* gravity)
{
	if (!scene || !scene->engine || !scene->engine->setPhysicsSceneGravityFunc || !gravity)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = scene->engine;
	return engine->setPhysicsSceneGravityFunc(engine, scene, gravity);
}

bool dsPhysicsScene_destroy(dsPhysicsScene* scene)
{
	if (!scene)
		return true;

	if (!scene->engine || !scene->engine->destroySceneFunc)
	{
		errno = EINVAL;
		return false;
	}

	return scene->engine->destroySceneFunc(scene->engine, scene);
}

float dsPhysicsScene_defaultCombineFriction(float frictionA, float frictionB);
float dsPhysicsScene_defaultCombineRestitution(float restitutionA, float hardnessA,
	float restitutionB, float hardnessB);
float dsPhysicsScene_combineFriction(const dsPhysicsScene* scene, float frictionA, float frictionB);
float dsPhysicsScene_combineRestitution(const dsPhysicsScene* scene, float restitutionA,
	float hardnessA, float restitutionB, float hardnessB);

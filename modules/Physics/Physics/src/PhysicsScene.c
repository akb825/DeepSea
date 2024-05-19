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

#include <DeepSea/Core/Thread/ReadWriteLock.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

inline static bool isReadLocked(const dsPhysicsScene* scene, const dsPhysicsSceneLock* lock)
{
	return lock->readLock == scene || lock->writeLock == scene;
}

inline static bool isWriteLocked(const dsPhysicsScene* scene, const dsPhysicsSceneLock* lock)
{
	return lock->writeLock == scene;
}

dsPhysicsScene* dsPhysicsScene_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	const dsPhysicsSceneSettings* settings, dsThreadPool* threadPool)
{
	if (!engine || (!allocator && !engine->allocator) || !settings || !engine->createSceneFunc ||
		!engine->destroySceneFunc)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = engine->allocator;

	if (!allocator->freeFunc)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Physics scene allocator must support freeing memory.");
		errno = EINVAL;
		return NULL;
	}

	return engine->createSceneFunc(engine, allocator, settings, threadPool);
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

uint32_t dsPhysicsScene_addPreStepListener(dsPhysicsScene* scene,
	dsOnPhysicsSceneStepFunction function, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc)
{
	if (!scene || !scene->engine || !scene->engine->addScenePreStepListenerFunc || !function)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return DS_INVALID_PHYSICS_ID;
	}

	dsPhysicsEngine* engine = scene->engine;
	uint32_t listenerID = engine->addScenePreStepListenerFunc(
		engine, scene, function, userData, destroyUserDataFunc);
	if (listenerID == DS_INVALID_PHYSICS_ID && destroyUserDataFunc)
		destroyUserDataFunc(userData);
	return listenerID;
}

bool dsPhysicsScene_removePreStepListener(dsPhysicsScene* scene, uint32_t listenerID)
{
	if (!scene || !scene->engine || !scene->engine->removeScenePreStepListenerFunc ||
		listenerID == DS_INVALID_PHYSICS_ID)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = scene->engine;
	return engine->removeScenePreStepListenerFunc(engine, scene, listenerID);
}

uint32_t dsPhysicsScene_addPostStepListener(dsPhysicsScene* scene,
	dsOnPhysicsSceneStepFunction function, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc)
{
	if (!scene || !scene->engine || !scene->engine->addScenePostStepListenerFunc || !function)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return DS_INVALID_PHYSICS_ID;
	}

	dsPhysicsEngine* engine = scene->engine;
	uint32_t listenerID = engine->addScenePostStepListenerFunc(
		engine, scene, function, userData, destroyUserDataFunc);
	if (listenerID == DS_INVALID_PHYSICS_ID && destroyUserDataFunc)
		destroyUserDataFunc(userData);
	return listenerID;
}

bool dsPhysicsScene_removePostStepListener(dsPhysicsScene* scene, uint32_t listenerID)
{
	if (!scene || !scene->engine || !scene->engine->removeScenePostStepListenerFunc ||
		listenerID == DS_INVALID_PHYSICS_ID)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = scene->engine;
	return engine->removeScenePostStepListenerFunc(engine, scene, listenerID);
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

bool dsPhysicsScene_lockRead(dsPhysicsSceneLock* outLock, dsPhysicsScene* scene)
{
	if (!outLock || !scene)
	{
		errno = EINVAL;
		return false;
	}

	if (scene->lock && !dsReadWriteLock_lockRead(scene->lock))
		return false;

	outLock->readLock = scene;
	outLock->writeLock = NULL;
	return true;
}

bool dsPhysicsScene_unlockRead(dsPhysicsSceneLock* outLock, dsPhysicsScene* scene)
{
	if (!outLock || !scene)
	{
		errno = EINVAL;
		return false;
	}

	if (outLock->readLock != scene)
	{
		errno = EPERM;
		return false;
	}

	if (scene->lock && !dsReadWriteLock_unlockRead(scene->lock))
		return false;

	outLock->readLock = outLock->writeLock = NULL;
	return true;
}

bool dsPhysicsScene_lockWrite(dsPhysicsSceneLock* outLock, dsPhysicsScene* scene)
{
	if (!outLock || !scene)
	{
		errno = EINVAL;
		return false;
	}

	if (scene->lock && !dsReadWriteLock_lockWrite(scene->lock))
		return false;

	outLock->readLock = NULL;
	outLock->writeLock = scene;
	return true;
}

bool dsPhysicsScene_unlockWrite(dsPhysicsSceneLock* outLock, dsPhysicsScene* scene)
{
	if (!outLock || !scene)
	{
		errno = EINVAL;
		return false;
	}

	if (outLock->writeLock != scene)
	{
		errno = EPERM;
		return false;
	}

	if (scene->lock && !dsReadWriteLock_unlockWrite(scene->lock))
		return false;

	outLock->readLock = outLock->writeLock = NULL;
	return true;
}

bool dsPhysicsScene_addRigidBodies(dsPhysicsScene* scene,
	dsRigidBody* const* rigidBodies, uint32_t rigidBodyCount, bool activate,
	const dsPhysicsSceneLock* lock)
{
	DS_PROFILE_FUNC_START();
	if (!scene || !scene->engine || !scene->engine->addSceneRigidBodiesFunc ||
		(!rigidBodies && rigidBodyCount > 0) || !lock)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!isWriteLocked(scene, lock))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Physics scene must have been locked for writing before adding rigid bodies.");
		errno = EPERM;
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

		// Assume that the rigid body won't be added/removed across threads for this sanity check.
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
	uint32_t rigidBodyCount, const dsPhysicsSceneLock* lock)
{
	DS_PROFILE_FUNC_START();
	if (!scene || !scene->engine || !scene->engine->removeSceneRigidBodiesFunc ||
		(!rigidBodies && rigidBodyCount > 0) || !lock)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!isWriteLocked(scene, lock))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Physics scene must have been locked for writing before removing rigid bodies.");
		errno = EPERM;
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

		// Assume that the rigid body won't be added/removed across threads for this sanity check.
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

bool dsPhysicsScene_addRigidBodyGroup(dsPhysicsScene* scene, dsRigidBodyGroup* group, bool activate,
	const dsPhysicsSceneLock* lock)
{
	DS_PROFILE_FUNC_START();
	if (!scene || !scene->engine || !scene->engine->addSceneRigidBodyGroupFunc || !group || !lock)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!isWriteLocked(scene, lock))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Physics scene must have been locked for writing before adding rigid body groups.");
		errno = EPERM;
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

bool dsPhysicsScene_removeRigidBodyGroup(dsPhysicsScene* scene, dsRigidBodyGroup* group,
	const dsPhysicsSceneLock* lock)
{
	DS_PROFILE_FUNC_START();
	if (!scene || !scene->engine || !scene->engine->removeSceneRigidBodyGroupFunc || !group ||
		!lock)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!isWriteLocked(scene, lock))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Physics scene must have been locked for writing before removing rigid body groups.");
		errno = EPERM;
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

uint32_t dsPhysicsScene_getActors(dsPhysicsActor** outActors, const dsPhysicsScene* scene,
	uint32_t firstIndex, uint32_t count, const dsPhysicsSceneLock* lock)
{
	if ((!outActors && count > 0) || !scene || !scene->engine ||
		!scene->engine->getSceneActorsFunc || !lock)
	{
		errno = EINVAL;
		return DS_INVALID_PHYSICS_ID;
	}

	if (!isReadLocked(scene, lock))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Physics scene must have been locked for reading or writing before querying actors.");
		errno = EPERM;
		return false;
	}

	if (count == 0 || firstIndex >= scene->actorCount)
		return 0;

	dsPhysicsEngine* engine = scene->engine;
	return engine->getSceneActorsFunc(outActors, engine, scene, firstIndex, count);
}

bool dsPhysicsScene_addConstraints(dsPhysicsScene* scene,
	dsPhysicsConstraint* const* constraints, uint32_t constraintCount, bool enable,
	const dsPhysicsSceneLock* lock)
{
	DS_PROFILE_FUNC_START();
	if (!scene || !scene->engine || !scene->engine->addSceneConstraintsFunc ||
		(!constraints && constraintCount > 0) || !lock)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!isWriteLocked(scene, lock))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Physics scene must have been locked for writing before adding constraints.");
		errno = EPERM;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (constraintCount == 0)
		DS_PROFILE_FUNC_RETURN(true);

	for (uint32_t i = 0; i < constraintCount; ++i)
	{
		dsPhysicsConstraint* constraint = constraints[i];
		if (!constraint || !constraint->firstActor || !constraint->secondActor)
		{
			errno = EINVAL;
			DS_PROFILE_FUNC_RETURN(false);
		}

		// Assume that the actors won't be added/removed across threads for this sanity check.
		if (constraint->firstActor->scene != scene || constraint->secondActor->scene != scene)
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
				"Cannot add a constraint to a scene without first adding its actors to the scene.");
			errno = EPERM;
			DS_PROFILE_FUNC_RETURN(false);
		}

		// Assume that the constraint won't be added/removed across threads for this sanity check.
		if (constraint->scene)
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
				"Cannot add a constraint to a scene when already associated with a scene.");
			errno = EPERM;
			DS_PROFILE_FUNC_RETURN(false);
		}
	}

	dsPhysicsEngine* engine = scene->engine;
	bool success = engine->addSceneConstraintsFunc(
		engine, scene, constraints, constraintCount, enable);
	DS_PROFILE_FUNC_RETURN(success);
}

bool dsPhysicsScene_removeConstraints(dsPhysicsScene* scene,
	dsPhysicsConstraint* const* constraints, uint32_t constraintCount,
	const dsPhysicsSceneLock* lock)
{
	DS_PROFILE_FUNC_START();
	if (!scene || !scene->engine || !scene->engine->removeSceneConstraintsFunc ||
		(!constraints && constraintCount > 0) || !lock)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!isWriteLocked(scene, lock))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Physics scene must have been locked for writing before removing constraints.");
		errno = EPERM;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (constraintCount == 0)
		DS_PROFILE_FUNC_RETURN(true);

	for (uint32_t i = 0; i < constraintCount; ++i)
	{
		dsPhysicsConstraint* constraint = constraints[i];
		if (!constraint)
		{
			errno = EINVAL;
			DS_PROFILE_FUNC_RETURN(false);
		}

		// Assume that the rigid body won't be added/removed across threads for this sanity check.
		if (constraint->scene != scene)
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
				"Cannot remove a constraint from a scene it's not associated with.");
			errno = EPERM;
			DS_PROFILE_FUNC_RETURN(false);
		}
	}

	dsPhysicsEngine* engine = scene->engine;
	bool success = engine->removeSceneConstraintsFunc(engine, scene, constraints, constraintCount);
	DS_PROFILE_FUNC_RETURN(success);
}

uint32_t dsPhysicsScene_getConstraints(dsPhysicsConstraint** outConstraints,
	const dsPhysicsScene* scene, uint32_t firstIndex, uint32_t count,
	const dsPhysicsSceneLock* lock)
{
	if ((!outConstraints && count > 0) || !scene || !scene->engine ||
		!scene->engine->getSceneConstraintsFunc || !lock)
	{
		errno = EINVAL;
		return DS_INVALID_PHYSICS_ID;
	}

	if (!isReadLocked(scene, lock))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Physics scene must have been locked for reading or writing before querying "
			"constraints.");
		errno = EPERM;
		return false;
	}

	if (count == 0 || firstIndex >= scene->actorCount)
		return 0;

	dsPhysicsEngine* engine = scene->engine;
	return engine->getSceneConstraintsFunc(outConstraints, engine, scene, firstIndex, count);
}

uint32_t dsPhysicsScene_castRay(const dsPhysicsScene* scene, const dsRay3f* ray,
	dsPhysicsQueryType queryType, void* userData, dsPhysicsLayer layer, uint64_t collisionGroup,
	dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc,
	dsCanIntersectPhysicsActorFunction canCollidePhysicsActorFunc,
	dsAddPhysicsRayIntersectionResult addResultFunc, const dsPhysicsSceneLock* lock)
{
	DS_PROFILE_FUNC_START();
	if (!scene || !scene->engine || !scene->engine->sceneCastRayFunc || !ray || !lock)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(DS_INVALID_PHYSICS_ID);
	}

	if (!isReadLocked(scene, lock))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Physics scene must have been locked for reading or "
			"writing before performing a ray cast.");
		errno = EPERM;
		DS_PROFILE_FUNC_RETURN(DS_INVALID_PHYSICS_ID);
	}

	dsPhysicsEngine* engine = scene->engine;
	uint32_t count = engine->sceneCastRayFunc(engine, scene, ray, queryType, userData, layer,
		collisionGroup, canCollisionGroupsCollideFunc, canCollidePhysicsActorFunc, addResultFunc);
	DS_PROFILE_FUNC_RETURN(count);
}

uint32_t dsPhysicsScene_intersectShapes(const dsPhysicsScene* scene,
	const dsPhysicsShapeInstance* shapes, uint32_t shapeCount, dsPhysicsQueryType queryType,
	void* userData, dsPhysicsLayer layer, uint64_t collisionGroup,
	dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc,
	dsCanIntersectPhysicsActorFunction canCollidePhysicsActorFunc,
	dsAddPhysicsShapeIntersectionResult addResultFunc,
	const dsPhysicsSceneLock* lock)
{
	DS_PROFILE_FUNC_START();
	if (!scene || !scene->engine || !scene->engine->sceneIntersectShapesFunc ||
		(!shapes && shapeCount > 0) || !lock)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(DS_INVALID_PHYSICS_ID);
	}

	for (uint32_t i = 0; i < shapeCount; ++i)
	{
		if (!shapes[i].shape)
		{
			errno = EINVAL;
			DS_PROFILE_FUNC_RETURN(DS_INVALID_PHYSICS_ID);
		}
	}

	if (!isReadLocked(scene, lock))
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Physics scene must have been locked for reading or "
			"writing before performing a ray cast.");
		errno = EPERM;
		DS_PROFILE_FUNC_RETURN(DS_INVALID_PHYSICS_ID);
	}

	if (shapeCount == 0)
		DS_PROFILE_FUNC_RETURN(0);

	dsPhysicsEngine* engine = scene->engine;
	uint32_t count = engine->sceneIntersectShapesFunc(engine, scene, shapes, shapeCount, queryType,
		userData, layer, collisionGroup, canCollisionGroupsCollideFunc, canCollidePhysicsActorFunc,
		addResultFunc);
	DS_PROFILE_FUNC_RETURN(count);
}

bool dsPhysicsScene_update(dsPhysicsScene* scene, float time, unsigned int stepCount)
{
	DS_PROFILE_FUNC_START();
	if (!scene || !scene->engine || !scene->engine->updateSceneFunc || time < 0.0f ||
		stepCount == 0)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (time == 0.0f)
		DS_PROFILE_FUNC_RETURN(true);

	if (scene->lock && !dsReadWriteLock_lockWrite(scene->lock))
		DS_PROFILE_FUNC_RETURN(false);

	// Forward a read lock even though we lock for writing so that the step callbacks cannot change
	// members.
	dsPhysicsSceneLock sceneLock = {.readLock = scene, .writeLock = NULL};
	dsPhysicsEngine* engine = scene->engine;
	bool success = engine->updateSceneFunc(engine, scene, time, stepCount, &sceneLock);

	if (scene->lock)
		dsReadWriteLock_unlockWrite(scene->lock);
	DS_PROFILE_FUNC_RETURN(success);
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

bool dsPhysicsScene_initialize(dsPhysicsScene* scene, dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsSceneSettings* settings)
{
	DS_ASSERT(scene);
	DS_ASSERT(engine);
	DS_ASSERT(allocator);
	DS_ASSERT(settings);

	scene->engine = engine;
	scene->allocator = allocator;
	if (settings->multiThreadedModifications)
	{
		scene->lock = dsReadWriteLock_create(
			allocator, "Physics Scene Read", "Physics Scene Write");
		if (!scene->lock)
			return false;
	}
	else
		scene->lock = NULL;
	scene->combineFrictionFunc = &dsPhysicsScene_defaultCombineFriction;
	scene->combineRestitutionFunc = &dsPhysicsScene_defaultCombineRestitution;
	scene->updatePhysicsActorContactPropertiesFunc = NULL;
	scene->updatePhysicsActorContactPropertiesUserData = NULL;
	scene->destroyUpdatePhysicsActorContactPropertiesUserDataFunc = NULL;
	scene->physicsActorContactManifoldAddedFunc = NULL;
	scene->physicsActorContactManifoldAddedUserData = NULL;
	scene->destroyPhysicsActorContactManifoldAddedUserDataFunc = NULL;
	scene->physicsActorContactManifoldUpdatedFunc = NULL;
	scene->physicsActorContactManifoldUpdatedUserData = NULL;
	scene->destroyPhysicsActorContactManifoldUpdatedUserDataFunc = NULL;
	scene->physicsActorContactManifoldRemovedFunc = NULL;
	scene->physicsActorContactManifoldRemovedUserData = NULL;
	scene->destroyPhysicsActorContactManifoldRemovedUserDataFunc = NULL;
	scene->gravity = settings->gravity;
	return true;
}

void dsPhysicsScene_shutdown(dsPhysicsScene* scene)
{
	DS_ASSERT(scene);

	dsReadWriteLock_destroy(scene->lock);
	if (scene->destroyUpdatePhysicsActorContactPropertiesUserDataFunc)
	{
		scene->destroyUpdatePhysicsActorContactPropertiesUserDataFunc(
			scene->updatePhysicsActorContactPropertiesUserData);
	}
	if (scene->destroyPhysicsActorContactManifoldAddedUserDataFunc)
	{
		scene->destroyPhysicsActorContactManifoldAddedUserDataFunc(
			scene->physicsActorContactManifoldAddedUserData);
	}
	if (scene->destroyPhysicsActorContactManifoldUpdatedUserDataFunc)
	{
		scene->destroyPhysicsActorContactManifoldUpdatedUserDataFunc(
			scene->physicsActorContactManifoldUpdatedUserData);
	}
	if (scene->destroyPhysicsActorContactManifoldRemovedUserDataFunc)
	{
		scene->destroyPhysicsActorContactManifoldRemovedUserDataFunc(
			scene->physicsActorContactManifoldRemovedUserData);
	}
}

float dsPhysicsScene_defaultCombineFriction(float frictionA, float frictionB);
float dsPhysicsScene_defaultCombineRestitution(float restitutionA, float hardnessA,
	float restitutionB, float hardnessB);
float dsPhysicsScene_combineFriction(const dsPhysicsScene* scene, float frictionA, float frictionB);
float dsPhysicsScene_combineRestitution(const dsPhysicsScene* scene, float restitutionA,
	float hardnessA, float restitutionB, float hardnessB);

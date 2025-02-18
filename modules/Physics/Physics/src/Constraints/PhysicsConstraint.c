/*
 * Copyright 2024-2025 Aaron Barany
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

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>

#include "Constraints/PhysicsConstraintLoad.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Streams/FileArchive.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/Types.h>

bool dsPhysicsConstraint_initialize(dsPhysicsConstraint* constraint, dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsConstraintType* type, const dsPhysicsActor* firstActor,
	const dsPhysicsActor* secondActor, void* impl,
	dsSetPhysicsConstraintEnabledFunction setEnabledFunc,
	dsGetPhysicsConstraintForceFunction getForceFunc,
	dsGetPhysicsConstraintForceFunction getTorqueFunc,
	dsDestroyPhysicsConstraintFunction destroyFunc)
{
	if (!constraint || !engine || !type || !allocator || !setEnabledFunc || !destroyFunc)
	{
		errno = EINVAL;
		return false;
	}

	constraint->engine = engine;
	constraint->allocator = dsAllocator_keepPointer(allocator);
	constraint->type = type;
	constraint->enabled = false;
	constraint->firstActor = firstActor;
	constraint->secondActor = secondActor;
	constraint->impl = impl;
	constraint->setEnabledFunc = setEnabledFunc;
	constraint->getForceFunc = getForceFunc;
	constraint->getTorqueFunc = getTorqueFunc;
	constraint->destroyFunc = destroyFunc;
	return true;
}

dsPhysicsConstraint* dsPhysicsConstraint_loadFile(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	dsFindPhysicsConstraintFunction findConstraintFunc, void* findConstraintUserData,
	const char* filePath)
{
	if (!engine || !findActorFunc || !filePath)
	{
		errno = EINVAL;
		return NULL;
	}

	dsFileStream stream;
	if (!dsFileStream_openPath(&stream, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Couldn't open physics constraint file '%s'.", filePath);
		return NULL;
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, (dsStream*)&stream, engine->allocator);
	dsFileStream_close(&stream);
	if (!buffer)
		return NULL;

	dsPhysicsConstraint* constraint = dsPhysicsConstraint_loadImpl(engine, allocator,
		findActorFunc, findActorUserData, findConstraintFunc, findConstraintUserData, buffer, size,
		filePath);
	DS_VERIFY(dsAllocator_free(engine->allocator, buffer));
	return constraint;
}

dsPhysicsConstraint* dsPhysicsConstraint_loadResource(dsPhysicsEngine* engine,
	dsAllocator* allocator, dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	dsFindPhysicsConstraintFunction findConstraintFunc, void* findConstraintUserData,
	dsFileResourceType type, const char* filePath)
{
	if (!engine || !findActorFunc || !filePath)
	{
		errno = EINVAL;
		return NULL;
	}

	dsResourceStream stream;
	if (!dsResourceStream_open(&stream, type, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Couldn't open physics constraint file '%s'.", filePath);
		return NULL;
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, (dsStream*)&stream, engine->allocator);
	dsResourceStream_close(&stream);
	if (!buffer)
		return NULL;

	dsPhysicsConstraint* constraint = dsPhysicsConstraint_loadImpl(engine, allocator, findActorFunc,
		findActorUserData, findConstraintFunc, findConstraintUserData, buffer, size, filePath);
	DS_VERIFY(dsAllocator_free(engine->allocator, buffer));
	return constraint;
}

dsPhysicsConstraint* dsPhysicsConstraint_loadArchive(dsPhysicsEngine* engine,
	dsAllocator* allocator, dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	dsFindPhysicsConstraintFunction findConstraintFunc, void* findConstraintUserData,
	const dsFileArchive* archive, const char* filePath)
{
	if (!engine || !findActorFunc || !archive || !filePath)
	{
		errno = EINVAL;
		return NULL;
	}

	dsStream* stream = dsFileArchive_openFile(archive, filePath);
	if (!stream)
	{
		DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Couldn't open physics constraint file '%s'.", filePath);
		return NULL;
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, stream, engine->allocator);
	dsStream_close(stream);
	if (!buffer)
		return NULL;

	dsPhysicsConstraint* constraint = dsPhysicsConstraint_loadImpl(engine, allocator, findActorFunc,
		findActorUserData, findConstraintFunc, findConstraintUserData, buffer, size, filePath);
	DS_VERIFY(dsAllocator_free(engine->allocator, buffer));
	return constraint;
}

dsPhysicsConstraint* dsPhysicsConstraint_loadStream(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	dsFindPhysicsConstraintFunction findConstraintFunc, void* findConstraintUserData,
	dsStream* stream)
{
	if (!engine || !findActorFunc || !stream)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t size;
	void* buffer = dsStream_readUntilEnd(&size, stream, engine->allocator);
	if (!buffer)
		return NULL;

	dsPhysicsConstraint* constraint = dsPhysicsConstraint_loadImpl(engine, allocator, findActorFunc,
		findActorUserData, findConstraintFunc, findConstraintUserData, buffer, size, NULL);
	DS_VERIFY(dsAllocator_free(engine->allocator, buffer));
	return constraint;
}

dsPhysicsConstraint* dsPhysicsConstraint_loadData(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	dsFindPhysicsConstraintFunction findConstraintFunc, void* findConstraintUserData,
	const void* data, size_t size)
{
	if (!engine || !findActorFunc || !data || size == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	return dsPhysicsConstraint_loadImpl(engine, allocator, findActorFunc, findActorUserData,
		findConstraintFunc, findConstraintUserData, data, size, NULL);
}

dsPhysicsConstraint* dsPhysicsConstraint_clone(const dsPhysicsConstraint* constraint,
	dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsPhysicsConstraint* firstConnectedConstraint, const dsPhysicsActor* secondActor,
	const dsPhysicsConstraint* secondConnectedConstraint)
{
	if (!constraint || !constraint->type || !constraint->type->cloneConstraintFunc)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = constraint->allocator;

	if (!firstActor)
		firstActor = constraint->firstActor;

	if (!secondActor)
		secondActor = constraint->secondActor;

	return constraint->type->cloneConstraintFunc(constraint, allocator, firstActor,
		firstConnectedConstraint, secondActor, secondConnectedConstraint);
}

bool dsPhysicsConstraint_setEnabled(dsPhysicsConstraint* constraint, bool enabled)
{
	if (!constraint || !constraint->setEnabledFunc || !constraint->engine)
	{
		errno = EINVAL;
		return false;
	}

	if (!constraint->scene)
	{
		errno = EPERM;
		return false;
	}

	return constraint->setEnabledFunc(constraint->engine, constraint, enabled);
}

bool dsPhysicsConstraint_getLastAppliedForce(
	dsVector3f* outForce, const dsPhysicsConstraint* constraint)
{
	if (!outForce || !constraint || !constraint->engine)
	{
		errno = EINVAL;
		return false;
	}

	if (!constraint->scene || !constraint->enabled)
	{
		errno = EPERM;
		return false;
	}

	if (constraint->getForceFunc)
		return constraint->getForceFunc(outForce, constraint->engine, constraint);

	outForce->x = outForce->y = outForce->z = 0.0f;
	return true;
}

bool dsPhysicsConstraint_getLastAppliedTorque(
	dsVector3f* outTorque, const dsPhysicsConstraint* constraint)
{
	if (!outTorque || !constraint || !constraint->engine)
	{
		errno = EINVAL;
		return false;
	}

	if (!constraint->scene || !constraint->enabled)
	{
		errno = EPERM;
		return false;
	}

	if (constraint->getTorqueFunc)
		return constraint->getTorqueFunc(outTorque, constraint->engine, constraint);

	outTorque->x = outTorque->y = outTorque->z = 0.0f;
	return true;
}

bool dsPhysicsConstraint_destroy(dsPhysicsConstraint* constraint)
{
	if (!constraint)
		return true;

	if (!constraint->destroyFunc)
	{
		errno = EINVAL;
		return false;
	}

	return constraint->destroyFunc(constraint->engine, constraint);
}

bool dsPhysicsConstraint_isValid(const dsPhysicsConstraint* constraint);

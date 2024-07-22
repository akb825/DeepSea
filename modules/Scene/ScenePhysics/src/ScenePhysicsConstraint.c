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

#include <DeepSea/ScenePhysics/ScenePhysicsConstraint.h>

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <string.h>

const char* const dsScenePhysicsConstraint_typeName = "PhysicsConstraint";

static dsCustomSceneResourceType resourceType;
const dsCustomSceneResourceType* dsScenePhysicsConstraint_type(void)
{
	return &resourceType;
}

dsScenePhysicsConstraint* dsScenePhysicsConstraint_create(dsAllocator* allocator,
	dsPhysicsConstraint* constraint, const char* firstRigidBodyInstanceName,
	const char* firstConnectedConstraintInstanceName, const char* secondRigidBodyInstanceName,
	const char* secondConnectedConstraintInstanceName)
{
	if (!allocator || !constraint)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t firstRigidBodyInstanceNameLen =
		firstRigidBodyInstanceName ? strlen(firstRigidBodyInstanceName) + 1 : 0;
	size_t firstConnectedConstraintInstanceNameLen =
		firstConnectedConstraintInstanceName ? strlen(firstConnectedConstraintInstanceName) + 1 : 0;
	size_t secondRigidBodyInstanceNameLen =
		secondRigidBodyInstanceName ? strlen(secondRigidBodyInstanceName) + 1 : 0;
	size_t secondConnectedConstraintInstanceNameLen = secondConnectedConstraintInstanceName ?
		strlen(secondConnectedConstraintInstanceName) + 1 : 0;

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsScenePhysicsConstraint)) +
		DS_ALIGNED_SIZE(firstRigidBodyInstanceNameLen) +
		DS_ALIGNED_SIZE(firstConnectedConstraintInstanceNameLen) +
		DS_ALIGNED_SIZE(secondRigidBodyInstanceNameLen) +
		DS_ALIGNED_SIZE(secondConnectedConstraintInstanceNameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsScenePhysicsConstraint* sceneConstraint = DS_ALLOCATE_OBJECT(&bufferAlloc,
		dsScenePhysicsConstraint);
	DS_ASSERT(sceneConstraint);
	sceneConstraint->allocator = dsAllocator_keepPointer(allocator);
	sceneConstraint->constraint = constraint;

	char* nameCopy;
	if (firstRigidBodyInstanceName)
	{
		nameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, firstRigidBodyInstanceNameLen);
		DS_ASSERT(nameCopy);
		memcpy(nameCopy, firstRigidBodyInstanceName, firstRigidBodyInstanceNameLen);
		sceneConstraint->firstRigidBodyInstanceName = nameCopy;
	}
	else
		sceneConstraint->firstRigidBodyInstanceName = NULL;

	if (firstConnectedConstraintInstanceName)
	{
		nameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char,
			firstConnectedConstraintInstanceNameLen);
		DS_ASSERT(nameCopy);
		memcpy(nameCopy, firstConnectedConstraintInstanceName,
			firstConnectedConstraintInstanceNameLen);
		sceneConstraint->firstConnectedConstraintInstanceName = nameCopy;
	}
	else
		sceneConstraint->firstConnectedConstraintInstanceName = NULL;

	if (secondRigidBodyInstanceName)
	{
		nameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, secondRigidBodyInstanceNameLen);
		DS_ASSERT(nameCopy);
		memcpy(nameCopy, secondRigidBodyInstanceName, secondRigidBodyInstanceNameLen);
		sceneConstraint->secondRigidBodyInstanceName = nameCopy;
	}
	else
		sceneConstraint->secondRigidBodyInstanceName = NULL;

	if (secondConnectedConstraintInstanceName)
	{
		nameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char,
			secondConnectedConstraintInstanceNameLen);
		DS_ASSERT(nameCopy);
		memcpy(nameCopy, secondConnectedConstraintInstanceName,
			secondConnectedConstraintInstanceNameLen);
		sceneConstraint->secondConnectedConstraintInstanceName = nameCopy;
	}
	else
		sceneConstraint->secondConnectedConstraintInstanceName = NULL;

	return sceneConstraint;
}

bool dsScenePhysicsConstraint_destroy(dsScenePhysicsConstraint* constraint)
{
	if (!constraint)
		return true;

	if (!dsScenePhysicsConstraint_destroy(constraint))
		return false;

	if (constraint->allocator)
		DS_VERIFY(dsAllocator_free(constraint->allocator, constraint));
	return true;
}

dsCustomSceneResource* dsScenePhysicsConstraint_createResource(dsAllocator* allocator,
	dsPhysicsConstraint* constraint)
{
	if (!allocator || !constraint)
	{
		errno = EINVAL;
		return NULL;
	}

	dsCustomSceneResource* customResource = DS_ALLOCATE_OBJECT(allocator, dsCustomSceneResource);
	if (!customResource)
		return NULL;

	customResource->allocator = dsAllocator_keepPointer(allocator);
	customResource->type = &resourceType;
	customResource->resource = constraint;
	customResource->destroyFunc =
		(dsDestroyCustomSceneResourceFunction)&dsScenePhysicsConstraint_destroy;
	return customResource;
}

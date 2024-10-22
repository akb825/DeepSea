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
	dsPhysicsConstraint* constraint, const char* firstRigidBodyInstance,
	const char* firstConnectedConstraintInstance, const char* secondRigidBodyInstance,
	const char* secondConnectedConstraintInstance)
{
	if (!allocator || !constraint)
	{
		errno = EINVAL;
		dsPhysicsConstraint_destroy(constraint);
		return NULL;
	}

	size_t firstRigidBodyInstanceLen =
		firstRigidBodyInstance ? strlen(firstRigidBodyInstance) + 1 : 0;
	size_t firstConnectedConstraintInstanceLen =
		firstConnectedConstraintInstance ? strlen(firstConnectedConstraintInstance) + 1 : 0;
	size_t secondRigidBodyInstanceLen =
		secondRigidBodyInstance ? strlen(secondRigidBodyInstance) + 1 : 0;
	size_t secondConnectedConstraintInstanceLen = secondConnectedConstraintInstance ?
		strlen(secondConnectedConstraintInstance) + 1 : 0;

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsScenePhysicsConstraint)) +
		DS_ALIGNED_SIZE(firstRigidBodyInstanceLen) +
		DS_ALIGNED_SIZE(firstConnectedConstraintInstanceLen) +
		DS_ALIGNED_SIZE(secondRigidBodyInstanceLen) +
		DS_ALIGNED_SIZE(secondConnectedConstraintInstanceLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		dsPhysicsConstraint_destroy(constraint);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsScenePhysicsConstraint* sceneConstraint = DS_ALLOCATE_OBJECT(&bufferAlloc,
		dsScenePhysicsConstraint);
	DS_ASSERT(sceneConstraint);
	sceneConstraint->allocator = dsAllocator_keepPointer(allocator);
	sceneConstraint->constraint = constraint;

	char* nameCopy;
	if (firstRigidBodyInstance)
	{
		nameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, firstRigidBodyInstanceLen);
		DS_ASSERT(nameCopy);
		memcpy(nameCopy, firstRigidBodyInstance, firstRigidBodyInstanceLen);
		sceneConstraint->firstRigidBodyInstance = nameCopy;
	}
	else
		sceneConstraint->firstRigidBodyInstance = NULL;

	if (firstConnectedConstraintInstance)
	{
		nameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char,
			firstConnectedConstraintInstanceLen);
		DS_ASSERT(nameCopy);
		memcpy(nameCopy, firstConnectedConstraintInstance, firstConnectedConstraintInstanceLen);
		sceneConstraint->firstConnectedConstraintInstance = nameCopy;
	}
	else
		sceneConstraint->firstConnectedConstraintInstance = NULL;

	if (secondRigidBodyInstance)
	{
		nameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, secondRigidBodyInstanceLen);
		DS_ASSERT(nameCopy);
		memcpy(nameCopy, secondRigidBodyInstance, secondRigidBodyInstanceLen);
		sceneConstraint->secondRigidBodyInstance = nameCopy;
	}
	else
		sceneConstraint->secondRigidBodyInstance = NULL;

	if (secondConnectedConstraintInstance)
	{
		nameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char,
			secondConnectedConstraintInstanceLen);
		DS_ASSERT(nameCopy);
		memcpy(nameCopy, secondConnectedConstraintInstance,
			secondConnectedConstraintInstanceLen);
		sceneConstraint->secondConnectedConstraintInstance = nameCopy;
	}
	else
		sceneConstraint->secondConnectedConstraintInstance = NULL;

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

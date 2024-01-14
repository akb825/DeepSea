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

#include <DeepSea/Physics/DefaultRigidBodyGroup.h>

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>

typedef struct dsDefaultRigidBodyGroup
{
	dsRigidBodyGroup group;
	dsSpinlock lock;
} dsDefaultRigidBodyGroup;

dsRigidBodyGroup* dsDefaultRigidBodyGroup_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsPhysicsMotionType motionType)
{
	DS_ASSERT(engine);
	DS_ASSERT(allocator);
	DS_ASSERT(allocator->freeFunc);

	dsDefaultRigidBodyGroup* defaultGroup = DS_ALLOCATE_OBJECT(allocator, dsDefaultRigidBodyGroup);
	if (!defaultGroup)
		return NULL;

	DS_VERIFY(dsSpinlock_initialize(&defaultGroup->lock));

	dsRigidBodyGroup* group = (dsRigidBodyGroup*)defaultGroup;
	group->engine = engine;
	group->allocator = allocator;
	group->motionType = motionType;
	group->rigidBodies = NULL;
	group->rigidBodyCount = 0;
	group->maxRigidBodies = 0;
	return group;
}

bool dsDefaultRigidBodyGroup_addRigidBody(dsRigidBodyGroup* group, dsRigidBody* rigidBody)
{
	DS_ASSERT(group);
	DS_ASSERT(rigidBody);

	dsDefaultRigidBodyGroup* defaultGroup = (dsDefaultRigidBodyGroup*)group;
	DS_VERIFY(dsSpinlock_lock(&defaultGroup->lock));
	uint32_t index = group->rigidBodyCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(group->allocator, group->rigidBodies, group->rigidBodyCount,
			group->maxRigidBodies, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&defaultGroup->lock));
		return false;
	}

	group->rigidBodies[index] = rigidBody;
	DS_VERIFY(dsSpinlock_unlock(&defaultGroup->lock));
	return true;
}

bool dsDefaultRigidBodyGroup_removeRigidBody(dsRigidBodyGroup* group, dsRigidBody* rigidBody)
{
	DS_ASSERT(group);
	DS_ASSERT(rigidBody);

	dsDefaultRigidBodyGroup* defaultGroup = (dsDefaultRigidBodyGroup*)group;
	DS_VERIFY(dsSpinlock_lock(&defaultGroup->lock));
	for (uint32_t i = 0; i < group->rigidBodyCount; ++i)
	{
		if (group->rigidBodies[i] != rigidBody)
			continue;

		// Constant-time removal since order doesn't matter.
		group->rigidBodies[i] = group->rigidBodies[group->rigidBodyCount - 1];
		--group->rigidBodyCount;
		DS_VERIFY(dsSpinlock_unlock(&defaultGroup->lock));
		return true;
	}

	DS_VERIFY(dsSpinlock_unlock(&defaultGroup->lock));
	errno = ENOTFOUND;
	return false;
}

bool dsDefaultRigidBodyGroup_destroy(dsPhysicsEngine* engine, dsRigidBodyGroup* group)
{
	DS_ASSERT(engine);
	DS_UNUSED(engine);
	DS_ASSERT(group);
	DS_ASSERT(group->rigidBodyCount == 0);

	dsDefaultRigidBodyGroup* defaultGroup = (dsDefaultRigidBodyGroup*)group;
	dsSpinlock_shutdown(&defaultGroup->lock);

	DS_VERIFY(dsAllocator_free(group->allocator, group->rigidBodies));
	DS_VERIFY(dsAllocator_free(group->allocator, group));
	return true;
}

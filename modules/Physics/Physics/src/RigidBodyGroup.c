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

#include <DeepSea/Physics/RigidBodyGroup.h>

#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/Types.h>

dsRigidBodyGroup* dsRigidBodyGroup_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsPhysicsMotionType motionType)
{
	if (!engine || !engine->createRigidBodyGroupFunc || !engine->destroyRigidBodyGroupFunc ||
		!allocator)
	{
		errno = EINVAL;
		return false;
	}

	if (!allocator->freeFunc)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Rigid body group allocator must support freeing memory.");
		errno = EINVAL;
		return NULL;
	}

	return engine->createRigidBodyGroupFunc(engine, allocator, motionType);
}

bool dsRigidBodyGroup_destroy(dsRigidBodyGroup* group)
{
	if (!group)
		return true;

	dsPhysicsEngine* engine = group->engine;
	if (!engine || !engine->destroyRigidBodyGroupFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (group->rigidBodyCount != 0)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Cannot destroy a rigid body group until its component "
			"rigid bodies have been destroyed.");
		errno = EPERM;
		return false;
	}

	return engine->destroyRigidBodyGroupFunc(engine, group);
}

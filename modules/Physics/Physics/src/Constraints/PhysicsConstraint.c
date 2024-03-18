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

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Error.h>

bool dsPhysicsConstraint_initialize(dsPhysicsConstraint* constraint, dsPhysicsEngine* engine,
	dsAllocator* allocator, dsPhysicsConstraintType type, bool enabled, void* impl,
	dsDestroyPhysicsConstraintFunction destroyFunc)
{
	if (!constraint || !engine || !allocator || !impl || !destroyFunc)
	{
		errno = EINVAL;
		return false;
	}

	constraint->engine = engine;
	constraint->allocator = dsAllocator_keepPointer(allocator);
	constraint->type = type;
	constraint->enabled = enabled;
	constraint->impl = impl;
	constraint->destroyFunc = destroyFunc;
	return true;
}

bool dsPhysicsConstraint_destroy(dsPhysicsConstraint* constraint)
{
	if (!constraint)
		return true;

	return constraint->destroyFunc(constraint->engine, constraint);
}

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

#include <DeepSea/Physics/Types.h>

bool dsPhysicsConstraint_initialize(dsPhysicsConstraint* constraint, dsPhysicsEngine* engine,
	dsAllocator* allocator, dsPhysicsConstraintType type, const dsPhysicsActor* firstActor,
	const dsPhysicsActor* secondActor, bool enabled, void* impl,
	dsGetPhysicsConstraintForceFunction getForceFunc,
	dsGetPhysicsConstraintForceFunction getTorqueFunc,
	dsDestroyPhysicsConstraintFunction destroyFunc)
{
	if (!constraint || !engine || !allocator || !firstActor || !secondActor || !destroyFunc)
	{
		errno = EINVAL;
		return false;
	}

	constraint->engine = engine;
	constraint->allocator = dsAllocator_keepPointer(allocator);
	constraint->type = type;
	constraint->enabled = enabled;
	constraint->firstActor = firstActor;
	constraint->secondActor = secondActor;
	constraint->impl = impl;
	constraint->getForceFunc = getForceFunc;
	constraint->getTorqueFunc = getTorqueFunc;
	constraint->destroyFunc = destroyFunc;
	return true;
}

bool dsPhysicsConstraint_setEnabled(dsPhysicsConstraint* constraint, bool enabled)
{
	if (!constraint || !constraint->engine || !constraint->engine->setConstraintEnabledFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = constraint->engine;
	return engine->setConstraintEnabledFunc(engine, constraint, enabled);
}

bool dsPhysicsConstraint_getLastAppliedForce(
	dsVector3f* outForce, const dsPhysicsConstraint* constraint)
{
	if (!outForce || !constraint || !constraint->engine)
	{
		errno = EINVAL;
		return false;
	}

	if (!constraint->enabled)
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

	if (!constraint->enabled)
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

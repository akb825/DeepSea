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

#include <DeepSea/Physics/Constraints/ConePhysicsConstraint.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>
#include <DeepSea/Physics/Types.h>

#include <math.h>

dsPhysicsConstraintType dsConePhysicsConstraint_type(void)
{
	static int type;
	return &type;
}

dsConePhysicsConstraint* dsConePhysicsConstraint_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, bool enabled, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstRotation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondRotation, float maxAngle)
{
	if (!engine || !engine->createConeConstraintFunc || !engine->destroyConeConstraintFunc ||
		!firstActor || !firstPosition || !firstRotation || !secondActor || !secondPosition ||
		!secondRotation || maxAngle < 0 || maxAngle > M_PI)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = engine->allocator;

	return engine->createConeConstraintFunc(engine, allocator, enabled, firstActor, firstPosition,
		firstRotation, secondActor, secondPosition, secondRotation, maxAngle);
}

bool dsConePhysicsConstraint_setMaxAngle(dsConePhysicsConstraint* constraint, float maxAngle)
{
	dsPhysicsConstraint* baseConstraint = (dsPhysicsConstraint*)constraint;
	if (!constraint || !baseConstraint->engine ||
		!baseConstraint->engine->setConeConstraintMaxAngleFunc || maxAngle < 0 || maxAngle > M_PI)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = baseConstraint->engine;
	return engine->setConeConstraintMaxAngleFunc(engine, constraint, maxAngle);
}

void dsConePhysicsConstraint_initialize(dsConePhysicsConstraint* constraint,
	dsPhysicsEngine* engine, dsAllocator* allocator, bool enabled, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstRotation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondRotation, float maxAngle, void* impl,
	dsGetPhysicsConstraintForceFunction getForceFunc,
	dsGetPhysicsConstraintForceFunction getTorqueFunc)
{
	DS_ASSERT(engine);
	DS_ASSERT(firstPosition);
	DS_ASSERT(firstRotation);
	DS_ASSERT(secondPosition);
	DS_ASSERT(secondRotation);
	DS_ASSERT(getTorqueFunc);
	DS_ASSERT(maxAngle >= 0.0f);
	DS_ASSERT(maxAngle <= M_PI);

	DS_VERIFY(dsPhysicsConstraint_initialize((dsPhysicsConstraint*)constraint, engine, allocator,
		dsConePhysicsConstraint_type(), firstActor, secondActor, enabled, impl, getForceFunc,
		getTorqueFunc, (dsDestroyPhysicsConstraintFunction)engine->destroyConeConstraintFunc));

	constraint->firstPosition = *firstPosition;
	constraint->secondPosition = *secondPosition;
	constraint->firstRotation = *firstRotation;
	constraint->secondRotation = *secondRotation;
	constraint->maxAngle = maxAngle;
}

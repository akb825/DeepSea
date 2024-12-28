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

#include <DeepSea/Math/Core.h>

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>
#include <DeepSea/Physics/Types.h>

static dsPhysicsConstraint* dsConePhysicsConstraint_clone(const dsPhysicsConstraint* constraint,
	dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsPhysicsConstraint* firstConnectedConstraint, const dsPhysicsActor* secondActor,
	const dsPhysicsConstraint* secondConnectedConstraint)
{
	DS_ASSERT(constraint);
	DS_ASSERT(constraint->type == dsConePhysicsConstraint_type());
	DS_UNUSED(firstConnectedConstraint);
	DS_UNUSED(secondConnectedConstraint);

	const dsConePhysicsConstraint* coneConstraint = (const dsConePhysicsConstraint*)constraint;
	return (dsPhysicsConstraint*)dsConePhysicsConstraint_create(constraint->engine, allocator,
		firstActor, &coneConstraint->firstPosition, &coneConstraint->firstOrientation, secondActor,
		&coneConstraint->secondPosition, &coneConstraint->secondOrientation,
		coneConstraint->maxAngle);
}

const dsPhysicsConstraintType* dsConePhysicsConstraint_type(void)
{
	static dsPhysicsConstraintType type = {&dsConePhysicsConstraint_clone};
	return &type;
}

dsConePhysicsConstraint* dsConePhysicsConstraint_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsActor* firstActor, const dsVector3f* firstPosition,
	const dsQuaternion4f* firstOrientation, const dsPhysicsActor* secondActor,
	const dsVector3f* secondPosition, const dsQuaternion4f* secondOrientation, float maxAngle)
{
	if (!engine || !engine->createConeConstraintFunc || !engine->destroyConeConstraintFunc ||
		!firstPosition || !firstOrientation || !secondPosition || !secondOrientation ||
		maxAngle < 0 || maxAngle > M_PIf)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = engine->allocator;

	return engine->createConeConstraintFunc(engine, allocator, firstActor, firstPosition,
		firstOrientation, secondActor, secondPosition, secondOrientation, maxAngle);
}

bool dsConePhysicsConstraint_setMaxAngle(dsConePhysicsConstraint* constraint, float maxAngle)
{
	dsPhysicsConstraint* baseConstraint = (dsPhysicsConstraint*)constraint;
	if (!constraint || !baseConstraint->engine ||
		!baseConstraint->engine->setConeConstraintMaxAngleFunc || maxAngle < 0 || maxAngle > M_PIf)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = baseConstraint->engine;
	return engine->setConeConstraintMaxAngleFunc(engine, constraint, maxAngle);
}

void dsConePhysicsConstraint_initialize(dsConePhysicsConstraint* constraint,
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstOrientation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondOrientation, float maxAngle, void* impl)
{
	DS_ASSERT(engine);
	DS_ASSERT(firstPosition);
	DS_ASSERT(firstOrientation);
	DS_ASSERT(secondPosition);
	DS_ASSERT(secondOrientation);
	DS_ASSERT(maxAngle >= 0.0f);
	DS_ASSERT(maxAngle <= M_PI);

	DS_VERIFY(dsPhysicsConstraint_initialize((dsPhysicsConstraint*)constraint, engine, allocator,
		dsConePhysicsConstraint_type(), firstActor, secondActor, impl,
		(dsSetPhysicsConstraintEnabledFunction)engine->setConeConstraintEnabledFunc,
		(dsGetPhysicsConstraintForceFunction)engine->getConeConstraintForceFunc,
		(dsGetPhysicsConstraintForceFunction)engine->getConeConstraintTorqueFunc,
		(dsDestroyPhysicsConstraintFunction)engine->destroyConeConstraintFunc));

	constraint->firstPosition = *firstPosition;
	constraint->secondPosition = *secondPosition;
	constraint->firstOrientation = *firstOrientation;
	constraint->secondOrientation = *secondOrientation;
	constraint->maxAngle = maxAngle;
}

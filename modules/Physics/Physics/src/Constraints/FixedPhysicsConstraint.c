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

#include <DeepSea/Physics/Constraints/FixedPhysicsConstraint.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>
#include <DeepSea/Physics/Types.h>

static dsPhysicsConstraint* dsFixedPhysicsConstraint_clone(const dsPhysicsConstraint* constraint,
	dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsPhysicsConstraint* firstConnectedConstraint, const dsPhysicsActor* secondActor,
	const dsPhysicsConstraint* secondConnectedConstraint)
{
	DS_ASSERT(constraint);
	DS_ASSERT(constraint->type == dsFixedPhysicsConstraint_type());
	DS_UNUSED(firstConnectedConstraint);
	DS_UNUSED(secondConnectedConstraint);

	const dsFixedPhysicsConstraint* fixedConstraint = (const dsFixedPhysicsConstraint*)constraint;
	return (dsPhysicsConstraint*)dsFixedPhysicsConstraint_create(constraint->engine, allocator,
		firstActor, &fixedConstraint->firstPosition, &fixedConstraint->firstOrientation,
		secondActor, &fixedConstraint->secondPosition, &fixedConstraint->secondOrientation);
}

const dsPhysicsConstraintType* dsFixedPhysicsConstraint_type(void)
{
	static dsPhysicsConstraintType type = {&dsFixedPhysicsConstraint_clone};
	return &type;
}

dsFixedPhysicsConstraint* dsFixedPhysicsConstraint_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsActor* firstActor, const dsVector3f* firstPosition,
	const dsQuaternion4f* firstOrientation, const dsPhysicsActor* secondActor,
	const dsVector3f* secondPosition, const dsQuaternion4f* secondOrientation)
{
	if (!engine || !engine->createFixedConstraintFunc || !engine->destroyFixedConstraintFunc ||
		!firstPosition || !firstOrientation || !secondPosition || !secondOrientation)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = engine->allocator;

	return engine->createFixedConstraintFunc(engine, allocator, firstActor, firstPosition,
		firstOrientation, secondActor, secondPosition, secondOrientation);
}

void dsFixedPhysicsConstraint_initialize(dsFixedPhysicsConstraint* constraint,
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstOrientation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondOrientation, void* impl)
{
	DS_ASSERT(constraint);
	DS_ASSERT(engine);
	DS_ASSERT(firstPosition);
	DS_ASSERT(firstOrientation);
	DS_ASSERT(secondPosition);
	DS_ASSERT(secondOrientation);

	DS_VERIFY(dsPhysicsConstraint_initialize((dsPhysicsConstraint*)constraint, engine, allocator,
		dsFixedPhysicsConstraint_type(), firstActor, secondActor, impl,
		(dsSetPhysicsConstraintEnabledFunction)engine->setFixedConstraintEnabledFunc,
		(dsGetPhysicsConstraintForceFunction)engine->getFixedConstraintForceFunc,
		(dsGetPhysicsConstraintForceFunction)engine->getFixedConstraintTorqueFunc,
		(dsDestroyPhysicsConstraintFunction)engine->destroyFixedConstraintFunc));

	constraint->firstPosition = *firstPosition;
	constraint->secondPosition = *secondPosition;
	constraint->firstOrientation = *firstOrientation;
	constraint->secondOrientation = *secondOrientation;
}

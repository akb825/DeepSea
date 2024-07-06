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

#include <DeepSea/Physics/Constraints/PointPhysicsConstraint.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>
#include <DeepSea/Physics/Types.h>

static dsPhysicsConstraint* dsPointPhysicsConstraint_clone(const dsPhysicsConstraint* constraint,
	dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsPhysicsConstraint* firstConnectedConstraint, const dsPhysicsActor* secondActor,
	const dsPhysicsConstraint* secondConnectedConstraint)
{
	DS_ASSERT(constraint);
	DS_ASSERT(constraint->type == dsPointPhysicsConstraint_type());
	DS_UNUSED(firstConnectedConstraint);
	DS_UNUSED(secondConnectedConstraint);

	const dsPointPhysicsConstraint* pointConstraint = (const dsPointPhysicsConstraint*)constraint;
	return (dsPhysicsConstraint*)dsPointPhysicsConstraint_create(constraint->engine, allocator,
		firstActor, &pointConstraint->firstPosition, secondActor, &pointConstraint->secondPosition);
}

const dsPhysicsConstraintType* dsPointPhysicsConstraint_type(void)
{
	static dsPhysicsConstraintType type = {&dsPointPhysicsConstraint_clone};
	return &type;
}

dsPointPhysicsConstraint* dsPointPhysicsConstraint_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsActor* firstActor, const dsVector3f* firstPosition,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition)
{
	if (!engine || !engine->createPointConstraintFunc || !engine->destroyPointConstraintFunc ||
		!firstPosition || !secondPosition)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = engine->allocator;

	return engine->createPointConstraintFunc(engine, allocator, firstActor, firstPosition,
		secondActor, secondPosition);
}

void dsPointPhysicsConstraint_initialize(dsPointPhysicsConstraint* constraint,
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsPhysicsActor* secondActor,
	const dsVector3f* secondPosition, void* impl)
{
	DS_ASSERT(constraint);
	DS_ASSERT(engine);
	DS_ASSERT(firstPosition);
	DS_ASSERT(secondPosition);

	DS_VERIFY(dsPhysicsConstraint_initialize((dsPhysicsConstraint*)constraint, engine, allocator,
		dsPointPhysicsConstraint_type(), firstActor, secondActor, impl,
		(dsSetPhysicsConstraintEnabledFunction)engine->setPointConstraintEnabledFunc,
		(dsGetPhysicsConstraintForceFunction)engine->getPointConstraintForceFunc, NULL,
		(dsDestroyPhysicsConstraintFunction)engine->destroyPointConstraintFunc));

	constraint->firstPosition = *firstPosition;
	constraint->secondPosition = *secondPosition;
}

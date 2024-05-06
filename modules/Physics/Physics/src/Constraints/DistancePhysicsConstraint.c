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

#include <DeepSea/Physics/Constraints/DistancePhysicsConstraint.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Math/Core.h>

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>
#include <DeepSea/Physics/Types.h>

dsPhysicsConstraintType dsDistancePhysicsConstraint_type(void)
{
	static int type;
	return &type;
}

dsDistancePhysicsConstraint* dsDistancePhysicsConstraint_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, bool enabled, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsPhysicsActor* secondActor,
	const dsVector3f* secondPosition, float minDistance, float maxDistance, float limitStiffness,
	float limitDamping)
{
	if (!engine || !engine->createDistanceConstraintFunc ||
		!engine->destroyDistanceConstraintFunc || !firstActor || !firstPosition || !secondActor ||
		!secondPosition || minDistance < 0.0f || maxDistance < 0.0f || minDistance > maxDistance ||
		limitStiffness < 0.0f || limitDamping < 0.0f || limitDamping > 0.0f)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = engine->allocator;

	return engine->createDistanceConstraintFunc(engine, allocator, enabled, firstActor,
		firstPosition, secondActor, secondPosition, minDistance, maxDistance, limitStiffness,
		limitDamping);
}

bool dsDistancePhysicsConstraint_setLimit(dsDistancePhysicsConstraint* constraint,
	float minDistance, float maxDistance, float stiffness,
float damping)
{
	dsPhysicsConstraint* baseConstraint = (dsPhysicsConstraint*)constraint;
	if (!constraint || !baseConstraint->engine ||
		!baseConstraint->engine->setDistanceConstraintLimitFunc || minDistance < 0.0f ||
		maxDistance < 0.0f || minDistance > maxDistance || stiffness < 0.0f || damping < 0.0f ||
		damping > 1.0f)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = baseConstraint->engine;
	return engine->setDistanceConstraintLimitFunc(engine, constraint, minDistance, maxDistance,
		stiffness, damping);
}

void dsDistancePhysicsConstraint_initialize(dsDistancePhysicsConstraint* constraint,
	dsPhysicsEngine* engine, dsAllocator* allocator, bool enabled, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsPhysicsActor* secondActor,
	const dsVector3f* secondPosition, float minDistance, float maxDistance, float limitStiffness,
	float limitDamping, void* impl, dsGetPhysicsConstraintForceFunction getForceFunc)
{
	DS_ASSERT(constraint);
	DS_ASSERT(engine);
	DS_ASSERT(firstPosition);
	DS_ASSERT(secondPosition);
	DS_ASSERT(minDistance >= 0.0f);
	DS_ASSERT(maxDistance >= 0.0f);
	DS_ASSERT(minDistance < maxDistance);
	DS_ASSERT(limitStiffness >= 0.0f);
	DS_ASSERT(limitDamping >= 0.0f && limitDamping <= 1.0f);
	DS_ASSERT(getForceFunc);

	DS_VERIFY(dsPhysicsConstraint_initialize((dsPhysicsConstraint*)constraint, engine, allocator,
		dsDistancePhysicsConstraint_type(), firstActor, secondActor, enabled, impl, getForceFunc,
		NULL, (dsDestroyPhysicsConstraintFunction)engine->destroyDistanceConstraintFunc));

	constraint->firstPosition = *firstPosition;
	constraint->secondPosition = *secondPosition;
	constraint->minDistance = minDistance;
	constraint->maxDistance = maxDistance;
	constraint->limitStiffness = limitStiffness;
	constraint->limitDamping = limitDamping;
}

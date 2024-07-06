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

#include <DeepSea/Physics/Constraints/GearPhysicsConstraint.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Quaternion.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>
#include <DeepSea/Physics/Constraints/RevolutePhysicsConstraint.h>
#include <DeepSea/Physics/Types.h>

static bool isConstraintValid(const dsRevolutePhysicsConstraint* constraint,
	const dsPhysicsActor* actor, const dsVector3f* axis)
{
	if (!constraint || !actor)
		return true;

	const dsPhysicsConstraint* baseConstraint = (const dsPhysicsConstraint*)constraint;
	dsVector3f constraintAxis;
	if (baseConstraint->firstActor == actor)
		dsQuaternion4f_getRotationAxis(&constraintAxis, &constraint->firstRotation);
	else if (baseConstraint->secondActor == actor)
		dsQuaternion4f_getRotationAxis(&constraintAxis, &constraint->secondRotation);
	else
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Revolute constraint for gear constraint doesn't reference expected actor.");
		return false;
	}

	const float epsilon = 1e-3f;
	float cosAngle = fabsf(dsVector3_dot(*axis, constraintAxis));
	if (cosAngle < 1.0f - epsilon)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Axis for gear and revolute constraints aren't aligned.");
		return false;
	}

	return true;
}

static dsPhysicsConstraint* dsGearPhysicsConstraint_clone(const dsPhysicsConstraint* constraint,
	dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsPhysicsConstraint* firstConnectedConstraint, const dsPhysicsActor* secondActor,
	const dsPhysicsConstraint* secondConnectedConstraint)
{
	DS_ASSERT(constraint);
	DS_ASSERT(constraint->type == dsGearPhysicsConstraint_type());

	if (firstConnectedConstraint &&
		firstConnectedConstraint->type != dsRevolutePhysicsConstraint_type())
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Gear first connected constraint must be a revolute constraint.");
		errno = EINVAL;
		return NULL;
	}

	if (firstConnectedConstraint &&
		firstConnectedConstraint->type != dsRevolutePhysicsConstraint_type())
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
			"Gear second connected constraint must be a revolute constraint.");
		errno = EINVAL;
		return NULL;
	}

	const dsGearPhysicsConstraint* gearConstraint = (const dsGearPhysicsConstraint*)constraint;
	return (dsPhysicsConstraint*)dsGearPhysicsConstraint_create(constraint->engine, allocator,
		firstActor, &gearConstraint->firstAxis,
		(const dsRevolutePhysicsConstraint*)firstConnectedConstraint, secondActor,
		&gearConstraint->secondAxis, (const dsRevolutePhysicsConstraint*)firstConnectedConstraint,
		gearConstraint->ratio);
}

const dsPhysicsConstraintType* dsGearPhysicsConstraint_type(void)
{
	static dsPhysicsConstraintType type = {&dsGearPhysicsConstraint_clone};
	return &type;
}

float dsGearPhysicsConstraint_computeRatio(unsigned int firstActorToothCount,
	unsigned int secondActorToothCount)
{
	if (firstActorToothCount == 0 || secondActorToothCount == 0)
	{
		errno = EINVAL;
		return 0.0f;
	}

	return (float)firstActorToothCount/(float)secondActorToothCount;
}

dsGearPhysicsConstraint* dsGearPhysicsConstraint_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsActor* firstActor, const dsVector3f* firstAxis,
	const dsRevolutePhysicsConstraint* firstConstraint, const dsPhysicsActor* secondActor,
	const dsVector3f* secondAxis, const dsRevolutePhysicsConstraint* secondConstraint, float ratio)
{
	if (!engine || !engine->createGearConstraintFunc || !engine->destroyGearConstraintFunc ||
		!firstAxis || !isConstraintValid(firstConstraint, firstActor, firstAxis) || !secondAxis ||
		!isConstraintValid(secondConstraint, secondActor, secondAxis) || ratio == 0.0f)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = engine->allocator;

	return engine->createGearConstraintFunc(engine, allocator, firstActor, firstAxis,
		firstConstraint, secondActor, secondAxis, secondConstraint, ratio);
}

bool dsGearPhysicsConstraint_setRatio(dsGearPhysicsConstraint* constraint, float ratio)
{
	dsPhysicsConstraint* baseConstraint = (dsPhysicsConstraint*)constraint;
	if (!constraint || !baseConstraint->engine ||
		!baseConstraint->engine->setGearConstraintRatioFunc || ratio == 0.0f)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = baseConstraint->engine;
	return engine->setGearConstraintRatioFunc(engine, constraint, ratio);
}

void dsGearPhysicsConstraint_initialize(dsGearPhysicsConstraint* constraint,
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstAxis, const dsRevolutePhysicsConstraint* firstConstraint,
	const dsPhysicsActor* secondActor, const dsVector3f* secondAxis,
	const dsRevolutePhysicsConstraint* secondConstraint, float ratio, void* impl)
{
	DS_ASSERT(constraint);
	DS_ASSERT(engine);
	DS_ASSERT(allocator);
	DS_ASSERT(firstAxis);
	DS_ASSERT(isConstraintValid(firstConstraint, firstActor, firstAxis));;
	DS_ASSERT(secondAxis);
	DS_ASSERT(isConstraintValid(secondConstraint, secondActor, secondAxis));
	DS_ASSERT(ratio != 0.0f);

	DS_VERIFY(dsPhysicsConstraint_initialize((dsPhysicsConstraint*)constraint, engine, allocator,
		dsGearPhysicsConstraint_type(), firstActor, secondActor, impl,
		(dsSetPhysicsConstraintEnabledFunction)engine->setGearConstraintEnabledFunc, NULL,
		(dsGetPhysicsConstraintForceFunction)engine->getGearConstraintTorqueFunc,
		(dsDestroyPhysicsConstraintFunction)engine->destroyGearConstraintFunc));

	constraint->firstAxis = *firstAxis;
	constraint->secondAxis = *secondAxis;
	constraint->firstConstraint = firstConstraint;
	constraint->secondConstraint = secondConstraint;
	constraint->ratio = ratio;
}

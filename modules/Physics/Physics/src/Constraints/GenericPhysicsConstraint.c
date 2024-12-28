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

#include <DeepSea/Physics/Constraints/GenericPhysicsConstraint.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Math/Core.h>

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>
#include <DeepSea/Physics/Types.h>

#include <string.h>

static dsPhysicsConstraint* dsGenericPhysicsConstraint_clone(const dsPhysicsConstraint* constraint,
	dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsPhysicsConstraint* firstConnectedConstraint, const dsPhysicsActor* secondActor,
	const dsPhysicsConstraint* secondConnectedConstraint)
{
	DS_ASSERT(constraint);
	DS_ASSERT(constraint->type == dsGenericPhysicsConstraint_type());
	DS_UNUSED(firstConnectedConstraint);
	DS_UNUSED(secondConnectedConstraint);

	const dsGenericPhysicsConstraint* genericConstraint =
		(const dsGenericPhysicsConstraint*)constraint;
	return (dsPhysicsConstraint*)dsGenericPhysicsConstraint_create(constraint->engine, allocator,
		firstActor, &genericConstraint->firstPosition, &genericConstraint->firstOrientation,
		secondActor, &genericConstraint->secondPosition, &genericConstraint->secondOrientation,
		genericConstraint->limits, genericConstraint->motors,
		genericConstraint->combineSwingTwistMotors);
}

const dsPhysicsConstraintType* dsGenericPhysicsConstraint_type(void)
{
	static dsPhysicsConstraintType type = {&dsGenericPhysicsConstraint_clone};
	return &type;
}

dsGenericPhysicsConstraint* dsGenericPhysicsConstraint_create(
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstOrientation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondOrientation,
	const dsGenericPhysicsConstraintLimit limits[DS_PHYSICS_CONSTRAINT_DOF_COUNT],
	const dsGenericPhysicsConstraintMotor motors[DS_PHYSICS_CONSTRAINT_DOF_COUNT],
	bool combineSwingTwistMotors)
{
	if (!engine || !engine->createSwingTwistConstraintFunc ||
		!engine->destroySwingTwistConstraintFunc || !firstPosition || !firstOrientation ||
		!secondPosition || !secondOrientation || !limits || !motors)
	{
		errno = EINVAL;
		return NULL;
	}

	for (unsigned int i = 0; i < DS_PHYSICS_CONSTRAINT_DOF_COUNT; ++i)
	{
		const dsGenericPhysicsConstraintLimit* limit = limits + i;
		if (limit->limitType < dsPhysicsConstraintLimitType_Fixed ||
			limit->limitType > dsPhysicsConstraintLimitType_Range ||
			limit->minValue > limit->maxValue || limit->stiffness < 0.0f || limit->damping < 0.0f ||
			limit->damping > 1.0f)
		{
			errno = EINVAL;
			return NULL;
		}

		const dsGenericPhysicsConstraintMotor* motor = motors + i;
		if (motor->motorType < dsPhysicsConstraintMotorType_Disabled ||
			motor->motorType > dsPhysicsConstraintMotorType_Velocity ||
			motor->maxForce < 0.0f)
		{
			errno = EINVAL;
			return NULL;
		}
	}

	for (unsigned int i = dsPhysicsConstraintDOF_RotateX;
		i <= (unsigned int)dsPhysicsConstraintDOF_RotateZ; ++i)
	{
		const dsGenericPhysicsConstraintLimit* limit = limits + i;
		if (limit->minValue < -M_PIf || limit->minValue > M_PIf || limit->maxValue < -M_PIf ||
			limit->maxValue > M_PIf)
		{
			errno = EINVAL;
			return NULL;
		}
	}

	if (!allocator)
		allocator = engine->allocator;

	return engine->createGenericConstraintFunc(engine, allocator, firstActor, firstPosition,
		firstOrientation, secondActor, secondPosition, secondOrientation, limits, motors,
		combineSwingTwistMotors);
}

bool dsGenericPhysicsConstraint_setLimit(dsGenericPhysicsConstraint* constraint,
	dsPhysicsConstraintDOF dof, dsPhysicsConstraintLimitType limitType, float minValue,
	float maxValue, float stiffness, float damping)
{
	dsPhysicsConstraint* baseConstraint = (dsPhysicsConstraint*)constraint;
	if (!constraint || !baseConstraint->engine ||
		!baseConstraint->engine->setGenericConstraintLimitFunc ||
		dof < dsPhysicsConstraintDOF_TranslateX || dof > dsPhysicsConstraintDOF_RotateZ ||
		limitType < dsPhysicsConstraintLimitType_Fixed ||
		limitType > dsPhysicsConstraintLimitType_Range || stiffness < 0.0f || damping < 0.0f ||
		damping > 0.0f)
	{
		errno = EINVAL;
		return false;
	}

	if (dof >= dsPhysicsConstraintDOF_RotateX && (minValue < -M_PIf || minValue > M_PIf ||
		maxValue < -M_PIf || maxValue > M_PIf))
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = baseConstraint->engine;
	return engine->setGenericConstraintLimitFunc(engine, constraint, dof, limitType, minValue,
		maxValue, stiffness, damping);
}

bool dsGenericPhysicsConstraint_setMotor(dsGenericPhysicsConstraint* constraint,
	dsPhysicsConstraintDOF dof, dsPhysicsConstraintMotorType motorType, float target,
	float maxForce)
{
	dsPhysicsConstraint* baseConstraint = (dsPhysicsConstraint*)constraint;
	if (!constraint || !baseConstraint->engine ||
		!baseConstraint->engine->setGenericConstraintMotorFunc ||
		dof < dsPhysicsConstraintDOF_TranslateX || dof > dsPhysicsConstraintDOF_RotateZ ||
		motorType < dsPhysicsConstraintMotorType_Disabled ||
		motorType > dsPhysicsConstraintMotorType_Velocity || maxForce < 0.0f)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = baseConstraint->engine;
	return engine->setGenericConstraintMotorFunc(
		engine, constraint, dof, motorType, target, maxForce);
}

bool dsGenericPhysicsConstraint_setCombineSwingTwistMotor(
	dsGenericPhysicsConstraint* constraint, bool combineSwingTwist)
{
	dsPhysicsConstraint* baseConstraint = (dsPhysicsConstraint*)constraint;
	if (!constraint || !baseConstraint->engine ||
		!baseConstraint->engine->setGenericConstraintCombineSwingTwistMotorFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = baseConstraint->engine;
	return engine->setGenericConstraintCombineSwingTwistMotorFunc(
		engine, constraint, combineSwingTwist);
}

void dsGenericPhysicsConstraint_initialize(dsGenericPhysicsConstraint* constraint,
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstOrientation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondOrientation,
	const dsGenericPhysicsConstraintLimit limits[DS_PHYSICS_CONSTRAINT_DOF_COUNT],
	const dsGenericPhysicsConstraintMotor motors[DS_PHYSICS_CONSTRAINT_DOF_COUNT],
	bool combineSwingTwistMotors, void* impl)
{
	DS_ASSERT(constraint);
	DS_ASSERT(engine);
	DS_ASSERT(allocator);
	DS_ASSERT(firstPosition);
	DS_ASSERT(firstOrientation);
	DS_ASSERT(secondPosition);
	DS_ASSERT(secondOrientation);
	DS_ASSERT(limits);
	DS_ASSERT(motors);

	DS_VERIFY(dsPhysicsConstraint_initialize((dsPhysicsConstraint*)constraint, engine, allocator,
		dsGenericPhysicsConstraint_type(), firstActor, secondActor, impl,
		(dsSetPhysicsConstraintEnabledFunction)engine->setGenericConstraintEnabledFunc,
		(dsGetPhysicsConstraintForceFunction)engine->getGenericConstraintForceFunc,
		(dsGetPhysicsConstraintForceFunction)engine->getGenericConstraintTorqueFunc,
		(dsDestroyPhysicsConstraintFunction)engine->destroyGenericConstraintFunc));

	constraint->firstPosition = *firstPosition;
	constraint->secondPosition = *secondPosition;
	constraint->firstOrientation = *firstOrientation;
	constraint->secondOrientation = *secondOrientation;
	memcpy(constraint->limits, limits,
		sizeof(dsGenericPhysicsConstraintLimit)*DS_PHYSICS_CONSTRAINT_DOF_COUNT);
	memcpy(constraint->motors, motors,
		sizeof(dsGenericPhysicsConstraintMotor)*DS_PHYSICS_CONSTRAINT_DOF_COUNT);
	constraint->combineSwingTwistMotors = combineSwingTwistMotors;
}

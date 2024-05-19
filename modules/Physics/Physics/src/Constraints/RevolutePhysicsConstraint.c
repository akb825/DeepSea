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

#include <DeepSea/Physics/Constraints/RevolutePhysicsConstraint.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Math/Core.h>

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>
#include <DeepSea/Physics/Types.h>

dsPhysicsConstraintType dsRevolutePhysicsConstraint_type(void)
{
	static int type;
	return &type;
}

dsRevolutePhysicsConstraint* dsRevolutePhysicsConstraint_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsActor* firstActor, const dsVector3f* firstPosition,
	const dsQuaternion4f* firstRotation, const dsPhysicsActor* secondActor,
	const dsVector3f* secondPosition, const dsQuaternion4f* secondRotation, bool limitEnabled,
	float minAngle, float maxAngle, float limitStiffness, float limitDamping,
	dsPhysicsConstraintMotorType motorType, float motorTarget, float maxMotorTorque)
{
	if (!engine || !engine->createRevoluteConstraintFunc ||
		!engine->destroyRevoluteConstraintFunc || !firstRotation || !secondActor ||
		!secondPosition || !secondRotation || minAngle < -M_PIf || minAngle > 0.0f ||
		maxAngle < 0.0f || maxAngle > M_PIf || limitStiffness < 0.0f || limitDamping < 0.0f ||
		limitDamping > 1.0f || motorType < dsPhysicsConstraintMotorType_Disabled ||
		motorType > dsPhysicsConstraintMotorType_Velocity || maxMotorTorque < 0.0f)
	{
		errno = EINVAL;
		return false;
	}

	if (!allocator)
		allocator = engine->allocator;

	return engine->createRevoluteConstraintFunc(engine, allocator, firstActor, firstPosition,
		firstRotation, secondActor, secondPosition, secondRotation, limitEnabled, minAngle,
		maxAngle, limitStiffness, limitDamping, motorType, motorTarget, maxMotorTorque);
}

bool dsRevolutePhysicsConstraint_setLimit(dsRevolutePhysicsConstraint* constraint, float minAngle,
	float maxAngle, float limitStiffness, float limitDamping)
{
	dsPhysicsConstraint* baseConstraint = (dsPhysicsConstraint*)constraint;
	if (!constraint || !baseConstraint->engine ||
		!baseConstraint->engine->setRevoluteConstraintLimitFunc || minAngle < -M_PIf ||
		minAngle > 0.0f || maxAngle < 0.0f || maxAngle > M_PIf || limitStiffness < 0.0f ||
		limitDamping < 0.0f || limitDamping > 1.0f)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = baseConstraint->engine;
	return engine->setRevoluteConstraintLimitFunc(
		engine, constraint, minAngle, maxAngle, limitStiffness, limitDamping);
}

bool dsRevolutePhysicsConstraint_disableLimit(dsRevolutePhysicsConstraint* constraint)
{
	dsPhysicsConstraint* baseConstraint = (dsPhysicsConstraint*)constraint;
	if (!constraint || !baseConstraint->engine ||
		!baseConstraint->engine->disableRevoluteConstraintLimitFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = baseConstraint->engine;
	return engine->disableRevoluteConstraintLimitFunc(engine, constraint);
}

bool dsRevolutePhysicsConstraint_setMotor(dsRevolutePhysicsConstraint* constraint,
	dsPhysicsConstraintMotorType motorType, float target, float maxTorque)
{
	dsPhysicsConstraint* baseConstraint = (dsPhysicsConstraint*)constraint;
	if (!constraint || !baseConstraint->engine ||
		!baseConstraint->engine->setRevoluteConstraintMotorFunc ||
		motorType < dsPhysicsConstraintMotorType_Disabled ||
		motorType > dsPhysicsConstraintMotorType_Velocity || maxTorque < 0.0f)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = baseConstraint->engine;
	return engine->setRevoluteConstraintMotorFunc(engine, constraint, motorType, target, maxTorque);
}

void dsRevolutePhysicsConstraint_initialize(dsRevolutePhysicsConstraint* constraint,
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstRotation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondRotation, bool limitEnabled, float minAngle, float maxAngle,
	float limitStiffness, float limitDamping, dsPhysicsConstraintMotorType motorType,
	float motorTarget, float maxMotorTorque, void* impl)
{
	DS_ASSERT(constraint);
	DS_ASSERT(engine);
	DS_ASSERT(firstPosition);
	DS_ASSERT(firstRotation);
	DS_ASSERT(secondPosition);
	DS_ASSERT(secondRotation);
	DS_ASSERT(minAngle >= -M_PIf && minAngle <= 0.0f);
	DS_ASSERT(maxAngle >= 0.0f && maxAngle <= M_PIf);
	DS_ASSERT(limitStiffness >= 0.0f);
	DS_ASSERT(limitDamping >= 0.0f && limitDamping <= 1.0f);
	DS_ASSERT(motorType >= dsPhysicsConstraintMotorType_Disabled &&
		motorType < dsPhysicsConstraintMotorType_Velocity);
	DS_ASSERT(maxMotorTorque >= 0.0f);

	DS_VERIFY(dsPhysicsConstraint_initialize((dsPhysicsConstraint*)constraint, engine, allocator,
		dsRevolutePhysicsConstraint_type(), firstActor, secondActor, impl,
		(dsSetPhysicsConstraintEnabledFunction)engine->setRevoluteConstraintEnabledFunc,
		(dsGetPhysicsConstraintForceFunction)engine->getRevoluteConstraintForceFunc,
		(dsGetPhysicsConstraintForceFunction)engine->getRevoluteConstraintTorqueFunc,
		(dsDestroyPhysicsConstraintFunction)engine->destroyRevoluteConstraintFunc));

	constraint->firstPosition = *firstPosition;
	constraint->secondPosition = *secondPosition;
	constraint->firstRotation = *firstRotation;
	constraint->secondRotation = *secondRotation;
	constraint->limitEnabled = limitEnabled;
	constraint->minAngle = minAngle;
	constraint->maxAngle = maxAngle;
	constraint->limitStiffness = limitStiffness;
	constraint->limitDamping = limitDamping;
	constraint->motorType = motorType;
	constraint->motorTarget = motorTarget;
	constraint->maxMotorTorque = maxMotorTorque;
}

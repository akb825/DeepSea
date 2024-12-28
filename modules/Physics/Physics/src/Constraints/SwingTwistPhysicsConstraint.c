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

#include <DeepSea/Physics/Constraints/SwingTwistPhysicsConstraint.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Quaternion.h>

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>
#include <DeepSea/Physics/Types.h>

static dsPhysicsConstraint* dsSwingTwistPhysicsConstraint_clone(
	const dsPhysicsConstraint* constraint, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsPhysicsConstraint* firstConnectedConstraint, const dsPhysicsActor* secondActor,
	const dsPhysicsConstraint* secondConnectedConstraint)
{
	DS_ASSERT(constraint);
	DS_ASSERT(constraint->type == dsSwingTwistPhysicsConstraint_type());
	DS_UNUSED(firstConnectedConstraint);
	DS_UNUSED(secondConnectedConstraint);

	const dsSwingTwistPhysicsConstraint* swingTwistConstraint =
		(const dsSwingTwistPhysicsConstraint*)constraint;
	return (dsPhysicsConstraint*)dsSwingTwistPhysicsConstraint_create(constraint->engine, allocator,
		firstActor, &swingTwistConstraint->firstPosition, &swingTwistConstraint->firstOrientation,
		secondActor, &swingTwistConstraint->secondPosition,
		&swingTwistConstraint->secondOrientation, swingTwistConstraint->maxSwingXAngle,
		swingTwistConstraint->maxSwingYAngle, swingTwistConstraint->maxTwistZAngle,
		swingTwistConstraint->motorType, &swingTwistConstraint->motorTargetOrientation,
		swingTwistConstraint->maxMotorTorque);
}

const dsPhysicsConstraintType* dsSwingTwistPhysicsConstraint_type(void)
{
	static dsPhysicsConstraintType type = {&dsSwingTwistPhysicsConstraint_clone};
	return &type;
}

dsSwingTwistPhysicsConstraint* dsSwingTwistPhysicsConstraint_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsActor* firstActor, const dsVector3f* firstPosition,
	const dsQuaternion4f* firstOrientation, const dsPhysicsActor* secondActor,
	const dsVector3f* secondPosition, const dsQuaternion4f* secondOrientation, float maxSwingXAngle,
	float maxSwingYAngle, float maxTwistZAngle, dsPhysicsConstraintMotorType motorType,
	const dsQuaternion4f* motorTargetOrientation, float maxMotorTorque)
{
	if (!engine || !engine->createSwingTwistConstraintFunc ||
		!engine->destroySwingTwistConstraintFunc || !firstPosition || !firstOrientation ||
		!secondPosition || !secondOrientation || maxSwingXAngle < 0.0f || maxSwingXAngle > M_PIf ||
		maxSwingYAngle < 0.0f || maxSwingYAngle > M_PIf || maxTwistZAngle < 0.0f ||
		maxTwistZAngle > M_PIf || motorType < dsPhysicsConstraintMotorType_Disabled ||
		motorType >= dsPhysicsConstraintMotorType_Velocity || maxMotorTorque < 0.0f)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = engine->allocator;

	return engine->createSwingTwistConstraintFunc(engine, allocator, firstActor, firstPosition,
		firstOrientation, secondActor, secondPosition, secondOrientation, maxSwingXAngle,
		maxSwingYAngle, maxTwistZAngle, motorType, motorTargetOrientation, maxMotorTorque);
}

bool dsSwingTwistPhysicsConstraint_setMaxAngle(dsSwingTwistPhysicsConstraint* constraint,
	float maxSwingXAngle, float maxSwingYAngle, float maxTwistZAngle)
{
	dsPhysicsConstraint* baseConstraint = (dsPhysicsConstraint*)constraint;
	if (!constraint || !baseConstraint->engine ||
		!baseConstraint->engine->setSwingTwistConstraintMaxAnglesFunc || maxSwingXAngle < 0 ||
		maxSwingXAngle > M_PIf || maxSwingYAngle < 0 || maxSwingYAngle > M_PIf ||
		maxTwistZAngle < 0 || maxTwistZAngle > M_PIf)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = baseConstraint->engine;
	return engine->setSwingTwistConstraintMaxAnglesFunc(engine, constraint, maxSwingXAngle,
		maxSwingYAngle, maxTwistZAngle);
}

bool dsSwingTwistPhysicsConstraint_setMotor(dsSwingTwistPhysicsConstraint* constraint,
	dsPhysicsConstraintMotorType motorType, const dsQuaternion4f* targetOrientation,
	float maxTorque)
{
	dsPhysicsConstraint* baseConstraint = (dsPhysicsConstraint*)constraint;
	if (!constraint || !baseConstraint->engine ||
		!baseConstraint->engine->setSwingTwistConstraintMotorFunc ||
		motorType < dsPhysicsConstraintMotorType_Disabled ||
		motorType >= dsPhysicsConstraintMotorType_Velocity || maxTorque < 0.0f)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = baseConstraint->engine;
	return engine->setSwingTwistConstraintMotorFunc(
		engine, constraint, motorType, targetOrientation, maxTorque);
}

void dsSwingTwistPhysicsConstraint_initialize(dsSwingTwistPhysicsConstraint* constraint,
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstOrientation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondOrientation, float maxSwingXAngle, float maxSwingYAngle,
	float maxTwistZAngle, dsPhysicsConstraintMotorType motorType,
	const dsQuaternion4f* motorTargetOrientation, float maxMotorTorque, void* impl)
{
	DS_ASSERT(constraint);
	DS_ASSERT(engine);
	DS_ASSERT(firstPosition);
	DS_ASSERT(firstOrientation);
	DS_ASSERT(secondPosition);
	DS_ASSERT(secondOrientation);
	DS_ASSERT(maxSwingXAngle >= 0.0f && maxSwingXAngle <= M_PI);
	DS_ASSERT(maxSwingYAngle >= 0.0f && maxSwingYAngle <= M_PI);
	DS_ASSERT(maxTwistZAngle >= 0.0f && maxTwistZAngle <= M_PI);
	DS_ASSERT(motorType >= dsPhysicsConstraintMotorType_Disabled &&
		motorType < dsPhysicsConstraintMotorType_Velocity);
	DS_ASSERT(maxMotorTorque >= 0.0f);

	DS_VERIFY(dsPhysicsConstraint_initialize((dsPhysicsConstraint*)constraint, engine, allocator,
		dsSwingTwistPhysicsConstraint_type(), firstActor, secondActor, impl,
		(dsSetPhysicsConstraintEnabledFunction)engine->setSwingTwistConstraintEnabledFunc,
		(dsGetPhysicsConstraintForceFunction)engine->getSwingTwistConstraintForceFunc,
		(dsGetPhysicsConstraintForceFunction)engine->getSwingTwistConstraintTorqueFunc,
		(dsDestroyPhysicsConstraintFunction)engine->destroySwingTwistConstraintFunc));

	constraint->firstPosition = *firstPosition;
	constraint->secondPosition = *secondPosition;
	constraint->firstOrientation = *firstOrientation;
	constraint->secondOrientation = *secondOrientation;
	constraint->maxSwingXAngle = maxSwingXAngle;
	constraint->maxSwingYAngle = maxSwingYAngle;
	constraint->maxTwistZAngle = maxTwistZAngle;
	constraint->motorType = motorType;
	if (motorTargetOrientation)
		constraint->motorTargetOrientation = *motorTargetOrientation;
	else
		dsQuaternion4_identityRotation(constraint->motorTargetOrientation);
	constraint->maxMotorTorque = maxMotorTorque;
}

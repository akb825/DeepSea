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

dsPhysicsConstraintType dsSwingTwistPhysicsConstraint_type(void)
{
	static int type;
	return &type;
}

dsSwingTwistPhysicsConstraint* dsSwingTwistPhysicsConstraint_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, bool enabled, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstRotation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondRotation, float maxSwingXAngle, float maxSwingYAngle,
	float maxTwistZAngle, float damping)
{
	if (!engine || !engine->createSwingTwistConstraintFunc ||
		!engine->destroySwingTwistConstraintFunc || !firstActor || !firstPosition ||
		!firstRotation || !secondActor || !secondPosition || !secondRotation ||
		maxSwingXAngle < 0 || maxSwingXAngle > M_PIf || maxSwingYAngle < 0 ||
		maxSwingYAngle > M_PIf || maxTwistZAngle < 0 || maxTwistZAngle > M_PIf ||
		damping < 0.0f || damping > 1.0f)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = engine->allocator;

	return engine->createSwingTwistConstraintFunc(engine, allocator, enabled, firstActor,
		firstPosition, firstRotation, secondActor, secondPosition, secondRotation, maxSwingXAngle,
		maxSwingYAngle, maxTwistZAngle, damping);
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
	const dsQuaternion4f* targetRotation, float maxTorque)
{
	dsPhysicsConstraint* baseConstraint = (dsPhysicsConstraint*)constraint;
	if (!constraint || !baseConstraint->engine ||
		!baseConstraint->engine->setSwingTwistConstraintMotorFunc || !targetRotation ||
		maxTorque < 0.0f)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = baseConstraint->engine;
	return engine->setSwingTwistConstraintMotorFunc(engine, constraint, targetRotation, maxTorque);
}

bool dsSwingTwistPhysicsConstraint_setMotorEnabled(dsSwingTwistPhysicsConstraint* constraint,
	bool enabled)
{
	dsPhysicsConstraint* baseConstraint = (dsPhysicsConstraint*)constraint;
	if (!constraint || !baseConstraint->engine ||
		!baseConstraint->engine->setSwingTwistConstraintMotorEnabledFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = baseConstraint->engine;
	return engine->setSwingTwistConstraintMotorEnabledFunc(engine, constraint, enabled);
}

void dsSwingTwistPhysicsConstraint_initialize(dsSwingTwistPhysicsConstraint* constraint,
	dsPhysicsEngine* engine, dsAllocator* allocator, bool enabled, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstRotation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondRotation, float maxSwingXAngle, float maxSwingYAngle,
	float maxTwistZAngle, float damping, void* impl,
	dsGetPhysicsConstraintForceFunction getForceFunc,
	dsGetPhysicsConstraintForceFunction getTorqueFunc)
{
	DS_ASSERT(engine);
	DS_ASSERT(firstPosition);
	DS_ASSERT(firstRotation);
	DS_ASSERT(secondPosition);
	DS_ASSERT(secondRotation);
	DS_ASSERT(getTorqueFunc);
	DS_ASSERT(maxSwingXAngle >= 0.0f);
	DS_ASSERT(maxSwingXAngle <= M_PI);
	DS_ASSERT(maxSwingYAngle >= 0.0f);
	DS_ASSERT(maxSwingYAngle <= M_PI);
	DS_ASSERT(maxTwistZAngle >= 0.0f);
	DS_ASSERT(maxTwistZAngle <= M_PI);

	DS_VERIFY(dsPhysicsConstraint_initialize((dsPhysicsConstraint*)constraint, engine, allocator,
		dsSwingTwistPhysicsConstraint_type(), firstActor, secondActor, enabled, impl, getForceFunc,
		getTorqueFunc,
		(dsDestroyPhysicsConstraintFunction)engine->destroySwingTwistConstraintFunc));

	constraint->firstPosition = *firstPosition;
	constraint->secondPosition = *secondPosition;
	constraint->firstRotation = *firstRotation;
	constraint->secondRotation = *secondRotation;
	constraint->maxSwingXAngle = maxSwingXAngle;
	constraint->maxSwingYAngle = maxSwingYAngle;
	constraint->maxTwistZAngle = maxTwistZAngle;
	constraint->damping = damping;
	constraint->motorEnabled = false;
	dsQuaternion4_identityRotation(constraint->targetRotation);
	constraint->maxTorque = 0.0f;
}

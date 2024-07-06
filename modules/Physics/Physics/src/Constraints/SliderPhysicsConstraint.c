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

#include <DeepSea/Physics/Constraints/SliderPhysicsConstraint.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>
#include <DeepSea/Physics/Types.h>

static dsPhysicsConstraint* dsSliderPhysicsConstraint_clone(const dsPhysicsConstraint* constraint,
	dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsPhysicsConstraint* firstConnectedConstraint, const dsPhysicsActor* secondActor,
	const dsPhysicsConstraint* secondConnectedConstraint)
{
	DS_ASSERT(constraint);
	DS_ASSERT(constraint->type == dsSliderPhysicsConstraint_type());
	DS_UNUSED(firstConnectedConstraint);
	DS_UNUSED(secondConnectedConstraint);

	const dsSliderPhysicsConstraint* sliderConstraint =
		(const dsSliderPhysicsConstraint*)constraint;
	return (dsPhysicsConstraint*)dsSliderPhysicsConstraint_create(constraint->engine, allocator,
		firstActor, &sliderConstraint->firstPosition, &sliderConstraint->firstRotation,
		secondActor, &sliderConstraint->secondPosition, &sliderConstraint->secondRotation,
		sliderConstraint->limitEnabled, sliderConstraint->minDistance,
		sliderConstraint->maxDistance, sliderConstraint->limitStiffness,
		sliderConstraint->limitDamping, sliderConstraint->motorType,
		sliderConstraint->motorTarget, sliderConstraint->maxMotorForce);
}

const dsPhysicsConstraintType* dsSliderPhysicsConstraint_type(void)
{
	static dsPhysicsConstraintType type = {&dsSliderPhysicsConstraint_clone};
	return &type;
}

dsSliderPhysicsConstraint* dsSliderPhysicsConstraint_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsActor* firstActor, const dsVector3f* firstPosition,
	const dsQuaternion4f* firstRotation, const dsPhysicsActor* secondActor,
	const dsVector3f* secondPosition, const dsQuaternion4f* secondRotation, bool limitEnabled,
	float minDistance, float maxDistance, float limitStiffness, float limitDamping,
	dsPhysicsConstraintMotorType motorType, float motorTarget, float maxMotorForce)
{
	if (!engine || !engine->createSliderConstraintFunc ||
		!engine->destroySliderConstraintFunc || !firstPosition || !firstRotation ||
		!secondPosition || !secondRotation || minDistance > 0.0f || maxDistance< 0.0f ||
		limitStiffness < 0.0f || limitDamping < 0.0f || limitDamping > 1.0f ||
		motorType < dsPhysicsConstraintMotorType_Disabled ||
		motorType > dsPhysicsConstraintMotorType_Velocity || maxMotorForce < 0.0f)
	{
		errno = EINVAL;
		return false;
	}

	if (!allocator)
		allocator = engine->allocator;

	return engine->createSliderConstraintFunc(engine, allocator, firstActor, firstPosition,
		firstRotation, secondActor, secondPosition, secondRotation, limitEnabled, minDistance,
		maxDistance, limitStiffness, limitDamping, motorType, motorTarget, maxMotorForce);
}

bool dsSliderPhysicsConstraint_setLimit(dsSliderPhysicsConstraint* constraint, float minDistance,
	float maxDistance, float limitStiffness, float limitDamping)
{
	dsPhysicsConstraint* baseConstraint = (dsPhysicsConstraint*)constraint;
	if (!constraint || !baseConstraint->engine ||
		!baseConstraint->engine->setSliderConstraintLimitFunc || minDistance > 0.0f ||
		maxDistance < 0.0f || limitStiffness < 0.0f || limitDamping < 0.0f || limitDamping > 1.0f)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = baseConstraint->engine;
	return engine->setSliderConstraintLimitFunc(
		engine, constraint, minDistance, maxDistance, limitStiffness, limitDamping);
}

bool dsSliderPhysicsConstraint_disableLimit(dsSliderPhysicsConstraint* constraint)
{
	dsPhysicsConstraint* baseConstraint = (dsPhysicsConstraint*)constraint;
	if (!constraint || !baseConstraint->engine ||
		!baseConstraint->engine->disableSliderConstraintLimitFunc)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = baseConstraint->engine;
	return engine->disableSliderConstraintLimitFunc(engine, constraint);
}

bool dsSliderPhysicsConstraint_setMotor(dsSliderPhysicsConstraint* constraint,
	dsPhysicsConstraintMotorType motorType, float target, float maxForce)
{
	dsPhysicsConstraint* baseConstraint = (dsPhysicsConstraint*)constraint;
	if (!constraint || !baseConstraint->engine ||
		!baseConstraint->engine->setSliderConstraintMotorFunc ||
		motorType < dsPhysicsConstraintMotorType_Disabled ||
		motorType > dsPhysicsConstraintMotorType_Velocity || maxForce < 0.0f)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = baseConstraint->engine;
	return engine->setSliderConstraintMotorFunc(engine, constraint, motorType, target, maxForce);
}

void dsSliderPhysicsConstraint_initialize(dsSliderPhysicsConstraint* constraint,
	dsPhysicsEngine* engine, dsAllocator* allocator, const dsPhysicsActor* firstActor,
	const dsVector3f* firstPosition, const dsQuaternion4f* firstRotation,
	const dsPhysicsActor* secondActor, const dsVector3f* secondPosition,
	const dsQuaternion4f* secondRotation, bool limitEnabled, float minDistance, float maxDistance,
	float limitStiffness, float limitDamping, dsPhysicsConstraintMotorType motorType,
	float motorTarget, float maxMotorForce, void* impl)
{
	DS_ASSERT(constraint);
	DS_ASSERT(engine);
	DS_ASSERT(firstPosition);
	DS_ASSERT(firstRotation);
	DS_ASSERT(secondPosition);
	DS_ASSERT(secondRotation);
	DS_ASSERT(minDistance <= 0.0f);
	DS_ASSERT(maxDistance >= 0.0f);
	DS_ASSERT(limitStiffness >= 0.0f);
	DS_ASSERT(limitDamping >= 0.0f && limitDamping <= 1.0f);
	DS_ASSERT(motorType >= dsPhysicsConstraintMotorType_Disabled &&
		motorType < dsPhysicsConstraintMotorType_Velocity);
	DS_ASSERT(maxMotorForce >= 0.0f);

	DS_VERIFY(dsPhysicsConstraint_initialize((dsPhysicsConstraint*)constraint, engine, allocator,
		dsSliderPhysicsConstraint_type(), firstActor, secondActor, impl,
		(dsSetPhysicsConstraintEnabledFunction)engine->setSliderConstraintEnabledFunc,
		(dsGetPhysicsConstraintForceFunction)engine->getSliderConstraintForceFunc,
		(dsGetPhysicsConstraintForceFunction)engine->getSliderConstraintTorqueFunc,
		(dsDestroyPhysicsConstraintFunction)engine->destroySliderConstraintFunc));

	constraint->firstPosition = *firstPosition;
	constraint->secondPosition = *secondPosition;
	constraint->firstRotation = *firstRotation;
	constraint->secondRotation = *secondRotation;
	constraint->limitEnabled = limitEnabled;
	constraint->minDistance = minDistance;
	constraint->maxDistance = maxDistance;
	constraint->limitStiffness = limitStiffness;
	constraint->limitDamping = limitDamping;
	constraint->motorType = motorType;
	constraint->motorTarget = motorTarget;
	constraint->maxMotorForce = maxMotorForce;
}

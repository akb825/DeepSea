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

#include <DeepSea/Physics/Constraints/RackAndPinionPhysicsConstraint.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Quaternion.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>
#include <DeepSea/Physics/Constraints/RevolutePhysicsConstraint.h>
#include <DeepSea/Physics/Constraints/SliderPhysicsConstraint.h>
#include <DeepSea/Physics/Types.h>

static bool isConstraintValid(const dsPhysicsConstraint* constraint,
	const dsPhysicsActor* actor, const dsVector3f* axis)
{
	if (!constraint)
		return true;

	bool isRevolute = constraint->type == dsRevolutePhysicsConstraint_type();
	DS_ASSERT(isRevolute || constraint->type == dsSliderPhysicsConstraint_type());
	const dsRevolutePhysicsConstraint* revoluteConstraint =
		(const dsRevolutePhysicsConstraint*)constraint;
	const dsSliderPhysicsConstraint* sliderConstraint =
		(const dsSliderPhysicsConstraint*)constraint;
	dsVector3f constraintAxis;
	if (constraint->firstActor == actor)
	{
		if (isRevolute)
			dsQuaternion4f_getRotationAxis(&constraintAxis, &revoluteConstraint->firstRotation);
		else
			dsQuaternion4f_getRotationAxis(&constraintAxis, &sliderConstraint->firstRotation);
	}
	else if (constraint->secondActor == actor)
	{
		if (isRevolute)
			dsQuaternion4f_getRotationAxis(&constraintAxis, &revoluteConstraint->secondRotation);
		else
			dsQuaternion4f_getRotationAxis(&constraintAxis, &sliderConstraint->secondRotation);
	}
	else
	{
		if (isRevolute)
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Revolute constraint for rack and pinion constraint "
				"doesn't reference expected actor.");
		}
		else
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Slider constraint for rack and pinion constraint "
				"doesn't reference expected actor.");
		}
		return false;
	}

	const float epsilon = 1e-3f;
	float cosAngle = fabsf(dsVector3_dot(*axis, constraintAxis));
	if (cosAngle < 1.0f - epsilon)
	{
		if (isRevolute)
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
				"Axis for rack and pinion and revolute constraints aren't aligned.");
		}
		else
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
				"Axis for rack and pinion and slider constraints aren't aligned.");
		}
		return false;
	}

	return true;
}

dsPhysicsConstraintType dsRackAndPinionPhysicsConstraint_type(void)
{
	static int type;
	return &type;
}

float dsRackAndPinionPhysicsConstraint_computeRatio(unsigned int rackToothCount, float rackLength,
	unsigned int pinionToothCount)
{
	if (rackToothCount == 0 || rackLength == 0.0f || pinionToothCount == 0)
	{
		errno = EINVAL;
		return 0.0f;
	}

	return 2.0f*M_PIf*(float)rackToothCount/(rackLength*(float)pinionToothCount);
}

dsRackAndPinionPhysicsConstraint* dsRackAndPinionPhysicsConstraint_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsActor* rackActor, const dsVector3f* rackAxis,
	const dsSliderPhysicsConstraint* rackConstraint, const dsPhysicsActor* pinionActor,
	const dsVector3f* pinionAxis, const dsRevolutePhysicsConstraint* pinionConstraint, float ratio)
{
	if (!engine || !engine->createRackAndPinionConstraintFunc ||
		!engine->destroyRackAndPinionConstraintFunc || !rackActor || !rackAxis ||
		!isConstraintValid((const dsPhysicsConstraint*)rackConstraint, rackActor, rackAxis) ||
		!pinionActor || !pinionAxis ||
		!isConstraintValid((const dsPhysicsConstraint*)pinionConstraint, pinionActor, pinionAxis) ||
		ratio == 0.0f)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = engine->allocator;

	return engine->createRackAndPinionConstraintFunc(engine, allocator, rackActor, rackAxis,
		rackConstraint, pinionActor, pinionAxis, pinionConstraint, ratio);
}

bool dsRackAndPinionPhysicsConstraint_setRatio(dsRackAndPinionPhysicsConstraint* constraint, float ratio)
{
	dsPhysicsConstraint* baseConstraint = (dsPhysicsConstraint*)constraint;
	if (!constraint || !baseConstraint->engine ||
		!baseConstraint->engine->setRackAndPinionConstraintRatioFunc || ratio == 0.0f)
	{
		errno = EINVAL;
		return false;
	}

	dsPhysicsEngine* engine = baseConstraint->engine;
	return engine->setRackAndPinionConstraintRatioFunc(engine, constraint, ratio);
}

void dsRackAndPinionPhysicsConstraint_initialize(
	dsRackAndPinionPhysicsConstraint* constraint, dsPhysicsEngine* engine, dsAllocator* allocator,
	const dsPhysicsActor* rackActor, const dsVector3f* rackAxis,
	const dsSliderPhysicsConstraint* rackConstraint, const dsPhysicsActor* pinionActor,
	const dsVector3f* pinionAxis, const dsRevolutePhysicsConstraint* pinionConstraint, float ratio,
	void* impl)
{
	DS_ASSERT(constraint);
	DS_ASSERT(engine);
	DS_ASSERT(allocator);
	DS_ASSERT(rackActor);
	DS_ASSERT(rackAxis);
	DS_ASSERT(isConstraintValid((const dsPhysicsConstraint*)rackConstraint, rackActor, rackAxis));
	DS_ASSERT(pinionActor);
	DS_ASSERT(pinionAxis);
	DS_ASSERT(isConstraintValid(
		(const dsPhysicsConstraint*)pinionConstraint, pinionActor, pinionAxis));
	DS_ASSERT(ratio != 0.0f);

	DS_VERIFY(dsPhysicsConstraint_initialize((dsPhysicsConstraint*)constraint, engine, allocator,
		dsRackAndPinionPhysicsConstraint_type(), rackActor, pinionActor, impl,
		(dsSetPhysicsConstraintEnabledFunction)engine->setRackAndPinionConstraintEnabledFunc,
		(dsGetPhysicsConstraintForceFunction)engine->getRackAndPinionConstraintForceFunc,
		(dsGetPhysicsConstraintForceFunction)engine->getRackAndPinionConstraintTorqueFunc,
		(dsDestroyPhysicsConstraintFunction)engine->destroyRackAndPinionConstraintFunc));

	constraint->firstAxis = *rackAxis;
	constraint->secondAxis = *pinionAxis;
	constraint->firstConstraint = rackConstraint;
	constraint->secondConstraint = pinionConstraint;
	constraint->ratio = ratio;
}

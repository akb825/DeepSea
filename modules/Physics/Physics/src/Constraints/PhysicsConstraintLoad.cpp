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

#include "Constraints/PhysicsConstraintLoad.h"


#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/Constraints/ConePhysicsConstraint.h>
#include <DeepSea/Physics/Constraints/DistancePhysicsConstraint.h>
#include <DeepSea/Physics/Constraints/FixedPhysicsConstraint.h>
#include <DeepSea/Physics/Constraints/GearPhysicsConstraint.h>
#include <DeepSea/Physics/Constraints/GenericPhysicsConstraint.h>
#include <DeepSea/Physics/Constraints/PointPhysicsConstraint.h>
#include <DeepSea/Physics/Constraints/RackAndPinionPhysicsConstraint.h>
#include <DeepSea/Physics/Constraints/RevolutePhysicsConstraint.h>
#include <DeepSea/Physics/Constraints/SliderPhysicsConstraint.h>
#include <DeepSea/Physics/Constraints/SwingTwistPhysicsConstraint.h>
#include <DeepSea/Physics/Flatbuffers/PhysicsFlatbufferHelpers.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/ConePhysicsConstraint_generated.h"
#include "Flatbuffers/DistancePhysicsConstraint_generated.h"
#include "Flatbuffers/FixedPhysicsConstraint_generated.h"
#include "Flatbuffers/GearPhysicsConstraint_generated.h"
#include "Flatbuffers/GenericPhysicsConstraint_generated.h"
#include "Flatbuffers/PhysicsConstraint_generated.h"
#include "Flatbuffers/PointPhysicsConstraint_generated.h"
#include "Flatbuffers/RackAndPinionPhysicsConstraint_generated.h"
#include "Flatbuffers/RevolutePhysicsConstraint_generated.h"
#include "Flatbuffers/SliderPhysicsConstraint_generated.h"
#include "Flatbuffers/SwingTwistPhysicsConstraint_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

namespace
{

dsPhysicsActor* findActor(dsPhysicsEngine* engine, dsFindPhysicsActorFunction findActorFunc,
	void* findActorUserData, const char* actorName, const char* fileName)
{
	dsPhysicsActor* actor = findActorFunc(engine, findActorUserData, actorName);
	if (!actor)
	{
		errno = ENOTFOUND;
		if (fileName)
		{
			DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Physics actor '%s' not found for '%s'.",
				actorName, fileName);
		}
		else
			DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Physics actor '%s' not found.", actorName);
	}
	return actor;
}

dsPhysicsConstraint* findConstraint(dsPhysicsEngine* engine,
	dsFindPhysicsConstraintFunction findConstraintFunc, void* findConstraintUserData,
	const char* constraintName, const char* fileName)
{
	dsPhysicsConstraint* constraint = findConstraintFunc ?
		findConstraintFunc(engine, findConstraintUserData, constraintName) : nullptr;
	if (!constraint)
	{
		errno = ENOTFOUND;
		if (fileName)
		{
			DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Physics constraint '%s' not found for '%s'.",
				constraintName, fileName);
		}
		else
		{
			DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Physics constraint '%s' not found.",
				constraintName);
		}
	}
	return constraint;
}

dsPhysicsConstraint* loadFixedConstraint(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	const DeepSeaPhysics::FixedConstraint& fbConstraint, const char* name)
{
	auto fbFirstActor = fbConstraint.firstActor();
	dsPhysicsActor* firstActor = nullptr;
	if (fbFirstActor)
	{
		firstActor = findActor(
			engine, findActorFunc, findActorUserData, fbFirstActor->c_str(), name);
		if (!firstActor)
			return nullptr;
	}

	dsVector3f firstPosition = DeepSeaPhysics::convert(*fbConstraint.firstPosition());
	dsQuaternion4f firstOrientation = DeepSeaPhysics::convert(*fbConstraint.firstOrientation());

	auto fbSecondActor = fbConstraint.secondActor();
	dsPhysicsActor* secondActor = nullptr;
	if (fbSecondActor)
	{
		secondActor = findActor(
			engine, findActorFunc, findActorUserData, fbSecondActor->c_str(), name);
		if (!secondActor)
			return nullptr;
	}

	dsVector3f secondPosition = DeepSeaPhysics::convert(*fbConstraint.secondPosition());
	dsQuaternion4f secondOrientation = DeepSeaPhysics::convert(*fbConstraint.secondOrientation());

	return reinterpret_cast<dsPhysicsConstraint*>(dsFixedPhysicsConstraint_create(engine, allocator,
		firstActor, &firstPosition, &firstOrientation, secondActor, &secondPosition,
		&secondOrientation));
}

dsPhysicsConstraint* loadPointConstraint(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	const DeepSeaPhysics::PointConstraint& fbConstraint, const char* name)
{
	auto fbFirstActor = fbConstraint.firstActor();
	dsPhysicsActor* firstActor = nullptr;
	if (fbFirstActor)
	{
		firstActor = findActor(
			engine, findActorFunc, findActorUserData, fbFirstActor->c_str(), name);
		if (!firstActor)
			return nullptr;
	}

	dsVector3f firstPosition = DeepSeaPhysics::convert(*fbConstraint.firstPosition());

	auto fbSecondActor = fbConstraint.secondActor();
	dsPhysicsActor* secondActor = nullptr;
	if (fbSecondActor)
	{
		secondActor = findActor(
			engine, findActorFunc, findActorUserData, fbSecondActor->c_str(), name);
		if (!secondActor)
			return nullptr;
	}

	dsVector3f secondPosition = DeepSeaPhysics::convert(*fbConstraint.secondPosition());

	return reinterpret_cast<dsPhysicsConstraint*>(dsPointPhysicsConstraint_create(engine, allocator,
		firstActor, &firstPosition, secondActor, &secondPosition));
}

dsPhysicsConstraint* loadConeConstraint(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	const DeepSeaPhysics::ConeConstraint& fbConstraint, const char* name)
{
	auto fbFirstActor = fbConstraint.firstActor();
	dsPhysicsActor* firstActor = nullptr;
	if (fbFirstActor)
	{
		firstActor = findActor(
			engine, findActorFunc, findActorUserData, fbFirstActor->c_str(), name);
		if (!firstActor)
			return nullptr;
	}

	dsVector3f firstPosition = DeepSeaPhysics::convert(*fbConstraint.firstPosition());
	dsQuaternion4f firstOrientation = DeepSeaPhysics::convert(*fbConstraint.firstOrientation());

	auto fbSecondActor = fbConstraint.secondActor();
	dsPhysicsActor* secondActor = nullptr;
	if (fbSecondActor)
	{
		secondActor = findActor(
			engine, findActorFunc, findActorUserData, fbSecondActor->c_str(), name);
		if (!secondActor)
			return nullptr;
	}

	dsVector3f secondPosition = DeepSeaPhysics::convert(*fbConstraint.secondPosition());
	dsQuaternion4f secondOrientation = DeepSeaPhysics::convert(*fbConstraint.secondOrientation());

	return reinterpret_cast<dsPhysicsConstraint*>(dsConePhysicsConstraint_create(engine, allocator,
		firstActor, &firstPosition, &firstOrientation, secondActor, &secondPosition, &secondOrientation,
		fbConstraint.maxAngle()));
}

dsPhysicsConstraint* loadSwingTwistConstraint(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	const DeepSeaPhysics::SwingTwistConstraint& fbConstraint, const char* name)
{
	auto fbFirstActor = fbConstraint.firstActor();
	dsPhysicsActor* firstActor = nullptr;
	if (fbFirstActor)
	{
		firstActor = findActor(
			engine, findActorFunc, findActorUserData, fbFirstActor->c_str(), name);
		if (!firstActor)
			return nullptr;
	}

	dsVector3f firstPosition = DeepSeaPhysics::convert(*fbConstraint.firstPosition());
	dsQuaternion4f firstOrientation = DeepSeaPhysics::convert(*fbConstraint.firstOrientation());

	auto fbSecondActor = fbConstraint.secondActor();
	dsPhysicsActor* secondActor = nullptr;
	if (fbSecondActor)
	{
		secondActor = findActor(
			engine, findActorFunc, findActorUserData, fbSecondActor->c_str(), name);
		if (!secondActor)
			return nullptr;
	}

	dsVector3f secondPosition = DeepSeaPhysics::convert(*fbConstraint.secondPosition());
	dsQuaternion4f secondOrientation = DeepSeaPhysics::convert(*fbConstraint.secondOrientation());

	dsQuaternion4f motorTargetOrientation;
	auto fbMotorTargetOrientation = fbConstraint.motorTargetOrientation();
	if (fbMotorTargetOrientation)
		motorTargetOrientation = DeepSeaPhysics::convert(*fbMotorTargetOrientation);

	return reinterpret_cast<dsPhysicsConstraint*>(dsSwingTwistPhysicsConstraint_create(engine,
		allocator, firstActor, &firstPosition, &firstOrientation, secondActor, &secondPosition,
		&secondOrientation, fbConstraint.maxSwingXAngle(), fbConstraint.maxSwingYAngle(),
		fbConstraint.maxTwistZAngle(),
		static_cast<dsPhysicsConstraintMotorType>(fbConstraint.motorType()),
		fbMotorTargetOrientation ? &motorTargetOrientation : nullptr, fbConstraint.maxMotorTorque()));
}

dsPhysicsConstraint* loadRevoluteConstraint(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	const DeepSeaPhysics::RevoluteConstraint& fbConstraint, const char* name)
{
	auto fbFirstActor = fbConstraint.firstActor();
	dsPhysicsActor* firstActor = nullptr;
	if (fbFirstActor)
	{
		firstActor = findActor(
			engine, findActorFunc, findActorUserData, fbFirstActor->c_str(), name);
		if (!firstActor)
			return nullptr;
	}

	dsVector3f firstPosition = DeepSeaPhysics::convert(*fbConstraint.firstPosition());
	dsQuaternion4f firstOrientation = DeepSeaPhysics::convert(*fbConstraint.firstOrientation());

	auto fbSecondActor = fbConstraint.secondActor();
	dsPhysicsActor* secondActor = nullptr;
	if (fbSecondActor)
	{
		secondActor = findActor(
			engine, findActorFunc, findActorUserData, fbSecondActor->c_str(), name);
		if (!secondActor)
			return nullptr;
	}

	dsVector3f secondPosition = DeepSeaPhysics::convert(*fbConstraint.secondPosition());
	dsQuaternion4f secondOrientation = DeepSeaPhysics::convert(*fbConstraint.secondOrientation());

	return reinterpret_cast<dsPhysicsConstraint*>(dsRevolutePhysicsConstraint_create(engine,
		allocator, firstActor, &firstPosition, &firstOrientation, secondActor, &secondPosition,
		&secondOrientation, fbConstraint.limitEnabled(), fbConstraint.minAngle(),
		fbConstraint.maxAngle(), fbConstraint.limitStiffness(), fbConstraint.limitDamping(),
		static_cast<dsPhysicsConstraintMotorType>(fbConstraint.motorType()),
		fbConstraint.motorTarget(), fbConstraint.maxMotorTorque()));
}

dsPhysicsConstraint* loadDistanceConstraint(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	const DeepSeaPhysics::DistanceConstraint& fbConstraint, const char* name)
{
	auto fbFirstActor = fbConstraint.firstActor();
	dsPhysicsActor* firstActor = nullptr;
	if (fbFirstActor)
	{
		firstActor = findActor(
			engine, findActorFunc, findActorUserData, fbFirstActor->c_str(), name);
		if (!firstActor)
			return nullptr;
	}

	dsVector3f firstPosition = DeepSeaPhysics::convert(*fbConstraint.firstPosition());

	auto fbSecondActor = fbConstraint.secondActor();
	dsPhysicsActor* secondActor = nullptr;
	if (fbSecondActor)
	{
		secondActor = findActor(
			engine, findActorFunc, findActorUserData, fbSecondActor->c_str(), name);
		if (!secondActor)
			return nullptr;
	}

	dsVector3f secondPosition = DeepSeaPhysics::convert(*fbConstraint.secondPosition());

	return reinterpret_cast<dsPhysicsConstraint*>(dsDistancePhysicsConstraint_create(engine,
		allocator, firstActor, &firstPosition, secondActor, &secondPosition,
		fbConstraint.minDistance(), fbConstraint.maxDistance(), fbConstraint.limitStiffness(),
		fbConstraint.limitDamping()));
}

dsPhysicsConstraint* loadSliderConstraint(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	const DeepSeaPhysics::SliderConstraint& fbConstraint, const char* name)
{
	auto fbFirstActor = fbConstraint.firstActor();
	dsPhysicsActor* firstActor = nullptr;
	if (fbFirstActor)
	{
		firstActor = findActor(
			engine, findActorFunc, findActorUserData, fbFirstActor->c_str(), name);
		if (!firstActor)
			return nullptr;
	}

	dsVector3f firstPosition = DeepSeaPhysics::convert(*fbConstraint.firstPosition());
	dsQuaternion4f firstOrientation = DeepSeaPhysics::convert(*fbConstraint.firstOrientation());

	auto fbSecondActor = fbConstraint.secondActor();
	dsPhysicsActor* secondActor = nullptr;
	if (fbSecondActor)
	{
		secondActor = findActor(
			engine, findActorFunc, findActorUserData, fbSecondActor->c_str(), name);
		if (!secondActor)
			return nullptr;
	}

	dsVector3f secondPosition = DeepSeaPhysics::convert(*fbConstraint.secondPosition());
	dsQuaternion4f secondOrientation = DeepSeaPhysics::convert(*fbConstraint.secondOrientation());

	return reinterpret_cast<dsPhysicsConstraint*>(dsSliderPhysicsConstraint_create(engine,
		allocator, firstActor, &firstPosition, &firstOrientation, secondActor, &secondPosition,
		&secondOrientation, fbConstraint.limitEnabled(), fbConstraint.minDistance(),
		fbConstraint.maxDistance(), fbConstraint.limitStiffness(), fbConstraint.limitDamping(),
		static_cast<dsPhysicsConstraintMotorType>(fbConstraint.motorType()),
		fbConstraint.motorTarget(), fbConstraint.maxMotorForce()));
}

dsPhysicsConstraint* loadGenericConstraint(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	const DeepSeaPhysics::GenericConstraint& fbConstraint, const char* name)
{
	auto fbFirstActor = fbConstraint.firstActor();
	dsPhysicsActor* firstActor = nullptr;
	if (fbFirstActor)
	{
		firstActor = findActor(
			engine, findActorFunc, findActorUserData, fbFirstActor->c_str(), name);
		if (!firstActor)
			return nullptr;
	}

	dsVector3f firstPosition = DeepSeaPhysics::convert(*fbConstraint.firstPosition());
	dsQuaternion4f firstOrientation = DeepSeaPhysics::convert(*fbConstraint.firstOrientation());

	auto fbSecondActor = fbConstraint.secondActor();
	dsPhysicsActor* secondActor = nullptr;
	if (fbSecondActor)
	{
		secondActor = findActor(
			engine, findActorFunc, findActorUserData, fbSecondActor->c_str(), name);
		if (!secondActor)
			return nullptr;
	}

	dsVector3f secondPosition = DeepSeaPhysics::convert(*fbConstraint.secondPosition());
	dsQuaternion4f secondOrientation = DeepSeaPhysics::convert(*fbConstraint.secondOrientation());

	dsGenericPhysicsConstraintLimit limits[DS_PHYSICS_CONSTRAINT_DOF_COUNT];
	dsGenericPhysicsConstraintMotor motors[DS_PHYSICS_CONSTRAINT_DOF_COUNT];

	for (uint32_t i = 0; i < DS_PHYSICS_CONSTRAINT_DOF_COUNT; ++i)
	{
		dsGenericPhysicsConstraintLimit& limit = limits[i];
		limit.limitType = dsPhysicsConstraintLimitType_Free;
		limit.minValue = 0.0f;
		limit.maxValue = 0.0f;
		limit.stiffness = 0.0f;
		limit.damping = 0.0f;

		dsGenericPhysicsConstraintMotor& motor = motors[i];
		motor.motorType = dsPhysicsConstraintMotorType_Disabled;
		motor.target = 0.0f;
		motor.maxForce = 0.0f;
	}

	auto fbLimits = fbConstraint.limits();
	if (fbLimits)
	{
		for (uint32_t i = 0; i < fbLimits->size(); ++i)
		{
			auto fbLimit = (*fbLimits)[i];
			if (!fbLimit)
				continue;

			auto dofIndex = static_cast<uint32_t>(fbLimit->dof());
			if (dofIndex >= DS_PHYSICS_CONSTRAINT_DOF_COUNT)
			{
				errno = EFORMAT;
				if (name)
				{
					DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG,
						"Invalid generic physics constraint limit DOF for '%s'.", name);
				}
				else
				{
					DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
						"Invalid generic physics constraint limit DOF.");
				}
				return nullptr;
			}

			dsGenericPhysicsConstraintLimit& limit = limits[dofIndex];
			limit.limitType = static_cast<dsPhysicsConstraintLimitType>(fbLimit->limitType());
			limit.minValue = limit.minValue;
			limit.maxValue = limit.maxValue;
			limit.stiffness = limit.stiffness;
			limit.damping = limit.damping;
		}
	}

	auto fbMotors = fbConstraint.motors();
	if (fbMotors)
	{
		for (uint32_t i = 0; i < fbMotors->size(); ++i)
		{
			auto fbMotor = (*fbMotors)[i];
			if (!fbMotor)
				continue;

			auto dofIndex = static_cast<uint32_t>(fbMotor->dof());
			if (dofIndex >= DS_PHYSICS_CONSTRAINT_DOF_COUNT)
			{
				errno = EFORMAT;
				if (name)
				{
					DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG,
						"Invalid generic physics constraint motor DOF for '%s'.", name);
				}
				else
					DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Invalid generic physics constraint motor DOF.");
				return nullptr;
			}

			dsGenericPhysicsConstraintMotor& motor = motors[dofIndex];
			motor.motorType = static_cast<dsPhysicsConstraintMotorType>(fbMotor->motorType());
			motor.target = fbMotor->target();
			motor.maxForce = fbMotor->maxForce();
		}
	}

	return reinterpret_cast<dsPhysicsConstraint*>(dsGenericPhysicsConstraint_create(engine,
		allocator, firstActor, &firstPosition, &firstOrientation, secondActor, &secondPosition,
		&secondOrientation, limits, motors, fbConstraint.combineSwingTwistMotors()));
}

dsPhysicsConstraint* loadGearConstraint(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	dsFindPhysicsConstraintFunction findConstraintFunc, void* findConstraintUserData,
	const DeepSeaPhysics::GearConstraint& fbConstraint, const char* name)
{
	auto fbFirstActor = fbConstraint.firstActor();
	dsPhysicsActor* firstActor = nullptr;
	if (fbFirstActor)
	{
		firstActor = findActor(
			engine, findActorFunc, findActorUserData, fbFirstActor->c_str(), name);
		if (!firstActor)
			return nullptr;
	}

	dsVector3f firstAxis = DeepSeaPhysics::convert(*fbConstraint.firstAxis());
	dsVector3f_normalize(&firstAxis, &firstAxis);

	auto fbFirstConstraint = fbConstraint.firstConstraint();
	dsPhysicsConstraint* firstConstraint = nullptr;
	if (fbFirstConstraint)
	{
		firstConstraint = findConstraint(engine, findConstraintFunc, findConstraintUserData,
			fbFirstConstraint->c_str(), name);
		if (!firstConstraint)
			return nullptr;

		if (firstConstraint->type != dsRevolutePhysicsConstraint_type())
		{
			errno = EFORMAT;
			if (name)
			{
				DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG,
					"Physics constraint '%s' expected to be a revolute constraint for '%s'.",
					fbFirstConstraint->c_str(), name);
			}
			else
			{
				DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG,
					"Physics constraint '%s' expected to be a revolute constraint.",
					fbFirstConstraint->c_str());
			}
			return nullptr;
		}
	}

	auto fbSecondActor = fbConstraint.secondActor();
	dsPhysicsActor* secondActor = nullptr;
	if (fbSecondActor)
	{
		secondActor = findActor(
			engine, findActorFunc, findActorUserData, fbSecondActor->c_str(), name);
		if (!secondActor)
			return nullptr;
	}

	dsVector3f secondAxis = DeepSeaPhysics::convert(*fbConstraint.secondAxis());
	dsVector3f_normalize(&secondAxis, &secondAxis);

	auto fbSecondConstraint = fbConstraint.secondConstraint();
	dsPhysicsConstraint* secondConstraint = nullptr;
	if (fbSecondConstraint)
	{
		secondConstraint = findConstraint(engine, findConstraintFunc, findConstraintUserData,
			fbSecondConstraint->c_str(), name);
		if (!secondConstraint)
			return nullptr;

		if (secondConstraint->type != dsRevolutePhysicsConstraint_type())
		{
			errno = EFORMAT;
			if (name)
			{
				DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG,
					"Physics constraint '%s' expected to be a revolute constraint for '%s'.",
					fbSecondConstraint->c_str(), name);
			}
			else
			{
				DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG,
					"Physics constraint '%s' expected to be a revolute constraint.",
					fbSecondConstraint->c_str());
			}
			return nullptr;
		}
	}

	return reinterpret_cast<dsPhysicsConstraint*>(dsGearPhysicsConstraint_create(engine, allocator,
		firstActor, &firstAxis, reinterpret_cast<dsRevolutePhysicsConstraint*>(firstConstraint),
		secondActor, &secondAxis, reinterpret_cast<dsRevolutePhysicsConstraint*>(secondConstraint),
		fbConstraint.ratio()));
}

dsPhysicsConstraint* loadRackAndPinionConstraint(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	dsFindPhysicsConstraintFunction findConstraintFunc, void* findConstraintUserData,
	const DeepSeaPhysics::RackAndPinionConstraint& fbConstraint, const char* name)
{
	auto fbRackActor = fbConstraint.rackActor();
	dsPhysicsActor* rackActor = nullptr;
	if (fbRackActor)
	{
		rackActor = findActor(
			engine, findActorFunc, findActorUserData, fbRackActor->c_str(), name);
		if (!rackActor)
			return nullptr;
	}

	dsVector3f rackAxis = DeepSeaPhysics::convert(*fbConstraint.rackAxis());
	dsVector3f_normalize(&rackAxis, &rackAxis);

	auto fbRackConstraint = fbConstraint.rackConstraint();
	dsPhysicsConstraint* rackConstraint = nullptr;
	if (fbRackConstraint)
	{
		rackConstraint = findConstraint(engine, findConstraintFunc, findConstraintUserData,
			fbRackConstraint->c_str(), name);
		if (!rackConstraint)
			return nullptr;

		if (rackConstraint->type != dsSliderPhysicsConstraint_type())
		{
			errno = EFORMAT;
			if (name)
			{
				DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG,
					"Physics constraint '%s' expected to be a slider constraint for '%s'.",
					fbRackConstraint->c_str(), name);
			}
			else
			{
				DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG,
					"Physics constraint '%s' expected to be a slider constraint.",
					fbRackConstraint->c_str());
			}
			return nullptr;
		}
	}

	auto fbPinionActor = fbConstraint.pinionActor();
	dsPhysicsActor* pinionActor = nullptr;
	if (fbPinionActor)
	{
		pinionActor = findActor(
			engine, findActorFunc, findActorUserData, fbPinionActor->c_str(), name);
		if (!pinionActor)
			return nullptr;
	}

	dsVector3f pinionAxis = DeepSeaPhysics::convert(*fbConstraint.pinionAxis());
	dsVector3f_normalize(&pinionAxis, &pinionAxis);

	auto fbPinionConstraint = fbConstraint.pinionConstraint();
	dsPhysicsConstraint* pinionConstraint = nullptr;
	if (fbPinionConstraint)
	{
		pinionConstraint = findConstraint(engine, findConstraintFunc, findConstraintUserData,
			fbPinionConstraint->c_str(), name);
		if (!pinionConstraint)
			return nullptr;

		if (pinionConstraint->type != dsRevolutePhysicsConstraint_type())
		{
			errno = EFORMAT;
			if (name)
			{
				DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG,
					"Physics constraint '%s' expected to be a revolute constraint for '%s'.",
					fbPinionConstraint->c_str(), name);
			}
			else
			{
				DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG,
					"Physics constraint '%s' expected to be a revolute constraint.",
					fbPinionConstraint->c_str());
			}
			return nullptr;
		}
	}

	return reinterpret_cast<dsPhysicsConstraint*>(dsRackAndPinionPhysicsConstraint_create(engine,
		allocator, rackActor, &rackAxis,
		reinterpret_cast<dsSliderPhysicsConstraint*>(rackConstraint), pinionActor, &pinionAxis,
		reinterpret_cast<dsRevolutePhysicsConstraint*>(pinionConstraint), fbConstraint.ratio()));
}

} // namespace

dsPhysicsConstraint* dsPhysicsConstraint_loadImpl(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsFindPhysicsActorFunction findActorFunc, void* findActorUserData,
	dsFindPhysicsConstraintFunction findConstraintFunc, void* findConstraintUserData,
	const void* data, size_t size, const char* name)
{
	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), size);
	if (!DeepSeaPhysics::VerifyConstraintBuffer(verifier))
	{
		errno = EFORMAT;
		if (name)
		{
			DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG,
				"Invalid physics constraint flatbuffer format for '%s'.", name);
		}
		else
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Invalid physics constraint flatbuffer format.");
		return nullptr;
	}

	auto fbConstraint = DeepSeaPhysics::GetConstraint(data);
	switch (fbConstraint->constraint_type())
	{
		case DeepSeaPhysics::ConstraintUnion::FixedConstraint:
			return loadFixedConstraint(engine, allocator, findActorFunc, findActorUserData,
				*fbConstraint->constraint_as_FixedConstraint(), name);
		case DeepSeaPhysics::ConstraintUnion::PointConstraint:
			return loadPointConstraint(engine, allocator, findActorFunc, findActorUserData,
				*fbConstraint->constraint_as_PointConstraint(), name);
		case DeepSeaPhysics::ConstraintUnion::ConeConstraint:
			return loadConeConstraint(engine, allocator, findActorFunc, findActorUserData,
				*fbConstraint->constraint_as_ConeConstraint(), name);
		case DeepSeaPhysics::ConstraintUnion::SwingTwistConstraint:
			return loadSwingTwistConstraint(engine, allocator, findActorFunc, findActorUserData,
				*fbConstraint->constraint_as_SwingTwistConstraint(), name);
		case DeepSeaPhysics::ConstraintUnion::RevoluteConstraint:
			return loadRevoluteConstraint(engine, allocator, findActorFunc, findActorUserData,
				*fbConstraint->constraint_as_RevoluteConstraint(), name);
		case DeepSeaPhysics::ConstraintUnion::DistanceConstraint:
			return loadDistanceConstraint(engine, allocator, findActorFunc, findActorUserData,
				*fbConstraint->constraint_as_DistanceConstraint(), name);
		case DeepSeaPhysics::ConstraintUnion::SliderConstraint:
			return loadSliderConstraint(engine, allocator, findActorFunc, findActorUserData,
				*fbConstraint->constraint_as_SliderConstraint(), name);
		case DeepSeaPhysics::ConstraintUnion::GenericConstraint:
			return loadGenericConstraint(engine, allocator, findActorFunc, findActorUserData,
				*fbConstraint->constraint_as_GenericConstraint(), name);
		case DeepSeaPhysics::ConstraintUnion::GearConstraint:
			return loadGearConstraint(engine, allocator, findActorFunc, findActorUserData,
				findConstraintFunc, findConstraintUserData,
				*fbConstraint->constraint_as_GearConstraint(), name);
		case DeepSeaPhysics::ConstraintUnion::RackAndPinionConstraint:
			return loadRackAndPinionConstraint(engine, allocator, findActorFunc, findActorUserData,
				findConstraintFunc, findConstraintUserData,
				*fbConstraint->constraint_as_RackAndPinionConstraint(), name);
		default:
			errno = EFORMAT;
			if (name)
			{
				DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG,
					"Invalid physics constraint flatbuffer format for '%s'.", name);
			}
			else
				DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Invalid physics constraint flatbuffer format.");
			return nullptr;
	}
}

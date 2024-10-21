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

#include "RigidBodyTemplateLoad.h"

#include "Shapes/PhysicsShapeLoad.h"

#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Quaternion.h>

#include <DeepSea/Physics/Flatbuffers/PhysicsFlatbufferHelpers.h>
#include <DeepSea/Physics/Shapes/PhysicsShape.h>
#include <DeepSea/Physics/RigidBodyTemplate.h>

#include <cstring>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/RigidBodyTemplate_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

dsRigidBodyTemplate* dsRigidBodyTemplate_loadImpl(dsPhysicsEngine* engine, dsAllocator* allocator,
	dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc,
	dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData, const void* data,
	size_t size, const char* name)
{
	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), size);
	if (!DeepSeaPhysics::VerifyRigidBodyTemplateBuffer(verifier))
	{
		errno = EFORMAT;
		if (name)
		{
			DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG,
				"Invalid rigid body template flatbuffer format for '%s'.", name);
		}
		else
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Invalid rigid body template flatbuffer format.");
		return nullptr;
	}

	auto fbTemplate = DeepSeaPhysics::GetRigidBodyTemplate(data);

	auto fbShapes = fbTemplate->shapes();
	if (fbShapes)
	{
		for (uint32_t i = 0; i < fbShapes->size(); ++i)
		{
			if (!(*fbShapes)[i])
			{
				errno = EFORMAT;
				if (name)
				{
					DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG,
						"Invalid rigid body template flatbuffer format for '%s'.", name);
				}
				else
				{
					DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
						"Invalid rigid body template flatbuffer format.");
				}
				return nullptr;
			}
		}
	}

	dsRigidBodyTemplate* rigidBodyTemplate = dsRigidBodyTemplate_create(engine, allocator,
		DeepSeaPhysics::convert(fbTemplate->flags()),
		DeepSeaPhysics::convert(fbTemplate->motionType()),
		DeepSeaPhysics::convert(fbTemplate->layer()), fbTemplate->friction(),
		fbTemplate->restitution(), fbTemplate->hardness(), fbShapes ? fbShapes->size() : 0);
	if (!rigidBodyTemplate)
		return nullptr;

	rigidBodyTemplate->dofMask = static_cast<dsPhysicsDOFMask>(fbTemplate->dofMask());
	rigidBodyTemplate->collisionGroup = fbTemplate->collisionGroup();
	rigidBodyTemplate->canCollisionGroupsCollideFunc = canCollisionGroupsCollideFunc;

	float linearDamping = fbTemplate->linearDamping();
	if (linearDamping >= 0.0f)
		rigidBodyTemplate->linearDamping = linearDamping;

	float angularDamping = fbTemplate->angularDamping();
	if (angularDamping >= 0.0f)
		rigidBodyTemplate->angularDamping = angularDamping;

	float maxLinearVelocity = fbTemplate->maxLinearVelocity();
	if (maxLinearVelocity >= 0.0f)
		rigidBodyTemplate->maxLinearVelocity = maxLinearVelocity;

	float maxAngularVelocity = fbTemplate->maxAngularVelocity();
	if (maxAngularVelocity >= 0.0f)
		rigidBodyTemplate->maxAngularVelocity = maxAngularVelocity;

	if (fbShapes)
	{
		for (uint32_t i = 0; i < fbShapes->size(); ++i)
		{
			auto fbShapeInstance = (*fbShapes)[i];

			dsPhysicsShape* shape = dsPhysicsShape_fromFlatbufferShape(engine, allocator,
				fbShapeInstance->shape(), findShapeFunc, findShapeUserData, name);
			if (!shape)
			{
				dsRigidBodyTemplate_destroy(rigidBodyTemplate);
				return nullptr;
			}

			auto fbTranslate = fbShapeInstance->translate();
			dsVector3f translate;
			if (fbTranslate)
				translate = DeepSeaPhysics::convert(*fbTranslate);

			auto fbRotate = fbShapeInstance->rotate();
			dsQuaternion4f rotate;
			if (fbRotate)
				rotate = DeepSeaPhysics::convert(*fbRotate);

			auto fbScale = fbShapeInstance->scale();
			dsVector3f scale;
			if (fbScale)
				scale = DeepSeaPhysics::convert(*fbScale);

			auto fbMaterial = fbShapeInstance->material();
			dsPhysicsShapePartMaterial material;
			if (fbMaterial)
				material = DeepSeaPhysics::convert(*fbMaterial);

			if (!dsRigidBodyTemplate_addShape(rigidBodyTemplate, shape,
					fbTranslate ? &translate : nullptr,
					fbRotate ? &rotate : nullptr, fbScale ? &scale : nullptr,
					fbShapeInstance->density(), fbMaterial ? &material : nullptr))
			{
				dsRigidBodyTemplate_destroy(rigidBodyTemplate);
				return nullptr;
			}
		}

		bool finalized;
		switch (fbTemplate->customMassProperties_type())
		{
			case DeepSeaPhysics::CustomMassProperties::ShiftedMass:
			{
				auto fbShiftedMass = fbTemplate->customMassProperties_as_ShiftedMass();
				float mass = fbShiftedMass->mass();

				auto fbRotationPointShift = fbShiftedMass->rotationPointShift();
				dsVector3f rotationPointShift;
				if (fbRotationPointShift)
					rotationPointShift = DeepSeaPhysics::convert(*fbRotationPointShift);

				finalized = dsRigidBodyTemplate_finalizeShapes(rigidBodyTemplate,
					mass < 0 ? &mass : nullptr,
					fbRotationPointShift ? &rotationPointShift : nullptr);
				break;
			}
			case DeepSeaPhysics::CustomMassProperties::MassProperties:
			{
				auto fbMassProperties = fbTemplate->customMassProperties_as_MassProperties();
				rigidBodyTemplate->massProperties = DeepSeaPhysics::convert(*fbMassProperties);
				finalized = true;
				break;
			}
			default:
				finalized = dsRigidBodyTemplate_finalizeShapes(rigidBodyTemplate, nullptr, nullptr);
				break;
		}

		if (!finalized)
		{
			dsRigidBodyTemplate_destroy(rigidBodyTemplate);
			return nullptr;
		}
	}

	return rigidBodyTemplate;
}

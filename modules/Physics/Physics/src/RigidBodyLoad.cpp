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

#include "RigidBodyLoad.h"

#include "Shapes/PhysicsShapeLoad.h"

#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Quaternion.h>

#include <DeepSea/Physics/Flatbuffers/PhysicsFlatbufferHelpers.h>
#include <DeepSea/Physics/Shapes/PhysicsShape.h>
#include <DeepSea/Physics/RigidBody.h>

#include <cstring>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/RigidBody_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

dsRigidBody* dsRigidBody_loadImpl(dsPhysicsEngine* engine, dsAllocator* allocator,
	void* userData, dsDestroyUserDataFunction destroyUserDataFunc,
	dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc,
	dsFindRigidBodyGroupFunction findRigidBodyGroupFunc, void* findRigidBodyGroupUserData,
	dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData, const void* data,
	size_t size, const char* name)
{
	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), size);
	if (!DeepSeaPhysics::VerifyRigidBodyBuffer(verifier))
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EFORMAT;
		if (name)
		{
			DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Invalid rigid body flatbuffer format for '%s'.",
				name);
		}
		else
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Invalid rigid body flatbuffer format.");
		return nullptr;
	}

	auto fbRigidBody = DeepSeaPhysics::GetRigidBody(data);

	auto fbShapes = fbRigidBody->shapes();
	if (fbShapes)
	{
		for (uint32_t i = 0; i < fbShapes->size(); ++i)
		{
			if (!(*fbShapes)[i])
			{
				if (destroyUserDataFunc)
					destroyUserDataFunc(userData);
				errno = EFORMAT;
				if (name)
				{
					DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG,
						"Invalid rigid body flatbuffer format for '%s'.", name);
				}
				else
					DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Invalid rigid body flatbuffer format.");
				return nullptr;
			}
		}
	}

	auto fbGroup = fbRigidBody->group();
	dsRigidBodyGroup* group = NULL;
	if (fbGroup)
	{
		group = findRigidBodyGroupFunc ?
			findRigidBodyGroupFunc(engine, findRigidBodyGroupUserData, fbGroup->c_str()) : nullptr;
		if (!group)
		{
			if (destroyUserDataFunc)
				destroyUserDataFunc(userData);
			errno = ENOTFOUND;
			if (name)
			{
				DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Rigid body group '%s' not found for '%s'.",
					fbGroup->c_str(), name);
			}
			else
			{
				DS_LOG_ERROR_F(DS_PHYSICS_LOG_TAG, "Rigid body group '%s' not found.",
					fbGroup->c_str());
			}
			return nullptr;
		}
	}

	dsRigidBodyInit init;
	init.userData = userData;
	init.destroyUserDataFunc = destroyUserDataFunc;
	init.group = group;
	init.flags = DeepSeaPhysics::convert(fbRigidBody->flags());
	if (group && group->motionType != dsPhysicsMotionType_Unknown)
		init.motionType = group->motionType;
	else
		init.motionType = DeepSeaPhysics::convert(fbRigidBody->motionType());
	init.dofMask = DeepSeaPhysics::convert(fbRigidBody->dofMask());
	init.layer = DeepSeaPhysics::convert(fbRigidBody->layer());
	init.collisionGroup = fbRigidBody->collisionGroup();
	init.canCollisionGroupsCollideFunc = canCollisionGroupsCollideFunc;

	auto fbPosition = fbRigidBody->position();
	if (fbPosition)
		init.position = DeepSeaPhysics::convert(*fbPosition);
	else
		std::memset(&init.position, 0, sizeof(dsVector3f));

	auto fbOrientation = fbRigidBody->orientation();
	if (fbOrientation)
		init.orientation = DeepSeaPhysics::convert(*fbOrientation);
	else
		dsQuaternion4_identityRotation(init.orientation);

	auto fbScale = fbRigidBody->scale();
	if (fbScale && (init.flags & dsRigidBodyFlags_Scalable))
		init.scale = DeepSeaPhysics::convert(*fbScale);
	else
		init.scale.x = init.scale.y = init.scale.z = 1.0f;

	auto fbLinearVelocity = fbRigidBody->linearVelocity();
	if (fbLinearVelocity)
		init.linearVelocity = DeepSeaPhysics::convert(*fbLinearVelocity);
	else
		std::memset(&init.linearVelocity, 0, sizeof(dsVector3f));

	auto fbAngularVelocity = fbRigidBody->angularVelocity();
	if (fbAngularVelocity)
		init.angularVelocity = DeepSeaPhysics::convert(*fbAngularVelocity);
	else
		std::memset(&init.angularVelocity, 0, sizeof(dsVector3f));

	init.friction = fbRigidBody->friction();
	init.restitution = fbRigidBody->restitution();
	init.hardness = fbRigidBody->hardness();

	init.linearDamping = fbRigidBody->linearDamping();
	if (init.linearDamping < 0)
		init.linearDamping = DS_DEFAULT_PHYSICS_DAMPING;

	init.angularDamping = fbRigidBody->angularDamping();
	if (init.angularDamping < 0)
		init.angularDamping = DS_DEFAULT_PHYSICS_DAMPING;

	init.maxLinearVelocity = fbRigidBody->maxLinearVelocity();
	if (init.maxLinearVelocity < 0)
		init.maxLinearVelocity = DS_DEFAULT_PHYSICS_MAX_LINEAR_VELOCITY;

	init.maxAngularVelocity = fbRigidBody->maxAngularVelocity();
	if (init.maxAngularVelocity < 0)
		init.maxAngularVelocity = DS_DEFAULT_PHYSICS_MAX_ANGULAR_VELOCITY;

	init.shapeCount = fbShapes ? fbShapes->size() : 0;

	dsRigidBody* rigidBody = dsRigidBody_create(engine, allocator, &init);
	if (!rigidBody)
		return nullptr;

	if (fbShapes)
	{
		for (uint32_t i = 0; i < fbShapes->size(); ++i)
		{
			auto fbShapeInstance = (*fbShapes)[i];

			dsPhysicsShape* shape = dsPhysicsShape_fromFlatbufferShape(engine, allocator,
				fbShapeInstance->shape(), findShapeFunc, findShapeUserData, name);
			if (!shape)
			{
				DS_VERIFY(dsRigidBody_destroy(rigidBody));
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

			if (!dsRigidBody_addShape(rigidBody, shape, fbTranslate ? &translate : nullptr,
					fbRotate ? &rotate : nullptr, fbScale ? &scale : nullptr,
					fbShapeInstance->density(), fbMaterial ? &material : nullptr))
			{
				DS_VERIFY(dsRigidBody_destroy(rigidBody));
				return nullptr;
			}
		}

		bool finalized;
		switch (fbRigidBody->customMassProperties_type())
		{
			case DeepSeaPhysics::CustomMassProperties::ShiftedMass:
			{
				auto fbShiftedMass = fbRigidBody->customMassProperties_as_ShiftedMass();
				float mass = fbShiftedMass->mass();

				auto fbRotationPointShift = fbShiftedMass->rotationPointShift();
				dsVector3f rotationPointShift;
				if (fbRotationPointShift)
					rotationPointShift = DeepSeaPhysics::convert(*fbRotationPointShift);

				finalized = dsRigidBody_finalizeShapes(rigidBody, mass < 0 ? &mass : nullptr,
					fbRotationPointShift ? &rotationPointShift : nullptr);
				break;
			}
			case DeepSeaPhysics::CustomMassProperties::MassProperties:
			{
				auto fbMassProperties = fbRigidBody->customMassProperties_as_MassProperties();
				dsPhysicsMassProperties massProperties = DeepSeaPhysics::convert(*fbMassProperties);
				finalized =
					dsRigidBody_finalizeShapesCustomMassProperties(rigidBody, &massProperties);
				break;
			}
			default:
				finalized = dsRigidBody_finalizeShapes(rigidBody, nullptr, nullptr);
				break;
		}

		if (!finalized)
		{
			DS_VERIFY(dsRigidBody_destroy(rigidBody));
			return nullptr;
		}
	}

	return rigidBody;
}

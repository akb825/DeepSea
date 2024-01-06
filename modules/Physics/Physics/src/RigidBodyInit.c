/*
 * Copyright 2023-2024 Aaron Barany
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

#include <DeepSea/Physics/RigidBodyInit.h>

#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Quaternion.h>

#include <string.h>

bool dsRigidBodyInit_initialize(dsRigidBodyInit* rigidBodyInit,
	dsRigidBodyFlags flags, dsPhysicsMotionType motionType, dsPhysicsLayer layer,
	const dsVector3f* position, dsQuaternion4f* orientation, const dsVector3f* scale, float mass,
	float friction, float restitution)
{
	if (!rigidBodyInit || mass < 0 || friction < 0 || restitution < 0 || restitution > 1)
	{
		errno = EINVAL;
		return false;
	}

	if ((flags & dsRigidBodyFlags_Sensor) && motionType == dsPhysicsMotionType_Dynamic)
	{
		DS_LOG_ERROR(DS_PHYSICS_LOG_TAG, "Rigid body sensors may not have dynamic motion type.");
		return false;
	}

	rigidBodyInit->userData = NULL;
	rigidBodyInit->destroyUserDataFunc = NULL;
	rigidBodyInit->flags = flags;
	rigidBodyInit->motionType = motionType;
	rigidBodyInit->dofMask = dsPhysicsDOFMask_All;
	rigidBodyInit->layer = layer;
	rigidBodyInit->collisionGroup = 0;
	rigidBodyInit->canCollisionGroupsCollideFunc = NULL;

	if (position)
		rigidBodyInit->position = *position;
	else
		memset(&rigidBodyInit->position, 0, sizeof(rigidBodyInit->position));

	if (orientation)
		rigidBodyInit->orientation = *orientation;
	else
		dsQuaternion4_identityRotation(rigidBodyInit->orientation);

	if (scale)
	{
		if (!(flags & dsRigidBodyFlags_Scalable) &&
			(scale->x != 1 || scale->y != 1 || scale->z != 1))
		{
			DS_LOG_ERROR(DS_PHYSICS_LOG_TAG,
				"Rigid body must have scalable flag set to have a scale.");
			return false;
		}
		rigidBodyInit->scale = *scale;
	}
	else
	{
		rigidBodyInit->scale.x = 1;
		rigidBodyInit->scale.y = 1;
		rigidBodyInit->scale.z = 1;
	}

	memset(&rigidBodyInit->linearVelocity, 0, sizeof(rigidBodyInit->linearVelocity));
	memset(&rigidBodyInit->angularVelocity, 0, sizeof(rigidBodyInit->angularVelocity));

	rigidBodyInit->mass = mass;
	rigidBodyInit->friction = friction;
	rigidBodyInit->restitution = restitution;
	rigidBodyInit->linearDamping = 0.05f;
	rigidBodyInit->angularDamping = 0.05f;
	rigidBodyInit->maxLinearVelocity = 500.0f;
	rigidBodyInit->maxAngularVelocity = (float)M_PI*15;
	rigidBodyInit->shapeCount = 0;
	return true;
}

bool dsRigidBodyInit_isValid(const dsRigidBodyInit* rigidBodyInit)
{
	if (!rigidBodyInit)
		return false;

	// Sensors can't be dynamic.
	if ((rigidBodyInit->flags & dsRigidBodyFlags_Sensor) &&
		rigidBodyInit->motionType == dsPhysicsMotionType_Dynamic)
	{
		return false;
	}

	// Must be identity scale if not scalable.
	if (!(rigidBodyInit->flags & dsRigidBodyFlags_Scalable) &&
		(rigidBodyInit->scale.x != 1 || rigidBodyInit->scale.y != 1 || rigidBodyInit->scale.z != 1))
	{
		return false;
	}

	// General ranges.
	return rigidBodyInit->mass >= 0 && rigidBodyInit->friction >= 0 &&
		rigidBodyInit->restitution >= 0 && rigidBodyInit->restitution <= 1 &&
		rigidBodyInit->linearDamping >= 0 && rigidBodyInit->angularDamping >= 0 &&
		rigidBodyInit->maxLinearVelocity >= 0 && rigidBodyInit->maxAngularVelocity >= 0;
}

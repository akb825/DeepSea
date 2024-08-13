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

#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Quaternion.h>

#include <string.h>

bool dsRigidBodyInit_initialize(dsRigidBodyInit* rigidBodyInit,
	dsRigidBodyFlags flags, dsPhysicsMotionType motionType, dsPhysicsLayer layer,
	const dsVector3f* position, dsQuaternion4f* orientation, const dsVector3f* scale,
	float friction, float restitution, float hardness)
{
	if (!rigidBodyInit || motionType < dsPhysicsMotionType_Static ||
		motionType > dsPhysicsMotionType_Dynamic || friction < 0 || restitution < 0 ||
		restitution > 1 || hardness < 0 || hardness > 1)
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
	rigidBodyInit->group = NULL;
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

	rigidBodyInit->friction = friction;
	rigidBodyInit->restitution = restitution;
	rigidBodyInit->hardness = hardness;
	rigidBodyInit->linearDamping = DS_DEFAULT_PHYSICS_DAMPING;
	rigidBodyInit->angularDamping = DS_DEFAULT_PHYSICS_DAMPING;
	rigidBodyInit->maxLinearVelocity = DS_DEFAULT_PHYSICS_MAX_LINEAR_VELOCITY;
	rigidBodyInit->maxAngularVelocity = DS_DEFAULT_PHYSICS_MAX_ANGULAR_VELOCITY;
	rigidBodyInit->shapeCount = 0;
	return true;
}

bool dsRigidBodyInit_initializeGroup(dsRigidBodyInit* rigidBodyInit,
	dsRigidBodyGroup* group, dsRigidBodyFlags flags, dsPhysicsMotionType motionType,
	dsPhysicsLayer layer, const dsVector3f* position, dsQuaternion4f* orientation,
	const dsVector3f* scale, float friction, float restitution, float hardness)
{
	if (!rigidBodyInit || !group || (group->motionType == dsPhysicsMotionType_Unknown &&
			(motionType < dsPhysicsMotionType_Static ||
			motionType > dsPhysicsMotionType_Dynamic)) ||
		(group->motionType != dsPhysicsMotionType_Unknown && motionType != group->motionType &&
			motionType != dsPhysicsMotionType_Unknown))
	{
		errno = EINVAL;
		return false;
	}

	if (motionType == dsPhysicsMotionType_Unknown)
		motionType = group->motionType;

	if (!dsRigidBodyInit_initialize(rigidBodyInit, flags, motionType, layer, position, orientation,
			scale, friction, restitution, hardness))
	{
		return false;
	}

	rigidBodyInit->group = group;
	return true;
}

bool dsRigidBodyInit_isValid(const dsRigidBodyInit* rigidBodyInit)
{
	if (!rigidBodyInit)
		return false;

	// Must be a valid motion type.
	if (rigidBodyInit->motionType < dsPhysicsMotionType_Static ||
		rigidBodyInit->motionType > dsPhysicsMotionType_Dynamic)
	{
		return false;
	}

	if (rigidBodyInit->group)
	{
		// Unless the group is unkown motion type, must share the same motion type with the group
		// and not allow changing motion type.
		if (rigidBodyInit->group->motionType != dsPhysicsMotionType_Unknown &&
			((rigidBodyInit->flags & dsRigidBodyFlags_MutableMotionType) ||
				rigidBodyInit->group->motionType != rigidBodyInit->motionType))
		{
			return false;
		}

		// Group may not be associated with a scene. Not a perfect check as it may be added to the
		// scene immediately after, but should be good enough to catch most mistakes.
		dsPhysicsScene* scene;
		DS_ATOMIC_LOAD_PTR(&rigidBodyInit->group->scene, &scene);
		if (scene)
			return false;
	}

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

	// Cannot have a velocity set if not dynamic.
	if (rigidBodyInit->motionType != dsPhysicsMotionType_Dynamic &&
		(rigidBodyInit->linearVelocity.x != 0 || rigidBodyInit->linearVelocity.y != 0 ||
		rigidBodyInit->linearVelocity.z != 0 || rigidBodyInit->angularVelocity.x != 0 ||
		rigidBodyInit->angularVelocity.y != 0 || rigidBodyInit->angularVelocity.z != 0))
	{
		return false;
	}

	// General ranges.
	return rigidBodyInit->friction >= 0 && rigidBodyInit->restitution >= 0 &&
		rigidBodyInit->restitution <= 1 && rigidBodyInit->hardness <= 0 &&
		rigidBodyInit->hardness >= 1 && rigidBodyInit->linearDamping >= 0 &&
		rigidBodyInit->angularDamping >= 0 && rigidBodyInit->maxLinearVelocity >= 0 &&
		rigidBodyInit->maxAngularVelocity >= 0;
}

/*
 * Copyright 2023 Aaron Barany
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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Physics/Export.h>
#include <DeepSea/Physics/RigidBodyTypes.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manipulate rigid bodies.
 * @see dsRigidBody
 */

/**
 * @brief Creates a rigid body.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the rigid body with.
 * @param allocator The allocator to create the rigid body with. This must support freeing memory.
 * @param initParams The initialization parameters to describe the rigid body.
 */
DS_PHYSICS_EXPORT dsRigidBody* dsRigidBody_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	const dsRigidBodyInit* initParams);

/**
 * @brief Adds flags to the rigid body.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to add flags to.
 * @param flags The flags to add to the rigid body, using a bitwise OR with the existing flags.
 * @return False if the flags couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_addFlags(dsRigidBody* rigidBody, dsRigidBodyFlags flags);

/**
 * @brief Removes flags from the rigid body.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to add flags to.
 * @param flags The flags to add to the rigid body, using an inverse bitwise AND with the existing
 *     flags.
 * @return False if the flags couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_removeFlags(dsRigidBody* rigidBody, dsRigidBodyFlags flags);

/**
 * @brief Sets the motion type for a rigid body.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to change the motion type on.
 * @param motionType The new motion type.
 * @return False if the motion type couldn't be changed.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setMotionType(dsRigidBody* rigidBody,
	dsPhysicsMotionType motionType);

/**
 * @brief Sets the degree of freedom mask for a rigid body.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to change the degrees of freedom on.
 * @param dofMask The new degree of freedom mask.
 * @return False if the degree of freedom mask couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setDOFMask(dsRigidBody* rigidBody, dsPhysicsDOFMask dofMask);

/**
 * @brief Sets the collision group for a rigid body.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to change the collision group on.
 * @param collisionGroup The new collision group.
 * @return False if the collision group couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setCollisionGroup(dsRigidBody* rigidBody,
	uint64_t collisionGroup);

/**
 * @brief Sets the can collision groups collide function for a rigid body.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to change the can collision groups collide function on.
 * @param canCollideFunc The new can collision groups collide function.
 * @return False if the can collision groups collide function couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setCanCollisionGroupsCollideFunction(dsRigidBody* rigidBody,
	dsCanCollisionGroupsCollideFunction canCollideFunc);

/**
 * @brief Sets the transform for a rigid body.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to change the transform on.
 * @param position The new position or NULL to leave unchanged.
 * @param orientation The new orientation or NULL to leave unchanged
 * @param scale The new scale or NULL to leave unchanged.
 * @return False if the transform couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setTransform(dsRigidBody* rigidBody, const dsVector3f* position,
	const dsQuaternion4f* orientation, const dsVector3f* scale);

/**
 * @brief Gets the transform matrix for a rigid body.
 * @remark errno will be set on failure.
 * @param[out] outTransform The output for the transform matrix.
 * @param rigidBody The rigid body to get the transform for.
 * @return False if the transform couldn't be computed.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_getTransformMatrix(dsMatrix44f* outTransform,
	const dsRigidBody* rigidBody);

/**
 * @brief Sets the transform for a rigid body based on a tranform matrix.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to change the transform on.
 * @param transform The transform matrix. This is expected to be orthogonal and not contain sheer.
 * @return False if the transform couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setTransformMatrix(dsRigidBody* rigidBody,
	const dsMatrix44f* transform);

/**
 * @brief Sets the center of mass of a rigid body.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to set the center of mass on.
 * @param centerOfMass The new center of mass reltive to the local coordinate space of the rigid
 *     body.
 * @return False if the center of mass couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setCenterOfMass(dsRigidBody* rigidBody,
	const dsVector3f* centerOfMass);

/**
 * @brief Sets the mass for a rigid body.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to change the mass on.
 * @param mass The new mass.
 * @return False if the mass couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setMass(dsRigidBody* rigidBody, float mass);

/**
 * @brief Sets the friction for a rigid body.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to change the friction on.
 * @param friction The new friction.
 * @return False if the friction couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setFriction(dsRigidBody* rigidBody, float friction);

/**
 * @brief Sets the restitution for a rigid body.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to change the restitution on.
 * @param restitution The new restitution.
 * @return False if the restitution couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setRestitution(dsRigidBody* rigidBody, float restitution);

/**
 * @brief Sets the linear damping for a rigid body.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to change the linear damping on.
 * @param linearDamping The new linear damping.
 * @return False if the linear damping couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setLinearDamping(dsRigidBody* rigidBody, float linearDamping);

/**
 * @brief Sets the angular damping for a rigid body.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to change the angular damping on.
 * @param angularDamping The new angular damping.
 * @return False if the angular damping couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setAngularDamping(dsRigidBody* rigidBody, float angularDamping);

/**
 * @brief Sets the max linear velocity for a rigid body.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to change the max linear velocity on.
 * @param maxLinearVelocity The new max linear velocity.
 * @return False if the max linear velocity couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setMaxLinearVelocity(dsRigidBody* rigidBody,
	float maxLinearVelocity);

/**
 * @brief Sets the max angular velocity for a rigid body.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to change the max angular velocity on.
 * @param maxAngularVelocity The new max angular velocity.
 * @return False if the max angular velocity couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setMaxAngularVelocity(dsRigidBody* rigidBody,
	float maxAngularVelocity);

/**
 * @brief Destroys a rigid body.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to destroy.
 * @return False if the rigid body couldn't be destroyed.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_destroy(dsRigidBody* rigidBody);

#ifdef __cplusplus
}
#endif

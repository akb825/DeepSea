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
 * @remark If the rigid body is part of a group, the group must not be added to a scene before
 *    creation.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the rigid body with.
 * @param allocator The allocator to create the rigid body with. If NULL the engine's allocator will
 *     be used. This must support freeing memory.
 * @param initParams The initialization parameters to describe the rigid body.
 * @return The rigid body or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsRigidBody* dsRigidBody_create(dsPhysicsEngine* engine, dsAllocator* allocator,
	const dsRigidBodyInit* initParams);

/**
 * @brief Adds a shape to a rigid body.
 *
 * This may not be called on a rigid body where shapesFinalized is true unless the
 * dsRigidBodyFlags_MutableShape flag is set, in which case shapesFinalized is set to false.
 *
 * @remark Transform factors that are NULL are an indication that they will never be set, and cannot
 *     be changed later with dsRigidBody_setShapeTransform*().
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to add the shape to.
 * @param shape The shape to add.
 * @param translate The translation of the shape or NULL to leave at the origin.
 * @param rotate The rotation of the shape or NULL to leave unrotated.
 * @param scale The scale of the shape or NULL to leave unscaled. No dimension of scale may be 0.
 * @param density The density of the shape. This will be ignored if the motion type is not dynamic
 *     and dsRigidBodyFlags_MutableShape is not set, otherwise it must be > 0.
 * @param material The material for the shape or NULL to use the rigid body's material. This will be
 *     ignored if the shape has per-face materials.
 * @return The ID of the shape or DS_INVALID_PHYSICS_ID if the shape couldn't be added.
 */
DS_PHYSICS_EXPORT uint32_t dsRigidBody_addShape(dsRigidBody* rigidBody, dsPhysicsShape* shape,
	const dsVector3f* translate, const dsQuaternion4f* rotate, const dsVector3f* scale,
	float density, const dsPhysicsShapePartMaterial* material);

/**
 * @brief Sets the transform for a shape on a rigid body.
 *
 * This may not be called on a rigid body where shapesFinalized is true unless the
 * dsRigidBodyFlags_MutableShape flag is set, in which case shapesFinalized is set to false.
 *
 * @remark Any transform factors that were originally NULL for dsRigidBody_addShape() must also be
 *     NULL when setting the transform.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to set the shape transform on.
 * @param shapeID The ID of the shape to set the transform on.
 * @param translate The translation of the shape or NULL to leave unchanged.
 * @param rotate The rotation of the shape or NULL to leave unchanged.
 * @param scale The scale of the shape or NULL to leave unchanged. No dimension of scale may be 0.
 * @return False if the transform couldn't be set on the shape.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setShapeTransformID(dsRigidBody* rigidBody, uint32_t shapeID,
	const dsVector3f* translate, const dsQuaternion4f* rotate, const dsVector3f* scale);

/**
 * @brief Sets the transform for a shape on a rigid body.
 *
 * This may not be called on a rigid body where shapesFinalized is true unless the
 * dsRigidBodyFlags_MutableShape flag is set, in which case shapesFinalized is set to false.
 *
 * @remark Any transform factors that were originally NULL for dsRigidBody_addShape() must also be
 *     NULL when setting the transform.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to set the shape transform on.
 * @param shapeIndex The index of the shape to set the transform on.
 * @param translate The translation of the shape or NULL to leave unchanged.
 * @param rotate The rotation of the shape or NULL to leave unchanged.
 * @param scale The scale of the shape or NULL to leave unchanged. No dimension of scale may be 0.
 * @return False if the transform couldn't be set on the shape.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setShapeTransformIndex(dsRigidBody* rigidBody,
	uint32_t shapeIndex, const dsVector3f* translate, const dsQuaternion4f* rotate,
	const dsVector3f* scale);

/**
 * @brief Sets the material for a shape on a rigid body.
 * @remark This may be called even if the dsRigidBodyFlags_MutableShape flag isn't set.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to set the shape material on.
 * @param shapeID The ID of the shape to set the material on.
 * @param material The material for the shape or NULL to use the rigid body's material. This will be
 *     ignored if the shape has per-face materials.
 * @return False if the transform couldn't be set on the shape.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setShapeMaterialID(dsRigidBody* rigidBody, uint32_t shapeID,
	const dsPhysicsShapePartMaterial* material);

/**
 * @brief Sets the material for a shape on a rigid body.
 * @remark This may be called even if the dsRigidBodyFlags_MutableShape flag isn't set.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to set the shape material on.
 * @param shapeIndex The index of the shape to set the transform on.
 * @param material The material for the shape or NULL to use the rigid body's material. This will be
 *     ignored if the shape has per-face materials.
 * @return False if the transform couldn't be set on the shape.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setShapeMaterialIndex(dsRigidBody* rigidBody,
	uint32_t shapeIndex, const dsPhysicsShapePartMaterial* material);

/**
 * @brief Removes a shape from a rigid body.
 *
 * This may not be called on a rigid body where shapesFinalized is true unless the
 * dsRigidBodyFlags_MutableShape flag is set, in which case shapesFinalized is set to false.
 *
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to set the shape transform on.
 * @param shapeID The ID of the shape to remove.
 * @return False if the shape couldn't be removed.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_removeShapeID(dsRigidBody* rigidBody, uint32_t shapeID);

/**
 * @brief Removes a shape from a rigid body.
 *
 * This may not be called on a rigid body where shapesFinalized is true unless the
 * dsRigidBodyFlags_MutableShape flag is set, in which case shapesFinalized is set to false.
 *
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to set the shape transform on.
 * @param shapeIndex The index of the shape to remove.
 * @return False if the shape couldn't be removed.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_removeShapeIndex(dsRigidBody* rigidBody, uint32_t shapeIndex);

/**
 * @brief Computes the default mass properties for the rigid body.
 *
 * This will be the mass properties used for dsRigidBody_finalizeShapes() when passing NULL for mass
 * and rotationPointShift. This may be used as a starting point when providing mass properties for
 * dsRigidBody_finalizeShapesCustomMassProperties(), such as transforming the mass properties (as
 * opposed to shifting it) to simulate a non-uniform density.
 *
 * This may not be called on a rigid body if the motion type isn't
 * dsPhysicsMotionType_Dynamic and the dsRigidBodyFlags_MutableMotionType flag isn't set.
 *
 * @remark errno will be set on failure.
 * @param[out] outMassProperties The mass properties to populate.
 * @param rigidBody The rigid body to compute the mass properties for.
 * @return False if the mass properties couldn't be computed.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_computeDefaultMassProperties(
	dsPhysicsMassProperties* outMassProperties, const dsRigidBody* rigidBody);

/**
 * @brief Finalizes the shapes on a rigid body, allowing it to be used for physics simulations.
 *
 * This will compute the mass properties based on the component shapes. By default the mass will be
 * computed by the density of each shape, though an explicit mass may be passed in instead if
 * desired. When an explicit mass is provided, the shape densities will still be used to determine
 * how much to contribute to the moment of inertia.
 *
 * By default the point of rotation will be the center of mass, but rotationPointShift may be used
 * to shift the point of rotation. For example, shifting the point of rotation up will put the
 * center of mass below the point of rotation, which can make an object more stable. (common when
 * creating vehicles)
 *
 * This will toggle the shapesFinalized flag to true. It may not be called on a rigid body where
 * shapesFinalized is already true unless dsRigidBodyFlags_MutableShape flag is set, in which case
 * this may be used to re-compute the mass properties.
 *
 * Computing the mass properties will be skipped if the motion type isn't
 * dsPhysicsMotionType_Dynamic and the dsRigidBodyFlags_MutableMotionType flag isn't set.
 *
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to finalize the shapes on.
 * @param mass The mass for the rigid body or NULL to use the mass based on the shape densities.
 *     This must be > 0 if set.
 * @param rotationPointShift The amount to shift the point of rotation relative to the center of
 *     mass or NULL to leave the point of rotation at the center of mass.
 * @return False if the shapes couldn't be finalized.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_finalizeShapes(dsRigidBody* rigidBody, const float* mass,
	const dsVector3f* rotationPointShift);

/**
 * @brief Finalizes the shapes on a rigid body, allowing it to be used for physics simulations.
 *
 * This may be used when a special moment of inertia is desired, such as simulating shapes with
 * non-uniform density or hollow shapes.
 *
 * This will toggle the shapesFinalized flag to true. It may not be called on a rigid body where
 * shapesFinalized is already true unless dsRigidBodyFlags_MutableShape flag is set, in which case
 * this may be used to replace the mass properties.
 *
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to finalize the shapes on.
 * @param massProperties The mass properties to use.
 * @return False if the shapes couldn't be finalized.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_finalizeShapesCustomMassProperties(dsRigidBody* rigidBody,
	const dsPhysicsMassProperties* massProperties);

/**
 * @brief Gets the material for a shape in a rigid body by ID.
 * @remark errno will be set on failure.
 * @param[out] outMaterial The shape material.
 * @param rigidBody The rigid body.
 * @param shapeID The ID for the shape.
 * @param faceIndex The index for the face for shapes that support per-face materials.
 * @return False if the material couldn't be queried.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_getShapeMaterialID(dsPhysicsShapePartMaterial* outMaterial,
	const dsRigidBody* rigidBody, uint32_t shapeID, uint32_t faceIndex);

/**
 * @brief Gets the material for a shape in a rigid body by index.
 * @remark errno will be set on failure.
 * @param[out] outMaterial The shape material.
 * @param rigidBody The rigid body.
 * @param shapeIndex The index for the shape.
 * @param faceIndex The index for the face for shapes that support per-face materials.
 * @return False if the material couldn't be queried.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_getShapeMaterialIndex(dsPhysicsShapePartMaterial* outMaterial,
	const dsRigidBody* rigidBody, uint32_t shapeIndex, uint32_t faceIndex);

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
 * @param scale The new scale or NULL to leave unchanged. No dimension of scale may be 0.
 * @param activate Whether to activate the rigid body if it's currently inactive.
 * @return False if the transform couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setTransform(dsRigidBody* rigidBody, const dsVector3f* position,
	const dsQuaternion4f* orientation, const dsVector3f* scale, bool activate);

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
 * @param activate Whether to activate the rigid body if it's currently inactive.
 * @return False if the transform couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setTransformMatrix(dsRigidBody* rigidBody,
	const dsMatrix44f* transform, bool activate);

/**
 * @brief Sets the target transform for a kinematic rigid body.
 *
 * This will linearly move the kinematic body for the next step of the physics simulation. When
 * using linear collision, this allows for the full linear step to be considered. Most commonly
 * this will be called as part of a dsOnPhysicsSceneStepFunction callback.
 *
 * This is only valid for a kinematic rigid body currently a part of a scene.
 *
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to set the kinematic target transform on.
 * @param time The time for the step, most commonly forwarded from the time parameter of a
 *     dsOnPhysicsSceneStepFunction callback.
 * @param position The new position or NULL to leave unchanged.
 * @param orientation The new orientation or NULL to leave unchanged.
 * @return False if the kinematic target couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setKinematicTarget(dsRigidBody* rigidBody,
	float time, const dsVector3f* position, const dsQuaternion4f* orientation);

/**
 * @brief Sets the target transform for a kinematic rigid body based on a transform matrix.
 *
 * This will linearly move the kinematic body for the next step of the physics simulation. When
 * using linear collision, this allows for the full linear step to be considered. Most commonly
 * this will be called as part of a dsOnPhysicsSceneStepFunction callback.
 *
 * This is only valid for a kinematic rigid body currently a part of a scene.
 *
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to set the kinematic target transform on.
 * @param time The time for the step, most commonly forwarded from the time parameter of a
 *     dsOnPhysicsSceneStepFunction callback.
 * @param transform The transform matrix. This is expected to be orthogonal and not contain sheer.
 *     If the transform has scale that's different from the current scale factor, it will be set
 *     as weith dsRigidBody_setTransform(), meaning that it will be immediately applied rather than
 *     over the time step.
 * @return False if the kinematic target couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setKinematicTargetMatrix(dsRigidBody* rigidBody,
	float time, const dsMatrix44f* transform);

/**
 * @brief Gets the position around which the rigid body will rotate in world space.
 * @remark The shapes must be finalized before calling this function.
 * @remark errno will be set on failure.
 * @param[out] outPosition The center of rotation in world space.
 * @param rigidBody The rigid body to change the mass on.
 * @return False if the rotation position qouldn't be queried.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_getWorldRotationPosition(dsVector3f* outPosition,
	const dsRigidBody* rigidBody);

/**
 * @brief Sets the mass for a rigid body.
 * @remark This will not conserve momentum.
 * @remark The shapes must be finalized before calling this function.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to change the mass on.
 * @param mass The new mass. This must be > 0.
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
 * @brief Sets the hardness for a rigid body.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to change the hardness on.
 * @param hardness The new hardness.
 * @return False if the restitution couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setHardness(dsRigidBody* rigidBody, float hardness);

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
 * @brief Gets the current linear velocity for a rigid body.
 *
 * This is only valid if the motion type is dynamic.
 *
 * @remark errno will be set on failure.
 * @param[out] outVelocity The storage for the linear velocity.
 * @param rigidBody The rigid body to get the linear velocity for.
 * @return False if the linear velocity couldn't be queried.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_getLinearVelocity(dsVector3f* outVelocity,
	const dsRigidBody* rigidBody);

/**
 * @brief Sets the current linear velocity for a rigid body.
 *
 * The volicty may only be set if the motion type is dynamic.
 *
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to set the linear velocity on.
 * @param velocity The new linear velocity.
 * @return False if the linear velocity couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setLinearVelocity(dsRigidBody* rigidBody,
	const dsVector3f* velocity);

/**
 * @brief Gets the current angular velocity for a rigid body.
 *
 * This is only valid if the motion type is dynamic.
 *
 * @remark errno will be set on failure.
 * @param[out] outVelocity The storage for the angular velocity.
 * @param rigidBody The rigid body to get the angular velocity for.
 * @return False if the angular velocity couldn't be queried.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_getAngularVelocity(dsVector3f* outVelocity,
	const dsRigidBody* rigidBody);

/**
 * @brief Sets the current angular velocity for a rigid body.
 *
 * The volicty may only be set if the motion type is dynamic.
 *
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to set the angular velocity on.
 * @param velocity The new angular velocity.
 * @return False if the angular velocity couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setAngularVelocity(dsRigidBody* rigidBody,
	const dsVector3f* velocity);

/**
 * @brief Adds a force to a rigid body.
 *
 * Forces may only be applied if the motion type is dynamic.
 *
 * The force will only be active until the next time the physics scene is stepped.
 *
 * @remark The shapes must be finalized before calling this function.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to add the force to.
 * @param force The force, or mass times acceleration.
 * @return False if the force couldn't be added.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_addForce(dsRigidBody* rigidBody, const dsVector3f* force);

/**
 * @brief Adds a force to a rigid body at a point.
 *
 * Forces may only be applied if the motion type is dynamic.
 *
 * This may add torque as well if the force isn't aligned with the center of rotation.
 * The force will only be active until the next time the physics scene is stepped.
 *
 * @remark The shapes must be finalized before calling this function.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to add the force to.
 * @param force The force, or mass times acceleration.
 * @param point The point the force is applied to.
 * @return False if the force couldn't be added.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_addForceAtPoint(dsRigidBody* rigidBody, const dsVector3f* force,
	const dsVector3f* point);

/**
 * @brief Clears the previously accumulated force for a rigid body.
 *
 * Forces may only be applied if the motion type is dynamic.
 *
 * This will clear any forces previously added with dsRigidBody_addForce() since the previous step
 * of the physics scene.
 *
 * @remark The shapes must be finalized before calling this function.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to clear the force on.
 * @return False if the force couldn't be cleared.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_clearForce(dsRigidBody* rigidBody);

/**
 * @brief Adds a torque to a rigid body.
 *
 * Forces may only be applied if the motion type is dynamic.
 *
 * The torque will only be active until the next time the physics scene is stepped.
 *
 * @remark The shapes must be finalized before calling this function.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to add the force to.
 * @param torque The torque, or mass times angular acceleration.
 * @return False if the force couldn't be added.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_addTorque(dsRigidBody* rigidBody, const dsVector3f* torque);

/**
 * @brief Clears the previously accumulated torque for a rigid body.
 *
 * Forces may only be applied if the motion type is dynamic.
 *
 * This will clear any torque previously added with dsRigidBody_addTorque() since the previous step
 * of the physics scene.
 *
 * @remark The shapes must be finalized before calling this function.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to clear the torque on.
 * @return False if the torque couldn't be cleared.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_clearTorque(dsRigidBody* rigidBody);

/**
 * @brief Adds a linear impulse to a rigid body.
 *
 * Forces may only be applied if the motion type is dynamic.
 *
 * The torque will only be active until the next time the physics scene is stepped.
 *
 * @remark The shapes must be finalized before calling this function.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to add the force to.
 * @param impulse The linear impulse, or mass times linear velocity.
 * @return False if the force couldn't be added.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_addLinearImpulse(dsRigidBody* rigidBody,
	const dsVector3f* impulse);

/**
 * @brief Clears the previously accumulated linear impulse for a rigid body.
 *
 * Forces may only be applied if the motion type is dynamic.
 *
 * This will clear any linear impulse previously added with dsRigidBody_addLinearImpulse() since the
 * previous step of the physics scene.
 *
 * @remark The shapes must be finalized before calling this function.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to clear the linear impulse on.
 * @return False if the linear impulse couldn't be cleared.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_clearLinearImpulse(dsRigidBody* rigidBody);

/**
 * @brief Adds a angular impulse to a rigid body.
 *
 * Forces may only be applied if the motion type is dynamic.
 *
 * The torque will only be active until the next time the physics scene is stepped.
 *
 * @remark The shapes must be finalized before calling this function.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to add the force to.
 * @param impulse The angular impulse, or mass times angular velocity.
 * @return False if the force couldn't be added.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_addAngularImpulse(dsRigidBody* rigidBody,
	const dsVector3f* impulse);

/**
 * @brief Clears the previously accumulated angular impulse for a rigid body.
 *
 * Forces may only be applied if the motion type is dynamic.
 *
 * This will clear any angular impulse previously added with dsRigidBody_addAngularImpulse() since
 * the previous step of the physics scene.
 *
 * @remark The shapes must be finalized before calling this function.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to clear the angular impulse on.
 * @return False if the angular impulse couldn't be cleared.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_clearAngularImpulse(dsRigidBody* rigidBody);

/**
 * @brief Sets whether a rigid body is active.
 *
 * An inactive rigid body is considered at rest and not moving.
 *
 * @remark The shapes must be finalized before calling this function.
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to set the active state on.
 * @param active Whether the rigid body is active.
 * @return False if the active state couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_setActive(dsRigidBody* rigidBody, bool active);

/**
 * @brief Destroys a rigid body.
 *
 * If the rigid body is part of a group or scene, it will be implicitly removed from both.
 *
 * @remark errno will be set on failure.
 * @param rigidBody The rigid body to destroy.
 * @return False if the rigid body couldn't be destroyed.
 */
DS_PHYSICS_EXPORT bool dsRigidBody_destroy(dsRigidBody* rigidBody);

/**
 * @brief Initializes a rigid body.
 *
 * This is called by the physics implementation to initialize the common members.
 *
 * @param[out] rigidBody The rigid body to initialize.
 * @param engine The physics engine the rigid body was created with.
 * @param allocator The allocator the rigid body was created with.
 * @param initParams The initialization parameters for the rigid body.
 */
DS_PHYSICS_EXPORT void dsRigidBody_initialize(dsRigidBody* rigidBody, dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsRigidBodyInit* initParams);

#ifdef __cplusplus
}
#endif

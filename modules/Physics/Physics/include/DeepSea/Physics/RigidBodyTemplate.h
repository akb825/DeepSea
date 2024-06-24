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
 * @brief Functions to create and manipulate rigid body templates.
 * @see dsRigidBodyTemplate
 */

/**
 * @brief Creates a rigid body template.
 *
 * This takes the most commonly changed attributes and sets the default values for the remaining
 * members.
 *
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the rigid body template with.
 * @param allocator The allocator to create the rigid body with template. If NULL the engine's allocator
 *     will be used. This must support freeing memory.
 * @param flags The flags for the rigid body.
 * @param motionType The initial motion type for the rigid body.
 * @param layer The layer the rigid body will be associated with.
 * @param friction The coefficient of friction, with 0 meaning no friction and increasing values
 *     having higher friction.
 * @param restitution The restitution of the rigid body, where 0 is fully inelastic and 1 is fully
 *     elastic.
 * @param hardness The hardness value, where 0 indicates to use this body's restitution on collision
 *     and 1 indicates to use the other body's restitution.
 * @param shapeCount The expected number of shapes. This will reserve space ahead of time, but may be
 *     left at 0 to dynamically grow size as needed.
 * @return The rigid body template or NULL if it couldn't be created.
 */
DS_PHYSICS_EXPORT dsRigidBodyTemplate* dsRigidBodyTemplate_create(dsPhysicsEngine* engine,
	dsAllocator* allocator, dsRigidBodyFlags flags, dsPhysicsMotionType motionType,
	dsPhysicsLayer layer, float friction, float restitution, float hardness, uint32_t shapeCount);

/**
 * @brief Loads a rigid body template from a file.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the rigid body template with.
 * @param allocator The allocator to create the rigid body template with. If NULL the engine's
 *     allocator will be used.
 * @param canCollisionGroupsCollideFunc Function to check whether two collision groups can collide.
 * @param findShapeFunc Function to find a shape by name. This will be used if a shape reference is
 *     used. All lookups will fail if this function is NULL.
 * @param findShapeUserData User data to pass to findShapeFunc.
 * @param filePath The file path for the physics rigid body to load.
 * @return The loaded rigid body template or NULL if it couldn't be loaded.
 */
DS_PHYSICS_EXPORT dsRigidBodyTemplate* dsRigidBodyTemplate_loadFile(dsPhysicsEngine* engine,
	dsAllocator* allocator, dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc,
	dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData, const char* filePath);

/**
 * @brief Loads a rigid body template from a resource file.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the rigid body template with.
 * @param allocator The allocator to create the rigid body template with. If NULL the engine's
 *     allocator will be used.
 * @param canCollisionGroupsCollideFunc Function to check whether two collision groups can collide.
 * @param findShapeFunc Function to find a shape by name. This will be used if a shape reference is
 *     used. All lookups will fail if this function is NULL.
 * @param findShapeUserData User data to pass to findShapeFunc.
 * @param type The type of resource to load.
 * @param filePath The file path for the physics shape to load.
 * @return The loaded physics rigid body or NULL if it couldn't be loaded.
 */
DS_PHYSICS_EXPORT dsRigidBodyTemplate* dsRigidBodyTemplate_loadResource(dsPhysicsEngine* engine,
	dsAllocator* allocator, dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc,
	dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData,
	dsFileResourceType type, const char* filePath);

/**
 * @brief Loads a rigid body template from a stream.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the rigid body template with.
 * @param allocator The allocator to create the rigid body template with. If NULL the engine's
 *     allocator will be used.
 * @param canCollisionGroupsCollideFunc Function to check whether two collision groups can collide.
 * @param findShapeFunc Function to find a shape by name. This will be used if a shape reference is
 *     used. All lookups will fail if this function is NULL.
 * @param findShapeUserData User data to pass to findShapeFunc.
 * @param stream The stream to load from.
 * @return The loaded rigid body template or NULL if it couldn't be loaded.
 */
DS_PHYSICS_EXPORT dsRigidBodyTemplate* dsRigidBodyTemplate_loadStream(dsPhysicsEngine* engine,
	dsAllocator* allocator, dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc,
	dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData, dsStream* stream);

/**
 * @brief Loads a rigid body template from a data buffer.
 * @remark errno will be set on failure.
 * @param engine The physics engine to create the rigid body template with.
 * @param allocator The allocator to create the rigid body template with. If NULL the engine's
 *     allocator will be used.
 * @param canCollisionGroupsCollideFunc Function to check whether two collision groups can collide.
 * @param findShapeFunc Function to find a shape by name. This will be used if a shape reference is
 *     used. All lookups will fail if this function is NULL.
 * @param findShapeUserData User data to pass to findShapeFunc.
 * @param data The data buffer to load from.
 * @param size The size of the data buffer.
 * @return The loaded physics rigid body or NULL if it couldn't be loaded.
 */
DS_PHYSICS_EXPORT dsRigidBodyTemplate* dsRigidBodyTemplate_loadData(dsPhysicsEngine* engine,
	dsAllocator* allocator, dsCanCollisionGroupsCollideFunction canCollisionGroupsCollideFunc,
	dsFindPhysicsShapeFunction findShapeFunc, void* findShapeUserData, const void* data,
	size_t size);

/**
 * @brief Adds a shape to a rigid body template.
 * @remark Transform factors that are NULL are an indication that they will never be set, and cannot
 *     be changed later with dsRigidBody_setShapeTransform*().
 * @remark errno will be set on failure.
 * @param rigidBodyTemplate The rigid body template to add the shape to.
 * @param shape The shape to add.
 * @param translate The translation of the shape or NULL to leave at the origin.
 * @param rotate The rotation of the shape or NULL to leave unrotated.
 * @param scale The scale of the shape or NULL to leave unscaled. No dimension of scale may be 0.
 * @param density The density of the shape. This will be ignored if the motion type is not dynamic
 *     and dsRigidBodyFlags_MutableShape is not set, otherwise it must be > 0.
 * @param material The material for the shape or NULL to use the rigid body's material. This will be
 *     ignored if the shape has per-face materials.
 * @return False if the shape couldn't be added.
 */
DS_PHYSICS_EXPORT bool dsRigidBodyTemplate_addShape(dsRigidBodyTemplate* rigidBodyTemplate,
	dsPhysicsShape* shape, const dsVector3f* translate, const dsQuaternion4f* rotate,
	const dsVector3f* scale, float density, const dsPhysicsShapePartMaterial* material);

/**
 * @brief Finalizes the shapes on a rigid body.
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
 * Computing the mass properties will be skipped if the motion type isn't
 * dsPhysicsMotionType_Dynamic and the dsRigidBodyFlags_MutableMotionType flag isn't set.
 *
 * @remark errno will be set on failure.
 * @param rigidBodyTemplate The rigid body template to finalize the shapes on.
 * @param mass The mass for the rigid body or NULL to use the mass based on the shape densities.
 *     This must be > 0 if set.
 * @param rotationPointShift The amount to shift the point of rotation relative to the center of
 *     mass or NULL to leave the point of rotation at the center of mass.
 * @return False if the shapes couldn't be finalized.
 */
DS_PHYSICS_EXPORT bool dsRigidBodyTemplate_finalizeShapes(dsRigidBodyTemplate* rigidBodyTemplate,
	const float* mass, const dsVector3f* rotationPointShift);

/**
 * @brief Instantiates a rigid body from the template.
 * @remark errno will be set on failure.
 * @param rigidBodyTemplate The rigid body to create the template from.
 * @param allocator The allocator to create the rigid body with or NULL to use the template's
 *     allocator.
 * @param userData The user data for the rigid body.
 * @param destroyUserDataFunc Function to destroy the user data. This will be called even if the
 *     creation of the rigid body fails.
 * @param group The rigid body group to create the rigid body with or NULL to not associate with a
 *     group.
 * @param position The position of the rigid body or NULL if at the origin.
 * @param orientation The orientation of the rigid body or NULL if not rotated.
 * @param linearVelocity The initial linear velocity or NULL to start at rest.
 * @param angularVelocity The initial angular velocity or NULL to start at rest.
 * @param scale The scale of the rigid body or NULL if not scaled.
 * @return The rigid body or NULL if it couldn't be created. The shapes will be finalized unless
 *     there are no shapes in the template.
 */
DS_PHYSICS_EXPORT dsRigidBody* dsRigidBodyTemplate_instantiate(
	const dsRigidBodyTemplate* rigidBodyTemplate, dsAllocator* allocator,
	void* userData, dsDestroyUserDataFunction destroyUserDataFunc, dsRigidBodyGroup* group,
	const dsVector3f* position, dsQuaternion4f* orientation, const dsVector3f* scale,
	const dsVector3f* linearVelocity, const dsVector3f* angularVelocity);

/**
 * @brief Destroys a rigid body template.
 * @param rigidBodyTemplate The rigid body template to destroy.
 */
DS_PHYSICS_EXPORT void dsRigidBodyTemplate_destroy(dsRigidBodyTemplate* rigidBodyTemplate);

#ifdef __cplusplus
}
#endif

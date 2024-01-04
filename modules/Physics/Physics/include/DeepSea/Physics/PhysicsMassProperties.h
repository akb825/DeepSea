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
#include <DeepSea/Physics/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions to create and manipulate physics mass properties.
 * @see dsPhysicsMassProperties
 */

/**
 * @brief Initializes mass properties to an empty value.
 *
 * This isn't by itself usable, but may be used as an invalid placeholder or an initial value for
 * dsPhysicsMassProperties_combine().
 *
 * @remark errno will be set on failure.
 * @param[out] massProperties The mass properties to initialize.
 * @return False if the parameters are invalid.
 */
DS_PHYSICS_EXPORT bool dsPhysicsMassProperties_initializeEmpty(
	dsPhysicsMassProperties* massProperties);

/**
 * @brief Initializes mass properties for a box.
 * @remark errno will be set on failure.
 * @param[out] massProperties The mass properties to initialize.
 * @param halfExtents The half extents for each axis. The extents must be > 0.
 * @param density The density of the box. This must be > 0.
 * @return False if the parameters are invalid.
 */
DS_PHYSICS_EXPORT bool dsPhysicsMassProperties_initializeBox(
	dsPhysicsMassProperties* massProperties, const dsVector3f* halfExtents, float density);

/**
 * @brief Initializes mass properties for a sphere.
 * @remark errno will be set on failure.
 * @param[out] massProperties The mass properties to initialize.
 * @param radius The radius of the sphere. This must be > 0.
 * @param density The density of the sphere. This must be > 0.
 * @return False if the parameters are invalid.
 */
DS_PHYSICS_EXPORT bool dsPhysicsMassProperties_initializeSphere(
	dsPhysicsMassProperties* massProperties, float radius, float density);

/**
 * @brief Initializes mass properties for a cylinder.
 * @remark errno will be set on failure.
 * @param[out] massProperties The mass properties to initialize.
 * @param halfHeight The half height of the cylinder. This must be > 0.
 * @param radius The radius of the cylinder. This must be > 0.
 * @param axis The axis to align the cylinder with.
 * @param density The density of the cylinder. This must be > 0.
 * @return False if the parameters are invalid.
 */
DS_PHYSICS_EXPORT bool dsPhysicsMassProperties_initializeCylinder(
	dsPhysicsMassProperties* massProperties, float halfHeight, float radius, dsPhysicsAxis axis,
	float density);

/**
 * @brief Initializes mass properties for a capsule.
 * @remark errno will be set on failure.
 * @param[out] massProperties The mass properties to initialize.
 * @param halfHeight The half height of the capsule. This must be > 0.
 * @param radius The radius of the capsule. This must be > 0.
 * @param axis The axis to align the capsule with.
 * @param density The density of the capsule. This must be > 0.
 * @return False if the parameters are invalid.
 */
DS_PHYSICS_EXPORT bool dsPhysicsMassProperties_initializeCapsule(
	dsPhysicsMassProperties* massProperties, float halfHeight, float radius, dsPhysicsAxis axis,
	float density);

/**
 * @brief Initializes mass properties for a capsule.
 * @remark errno will be set on failure.
 * @param[out] massProperties The mass properties to initialize.
 * @param height The height of the cone. This must be > 0.
 * @param radius The radius of the cone. This must be > 0.
 * @param axis The axis to align the cone with.
 * @param density The density of the cone. This must be > 0.
 * @return False if the parameters are invalid.
 */
DS_PHYSICS_EXPORT bool dsPhysicsMassProperties_initializeCone(
	dsPhysicsMassProperties* massProperties, float height, float radius, dsPhysicsAxis axis,
	float density);

/**
 * @brief Initializes mass properties for a triangle mesh.
 * @remark errno will be set on failure.
 * @param[out] massProperties The mass properties to initialize.
 * @param vertices Pointer to the first vertex. Each vertex is defined as 3 floats.
 * @param vertexCount The number of vertices. At least 3 vertices must be provided.
 * @param vertexStride The stride in bytes between each vertex. This must be at least
 *     3*sizeof(float).
 * @param indices The indices for the mesh. At least 3 indices must be provided.
 * @param indexCount The number of indices. This must be a multiple of 3.
 * @param indexSize The size of the index. Must be sizeof(uint16_t) or sizeof(uint32_t).
 * @param density The density of the cconvex hull. This must be > 0.
 * @return False if the parameters are invalid.
 */
DS_PHYSICS_EXPORT bool dsPhysicsMassProperties_initializeMesh(
	dsPhysicsMassProperties* massProperties, const void* vertices, uint32_t vertexCount,
	size_t vertexStride, const void* indices, uint32_t indexCount, size_t indexSize,
	float density);

/**
 * @brief Initializes mass properties by combining multiple mass properties together.
 *
 * This can be used to create inertia for a compound shape. The inertia frame of reference will be
 * at the center of mass and identity rotation.
 *
 * @remark errno will be set on failure.
 * @param[out] massProperties The mass properties to initialize.
 * @param componentMassProperties Array of pointers to the mass properties to combine.
 * @param componentMassPropertiesCount The number of mass properties to combine.
 * @return False if the parameters are invalid.
 */
DS_PHYSICS_EXPORT bool dsPhysicsMassProperties_initializeCombined(
	dsPhysicsMassProperties* massProperties,
	const dsPhysicsMassProperties* const* componentMassProperties,
	uint32_t componentMassPropertiesCount);

/**
 * @brief Sets the mass for the mass properties.
 *
 * This will scale the inertia for the change in mass.
 *
 * @remark errno will be set on failure.
 * @param massProperties The mass properties.
 * @param mass The new mass.
 * @return False if the parameters are invalid.
 */
DS_PHYSICS_EXPORT bool dsPhysicsMassProperties_setMass(dsPhysicsMassProperties* massProperties,
	float mass);

/**
 * @brief Transforms the mass properties.
 *
 * This will move the inertia relative to object space. Scale will be applied immediately, but the
 * translate and rotate will only only modify the inertia transform factors.
 *
 * The transform will be applied in the order of scale, rotate, translate.
 *
 * @remark A non-uniform scale may not be applied if there is already a rotation on the mass
 * properties as this would introduce a shear.
 *
 * @remark errno will be set on failure.
 * @param massProperties The mass properties.
 * @param translate The amount to translate or NULL to not translate.
 * @param rotate The amount to rotate or NULL to not rotate.
 * @param scale The amount to scale or NULL to not scale. This will be applied immediately and
 *     modify both the inertia tensor and mass.
 * @return False if the transforn couldn't be applied.
 */
DS_PHYSICS_EXPORT bool dsPhysicsMassProperties_transform(dsPhysicsMassProperties* massProperties,
	const dsVector3f* translate, const dsQuaternion4f* rotate, const dsVector3f* scale);

/**
 * @brief Shifts mass properties so that the frame for the inertia tensor is moved, but the shape's
 * center of mass remains the same.
 *
 * This will modify the inertia translate and rotate as with dsPhysicsMassProperties_transform(),
 * but will also adjust the inertia tensor to match the change. Conceptually, this will translate
 * the point of rotation away from the center of mass and adjust the reference orientation to an
 * arbitrary axis while keeping the shape in the same position and orientation.
 *
 * @remark errno will be set on failure.
 * @param massProperties The mass properties.
 * @param translate The amount to translte or NULL to not translate.
 * @param rotate The amount to rotate or NULL to not rotate.
 * @return False if the shift couldn't be applied.
 */
DS_PHYSICS_EXPORT bool dsPhysicsMassProperties_shift(dsPhysicsMassProperties* massProperties,
	const dsVector3f* translate, const dsQuaternion4f* rotate);

/**
 * @brief Gets the final inertia tensor for mass properties relative to inertiaTranslate and
 *     inertiaRotate.
 * @remark errno will be set on failure.
 * @param[out] outInertia The final inertia tensor.
 * @param massProperties The mass properties.
 * @return False if the parameters are invalid.
 */
DS_PHYSICS_EXPORT bool dsPhysicsMassProperties_getInertia(dsMatrix33f* outInertia,
	const dsPhysicsMassProperties* massProperties);

/**
 * @brief Gets the final inertia tensor for mass properties relative to inertiaTranslate and
 * inertiaRotate, decomposed into the rotation and diagonal.
 *
 * Multiplying the matrices
 * outInertiaRotate*diagonal(outInertiaDiagonal)*transpose(outInertiaRotate) will give the same
 * matrix as dsPhysicsMassProperties_getInertia().
 *
 * @remark errno will be set on failure.
 * @param[out] outInertiaRotate The rotation for the final inertia tensor. This does NOT include the
 *     rotation from inertiaRotate.
 * @param[out] outInertiaDiagonal The diagonal factor for the inertia. These are the scales applied
 *     in the frame of outInertiaRotate to restore the original matrix.
 * @param massProperties The mass properties.
 * @return False if the parameters are invalid.
 */
DS_PHYSICS_EXPORT bool dsPhysicsMassProperties_getDecomposedInertia(dsMatrix33f* outInertiaRotate,
	dsVector3f* outInertiaDiagonal, const dsPhysicsMassProperties* massProperties);

#ifdef __cplusplus
}
#endif

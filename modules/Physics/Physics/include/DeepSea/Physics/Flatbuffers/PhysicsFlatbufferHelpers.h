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

#include <DeepSea/Math/Quaternion.h>
#include <DeepSea/Math/Types.h>

#include <DeepSea/Physics/Types.h>

#include <cstring>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include <DeepSea/Physics/Flatbuffers/PhysicsCommon_generated.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

/**
 * @file
 * @brief Helper functions for working with Physics flatbuffer types.
 */

namespace DeepSeaPhysics
{

/**
 * @brief Converts from a flatbuffer Vector3f to a dsVector3f.
 * @param vector The vector to convert.
 * @return The converted vector.
 */
inline const dsVector3f& convert(const Vector3f& vector)
{
	return reinterpret_cast<const dsVector3f&>(vector);
}

/**
 * @brief Converts from a flatbuffer Quaternion4f to a dsQuaternion4f.
 * @param quaternion The quaternion to convert.
 * @return The converted quaternion.
 */
inline dsQuaternion4f convert(const Quaternion4f& quaternion)
{
	// Avoid unaligned access.
	dsQuaternion4f value = {{quaternion.i(), quaternion.j(), quaternion.k(), quaternion.r()}};
	return value;
}

/**
 * @brief Converts from a flatbuffer Matrix33f to a dsMatrix33f.
 * @param matrix The matr9x to convert.
 * @return The converted matrix.
 */
inline const dsMatrix33f& convert(const Matrix33f& matrix)
{
	return reinterpret_cast<const dsMatrix33f&>(matrix);
}

/**
 * @brief Converts from a flatbuffer Axis to a dsPhysicsAxis.
 * @param axis The axis to convert.
 * @return The converted axis.
 */
inline dsPhysicsAxis convert(Axis axis)
{
	return static_cast<dsPhysicsAxis>(axis);
}

/**
 * @brief Converts from a flatbuffer ShapePartMaterial to a dsPhysicsShapePartMaterial.
 * @param material The shape part material to convert.
 * @return The converted shape part material.
 */
inline dsPhysicsShapePartMaterial convert(const ShapePartMaterial& material)
{
	dsPhysicsShapePartMaterial value =
		{material.friction(), material.restitution(), material.hardness()};
	return value;
}

/**
 * @brief Converts from a flatbuffer MotionType to a dsPhysicsMotionType.
 * @param motionType The motion type to convert.
 * @return The converted motion type.
 */
inline dsPhysicsMotionType convert(MotionType motionType)
{
	return static_cast<dsPhysicsMotionType>(motionType);
}

/**
 * @brief Converts from a flatbuffer DOFMask to a dsPhysicsDOFMask.
 * @param mask The DOF mask to convert.
 * @return The converted DOF mask.
 */
inline dsPhysicsDOFMask convert(DOFMask mask)
{
	return static_cast<dsPhysicsDOFMask>(mask);
}

/**
 * @brief Converts from a flatbuffer PhysicsLayer to a dsPhysicsLayer.
 * @param layer The physics layer to convert.
 * @return The converted physics layer.
 */
inline dsPhysicsLayer convert(PhysicsLayer layer)
{
	return static_cast<dsPhysicsLayer>(layer);
}

/**
 * @brief Converts from a flatbuffer RigidBodyFlags to a dsRigidBodyFlags.
 * @param flags The rigid body flags to convert.
 * @return The converted physics rigid body flags.
 */
inline dsRigidBodyFlags convert(RigidBodyFlags flags)
{
	return static_cast<dsRigidBodyFlags>(flags);
}

/**
 * @brief Converts from a flatbuffer MassProperties to a dsPhysicsMassProperties.
 * @param massProperties The mass properties to convert.
 * @return The converted mass properties.
 */
inline dsPhysicsMassProperties convert(const MassProperties& massProperties)
{
	dsPhysicsMassProperties value;
	value.centeredInertia = DeepSeaPhysics::convert(*massProperties.centeredInertia());

	auto fbInertiaTranslate = massProperties.inertiaTranslate();
	if (fbInertiaTranslate)
		value.inertiaTranslate = convert(*fbInertiaTranslate);
	else
		std::memset(&value.inertiaTranslate, 0, sizeof(dsVector3f));

	auto fbCenterOfMass = massProperties.centerOfMass();
	if (fbCenterOfMass)
		value.centerOfMass = convert(*fbCenterOfMass);
	else
		value.centerOfMass = value.inertiaTranslate;

	auto fbInertiaRotate = massProperties.inertiaRotate();
	if (fbInertiaRotate)
		value.inertiaRotate = convert(*fbInertiaRotate);
	else
		dsQuaternion4_identityRotation(value.inertiaRotate);

	value.mass = massProperties.mass();
	return value;
}

} // namespace DeepSeaPhysics

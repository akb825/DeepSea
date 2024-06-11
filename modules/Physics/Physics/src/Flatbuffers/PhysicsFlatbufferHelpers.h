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

#include <DeepSea/Math/Types.h>
#include <DeepSea/Physics/Types.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/PhysicsCommon_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

namespace DeepSeaPhysics
{

inline const dsVector3f& convert(const Vector3f& vector)
{
	return reinterpret_cast<const dsVector3f&>(vector);
}

inline dsQuaternion4f convert(const Quaternion4f& quaternion)
{
	// Avoid unaligned access.
	dsQuaternion4f value = {{quaternion.i(), quaternion.j(), quaternion.k(), quaternion.r()}};
	return value;
}

inline dsPhysicsAxis convert(Axis axis)
{
	return static_cast<dsPhysicsAxis>(axis);
}

inline dsPhysicsShapePartMaterial convert(const ShapePartMaterial& material)
{
	dsPhysicsShapePartMaterial value =
		{material.friction(), material.restitution(), material.hardness()};
	return value;
}

} // namespace DeepSeaPhysics

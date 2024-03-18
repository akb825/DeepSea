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
#include <DeepSea/Physics/Shapes/Types.h>
#include <DeepSea/Physics/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for manipulating physics shapes.
 * @see dsPhysicsShape
 */

/**
 * @brief Initializes a physics shape.
 *
 * This is called by the specific shape's initialize functions.
 *
 * @remark errno will be set on failure.
 * @param shape The shape to initialize.
 * @param engine The physics engine creating the shape.
 * @param allocator The allocator the shape was created with.
 * @param type The type of the shape.
 * @param bounds The bounds of the shape.
 * @param impl The underlying implementation of the shape.
 * @param destroyFunc The function to destroy the shape with when the reference count reaches 0.
 * @return False if the parameters are invalid.
 */
DS_PHYSICS_EXPORT bool dsPhysicsShape_initialize(dsPhysicsShape* shape, dsPhysicsEngine* engine,
	dsAllocator* allocator, const dsPhysicsShapeType* type, const dsAlignedBox3f* bounds,
	void* impl, dsDestroyPhysicsShapeFunction destroyFunc);

/**
 * @brief Gets the mass properties for a shape.
 * @remark errno will be set on failure.
 * @param[out] outMassProperties The mass properties to populate.
 * @param shape The shape to get the mass properties for.
 * @param density The density of the shape. This must be > 0.
 * @return False if the parameters are invalid or the shape isn't valid to get mass properties for.
 */
DS_PHYSICS_EXPORT bool dsPhysicsShape_getMassProperties(dsPhysicsMassProperties* outMassProperties,
	const dsPhysicsShape* shape, float density);

/**
 * @brief Gets the material for a shape.
 * @remark errno will be set on failure.
 * @param[out] outMaterial The material to populate.
 * @param shape The shape to get the material for.
 * @param faceIndex The face index to get the material for.
 * @return False if no material is available.
 */
DS_PHYSICS_EXPORT bool dsPhysicsShape_getMaterial(dsPhysicsShapePartMaterial* outMaterial,
	const dsPhysicsShape* shape, uint32_t faceIndex);

/**
 * @brief Increments the reference count to the shape.
 * @remark This function is thread-safe.
 * @param shape The shape to increment the reference count to.
 * @return The shape with the incremented reference count.
 */
DS_PHYSICS_EXPORT dsPhysicsShape* dsPhysicsShape_addRef(dsPhysicsShape* shape);

/**
 * @brief Decrements the reference count to the shape.
 *
 * Once the reference count reaches 0 the shape will be destroyed.
 *
 * @remark This function is thread-safe.
 * @param shape The shape to decrement the reference count from.
 */
DS_PHYSICS_EXPORT void dsPhysicsShape_freeRef(dsPhysicsShape* shape);

#ifdef __cplusplus
}
#endif

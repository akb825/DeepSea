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
#include <DeepSea/ScenePhysics/Types.h>
#include <DeepSea/ScenePhysics/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function for registering dsPhysicsConstraint with dsSceneResources.
 */

/**
 * @brief The type name for a scene constraint.
 */
DS_SCENEPHYSICS_EXPORT extern const char* const dsScenePhysicsConstraint_typeName;

/**
 * @brief Gets the type for the dsScenePhysicsConstraint custom type for storage in dsSceneResources.
 * @return The custom type.
 */
DS_SCENEPHYSICS_EXPORT const dsCustomSceneResourceType* dsScenePhysicsConstraint_type(void);

/**
 * @brief Creates a scene physics constraint.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the scene constraint.
 * @param constraint The base physics constraint. This will take ownership of the constraint,
 *     and destroy it immediately if creation fails.
 * @param firstRigidBody The name for the first rigid body, or NULL to use what exists in the base
 *     constraint.
 * @param firstConnectedConstraint The name for the constraint for the first actor that is related
 *     to this constraint, or NULL to use what exists in the base constraint. This will be copied if
 *     not NULL.
 * @param secondRigidBody The name for the second rigid body, or NULL to use what exists in the base
 *     constraint. This will be copied if not NULL.
 * @param secondConnectedConstraint The name for the constraint for the second actor that is related
 *     to this constraint, or NULL to use what exists in the base constraint. This will be copied if
 *     not NULL.
 * @return The scene constraint or NULL if an error occurred.
 */
DS_SCENEPHYSICS_EXPORT dsScenePhysicsConstraint* dsScenePhysicsConstraint_create(
	dsAllocator* allocator, dsPhysicsConstraint* constraint, const char* firstRigidBody,
	const char* firstConnectedConstraint, const char* secondRigidBody,
	const char* secondConnectedConstraint);

/**
 * @brief Destroys a scene physics constraint.
 * @remark errno will be set on failure.
 * @param constraint The constraint to destroy.
 * @return False if the constraint couldn't be destroyed.
 */
DS_SCENEPHYSICS_EXPORT bool dsScenePhysicsConstraint_destroy(dsScenePhysicsConstraint* constraint);

/**
 * @brief Creates a custom resource to wrap a dsScenePhysicsConstraint.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the custom resource.
 * @param constraint The constraint to wrap.
 * @return The custom resource or NULL if an error occurred.
 */
DS_SCENEPHYSICS_EXPORT dsCustomSceneResource* dsScenePhysicsConstraint_createResource(
	dsAllocator* allocator, dsPhysicsConstraint* constraint);

#ifdef __cplusplus
}
#endif

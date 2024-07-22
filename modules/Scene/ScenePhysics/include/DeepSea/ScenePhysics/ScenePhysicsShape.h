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
#include <DeepSea/Physics/Types.h>
#include <DeepSea/Scene/Types.h>
#include <DeepSea/ScenePhysics/Export.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Function for registering dsPhysicsShape with dsSceneResources.
 */

/**
 * @brief The type name for a scene shape.
 */
DS_SCENEPHYSICS_EXPORT extern const char* const dsScenePhysicsShape_typeName;

/**
 * @brief Gets the type for the dsScenePhysicsShape custom type for storage in dsSceneResources.
 * @return The custom type.
 */
DS_SCENEPHYSICS_EXPORT const dsCustomSceneResourceType* dsScenePhysicsShape_type(void);

/**
 * @brief Creates a custom resource to wrap a dsPhysicsShape.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the custom resource.
 * @param shape The shape to wrap. The reference count will be incremented on creation.
 * @return The custom resource or NULL if an error occurred.
 */
DS_SCENEPHYSICS_EXPORT dsCustomSceneResource* dsScenePhysicsShape_create(dsAllocator* allocator,
	dsPhysicsShape* shape);

#ifdef __cplusplus
}
#endif

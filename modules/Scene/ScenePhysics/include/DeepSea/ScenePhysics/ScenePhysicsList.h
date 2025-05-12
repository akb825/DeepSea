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
#include <DeepSea/ScenePhysics/Export.h>
#include <DeepSea/ScenePhysics/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating and manipulating scene physics lists.
 *
 * This is responsible for creating the per-instance data for dsRigidBodyNode and updating and
 * managing a dsPhysicsScene. There should only be a single physics scene list within a scene.
 *
 * A dsSceneShiftNode may be added to the physics list, in which case the positions will be shifted
 * based on the origin. Only one shift node may be used.
 */

/**
 * @brief The scene physics list type name.
 */
DS_SCENEPHYSICS_EXPORT extern const char* const dsScenePhysicsList_typeName;

/**
 * @brief Gets the type of a scene physics list.
 * @return The type of a scene physics list.
 */
DS_SCENEPHYSICS_EXPORT dsSceneItemListType dsScenePhysicsList_type(void);

/**
 * @brief Creates a scene physics list.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the list with. This must support freeing memory.
 * @param name The name of the scene physics list. This will be copied.
 * @param physicsScene The physics scene to manage.
 * @param takeOwnership Whether to take ownership of the physics scene. If true and creation fails,
 *     the physics scene will be destroyed immediately.
 * @param targetStepTime The step time that is desired. This will keep each step as close to this
 *     time as possible.
 * @return The scene physics list or NULL if an error occurred.
 */
DS_SCENEPHYSICS_EXPORT dsSceneItemList* dsScenePhysicsList_create(dsAllocator* allocator,
	const char* name, dsPhysicsScene* physicsScene, bool takeOwnership, float targetStepTime);

/**
 * @brief Gets a physics scene from a scene.
 *
 * This will check for a scene physics list within the scene.
 *
 * @remark errno will be set on failure.
 * @param scene The scene to find the physics scene in.
 * @return The physics scene or NULL if there isn't one present.
 */
DS_SCENEPHYSICS_EXPORT dsPhysicsScene* dsScenePhysicsList_getPhysicsScene(dsScene* scene);

#ifdef __cplusplus
}
#endif

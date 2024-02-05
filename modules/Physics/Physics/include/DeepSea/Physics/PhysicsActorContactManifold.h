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
 * @brief Functions to work with physics actor contact manifolds.
 * @see dsPhysicsActorContactManifold
 * @see dsPhysicsActorContactPoint
 * @see dsPhysicsActorContactSetting
 */

/**
 * @brief Gets a contact point for a contact manifold.
 * @remark errno will be set on failure.
 * @param[out] outPoint The storage for the contact point.
 * @param manifold The contact manifold to get the contact point for.
 * @param index The index of the point.
 * @return False if the contact point couldn't be queried.
 */
DS_PHYSICS_EXPORT bool dsPhysicsActorContactManifold_getContactPoint(
	dsPhysicsActorContactPoint* outPoint, const dsPhysicsActorContactManifold* manifold,
	uint32_t index);

/**
 * @brief Gets contact properties for a contact manifold.
 * @remark errno will be set on failure.
 * @param[out] outProperties The storage for the contact properties.
 * @param manifold The contact manifold to get the contact properties for.
 * @param index The index of the contact properties.
 * @return False if the contact properties couldn't be queried.
 */
DS_PHYSICS_EXPORT bool dsPhysicsActorContactManifold_getContactProperties(
	dsPhysicsActorContactProperties* outProperties, const dsPhysicsActorContactManifold* manifold,
	uint32_t index);

/**
 * @brief Sets contact properties for a contact manifold.
 * @remark errno will be set on failure.
 * @param manifold The contact manifold to set the contact properties on.
 * @param index The index of the contact properties.
 * @param properties The contact properties to set.
 * @return False if the contact properties couldn't be set.
 */
DS_PHYSICS_EXPORT bool dsPhysicsActorContactManifold_setContactProperties(
	dsPhysicsActorContactManifold* manifold, uint32_t index,
	const dsPhysicsActorContactProperties* properties);

#ifdef __cplusplus
}
#endif

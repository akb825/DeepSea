/*
 * Copyright 2022 Aaron Barany
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
#include <DeepSea/SceneParticle/Export.h>
#include <DeepSea/SceneParticle/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for creating scene particle emitter factories.
 */

/**
 * @brief Gets the type of a scene particle emitter factory.
 * @return The type of a scene particle emitter factory.
 */
DS_SCENEPARTICLE_EXPORT const dsCustomSceneResourceType* dsSceneParticleEmitterFactory_type(void);

/**
 * @brief Creates a scene particle emitter factory.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the factory with.
 * @param createEmitterFunc Function to create the particle emitter with.
 * @param userData User data used to aid in creating particle emitters.This may be NULL if the user
 *     data doesn't need to be destroyed. This will be called if an error occurs when creating the
 *     factory.
 * @param destroyUserDataFunc Function to destroy the user data.
 * @return The particle draw or NULL if an error occurred.
 */
DS_SCENEPARTICLE_EXPORT dsSceneParticleEmitterFactory* dsSceneParticleEmitterFactory_create(
	dsAllocator* allocator, dsCreateSceneParticleNodeEmitterFunction createEmitterFunc,
	void* userData, dsDestroySceneUserDataFunction destroyUserDataFunc);

/**
 * @brief Destroys a scene particle emitter factory.
 * @param factory The scene particle emitter factory to destroy.
 */
DS_SCENEPARTICLE_EXPORT void dsSceneParticleEmitterFactory_destroy(
	dsSceneParticleEmitterFactory* factory);

#ifdef __cplusplus
}
#endif

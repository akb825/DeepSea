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
 * @brief Function to create a particle emitter factory for a standard particle emitter.
 */

/**
 * @brief The type name for a scene standard particle emitter factory.
 */
DS_SCENEPARTICLE_EXPORT extern const char* const dsSceneStandardParticleEmitterFactory_typeName;

/**
 * @brief Creates a scene standard particle emitter factory.
 * @remark errno will be set on failure.
 * @param allocator The allocator to create the factory with.
 * @param params The particle emitter parameters. This will be copied.
 * @param seed The initial seed for the particles. This will be updated each new emitter.
 * @param options The stnadanrd particle emitter options.
 * @param enabled Whether or not the emitters are enabled on creation.
 * @param startTime The time to start the particle emitter at. The first frame this is updated the
 *     create particles and advance them to this time.
 * @param relativeNode The node to make the transform of the particles relative to. This should be
 *     an ancestor to the node the emitter factory will be used with. This won't hold a reference
 *     count to avoid cycles, and will be treated as NULL if not valid or not an ancestor node. If
 *     set, the particles will use that transform and the volume transform will be the relative
 *     transform to the particle node. If unset, the particles will use the particle node's
 *     transform and the volume transform will be used.
 * @return The particle draw or NULL if an error occurred.
 */
DS_SCENEPARTICLE_EXPORT dsSceneParticleEmitterFactory* dsSceneStandardParticleEmitterFactory_create(
	dsAllocator* allocator, const dsParticleEmitterParams* params, uint32_t seed,
	const dsStandardParticleEmitterOptions* options, bool enabled, float startTime,
	const dsSceneNode* relativeNode);

/**
 * @brief Gets the particle emitter parameters from a standard particle emitter factory.
 * @remark errno will be set on failure.
 * @param factory The factory to get the parameters for.
 * @return The parameters or NULL if th factory is NULL or not the correct type.
 */
DS_SCENEPARTICLE_EXPORT dsParticleEmitterParams* dsSceneParticleEmitterFactory_getEmitterParams(
	dsSceneParticleEmitterFactory* factory);

/**
 * @brief Gets the standard particle emitter options from a standard particle emitter factory.
 * @remark errno will be set on failure.
 * @param factory The factory to get the options for.
 * @return The options or NULL if th factory is NULL or not the correct type.
 */
DS_SCENEPARTICLE_EXPORT dsStandardParticleEmitterOptions* dsSceneParticleEmitterFactory_getSandardOptions(
	dsSceneParticleEmitterFactory* factory);

/**
 * @brief Gets the current seed for a standard particle emitter factory.
 * @param factory The factory to get the seed for.
 * @return The current seed.
 */
DS_SCENEPARTICLE_EXPORT uint32_t dsSceneStandardParticleEmitterFactory_getSeed(
	const dsSceneParticleEmitterFactory* factory);

/**
 * @brief Sets the current seed for a standard particle emitter factory.
 * @remark errno will be set on failure.
 * @param factory The factory to set the seed for.
 * @param seed The new seed for the factory.
 * @return False if the factory is invalid.
 */
DS_SCENEPARTICLE_EXPORT bool dsSceneStandardParticleEmitterFactory_setSeed(
	const dsSceneParticleEmitterFactory* factory, uint32_t seed);

/**
 * @brief Gets the current enabled state for a standard particle emitter factory.
 * @param factory The factory to get the enabled state for.
 * @return Whether or not the particle emitters are enabled when created.
 */
DS_SCENEPARTICLE_EXPORT bool dsSceneStandardParticleEmitterFactory_getEnabled(
	const dsSceneParticleEmitterFactory* factory);

/**
 * @brief Sets the current enabled state for a standard particle emitter factory.
 * @remark errno will be set on failure.
 * @param factory The factory to set the seed for.
 * @param enabled Whether or not the particle emitters are enabled when created.
 * @return False if the factory is invalid.
 */
DS_SCENEPARTICLE_EXPORT bool dsSceneStandardParticleEmitterFactory_setEnabled(
	const dsSceneParticleEmitterFactory* factory, bool enabled);

/**
 * @brief Gets the relative node for a standard particle emitter factory.
 * @param factory The factory to get the enabled state for.
 * @return The relative node used for the particle transformations.
 */
DS_SCENEPARTICLE_EXPORT const dsSceneNode* dsSceneStandardParticleEmitterFactory_getRelativeNode(
	const dsSceneParticleEmitterFactory* factory);

/**
 * @brief Sets the relative node for a standard particle emitter factory.
 * @remark errno will be set on failure.
 * @param factory The factory to set the seed for.
 * @param relativeNode The relative node used for the particle transformations.
 * @return False if the factory is invalid.
 */
DS_SCENEPARTICLE_EXPORT bool dsSceneStandardParticleEmitterFactory_setRelativeNode(
	const dsSceneParticleEmitterFactory* factory, const dsSceneNode* relativeNode);

#ifdef __cplusplus
}
#endif

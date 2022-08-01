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
#include <DeepSea/Particle/Export.h>
#include <DeepSea/Particle/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Functions for working with particle volumes.
 * @remark These use asserts rather than error checking for each of the functions. As many particles
 *     may be used at once, performance is considered more important than safety.
 * @see dsParticleVolume
 */

/**
 * @brief Generates a random position inside a volume.
 * @param[out] result The result position.
 * @param[inout] random The random number generator.
 * @param volume The volume to create the position from.
 */
DS_PARTICLE_EXPORT void dsParticleVolume_randomPosition(dsVector3f* result, dsRandom* random,
	const dsParticleVolume* volume);

#ifdef __cplusplus
}
#endif

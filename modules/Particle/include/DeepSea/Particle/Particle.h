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
 * @brief Functions for working with particles.
 * @remark These use asserts rather than error checking for each of the functions. As many particles
 *     may be used at once, performance is considered more important than safety.
 * @see dsParticle
 */

/**
 * @brief Generates a random position for a particle.
 * @param[inout] particle The particle to create the position for.
 * @param[inout] seed The random seed to generate the random position from.
 * @param volume The volume to create the position from.
 * @param volumeMatrix The transform matrix for the volume.
 */
DS_PARTICLE_EXPORT void dsParticle_randomPosition(dsParticle* particle, uint32_t* seed,
	const dsParticleVolume* volume, const dsMatrix44f* volumeMatrix);

/**
 * @brief Creates a direction matrix for use in dsParticle_randomDirection() with a single base
 *     direction.
 * @param[out] result The direction matrix.
 * @param baseDirection The base direction to orient the matrix. This is expected to be normalized.
 */
DS_PARTICLE_EXPORT void dsParticle_createDirectionMatrix(dsMatrix33f* result,
	const dsVector3f* baseDirection);

/**
 * @brief Generates a random direction for a particle.
 * @param[inout] particle The particle to create the direction for.
 * @param[inout] seed The random seed to generate the random direction from.
 * @param directionMatrix The matrix for orienting the direction. The Z axis (column 2) is the base
 *     direction.
 * @param directionSpread The spread along the base direction as an angle in radians. A value of 0
 *     will always follow the base direction, pi/2 would be a hemisphere, and pi would be a full
 *     sphere.
 */
DS_PARTICLE_EXPORT void dsParticle_randomDirection(dsParticle* particle, uint32_t* seed,
	const dsMatrix33f* directionMatrix, float directionSpread);

/**
 * @brief Generates a random rotation for a particle.
 * @param[inout] particle The particle to create the rotation for.
 * @param[inout] seed The random seed to generate the random rotation from.
 * @param xRotationRange The minimum and maximum random rotation in radians around the X axis. The
 *     minimum can be larger than the maximum to wrap around the 2*PI boundary.
 * @param yRotationRange The minimum and maximum random rotation in radians around the Y axis. The
 *     minimum can be larger than the maximum to wrap around the 2*PI boundary.
 */
DS_PARTICLE_EXPORT void dsParticle_randomRotation(dsParticle* particle, uint32_t* seed,
	const dsVector2f* xRotationRange, const dsVector2f* yRotationRange);

/**
 * @brief Generates a random color for a particle.
 * @param[inout] particle The particle to create the color for.
 * @param[inout] seed The random seed to generate the random color from.
 * @param hueRange The minimum and maximum hue values for the color in the range [0, 360]. The
 *     minimum can be larger than the maximum, which will wrap around. (e.g. min 300 and max 60
 *     will wrap around at 360 back to 0)
 * @param saturationRange The minimum and maximum saturation values for the color in the range
 *     [0, 1].
 * @param valueRange The minimum and maximum values for the color in the range [0, 1].
 */
DS_PARTICLE_EXPORT void dsParticle_randomColor(dsParticle* particle, uint32_t* seed,
	const dsVector2f* hueRange, const dsVector2f* saturationRange, const dsVector2f* valueRange);

/**
 * @brief Generates a random intensity for a particle.
 * @param[inout] particle The particle to create the color for.
 * @param[inout] seed The random seed to generate the random color from.
 * @param intensityRange The minimum and maximum intensity values.
 */
DS_PARTICLE_EXPORT void dsParticle_randomIntensity(dsParticle* particle, uint32_t* seed,
	const dsVector2f* intensityRange);

/**
 * @brief Generates a random texture index for a particle.
 * @param[inout] particle The particle to create the color for.
 * @param[inout] seed The random seed to generate the random color from.
 * @param textureRange The minimum and maximum texture indices.
 */
DS_PARTICLE_EXPORT void dsParticle_randomTexture(dsParticle* particle, uint32_t* seed,
	const dsVector2i* textureRange);

#ifdef __cplusplus
}
#endif


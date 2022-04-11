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
#include <DeepSea/Core/Types.h>
#include <DeepSea/Geometry/Types.h>
#include <DeepSea/Math/Types.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @file
 * @brief Includes all of the types used in the DeepSea/Particle library.
 */

/**
 * @brief Log tag used by the particle library.
 */
#define DS_PARTICLE_LOG_TAG "particle"

/**
 * @brief Enum for a volume used to create particles in.
 */
typedef enum dsParticleVolumeType
{
	dsParticleVolumeType_Box,     ///< Aligned box.
	dsParticleVolumeType_Sphere,  ///< Sphere.
	dsParticleVolumeType_Cylinder ///< Cylinder.
} dsParticleVolumeType;

/**
 * @brief Struct describing the volume to create particles in.
 * @see ParticleVolume.h
 */
typedef struct dsParticleVolume
{
	/**
	 * @brief The type of the volume.
	 */
	dsParticleVolumeType type;

	union
	{
		/**
		 * @brief The box when type is dsParticleVolumeType_Box.
		 */
		dsAlignedBox3f box;

		/**
		 * @brief The spehre when the type is dsParticleVolumeType_Sphere.
		 */
		struct
		{
			/**
			 * @brief The center of the sphere.
			 */
			dsVector3f center;

			/**
			 * @brief The radius of the sphere.
			 */
			float radius;
		} sphere;

		/**
		 * @brief The cylinder when the type is dsParticleVolumeType_Cylinder.
		 */
		struct
		{
			/**
			 * @brief The center of the cylinder.
			 */
			dsVector3f center;

			/**
			 * @brief The radius of the cylinder along the XY plane.
			 */
			float radius;

			/**
			 * @brief The height of the cylinder along the Z axis.
			 */
			float height;
		} cylinder;
	};
} dsParticleVolume;

/**
 * @brief Struct describing a single particle.
 *
 * Different particle emitters may allocate more space for each particle for extra state used by the
 * emitter.
 *
 * @see Particle.h
 */
typedef struct dsParticle
{
	/**
	 * @brief The position of the particle.
	 */
	dsVector3f position;

	/**
	 * @brief The size of the particle.
	 */
	dsVector2f size;

	/**
	 * @brief The direction of the particle.
	 */
	dsVector3f direction;

	/**
	 * @brief The rotation of the particle.
	 */
	dsVector2f rotation;

	/**
	 * @brief The color of the particle.
	 */
	dsColor color;

	/**
	 * @brief The intensity of the particle.
	 *
	 * This can be used to make the color brighter, such as for emissive particles.
	 */
	float intensity;

	/**
	 * @brief Index of the texture when using a texture array.
	 */
	uint32_t textureIndex;

	/**
	 * @brief T value for the lifetime of the particle.
	 *
	 * This should be in the range [0, 1].
	 */
	float t;
} dsParticle;

/**
 * @brief Struct describing an emitter of particles.
 *
 * Different implementations can effectively subclass this type by having it as the first member of
 * the structure. This can be done to add additional data to the structure and have it be freely
 * casted between dsParticleEmitter and the true internal type.
 *
 * @see ParticleEmitter.h
 */
typedef struct dsParticleEmitter dsParticleEmitter;

/**
 * @brief Function to update a particle emitter.
 * @param emitter The particle emitter to update.
 * @param time The time that has elapsed from the last update.
 * @param curParticles The current list of particles.
 * @param curParticleCount The number of currently active particles.
 * @param nextParticles The list of next particles to populate.
 * @return The new number of particles.
 */
typedef uint32_t (*dsUpdateParticleEmitterFunction)(dsParticleEmitter* emitter, float time,
	const uint8_t* curParticles, uint32_t curParticleCount, uint8_t* nextParticles);

/**
 * @brief Function to destroy a particle emitter.
 * @param emitter The particle emitter to destroy.
 */
typedef void (*dsDestroyParticleEmitterFunction)(dsParticleEmitter* emitter);

/**
 * @copydoc dsParticleEmitter
 */
struct dsParticleEmitter
{
	/**
	 * @brief The allocator the particle emitter was created with.
	 */
	dsAllocator* allocator;

	/**
	 * @brief The list of active particles.
	 */
	uint8_t* particles;

	/**
	 * @brief Temporary list of particles used during processing.
	 */
	uint8_t* tempParticles;

	/**
	 * @brief The size of a particle.
	 */
	uint32_t sizeofParticle;

	/**
	 * @brief The current number of particles.
	 */
	uint32_t particleCount;

	/**
	 * @brief The maximum number of particles that can be active at once.
	 */
	uint32_t maxParticles;

	/**
	 * @brief Function to update the particle emitter.
	 */
	dsUpdateParticleEmitterFunction updateFunc;

	/**
	 * @brief Function to destroy the particle emitter.
	 */
	dsDestroyParticleEmitterFunction destroyFunc;
};

/**
 * @brief Struct describing options for controlling a particle emitter.
 * @param StandardParticleEmitter.h
 */
typedef struct dsStandardParticleEmitterOptions
{
	/**
	 * @brief The volume to spawn particles in.
	 */
	dsParticleVolume spawnVolume;

	/**
	 * @brief The matrix to transform the volume when spawning particles.
	 */
	dsMatrix44f volumeMatrix;

	/**
	 * @brief The minimum and maximum width of the particle.
	 */
	dsVector2f widthRange;

	/**
	 * @brief The minimum and maximum height of the particle.
	 *
	 * Set to negative values to guarantee the particle remains square.
	 */
	dsVector2f heightRange;

	/**
	 * @brief The base direction particles move in.
	 */
	dsVector3f baseDirection;

	/**
	 * @brief The spread along the base direction as an angle in radians.
	 *
	 * A value of 0 will always follow the base direction, pi/2 would be a hemisphere, and pi would
	 * be a full sphere.
	 */
	float directionSpread;

	/**
	 * @brief The minimum and maximum time in seconds between spawning particles.
	 */
	dsVector2f spawnTimeRange;

	/**
	 * @brief The minimum and maximum time in seconds a particle is active for.
	 */
	dsVector2f activeTimeRange;

	/**
	 * @brief The minimum and maximum speed particles travel at.
	 */
	dsVector2f speedRange;

	/**
	 * @brief The minimum and maximum rotation speed in radians per second.
	 */
	dsVector2f rotationRange;

	/**
	 * @brief The minimum and maximum texture indices to use.
	 */
	dsVector2i textureRange;

	/**
	 * @brief The minimum and maximum hue values for the color in the range [0, 360].
	 *
	 * The minimum can be larger than the maximum, which will wrap around. (e.g. min 300 and max 60
	 * will wrap around at 360 back to 0)
	 */
	dsVector2f colorHueRange;

	/**
	 * @brief The minimum and maximum saturation values for the color in the range [0, 1].
	 */
	dsVector2f colorSaturationRange;

	/**
	 * @brief The minimum and maximum values for the color in the range [0, 1].
	 */
	dsVector2f colorValueRange;

	/**
	 * @brief The minimum and maximum intensity values.
	 */
	dsVector2f intensityRange;
} dsStandardParticleEmitterOptions;

/**
 * @brief Struct describing a particle emitter usable in most situations.
 * @see StandardParticleEmitter.h
 */
typedef struct dsStandardParticleEmitter dsStandardParticleEmitter;

#ifdef __cplusplus
}
#endif

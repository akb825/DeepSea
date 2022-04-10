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

#include <DeepSea/Particle/Particle.h>

#include <DeepSea/Core/Error.h>

#include <DeepSea/Math/Color.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Random.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Particle/ParticleVolume.h>

static inline float randomWrappedFloat(uint32_t* seed, float minVal, float maxVal, float wrapVal)
{
	if (minVal > maxVal)
	{
		float val = dsRandomFloat(seed, minVal, maxVal + wrapVal);
		return dsWrapf(val, 0, wrapVal);
	}
	else
		return dsRandomFloat(seed, minVal, maxVal);
}

void dsParticle_randomPosition(dsParticle* particle, uint32_t* seed, const dsParticleVolume* volume,
	const dsMatrix44f* volumeMatrix)
{
	DS_ASSERT(particle);
	DS_ASSERT(seed);
	DS_ASSERT(volume);
	DS_ASSERT(volumeMatrix);

	dsVector4f basePos;
	basePos.w = 1.0f;
	dsParticleVolume_randomPosition((dsVector3f*)&basePos, seed, volume);

	dsVector4f transformedPos;
	dsMatrix44_transform(transformedPos, *volumeMatrix, basePos);
	particle->position = *(dsVector3f*)&transformedPos;
}

void dsParticle_createDirectionMatrix(dsMatrix33f* result, const dsVector3f* baseDirection)
{
	DS_ASSERT(result);
	DS_ASSERT(baseDirection);

	result->columns[2] = *baseDirection;

	result->columns[0].x = 1;
	result->columns[0].y = 0;
	result->columns[0].z = 0;
	if (dsEpsilonEqualf(dsVector3_dot(result->columns[0], result->columns[2]), 1, 1e-4f))
	{
		result->columns[0].x = 0;
		result->columns[1].x = 0;
	}

	dsVector3_cross(result->columns[1], result->columns[2], result->columns[0]);
	dsVector3f_normalize(&result->columns[1], &result->columns[1]);
	dsVector3_cross(result->columns[0], result->columns[1], result->columns[2]);
}

void dsParticle_randomDirection(dsParticle* particle, uint32_t* seed,
	const dsMatrix33f* directionMatrix, float directionSpread)
{
	DS_ASSERT(particle);
	DS_ASSERT(seed);
	DS_ASSERT(directionMatrix);

	if (directionSpread <= 0)
	{
		particle->direction = directionMatrix->columns[2];
		return;
	}

	float theta = dsRandomFloat(seed, 0, (float)(2*M_PI));
	float phi = dsRandomFloat(seed, 0, directionSpread);
	float cosPhi = cosf(phi);
	float sinPhi = sinf(phi);
	dsVector3f direction = {{cosf(theta)*sinPhi, sinf(theta)*sinPhi, cosPhi}};
	dsMatrix33_transform(particle->direction, *directionMatrix, direction);
}

void dsParticle_randomRotation(dsParticle* particle, uint32_t* seed,
	const dsVector2f* xRotationRange, const dsVector2f* yRotationRange)
{
	DS_ASSERT(particle);
	DS_ASSERT(seed);
	DS_ASSERT(xRotationRange);
	DS_ASSERT(yRotationRange);

	const float wrapVal = (float)(2*M_PI);
	particle->rotation.x = randomWrappedFloat(seed, xRotationRange->x, xRotationRange->y, wrapVal);
	particle->rotation.y = randomWrappedFloat(seed, xRotationRange->y, xRotationRange->y, wrapVal);
}

void dsParticle_randomColor(dsParticle* particle, uint32_t* seed, const dsVector2f* hueRange,
	const dsVector2f* saturationRange, const dsVector2f* valueRange)
{
	DS_ASSERT(particle);
	DS_ASSERT(seed);
	DS_ASSERT(hueRange);
	DS_ASSERT(saturationRange);
	DS_ASSERT(valueRange);

	dsHSVColor color = {{randomWrappedFloat(seed, hueRange->x, hueRange->y, 360),
		dsRandomFloat(seed, saturationRange->x, saturationRange->y),
		dsRandomFloat(seed, valueRange->x, valueRange->y)}};
	particle->color = dsColor_fromHSVColor(&color);
}

void dsParticle_randomIntensity(dsParticle* particle, uint32_t* seed,
	const dsVector2f* intensityRange)
{
	DS_ASSERT(particle);
	DS_ASSERT(seed);
	DS_ASSERT(intensityRange);

	particle->intensity = dsRandomFloat(seed, intensityRange->x, intensityRange->y);
}

void dsParticle_randomTexture(dsParticle* particle, uint32_t* seed, const dsVector2i* textureRange)
{
	DS_ASSERT(particle);
	DS_ASSERT(seed);
	DS_ASSERT(textureRange);

	particle->textureIndex = dsRandomInt(seed, textureRange->x, textureRange->y);
}
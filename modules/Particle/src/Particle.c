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

static inline float randomWrappedFloat(dsRandom* random, float minVal, float maxVal, float wrapVal)
{
	if (minVal > maxVal)
	{
		float val = dsRandom_nextFloatRange(random, minVal, maxVal + wrapVal);
		return dsWrapf(val, 0, wrapVal);
	}
	else
		return dsRandom_nextFloatRange(random, minVal, maxVal);
}

void dsParticle_randomPosition(dsParticle* particle, dsRandom* random,
	const dsParticleVolume* volume, const dsMatrix44f* volumeMatrix)
{
	DS_ASSERT(particle);
	DS_ASSERT(random);
	DS_ASSERT(volume);
	DS_ASSERT(volumeMatrix);

	dsVector4f basePos;
	basePos.w = 1.0f;
	dsParticleVolume_randomPosition((dsVector3f*)&basePos, random, volume);

	dsVector4f transformedPos;
	dsMatrix44_transform(transformedPos, *volumeMatrix, basePos);
	particle->position = *(dsVector3f*)&transformedPos;
}

void dsParticle_randomSize(dsParticle* particle, dsRandom* random, const dsVector2f* widthRange,
	const dsVector2f* heightRange)
{
	DS_ASSERT(particle);
	DS_ASSERT(random);
	DS_ASSERT(widthRange);

	particle->size.x = dsRandom_nextFloatRange(random, widthRange->x, widthRange->y);
	if (!heightRange || heightRange->y < 0)
		particle->size.y = particle->size.x;
	else
		particle->size.y = dsRandom_nextFloatRange(random, heightRange->x, heightRange->y);
}

void dsParticle_createDirectionMatrix(dsMatrix33f* result, const dsVector3f* baseDirection)
{
	DS_ASSERT(result);
	DS_ASSERT(baseDirection);
	DS_ASSERT(dsEpsilonEqualf(dsVector3f_len(baseDirection), 1.0f, 1e-5f));

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

void dsParticle_randomDirection(dsVector3f* outDirection, dsRandom* random,
	const dsMatrix33f* directionMatrix, float directionSpread)
{
	DS_ASSERT(outDirection);
	DS_ASSERT(random);
	DS_ASSERT(directionMatrix);

	if (directionSpread <= 0)
	{
		*outDirection = directionMatrix->columns[2];
		return;
	}

	float theta = dsRandom_nextFloatRange(random, 0, (float)(2*M_PI));
	float phi = dsRandom_nextFloatRange(random, 0, directionSpread);
	float cosPhi = cosf(phi);
	float sinPhi = sinf(phi);
	dsVector3f direction = {{cosf(theta)*sinPhi, sinf(theta)*sinPhi, cosPhi}};
	dsMatrix33_transform(*outDirection, *directionMatrix, direction);
}

void dsParticle_randomRotation(dsParticle* particle, dsRandom* random,
	const dsVector2f* xRotationRange, const dsVector2f* yRotationRange)
{
	DS_ASSERT(particle);
	DS_ASSERT(random);
	DS_ASSERT(xRotationRange);
	DS_ASSERT(yRotationRange);

	const float wrapVal = (float)(2*M_PI);
	particle->rotation.x =
		randomWrappedFloat(random, xRotationRange->x, xRotationRange->y, wrapVal);
	particle->rotation.y =
		randomWrappedFloat(random, xRotationRange->y, xRotationRange->y, wrapVal);
}

void dsParticle_randomColor(dsParticle* particle, dsRandom* random, const dsVector2f* hueRange,
	const dsVector2f* saturationRange, const dsVector2f* valueRange)
{
	DS_ASSERT(particle);
	DS_ASSERT(random);
	DS_ASSERT(hueRange);
	DS_ASSERT(saturationRange);
	DS_ASSERT(valueRange);

	dsHSVColor color = {{randomWrappedFloat(random, hueRange->x, hueRange->y, 360),
		dsRandom_nextFloatRange(random, saturationRange->x, saturationRange->y),
		dsRandom_nextFloatRange(random, valueRange->x, valueRange->y)}};
	particle->color = dsColor_fromHSVColor(&color);
}

void dsParticle_randomIntensity(dsParticle* particle, dsRandom* random,
	const dsVector2f* intensityRange)
{
	DS_ASSERT(particle);
	DS_ASSERT(random);
	DS_ASSERT(intensityRange);

	particle->intensity = dsRandom_nextFloatRange(random, intensityRange->x, intensityRange->y);
}

void dsParticle_randomTexture(dsParticle* particle, dsRandom* random,
	const dsVector2i* textureRange)
{
	DS_ASSERT(particle);
	DS_ASSERT(random);
	DS_ASSERT(textureRange);

	particle->textureIndex = dsRandom_nextUInt32Range(random, textureRange->x, textureRange->y);
}

/*
 * Copyright 2022-2026 Aaron Barany
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

#include <DeepSea/Particle/ParticleVolume.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Geometry/AlignedBox3x.h>

#include <DeepSea/Math/Random.h>
#include <DeepSea/Math/Vector3x.h>
#include <DeepSea/Math/Trig.h>

void dsParticleVolume_randomPosition(
	dsVector3xf* result, dsRandom* random, const dsParticleVolume* volume)
{
	DS_ASSERT(result);
	DS_ASSERT(random);
	DS_ASSERT(volume);

	switch (volume->type)
	{
		case dsParticleVolumeType_Box:
		{
			dsVector3xf center, range;
			dsAlignedBox3xf_center(&center, &volume->box);
			dsAlignedBox3xf_extents(&range, &volume->box);
			dsVector3xf_scale(&range, &range, 0.5f);
			for (unsigned int i = 0; i < 3; ++i)
			{
				result->values[i] = dsRandom_nextFloatCenteredRange(
					random, center.values[i], range.values[i]);
			}
			result->w = 0.0f;
			return;
		}
		case dsParticleVolumeType_Sphere:
		{
			float theta = dsRandom_nextFloatRange(random, 0, 2*M_PIf);
			float phi = dsRandom_nextFloatRange(random, -M_PIf, M_PIf);
			float sinTheta, cosTheta, sinPhi, cosPhi;
			dsSinCosf(&sinTheta, &cosTheta, theta);
			dsSinCosf(&sinPhi, &cosPhi, phi);
			dsVector3xf offset = {{cosTheta*cosPhi, sinTheta*cosPhi, sinPhi}};

			float radius = dsRandom_nextFloatRange(random, 0.0f, volume->sphere.radius);
			dsVector3xf_scale(&offset, &offset, radius);
			dsVector3xf_add(result, &volume->sphere.center, &offset);
			return;
		}
		case dsParticleVolumeType_Cylinder:
		{
			float theta = dsRandom_nextFloatRange(random, 0, 2*M_PIf);
			float radius = dsRandom_nextFloatRange(random, 0, volume->sphere.radius);
			float sinTheta, cosTheta;
			dsSinCosf(&sinTheta, &cosTheta, theta);
			dsVector3xf offset;
			offset.x = cosTheta*radius;
			offset.y = sinTheta*radius;
			offset.z = dsRandom_nextFloatCenteredRange(random, 0.0f, volume->cylinder.height/2);
			offset.w = 0.0f;
			dsVector3xf_add(result, &volume->cylinder.center, &offset);
			return;
		}
	}

	DS_ASSERT(false);
}

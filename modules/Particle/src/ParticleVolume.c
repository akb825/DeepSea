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

#include <DeepSea/Particle/ParticleVolume.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Geometry/AlignedBox3.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Random.h>
#include <DeepSea/Math/Vector3.h>

void dsParticleVolume_randomPosition(dsVector3f* result, dsRandom* random,
	const dsParticleVolume* volume)
{
	DS_ASSERT(result);
	DS_ASSERT(random);
	DS_ASSERT(volume);

	switch (volume->type)
	{
		case dsParticleVolumeType_Box:
		{
			dsVector3f center, range;
			dsAlignedBox3_center(center, volume->box);
			dsAlignedBox3_extents(range, volume->box);
			dsVector3_scale(range, range, 0.5f);
			for (unsigned int i = 0; i < 3; ++i)
			{
				result->values[i] = dsRandom_nextFloatCenteredRange(random, center.values[i],
					range.values[i]);
			}
			return;
		}
		case dsParticleVolumeType_Sphere:
		{
			float theta = dsRandom_nextFloatRange(random, 0, (float)(2*M_PI));
			float phi = dsRandom_nextFloatRange(random, (float)-M_PI, (float)M_PI);
			float cosPhi = cosf(phi);
			dsVector3f offset;
			offset.x = cosf(theta)*cosPhi;
			offset.y = sinf(theta)*cosPhi;
			offset.z = sinf(phi);

			float radius = dsRandom_nextFloatRange(random, 0.0f, volume->sphere.radius);
			dsVector3_scale(offset, offset, radius);
			dsVector3_add(*result, volume->sphere.center, offset);
			return;
		}
		case dsParticleVolumeType_Cylinder:
		{
			float theta = dsRandom_nextFloatRange(random, 0, (float)(2*M_PI));
			float radius = dsRandom_nextFloatRange(random, 0, volume->sphere.radius);
			dsVector3f offset;
			offset.x = cosf(theta)*radius;
			offset.y = sinf(theta)*radius;
			offset.z = dsRandom_nextFloatCenteredRange(random, 0.0f, volume->cylinder.height/2);
			dsVector3_add(*result, volume->cylinder.center, offset);
			return;
		}
	}

	DS_ASSERT(false);
}

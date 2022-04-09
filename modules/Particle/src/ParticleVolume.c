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
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Random.h>
#include <DeepSea/Math/Vector3.h>

void dsParticleVolume_randomPosition(dsVector3f* result, uint32_t* seed,
	const dsParticleVolume* volume)
{
	DS_ASSERT(result);
	DS_ASSERT(seed);
	DS_ASSERT(volume);

	switch (volume->type)
	{
		case dsParticleVolumeType_Box:
			result->x = dsRandomFloat(seed, volume->box.min.x, volume->box.max.x);
			result->y = dsRandomFloat(seed, volume->box.min.y, volume->box.max.y);
			result->z = dsRandomFloat(seed, volume->box.min.z, volume->box.max.z);
			return;
		case dsParticleVolumeType_Sphere:
		{
			float theta = dsRandomFloat(seed, 0, (float)(2*M_PI));
			float phi = dsRandomFloat(seed, (float)-M_PI, (float)M_PI);
			float cosPhi = cosf(phi);
			dsVector3f offset;
			offset.x = cosf(theta)*cosPhi;
			offset.y = sinf(theta)*cosPhi;
			offset.z = sinf(phi);
			dsVector3_scale(offset, offset, volume->sphere.radius);
			dsVector3_add(*result, *result, offset);
			return;
		}
		case dsParticleVolumeType_Cylinder:
		{
			float theta = dsRandomFloat(seed, 0, (float)(2*M_PI));
			dsVector3f offset;
			offset.x = cosf(theta)*volume->cylinder.radius;
			offset.y = sinf(theta)*volume->cylinder.radius;
			offset.z = dsRandomFloat(seed, -volume->cylinder.height/2,
				volume->cylinder.height/2);
			dsVector3_add(*result, *result, offset);
			return;
		}
	}

	DS_ASSERT(false);
}

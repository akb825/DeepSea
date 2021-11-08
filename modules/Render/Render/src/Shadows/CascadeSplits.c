/*
 * Copyright 2021 Aaron Barany
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

#include <DeepSea/Render/Shadows/CascadeSplits.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Math/Core.h>

#include <float.h>

// https://developer.download.nvidia.com/SDK/10.5/opengl/src/cascaded_shadow_maps/doc/cascaded_shadow_maps.pdf

unsigned int dsComputeCascadeCount(float near, float far, float maxFirstSplitDist, float expFactor,
	unsigned int maxCascades)
{
	if (near <= 0 || near >= far || maxFirstSplitDist <= 0 || expFactor < 0 || expFactor > 1 ||
		maxCascades == 0)
	{
		errno = EINVAL;
		return 0;
	}

	if (far <= maxFirstSplitDist)
		return 1;

	// Brute force is the simplest way rather than trying to isolate N with both th exponential and
	// linear factor. maxCascades is assumed to be small, typically up to 4.
	for (unsigned int i = 1; i < maxCascades; ++i)
	{
		if (dsComputeCascadeDistance(near, far, FLT_MAX, expFactor, 0, i) <= maxFirstSplitDist)
			return i;
	}

	return maxCascades;
}

float dsComputeCascadeDistance(float near, float far, float maxFirstSplitDist, float expFactor,
	unsigned int index, unsigned int cascadeCount)
{
	if (near <= 0 || near >= far || expFactor < 0 || expFactor > 1)
	{
		errno = EINVAL;
		return 0;
	}

	if (index >= cascadeCount)
	{
		errno = EINDEX;
		return 0;
	}

	float cascadeFrac = (float)(index + 1)/(float)cascadeCount;
	float linDist = near + cascadeFrac*(far - near);
	float expDist = near*powf(far/near, cascadeFrac);
	float distance = dsLerp(linDist, expDist, expFactor);
	if (index == 0)
		distance = dsMin(distance, maxFirstSplitDist);
	return distance;
}

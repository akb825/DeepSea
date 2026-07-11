/*
 * Copyright 2017-2026 Aaron Barany
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

#include <DeepSea/VectorDraw/Gradient.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Color.h>
#include <DeepSea/Math/Core.h>

#include <string.h>

size_t dsGradient_fullAllocSize(uint32_t stopCount)
{
	size_t fullSize = sizeof(dsGradient);
	if (!dsAddAlignedArraySize(&fullSize, sizeof(dsGradientStop), stopCount, DS_ALLOC_ALIGNMENT))
		return 0;

	return fullSize;
}

dsGradient* dsGradient_create(
	dsAllocator* allocator, const dsGradientStop* stops, uint32_t stopCount)
{
	if (!allocator || !stops || stopCount == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	float lastT = -1;
	for (uint32_t i = 0; i < stopCount; ++i)
	{
		const dsGradientStop* stop = stops + i;
		if (stop->position < 0 || stop->position > 1 || stop->position <= lastT)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG, "Gradient stops must be monotonically increasing "
				"and in the range [0, 1].");
			return NULL;
		}
	}

	size_t fullSize = dsGradient_fullAllocSize(stopCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsGradient* gradient = DS_ALLOCATE_OBJECT(&bufferAlloc, dsGradient);
	DS_ASSERT(gradient);
	gradient->allocator = dsAllocator_keepPointer(allocator);
	gradient->stops = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsGradientStop, stopCount);
	DS_ASSERT(gradient->stops);
	memcpy(gradient->stops, stops, sizeof(dsGradientStop)*stopCount);
	gradient->stopCount = stopCount;

	return gradient;
}

bool dsGradient_isValid(const dsGradient* gradient)
{
	if (!gradient || !gradient->stops || gradient->stopCount == 0)
		return false;

	float lastT = -1;
	for (uint32_t i = 0; i < gradient->stopCount; ++i)
	{
		const dsGradientStop* stop = gradient->stops + i;
		if (stop->position < 0 || stop->position > 1 || stop->position <= lastT)
			return false;
	}

	return true;
}

dsColor dsGradient_evaluate(const dsGradient* gradient, float t, bool srgb)
{
	if (!gradient || !gradient->stops || gradient->stopCount == 0)
	{
		dsColor color = {{0, 0, 0, 0}};
		return color;
	}

	if (t <= gradient->stops[0].position)
		return gradient->stops[0].color;
	else if (t >= gradient->stops[gradient->stopCount - 1].position)
		return gradient->stops[gradient->stopCount - 1].color;

	for (uint32_t i = 1; i < gradient->stopCount; ++i)
	{
		const dsGradientStop* stop = gradient->stops + i;
		if (t > stop->position)
			continue;

		const dsGradientStop* prevStop = stop - 1;
		float interpT = t - prevStop->position;
		interpT /= stop->position - prevStop->position;

		dsColor color1 = prevStop->color;
		dsColor color2 = stop->color;
		if (srgb)
			return dsColor_lerpSRGB(color1, color2, interpT);
		else
			return dsColor_lerp(color1, color2, interpT);
	}

	// This should only be reached for an invalid gradient.
	DS_ASSERT(false);
	dsColor color = {{0, 0, 0, 0}};
	return color;
}

void dsGradient_destroy(dsGradient* gradient)
{
	if (!gradient || !gradient->allocator)
		return;

	dsAllocator_free(gradient->allocator, gradient);
}

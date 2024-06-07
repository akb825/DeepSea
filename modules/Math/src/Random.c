/*
 * Copyright 2016-2024 Aaron Barany
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

#include <DeepSea/Math/Random.h>

#include <DeepSea/Core/DeviceRandom.h>
#include <DeepSea/Core/Atomic.h>

#include <time.h>

#if DS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif DS_APPLE
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

static uint64_t createFallbackSeed(void)
{
	// Use a counter to ensure that if this is called at a faster rate than the timer supports
	// different results will be returned.
	static uint32_t counter = 0;
	uint64_t curCounter = DS_ATOMIC_FETCH_ADD32(&counter, 1);

	uint64_t timeSeed;

#if DS_WINDOWS

	DS_VERIFY(QueryPerformanceCounter((LARGE_INTEGER*)&timeSeed));

#elif DS_APPLE

	timeSeed = mach_absolute_time();

#else

	struct timespec tp;
	DS_VERIFY(clock_gettime(CLOCK_MONOTONIC, &tp) == 0);
	timeSeed = tp.tv_nsec;

#endif

	// Use the lower (high frequency) values from the high precision timer. Since this is typically
	// since boot, use seconds since epoch as a low frequency value in the upper bits.
	timeSeed = (timeSeed & 0xFFFFFFFF) | ((uint64_t)time(NULL) << 32);

	return dsRandom_nextSeed(&timeSeed) ^ dsRandom_nextSeed(&curCounter);
}

uint64_t dsRandom_createSeed(void)
{
	uint64_t seed;
	if (!dsDeviceRandomBytes(&seed, sizeof(uint64_t)))
		return createFallbackSeed();
	return seed;
}

void dsRandom_seed(dsRandom* random, uint64_t seed)
{
	DS_ASSERT(random);
	for (unsigned int i = 0; i < 4; ++i)
		random->state[i] = dsRandom_nextSeed(&seed);
}

void dsRandom_initialize(dsRandom* random)
{
	DS_ASSERT(random);
	// Cannot have all 0s as an initial state. This is so unlikely to happen (2^-256) that it's not
	// worth treating differently to the normal fallback of the device unable to provide random
	// numbers.
	if (!dsDeviceRandomBytes(random, sizeof(dsRandom)) ||
		(random->state[0] | random->state[1] | random->state[2] | random->state[3]) == 0)
	{
		dsRandom_seed(random, createFallbackSeed());
	}
}

uint64_t dsRandom_nextSeed(uint64_t* state);
uint64_t dsRandom_next(dsRandom* random);
bool dsRandom_nextBool(dsRandom* random);
uint32_t dsRandom_nextUInt32(dsRandom* random, uint32_t maxBound);
uint32_t dsRandom_nextUInt32Range(dsRandom* random, uint32_t minVal, uint32_t maxBound);
int32_t dsRandom_nextInt32Range(dsRandom* random, int32_t minVal, int32_t maxBound);
uint64_t dsRandom_nextUInt64(dsRandom* random, uint64_t maxBound);
uint64_t dsRandom_nextUInt64Range(dsRandom* random, uint64_t minVal, uint64_t maxBound);
int64_t dsRandom_nextInt64Range(dsRandom* random, int64_t minVal, int64_t maxBound);
float dsRandom_nextFloat(dsRandom* random);
float dsRandom_nextSignedFloat(dsRandom* random);
float dsRandom_nextFloatRange(dsRandom* random, float minVal, float maxBound);
float dsRandom_nextFloatCenteredRange(dsRandom* random, float centerValue, float range);
double dsRandom_nextDouble(dsRandom* random);
double dsRandom_nextDoubleRange(dsRandom* random, double minVal, double maxBound);
double dsRandom_nextSignedDouble(dsRandom* random);
double dsRandom_nextDoubleCenteredRange(dsRandom* random, double centerValue, double range);

/*
 * Copyright 2016-2022 Aaron Barany
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
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <math.h>

#if DS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif DS_APPLE
#include <mach/mach.h>
#include <mach/mach_time.h>
#else
#include <time.h>
#endif

inline static uint32_t nextRandomValue(uint32_t seed)
{
	uint64_t temp = seed ? seed : 1;
	temp = temp*48271 % (DS_RANDOM_MAX + 1);
	return (uint32_t)temp;
}

uint32_t dsRandomSeed(void)
{
	static uint32_t counter;
	uint32_t curCounter = DS_ATOMIC_FETCH_ADD32(&counter, 1);

	uint32_t highFrequencySeed;
	uint32_t lowFrequencySeed;

#if DS_WINDOWS

	LARGE_INTEGER value;
	QueryPerformanceCounter(&value);
	lowFrequencySeed = value.LowPart;
	highFrequencySeed = value.HighPart;

#elif DS_APPLE

	uint64_t fullTime = mach_absolute_time();
	lowFrequencySeed = (uint32_t)fullTime;
	highFrequencySeed = (uint32_t)(fullTime >> 32);

#else

	struct timespec tp;
	DS_VERIFY(clock_gettime(CLOCK_MONOTONIC, &tp) == 0);
	lowFrequencySeed = (uint32_t)tp.tv_nsec;
	highFrequencySeed = (uint32_t)tp.tv_sec;

#endif

	return nextRandomValue(curCounter) ^ nextRandomValue(lowFrequencySeed) ^
		nextRandomValue(highFrequencySeed);
}

uint32_t dsRandom(uint32_t* seed)
{
	DS_ASSERT(seed);
	return *seed = nextRandomValue(*seed);
}

double dsRandomDouble(uint32_t* seed, double minVal, double maxVal)
{
	double range = maxVal - minVal;
	double baseVal = (double)dsRandom(seed)/DS_RANDOM_MAX;
	return baseVal*range + minVal;
}

float dsRandomFloat(uint32_t* seed, float minVal, float maxVal)
{
	float range = maxVal - minVal;
	double baseVal = (double)dsRandom(seed)/DS_RANDOM_MAX;
	return (float)(baseVal*range) + minVal;
}

int dsRandomInt(uint32_t* seed, int minVal, int maxVal)
{
	int range = maxVal - minVal + 1;
	double baseVal = (double)dsRandom(seed)/(DS_RANDOM_MAX + 1);
	return (int)floor(baseVal*range) + minVal;
}

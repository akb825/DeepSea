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
#include <DeepSea/Core/Atomic.h>
#include <math.h>
#include <time.h>

#if DS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif DS_APPLE
#include <mach/mach.h>
#include <mach/mach_time.h>
#endif

inline static uint64_t splitMix64NoUpdte(uint64_t value)
{
	value += 0x9e3779b97f4a7c15ULL;
	value = (value ^ (value >> 30))*0xbf58476d1ce4e5b9ULL;
	value = (value ^ (value >> 27))*0x94d049bb133111ebULL;
	return value ^ (value >> 31);
}

inline static uint64_t splitMix64(uint64_t* state)
{
	*state += 0x9e3779b97f4a7c15ULL;
	uint64_t value = *state;
	value = (value ^ (value >> 30))*0xbf58476d1ce4e5b9ULL;
	value = (value ^ (value >> 27))*0x94d049bb133111ebULL;
	return value ^ (value >> 31);
}

static inline uint64_t rotl(uint64_t x, int k)
{
	return (x << k) | (x >> (64 - k));
}

uint64_t dsRandom_createSeed(void)
{
	// Use a counter to ensure that if this is called at a faster rate than the timer supports
	// different results will be returned.
	static uint32_t counter = 0;
	uint32_t curCounter = DS_ATOMIC_FETCH_ADD32(&counter, 1);

	uint64_t timeSeed;

#if DS_WINDOWS

	QueryPerformanceCounter((LARGE_INTEGER*)&timeSeed);

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

	return splitMix64NoUpdte(timeSeed) ^ splitMix64NoUpdte(curCounter);
}

void dsRandom_seed(dsRandom* random, uint64_t seed)
{
	DS_ASSERT(random);
	for (unsigned int i = 0; i < 4; ++i)
		random->state[i] = splitMix64(&seed);
}

uint64_t dsRandom_next(dsRandom* random)
{
	DS_ASSERT(random);
	uint64_t next = rotl(random->state[1]*5, 7)*9;
	uint64_t temp = random->state[1] << 17;

	random->state[2] ^= random->state[0];
	random->state[3] ^= random->state[1];
	random->state[1] ^= random->state[2];
	random->state[0] ^= random->state[3];

	random->state[2] ^= temp;
	random->state[3] = rotl(random->state[3], 45);

	return next;
}

bool dsRandom_nextBool(dsRandom* random);
uint32_t dsRandom_nextUInt32(dsRandom* random, uint32_t maxBound);
uint32_t dsRandom_nextUInt32Range(dsRandom* random, uint32_t minVal, uint32_t maxBound);
int32_t dsRandom_nextInt32Range(dsRandom* random, int32_t minVal, int32_t maxBound);
uint64_t dsRandom_nextUInt64(dsRandom* random, uint64_t maxBound);
uint64_t dsRandom_nextUInt64Range(dsRandom* random, uint64_t minVal, uint64_t maxBound);
int64_t dsRandom_nextInt64Range(dsRandom* random, int64_t minVal, int64_t maxBound);
float dsRandom_nextFloat(dsRandom* random);
float dsRandom_nextFloatRange(dsRandom* random, float minVal, float maxBound);
double dsRandom_nextDouble(dsRandom* random);
double dsRandom_nextDoubleRange(dsRandom* random, double minVal, double maxBound);

/*
 * Copyright 2016-2026 Aaron Barany
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

#include <DeepSea/Core/Timer.h>
#include <DeepSea/Core/Assert.h>
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

uint64_t dsTimer_currentTicks(void)
{
#if DS_WINDOWS
	LARGE_INTEGER value;
	DS_VERIFY(QueryPerformanceCounter(&value));
	return value.QuadPart;
#elif DS_APPLE
	return mach_absolute_time();
#else
	struct timespec tp;
	DS_VERIFY(clock_gettime(CLOCK_MONOTONIC, &tp) == 0);
	return tp.tv_sec*1000000000LLU + tp.tv_nsec;
#endif
}

dsTimer dsTimer_create(void)
{
	dsTimer timer;

#if DS_WINDOWS
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	timer.scale = 1.0/frequency.QuadPart;
#elif DS_APPLE
	mach_timebase_info_data_t timebaseInfo;
	DS_VERIFY(mach_timebase_info(&timebaseInfo) == KERN_SUCCESS);
	// Mach time scale is in nanoseconds.
	timer.scale = (double)timebaseInfo.numer/timebaseInfo.denom*1e-9;
#else
	timer.scale = 1e-9;
#endif

	return timer;
}

uint64_t dsTimer_convertTicks(dsTimer timer, double origScale, uint64_t ticks)
{
	// Avoid any potential loss in precision.
	if (timer.scale == origScale)
		return ticks;

	double scaleRatio = origScale/timer.scale;
	return (uint64_t)(round((double)ticks*scaleRatio));
}

double dsTimer_ticksToSeconds(dsTimer timer, int64_t ticks);

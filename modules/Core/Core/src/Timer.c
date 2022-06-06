/*
 * Copyright 2016 Aaron Barany
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

#if DS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif DS_APPLE
#include <mach/mach.h>
#include <mach/mach_time.h>
#else
#include <time.h>
#endif

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
	timer.scale = 0;
#endif

	return timer;
}

double dsTimer_time(dsTimer timer)
{
#if DS_WINDOWS

	DS_ASSERT(timer.scale > 0);
	LARGE_INTEGER value;
	QueryPerformanceCounter(&value);
	return value.QuadPart*timer.scale;

#elif DS_APPLE

	DS_ASSERT(timer.scale > 0);
	return (double)mach_absolute_time()*timer.scale;

#else

	DS_UNUSED(timer);
	struct timespec tp;
	DS_VERIFY(clock_gettime(CLOCK_MONOTONIC, &tp) == 0);
	return (double)tp.tv_sec + 1e-9*(double)tp.tv_nsec;

#endif
}

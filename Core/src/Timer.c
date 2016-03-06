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
#include <DeepSea/Core/Config.h>
#include <stdbool.h>

#if DS_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif DS_APPLE
#include <mach/mach.h>
#include <mac/mach_time.h>
#else
#include <time.h>
#endif

void dsTimer_initialize(dsTimer* timer)
{
	DS_ASSERT(timer);

#if DS_WINDOWS

	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	timer->scale = 1.0/frequency.QuadPart;

#elif DS_APPLE

	mach_timebase_info_data_t timebaseInfo;
	DEBUG_VERIFY(mach_timebase_info(&timebaseInfo) == KERN_SUCCESS);
	timer->scale = (double)timebaseInfo.number/timebaseInfo.denom;

#else
	DS_UNUSED(timer);
#endif
}

double dsTimer_getTime(dsTimer* timer)
{
	DS_ASSERT(timer);

#if DS_WINDOWS

	LARGE_INTEGER value;
	QueryPerformanceCounter(&value);
	return value.QuadPart*timer->scale;

#elif DS_APPLE

	return mach_absolute_time()*timer->scale;

#else

	DS_UNUSED(timer);
	struct timespec tp;
	DS_VERIFY(clock_gettime(CLOCK_MONOTONIC, &tp) == 0);
	return tp.tv_sec + 1e-9*tp.tv_nsec;

#endif
}

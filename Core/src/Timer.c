#include <DeepSea/Core/Timer.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Config.h>
#include <stdbool.h>

#if DS_WINDOWS
#include WIN32_LEAN_AND_MEAN
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

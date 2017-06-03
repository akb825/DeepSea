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

#include <DeepSea/Core/Thread/ConditionVariable.h>

#include "MutexImpl.h"
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Memory.h>
#include <DeepSea/Core/Error.h>

#if !DS_WINDOWS
#include <sys/time.h>
#endif

struct dsConditionVariable
{
#if DS_WINDOWS
	CONDITION_VARIABLE condition;
#else
	pthread_cond_t condition;
#endif
	const char* name;
	dsAllocator* allocator;
	bool shouldFree;
};

size_t dsConditionVariable_sizeof(void)
{
	return sizeof(dsConditionVariable);
}

size_t dsConditionVariable_fullAllocSize(void)
{
	return DS_ALIGNED_SIZE(sizeof(dsConditionVariable));
}

dsConditionVariable* dsConditionVariable_create(dsAllocator* allocator, const char* name)
{
	dsConditionVariable* condition;
	if (allocator)
	{
		condition = (dsConditionVariable*)dsAllocator_alloc(allocator,
			sizeof(dsConditionVariable));
	}
	else
		condition = (dsConditionVariable*)malloc(sizeof(dsConditionVariable));

	if (!condition)
		return NULL;

#if DS_WINDOWS
	InitializeConditionVariable(&condition->condition);
#else

	if (pthread_cond_init(&condition->condition, NULL) != 0)
	{
		if (allocator)
			dsAllocator_free(allocator, condition);
		else
			free(condition);
		return NULL;
	}

#endif

	condition->name = name ? name : "Condition";
	condition->allocator = dsAllocator_keepPointer(allocator);
	condition->shouldFree = !allocator || allocator->freeFunc;
	return condition;
}

dsConditionVariableResult dsConditionVariable_wait(dsConditionVariable* condition,
	dsMutex* mutex)
{
	if (!condition || !mutex)
	{
		errno = EINVAL;
		return dsConditionVariableResult_Error;
	}

	DS_PROFILE_LOCK_END();
	DS_PROFILE_WAIT_START(condition->name);

#if DS_WINDOWS
	bool retVal = SleepConditionVariableCS(&condition->condition, &mutex->mutex, INFINITE);
	if (!retVal)
		errno = EINVAL;
#else
	int errorCode = pthread_cond_wait(&condition->condition, &mutex->mutex);
	if (errorCode != 0)
		errno = errorCode;
	bool retVal = errorCode == 0;
#endif

	DS_PROFILE_WAIT_END();
	DS_PROFILE_LOCK_START(mutex->name);
	return retVal ? dsConditionVariableResult_Success : dsConditionVariableResult_Error;
}

dsConditionVariableResult dsConditionVariable_timedWait(
	dsConditionVariable* condition, dsMutex* mutex, unsigned int milliseconds)
{
	if (!condition || !mutex)
	{
		errno = EINVAL;
		return dsConditionVariableResult_Error;
	}

	DS_PROFILE_LOCK_END();
	DS_PROFILE_WAIT_START(condition->name);

	dsConditionVariableResult result;
#if DS_WINDOWS

	if (SleepConditionVariableCS(&condition->condition, &mutex->mutex, milliseconds))
		result = dsConditionVariableResult_Success;
	else if (GetLastError() == ERROR_TIMEOUT)
		result = dsConditionVariableResult_Timeout;
	else
	{
		errno = EINVAL;
		result = dsConditionVariableResult_Error;
	}

#else

	struct timespec time;
	time.tv_sec = milliseconds/1000;
	time.tv_nsec = (milliseconds % 1000)*1000000;

	struct timeval curTime;
	DS_VERIFY(gettimeofday(&curTime, NULL) == 0);
	time.tv_nsec += curTime.tv_usec*1000;
	time.tv_sec += curTime.tv_sec + time.tv_nsec/1000000000;
	time.tv_nsec = time.tv_nsec % 1000000000;

	int retVal = pthread_cond_timedwait(&condition->condition, &mutex->mutex, &time);
	if (retVal == ETIMEDOUT)
		result = dsConditionVariableResult_Timeout;
	else if (retVal == 0)
		result = dsConditionVariableResult_Success;
	else
	{
		result = dsConditionVariableResult_Error;
		errno = retVal;
	}

#endif

	DS_PROFILE_WAIT_END();
	DS_PROFILE_LOCK_START(mutex->name);
	return result;
}

bool dsConditionVariable_notifyOne(dsConditionVariable* condition)
{
	if (!condition)
	{
		errno = EINVAL;
		return false;
	}

#if DS_WINDOWS
	WakeConditionVariable(&condition->condition);
	return true;
#else
	int errorCode = pthread_cond_signal(&condition->condition);
	if (errorCode != 0)
		errno = errorCode;
	return errorCode == 0;
#endif
}

bool dsConditionVariable_notifyAll(dsConditionVariable* condition)
{
	if (!condition)
	{
		errno = EINVAL;
		return false;
	}

#if DS_WINDOWS
	WakeAllConditionVariable(&condition->condition);
	return true;
#else
	int errorCode = pthread_cond_broadcast(&condition->condition);
	if (errorCode != 0)
		errno = errorCode;
	return errorCode == 0;
#endif
}

void dsConditionVariable_destroy(dsConditionVariable* condition)
{
	if (!condition)
		return;

#if !DS_WINDOWS
	DS_VERIFY(pthread_cond_destroy(&condition->condition) == 0);
#endif

	if (!condition->shouldFree)
		return;

	if (condition->allocator)
		DS_VERIFY(dsAllocator_free(condition->allocator, condition));
	else
		free(condition);
}

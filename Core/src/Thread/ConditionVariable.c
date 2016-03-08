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
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include "MutexImpl.h"
#include <errno.h>

struct dsConditionVariable
{
#if DS_WINDOWS
	CONDITION_VARIABLE condition;
#else
	pthread_cond_t condition;
#endif
	dsAllocator* allocator;
};

unsigned int dsConditionVariable_sizeof()
{
	return sizeof(dsConditionVariable);
}

dsConditionVariable* dsConditionVariable_create(dsAllocator* allocator)
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

	condition->allocator = allocator;
	return condition;
}

dsConditionVariableResult dsConditionVariable_wait(dsConditionVariable* condition,
	dsMutex* mutex)
{
	if (!condition || !mutex)
		return dsConditionVariableResult_Error;

#if DS_WINDOWS
	BOOL retVal = SleepConditionVariableCS(&condition->condition, &mutex->mutex, INFINITE);
	return retVal ? dsConditionVariableResult_Success : dsConditionVariableResult_Error;
#else
	int retVal = pthread_cond_wait(&condition->condition, &mutex->mutex);
	return retVal == 0 ? dsConditionVariableResult_Success : dsConditionVariableResult_Error;
#endif
}

dsConditionVariableResult dsConditionVariable_timedWait(
	dsConditionVariable* condition, dsMutex* mutex, unsigned int milliseconds)
{
	if (!condition || !mutex)
		return dsConditionVariableResult_Error;

#if DS_WINDOWS

	BOOL retVal = SleepConditionVariableCS(&condition->condition, &mutex->mutex, milliseconds);
	if (retVal == ERROR_TIMEOUT)
		return dsConditionVariableResult_Timeout;
	else if (retVal)
		return dsConditionVariableResult_Success;
	else
		return dsConditionVariableResult_Error;

#else

	struct timespec time;
	time.tv_sec = milliseconds/1000;
	time.tv_nsec = (milliseconds % 1000)*1000000;
	int retVal = pthread_cond_timedwait(&condition->condition, &mutex->mutex, &time);
	if (retVal == ETIMEDOUT)
		return dsConditionVariableResult_Timeout;
	else if (retVal == 0)
		return dsConditionVariableResult_Success;
	else
		return dsConditionVariableResult_Error;

#endif
}

bool dsConditionVariable_notifyOne(dsConditionVariable* condition)
{
	if (!condition)
		return false;

#if DS_WINDOWS
	WakeConditionVariable(&condition->condition);
	return true;
#else
	return pthread_cond_signal(&condition->condition) == 0;
#endif
}

bool dsConditionVariable_notifyAll(dsConditionVariable* condition)
{
	if (!condition)
		return false;

#if DS_WINDOWS
	WakeAllConditionVariable(&condition->condition);
	return true;
#else
	return pthread_cond_broadcast(&condition->condition) == 0;
#endif
}

void dsConditionVariable_destroy(dsConditionVariable* condition)
{
	if (!condition)
		return;

	if (condition->allocator)
		DS_VERIFY(dsAllocator_free(condition->allocator, condition));
	else
		free(condition);
}

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

#include <DeepSea/Core/Thread/Mutex.h>

#include "MutexImpl.h"
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Memory.h>
#include <DeepSea/Core/Error.h>
#include <stdlib.h>

size_t dsMutex_sizeof(void)
{
	return sizeof(dsMutex);
}

size_t dsMutex_fullAllocSize(void)
{
	return DS_ALIGNED_SIZE(sizeof(dsMutex));
}

dsMutex* dsMutex_create(dsAllocator* allocator, const char* name)
{
	dsMutex* mutex;
	if (allocator)
		mutex = DS_ALLOCATE_OBJECT(allocator, dsMutex);
	else
		mutex = (dsMutex*)malloc(sizeof(dsMutex));

	if (!mutex)
		return NULL;

#if DS_WINDOWS
	InitializeCriticalSection(&mutex->mutex);
#else

	int errorCode = pthread_mutex_init(&mutex->mutex, NULL);
	if (errorCode != 0)
	{
		if (allocator)
			dsAllocator_free(allocator, mutex);
		else
			free(mutex);
		errno = errorCode;
		return NULL;
	}

#endif

	mutex->name = name ? name : "Mutex";
	mutex->allocator = dsAllocator_keepPointer(allocator);
	mutex->shouldFree = !allocator || allocator->freeFunc;
	return mutex;
}

bool dsMutex_tryLock(dsMutex* mutex)
{
	if (!mutex)
	{
		errno = EINVAL;
		return false;
	}

	bool retVal;
#if DS_WINDOWS
	retVal = TryEnterCriticalSection(&mutex->mutex);
	if (!retVal)
		errno = EBUSY;
#else
	int errorCode = pthread_mutex_trylock(&mutex->mutex);
	if (errorCode != 0)
		errno = errorCode;
	retVal = errorCode == 0;
#endif

	if (retVal)
	{
		DS_PROFILE_DYNAMIC_LOCK_START(mutex->name);
	}
	return retVal;
}

bool dsMutex_lock(dsMutex* mutex)
{
	if (!mutex)
	{
		errno = EINVAL;
		return false;
	}

	DS_PROFILE_DYNAMIC_LOCK_START(mutex->name);
	DS_PROFILE_DYNAMIC_WAIT_START(mutex->name);

	bool retVal;
#if DS_WINDOWS
	EnterCriticalSection(&mutex->mutex);
	retVal = true;
#else
	int errorCode = pthread_mutex_lock(&mutex->mutex);
	if (errorCode != 0)
		errno = errorCode;
	retVal = errorCode == 0;
#endif

	DS_PROFILE_WAIT_END();
	if (!retVal)
	{
		DS_PROFILE_LOCK_END();
	}
	return retVal;
}

bool dsMutex_unlock(dsMutex* mutex)
{
	if (!mutex)
	{
		errno = EINVAL;
		return false;
	}

	bool retVal;
#if DS_WINDOWS
	LeaveCriticalSection(&mutex->mutex);
	retVal = true;
#else
	int errorCode = pthread_mutex_unlock(&mutex->mutex);
	if (errorCode != 0)
		errno = errorCode;
	retVal = errorCode == 0;
#endif

	if (retVal)
	{
		DS_PROFILE_LOCK_END();
	}
	return retVal;
}

void dsMutex_destroy(dsMutex* mutex)
{
	if (!mutex)
		return;

#if DS_WINDOWS
	DeleteCriticalSection(&mutex->mutex);
#else
	DS_VERIFY(pthread_mutex_destroy(&mutex->mutex) == 0);
#endif

	if (!mutex->shouldFree)
		return;

	if (mutex->allocator)
		DS_VERIFY(dsAllocator_free(mutex->allocator, mutex));
	else
		free(mutex);
}

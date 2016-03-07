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

#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Memory/GenericAllocator.h>
#include "MutexImpl.h"
#include <stdlib.h>

dsMutex* dsMutex_create(dsGenericAllocator* allocator)
{
	dsMutex* mutex;
	if (allocator)
		mutex = (dsMutex*)dsGenericAllocator_alloc(allocator, sizeof(dsMutex));
	else
		mutex = (dsMutex*)malloc(sizeof(dsMutex));

	if (!mutex)
		return NULL;

#if DS_WINDOWS
	InitializeCriticalSection(&mutex->mutex);
#else

	if (pthread_mutex_init(&mutex->mutex, NULL) != 0)
	{
		if (allocator)
			dsGenericAllocator_free(allocator, mutex);
		else
			free(mutex);
		return NULL;
	}

#endif

	mutex->allocator = allocator;
	return mutex;
}

bool dsMutex_tryLock(dsMutex* mutex)
{
	if (!mutex)
		return false;

#if DS_WINDOWS
	return TryEnterCriticalSection(&mutex->mutex);
#else
	return pthread_mutex_trylock(&mutex->mutex) == 0;
#endif
}

bool dsMutex_lock(dsMutex* mutex)
{
	if (!mutex)
		return false;

#if DS_WINDOWS
	EnterCriticalSection(&mutex->mutex);
	return true;
#else
	return pthread_mutex_lock(&mutex->mutex) == 0;
#endif
}

bool dsMutex_unlock(dsMutex* mutex)
{
	if (!mutex)
		return false;

#if DS_WINDOWS
	LeaveCriticalSection(&mutex->mutex);
	return true;
#else
	return pthread_mutex_unlock(&mutex->mutex) == 0;
#endif
}

void dsMutex_destroy(dsMutex* mutex)
{
	if (!mutex)
		return;

	if (mutex->allocator)
		DS_VERIFY(dsGenericAllocator_free(mutex->allocator, mutex));
	else
		free(mutex);
}

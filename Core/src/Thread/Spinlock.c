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

#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Error.h>

#if DS_WINDOWS
#include <DeepSea/Core/Atomic.h>
#else
#include <pthread.h>
#include <string.h>
#endif

bool dsSpinlock_initialize(dsSpinlock* spinlock)
{
	if (!spinlock)
	{
		errno = EINVAL;
		return false;
	}

#if DS_WINDOWS

	uint32_t counter = 0;
	DS_ATOMIC_STORE32(&spinlock->counter, &counter);
	return true;

#else
	int errorCode = pthread_spin_init(&spinlock->spinlock, PTHREAD_PROCESS_PRIVATE);
	if (errorCode != 0)
		errno = errorCode;
	return errorCode == 0;
#endif
}

bool dsSpinlock_tryLock(dsSpinlock* spinlock)
{
	if (!spinlock)
	{
		errno = EINVAL;
		return false;
	}

#if DS_WINDOWS

	uint32_t expected = 0;
	uint32_t value = 1;
	bool retVal = DS_ATOMIC_COMPARE_EXCHANGE32(&spinlock->counter, &expected, &value, false);
	if (!retVal)
		errno = EBUSY;
	return retVal;

#else
	int errorCode = pthread_spin_trylock(&spinlock->spinlock);
	if (errorCode != 0)
		errno = errorCode;
	return errorCode == 0;
#endif
}

bool dsSpinlock_lock(dsSpinlock* spinlock)
{
	if (!spinlock)
	{
		errno = EINVAL;
		return false;
	}

#if DS_WINDOWS

	uint32_t expected;
	uint32_t value = 1;
	do
	{
		expected = 0;
	} while (!DS_ATOMIC_COMPARE_EXCHANGE32(&spinlock->counter, &expected, &value, true));

	return true;

#else
	int errorCode = pthread_spin_lock(&spinlock->spinlock);
	if (errorCode != 0)
		errno = errorCode;
	return errorCode == 0;
#endif
}

bool dsSpinlock_unlock(dsSpinlock* spinlock)
{
	if (!spinlock)
	{
		errno = EINVAL;
		return false;
	}

#if DS_WINDOWS

	uint32_t expected = 1;
	uint32_t value = 0;
	return DS_ATOMIC_COMPARE_EXCHANGE32(&spinlock->counter, &expected, &value, false);

#else
	int errorCode = pthread_spin_unlock(&spinlock->spinlock);
	if (errorCode != 0)
		errno = errorCode;
	return errorCode == 0;
#endif
}

void dsSpinlock_destroy(dsSpinlock* spinlock)
{
	if (!spinlock)
		return;

#if DS_WINDOWS
	uint32_t counter = 0;
	DS_ATOMIC_STORE32(&spinlock->counter, &counter);
#else
	pthread_spin_destroy(&spinlock->spinlock);
#endif
}

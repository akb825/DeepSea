/*
 * Copyright 2024 Aaron Barany
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

#include <DeepSea/Core/Thread/ReadWriteSpinlock.h>

#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>

bool dsReadWriteSpinlock_initialize(dsReadWriteSpinlock* lock)
{
	if (!lock)
	{
		errno = EINVAL;
		return false;
	}

	DS_VERIFY(dsSpinlock_initialize(&lock->lock));
	lock->readCount = 0;
	return true;
}

bool dsReadWriteSpinlock_lockRead(dsReadWriteSpinlock* lock)
{
	if (!lock)
	{
		errno = EINVAL;
		return false;
	}

	DS_VERIFY(dsSpinlock_lock(&lock->lock));
	DS_ATOMIC_FETCH_ADD32(&lock->readCount, 1);
	DS_VERIFY(dsSpinlock_unlock(&lock->lock));
	return true;
}

bool dsReadWriteSpinlock_unlockRead(dsReadWriteSpinlock* lock)
{
	if (!lock)
	{
		errno = EINVAL;
		return false;
	}

	// Double check that it was previously locked for reading.
	uint32_t curCount, newCount;
	DS_ATOMIC_LOAD32(&lock->readCount, &curCount);
	do
	{
		if (curCount == 0)
		{
			errno = EPERM;
			return false;
		}

		newCount = curCount - 1;
	}  while (!DS_ATOMIC_COMPARE_EXCHANGE32(&lock->readCount, &curCount, &newCount, true));
	return true;
}

bool dsReadWriteSpinlock_lockWrite(dsReadWriteSpinlock* lock)
{
	if (!lock)
	{
		errno = EINVAL;
		return false;
	}

	DS_VERIFY(dsSpinlock_lock(&lock->lock));

	// Wait until not reading.
	do
	{
		uint32_t readCount;
		DS_ATOMIC_LOAD32(&lock->readCount, &readCount);
		if (readCount == 0)
			break;

		// Let other thread that is reading continue. Keep spinlock locked since the read unlock
		// uses atomics.
		dsThread_yield();
	} while (true);
	return true;
}

bool dsReadWriteSpinlock_unlockWrite(dsReadWriteSpinlock* lock)
{
	if (!lock)
	{
		errno = EINVAL;
		return false;
	}

	return dsSpinlock_unlock(&lock->lock);
}

void dsReadWriteSpinlock_shutdown(dsReadWriteSpinlock* lock)
{
	if (lock)
		dsSpinlock_shutdown(&lock->lock);
}

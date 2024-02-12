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

#include <DeepSea/Core/Thread/ReadWriteLock.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Thread/ConditionVariable.h>
#include <DeepSea/Core/Thread/Mutex.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Profile.h>

struct dsReadWriteLock
{
	dsAllocator* allocator;
	const char* readName;
	const char* writeName;
	dsMutex* stateMutex;
	dsConditionVariable* condition;
	uint32_t readCount;
	uint32_t writeCount;
};

size_t dsReadWriteLock_sizeof(void)
{
	return sizeof(dsReadWriteLock);
}

size_t dsReadWriteLock_fullAllocSize(void)
{
	return DS_ALIGNED_SIZE(sizeof(dsReadWriteLock)) + dsMutex_fullAllocSize() +
		dsConditionVariable_fullAllocSize();
}

dsReadWriteLock* dsReadWriteLock_create(
	dsAllocator* allocator, const char* readName, const char* writeName)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = dsReadWriteLock_fullAllocSize();
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsReadWriteLock* lock = DS_ALLOCATE_OBJECT(&bufferAlloc, dsReadWriteLock);
	DS_ASSERT(lock);

	lock->allocator = dsAllocator_keepPointer(allocator);
	lock->stateMutex = dsMutex_create((dsAllocator*)&bufferAlloc, "Read/Write State");
	lock->condition =
		dsConditionVariable_create((dsAllocator*)&bufferAlloc, "Read/Write Condition");
	if (!lock->stateMutex || !lock->condition)
	{
		dsMutex_destroy(lock->stateMutex);
		dsConditionVariable_destroy(lock->condition);
		if (lock->allocator)
			DS_VERIFY(dsAllocator_free(lock->allocator, lock));
		return NULL;
	}

	lock->readName = readName ? readName : "Read Lock";
	lock->writeName = writeName ? writeName : "Write Lock";
	lock->readCount = 0;
	lock->writeCount = 0;
	return lock;
}

bool dsReadWriteLock_tryLockRead(dsReadWriteLock* lock)
{
	if (!lock)
	{
		errno = EINVAL;
		return false;
	}

	if (!dsMutex_lock(lock->stateMutex))
		return false;

	if (lock->writeCount > 0)
	{
		DS_VERIFY(dsMutex_unlock(lock->stateMutex));
		errno = EBUSY;
		return false;
	}

	++lock->readCount;
	DS_VERIFY(dsMutex_unlock(lock->stateMutex));
	DS_PROFILE_LOCK_START(lock->readName);
	return true;
}

bool dsReadWriteLock_lockRead(dsReadWriteLock* lock)
{
	if (!lock)
	{
		errno = EINVAL;
		return false;
	}

	DS_PROFILE_LOCK_START(lock->readName);
	if (!dsMutex_lock(lock->stateMutex))
	{
		DS_PROFILE_LOCK_END();
		return false;
	}

	while (lock->writeCount > 0)
		dsConditionVariable_wait(lock->condition, lock->stateMutex);

	++lock->readCount;
	DS_VERIFY(dsMutex_unlock(lock->stateMutex));
	return true;
}

bool dsReadWriteLock_unlockRead(dsReadWriteLock* lock)
{
	if (!lock)
	{
		errno = EINVAL;
		return false;
	}

	if (!dsMutex_lock(lock->stateMutex))
		return false;

	if (lock->readCount == 0)
	{
		DS_VERIFY(dsMutex_unlock(lock->stateMutex));
		errno = EPERM;
		return false;
	}

	if (--lock->readCount == 0)
		dsConditionVariable_notifyAll(lock->condition);
	DS_VERIFY(dsMutex_unlock(lock->stateMutex));
	DS_PROFILE_LOCK_END();
	return true;
}

bool dsReadWriteLock_tryLockWrite(dsReadWriteLock* lock)
{
	if (!lock)
	{
		errno = EINVAL;
		return false;
	}

	if (!dsMutex_lock(lock->stateMutex))
		return false;

	if (lock->readCount > 0 || lock->writeCount > 0)
	{
		DS_VERIFY(dsMutex_unlock(lock->stateMutex));
		errno = EBUSY;
		return false;
	}

	++lock->writeCount;
	DS_VERIFY(dsMutex_unlock(lock->stateMutex));
	DS_PROFILE_LOCK_START(lock->writeName);
	return true;
}

bool dsReadWriteLock_lockWrite(dsReadWriteLock* lock)
{
	if (!lock)
	{
		errno = EINVAL;
		return false;
	}

	DS_PROFILE_LOCK_START(lock->writeName);
	if (!dsMutex_lock(lock->stateMutex))
	{
		DS_PROFILE_LOCK_END();
		return false;
	}

	while (lock->readCount > 0 || lock->writeCount > 0)
		dsConditionVariable_wait(lock->condition, lock->stateMutex);

	++lock->writeCount;
	DS_VERIFY(dsMutex_unlock(lock->stateMutex));
	return true;
}

bool dsReadWriteLock_unlockWrite(dsReadWriteLock* lock)
{
	if (!lock)
	{
		errno = EINVAL;
		return false;
	}

	if (!dsMutex_lock(lock->stateMutex))
		return false;

	if (lock->writeCount == 0)
	{
		DS_VERIFY(dsMutex_unlock(lock->stateMutex));
		errno = EPERM;
		return false;
	}

	if (--lock->writeCount == 0)
		dsConditionVariable_notifyAll(lock->condition);
	DS_VERIFY(dsMutex_unlock(lock->stateMutex));
	DS_PROFILE_LOCK_END();
	return true;
}

void dsReadWriteLock_destroy(dsReadWriteLock* lock)
{
	if (!lock)
		return;

	dsMutex_destroy(lock->stateMutex);
	dsConditionVariable_destroy(lock->condition);
	if (lock->allocator)
		DS_VERIFY(dsAllocator_free(lock->allocator, lock));
}

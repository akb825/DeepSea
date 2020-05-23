/*
 * Copyright 2018 Aaron Barany
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

#include <DeepSea/Core/Memory/Lifetime.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>

struct dsLifetime
{
	dsAllocator* allocator;
	void* object;
	uint32_t refCount;
	uint32_t acquireCount;
	dsSpinlock lock;
};

size_t dsLifetime_sizeof(void)
{
	return sizeof(dsLifetime);
}

size_t dsLifetime_fullAllocSize(void)
{
	return DS_ALIGNED_SIZE(sizeof(dsLifetime));
}

dsLifetime* dsLifetime_create(dsAllocator* allocator, void* object)
{
	if (!allocator || !object)
	{
		errno = EINVAL;
		return NULL;
	}

	dsLifetime* lifetime = DS_ALLOCATE_OBJECT(allocator, dsLifetime);
	if (!lifetime)
		return NULL;

	lifetime->allocator = dsAllocator_keepPointer(allocator);
	lifetime->object = object;
	lifetime->refCount = 1;
	lifetime->acquireCount = 0;
	DS_VERIFY(dsSpinlock_initialize(&lifetime->lock));

	return lifetime;
}

dsLifetime* dsLifetime_addRef(dsLifetime* lifetime)
{
	if (!lifetime)
		return NULL;

	DS_ATOMIC_FETCH_ADD32(&lifetime->refCount, 1);
	return lifetime;
}

void dsLifetime_freeRef(dsLifetime* lifetime)
{
	if (!lifetime)
		return;

	uint32_t prevRef = DS_ATOMIC_FETCH_ADD32(&lifetime->refCount, -1);
	DS_ASSERT(prevRef > 0);
	if (prevRef > 1)
		return;

	DS_ASSERT(!lifetime->object);
	dsSpinlock_shutdown(&lifetime->lock);
	if (lifetime->allocator)
		DS_VERIFY(dsAllocator_free(lifetime->allocator, lifetime));
}

void* dsLifetime_acquire(dsLifetime* lifetime)
{
	if (!lifetime)
		return NULL;

	DS_VERIFY(dsSpinlock_lock(&lifetime->lock));
	void* object = lifetime->object;
	if (object)
		++lifetime->acquireCount;
	DS_VERIFY(dsSpinlock_unlock(&lifetime->lock));

	return object;
}

void dsLifetime_release(dsLifetime* lifetime)
{
	if (!lifetime)
		return;

	DS_VERIFY(dsSpinlock_lock(&lifetime->lock));
	DS_ASSERT(lifetime->acquireCount > 0);
	--lifetime->acquireCount;
	DS_VERIFY(dsSpinlock_unlock(&lifetime->lock));
}

void* dsLifetime_getObject(dsLifetime* lifetime)
{
	if (!lifetime)
		return NULL;

	DS_VERIFY(dsSpinlock_lock(&lifetime->lock));
	void* object = lifetime->object;
	DS_VERIFY(dsSpinlock_unlock(&lifetime->lock));

	return object;
}

void dsLifetime_destroy(dsLifetime* lifetime)
{
	if (!lifetime)
		return;

	do
	{
		DS_VERIFY(dsSpinlock_lock(&lifetime->lock));
		if (lifetime->acquireCount == 0)
		{
			lifetime->object = NULL;
			DS_VERIFY(dsSpinlock_unlock(&lifetime->lock));
			dsLifetime_freeRef(lifetime);
			return;
		}

		DS_VERIFY(dsSpinlock_unlock(&lifetime->lock));
		dsThread_yield();

	} while(true);
}

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
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>

bool dsSpinlock_initialize(dsSpinlock* spinlock)
{
	if (!spinlock)
		return false;

	uint32_t counter = 0;
	DS_ATOMIC_STORE32(&spinlock->counter, &counter);
	return true;
}

bool dsSpinlock_tryLock(dsSpinlock* spinlock)
{
	DS_ASSERT(spinlock);
	uint32_t expected = 0;
	uint32_t value = 1;
	return DS_ATOMIC_COMPARE_EXCHANGE32(&spinlock->counter, &expected, &value, false);
}

void dsSpinlock_lock(dsSpinlock* spinlock)
{
	DS_ASSERT(spinlock);

	uint32_t expected;
	uint32_t value = 1;
	do
	{
		expected = 0;
	} while (!DS_ATOMIC_COMPARE_EXCHANGE32(&spinlock->counter, &expected, &value, true));
}

void dsSpinlock_unlock(dsSpinlock* spinlock)
{
	DS_ASSERT(spinlock);
	uint32_t expected = 1;
	uint32_t value = 0;
	DS_VERIFY(DS_ATOMIC_COMPARE_EXCHANGE32(&spinlock->counter, &expected, &value, false));
}

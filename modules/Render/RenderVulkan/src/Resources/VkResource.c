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

#include "Resources/VkResource.h"
#include "VkRendererInternal.h"
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>

void dsVkResource_initialize(dsVkResource* resource)
{
	DS_VERIFY(dsSpinlock_initialize(&resource->lock));
	resource->commandBufferCount = 0;
	resource->lastUsedSubmit = DS_NOT_SUBMITTED;
}

bool dsVkResource_isInUse(dsVkResource* resource, uint64_t finishedSubmitCount)
{
	uint32_t commandBufferCount;
	DS_ATOMIC_LOAD32(&resource->commandBufferCount, &commandBufferCount);
	DS_VERIFY(dsSpinlock_lock(&resource->lock));
	uint64_t lastUsedSubmit = resource->lastUsedSubmit;
	DS_VERIFY(dsSpinlock_unlock(&resource->lock));
	return commandBufferCount > 0 ||
		(lastUsedSubmit != DS_NOT_SUBMITTED && lastUsedSubmit > finishedSubmitCount);
}

void dsVkResource_waitUntilNotInUse(dsVkResource* resource, dsRenderer* renderer)
{
	DS_VERIFY(dsSpinlock_lock(&resource->lock));
	uint64_t lastUsedSubmit = resource->lastUsedSubmit;
	DS_VERIFY(dsSpinlock_unlock(&resource->lock));

	if (lastUsedSubmit == DS_NOT_SUBMITTED)
		return;

	dsVkRenderer_waitForSubmit(renderer, lastUsedSubmit, DS_DEFAULT_WAIT_TIMEOUT);
}

void dsVkResource_shutdown(dsVkResource* resource)
{
	dsSpinlock_shutdown(&resource->lock);
}

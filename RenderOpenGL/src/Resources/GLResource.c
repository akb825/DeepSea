/*
 * Copyright 2017 Aaron Barany
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

#include "Resources/GLResource.h"
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>

void dsGLResource_initialize(dsGLResource* resource)
{
	DS_VERIFY(dsSpinlock_initialize(&resource->lock));
	resource->internalRef = 0;
	resource->defferDestroy = false;
}

void dsGLResource_addRef(dsGLResource* resource)
{
	DS_ATOMIC_FETCH_ADD32(&resource->internalRef, 1);
}

bool dsGLResource_freeRef(dsGLResource* resource)
{
	DS_VERIFY(dsSpinlock_lock(&resource->lock));
	if (DS_ATOMIC_FETCH_ADD32(&resource->internalRef, -1) != 1)
	{
		DS_VERIFY(dsSpinlock_unlock(&resource->lock));
		return false;
	}

	bool deffer = resource->defferDestroy;
	DS_VERIFY(dsSpinlock_unlock(&resource->lock));
	if (deffer)
		dsSpinlock_destroy(&resource->lock);
	return deffer;
}

bool dsGLResource_destroy(dsGLResource* resource)
{
	DS_VERIFY(dsSpinlock_lock(&resource->lock));
	uint32_t internalRef;
	DS_ATOMIC_LOAD32(&resource->internalRef, &internalRef);
	if (internalRef > 0)
	{
		resource->defferDestroy = true;
		DS_VERIFY(dsSpinlock_unlock(&resource->lock));
		return false;
	}

	DS_VERIFY(dsSpinlock_unlock(&resource->lock));
	dsSpinlock_destroy(&resource->lock);
	return true;
}

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

#include "Resources/GLGfxFence.h"

#include "AnyGL/AnyGL.h"
#include "AnyGL/gl.h"
#include "Resources/GLGfxBuffer.h"
#include "GLCommandBuffer.h"
#include "GLHelpers.h"
#include "GLRendererInternal.h"
#include "Types.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

dsGfxFence* dsGLGfxFence_create(dsResourceManager* resourceManager, dsAllocator* allocator)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);

	dsGLGfxFence* fence = (dsGLGfxFence*)dsAllocator_alloc(allocator, sizeof(dsGLGfxFence));
	if (!fence)
		return NULL;

	dsGfxFence* baseFence = (dsGfxFence*)fence;
	baseFence->resourceManager = resourceManager;
	baseFence->allocator = dsAllocator_keepPointer(allocator);

	DS_VERIFY(dsSpinlock_initialize(&fence->lock));
	fence->sync = NULL;

	return baseFence;
}

bool dsGLGfxFence_set(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsGfxFence** fences, uint32_t fenceCount, bool bufferReadback)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(fences);

	// Check if any fence is currently set.
	for (uint32_t i = 0; i < fenceCount; ++i)
	{
		dsGLGfxFence* glFence = (dsGLGfxFence*)fences[i];
		DS_VERIFY(dsSpinlock_lock(&glFence->lock));
		bool set = glFence->sync != NULL;
		DS_VERIFY(dsSpinlock_unlock(&glFence->lock));

		if (set)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
				"Attempting to set a fence before it's been reset.");
			return false;
		}
	}

	dsGLFenceSyncRef* sync = dsGLRenderer_createSyncRef(resourceManager->renderer);
	if (!sync)
		return false;

	// Add a reference for each fence.
	sync->refCount += fenceCount;

	// Set the sync reference on each fence.
	for (uint32_t i = 0; i < fenceCount; ++i)
	{
		dsGLGfxFence* glFence = (dsGLGfxFence*)fences[i];
		DS_VERIFY(dsSpinlock_lock(&glFence->lock));
		bool set = glFence->sync != NULL;
		if (!set)
			glFence->sync = sync;
		DS_VERIFY(dsSpinlock_unlock(&glFence->lock));

		// Another thread may have set the fence.
		if (set)
		{
			// Remove from any fence previously set.
			for (size_t j = 0; j < i; ++j)
			{
				dsGLGfxFence* otherFence = (dsGLGfxFence*)fences[j];
				DS_VERIFY(dsSpinlock_lock(&otherFence->lock));
				if (otherFence->sync == sync)
				{
					dsGLFenceSyncRef_freeRef(sync);
					otherFence->sync = NULL;
				}
				DS_VERIFY(dsSpinlock_lock(&otherFence->lock));
			}

			// Should only have ref counts for any remaining fences, and nothing else should have a
			// pointer to this sync reference.
			DS_ASSERT(sync->refCount == fenceCount - i + 1);
			sync->refCount = 1;
			dsGLFenceSyncRef_freeRef(sync);

			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG,
				"Attempting to set a fence before it's been reset.");
			return false;
		}
	}

	bool retVal = dsGLCommandBuffer_setFenceSyncs(commandBuffer, &sync, 1, bufferReadback);
	if (!retVal)
	{
		for (uint32_t i = 0; i < fenceCount; ++i)
		{
			dsGLGfxFence* glFence = (dsGLGfxFence*)fences[i];
			DS_VERIFY(dsSpinlock_lock(&glFence->lock));
			// Another thread could have reset the sync in the meantime.
			if (glFence->sync == sync)
			{
				dsGLFenceSyncRef_freeRef(sync);
				glFence->sync = NULL;
			}
			DS_VERIFY(dsSpinlock_unlock(&glFence->lock));
		}
	}

	dsGLFenceSyncRef_freeRef(sync);
	return retVal;
}

dsGfxFenceResult dsGLGfxFence_wait(dsResourceManager* resourceManager, dsGfxFence* fence,
	uint64_t timeout)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(fence);

	// Need to grab the current sync and add ref it internally in case another thread simultaneously
	// resets it.
	dsGLGfxFence* glFence = (dsGLGfxFence*)fence;
	DS_VERIFY(dsSpinlock_lock(&glFence->lock));
	dsGLFenceSyncRef* syncRef = glFence->sync;
	if (!syncRef)
	{
		DS_VERIFY(dsSpinlock_unlock(&glFence->lock));
		return dsGfxFenceResult_Unset;
	}
	dsGLFenceSyncRef_addRef(syncRef);
	DS_VERIFY(dsSpinlock_unlock(&glFence->lock));

	dsGLFenceSync* sync;
	DS_ATOMIC_LOAD_PTR(&syncRef->sync, &sync);
	if (!sync)
	{
		dsGLFenceSync_freeRef(sync);
		return dsGfxFenceResult_WaitingToQueue;
	}

	GLenum glResult = glClientWaitSync(sync->glSync, 0, timeout);
	dsGfxFenceResult result;
	if (glResult == GL_ALREADY_SIGNALED || glResult == GL_CONDITION_SATISFIED)
		result = dsGfxFenceResult_Success;
	else if (glResult == GL_TIMEOUT_EXPIRED)
		result = dsGfxFenceResult_Timeout;
	else
	{
		DS_ASSERT(glResult == GL_WAIT_FAILED);
		GLenum lastError = dsGetLastGLError();
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error waiting for fence: %s",
			AnyGL_errorString(lastError));
		errno = dsGetGLErrno(lastError);
		result = dsGfxFenceResult_Error;
	}

	dsGLFenceSyncRef_freeRef(syncRef);
	return result;
}

bool dsGLGfxFence_reset(dsResourceManager* resourceManager, dsGfxFence* fence)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(fence);

	dsGLGfxFence* glFence = (dsGLGfxFence*)fence;
	DS_VERIFY(dsSpinlock_lock(&glFence->lock));
	dsGLFenceSyncRef* sync = glFence->sync;
	glFence->sync = NULL;
	DS_VERIFY(dsSpinlock_unlock(&glFence->lock));

	if (sync)
		dsGLFenceSyncRef_freeRef(sync);
	return true;
}

bool dsGLGfxFence_destroy(dsResourceManager* resourceManager, dsGfxFence* fence)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(fence);

	dsGLGfxFence* glFence = (dsGLGfxFence*)fence;
	if (glFence->sync)
		dsGLFenceSyncRef_freeRef(glFence->sync);
	dsSpinlock_destroy(&glFence->lock);
	if (fence->allocator)
		return dsAllocator_free(fence->allocator, fence);

	return true;
}

void dsGLFenceSync_addRef(dsGLFenceSync* sync)
{
	DS_ASSERT(sync);
	DS_ATOMIC_FETCH_ADD32(&sync->refCount, 1);
}

void dsGLFenceSync_freeRef(dsGLFenceSync* sync)
{
	DS_ASSERT(sync);
	if (DS_ATOMIC_FETCH_ADD32(&sync->refCount, -1) != 1)
		return;

	DS_ASSERT(sync->glSync);
	glDeleteSync(sync->glSync);
	DS_VERIFY(dsAllocator_free(sync->allocator, sync));
}

void dsGLFenceSyncRef_addRef(dsGLFenceSyncRef* sync)
{
	DS_ASSERT(sync);
	DS_ATOMIC_FETCH_ADD32(&sync->refCount, 1);
}

void dsGLFenceSyncRef_freeRef(dsGLFenceSyncRef* sync)
{
	DS_ASSERT(sync);
	if (DS_ATOMIC_FETCH_ADD32(&sync->refCount, -1) != 1)
		return;

	if (sync->sync)
		dsGLFenceSync_freeRef(sync->sync);
	DS_VERIFY(dsAllocator_free(sync->allocator, sync));
}

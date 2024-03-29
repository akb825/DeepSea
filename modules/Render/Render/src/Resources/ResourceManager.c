/*
 * Copyright 2016-2023 Aaron Barany
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

#include <DeepSea/Render/Resources/ResourceManager.h>

#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Thread/ThreadPool.h>
#include <DeepSea/Core/Thread/ThreadStorage.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Render/Types.h>

#include <stdlib.h>
#include <string.h>

static void startThreadFunc(void* userData)
{
	dsResourceManager* resourceManager = (dsResourceManager*)userData;
	if (!dsResourceManager_acquireResourceContext(resourceManager))
	{
		DS_LOG_FATAL(DS_RENDER_LOG_TAG,
			"Couldn't acquire resource context for thread pool thread.");
		abort();
	}
}

static void endThreadFunc(void* userData)
{
	dsResourceManager* resourceManager = (dsResourceManager*)userData;
	dsResourceManager_releaseResourceContext(resourceManager);
}

const char* dsResourceManager_noContextError = "Resources can only be manipulated from the main "
	"thread or threads that have created a resource context.";

dsThreadPool* dsResourceManager_createThreadPool(dsAllocator* allocator,
	dsResourceManager* resourceManager, unsigned int threadCount, size_t stackSize)
{
	if (!allocator || !resourceManager)
	{
		errno = EINVAL;
		return NULL;
	}

	if (threadCount > resourceManager->maxResourceContexts)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
			"Thread pool thread count %u exceeds maximum number of resource contexts %u.",
			threadCount, resourceManager->maxResourceContexts);
		errno = EINVAL;
		return NULL;
	}

	return dsThreadPool_create(allocator, threadCount, stackSize, &startThreadFunc, &endThreadFunc,
		resourceManager);
}

bool dsResourceManager_acquireResourceContext(dsResourceManager* resourceManager)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || !resourceManager->renderer ||
		!resourceManager->acquireResourceContextFunc ||
		!resourceManager->releaseResourceContextFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (dsThread_equal(resourceManager->renderer->mainThread, dsThread_thisThreadID()))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot acquire a resource context for the main thread.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (dsThreadStorage_get(resourceManager->_resourceContext))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resource context already acquired for this thread.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	// Locklessly update the resource context count. It will fail if the current count is >= the
	// maximum at any time, guaranteeing that it will retry if the value has been updated before the
	// has been written to the count.
	uint32_t resourceContextCount;
	uint32_t newResourceContextCount;
	DS_ATOMIC_LOAD32(&resourceManager->resourceContextCount, &resourceContextCount);
	do
	{
		if (resourceContextCount >= resourceManager->maxResourceContexts)
		{
			errno = ESIZE;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Maximum render contexts exceeded.");
			DS_PROFILE_FUNC_RETURN(false);
		}
		newResourceContextCount = resourceContextCount + 1;
	}
	while (!DS_ATOMIC_COMPARE_EXCHANGE32(&resourceManager->resourceContextCount,
		&resourceContextCount, &newResourceContextCount, true));

	dsResourceContext* context = resourceManager->acquireResourceContextFunc(resourceManager);
	if (!context)
	{
		// Allocation failed, decrement the context count incremented earlier.
		DS_ATOMIC_FETCH_ADD32(&resourceManager->resourceContextCount, -1);
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!dsThreadStorage_set(resourceManager->_resourceContext, context))
	{
		// Setting the context failed, decrement the context count incremented earlier.
		resourceManager->releaseResourceContextFunc(resourceManager, context);
		DS_ATOMIC_FETCH_ADD32(&resourceManager->resourceContextCount, -1);
		DS_PROFILE_FUNC_RETURN(false);
	}

	DS_PROFILE_FUNC_RETURN(true);
}

bool dsResourceManager_flushResourceContext(dsResourceManager* resourceManager)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || !resourceManager->flushResourceContextFunc)
		DS_PROFILE_FUNC_RETURN(true);

	// Not needed for main thread.
	if (dsThread_equal(resourceManager->renderer->mainThread, dsThread_thisThreadID()))
		DS_PROFILE_FUNC_RETURN(true);

	dsResourceContext* context = (dsResourceContext*)dsThreadStorage_get(
		resourceManager->_resourceContext);
	if (!context)
	{
		errno = EPERM;
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool result = resourceManager->flushResourceContextFunc(resourceManager, context);
	DS_PROFILE_FUNC_RETURN(result);
}

bool dsResourceManager_releaseResourceContext(dsResourceManager* resourceManager)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || !resourceManager->releaseResourceContextFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	// Destroying a context when not set is a NOP.
	dsResourceContext* context = (dsResourceContext*)dsThreadStorage_get(
		resourceManager->_resourceContext);
	if (!context)
		DS_PROFILE_FUNC_RETURN(true);

	if (!resourceManager->releaseResourceContextFunc(resourceManager, context))
		DS_PROFILE_FUNC_RETURN(false);

	DS_ATOMIC_FETCH_ADD32(&resourceManager->resourceContextCount, -1);
	DS_VERIFY(dsThreadStorage_set(resourceManager->_resourceContext, NULL));
	DS_PROFILE_FUNC_RETURN(true);
}

bool dsResourceManager_canUseResources(const dsResourceManager* resourceManager)
{
	if (!resourceManager || !resourceManager->renderer)
		return false;

	return dsThread_equal(resourceManager->renderer->mainThread, dsThread_thisThreadID()) ||
		dsThreadStorage_get(resourceManager->_resourceContext);
}

void dsResourceManager_reportStatistics(const dsResourceManager* resourceManager)
{
	DS_PROFILE_FUNC_START();
	if (!resourceManager)
		DS_PROFILE_FUNC_RETURN_VOID();

	DS_PROFILE_STAT("ResourceManager", "Resource contexts", resourceManager->resourceContextCount);
	DS_PROFILE_STAT("ResourceManager", "Buffers", resourceManager->bufferCount);
	DS_PROFILE_STAT("ResourceManager", "Buffer memory (MB)",
		(double)resourceManager->bufferMemorySize/(1024.0*1024.0));
	DS_PROFILE_STAT("ResourceManager", "Geometries", resourceManager->geometryCount);
	DS_PROFILE_STAT("ResourceManager", "Textures", resourceManager->textureCount);
	DS_PROFILE_STAT("ResourceManager", "Texture memory (MB)",
		(double)resourceManager->textureMemorySize/(1024.0*1024.0));
	DS_PROFILE_STAT("ResourceManager", "Renderbuffers", resourceManager->renderbufferCount);
	DS_PROFILE_STAT("ResourceManager", "Renderbuffer memory (MB)",
		(double)resourceManager->renderbufferMemorySize/(1024.0*1024.0));
	DS_PROFILE_STAT("ResourceManager", "Framebuffers", resourceManager->framebufferCount);
	DS_PROFILE_STAT("ResourceManager", "Fences", resourceManager->fenceCount);
	DS_PROFILE_STAT("ResourceManager", "Query Pools", resourceManager->queryPoolCount);
	DS_PROFILE_STAT("ResourceManager", "Shader modules", resourceManager->shaderModuleCount);
	DS_PROFILE_STAT("ResourceManager", "Shaders", resourceManager->shaderCount);
	DS_PROFILE_STAT("ResourceManager", "Material descriptions", resourceManager->materialDescCount);
	DS_PROFILE_STAT("ResourceManager", "Shader variable group descriptions",
		resourceManager->shaderVariableGroupDescCount);
	DS_PROFILE_STAT("ResourceManager", "Shader variable groups",
		resourceManager->shaderVariableGroupCount);
	DS_PROFILE_FUNC_RETURN_VOID();
}

bool dsResourceManager_initialize(dsResourceManager* resourceManager)
{
	if (!resourceManager)
	{
		errno = EINVAL;
		return false;
	}

	memset(resourceManager, 0, sizeof(dsResourceManager));
	if (!dsThreadStorage_initialize(&resourceManager->_resourceContext))
		return false;

	return true;
}

void dsResourceManager_shutdown(dsResourceManager* resourceManager)
{
	if (!resourceManager)
		return;

	dsThreadStorage_shutdown(&resourceManager->_resourceContext);

	// Detect leaks of resources.
	if (resourceManager->resourceContextCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%u resource contexts remain allocated.",
			resourceManager->resourceContextCount);
	}

	if (resourceManager->bufferCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%u graphics buffers remain allocated.",
			resourceManager->bufferCount);
	}

	if (resourceManager->textureCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%u textures remain allocated.",
			resourceManager->textureCount);
	}

	if (resourceManager->renderbufferCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%u renderbuffers remain allocated.",
			resourceManager->renderbufferCount);
	}

	if (resourceManager->framebufferCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%u framebuffers remain allocated.",
			resourceManager->framebufferCount);
	}

	if (resourceManager->fenceCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%u fences remain allocated.",
			resourceManager->fenceCount);
	}

	if (resourceManager->queryPoolCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%u query pools remain allocated.",
			resourceManager->queryPoolCount);
	}

	if (resourceManager->shaderModuleCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%u shader modules remain allocated.",
			resourceManager->shaderModuleCount);
	}

	if (resourceManager->materialDescCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%u material descriptions remain allocated.",
			resourceManager->materialDescCount);
	}

	if (resourceManager->shaderVariableGroupDescCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%u shader variable group descriptions remain allocated.",
			resourceManager->shaderVariableGroupDescCount);
	}

	if (resourceManager->shaderVariableGroupCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%u shader variable groups remain allocated.",
			resourceManager->shaderVariableGroupCount);
	}

	if (resourceManager->shaderCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%u shaders remain allocated.",
			resourceManager->shaderCount);
	}
}

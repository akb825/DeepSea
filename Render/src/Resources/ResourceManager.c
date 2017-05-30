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

#include <DeepSea/Render/Resources/ResourceManager.h>

#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Thread/ThreadStorage.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Render/Types.h>
#include <string.h>

const char* dsResourceManager_noContextError = "Resources can only be manipulated from the main "
	"thread or threads that have created a resource context.";

bool dsResourceManager_createResourceContext(dsResourceManager* resourceManager)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || !resourceManager->renderer ||
		!resourceManager->createResourceContextFunc || !resourceManager->destroyResourceContextFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (dsThread_equal(resourceManager->renderer->mainThread, dsThread_thisThreadId()))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot create a resource context for the main thread.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (dsThreadStorage_get(resourceManager->_resourceContext))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resource context already created for this thread.");
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
			errno = EINDEX;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Maximum render contexts exceeded.");
			DS_PROFILE_FUNC_RETURN(false);
		}
		newResourceContextCount = resourceContextCount + 1;
	}
	while (!DS_ATOMIC_COMPARE_EXCHANGE32(&resourceManager->resourceContextCount,
		&resourceContextCount, &newResourceContextCount, true));

	dsResourceContext* context = resourceManager->createResourceContextFunc(resourceManager);
	if (!context)
	{
		// Allocation failed, decrement the context count incremented earlier.
		DS_ATOMIC_FETCH_ADD32(&resourceManager->resourceContextCount, -1);
		DS_PROFILE_FUNC_RETURN(false);
	}

	if (!dsThreadStorage_set(resourceManager->_resourceContext, context))
	{
		// Setting the context failed, decrement the context count incremented earlier.
		resourceManager->destroyResourceContextFunc(resourceManager, context);
		DS_ATOMIC_FETCH_ADD32(&resourceManager->resourceContextCount, -1);
		DS_PROFILE_FUNC_RETURN(false);
	}

	DS_PROFILE_FUNC_RETURN(true);
}

bool dsResourceManager_destroyResourceContext(dsResourceManager* resourceManager)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager ||  !resourceManager->destroyResourceContextFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	// Destroying a context when not set is a NOP.
	dsResourceContext* context = (dsResourceContext*)dsThreadStorage_get(
		resourceManager->_resourceContext);
	if (!context)
		DS_PROFILE_FUNC_RETURN(true);

	if (!resourceManager->destroyResourceContextFunc(resourceManager, context))
		DS_PROFILE_FUNC_RETURN(false);

	DS_ATOMIC_FETCH_ADD32(&resourceManager->resourceContextCount, -1);
	DS_VERIFY(dsThreadStorage_set(resourceManager->_resourceContext, NULL));
	DS_PROFILE_FUNC_RETURN(true);
}

bool dsResourceManager_canUseResources(const dsResourceManager* resourceManager)
{
	if (!resourceManager || !resourceManager->renderer)
		return false;

	return dsThread_equal(resourceManager->renderer->mainThread, dsThread_thisThreadId()) ||
		dsThreadStorage_get(resourceManager->_resourceContext);
}

void dsResourceManager_reportStatistics(const dsResourceManager* resourceManager)
{
	if (!resourceManager)
		return;

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
		(double)resourceManager->renderbufferMemorySize);
	DS_PROFILE_STAT("ResourceManager", "Framebuffers", resourceManager->framebufferCount);
	DS_PROFILE_STAT("ResourceManager", "Fences", resourceManager->fenceCount);
	DS_PROFILE_STAT("ResourceManager", "Shader modules", resourceManager->shaderModuleCount);
	DS_PROFILE_STAT("ResourceManager", "Shaders", resourceManager->shaderCount);
	DS_PROFILE_STAT("ResourceManager", "Material descriptions", resourceManager->materialDescCount);
	DS_PROFILE_STAT("ResourceManager", "Materials", resourceManager->materialCount);
	DS_PROFILE_STAT("ResourceManager", "Shader variable group descriptions",
		resourceManager->shaderVariableGroupDescCount);
	DS_PROFILE_STAT("ResourceManager", "Shader variable groups",
		resourceManager->shaderVariableGroupCount);
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

	dsThreadStorage_destroy(&resourceManager->_resourceContext);

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

	if (resourceManager->materialCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%u materials remain allocated.",
			resourceManager->materialCount);
	}

	if (resourceManager->shaderVariableGroupDescCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%u shader variable group descriptions remain allocated.",
			resourceManager->shaderVariableGroupDescCount);
	}

	if (resourceManager->shaderVariableGroupCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%u shader variable groups remain allocated.",
			resourceManager->shaderVariableGroupDescCount);
	}

	if (resourceManager->shaderCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%u shaders remain allocated.",
			resourceManager->shaderCount);
	}

	if (resourceManager->framebufferCount)
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "%u framebuffers remain allocated.",
			resourceManager->framebufferCount);
	}
}

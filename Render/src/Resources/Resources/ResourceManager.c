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

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Thread/ThreadStorage.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Types.h>
#include <errno.h>

bool dsResourceManager_createResourceContext(dsResourceManager* resourceManager)
{
	if (!resourceManager || !resourceManager->renderer ||
		!resourceManager->createResourceContextFunc || !resourceManager->destroyResourceContextFunc)
	{
		errno = EINVAL;
		return false;
	}

	if (dsThread_equal(resourceManager->renderer->mainThread, dsThread_thisThreadId()))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Cannot create a resource context for the main thread.");
		return false;
	}

	if (dsThreadStorage_get(resourceManager->_resourceContext))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Resource context already created for this thread.");
		return false;
	}

	if (resourceManager->resourceContextCount >= resourceManager->maxResourceContexts)
	{
		errno = ERANGE;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Maximum render contexts exceeded.");
		return false;
	}

	dsResourceContext* context = resourceManager->createResourceContextFunc(resourceManager);
	if (!context)
		return false;

	if (!dsThreadStorage_set(resourceManager->_resourceContext, context))
		return false;

	return true;
}

bool dsResourceManager_destroyResourceContext(dsResourceManager* resourceManager)
{
	if (!resourceManager ||  !resourceManager->destroyResourceContextFunc)
	{
		errno = EINVAL;
		return false;
	}

	// Destroying a context when not set is a NOP.
	dsResourceContext* context = (dsResourceContext*)dsThreadStorage_get(
		resourceManager->_resourceContext);
	if (!context)
		return true;

	if (!resourceManager->destroyResourceContextFunc(resourceManager, context))
		return false;

	DS_VERIFY(dsThreadStorage_set(resourceManager->_resourceContext, NULL));
	return true;
}

bool dsResourceManager_canUseResources(const dsResourceManager* resourceManager)
{
	if (!resourceManager || !resourceManager->renderer)
		return false;

	return dsThread_equal(resourceManager->renderer->mainThread, dsThread_thisThreadId()) ||
		dsThreadStorage_get(resourceManager->_resourceContext);
}

bool dsResourceManager_initialize(dsResourceManager* resourceManager)
{
	if (!resourceManager)
	{
		errno = EINVAL;
		return false;
	}

	if (!dsThreadStorage_initialize(&resourceManager->_resourceContext))
		return false;

	return true;
}

void dsResourceManager_shutdown(dsResourceManager* resourceManager)
{
	if (!resourceManager)
		return;

	dsThreadStorage_destroy(&resourceManager->_resourceContext);
}

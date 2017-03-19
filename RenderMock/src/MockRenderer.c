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

#include <DeepSea/RenderMock/MockRenderer.h>

#include "Resources/MockResourceManager.h"
#include "MockRenderSurface.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Renderer.h>

dsRenderer* dsMockRenderer_create(dsAllocator* allocator)
{
	if (!allocator)
	{
		errno = EINVAL;
		return NULL;
	}

	dsRenderer* renderer = (dsRenderer*)dsAllocator_alloc(allocator, sizeof(dsRenderer));
	if (!renderer)
		return NULL;

	if (!dsRenderer_initialize(renderer))
	{
		if (allocator->freeFunc)
			dsAllocator_free(allocator, renderer);
		return NULL;
	}

	dsResourceManager* resourceManager = dsMockResourceManager_create(renderer, allocator);
	if (!resourceManager)
	{
		if (allocator->freeFunc)
			dsAllocator_free(allocator, renderer);
		return NULL;
	}

	renderer->allocator = dsAllocator_keepPointer(allocator);
	renderer->resourceManager = resourceManager;

	renderer->surfaceColorFormat = dsGfxFormat_decorate(dsGfxFormat_R8G8B8, dsGfxFormat_UNorm);
	renderer->surfaceDepthStencilFormat = dsGfxFormat_D24S8;
	renderer->surfaceSamples = 4;

	renderer->createRenderSurfaceFunc = &dsMockRenderSurface_create;
	renderer->updateRenderSurfaceFunc = &dsMockRenderSurface_update;
	renderer->destroyRenderSurfaceFunc = &dsMockRenderSurface_destroy;

	return renderer;
}

void dsMockRenderer_destroy(dsRenderer* renderer)
{
	if (!renderer)
		return;

	dsMockResourceManager_destroy(renderer->resourceManager);
	dsRenderer_shutdown(renderer);
	if (renderer->allocator)
		dsAllocator_free(renderer->allocator, renderer);
}

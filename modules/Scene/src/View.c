/*
 * Copyright 2019 Aaron Barany
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

#include <DeepSea/Scene/View.h>

#include "SceneTypes.h"
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Geometry/Frustum3.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Render/Resources/Framebuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Renderbuffer.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Scene/SceneCullManager.h>
#include <string.h>

typedef struct IndexNode
{
	dsHashTableNode node;
	uint32_t index;
} IndexNode;

typedef struct dsViewPrivate
{
	dsView view;

	dsViewSurfaceInfo* surfaceInfos;
	void** surfaces;
	dsViewFramebufferInfo* framebufferInfos;
	dsFramebuffer** framebuffers;
	uint32_t* pipelineFramebuffers;
	uint32_t surfaceCount;
	uint32_t framebufferCount;

	dsHashTable* surfaceTable;
	dsFramebufferSurface* tempSurfaces;

	uint32_t lastSurfaceSamples;
	bool sizeUpdated;
	bool surfaceSet;
} dsViewPrivate;

static size_t fullAllocSize(const dsScene* scene, const dsViewSurfaceInfo* surfaces,
	uint32_t surfaceCount, const dsViewFramebufferInfo* framebuffers, uint32_t framebufferCount)
{
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsViewPrivate)) +
		DS_ALIGNED_SIZE(sizeof(dsViewSurfaceInfo)*surfaceCount) +
		DS_ALIGNED_SIZE(sizeof(void*)*surfaceCount) +
		DS_ALIGNED_SIZE(sizeof(IndexNode)*surfaceCount) +
		dsHashTable_fullAllocSize(dsHashTable_getTableSize(surfaceCount)) +
		DS_ALIGNED_SIZE(sizeof(dsViewFramebufferInfo)*framebufferCount) +
		DS_ALIGNED_SIZE(sizeof(dsFramebuffer*)*framebufferCount) +
		DS_ALIGNED_SIZE(sizeof(uint32_t)*scene->pipelineCount);
	if (scene->globalValueCount > 0)
		fullSize += dsSharedMaterialValues_fullAllocSize(scene->globalValueCount);

	for (uint32_t i = 0; i < surfaceCount; ++i)
		fullSize += DS_ALIGNED_SIZE(strlen(surfaces[i].name) + 1);

	uint32_t maxSurfaces = 0;
	for (uint32_t i = 0; i < framebufferCount; ++i)
	{
		const dsViewFramebufferInfo* framebuffer = framebuffers + i;
		fullSize += DS_ALIGNED_SIZE(strlen(framebuffer->name) + 1);
		fullSize += DS_ALIGNED_SIZE(sizeof(dsFramebufferSurface)*framebuffer->surfaceCount);
		for (uint32_t i = 0; i < framebuffer->surfaceCount; ++i)
			fullSize += DS_ALIGNED_SIZE(strlen((const char*)framebuffer->surfaces[i].surface) + 1);
		maxSurfaces = dsMax(maxSurfaces, framebuffer->surfaceCount);
	}

	fullSize += DS_ALIGNED_SIZE(sizeof(dsFramebufferSurface*)*maxSurfaces);

	return fullSize;
}

static bool validateSurfacesFramebuffers(const dsResourceManager* resourceManager,
	const dsViewSurfaceInfo* surfaces, uint32_t surfaceCount,
	const dsViewFramebufferInfo* framebuffers, uint32_t framebufferCount)
{
	for (uint32_t i = 0; i < surfaceCount; ++i)
	{
		const dsViewSurfaceInfo* surface = surfaces + i;
		if (!surface->name)
		{
			errno = EINVAL;
			return false;
		}

		if (surface->surface)
			continue;

		if (surface->surfaceType >= dsGfxSurfaceType_ColorRenderSurface &&
			surface->surfaceType <= dsGfxSurfaceType_DepthRenderSurfaceRight)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_SCENE_LOG_TAG, "View cannot automatically create render surfaces, only "
				"offscreens and renderbuffers.");
			return false;
		}

		if (!dsGfxFormat_offscreenSupported(resourceManager, surface->createInfo.format))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Format not supported for offscreens or renderbuffers.");
			return false;
		}

		if (surface->createInfo.width == 0 && surface->widthRatio <= 0.0f)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Invalid surface width.");
			return false;
		}

		if (surface->createInfo.height == 0 && surface->heightRatio <= 0.0f)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Invalid surface height.");
			return false;
		}
	}

	for (uint32_t i = 0; i < framebufferCount; ++i)
	{
		const dsViewFramebufferInfo* framebuffer = framebuffers + i;
		if (!framebuffer->surfaces && framebuffer->surfaceCount != 0)
		{
			errno = EINVAL;
			return false;
		}

		for (uint32_t i = 0; i < framebuffer->surfaceCount; ++i)
		{
			if (!framebuffer->surfaces[i].surface)
			{
				errno = EINVAL;
				return false;
			}
		}
	}

	return true;
}

static void destroyMidCreate(dsView* view)
{
	dsSharedMaterialValues_destroy(view->globalValues);
	if (view->destroyUserDataFunc)
		view->destroyUserDataFunc(view->userData);
	if (view->allocator)
		DS_VERIFY(dsAllocator_free(view->allocator, view));
}

uint32_t dsView_registerCullID(const dsView* view, dsSceneCullID cullID)
{
	if (!view || !cullID)
	{
		errno = EINVAL;
		return DS_NO_SCENE_CULL;
	}

	return dsSceneCullManager_registerCullID((dsSceneCullManager*)&view->cullManager, cullID);
}

dsView* dsView_create(const dsScene* scene, dsAllocator* allocator,
	const dsViewSurfaceInfo* surfaces, uint32_t surfaceCount,
	const dsViewFramebufferInfo* framebuffers, uint32_t framebufferCount, uint32_t width,
	uint32_t height, void* userData, dsDestroySceneUserDataFunction destroyUserDataFunc)
{
	if (!scene || !surfaces || surfaceCount == 0 || !framebuffers || framebufferCount == 0)
	{
		errno = EINVAL;
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return NULL;
	}

	if (width == 0 || height == 0)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "View size must not be 0.");
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return NULL;
	}

	if (!allocator)
		allocator = scene->allocator;

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "View allocator must support freeing memory.");
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return NULL;
	}

	if (!validateSurfacesFramebuffers(scene->renderer->resourceManager, surfaces, surfaceCount,
			framebuffers, framebufferCount))
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return NULL;
	}

	size_t fullSize = fullAllocSize(scene, surfaces, surfaceCount, framebuffers, framebufferCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsViewPrivate* privateView = DS_ALLOCATE_OBJECT(&bufferAlloc, dsViewPrivate);
	DS_ASSERT(privateView);

	dsRenderer* renderer = scene->renderer;
	dsView* view = (dsView*)privateView;
	view->scene = scene;
	view->allocator = dsAllocator_keepPointer(allocator);
	view->userData = userData;
	view->destroyUserDataFunc = destroyUserDataFunc;
	view->width = width;
	view->height = height;
	dsMatrix44_identity(view->cameraMatrix);
	dsMatrix44_identity(view->viewMatrix);
	dsMatrix44_identity(view->projectionMatrix);
	dsMatrix44_identity(view->viewProjectionMatrix);
	dsFrustum3_fromMatrix(view->viewFrustum, view->viewProjectionMatrix, renderer->clipHalfDepth,
		renderer->clipInvertY);
	view->viewport.min.x = 0;
	view->viewport.min.y = 0;
	view->viewport.min.z = 0;
	view->viewport.max.x = (float)width;
	view->viewport.max.y = (float)height;
	view->viewport.max.z = 1;
	dsSceneCullManager_reset(&view->cullManager);

	if (scene->globalValueCount > 0)
	{
		view->globalValues = dsSharedMaterialValues_create((dsAllocator*)&bufferAlloc,
			scene->globalValueCount);
		DS_ASSERT(view->globalValues);
	}
	else
		view->globalValues = NULL;

	privateView->surfaceInfos = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsViewSurfaceInfo,
		surfaceCount);
	DS_ASSERT(privateView->surfaceInfos);
	memcpy(privateView->surfaceInfos, surfaces, surfaceCount);

	privateView->surfaces = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, void*, surfaceCount);
	DS_ASSERT(privateView->surfaces);

	uint32_t surfaceTableSize = dsHashTable_getTableSize(surfaceCount);
	privateView->surfaceTable = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		dsHashTable_fullAllocSize(surfaceTableSize));
	DS_ASSERT(privateView->surfaceTable);
	DS_VERIFY(dsHashTable_initialize(privateView->surfaceTable, surfaceTableSize, &dsHashString,
		&dsHashStringEqual));

	IndexNode* surfaceNodes = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, IndexNode, surfaceCount);
	DS_ASSERT(surfaceNodes);

	for (uint32_t i = 0; i < surfaceCount; ++i)
	{
		dsViewSurfaceInfo* surfaceInfo = privateView->surfaceInfos + i;
		size_t nameLen = strlen(surfaces[i].name) + 1;
		surfaceInfo->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
		memcpy((void*)surfaceInfo->name, surfaces[i].name, nameLen);
		privateView->surfaces[i] = surfaceInfo->surface;

		IndexNode* node = surfaceNodes + i;
		node->index = i;
		if (!dsHashTable_insert(privateView->surfaceTable, surfaceInfo->name,
			(dsHashTableNode*)node, NULL))
		{
			errno = EINVAL;
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Surface '%s' provided multiple times to the view.",
				surfaceInfo->name);
			destroyMidCreate(view);
			return NULL;
		}
	}

	privateView->framebufferInfos = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsViewFramebufferInfo,
		framebufferCount);
	DS_ASSERT(privateView->framebufferInfos);
	memcpy(privateView->framebufferInfos, framebuffers, framebufferCount);

	privateView->framebuffers = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsFramebuffer*,
		framebufferCount);
	DS_ASSERT(privateView->framebuffers);
	memset(privateView->framebuffers, 0, sizeof(dsFramebuffer*));

	uint32_t maxSurfaces = 0;
	for (uint32_t i = 0; i < framebufferCount; ++i)
	{
		dsViewFramebufferInfo* framebufferInfo = privateView->framebufferInfos + i;
		size_t nameLen = strlen(framebuffers[i].name) + 1;
		framebufferInfo->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
		memcpy((void*)framebufferInfo->name, framebuffers[i].name, nameLen);

		framebufferInfo->surfaces = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsFramebufferSurface,
			framebuffers[i].surfaceCount);
		DS_ASSERT(framebufferInfo->surfaces);
		memcpy((void*)framebufferInfo->surfaces, framebuffers[i].surfaces,
			sizeof(dsFramebufferSurface)*framebuffers[i].surfaceCount);
		for (uint32_t j = 0; j < framebufferInfo->surfaceCount; ++j)
		{
			const char* surfaceName = (const char*)framebuffers[i].surfaces[j].surface;
			dsFramebufferSurface* surface = (dsFramebufferSurface*)framebufferInfo->surfaces + i;
			IndexNode* node = (IndexNode*)dsHashTable_find(privateView->surfaceTable, surfaceName);
			if (!node)
			{
				errno = EINVAL;
				DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Framebuffer surface '%s' not in the view.",
					surfaceName);
				destroyMidCreate(view);
				return NULL;
			}

			if (privateView->surfaceInfos[node->index].surfaceType != surface->surfaceType)
			{
				errno = EINVAL;
				DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
					"Framebuffer surface type doesn't match for surface '%s'.",
					surfaceName);
				destroyMidCreate(view);
				return NULL;
			}

			nameLen = strlen(surfaceName) + 1;
			surface->surface = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
			DS_ASSERT(surface->surface);
			memcpy(surface->surface, surfaceName, nameLen);
		}

		maxSurfaces = dsMax(maxSurfaces, framebufferInfo->surfaceCount);
	}

	privateView->tempSurfaces = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsFramebufferSurface,
		maxSurfaces);
	DS_ASSERT(privateView->tempSurfaces || maxSurfaces == 0);

	privateView->pipelineFramebuffers = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, uint32_t,
		scene->pipelineCount);
	DS_ASSERT(privateView->pipelineFramebuffers);
	for (uint32_t i = 0; i < scene->pipelineCount; ++i)
	{
		dsSceneRenderPass* renderPass = scene->pipeline[i].renderPass;
		if (!renderPass)
			continue;

		bool found = false;
		for (uint32_t j = 0; j < framebufferCount; ++j)
		{
			if (strcmp(renderPass->framebuffer, framebuffers[i].name) == 0)
			{
				privateView->pipelineFramebuffers[i] = j;
				found = true;
				break;
			}
		}

		if (!found)
		{
			errno = EINVAL;
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
				"Framebuffer '%s' requested from scene's pipeline not in the view.",
				renderPass->framebuffer);
			destroyMidCreate(view);
			return NULL;
		}
	}

	privateView->lastSurfaceSamples = renderer->surfaceSamples;
	privateView->sizeUpdated = true;
	privateView->surfaceSet = true;

	return view;
}

bool dsView_setDimensions(dsView* view, uint32_t width, uint32_t height)
{
	if (!view || width == 0 || height == 0)
	{
		errno = EINVAL;
		return false;
	}

	if (view->width == width && view->height == height)
		return true;

	dsViewPrivate* privateView = (dsViewPrivate*)view;
	view->width = width;
	view->height = height;
	privateView->sizeUpdated = true;
	return true;
}

void* dsView_getSurface(dsGfxSurfaceType* outType, const dsView* view, const char* name)
{
	if (!view || !name)
		return NULL;

	const dsViewPrivate* privateView = (const dsViewPrivate*)view;
	IndexNode* foundNode = (IndexNode*)dsHashTable_find(privateView->surfaceTable, name);
	if (!foundNode)
		return NULL;

	if (outType)
		*outType = privateView->surfaceInfos[foundNode->index].surfaceType;
	return privateView->surfaces[foundNode->index];
}

bool dsView_setSurface(dsView* view, const char* name, void* surface, dsGfxSurfaceType surfaceType)
{
	if (!view || !name || !surface)
	{
		errno = EINVAL;
		return false;
	}

	dsViewPrivate* privateView = (dsViewPrivate*)view;
	IndexNode* foundNode = (IndexNode*)dsHashTable_find(privateView->surfaceTable, name);
	if (!foundNode)
	{
		errno = ENOTFOUND;
		return false;
	}

	dsViewSurfaceInfo* surfaceInfo = privateView->surfaceInfos + foundNode->index;
	if (!surfaceInfo->surface || surfaceInfo->surfaceType != surfaceType)
	{
		errno = EPERM;
		return false;
	}

	if (surfaceInfo->surface == surface)
		return true;

	surfaceInfo->surface = surface;
	privateView->surfaces[foundNode->index] = surface;
	privateView->surfaceSet = true;
	return true;
}

bool dsView_update(dsView* view)
{
	if (!view)
	{
		errno = EINVAL;
		return false;
	}

	dsViewPrivate* privateView = (dsViewPrivate*)view;
	dsRenderer* renderer = view->scene->renderer;
	dsResourceManager* resourceManager = renderer->resourceManager;
	bool sizeChanged = privateView->sizeUpdated;
	bool surfaceSet = privateView->surfaceSet;
	bool samplesChanged = privateView->lastSurfaceSamples != renderer->surfaceSamples;
	if (!sizeChanged && !surfaceSet && !samplesChanged)
		return true;

	for (uint32_t i = 0; i < privateView->surfaceCount; ++i)
	{
		const dsViewSurfaceInfo* surfaceInfo = privateView->surfaceInfos + i;
		// Leave explicitly provided surfaces untouched.
		if (surfaceInfo->surface)
			continue;

		// Check if it would have changed.
		if (privateView->surfaces[i] &&
			((surfaceInfo->createInfo.width > 0 && surfaceInfo->createInfo.height > 0) ||
				!sizeChanged) &&
			(surfaceInfo->createInfo.samples != DS_DEFAULT_ANTIALIAS_SAMPLES || !samplesChanged))
		{
			continue;
		}

		uint32_t width;
		if (surfaceInfo->createInfo.width > 0)
			width = surfaceInfo->createInfo.width;
		else
			width = (uint32_t)roundf(surfaceInfo->widthRatio*(float)view->width);

		uint32_t height;
		if (surfaceInfo->createInfo.height > 0)
			height = surfaceInfo->createInfo.height;
		else
			height = (uint32_t)roundf(surfaceInfo->heightRatio*(float)view->height);

		switch (surfaceInfo->surfaceType)
		{
			case dsGfxSurfaceType_Offscreen:
			{
				dsTextureInfo textureInfo = surfaceInfo->createInfo;
				textureInfo.width = width;
				textureInfo.height = height;
				dsOffscreen* offscreen = dsTexture_createOffscreen(resourceManager, view->allocator,
					surfaceInfo->usage, surfaceInfo->memoryHints, &textureInfo,
					surfaceInfo->resolve);
				if (!offscreen)
					return false;

				DS_VERIFY(dsTexture_destroy((dsTexture*)privateView->surfaces[i]));
				privateView->surfaces[i] = offscreen;
				break;
			}
			case dsGfxSurfaceType_Renderbuffer:
			{
				dsRenderbuffer* renderbuffer = dsRenderbuffer_create(resourceManager,
					view->allocator, surfaceInfo->usage, surfaceInfo->createInfo.format, width,
					height, surfaceInfo->createInfo.samples);
				if (!renderbuffer)
					return false;

				DS_VERIFY(dsRenderbuffer_destroy((dsRenderbuffer*)privateView->surfaces[i]));
				privateView->surfaces[i] = renderbuffer;
				break;
			}
			default:
				DS_ASSERT(false);
				break;
		}
		surfaceSet = true;
	}

	// Only re-create framebuffers if surfaces have been set.
	if (!surfaceSet)
	{
		privateView->sizeUpdated = false;
		privateView->surfaceSet = false;
		privateView->lastSurfaceSamples = renderer->surfaceSamples;
		return true;
	}

	// Re-create all framebuffers to avoid complicated logic to decide which ones specifically need
	// to change.
	for (uint32_t i = 0; i < privateView->framebufferCount; ++i)
	{
		const dsViewFramebufferInfo* framebufferInfo = privateView->framebufferInfos + i;

		uint32_t width;
		if (framebufferInfo->width > 0)
			width = (uint32_t)roundf(framebufferInfo->width);
		else
			width = (uint32_t)roundf(-framebufferInfo->width*(float)view->width);

		uint32_t height;
		if (framebufferInfo->height > 0)
			height = (uint32_t)roundf(framebufferInfo->height);
		else
			height = (uint32_t)roundf(-framebufferInfo->height*(float)view->height);

		for (uint32_t j = 0; j < framebufferInfo->surfaceCount; ++j)
		{
			dsFramebufferSurface* surface = privateView->tempSurfaces + j;
			*surface = framebufferInfo->surfaces[j];

			IndexNode* foundNode = (IndexNode*)dsHashTable_find(privateView->surfaceTable,
				surface->surface);
			DS_ASSERT(foundNode);
			DS_ASSERT(privateView->surfaceInfos[foundNode->index].surfaceType ==
				surface->surfaceType);
			surface->surface = privateView->surfaces[foundNode->index];
		}

		dsFramebuffer* framebuffer = dsFramebuffer_create(resourceManager, view->allocator,
			framebufferInfo->name, privateView->tempSurfaces, framebufferInfo->surfaceCount, width,
			height, framebufferInfo->layers);
		if (!framebuffer)
			return false;

		DS_VERIFY(dsFramebuffer_destroy(privateView->framebuffers[i]));
		privateView->framebuffers[i] = framebuffer;
	}

	privateView->sizeUpdated = false;
	privateView->surfaceSet = false;
	privateView->lastSurfaceSamples = renderer->surfaceSamples;
	return true;
}

bool dsView_destroy(dsView* view)
{
	if (!view)
		return true;

	dsViewPrivate* privateView = (dsViewPrivate*)view;
	for (uint32_t i = 0; i < privateView->framebufferCount; ++i)
	{
		if (!dsFramebuffer_destroy(privateView->framebuffers[i]))
		{
			DS_ASSERT(i == 0);
			return false;
		}
	}

	for (uint32_t i = 0; i < privateView->surfaceCount; ++i)
	{
		const dsViewSurfaceInfo* surfaceInfo = privateView->surfaceInfos + i;
		if (surfaceInfo->surface)
			continue;

		switch (surfaceInfo->surfaceType)
		{
			case dsGfxSurfaceType_Offscreen:
				DS_VERIFY(dsTexture_destroy((dsTexture*)privateView->surfaces[i]));
				break;
			case dsGfxSurfaceType_Renderbuffer:
				DS_VERIFY(dsRenderbuffer_destroy((dsRenderbuffer*)privateView->surfaces[i]));
				break;
			default:
				DS_ASSERT(false);
				break;
		}
	}

	dsSharedMaterialValues_destroy(view->globalValues);
	if (view->destroyUserDataFunc)
		view->destroyUserDataFunc(view->userData);
	if (view->allocator)
		DS_VERIFY(dsAllocator_free(view->allocator, view));

	return true;
}

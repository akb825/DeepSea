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

#include "SceneThreadManagerInternal.h"
#include "SceneTypes.h"
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Geometry/Frustum3.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Render/Resources/Framebuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Renderbuffer.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/RenderPass.h>
#include <DeepSea/Scene/SceneGlobalData.h>
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
		for (uint32_t j = 0; j < framebuffer->surfaceCount; ++j)
			fullSize += DS_ALIGNED_SIZE(strlen((const char*)framebuffer->surfaces[j].surface) + 1);
		maxSurfaces = dsMax(maxSurfaces, framebuffer->surfaceCount);
	}

	fullSize += DS_ALIGNED_SIZE(sizeof(dsFramebufferSurface)*maxSurfaces);

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

		if (!dsGfxFormat_renderTargetSupported(resourceManager, surface->createInfo.format))
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

		for (uint32_t j = 0; j < 3; ++j)
		{
			if (framebuffer->viewport.min.values[j] < 0.0f ||
				framebuffer->viewport.max.values[j] > 1.0f)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_SCENE_LOG_TAG,
					"View framebuffer viewport values must be in the range [0, 1].");
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

static void updatedCameraProjection(dsView* view)
{
	dsRenderer* renderer = view->scene->renderer;
	dsMatrix44_mul(view->viewProjectionMatrix, view->projectionMatrix, view->viewMatrix);
	dsFrustum3_fromMatrix(view->viewFrustum, view->viewProjectionMatrix, renderer->clipHalfDepth,
		renderer->clipInvertY);
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
	memcpy(privateView->surfaceInfos, surfaces, sizeof(dsViewSurfaceInfo)*surfaceCount);

	privateView->surfaces = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, void*, surfaceCount);
	DS_ASSERT(privateView->surfaces);
	privateView->surfaceCount = surfaceCount;

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
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Surface '%s' isn't unique within the view.",
				surfaceInfo->name);
			destroyMidCreate(view);
			return NULL;
		}
	}

	privateView->framebufferInfos = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsViewFramebufferInfo,
		framebufferCount);
	DS_ASSERT(privateView->framebufferInfos);
	memcpy(privateView->framebufferInfos, framebuffers,
		sizeof(dsViewFramebufferInfo)*framebufferCount);

	privateView->framebuffers = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsFramebuffer*,
		framebufferCount);
	DS_ASSERT(privateView->framebuffers);
	memset(privateView->framebuffers, 0, sizeof(dsFramebuffer*));
	privateView->framebufferCount = framebufferCount;

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
			dsFramebufferSurface* surface = (dsFramebufferSurface*)framebufferInfo->surfaces + j;
			const char* surfaceName = (const char*)surface->surface;
			IndexNode* node = (IndexNode*)dsHashTable_find(privateView->surfaceTable, surfaceName);
			if (!node)
			{
				errno = EINVAL;
				DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Framebuffer surface '%s' not in the view.",
					surfaceName);
				destroyMidCreate(view);
				return NULL;
			}

			if (surface->surfaceType == (dsGfxSurfaceType)-1)
				surface->surfaceType = privateView->surfaceInfos[node->index].surfaceType;
			else if (privateView->surfaceInfos[node->index].surfaceType != surface->surfaceType)
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

bool dsView_setCameraMatrix(dsView* view, const dsMatrix44f* camera)
{
	if (!view || !camera)
	{
		errno = EINVAL;
		return false;
	}

	view->cameraMatrix = *camera;
	dsMatrix44_fastInvert(view->viewMatrix, *camera);
	updatedCameraProjection(view);
	return true;
}

bool dsView_setProjectionMatrix(dsView* view, const dsMatrix44f* projection)
{
	if (!view || !projection)
	{
		errno = EINVAL;
		return false;
	}

	view->projectionMatrix = *projection;
	updatedCameraProjection(view);
	return true;
}

bool dsView_setCameraAndProjectionMatrices(dsView* view, const dsMatrix44f* camera,
	const dsMatrix44f* projection)
{
	if (!view || !camera || !projection)
	{
		errno = EINVAL;
		return false;
	}

	view->cameraMatrix = *camera;
	dsMatrix44_fastInvert(view->viewMatrix, *camera);
	view->projectionMatrix = *projection;
	updatedCameraProjection(view);
	return true;
}

bool dsView_update(dsView* view)
{
	DS_PROFILE_FUNC_START();
	if (!view)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsViewPrivate* privateView = (dsViewPrivate*)view;
	dsRenderer* renderer = view->scene->renderer;
	dsResourceManager* resourceManager = renderer->resourceManager;
	bool sizeChanged = privateView->sizeUpdated;
	bool surfaceSet = privateView->surfaceSet;
	bool samplesChanged = privateView->lastSurfaceSamples != renderer->surfaceSamples;
	if (!sizeChanged && !surfaceSet && !samplesChanged)
		DS_PROFILE_FUNC_RETURN(true);

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
					DS_PROFILE_FUNC_RETURN(false);

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
					DS_PROFILE_FUNC_RETURN(false);

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
			DS_PROFILE_FUNC_RETURN(false);

		DS_VERIFY(dsFramebuffer_destroy(privateView->framebuffers[i]));
		privateView->framebuffers[i] = framebuffer;
	}

	privateView->sizeUpdated = false;
	privateView->surfaceSet = false;
	privateView->lastSurfaceSamples = renderer->surfaceSamples;
	DS_PROFILE_FUNC_RETURN(true);
}

bool dsView_draw(dsView* view, dsCommandBuffer* commandBuffer, dsSceneThreadManager* threadManager)
{
	DS_PROFILE_FUNC_START();
	if (!view || !commandBuffer)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsViewPrivate* privateView = (dsViewPrivate*)view;
	const dsScene* scene = view->scene;

	// First setup the global data.
	for (uint32_t i = 0; i < scene->globalDataCount; ++i)
	{
		dsSceneGlobalData* globalData = scene->globalData[i];
		if (!globalData->populateDataFunc(globalData, view, commandBuffer))
			DS_PROFILE_FUNC_RETURN(false);
	}

	if (threadManager)
	{
		bool result = dsSceneThreadManager_draw(threadManager, view, commandBuffer,
			privateView->framebufferInfos, privateView->framebuffers,
			privateView->pipelineFramebuffers);
		DS_PROFILE_FUNC_RETURN(result);
	}

	// Then process the shared items.
	for (uint32_t i = 0; i < scene->sharedItemCount; ++i)
	{
		dsSceneItemLists* sharedItems = scene->sharedItems + i;
		for (uint32_t j = 0; j < sharedItems->count; ++j)
		{
			dsSceneItemList* itemList = sharedItems->itemLists[i];
			itemList->commitFunc(itemList, view, commandBuffer);
		}
	}

	// Then process the scene pipeline.
	for (uint32_t i = 0; i < scene->pipelineCount; ++i)
	{
		dsSceneRenderPass* sceneRenderPass = scene->pipeline[i].renderPass;
		if (sceneRenderPass)
		{
			dsRenderPass* renderPass = sceneRenderPass->renderPass;

			uint32_t framebufferIndex = privateView->pipelineFramebuffers[i];
			const dsViewFramebufferInfo* framebufferInfo =
				privateView->framebufferInfos + framebufferIndex;
			dsFramebuffer* framebuffer = privateView->framebuffers[framebufferIndex];

			dsAlignedBox3f viewport = framebufferInfo->viewport;
			float width = (float)framebuffer->width;
			if (width < 0)
				width *= (float)view->width;
			float height = (float)framebuffer->height;
			if (height < 0)
				height *= (float)view->height;
			viewport.min.x *= width;
			viewport.max.x *= width;
			viewport.min.y *= height;
			viewport.max.y *= height;
			uint32_t clearValueCount =
				sceneRenderPass->clearValues ? sceneRenderPass->renderPass->attachmentCount : 0;
			if (!dsRenderPass_begin(renderPass, commandBuffer, framebuffer, &viewport,
					sceneRenderPass->clearValues, clearValueCount))
			{
				DS_PROFILE_FUNC_RETURN(false);
			}

			for (uint32_t j = 0; j < renderPass->subpassCount; ++j)
			{
				dsSceneItemLists* drawLists = sceneRenderPass->drawLists + j;
				for (uint32_t k = 0; k < drawLists->count; ++k)
				{
					dsSceneItemList* itemList = drawLists->itemLists[k];
					itemList->commitFunc(itemList, view, commandBuffer);
				}

				if (j != renderPass->subpassCount - 1)
					DS_VERIFY(dsRenderPass_nextSubpass(renderPass, commandBuffer));
			}

			DS_VERIFY(dsRenderPass_end(renderPass, commandBuffer));
		}
		else
		{
			dsSceneItemList* computeItems = scene->pipeline[i].computeItems;
			DS_ASSERT(computeItems);
			computeItems->commitFunc(computeItems, view, commandBuffer);
		}
	}

	// Cleanup global data.
	for (uint32_t i = 0; i < scene->globalDataCount; ++i)
	{
		dsSceneGlobalData* globalData = scene->globalData[i];
		if (globalData->finishFunc)
			globalData->finishFunc(globalData);
	}

	DS_PROFILE_FUNC_RETURN(true);
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

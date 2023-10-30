/*
 * Copyright 2019-2023 Aaron Barany
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
#include "ViewInternal.h"

#include "SceneThreadManagerInternal.h"
#include "SceneTypes.h"

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Streams/Stream.h>
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
#include <DeepSea/Render/ProjectionParams.h>
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Render/RenderPass.h>

#include <DeepSea/Scene/SceneLoadScratchData.h>

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
	dsRotatedFramebuffer* framebuffers;
	uint32_t* pipelineFramebuffers;
	uint32_t surfaceCount;
	uint32_t framebufferCount;

	dsHashTable* surfaceTable;
	dsFramebufferSurface* tempSurfaces;

	uint32_t lastSurfaceSamples;
	uint32_t lastDefaultSamples;
	bool sizeUpdated;
	bool surfaceSet;
} dsViewPrivate;

static size_t fullAllocSize(uint32_t* outOffscreenSurfaceCount, const dsScene* scene,
	const dsViewSurfaceInfo* surfaces, uint32_t surfaceCount,
	const dsViewFramebufferInfo* framebuffers, uint32_t framebufferCount)
{
	*outOffscreenSurfaceCount = 0;
	for (uint32_t i = 0; i < surfaceCount; ++i)
	{
		if (surfaces[i].surfaceType == dsGfxSurfaceType_Offscreen)
			++*outOffscreenSurfaceCount;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsViewPrivate)) +
		dsSharedMaterialValues_fullAllocSize(scene->globalValueCount + *outOffscreenSurfaceCount) +
		DS_ALIGNED_SIZE(sizeof(dsViewSurfaceInfo)*surfaceCount) +
		DS_ALIGNED_SIZE(sizeof(void*)*surfaceCount) +
		DS_ALIGNED_SIZE(sizeof(IndexNode)*surfaceCount) +
		dsHashTable_fullAllocSize(dsHashTable_tableSize(surfaceCount)) +
		DS_ALIGNED_SIZE(sizeof(dsViewFramebufferInfo)*framebufferCount) +
		DS_ALIGNED_SIZE(sizeof(dsRotatedFramebuffer)*framebufferCount) +
		DS_ALIGNED_SIZE(sizeof(uint32_t)*scene->pipelineCount);

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
		{
			if (surface->surfaceType >= dsGfxSurfaceType_ColorRenderSurface &&
				surface->surfaceType <= dsGfxSurfaceType_DepthRenderSurfaceRight)
			{
				dsRenderSurface* renderSurface = (dsRenderSurface*)surface->surface;
				if ((renderSurface->usage & dsRenderSurfaceUsage_ClientRotations) &&
					!surface->windowFramebuffer)
				{
					errno = EINVAL;
					DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Window surface '%s' has client rotations "
						"enabled, but does not have windowFramebuffer set to true.", surface->name);
					return false;
				}
			}
			continue;
		}

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
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
				"Format not supported for offscreens or renderbuffers for surface '%s'.",
				surface->name);
			return false;
		}

		if (surface->createInfo.width == 0 && surface->widthRatio <= 0.0f)
		{
			errno = EINVAL;
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Invalid width for surface '%s'.", surface->name);
			return false;
		}

		if (surface->createInfo.height == 0 && surface->heightRatio <= 0.0f)
		{
			errno = EINVAL;
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Invalid height for surface '%s'.", surface->name);
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

static bool isLayerInRange(const dsFramebufferSurface* surface, uint32_t layers)
{
	switch (surface->surfaceType)
	{
		case dsGfxSurfaceType_ColorRenderSurface:
		case dsGfxSurfaceType_ColorRenderSurfaceLeft:
		case dsGfxSurfaceType_ColorRenderSurfaceRight:
		case dsGfxSurfaceType_DepthRenderSurface:
		case dsGfxSurfaceType_DepthRenderSurfaceLeft:
		case dsGfxSurfaceType_DepthRenderSurfaceRight:
		case dsGfxSurfaceType_Renderbuffer:
			return layers == 1;
		case dsGfxSurfaceType_Offscreen:
		{
			dsOffscreen* offscreen = (dsOffscreen*)surface->surface;
			uint32_t surfaceLayers = dsMax(1U, offscreen->info.depth);
			uint32_t layer = surface->layer;
			if (offscreen->info.dimension == dsTextureDim_Cube)
			{
				layer = layer*6 + surface->cubeFace;
				layers *= 6;
				surfaceLayers *= 6;
			}

			return DS_IS_BUFFER_RANGE_VALID(layer, layers, surfaceLayers);
		}
	}

	DS_ASSERT(false);
	return false;
}

static void destroyMidCreate(dsView* view)
{
	dsSharedMaterialValues_destroy(view->globalValues);
	if (view->destroyUserDataFunc)
		view->destroyUserDataFunc(view->userData);
	if (view->allocator)
		DS_VERIFY(dsAllocator_free(view->allocator, view));
}

static void updatePreRotatedDimensions(dsView* view)
{
	switch (view->rotation)
	{
		case dsRenderSurfaceRotation_90:
		case dsRenderSurfaceRotation_270:
			view->preRotateWidth = view->height;
			view->preRotateHeight = view->width;
			break;
		default:
			view->preRotateWidth = view->width;
			view->preRotateHeight = view->height;
			break;
	}
}

static void updatedCameraProjection(dsView* view)
{
	dsRenderer* renderer = view->scene->renderer;
	DS_VERIFY(dsProjectionParams_createMatrix(&view->projectionMatrix, &view->projectionParams,
		renderer));
	dsMatrix44f_mul(&view->viewProjectionMatrix, &view->projectionMatrix, &view->viewMatrix);
	DS_VERIFY(dsRenderer_frustumFromMatrix(&view->viewFrustum, renderer,
		&view->viewProjectionMatrix));
}

static bool bindOffscreenVariables(dsView* view)
{
	dsViewPrivate* viewPrivate = (dsViewPrivate*)view;
	for (uint32_t i = 0; i < viewPrivate->surfaceCount; ++i)
	{
		const dsViewSurfaceInfo* surfaceInfo = viewPrivate->surfaceInfos + i;
		if (surfaceInfo->surfaceType == dsGfxSurfaceType_Offscreen &&
			!dsSharedMaterialValues_setTextureName(view->globalValues, surfaceInfo->name,
				(dsTexture*)viewPrivate->surfaces[i]))
		{
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Couldn't bind view offscreen '%s'.",
				surfaceInfo->name);
			return false;
		}
	}

	return true;
}

dsView* dsView_loadImpl(const dsScene* scene, dsAllocator* allocator,
	dsAllocator* resourceAllocator, dsSceneLoadScratchData* scratchData, const void* data,
	size_t dataSize,  const dsViewSurfaceInfo* surfaces, uint32_t surfaceCount, uint32_t width,
	uint32_t height, dsRenderSurfaceRotation rotation, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc, const char* fileName);

dsView* dsView_create(const dsScene* scene, dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsViewSurfaceInfo* surfaces, uint32_t surfaceCount,
	const dsViewFramebufferInfo* framebuffers, uint32_t framebufferCount, uint32_t width,
	uint32_t height, dsRenderSurfaceRotation rotation, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc)
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
	if (!resourceAllocator)
		resourceAllocator = allocator;

	if (!resourceAllocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "View resource allocator must support freeing memory.");
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

	uint32_t offscreenSurfaceCount;
	size_t fullSize = fullAllocSize(&offscreenSurfaceCount, scene, surfaces, surfaceCount,
		framebuffers, framebufferCount);
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
	view->resourceAllocator = dsAllocator_keepPointer(resourceAllocator);
	view->userData = userData;
	view->destroyUserDataFunc = destroyUserDataFunc;
	view->width = width;
	view->height = height;
	view->rotation = rotation;
	updatePreRotatedDimensions(view);
	dsMatrix44_identity(view->cameraMatrix);
	dsMatrix44_identity(view->viewMatrix);
	dsView_setOrthoProjection(view, -1, 1, -1, 1, -1, 1);
	view->lodBias = 1.0f;

	uint32_t variableCount = scene->globalValueCount + offscreenSurfaceCount;
	if (variableCount > 0)
	{
		view->globalValues = dsSharedMaterialValues_create((dsAllocator*)&bufferAlloc,
			variableCount);
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

	uint32_t surfaceTableSize = dsHashTable_tableSize(surfaceCount);
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

	privateView->framebuffers = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsRotatedFramebuffer,
		framebufferCount);
	DS_ASSERT(privateView->framebuffers);
	memset(privateView->framebuffers, 0, sizeof(dsRotatedFramebuffer)*framebufferCount);
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
		bool rotated = false;
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

			dsViewSurfaceInfo* surfaceInfo = privateView->surfaceInfos + node->index;
			if (surface->surfaceType == (dsGfxSurfaceType)-1)
				surface->surfaceType = surfaceInfo->surfaceType;
			else if (surfaceInfo->surfaceType != surface->surfaceType)
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

			if (j == 0)
				rotated = surfaceInfo->windowFramebuffer;
			else if (surfaceInfo->windowFramebuffer != rotated)
			{
				errno = EINVAL;
				DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Framebuffer '%s' cannot contain surfaces both "
					"with and without windowFramebuffer set.", framebufferInfo->name);
				destroyMidCreate(view);
				return NULL;
			}
		}

		privateView->framebuffers[i].rotated = rotated;
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
			if (strcmp(renderPass->framebuffer, framebuffers[j].name) == 0)
			{
				privateView->pipelineFramebuffers[i] = j;
				found = true;
				break;
			}
		}

		if (!found)
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG,
				"Framebuffer '%s' requested from scene's pipeline not in the view.",
				renderPass->framebuffer);
			destroyMidCreate(view);
			return NULL;
		}
	}

	privateView->lastSurfaceSamples = renderer->surfaceSamples;
	privateView->lastDefaultSamples = renderer->defaultSamples;
	privateView->sizeUpdated = true;
	privateView->surfaceSet = true;

	return view;
}

dsView* dsView_loadFile(const dsScene* scene, dsAllocator* allocator,
	dsAllocator* resourceAllocator, dsSceneLoadScratchData* scratchData,
	const dsViewSurfaceInfo* surfaces, uint32_t surfaceCount, uint32_t width, uint32_t height,
	dsRenderSurfaceRotation rotation, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc, const char* filePath)
{
	DS_PROFILE_FUNC_START();

	if (!scene || !scratchData || !filePath || (!surfaces && surfaceCount > 0))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	dsFileStream stream;
	if (!dsFileStream_openPath(&stream, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open view file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsSceneLoadScratchData_readUntilEnd(&size, scratchData, (dsStream*)&stream);
	dsFileStream_close(&stream);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsView* view = dsView_loadImpl(scene, allocator, resourceAllocator, scratchData, buffer, size,
		surfaces, surfaceCount, width, height, rotation, userData, destroyUserDataFunc, filePath);
	DS_VERIFY(dsSceneLoadScratchData_freeReadBuffer(scratchData, buffer));
	DS_PROFILE_FUNC_RETURN(view);
}

dsView* dsView_loadResource(const dsScene* scene, dsAllocator* allocator,
	dsAllocator* resourceAllocator, dsSceneLoadScratchData* scratchData,
	const dsViewSurfaceInfo* surfaces, uint32_t surfaceCount, uint32_t width, uint32_t height,
	dsRenderSurfaceRotation rotation, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc, dsFileResourceType type,
	const char* filePath)
{
	DS_PROFILE_FUNC_START();

	if (!scene || !scratchData || !filePath || (!surfaces && surfaceCount > 0))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsResourceStream stream;
	if (!dsResourceStream_open(&stream, type, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open view file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsSceneLoadScratchData_readUntilEnd(&size, scratchData, (dsStream*)&stream);
	dsResourceStream_close(&stream);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsView* view = dsView_loadImpl(scene, allocator, resourceAllocator, scratchData, buffer, size,
		surfaces, surfaceCount, width, height, rotation, userData, destroyUserDataFunc, filePath);
	DS_VERIFY(dsSceneLoadScratchData_freeReadBuffer(scratchData, buffer));
	DS_PROFILE_FUNC_RETURN(view);
}

dsView* dsView_loadStream(const dsScene* scene, dsAllocator* allocator,
	dsAllocator* resourceAllocator, dsSceneLoadScratchData* scratchData,
	const dsViewSurfaceInfo* surfaces, uint32_t surfaceCount, uint32_t width, uint32_t height,
	dsRenderSurfaceRotation rotation, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc, dsStream* stream)
{
	DS_PROFILE_FUNC_START();

	if (!scene || !scratchData || !stream || (!surfaces && surfaceCount > 0))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	size_t size;
	void* buffer = dsSceneLoadScratchData_readUntilEnd(&size, scratchData, stream);
	if (!buffer)
		DS_PROFILE_FUNC_RETURN(NULL);

	dsView* view = dsView_loadImpl(scene, allocator, resourceAllocator, scratchData, buffer, size,
		surfaces, surfaceCount, width, height, rotation, userData, destroyUserDataFunc, NULL);
	DS_VERIFY(dsSceneLoadScratchData_freeReadBuffer(scratchData, buffer));
	DS_PROFILE_FUNC_RETURN(view);
}

dsView* dsView_loadData(const dsScene* scene, dsAllocator* allocator,
	dsAllocator* resourceAllocator, dsSceneLoadScratchData* scratchData,
	const dsViewSurfaceInfo* surfaces, uint32_t surfaceCount, uint32_t width, uint32_t height,
	dsRenderSurfaceRotation rotation, void* userData,
	dsDestroySceneUserDataFunction destroyUserDataFunc, const void* data, size_t size)
{
	DS_PROFILE_FUNC_START();

	if (!scene || !scratchData || !data || size == 0 || (!surfaces && surfaceCount > 0))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsView* view = dsView_loadImpl(scene, allocator, resourceAllocator, scratchData, data, size,
		surfaces, surfaceCount, width, height, rotation, userData, destroyUserDataFunc, NULL);
	DS_PROFILE_FUNC_RETURN(view);
}

bool dsView_setDimensions(dsView* view, uint32_t width, uint32_t height,
	dsRenderSurfaceRotation rotation)
{
	if (!view || width == 0 || height == 0)
	{
		errno = EINVAL;
		return false;
	}

	if (view->width == width && view->height == height && view->rotation == rotation)
		return true;

	dsViewPrivate* privateView = (dsViewPrivate*)view;
	view->width = width;
	view->height = height;
	view->rotation = rotation;
	updatePreRotatedDimensions(view);
	privateView->sizeUpdated = true;

	if (view->projectionParams.type == dsProjectionType_Perspective)
	{
		view->projectionParams.perspectiveParams.aspect = (float)width/(float)height;
		updatedCameraProjection(view);
	}

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

	// NOTE: This check can fail if deleting and re-allocating objects.
	/*if (surfaceInfo->surface == surface)
		return true;*/

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
	dsMatrix44f_fastInvert(&view->viewMatrix, camera);
	updatedCameraProjection(view);
	return true;
}

bool dsView_setOrthoProjection(dsView* view, float left, float right, float bottom, float top,
	float near, float far)
{
	if (!view)
	{
		errno = EINVAL;
		return false;
	}

	if (!dsProjectionParams_makeOrtho(&view->projectionParams, left, right, bottom, top, near, far))
		return false;

	updatedCameraProjection(view);
	return true;
}

bool dsView_setFrustumProjection(dsView* view, float left, float right, float bottom, float top,
	float near, float far)
{
	if (!view)
	{
		errno = EINVAL;
		return false;
	}

	if (!dsProjectionParams_makeFrustum(
			&view->projectionParams, left, right, bottom, top, near, far))
	{
		return false;
	}

	updatedCameraProjection(view);
	return true;
}

bool dsView_setPerspectiveProjection(dsView* view, float fovy, float near, float far)
{
	if (!view)
	{
		errno = EINVAL;
		return false;
	}

	float aspect = (float)view->width/(float)view->height;
	if (!dsProjectionParams_makePerspective(&view->projectionParams, fovy, aspect, near, far))
		return false;

	updatedCameraProjection(view);
	return true;
}

bool dsView_setProjectionParams(dsView* view, const dsProjectionParams* params)
{
	if (!view || !params)
	{
		errno = EINVAL;
		return false;
	}

	view->projectionParams = *params;
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
	bool surfaceSamplesChanged = privateView->lastSurfaceSamples != renderer->surfaceSamples;
	bool defaultSamplesChanged = privateView->lastDefaultSamples != renderer->defaultSamples;
	if (!sizeChanged && !surfaceSet && !surfaceSamplesChanged && !defaultSamplesChanged)
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
			(surfaceInfo->createInfo.samples != DS_SURFACE_ANTIALIAS_SAMPLES ||
				!surfaceSamplesChanged) &&
			(surfaceInfo->createInfo.samples != DS_DEFAULT_ANTIALIAS_SAMPLES ||
				!defaultSamplesChanged))
		{
			continue;
		}

		uint32_t width;
		if (surfaceInfo->createInfo.width > 0)
			width = surfaceInfo->createInfo.width;
		else
		{
			width = surfaceInfo->windowFramebuffer ? view->preRotateWidth : view->width;
			width = (uint32_t)roundf(surfaceInfo->widthRatio*(float)width);
		}

		uint32_t height;
		if (surfaceInfo->createInfo.height > 0)
			height = surfaceInfo->createInfo.height;
		else
		{
			height = surfaceInfo->windowFramebuffer ? view->preRotateHeight : view->height;
			height = (uint32_t)roundf(surfaceInfo->heightRatio*(float)height);
		}

		switch (surfaceInfo->surfaceType)
		{
			case dsGfxSurfaceType_Offscreen:
			{
				dsTextureInfo textureInfo = surfaceInfo->createInfo;
				textureInfo.width = width;
				textureInfo.height = height;
				dsOffscreen* offscreen = dsTexture_createOffscreen(resourceManager,
					view->resourceAllocator, surfaceInfo->usage, surfaceInfo->memoryHints,
					&textureInfo, surfaceInfo->resolve);
				if (!offscreen)
					DS_PROFILE_FUNC_RETURN(false);

				DS_VERIFY(dsTexture_destroy((dsTexture*)privateView->surfaces[i]));
				privateView->surfaces[i] = offscreen;
				break;
			}
			case dsGfxSurfaceType_Renderbuffer:
			{
				dsRenderbuffer* renderbuffer = dsRenderbuffer_create(resourceManager,
					view->resourceAllocator, surfaceInfo->usage, surfaceInfo->createInfo.format,
					width, height, surfaceInfo->createInfo.samples);
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
	}

	// Re-create all framebuffers to avoid complicated logic to decide which ones specifically need
	// to change.
	for (uint32_t i = 0; i < privateView->framebufferCount; ++i)
	{
		const dsViewFramebufferInfo* framebufferInfo = privateView->framebufferInfos + i;

		bool rotated = false;
		bool outOfRange = false;
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
			DS_ASSERT(j == 0 || rotated == privateView->surfaceInfos->windowFramebuffer);
			rotated = privateView->surfaceInfos->windowFramebuffer;

			if (!isLayerInRange(surface, framebufferInfo->layers))
				outOfRange = true;
		}

		uint32_t width;
		if (framebufferInfo->width > 0)
			width = (uint32_t)roundf(framebufferInfo->width);
		else
		{
			width = rotated ? view->preRotateWidth : view->width;
			width = (uint32_t)roundf(-framebufferInfo->width*(float)width);
		}

		uint32_t height;
		if (framebufferInfo->height > 0)
			height = (uint32_t)roundf(framebufferInfo->height);
		else
		{
			height = rotated ? view->preRotateHeight : view->height;
			height = (uint32_t)roundf(-framebufferInfo->height*(float)height);
		}

		dsFramebuffer* framebuffer = NULL;
		if (outOfRange)
		{
			DS_LOG_WARNING_F(DS_SCENE_LOG_TAG, "Ignoring framebuffer %s with layers out of range.",
				framebufferInfo->name);
		}
		else
		{
			framebuffer = dsFramebuffer_create(resourceManager,
				view->resourceAllocator, framebufferInfo->name, privateView->tempSurfaces,
				framebufferInfo->surfaceCount, width, height, framebufferInfo->layers);
			if (!framebuffer)
				DS_PROFILE_FUNC_RETURN(false);
		}

		DS_VERIFY(dsFramebuffer_destroy(privateView->framebuffers[i].framebuffer));
		privateView->framebuffers[i].framebuffer = framebuffer;
	}

	privateView->sizeUpdated = false;
	privateView->surfaceSet = false;
	privateView->lastSurfaceSamples = renderer->surfaceSamples;
	privateView->lastDefaultSamples = renderer->defaultSamples;
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
	if (!bindOffscreenVariables(view))
		DS_PROFILE_FUNC_RETURN(false);

	if (threadManager)
	{
		bool result = dsSceneThreadManager_draw(threadManager, view, commandBuffer,
			privateView->framebufferInfos, privateView->framebuffers,
			privateView->pipelineFramebuffers);
		DS_PROFILE_FUNC_RETURN(result);
	}

	// Then process the shared items.
	DS_PROFILE_SCOPE_START("Shared Items");
	for (uint32_t i = 0; i < scene->sharedItemCount; ++i)
	{
		dsSceneItemLists* sharedItems = scene->sharedItems + i;
		for (uint32_t j = 0; j < sharedItems->count; ++j)
		{
			dsSceneItemList* itemList = sharedItems->itemLists[j];
			if (itemList->commitFunc)
			{
				DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);
				itemList->commitFunc(itemList, view, commandBuffer);
				DS_PROFILE_SCOPE_END();
			}
		}
	}
	DS_PROFILE_SCOPE_END();

	// Then process the scene pipeline.
	DS_PROFILE_SCOPE_START("Draw");
	for (uint32_t i = 0; i < scene->pipelineCount; ++i)
	{
		dsSceneRenderPass* sceneRenderPass = scene->pipeline[i].renderPass;
		if (sceneRenderPass)
		{
			dsRenderPass* renderPass = sceneRenderPass->renderPass;

			uint32_t framebufferIndex = privateView->pipelineFramebuffers[i];
			const dsViewFramebufferInfo* framebufferInfo =
				privateView->framebufferInfos + framebufferIndex;
			const dsRotatedFramebuffer* framebuffer = privateView->framebuffers + framebufferIndex;

			// Skipped due to framebuffer out of range. (e.g. support up to N layers, but have fewer
			// in the currently bound offscreen)
			if (!framebuffer->framebuffer)
				continue;

			// Execute any actions needed outside of the render pass.
			for (uint32_t j = 0; j < renderPass->subpassCount; ++j)
			{
				dsSceneItemLists* drawLists = sceneRenderPass->drawLists + j;
				for (uint32_t k = 0; k < drawLists->count; ++k)
				{
					dsSceneItemList* itemList = drawLists->itemLists[k];
					if (itemList->preRenderPassFunc)
					{
						DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);
						itemList->preRenderPassFunc(itemList, view, commandBuffer);
						DS_PROFILE_SCOPE_END();
					}
				}
			}

			dsAlignedBox3f viewport = framebufferInfo->viewport;
			dsView_adjustViewport(&viewport, view, framebuffer->rotated);
			float width = (float)framebuffer->framebuffer->width;
			if (width < 0)
				width *= (float)view->width;
			float height = (float)framebuffer->framebuffer->height;
			if (height < 0)
				height *= (float)view->height;
			viewport.min.x *= width;
			viewport.max.x *= width;
			viewport.min.y *= height;
			viewport.max.y *= height;
			uint32_t clearValueCount =
				sceneRenderPass->clearValues ? sceneRenderPass->renderPass->attachmentCount : 0;
			if (!dsRenderPass_begin(renderPass, commandBuffer, framebuffer->framebuffer, &viewport,
					sceneRenderPass->clearValues, clearValueCount, false))
			{
				DS_PROFILE_FUNC_RETURN(false);
			}

			for (uint32_t j = 0; j < renderPass->subpassCount; ++j)
			{
				dsSceneItemLists* drawLists = sceneRenderPass->drawLists + j;
				for (uint32_t k = 0; k < drawLists->count; ++k)
				{
					dsSceneItemList* itemList = drawLists->itemLists[k];
					DS_ASSERT(itemList->commitFunc);
					DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);
					itemList->commitFunc(itemList, view, commandBuffer);
					DS_PROFILE_SCOPE_END();
				}

				if (j != renderPass->subpassCount - 1)
					DS_VERIFY(dsRenderPass_nextSubpass(renderPass, commandBuffer, false));
			}

			DS_VERIFY(dsRenderPass_end(renderPass, commandBuffer));
		}
		else
		{
			dsSceneItemList* computeItems = scene->pipeline[i].computeItems;
			DS_ASSERT(computeItems);
			if (computeItems->commitFunc)
				computeItems->commitFunc(computeItems, view, commandBuffer);
		}
	}
	DS_PROFILE_SCOPE_END();

	DS_PROFILE_FUNC_RETURN(true);
}

bool dsView_destroy(dsView* view)
{
	if (!view)
		return true;

	dsViewPrivate* privateView = (dsViewPrivate*)view;
	for (uint32_t i = 0; i < privateView->framebufferCount; ++i)
	{
		if (!dsFramebuffer_destroy(privateView->framebuffers[i].framebuffer))
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

void dsView_adjustViewport(dsAlignedBox3f* viewport, const dsView* view, bool rotated)
{
	if (!rotated)
		return;

	switch (view->rotation)
	{
		case dsRenderSurfaceRotation_0:
			break;
		case dsRenderSurfaceRotation_90:
		{
			float tempX = viewport->min.x;
			float tempY = viewport->min.y;
			viewport->min.x = 1.0f - viewport->max.y;
			viewport->min.y = tempX;
			tempX = viewport->max.x;
			viewport->max.x = 1.0f - tempY;
			viewport->max.y = tempX;
			break;
		}
		case dsRenderSurfaceRotation_180:
		{
			float tempX = viewport->min.x;
			float tempY = viewport->min.y;
			viewport->min.x = 1.0f - viewport->max.x;
			viewport->min.y = 1.0f - viewport->max.y;
			viewport->max.x = 1.0f - tempX;
			viewport->max.y = 1.0f - tempY;
			break;
		}
		case dsRenderSurfaceRotation_270:
		{
			float tempX = viewport->min.x;
			float tempY = viewport->min.y;
			viewport->min.x = tempY;
			viewport->min.y = 1.0f - viewport->max.x;
			tempY = viewport->max.y;
			viewport->max.x = tempY;
			viewport->max.y = 1.0f - tempX;
			break;
		}
	}
}

/*
 * Copyright 2020-2025 Aaron Barany
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

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/Scene.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/View_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

#include <string.h>

#define PRINT_FLATBUFFER_ERROR(message, name) \
	do \
	{ \
		if (name) \
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, message " for '%s'.", name); \
		else \
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, message "."); \
	} while (false)

template <typename T>
using FlatbufferVector = flatbuffers::Vector<flatbuffers::Offset<T>>;

static size_t getTempSize(const FlatbufferVector<DeepSeaScene::Surface>* fbSurfaces,
	uint32_t prevSurfaceCount, const FlatbufferVector<DeepSeaScene::Framebuffer>& fbFramebuffers,
	const char* fileName)
{
	uint32_t totalSurfaceCount = prevSurfaceCount;
	if (fbSurfaces)
		totalSurfaceCount += fbSurfaces->size();
	if (totalSurfaceCount == 0)
	{
		PRINT_FLATBUFFER_ERROR("View contains no surfaces", fileName);
		return 0;
	}

	size_t tempSize = DS_ALIGNED_SIZE(totalSurfaceCount*sizeof(dsViewSurfaceInfo));

	uint32_t framebufferCount = fbFramebuffers.size();
	if (framebufferCount == 0)
	{
		PRINT_FLATBUFFER_ERROR("View framebuffer array is empty", fileName);
		return 0;
	}
	tempSize += DS_ALIGNED_SIZE(framebufferCount*sizeof(dsViewFramebufferInfo));

	for (auto fbFramebuffer : fbFramebuffers)
	{
		if (!fbFramebuffer)
		{
			PRINT_FLATBUFFER_ERROR("View framebuffer is null", fileName);
			return 0;
		}

		auto fbFramebufferSurfaces = fbFramebuffer->surfaces();
		if (fbFramebufferSurfaces && fbFramebufferSurfaces->size() > 0)
			tempSize += DS_ALIGNED_SIZE(fbFramebufferSurfaces->size()*sizeof(dsFramebufferSurface));
	}

	return tempSize;
}

extern "C"
dsView* dsView_loadImpl(dsAllocator* allocator, const char* name, const dsScene* scene,
	dsAllocator* resourceAllocator, dsSceneLoadScratchData* scratchData, const void* data,
	size_t dataSize,  const dsViewSurfaceInfo* surfaces, uint32_t surfaceCount, uint32_t width,
	uint32_t height, dsRenderSurfaceRotation rotation, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc, const char* fileName)
{
	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), dataSize);
	if (!DeepSeaScene::VerifyViewBuffer(verifier))
	{
		errno = EFORMAT;
		PRINT_FLATBUFFER_ERROR("Invalid view flatbuffer format", fileName);
		return nullptr;
	}

	auto fbView = DeepSeaScene::GetView(data);
	auto fbSurfaces = fbView->surfaces();
	auto fbFramebuffers = fbView->framebuffers();

	uint32_t fileSurfaceCount = 0;
	dsViewSurfaceInfo* fileSurfaces = nullptr;
	uint32_t allSurfaceCount = surfaceCount;
	dsViewSurfaceInfo* allSurfaces = nullptr;

	uint32_t framebufferCount = 0;
	dsViewFramebufferInfo* framebuffers = nullptr;

	dsAllocator* scratchAllocator = dsSceneLoadScratchData_getAllocator(scratchData);
	DS_ASSERT(scratchAllocator);
	size_t tempSize = getTempSize(fbSurfaces, surfaceCount, *fbFramebuffers, fileName);
	if (tempSize == 0)
	{
		errno = EFORMAT;
		return nullptr;
	}

	void* tempBuffer = dsAllocator_alloc(scratchAllocator, tempSize);
	if (!tempBuffer)
		return nullptr;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, tempBuffer, tempSize));

	dsView* view = nullptr;
	if (fbSurfaces)
	{
		fileSurfaceCount = fbSurfaces->size();
		allSurfaceCount += fileSurfaceCount;
	}

	allSurfaces = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsViewSurfaceInfo, allSurfaceCount);
	DS_VERIFY(allSurfaces);
	memcpy(allSurfaces, surfaces, surfaceCount*sizeof(dsViewSurfaceInfo));
	fileSurfaces = allSurfaces + surfaceCount;
	for (uint32_t i = 0; i < fileSurfaceCount; ++i)
	{
		auto fbSurface = (*fbSurfaces)[i];
		if (!fbSurface)
		{
			errno = EFORMAT;
			PRINT_FLATBUFFER_ERROR("View surface is null", fileName);
			goto finished;
		}

		dsViewSurfaceInfo* surface = fileSurfaces + i;
		surface->name = fbSurface->name()->c_str();
		switch (fbSurface->type())
		{
			case DeepSeaScene::SurfaceType::Renderbuffer:
				surface->surfaceType = dsGfxSurfaceType_Renderbuffer;
				break;
			case DeepSeaScene::SurfaceType::Offscreen:
				surface->surfaceType = dsGfxSurfaceType_Offscreen;
				break;
			default:
				errno = EFORMAT;
				PRINT_FLATBUFFER_ERROR("Invalid view surface type", fileName);
				goto finished;
		}
		surface->usage = fbSurface->usage();
		surface->memoryHints = static_cast<dsGfxMemory>(fbSurface->memoryHints());
		surface->createInfo.format = DeepSeaScene::convert(
			dsScene_getRenderer(scene), fbSurface->format(), fbSurface->decoration());
		surface->createInfo.dimension = DeepSeaScene::convert(fbSurface->dimension());
		surface->createInfo.width = fbSurface->width();
		surface->widthRatio = fbSurface->widthRatio();
		surface->createInfo.height = fbSurface->height();
		surface->heightRatio = fbSurface->heightRatio();
		surface->createInfo.depth = fbSurface->depth();
		surface->createInfo.mipLevels = fbSurface->mipLevels();
		surface->createInfo.samples = fbSurface->samples();
		surface->resolve = fbSurface->resolve();
		surface->windowFramebuffer = fbSurface->windowFramebuffer();
		surface->surface = nullptr;
	}

	framebufferCount = fbFramebuffers->size();
	framebuffers = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsViewFramebufferInfo, framebufferCount);
	for (uint32_t i = 0; i < framebufferCount; ++i)
	{
		auto fbFramebuffer = (*fbFramebuffers)[i];
		DS_ASSERT(fbFramebuffer);
		dsViewFramebufferInfo* framebuffer = framebuffers + i;

		framebuffer->name = fbFramebuffer->name()->c_str();
		auto fbFramebufferSurfaces = fbFramebuffer->surfaces();
		if (fbFramebufferSurfaces && fbFramebufferSurfaces->size() > 0)
		{
			framebuffer->surfaceCount = fbFramebufferSurfaces->size();
			framebuffer->surfaces = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsFramebufferSurface,
				framebuffer->surfaceCount);
			DS_ASSERT(framebuffer->surfaces);
			for (uint32_t j = 0; j < framebuffer->surfaceCount; ++j)
			{
				auto fbSurface = (*fbFramebufferSurfaces)[j];
				if (!fbSurface)
				{
					errno = EFORMAT;
					PRINT_FLATBUFFER_ERROR("View framebuffer surface is null", fileName);
					goto finished;
				}

				auto surface = const_cast<dsFramebufferSurface*>(framebuffer->surfaces + j);

#if DS_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif
				surface->surfaceType = static_cast<dsGfxSurfaceType>(-1);
#if DS_GCC
#pragma GCC diagnostic pop
#endif

				surface->surface = const_cast<char*>(fbSurface->name()->c_str());
				surface->cubeFace = DeepSeaScene::convert(fbSurface->face());
				surface->layer = fbSurface->layer();
				surface->mipLevel = fbSurface->mipLevel();
			}
		}
		else
		{
			framebuffer->surfaces = nullptr;
			framebuffer->surfaceCount = 0;
		}
		framebuffer->width = fbFramebuffer->width();
		framebuffer->height = fbFramebuffer->height();
		framebuffer->layers = fbFramebuffer->layers();

		auto fbViewport =  fbFramebuffer->viewport();
		if (fbViewport)
			framebuffer->viewport = DeepSeaScene::convert(*fbViewport);
		else
		{
			framebuffer->viewport.min.x = 0.0f;
			framebuffer->viewport.min.y = 0.0f;
			framebuffer->viewport.min.z = 0.0f;
			framebuffer->viewport.max.x = 1.0f;
			framebuffer->viewport.max.y = 1.0f;
			framebuffer->viewport.max.z = 1.0f;
		}
	}

	view = dsView_create(allocator, name, scene, resourceAllocator, allSurfaces, allSurfaceCount,
		framebuffers, framebufferCount, width, height, rotation, userData, destroyUserDataFunc);

finished:
	DS_VERIFY(dsAllocator_free(scratchAllocator, tempBuffer));
	return view;
}

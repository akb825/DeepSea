/*
 * Copyright 2020-2024 Aaron Barany
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

#include "SceneVectorItemListLoad.h"

#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/ItemLists/SceneInstanceData.h>
#include <DeepSea/Scene/SceneLoadContext.h>

#include <DeepSea/SceneVectorDraw/SceneVectorItemList.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/SceneVectorItemList_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

dsSceneItemList* dsSceneVectorItemList_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void*, const char* name, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaSceneVectorDraw::VerifyVectorItemListBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_VECTOR_DRAW_LOG_TAG, "Invalid vector item list flatbuffer format.");
		return nullptr;
	}

	auto fbVectorList = DeepSeaSceneVectorDraw::GetVectorItemList(data);
	auto fbInstanceData = fbVectorList->instanceData();
	auto fbDynamicRenderStates = fbVectorList->dynamicRenderStates();
	auto fbViews = fbVectorList->views();

	dsResourceManager* resourceManager =
		dsSceneLoadContext_getRenderer(loadContext)->resourceManager;
	uint32_t instanceDataCount = 0;
	dsSceneInstanceData** instanceData = nullptr;
	dsDynamicRenderStates dynamicRenderStates;
	uint32_t viewCount = 0;
	const char** views = nullptr;

	if (fbInstanceData && fbInstanceData->size() > 0)
	{
		instanceDataCount = fbInstanceData->size();
		instanceData = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsSceneInstanceData*, instanceDataCount);
		DS_ASSERT(instanceData);

		for (uint32_t i = 0; i < instanceDataCount; ++i)
		{
			auto fbInstance = (*fbInstanceData)[i];
			dsSceneInstanceData* instance = nullptr;
			if (fbInstance)
			{
				auto fbData = fbInstance->data();
				instance = dsSceneInstanceData_load(allocator, resourceAllocator, loadContext,
					scratchData, fbInstance->type()->c_str(), fbData->data(), fbData->size());
			}
			else
			{
				errno = EFORMAT;
				DS_LOG_ERROR(DS_SCENE_VECTOR_DRAW_LOG_TAG,
					"Vector item list instance data is null.");
			}

			if (!instance)
			{
				// Only clean up the instances loaded so far.
				instanceDataCount = i;
				goto error;
			}

			instanceData[i] = instance;
		}
	}

	if (fbDynamicRenderStates)
	{
		dynamicRenderStates.lineWidth = fbDynamicRenderStates->lineWidth();
		dynamicRenderStates.depthBiasConstantFactor =
			fbDynamicRenderStates->depthBiasConstantFactor();
		dynamicRenderStates.depthBiasClamp = fbDynamicRenderStates->depthBiasClamp();
		dynamicRenderStates.depthBiasSlopeFactor = fbDynamicRenderStates->depthBiasSlopeFactor();

		auto fbBlendConstants = fbDynamicRenderStates->blendConstants();
		if (fbBlendConstants)
		{
			dynamicRenderStates.blendConstants.r = fbBlendConstants->red();
			dynamicRenderStates.blendConstants.g = fbBlendConstants->green();
			dynamicRenderStates.blendConstants.b = fbBlendConstants->blue();
			dynamicRenderStates.blendConstants.a = fbBlendConstants->alpha();
		}
		else
		{
			dynamicRenderStates.blendConstants.r = 0.0f;
			dynamicRenderStates.blendConstants.g = 0.0f;
			dynamicRenderStates.blendConstants.b = 0.0f;
			dynamicRenderStates.blendConstants.a = 1.0f;
		}

		auto fbDepthBounds = fbDynamicRenderStates->depthBounds();
		if (fbDepthBounds)
		{
			dynamicRenderStates.depthBounds.x = fbDepthBounds->x();
			dynamicRenderStates.depthBounds.y = fbDepthBounds->y();
		}
		else
		{
			dynamicRenderStates.depthBounds.x = 0.0f;
			dynamicRenderStates.depthBounds.y = 1.0f;
		}

		dynamicRenderStates.frontStencilCompareMask =
			fbDynamicRenderStates->frontStencilCompareMask();
		dynamicRenderStates.backStencilCompareMask =
			fbDynamicRenderStates->backStencilCompareMask();
		dynamicRenderStates.frontStencilWriteMask =
			fbDynamicRenderStates->frontStencilWriteMask();
		dynamicRenderStates.backStencilWriteMask = fbDynamicRenderStates->backStencilWriteMask();
		dynamicRenderStates.frontStencilReference = fbDynamicRenderStates->frontStencilReference();
		dynamicRenderStates.backStencilReference = fbDynamicRenderStates->backStencilReference();
	}

	if (fbViews && fbViews->size() > 0)
	{
		viewCount = fbViews->size();
		views = DS_ALLOCATE_STACK_OBJECT_ARRAY(const char*, viewCount);
		for (uint32_t i = 0; i < viewCount; ++i)
		{
			auto fbView = (*fbViews)[i];
			if (!fbView)
			{
				errno = EFORMAT;
				DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Vector item list view name is null.");
				goto error;
			}

			views[i] = fbView->c_str();
		}
	}

	return reinterpret_cast<dsSceneItemList*>(dsSceneVectorItemList_create(allocator, name,
		resourceManager, instanceData, instanceDataCount,
		fbDynamicRenderStates ? &dynamicRenderStates : nullptr, views, viewCount));

error:
	// instanceDataCount should be the number that we need to clean up.
	for (uint32_t i = 0; i < instanceDataCount; ++i)
		dsSceneInstanceData_destroy(instanceData[i]);
	return nullptr;
}

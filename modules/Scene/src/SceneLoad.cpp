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

#include <DeepSea/Scene/Scene.h>

#include "Flatbuffers/Scene_generated.h"
#include "SceneTypes.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Render/RenderPass.h>

#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/ItemLists/SceneItemList.h>
#include <DeepSea/Scene/SceneGlobalData.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/SceneRenderPass.h>

#define PRINT_FLATBUFFER_ERROR(message, name) \
	do \
	{ \
		if (name) \
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, message " for '%s'.", name); \
		else \
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, message); \
	} while (false)

template <typename T>
using FlatbufferVector = flatbuffers::Vector<flatbuffers::Offset<T>>;

static size_t getTempSize(const FlatbufferVector<DeepSeaScene::SceneItemLists>* fbSharedItems,
	const FlatbufferVector<DeepSeaScene::ScenePipelineItem>& fbPipeline,
	const FlatbufferVector<DeepSeaScene::GlobalData>* fbGlobalData)
{
	size_t tempSize = 0;
	if (fbSharedItems && fbSharedItems->size() > 0)
	{
		uint32_t count = fbSharedItems->size();
		tempSize += DS_ALIGNED_SIZE(count*sizeof(dsSceneItemLists));
		for (auto fbItemsArray : *fbSharedItems)
		{
			if (!fbItemsArray)
			{
				DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene shared item list array is null.");
				return 0;
			}

			uint32_t itemCount = fbItemsArray->itemLists()->size();
			if (itemCount == 0)
			{
				DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene shared item list array is empty.");
				return 0;
			}
			tempSize += DS_ALIGNED_SIZE(itemCount*sizeof(dsSceneItemList*));
		}
	}

	uint32_t pipelineCount = fbPipeline.size();
	if (pipelineCount == 0)
	{
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene pipeline is empty.");
		return 0;
	}
	size_t maxRenderPassSize = 0;
	for (auto fbPipelineItem : fbPipeline)
	{
		if (!fbPipelineItem)
		{
			DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene pipeline item is null.");
			return 0;
		}

		if (auto fbRenderPass = fbPipelineItem->item_as_RenderPass())
		{
			size_t renderPassSize = 0;
			auto fbAttachments = fbRenderPass->attachments();
			if (fbAttachments)
			{
				uint32_t attachmentCount = fbAttachments->size();
				renderPassSize += DS_ALIGNED_SIZE(attachmentCount*sizeof(dsAttachmentInfo)) +
					DS_ALIGNED_SIZE(attachmentCount*sizeof(dsSurfaceClearValue));
			}
			auto fbSubpasses = fbRenderPass->subpasses();
			uint32_t subpassCount = fbSubpasses->size();
			if (subpassCount == 0)
			{
				DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene render pass subpass array is empty.");
				return 0;
			}

			renderPassSize += DS_ALIGNED_SIZE(subpassCount*sizeof(dsRenderSubpassInfo)) +
				DS_ALIGNED_SIZE(subpassCount*sizeof(dsSceneItemLists));
			for (auto fbSubpass : *fbSubpasses)
			{
				if (!fbSubpass)
				{
					DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene subpass is null.");
					return 0;
				}

				auto fbInputAttachments = fbSubpass->inputAttachments();
				if (fbInputAttachments && fbInputAttachments->size())
					renderPassSize += DS_ALIGNED_SIZE(fbInputAttachments->size()*sizeof(uint32_t));

				auto fbColorAttachments = fbSubpass->colorAttachments();
				if (fbColorAttachments && fbColorAttachments->size())
				{
					renderPassSize +=
						DS_ALIGNED_SIZE(fbColorAttachments->size()*sizeof(dsAttachmentRef));
				}

				if (fbSubpass->depthStencilAttachment())
					renderPassSize += DS_ALIGNED_SIZE(sizeof(dsAttachmentRef));

				uint32_t drawListCount = fbSubpass->drawLists()->size();
				if (drawListCount > 0)
				{
					DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene subpass draw list array is empty.");
					return 0;
				}

				renderPassSize += DS_ALIGNED_SIZE(drawListCount*sizeof(dsSceneItemLists));
			}

			auto fbDependencies = fbRenderPass->dependencies();
			if (fbDependencies)
			{
				fbDependencies +=
					DS_ALIGNED_SIZE(fbDependencies->size()*sizeof(dsSubpassDependency));
			}

			maxRenderPassSize = std::max(maxRenderPassSize, renderPassSize);
		}
		else if (!fbPipelineItem->item_as_SceneItemList())
		{
			// empty
		}
		else
		{
			DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene pipeline item is null.");
			return 0;
		}
	}
	tempSize += maxRenderPassSize;

	if (fbGlobalData && fbGlobalData->size() > 0)
		tempSize += DS_ALIGNED_SIZE(fbGlobalData->size()*sizeof(dsSceneGlobalData*));

	return tempSize;
}

static void destroyItemLists(const dsSceneItemLists* itemLists, uint32_t itemListsCount)
{
	for (uint32_t i = 0; i < itemListsCount; ++i)
	{
		const dsSceneItemLists* lists = itemLists + i;
		for (uint32_t j = 0; j < lists->count; ++j)
			dsSceneItemList_destroy(lists->itemLists[j]);
	}
}

static dsSceneRenderPass* createRenderPass(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData,
	dsAllocator* scratchAllocator, dsRenderer* renderer,
	const DeepSeaScene::RenderPass& fbRenderPass)
{
	dsAttachmentInfo* attachments = nullptr;
	dsSurfaceClearValue* clearValues = nullptr;
	uint32_t attachmentCount = 0;
	auto fbAttachments = fbRenderPass.attachments();
	if (fbAttachments && fbAttachments->size() > 0)
	{
		attachmentCount = fbAttachments->size();
		attachments = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, dsAttachmentInfo, attachmentCount);
		DS_ASSERT(attachments);

		clearValues =
			DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, dsSurfaceClearValue, attachmentCount);
		DS_ASSERT(clearValues);

		for (uint32_t i = 0; i < attachmentCount; ++i)
		{
			auto fbAttachment = (*fbAttachments)[i];
			if (!fbAttachment)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene render pass attachment is null.");
				return nullptr;
			}

			dsAttachmentInfo* attachment = attachments + i;
			attachment->usage = static_cast<dsAttachmentUsage>(fbAttachment->usage());
			attachment->format = DeepSeaScene::convert(fbAttachment->format(),
				fbAttachment->decoration());
			attachment->samples = fbAttachment->samples();

			if (auto fbClearColorFloat = fbAttachment->clearValue_as_ClearColorFloat())
			{
				dsColor4f& colorValue = clearValues[i].colorValue.floatValue;
				colorValue.r = fbClearColorFloat->red();
				colorValue.g = fbClearColorFloat->green();
				colorValue.b = fbClearColorFloat->blue();
				colorValue.a = fbClearColorFloat->alpha();
			}
			else if (auto fbClearColorInt = fbAttachment->clearValue_as_ClearColorInt())
			{
				int* colorValues = clearValues[i].colorValue.intValue;
				colorValues[0] = fbClearColorInt->red();
				colorValues[1] = fbClearColorInt->green();
				colorValues[2] = fbClearColorInt->blue();
				colorValues[3] = fbClearColorInt->alpha();
			}
			else if (auto fbClearColorUInt = fbAttachment->clearValue_as_ClearColorUInt())
			{
				unsigned int* colorValues = clearValues[i].colorValue.uintValue;
				colorValues[0] = fbClearColorUInt->red();
				colorValues[1] = fbClearColorUInt->green();
				colorValues[2] = fbClearColorUInt->blue();
				colorValues[3] = fbClearColorUInt->alpha();
			}
			else if (auto fbDepthStenciValue = fbAttachment->clearValue_as_ClearDepthStencil())
			{
				dsDepthStencilValue* depthStencilValue = &clearValues[i].depthStencil;
				depthStencilValue->depth = fbDepthStenciValue->depth();
				depthStencilValue->stencil = fbDepthStenciValue->stencil();
			}
		}
	}

	auto fbSubpasses = fbRenderPass.subpasses();
	uint32_t subpassCount = fbSubpasses->size();
	auto subpasses = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, dsRenderSubpassInfo, subpassCount);
	DS_ASSERT(subpasses);
	auto drawLists = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, dsSceneItemLists, subpassCount);
	DS_ASSERT(drawLists);
	for (uint32_t i = 0; i < subpassCount; ++i)
	{
		auto fbSubpass = (*fbSubpasses)[i];
		DS_ASSERT(fbSubpass);

		dsRenderSubpassInfo* subpass = subpasses + i;
		subpass->name = fbSubpass->name()->c_str();

		auto fbInputAttachments = fbSubpass->inputAttachments();
		if (fbInputAttachments && fbInputAttachments->size() > 0)
		{
			subpass->inputAttachmentCount = fbInputAttachments->size();
			subpass->inputAttachments = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, uint32_t,
				subpass->inputAttachmentCount);
			DS_ASSERT(subpass->inputAttachments);
			for (uint32_t j = 0; j < subpass->inputAttachmentCount; ++j)
				const_cast<uint32_t&>(subpass->inputAttachments[j]) = (*fbInputAttachments)[j];
		}
		else
		{
			subpass->inputAttachments = nullptr;
			subpass->inputAttachmentCount = 0;
		}

		auto fbColorAttachments = fbSubpass->colorAttachments();
		if (fbColorAttachments && fbColorAttachments->size() > 0)
		{
			subpass->colorAttachmentCount = fbInputAttachments->size();
			subpass->colorAttachments = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, dsAttachmentRef,
				subpass->colorAttachmentCount);
			DS_ASSERT(subpass->colorAttachments);
			for (uint32_t j = 0; j < subpass->colorAttachmentCount; ++j)
			{
				auto attachment = const_cast<dsAttachmentRef*>(subpass->colorAttachments + i);
				auto fbAttachment = (*fbColorAttachments)[j];
				if (fbAttachment)
				{
					attachment->attachmentIndex = fbAttachment->index();;
					attachment->resolve = fbAttachment->resolve();
				}
				else
				{
					attachment->attachmentIndex = DS_NO_ATTACHMENT;
					attachment->resolve = false;
				}
			}
		}
		else
		{
			subpass->colorAttachments = nullptr;
			subpass->colorAttachmentCount = 0;
		}

		auto fbDepthStencilAttachment = fbSubpass->depthStencilAttachment();
		if (fbDepthStencilAttachment)
		{
			subpass->depthStencilAttachment.attachmentIndex = fbDepthStencilAttachment->index();
			subpass->depthStencilAttachment.resolve = fbDepthStencilAttachment->resolve();
		}
		else
		{
			subpass->depthStencilAttachment.attachmentIndex = DS_NO_ATTACHMENT;
			subpass->depthStencilAttachment.resolve = false;
		}

		auto fbDrawLists = fbSubpass->drawLists();
		dsSceneItemLists* subpassDrawLists = drawLists + i;
		subpassDrawLists->count = fbDrawLists->size();
		subpassDrawLists->itemLists = DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, dsSceneItemList*,
			subpassDrawLists->count);
		DS_ASSERT(subpassDrawLists->itemLists);
		for (uint32_t j = 0; j < subpassDrawLists->count; ++j)
		{
			auto fbItemList = (*fbDrawLists)[j];
			if (!fbItemList)
			{
				errno = EINVAL;
				DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene render pass draw list is null.");
				return nullptr;
			}

			auto fbData = fbItemList->data();
			subpassDrawLists->itemLists[j] = dsSceneItemList_load(allocator, resourceAllocator,
				loadContext, scratchData, fbItemList->type()->c_str(), fbItemList->name()->c_str(),
				fbData->data(), fbData->size());
			if (!subpassDrawLists->itemLists[j])
			{
				for (uint32_t k = 0; k < j; ++k)
				{
					dsSceneItemList_destroy(subpassDrawLists->itemLists[j]);
					errno = EINVAL;
					return nullptr;
				}
			}
		}
	}
	uint32_t dependencyCount = DS_DEFAULT_SUBPASS_DEPENDENCIES;
	dsSubpassDependency* dependencies = nullptr;
	auto fbDependencies = fbRenderPass.dependencies();
	if (fbDependencies)
	{
		dependencyCount = fbDependencies->size();
		if (dependencyCount > 0)
		{
			dependencies =
				DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, dsSubpassDependency, dependencyCount);
			DS_ASSERT(dependencies);

			for (uint32_t i = 0; i < dependencyCount; ++i)
			{
				auto fbDependency = (*fbDependencies)[i];
				if (!fbDependency)
				{
					destroyItemLists(drawLists, subpassCount);
					errno = EINVAL;
					DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene render subpass dependency is null.");
					return nullptr;
				}

				dsSubpassDependency* dependency = dependencies + i;
				dependency->srcSubpass = fbDependency->srcSubpass();
				dependency->srcStages = static_cast<dsGfxPipelineStage>(fbDependency->srcStages());
				dependency->srcAccess = static_cast<dsGfxAccess>(fbDependency->srcAccess());
				dependency->dstStages = static_cast<dsGfxPipelineStage>(fbDependency->dstStages());
				dependency->dstAccess = static_cast<dsGfxAccess>(fbDependency->dstAccess());
				dependency->regionDependency = fbDependency->regionDependency();
			}
		}
	}

	dsRenderPass* renderPass = dsRenderPass_create(renderer, resourceAllocator, attachments,
		attachmentCount, subpasses, subpassCount, dependencies, dependencyCount);
	if (!renderPass)
		return nullptr;

	return dsSceneRenderPass_create(allocator, renderPass, fbRenderPass.framebuffer()->c_str(),
		clearValues, attachmentCount, drawLists, subpassCount);
}

extern "C"
dsScene* dsScene_loadImpl(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData, const void* data,
	size_t dataSize, void* userData, dsDestroySceneUserDataFunction destroyUserDataFunc,
	const char* name)
{
	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), dataSize);
	if (!DeepSeaScene::VerifySceneBuffer(verifier))
	{
		errno = EFORMAT;
		PRINT_FLATBUFFER_ERROR("Invalid scene resources flatbuffer format", name);
		return nullptr;
	}

	dsRenderer* renderer = dsSceneLoadContext_getRenderer(loadContext);

	auto fbScene = DeepSeaScene::GetScene(data);
	auto fbSharedItems = fbScene->sharedItems();
	auto fbPipeline = fbScene->pipeline();
	auto fbGlobalData = fbScene->globalData();

	uint32_t sharedItemCount = 0;
	dsSceneItemLists* sharedItems = nullptr;

	uint32_t pipelineCount = 0;
	dsScenePipelineItem* pipeline = nullptr;

	uint32_t globalDataCount = 0;
	dsSceneGlobalData** globalData = nullptr;

	dsScene* scene = nullptr;

	dsAllocator* scratchAllocator = dsSceneLoadScratchData_getAllocator(scratchData);
	DS_ASSERT(scratchAllocator);
	size_t tempSize = getTempSize(fbSharedItems, *fbPipeline, fbGlobalData);
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

	if (fbSharedItems && fbSharedItems->size() > 0)
	{
		sharedItemCount = fbSharedItems->size();
		// Align the size so it can be popped at the same time as the internal lists.ists)));
		sharedItems = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneItemLists, sharedItemCount);
		DS_ASSERT(sharedItems);

		for (uint32_t i = 0; i < sharedItemCount; ++i)
		{
			auto fbItemsArray = (*fbSharedItems)[i];
			DS_ASSERT(fbItemsArray);
			auto fbItems = fbItemsArray->itemLists();
			dsSceneItemLists* items = sharedItems + i;
			items->count = fbItems->size();
			DS_ASSERT(items->count > 0);
			items->itemLists = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneItemList*, items->count);
			DS_ASSERT(items->itemLists);

			for (uint32_t j = 0; j < items->count; ++j)
			{
				auto fbItemList = (*fbItems)[j];
				if (!fbItemList)
				{
					errno = EFORMAT;
					DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene shared item list is null.");
					// Only clear out what's been set so far.
					sharedItemCount = i + 1;
					items->count = j;
					goto finished;
				}

				auto fbData = fbItemList->data();
				items->itemLists[j] = dsSceneItemList_load(allocator, resourceAllocator,
					loadContext, scratchData, fbItemList->type()->c_str(),
					fbItemList->name()->c_str(), fbData->data(), fbData->size());
				if (!items->itemLists[j])
				{
					errno = EFORMAT;
					// Only clear out what's been set so far.
					sharedItemCount = i + 1;
					items->count = j;
					goto finished;
				}
			}
		}
	}

	pipelineCount = fbPipeline->size();
	pipeline = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsScenePipelineItem, pipelineCount);
	for (uint32_t i = 0; i < pipelineCount; ++i)
	{
		auto fbPipelineItem = (*fbPipeline)[i];
		if (auto fbRenderPass = fbPipelineItem->item_as_RenderPass())
		{
			auto tempAllocator = reinterpret_cast<dsAllocator*>(&bufferAlloc);
			size_t prevTempBufferSize = tempAllocator->size;
			dsSceneRenderPass* renderPass = createRenderPass(allocator, resourceAllocator,
				loadContext, scratchData, tempAllocator, renderer, *fbRenderPass);
			if (!renderPass)
			{
				// Only clear out what's been set so far.
				pipelineCount = i;
				goto finished;
			}

			// Restore previous temporary allocator size since allocations are no longer needed.
			tempAllocator->size = prevTempBufferSize;
			pipeline[i].renderPass = renderPass;
			pipeline[i].computeItems = nullptr;
		}
		else if (auto fbItemList = fbPipelineItem->item_as_SceneItemList())
		{
			auto fbData = fbItemList->data();
			pipeline[i].computeItems = dsSceneItemList_load(allocator, resourceAllocator,
				loadContext, scratchData, fbItemList->type()->c_str(), fbItemList->name()->c_str(),
				fbData->data(), fbData->size());
			if (!pipeline[i].computeItems)
			{
				// Only clear out what's been set so far.
				pipelineCount = i;
				goto finished;
			}
			pipeline[i].renderPass = nullptr;
		}
	}

	if (fbGlobalData && fbGlobalData->size())
	{
		globalDataCount = fbGlobalData->size();
		globalData =
			DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, dsSceneGlobalData*, globalDataCount);
		for (uint32_t i = 0; i < globalDataCount; ++i)
		{
			auto fbGlobalDataItem = (*fbGlobalData)[i];
			if (!fbGlobalData)
			{
				errno = EFORMAT;
				DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene global data is null.");
				// Only clear out what's been set so far.
				globalDataCount = i;
				goto finished;
			}

			auto fbData = fbGlobalDataItem->data();
			globalData[i] = dsSceneGlobalData_load(allocator, resourceAllocator, loadContext,
				scratchData, fbGlobalDataItem->type()->c_str(), fbData->data(), fbData->size());
			if (!globalData[i])
			{
				// Only clear out what's been set so far.
				globalDataCount = i;
				goto finished;
			}
		}
	}

	scene = dsScene_create(allocator, renderer, sharedItems, sharedItemCount, pipeline,
		pipelineCount, globalData, globalDataCount, userData, destroyUserDataFunc);

finished:
	// Counts contain the items that need to be cleaned up.
	for (uint32_t i = 0; i < sharedItemCount; ++i)
	{
		dsSceneItemLists* itemLists = sharedItems + i;
		for (uint32_t j = 0; j < itemLists->count; ++j)
			dsSceneItemList_destroy(itemLists->itemLists[j]);
	}
	for (uint32_t i = 0; i < pipelineCount; ++i)
	{
		dsScenePipelineItem* curItem = pipeline + i;
		if (curItem->renderPass)
			dsSceneRenderPass_destroy(curItem->renderPass);
		else if (curItem->computeItems)
			dsSceneItemList_destroy(curItem->computeItems);
	}
	for (uint32_t i = 0; i < globalDataCount; ++i)
		dsSceneGlobalData_destroy(globalData[i]);

	DS_VERIFY(dsAllocator_free(scratchAllocator, tempBuffer));
	return scene;
}

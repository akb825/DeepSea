/*
 * Copyright 2023-2025 Aaron Barany
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

#include <DeepSea/SceneLighting/SceneComputeSSAO.h>

#include "SceneSSAOShared.h"

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Renderer.h>

#include <DeepSea/Scene/ItemLists/SceneItemList.h>

#include <string.h>

struct dsSceneComputeSSAO
{
	dsSceneItemList itemList;
	dsResourceManager* resourceManager;
	dsAllocator* resourceAllocator;
	dsShader* shader;
	dsMaterial* material;

	dsGfxBuffer* randomOffsets;
	dsTexture* randomRotations;
};

void dsSceneComputeSSAO_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	dsSceneComputeSSAO* ssao = (dsSceneComputeSSAO*)itemList;
	if (!DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG,
			dsShader_bindCompute(ssao->shader, commandBuffer, ssao->material, view->globalValues)))
	{
		return;
	}

	uint32_t x = (view->preRotateWidth + DS_SCENE_COMPUTE_SSAO_TILE_SIZE - 1)/
		DS_SCENE_COMPUTE_SSAO_TILE_SIZE;
	uint32_t y = (view->preRotateHeight + DS_SCENE_COMPUTE_SSAO_TILE_SIZE - 1)/
		DS_SCENE_COMPUTE_SSAO_TILE_SIZE;
	DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG,
		dsRenderer_dispatchCompute(commandBuffer->renderer, commandBuffer, x, y, 1));

	DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsShader_unbindCompute(ssao->shader, commandBuffer));
}

const char* const dsSceneComputeSSAO_typeName = "ComputeSSAO";

const dsSceneItemListType* dsSceneComputeSSAO_type(void)
{
	static dsSceneItemListType type =
	{
		.commitFunc = &dsSceneComputeSSAO_commit,
		.destroyFunc = (dsDestroySceneItemListFunction)&dsSceneComputeSSAO_destroy
	};
	return &type;
}

dsSceneComputeSSAO* dsSceneComputeSSAO_create(dsAllocator* allocator, dsResourceManager* resourceManager,
	dsAllocator* resourceAllocator, const char* name, dsShader* shader, dsMaterial* material)
{
	if (!allocator || !resourceManager || !name || !shader || !material ||
		!dsSceneSSAO_canUseMaterial(material))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!dsShader_hasStage(shader, dsShaderStage_Compute))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG,
			"Scene compute SSAO shader must have a compute stage.");
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG,
			"Scene compute SSAO allocator must support freeing memory.");
		return NULL;
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneComputeSSAO)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneComputeSSAO* ssao = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneComputeSSAO);
	DS_ASSERT(ssao);

	dsSceneItemList* itemList = (dsSceneItemList*)ssao;
	itemList->allocator = dsAllocator_keepPointer(allocator);
	itemList->type = dsSceneComputeSSAO_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsUniqueNameID_create(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = true;
	itemList->skipPreRenderPass = false;

	ssao->resourceManager = resourceManager;
	ssao->resourceAllocator = resourceAllocator;
	ssao->shader = shader;
	ssao->material = material;
	ssao->randomOffsets = NULL;
	ssao->randomRotations = NULL;

	ssao->randomOffsets = dsSceneSSAO_createRandomOffsets(resourceManager, resourceAllocator);
	if (!ssao->randomOffsets)
	{
		dsSceneComputeSSAO_destroy(ssao);
		return NULL;
	}

	ssao->randomRotations = dsSceneSSAO_createRandomRotations(resourceManager, resourceAllocator);
	if (!ssao->randomRotations)
	{
		dsSceneComputeSSAO_destroy(ssao);
		return NULL;
	}

	dsSceneSSAO_setMaterialValues(ssao->material, ssao->randomOffsets, ssao->randomRotations);
	return ssao;
}

dsShader* dsSceneComputeSSAO_getShader(const dsSceneComputeSSAO* ssao)
{
	if (!ssao)
		return NULL;

	return ssao->shader;
}

bool dsSceneComputeSSAO_setShader(dsSceneComputeSSAO* ssao, dsShader* shader)
{
	if (!ssao || !shader)
	{
		errno = EINVAL;
		return false;
	}

	ssao->shader = shader;
	return true;
}

dsMaterial* dsSceneComputeSSAO_getMaterial(const dsSceneComputeSSAO* ssao)
{
	if (!ssao)
		return NULL;

	return ssao->material;
}

bool dsSceneComputeSSAO_setMaterial(dsSceneComputeSSAO* ssao, dsMaterial* material)
{
	if (!ssao || !material || !dsSceneSSAO_canUseMaterial(material))
	{
		errno = EINVAL;
		return false;
	}

	ssao->material = material;
	dsSceneSSAO_setMaterialValues(ssao->material, ssao->randomOffsets, ssao->randomRotations);
	return true;
}

void dsSceneComputeSSAO_destroy(dsSceneComputeSSAO* ssao)
{
	if (!ssao)
		return;

	dsSceneItemList* itemList = (dsSceneItemList*)ssao;

	dsGfxBuffer_destroy(ssao->randomOffsets);
	dsTexture_destroy(ssao->randomRotations);

	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

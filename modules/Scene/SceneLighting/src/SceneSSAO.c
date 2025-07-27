/*
 * Copyright 2021-2025 Aaron Barany
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

#include <DeepSea/SceneLighting/SceneSSAO.h>

#include "SceneSSAOShared.h"

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Renderer.h>

#include <DeepSea/Scene/ItemLists/SceneFullScreenResolve.h>
#include <DeepSea/Scene/ItemLists/SceneItemList.h>

#include <string.h>

struct dsSceneSSAO
{
	dsSceneItemList itemList;
	dsResourceManager* resourceManager;
	dsAllocator* resourceAllocator;
	dsShader* shader;
	dsMaterial* material;

	dsDrawGeometry* geometry;
	dsGfxBuffer* randomOffsets;
	dsTexture* randomRotations;
};

static void dsSceneSSAO_commit(
	dsSceneItemList* itemList, const dsView* view, dsCommandBuffer* commandBuffer)
{
	dsSceneSSAO* ssao = (dsSceneSSAO*)itemList;
	if (!DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG,
			dsShader_bind(ssao->shader, commandBuffer, ssao->material, view->globalValues, NULL)))
	{
		return;
	}

	dsDrawRange drawRange = {4, 1, 0, 0};
	DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsRenderer_draw(commandBuffer->renderer, commandBuffer,
		ssao->geometry, &drawRange, dsPrimitiveType_TriangleStrip));

	DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsShader_unbind(ssao->shader, commandBuffer));
}

static uint32_t dsSceneSSAO_hash(const dsSceneItemList* itemList, uint32_t commonHash)
{
	DS_ASSERT(itemList);
	const dsSceneSSAO* ssao = (const dsSceneSSAO*)itemList;
	const void* hashPtrs[2] = {ssao->shader, ssao->material};
	return dsHashCombineBytes(commonHash, hashPtrs, sizeof(hashPtrs));
}

static bool dsSceneSSAO_equal(const dsSceneItemList* left, const dsSceneItemList* right)
{
	DS_ASSERT(left);
	DS_ASSERT(left->type == dsSceneSSAO_type());
	DS_ASSERT(right);
	DS_ASSERT(right->type == dsSceneSSAO_type());

	const dsSceneSSAO* leftSSAO = (const dsSceneSSAO*)left;
	const dsSceneSSAO* rightSSAO = (const dsSceneSSAO*)right;
	return leftSSAO->shader == rightSSAO->shader && leftSSAO->material == rightSSAO->material;
}

static void dsSceneSSAO_destroy(dsSceneItemList* itemList)
{
	DS_ASSERT(itemList);
	dsSceneSSAO* ssao = (dsSceneSSAO*)itemList;

	if (ssao->geometry)
		dsSceneFullScreenResolve_destroyGeometry();
	dsGfxBuffer_destroy(ssao->randomOffsets);
	dsTexture_destroy(ssao->randomRotations);

	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

const char* const dsSceneSSAO_typeName = "SSAO";

static dsSceneItemListType itemListType =
{
	.commitFunc = &dsSceneSSAO_commit,
	.hashFunc = &dsSceneSSAO_hash,
	.equalFunc = &dsSceneSSAO_equal,
	.destroyFunc = &dsSceneSSAO_destroy
};

const dsSceneItemListType* dsSceneSSAO_type(void)
{
	return &itemListType;
}

dsSceneSSAO* dsSceneSSAO_create(dsAllocator* allocator, dsResourceManager* resourceManager,
	dsAllocator* resourceAllocator, const char* name, dsShader* shader, dsMaterial* material)
{
	if (!allocator || !resourceManager || !name || !shader || !material ||
		!dsSceneSSAO_canUseMaterial(material))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG,
			"Scene SSAO allocator must support freeing memory.");
		return NULL;
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneSSAO)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneSSAO* ssao = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneSSAO);
	DS_ASSERT(ssao);

	dsSceneItemList* itemList = (dsSceneItemList*)ssao;
	itemList->allocator = dsAllocator_keepPointer(allocator);
	itemList->type = dsSceneSSAO_type();
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
	ssao->geometry = NULL;
	ssao->randomOffsets = NULL;
	ssao->randomRotations = NULL;

	ssao->geometry = dsSceneFullScreenResolve_createGeometry(resourceManager);
	if (!ssao->geometry)
	{
		dsSceneSSAO_destroy(itemList);
		return NULL;
	}

	ssao->randomOffsets = dsSceneSSAO_createRandomOffsets(resourceManager, resourceAllocator);
	if (!ssao->randomOffsets)
	{
		dsSceneSSAO_destroy(itemList);
		return NULL;
	}

	ssao->randomRotations = dsSceneSSAO_createRandomRotations(resourceManager, resourceAllocator);
	if (!ssao->randomRotations)
	{
		dsSceneSSAO_destroy(itemList);
		return NULL;
	}

	dsSceneSSAO_setMaterialValues(ssao->material, ssao->randomOffsets, ssao->randomRotations);
	return ssao;
}

dsShader* dsSceneSSAO_getShader(const dsSceneSSAO* ssao)
{
	if (!ssao)
		return NULL;

	return ssao->shader;
}

bool dsSceneSSAO_setShader(dsSceneSSAO* ssao, dsShader* shader)
{
	if (!ssao || !shader)
	{
		errno = EINVAL;
		return false;
	}

	ssao->shader = shader;
	return true;
}

dsMaterial* dsSceneSSAO_getMaterial(const dsSceneSSAO* ssao)
{
	if (!ssao)
		return NULL;

	return ssao->material;
}

bool dsSceneSSAO_setMaterial(dsSceneSSAO* ssao, dsMaterial* material)
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

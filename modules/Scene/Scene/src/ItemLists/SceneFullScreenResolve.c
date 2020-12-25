/*
 * Copyright 2020 Aaron Barany
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

#include <DeepSea/Scene/ItemLists/SceneFullScreenResolve.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/CommandBuffer.h>
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Scene/ItemLists/SceneItemList.h>

#include <string.h>

struct dsSceneFullScreenResolve
{
	dsSceneItemList itemList;
	dsShader* shader;
	dsMaterial* material;
	dsDynamicRenderStates renderStates;
	bool hasRenderStates;

	dsGfxBuffer* vertexData;
	dsDrawGeometry* geometry;
};

const char* const dsSceneFullScreenResolve_typeName = "FullScreenResolve";

static const uint16_t vertexData[] =
{
	0, 0xFFFF,
	0, 0,
	0xFFFF, 0xFFFF,
	0xFFFF, 0
};

static uint64_t dsSceneFullScreenResolve_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	const dsMatrix44f* transform, dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(itemList);
	DS_UNUSED(node);
	DS_UNUSED(transform);
	DS_UNUSED(itemData);
	DS_UNUSED(thisItemData);
	return DS_NO_SCENE_NODE;
}

static void dsSceneFullScreenResolve_removeNode(dsSceneItemList* itemList, uint64_t nodeID)
{
	DS_UNUSED(itemList);
	DS_UNUSED(nodeID);
}

static void dsCommitSceneItemList_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	dsSceneFullScreenResolve* resolve = (dsSceneFullScreenResolve*)itemList;
	dsDynamicRenderStates* renderStates =
		resolve->hasRenderStates ? &resolve->renderStates : NULL;
	const dsRenderSubpassInfo* curSubpass = commandBuffer->boundRenderPass->subpasses +
		commandBuffer->activeRenderSubpass;
	if (!DS_CHECK(DS_SCENE_LOG_TAG, dsSharedMaterialValues_setSubpassInputs(
			view->globalValues, resolve->shader, curSubpass,
			commandBuffer->boundFramebuffer, dsMaterialBinding_Global)) ||
		!DS_CHECK(DS_SCENE_LOG_TAG, dsShader_bind(resolve->shader, commandBuffer,
			resolve->material, view->globalValues, renderStates)))
	{
		return;
	}

	dsDrawRange drawRange = {4, 0, 0, 0};
	DS_CHECK(DS_SCENE_LOG_TAG, dsRenderer_draw(commandBuffer->renderer, commandBuffer,
		resolve->geometry, &drawRange, dsPrimitiveType_TriangleFan));

	DS_CHECK(DS_SCENE_LOG_TAG, dsShader_unbind(resolve->shader, commandBuffer));
}

dsSceneItemList* dsSceneFullScreenResolve_create(dsAllocator* allocator, const char* name,
	dsResourceManager* resourceManager, dsAllocator* resourceAllocator, dsShader* shader,
	dsMaterial* material, const dsDynamicRenderStates* renderStates)
{
	if (!allocator || !name || !resourceManager || !shader || !material)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneFullScreenResolve)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneFullScreenResolve* resolve = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneFullScreenResolve);
	DS_ASSERT(resolve);

	dsSceneItemList* itemList = (dsSceneItemList*)resolve;
	itemList->allocator = dsAllocator_keepPointer(allocator);
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsHashString(name);
	itemList->needsCommandBuffer = true;
	itemList->addNodeFunc = &dsSceneFullScreenResolve_addNode;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = &dsSceneFullScreenResolve_removeNode;
	itemList->commitFunc = &dsCommitSceneItemList_commit;
	itemList->destroyFunc = (dsDestroySceneItemListFunction)&dsSceneFullScreenResolve_destroy;

	resolve->shader = shader;
	resolve->material = material;
	if (renderStates)
	{
		resolve->renderStates = *renderStates;
		resolve->hasRenderStates = true;
	}
	else
		resolve->hasRenderStates = false;
	resolve->vertexData = NULL;
	resolve->geometry = NULL;

	resolve->vertexData = dsGfxBuffer_create(resourceManager, resourceAllocator,
		dsGfxBufferUsage_Vertex, dsGfxMemory_GPUOnly | dsGfxMemory_Static | dsGfxMemory_Draw,
		vertexData, sizeof(vertexData));
	if (!resolve->vertexData)
	{
		dsSceneFullScreenResolve_destroy(resolve);
		return NULL;
	}

	dsVertexBuffer vertexBuffer;
	vertexBuffer.buffer = buffer;
	vertexBuffer.offset = 0;
	vertexBuffer.count = 4;
	DS_VERIFY(dsVertexFormat_initialize(&vertexBuffer.format));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexBuffer.format, dsVertexAttrib_Position, true));
	vertexBuffer.format.elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X16Y16, dsGfxFormat_UNorm);
	dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS] =
		{&vertexBuffer, NULL, NULL, NULL};

	resolve->geometry =
		dsDrawGeometry_create(resourceManager, resourceAllocator, vertexBuffers, NULL);
	if (!resolve->geometry)
	{
		dsSceneFullScreenResolve_destroy(resolve);
		return NULL;
	}

	return itemList;
}

void dsSceneFullScreenResolve_destroy(dsSceneFullScreenResolve* resolve)
{
	if (!resolve)
		return;

	dsDrawGeometry_destroy(resolve->geometry);
	dsGfxBuffer_destroy(resolve->vertexData);

	dsSceneItemList* itemList = (dsSceneItemList*)resolve;
	if (itemList->allocator)
		dsAllocator_free(itemList->allocator, itemList);
}

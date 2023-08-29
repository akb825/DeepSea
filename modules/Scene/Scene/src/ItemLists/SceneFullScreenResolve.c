/*
 * Copyright 2020-2023 Aaron Barany
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
#include <DeepSea/Core/Atomic.h>
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

	dsDrawGeometry* geometry;
};

static uint32_t spinlock;
static uint32_t geometryRefCount;
static dsGfxBuffer* vertexData;
static dsDrawGeometry* geometry;

static void dsCommitSceneItemList_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	dsSceneFullScreenResolve* resolve = (dsSceneFullScreenResolve*)itemList;
	dsDynamicRenderStates* renderStates =
		resolve->hasRenderStates ? &resolve->renderStates : NULL;
	if (!DS_CHECK(DS_SCENE_LOG_TAG, dsShader_bind(resolve->shader, commandBuffer,
			resolve->material, view->globalValues, renderStates)))
	{
		return;
	}

	dsDrawRange drawRange = {4, 1, 0, 0};
	DS_CHECK(DS_SCENE_LOG_TAG, dsRenderer_draw(commandBuffer->renderer, commandBuffer,
		resolve->geometry, &drawRange, dsPrimitiveType_TriangleStrip));

	DS_CHECK(DS_SCENE_LOG_TAG, dsShader_unbind(resolve->shader, commandBuffer));
}

const char* const dsSceneFullScreenResolve_typeName = "FullScreenResolve";

dsSceneItemListType dsSceneFullScreenResolve_type(void)
{
	static int type;
	return &type;
}

dsDrawGeometry* dsSceneFullScreenResolve_createGeometry(dsResourceManager* resourceManager)
{
	// Can't use a regular spinlock since can't initialize with a static variable.
	uint32_t expectedLocked;
	uint32_t locked = true;
	do
	{
		expectedLocked = false;
	}
	while (!DS_ATOMIC_COMPARE_EXCHANGE32(&spinlock, &expectedLocked, &locked, false));

	if (geometryRefCount++ == 0)
	{
		const int16_t vertices[] =
		{
			-INT16_MAX, INT16_MAX,
			-INT16_MAX, -INT16_MAX,
			INT16_MAX, INT16_MAX,
			INT16_MAX, -INT16_MAX
		};

		vertexData = dsGfxBuffer_create(resourceManager, NULL, dsGfxBufferUsage_Vertex,
			dsGfxMemory_GPUOnly | dsGfxMemory_Static | dsGfxMemory_Draw, vertices,
			sizeof(vertices));
		if (vertexData)
		{
			dsVertexBuffer vertexBuffer;
			vertexBuffer.buffer = vertexData;
			vertexBuffer.offset = 0;
			vertexBuffer.count = 4;
			DS_VERIFY(dsVertexFormat_initialize(&vertexBuffer.format));
			DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexBuffer.format, dsVertexAttrib_Position, true));
			vertexBuffer.format.elements[dsVertexAttrib_Position].format =
				dsGfxFormat_decorate(dsGfxFormat_X16Y16, dsGfxFormat_SNorm);
			DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(&vertexBuffer.format));
			dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS] =
				{&vertexBuffer, NULL, NULL, NULL};

			geometry = dsDrawGeometry_create(resourceManager, NULL, vertexBuffers, NULL);
			if (!geometry)
			{
				dsGfxBuffer_destroy(vertexData);
				vertexData = NULL;
			}
		}
	}

	expectedLocked = false;
	DS_ATOMIC_EXCHANGE32(&spinlock, &expectedLocked, &locked);
	DS_ASSERT(locked);

	return geometry;
}

void dsSceneFullScreenResolve_destroyGeometry(void)
{
	// Can't use a regular spinlock since can't initialize with a static variable.
	uint32_t expectedLocked;
	uint32_t locked = true;
	do
	{
		expectedLocked = false;
	}
	while (!DS_ATOMIC_COMPARE_EXCHANGE32(&spinlock, &expectedLocked, &locked, false));

	DS_ASSERT(geometryRefCount > 0);
	if (--geometryRefCount == 0)
	{
		dsDrawGeometry_destroy(geometry);
		dsGfxBuffer_destroy(vertexData);
		geometry = NULL;
		vertexData = NULL;
	}

	expectedLocked = false;
	DS_ATOMIC_EXCHANGE32(&spinlock, &expectedLocked, &locked);
	DS_ASSERT(locked);
}

dsSceneFullScreenResolve* dsSceneFullScreenResolve_create(dsAllocator* allocator, const char* name,
	dsResourceManager* resourceManager, dsShader* shader, dsMaterial* material,
	const dsDynamicRenderStates* renderStates)
{
	if (!allocator || !name || !resourceManager || !shader || !material)
	{
		errno = EINVAL;
		return NULL;
	}

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
	itemList->type = dsSceneFullScreenResolve_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsHashString(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = true;
	itemList->addNodeFunc = NULL;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = NULL;
	itemList->preTransformUpdateFunc = NULL;
	itemList->updateFunc = NULL;
	itemList->preRenderPassFunc = NULL;
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

	resolve->geometry = dsSceneFullScreenResolve_createGeometry(resourceManager);
	if (!resolve->geometry)
	{
		dsSceneFullScreenResolve_destroy(resolve);
		return NULL;
	}

	return resolve;
}

void dsSceneFullScreenResolve_destroy(dsSceneFullScreenResolve* resolve)
{
	if (!resolve)
		return;

	if (resolve->geometry)
		dsSceneFullScreenResolve_destroyGeometry();

	dsSceneItemList* itemList = (dsSceneItemList*)resolve;
	if (itemList->allocator)
		dsAllocator_free(itemList->allocator, itemList);
}

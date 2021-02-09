/*
 * Copyright 2021 Aaron Barany
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

#include <DeepSea/SceneLighting/DeferredLightResolve.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Math/Core.h>

#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Renderer.h>

#include <DeepSea/Scene/ItemLists/SceneItemList.h>
#include <DeepSea/SceneLighting/SceneLightSet.h>
#include <DeepSea/SceneLighting/SceneLight.h>

#include <string.h>
#include <limits.h>

#define FRAME_DELAY 3

#define MAX_DIRECTIONAL_LIGHTS ((USHRT_MAX - 1)/DS_DIRECTIONAL_LIGHT_VERTEX_COUNT)
#define MAX_POINT_LIGHTS ((USHRT_MAX - 1)/DS_POINT_LIGHT_VERTEX_COUNT)
#define MAX_SPOT_LIGHTS ((USHRT_MAX - 1)/DS_SPOT_LIGHT_VERTEX_COUNT)

typedef struct BufferInfo
{
	dsGfxBuffer* buffer;
	dsDrawGeometry* ambientGometry;
	dsDrawGeometry* directionalGometry;
	dsDrawGeometry* pointGometry;
	dsDrawGeometry* spotGometry;
	uint64_t lastUsedFrame;
} BufferInfo;

typedef struct TraverseData
{
	BufferInfo* buffers;
	dsDirectionalLightVertex* directionalVerts;
	dsPointLightVertex* pointVerts;
	dsSpotLightVertex* spotVerts;

	uint16_t* directionalIndices;
	uint16_t* pointIndices;
	uint16_t* spotIndices;

	float intensityThreshold;
	uint32_t directionalCount;
	uint32_t pointCount;
	uint32_t spotCount;
} TraverseData;

struct dsDeferredLightResolve
{
	dsSceneItemList itemList;
	dsAllocator* resourceAllocator;

	const dsSceneLightSet* lightSet;
	dsShader* ambientShader;
	dsMaterial* ambientMaterial;
	dsShader* directionalShader;
	dsMaterial* directionalMaterial;
	dsShader* pointShader;
	dsMaterial* pointMaterial;
	dsShader* spotShader;
	dsMaterial* spotMaterial;
	float intensityThreshold;

	size_t bufferSize;

	size_t ambientVertexOffset;
	size_t directionalVertexOffset;
	size_t pointVertexOffset;
	size_t spotVertexOffset;

	size_t ambientIndexOffset;
	size_t directionalIndexOffset;
	size_t pointIndexOffset;
	size_t spotIndexOffset;

	BufferInfo* buffers;
	uint32_t bufferCount;
	uint32_t maxBuffers;
};

static void freeBuffers(BufferInfo* buffers)
{
	dsDrawGeometry_destroy(buffers->ambientGometry);
	dsDrawGeometry_destroy(buffers->directionalGometry);
	dsDrawGeometry_destroy(buffers->pointGometry);
	dsDrawGeometry_destroy(buffers->spotGometry);
	dsGfxBuffer_destroy(buffers->buffer);
}

static BufferInfo* getDrawBuffers(dsDeferredLightResolve* resolve, dsRenderer* renderer)
{
	for (uint32_t i = 0; i < resolve->bufferCount; ++i)
	{
		BufferInfo* buffer = resolve->buffers + i;
		if (buffer->lastUsedFrame + FRAME_DELAY <= renderer->frameNumber)
		{
			buffer->lastUsedFrame = renderer->frameNumber;
			return buffer;
		}
	}

	dsSceneItemList* itemList = (dsSceneItemList*)resolve;
	uint32_t maxLights = dsSceneLightSet_getMaxLights(resolve->lightSet);
	uint32_t index = resolve->bufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, resolve->buffers, resolve->bufferCount,
			resolve->maxBuffers, 1))
	{
		return NULL;
	}

	dsResourceManager* resourceManager = renderer->resourceManager;
	BufferInfo* buffers = resolve->buffers + index;
	memset(buffers, 0, sizeof(BufferInfo));

	buffers->buffer = dsGfxBuffer_create(resourceManager, resolve->resourceAllocator,
		dsGfxBufferUsage_Vertex | dsGfxBufferUsage_Index,
		dsGfxMemory_Draw | dsGfxMemory_Stream | dsGfxMemory_Synchronize, NULL, resolve->bufferSize);
	if (!buffers->buffer)
	{
		--resolve->bufferCount;
		return NULL;
	}

	dsVertexBuffer ambientVertices;
	ambientVertices.buffer = buffers->buffer;
	ambientVertices.offset = resolve->ambientVertexOffset;
	ambientVertices.count = DS_AMBIENT_LIGHT_VERTEX_COUNT;
	DS_VERIFY(dsSceneLight_getAmbientLightVertexFormat(&ambientVertices.format));

	dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS] =
		{&ambientVertices, NULL, NULL, NULL};

	dsIndexBuffer indexBuffer;
	indexBuffer.buffer = buffers->buffer;
	indexBuffer.offset = resolve->ambientIndexOffset;
	indexBuffer.count = DS_DIRECTIONAL_LIGHT_INDEX_COUNT;
	indexBuffer.indexSize = (uint32_t)sizeof(uint16_t);
	buffers->ambientGometry = dsDrawGeometry_create(resourceManager, resolve->resourceAllocator,
		vertexBuffers, &indexBuffer);

	dsVertexBuffer directionalVertices;
	directionalVertices.buffer = buffers->buffer;
	directionalVertices.offset = resolve->directionalVertexOffset;
	directionalVertices.count = DS_DIRECTIONAL_LIGHT_VERTEX_COUNT*maxLights;
	DS_VERIFY(dsSceneLight_getDirectionalLightVertexFormat(&directionalVertices.format));

	vertexBuffers[0] = &directionalVertices;

	indexBuffer.offset = resolve->directionalIndexOffset;
	indexBuffer.count = DS_DIRECTIONAL_LIGHT_INDEX_COUNT;

	buffers->directionalGometry = dsDrawGeometry_create(resourceManager, resolve->resourceAllocator,
		vertexBuffers, &indexBuffer);

	dsVertexBuffer pointVertices;
	pointVertices.buffer = buffers->buffer;
	pointVertices.offset = resolve->pointVertexOffset;
	pointVertices.count = DS_POINT_LIGHT_VERTEX_COUNT*maxLights;
	DS_VERIFY(dsSceneLight_getPointLightVertexFormat(&pointVertices.format));

	vertexBuffers[0] = &pointVertices;

	indexBuffer.offset = resolve->pointIndexOffset;
	indexBuffer.count = DS_POINT_LIGHT_INDEX_COUNT;

	buffers->pointGometry = dsDrawGeometry_create(resourceManager, resolve->resourceAllocator,
		vertexBuffers, &indexBuffer);

	dsVertexBuffer spotVertices;
	spotVertices.buffer = buffers->buffer;
	spotVertices.offset = resolve->spotVertexOffset;
	spotVertices.count = DS_SPOT_LIGHT_VERTEX_COUNT*maxLights;
	DS_VERIFY(dsSceneLight_getSpotLightVertexFormat(&spotVertices.format));

	vertexBuffers[0] = &spotVertices;

	indexBuffer.offset = resolve->spotIndexOffset;
	indexBuffer.count = DS_SPOT_LIGHT_INDEX_COUNT;

	buffers->spotGometry = dsDrawGeometry_create(resourceManager, resolve->resourceAllocator,
		vertexBuffers, &indexBuffer);

	if (!buffers->ambientGometry || !buffers->directionalGometry || !buffers->pointGometry ||
		!buffers->spotGometry)
	{
		freeBuffers(buffers);
		--resolve->bufferCount;
		return NULL;
	}

	buffers->lastUsedFrame = renderer->frameNumber;
	return buffers;
}

static bool visitLights(void* userData, const dsSceneLightSet* lightSet, const dsSceneLight* light)
{
	DS_UNUSED(lightSet);
	TraverseData* traverseData = (TraverseData*)userData;
	switch (light->type)
	{
		case dsSceneLightType_Directional:
		{
			uint16_t firstIndex =
				(uint16_t)((traverseData->directionalCount % MAX_DIRECTIONAL_LIGHTS)*
					DS_DIRECTIONAL_LIGHT_VERTEX_COUNT);
			DS_VERIFY(dsSceneLight_getDirectionalLightVertices(traverseData->directionalVerts,
				DS_DIRECTIONAL_LIGHT_VERTEX_COUNT, traverseData->directionalIndices,
				DS_DIRECTIONAL_LIGHT_INDEX_COUNT, light, firstIndex));

			traverseData->directionalVerts += DS_DIRECTIONAL_LIGHT_VERTEX_COUNT;
			traverseData->directionalIndices += DS_DIRECTIONAL_LIGHT_INDEX_COUNT;
			++traverseData->directionalCount;
			break;
		}
		case dsSceneLightType_Point:
		{
			uint16_t firstIndex = (uint16_t)((traverseData->pointCount % MAX_POINT_LIGHTS)*
				DS_POINT_LIGHT_VERTEX_COUNT);
			DS_VERIFY(dsSceneLight_getPointLightVertices(traverseData->pointVerts,
				DS_POINT_LIGHT_VERTEX_COUNT, traverseData->pointIndices,
				DS_POINT_LIGHT_INDEX_COUNT, light, traverseData->intensityThreshold, firstIndex));

			traverseData->pointVerts += DS_POINT_LIGHT_VERTEX_COUNT;
			traverseData->pointIndices += DS_POINT_LIGHT_INDEX_COUNT;
			++traverseData->pointCount;
			break;
		}
		case dsSceneLightType_Spot:
		{
			uint16_t firstIndex = (uint16_t)((traverseData->spotCount % MAX_SPOT_LIGHTS)*
					DS_SPOT_LIGHT_VERTEX_COUNT);
			DS_VERIFY(dsSceneLight_getSpotLightVertices(traverseData->spotVerts,
				DS_SPOT_LIGHT_VERTEX_COUNT, traverseData->spotIndices,
				DS_SPOT_LIGHT_INDEX_COUNT, light, traverseData->intensityThreshold, firstIndex));

			traverseData->spotVerts += DS_SPOT_LIGHT_VERTEX_COUNT;
			traverseData->spotIndices += DS_SPOT_LIGHT_INDEX_COUNT;
			++traverseData->spotCount;
			break;
		}
	}

	return true;
}

const char* const dsDeferredLightResolve_typeName = "DeferredLightResolve";

uint64_t dsDeferredLightResolve_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	const dsMatrix44f* transform, dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(itemList);
	DS_UNUSED(node);
	DS_UNUSED(itemData);
	DS_UNUSED(transform);
	DS_UNUSED(thisItemData);
	return DS_NO_SCENE_NODE;
}

void dsDeferredLightResolve_removeNode(dsSceneItemList* itemList, uint64_t nodeID)
{
	DS_UNUSED(itemList);
	DS_UNUSED(nodeID);
}

void dsDeferredLightResolve_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	dsDeferredLightResolve* resolve = (dsDeferredLightResolve*)itemList;
	dsRenderer* renderer = commandBuffer->renderer;
	BufferInfo* buffers = getDrawBuffers(resolve, renderer);
	if (!buffers)
		return;

	uint8_t* dstData = (uint8_t*)dsGfxBuffer_map(buffers->buffer, dsGfxBufferMap_Write, 0,
		DS_MAP_FULL_BUFFER);
	if (!DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dstData != NULL))
		return;

	// Populate ambient data.
	dsColor3f ambientColor;
	DS_VERIFY(dsSceneLightSet_getAmbient(&ambientColor, resolve->lightSet));
	dsAmbientLightVertex* ambientVerts =
		(dsAmbientLightVertex*)(dstData + resolve->ambientVertexOffset);
	uint16_t* ambientIndices = (uint16_t*)(dstData + resolve->ambientIndexOffset);
	DS_VERIFY(dsSceneLight_getAmbientLightVertices(ambientVerts, DS_AMBIENT_LIGHT_VERTEX_COUNT,
		ambientIndices, DS_AMBIENT_LIGHT_INDEX_COUNT, &ambientColor, 0));

	// Populate other light data.
	TraverseData traverseData =
	{
		buffers,
		(dsDirectionalLightVertex*)(dstData + resolve->directionalVertexOffset),
		(dsPointLightVertex*)(dstData + resolve->pointVertexOffset),
		(dsSpotLightVertex*)(dstData + resolve->spotVertexOffset),
		(uint16_t*)(dstData + resolve->directionalIndexOffset),
		(uint16_t*)(dstData + resolve->pointIndexOffset),
		(uint16_t*)(dstData + resolve->spotIndexOffset),
		resolve->intensityThreshold,
		0, 0, 0
	};

	dsSceneLightSet_forEachLightInFrustum(resolve->lightSet, &view->viewFrustum, &visitLights,
		&traverseData);
	DS_VERIFY(dsGfxBuffer_unmap(buffers->buffer));

	// Draw each set of lights.
	if (!DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsShader_bind(resolve->ambientShader, commandBuffer,
			resolve->ambientMaterial, view->globalValues, NULL)))
	{
		return;
	}

	dsDrawIndexedRange drawRange;
	drawRange.indexCount = DS_AMBIENT_LIGHT_INDEX_COUNT;
	drawRange.instanceCount = 1;
	drawRange.firstIndex = 0;
	drawRange.indexCount = DS_AMBIENT_LIGHT_INDEX_COUNT;
	drawRange.vertexOffset = 0;
	drawRange.firstInstance = 0;
	DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsRenderer_drawIndexed(renderer, commandBuffer,
		buffers->ambientGometry, &drawRange, dsPrimitiveType_TriangleList));

	DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsShader_unbind(resolve->ambientShader, commandBuffer));

	if (traverseData.directionalCount > 0)
	{
		if (!DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsShader_bind(resolve->directionalShader,
				commandBuffer, resolve->directionalMaterial, view->globalValues, NULL)))
		{
			return;
		}

		uint32_t maxLightVerts =
			MAX_DIRECTIONAL_LIGHTS*DS_DIRECTIONAL_LIGHT_VERTEX_COUNT;
		uint32_t maxLightIndices =
			MAX_DIRECTIONAL_LIGHTS*DS_DIRECTIONAL_LIGHT_INDEX_COUNT;
		uint32_t indexCount = traverseData.directionalCount*DS_DIRECTIONAL_LIGHT_INDEX_COUNT;
		for (uint32_t vertOffset = 0, indexOffset = 0; indexOffset < indexCount;
			vertOffset += maxLightVerts, indexOffset += maxLightIndices)
		{
			drawRange.vertexOffset = vertOffset;
			drawRange.indexCount = dsMin(maxLightIndices, indexCount - indexOffset);
			DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsRenderer_drawIndexed(renderer, commandBuffer,
				buffers->directionalGometry, &drawRange, dsPrimitiveType_TriangleList));
		}

		DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG,
			dsShader_unbind(resolve->directionalShader, commandBuffer));
	}

	if (traverseData.pointCount > 0)
	{
		if (!DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsShader_bind(resolve->pointShader, commandBuffer,
				resolve->pointMaterial, view->globalValues, NULL)))
		{
			return;
		}

		uint32_t maxLightVerts = MAX_POINT_LIGHTS*DS_POINT_LIGHT_VERTEX_COUNT;
		uint32_t maxLightIndices = MAX_POINT_LIGHTS*DS_POINT_LIGHT_INDEX_COUNT;
		uint32_t indexCount = traverseData.pointCount*DS_POINT_LIGHT_INDEX_COUNT;
		for (uint32_t vertOffset = 0, indexOffset = 0; indexOffset < indexCount;
			vertOffset += maxLightVerts, indexOffset += maxLightIndices)
		{
			drawRange.vertexOffset = vertOffset;
			drawRange.indexCount = dsMin(maxLightIndices, indexCount - indexOffset);
			DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsRenderer_drawIndexed(renderer, commandBuffer,
				buffers->pointGometry, &drawRange, dsPrimitiveType_TriangleList));
		}

		DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG,
			dsShader_unbind(resolve->pointShader, commandBuffer));
	}

	if (traverseData.spotCount > 0)
	{
		if (!DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsShader_bind(resolve->spotShader, commandBuffer,
				resolve->spotMaterial, view->globalValues, NULL)))
		{
			return;
		}

		uint32_t maxLightVerts = MAX_SPOT_LIGHTS*DS_SPOT_LIGHT_VERTEX_COUNT;
		uint32_t maxLightIndices = MAX_SPOT_LIGHTS*DS_SPOT_LIGHT_INDEX_COUNT;
		uint32_t indexCount = traverseData.spotCount*DS_SPOT_LIGHT_INDEX_COUNT;
		for (uint32_t vertOffset = 0, indexOffset = 0; indexOffset < indexCount;
			vertOffset += maxLightVerts, indexOffset += maxLightIndices)
		{
			drawRange.vertexOffset = vertOffset;
			drawRange.indexCount = dsMin(maxLightIndices, indexCount - indexOffset);
			DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsRenderer_drawIndexed(renderer, commandBuffer,
				buffers->spotGometry, &drawRange, dsPrimitiveType_TriangleList));
		}

		DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG,
			dsShader_unbind(resolve->spotShader, commandBuffer));
	}
}

dsDeferredLightResolve* dsDeferredLightResolve_create(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const char* name, const dsSceneLightSet* lightSet,
	dsShader* ambientShader, dsMaterial* ambientMaterial, dsShader* directionalShader,
	dsMaterial* directionalMaterial, dsShader* pointShader, dsMaterial* pointMaterial,
	dsShader* spotShader, dsMaterial* spotMaterial, float intensityThreshold)
{
	if (!allocator || !name || !lightSet || !ambientShader || !ambientMaterial ||
		!directionalShader || !directionalMaterial || !pointShader || !pointMaterial ||
		!spotShader || !spotMaterial || intensityThreshold <= 0)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG,
			"Deferred light resolve allocator must support freeing memory.");
		return NULL;
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsDeferredLightResolve)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsDeferredLightResolve* resolve = DS_ALLOCATE_OBJECT(&bufferAlloc, dsDeferredLightResolve);
	DS_ASSERT(resolve);

	dsSceneItemList* itemList = (dsSceneItemList*)resolve;
	itemList->allocator = allocator;
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsHashString(name);
	itemList->needsCommandBuffer = true;
	itemList->addNodeFunc = &dsDeferredLightResolve_addNode;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = &dsDeferredLightResolve_removeNode;
	itemList->commitFunc = &dsDeferredLightResolve_commit;
	itemList->destroyFunc = (dsDestroySceneItemListFunction)&dsDeferredLightResolve_destroy;

	resolve->resourceAllocator = resourceAllocator;
	resolve->lightSet = lightSet;
	resolve->ambientShader = ambientShader;
	resolve->ambientMaterial = ambientMaterial;
	resolve->directionalShader = directionalShader;
	resolve->directionalMaterial = directionalMaterial;
	resolve->pointShader = pointShader;
	resolve->pointMaterial = pointMaterial;
	resolve->spotShader = spotShader;
	resolve->spotMaterial = spotMaterial;
	resolve->intensityThreshold = intensityThreshold;

	size_t ambientVertexSize = sizeof(dsAmbientLightVertex)*DS_AMBIENT_LIGHT_VERTEX_COUNT;
	size_t directionalVertexSize =
		sizeof(dsDirectionalLightVertex)*DS_DIRECTIONAL_LIGHT_VERTEX_COUNT;
	size_t pointVertexSize = sizeof(dsPointLightVertex)*DS_POINT_LIGHT_VERTEX_COUNT;
	size_t spotVertexSize = sizeof(dsSpotLightVertex)*DS_SPOT_LIGHT_VERTEX_COUNT;

	size_t ambientIndexSize = sizeof(uint16_t)*DS_DIRECTIONAL_LIGHT_INDEX_COUNT;
	size_t directionalIndexSize = sizeof(uint16_t)*DS_DIRECTIONAL_LIGHT_INDEX_COUNT;
	size_t pointIndexSize = sizeof(uint16_t)*DS_POINT_LIGHT_INDEX_COUNT;
	size_t spotIndexSize = sizeof(uint16_t)*DS_SPOT_LIGHT_INDEX_COUNT;

	uint32_t maxLights = dsSceneLightSet_getMaxLights(lightSet);

	resolve->bufferSize = ambientVertexSize + ambientIndexSize + (directionalVertexSize +
		pointVertexSize + spotVertexSize + directionalIndexSize + pointIndexSize +
		spotIndexSize)*maxLights;

	resolve->ambientVertexOffset = 0;
	resolve->directionalVertexOffset = resolve->ambientVertexOffset + ambientVertexSize;
	resolve->pointVertexOffset = resolve->directionalVertexOffset + directionalVertexSize*maxLights;
	resolve->spotVertexOffset = resolve->pointVertexOffset + pointVertexSize*maxLights;

	resolve->ambientIndexOffset = resolve->spotVertexOffset + spotVertexSize*maxLights;
	resolve->directionalIndexOffset = resolve->ambientIndexOffset + ambientIndexSize;
	resolve->pointIndexOffset = resolve->directionalIndexOffset + directionalIndexSize*maxLights;
	resolve->spotIndexOffset = resolve->pointIndexOffset + pointIndexSize*maxLights;

	resolve->buffers = NULL;
	resolve->bufferCount = 0;
	resolve->maxBuffers = 0;

	return resolve;
}

dsShader* dsDeferredLightResolve_getAmbientShader(const dsDeferredLightResolve* resolve)
{
	if (!resolve)
	{
		errno = EINVAL;
		return NULL;
	}

	return resolve->ambientShader;
}

bool dsDeferredLightResolve_setAmbientShader(dsDeferredLightResolve* resolve, dsShader* shader)
{
	if (!resolve || !shader)
	{
		errno = EINVAL;
		return false;
	}

	resolve->ambientShader = shader;
	return true;
}

dsMaterial* dsDeferredLightResolve_getAmbientMaterial(const dsDeferredLightResolve* resolve)
{
	if (!resolve)
	{
		errno = EINVAL;
		return NULL;
	}

	return resolve->ambientMaterial;
}

bool dsDeferredLightResolve_setAmbientMaterial(dsDeferredLightResolve* resolve,
	dsMaterial* material)
{
	if (!resolve || !material)
	{
		errno = EINVAL;
		return false;
	}

	resolve->ambientMaterial = material;
	return true;
}

dsShader* dsDeferredLightResolve_getDirectionalShader(const dsDeferredLightResolve* resolve)
{
	if (!resolve)
	{
		errno = EINVAL;
		return NULL;
	}

	return resolve->directionalShader;
}

bool dsDeferredLightResolve_setDirectionalShader(dsDeferredLightResolve* resolve, dsShader* shader)
{
	if (!resolve || !shader)
	{
		errno = EINVAL;
		return false;
	}

	resolve->directionalShader = shader;
	return true;
}

dsMaterial* dsDeferredLightResolve_getDirectionalMaterial(const dsDeferredLightResolve* resolve)
{
	if (!resolve)
	{
		errno = EINVAL;
		return NULL;
	}

	return resolve->directionalMaterial;
}

bool dsDeferredLightResolve_setDirectionalMaterial(dsDeferredLightResolve* resolve,
	dsMaterial* material)
{
	if (!resolve || !material)
	{
		errno = EINVAL;
		return false;
	}

	resolve->directionalMaterial = material;
	return true;
}

dsShader* dsDeferredLightResolve_getPointShader(const dsDeferredLightResolve* resolve)
{
	if (!resolve)
	{
		errno = EINVAL;
		return NULL;
	}

	return resolve->pointShader;
}

bool dsDeferredLightResolve_setPointShader(dsDeferredLightResolve* resolve, dsShader* shader)
{
	if (!resolve || !shader)
	{
		errno = EINVAL;
		return false;
	}

	resolve->pointShader = shader;
	return true;
}

dsMaterial* dsDeferredLightResolve_getPointMaterial(const dsDeferredLightResolve* resolve)
{
	if (!resolve)
	{
		errno = EINVAL;
		return NULL;
	}

	return resolve->pointMaterial;
}

bool dsDeferredLightResolve_setPointMaterial(dsDeferredLightResolve* resolve, dsMaterial* material)
{
	if (!resolve || !material)
	{
		errno = EINVAL;
		return false;
	}

	resolve->pointMaterial = material;
	return true;
}

dsShader* dsDeferredLightResolve_getSpotShader(const dsDeferredLightResolve* resolve)
{
	if (!resolve)
	{
		errno = EINVAL;
		return NULL;
	}

	return resolve->spotShader;
}

bool dsDeferredLightResolve_setSpotShader(dsDeferredLightResolve* resolve, dsShader* shader)
{
	if (!resolve || !shader)
	{
		errno = EINVAL;
		return false;
	}

	resolve->spotShader = shader;
	return true;
}

dsMaterial* dsDeferredLightResolve_getSpotMaterial(const dsDeferredLightResolve* resolve)
{
	if (!resolve)
	{
		errno = EINVAL;
		return NULL;
	}

	return resolve->spotMaterial;
}

bool dsDeferredLightResolve_setSpotMaterial(dsDeferredLightResolve* resolve, dsMaterial* material)
{
	if (!resolve || !material)
	{
		errno = EINVAL;
		return false;
	}

	resolve->spotMaterial = material;
	return true;
}

float dsDeferredLightResolve_getIntensityThreshold(const dsDeferredLightResolve* resolve)
{
	if (!resolve)
	{
		errno = EINVAL;
		return 0;
	}

	return resolve->intensityThreshold;
}

bool dsDeferredLightResolve_setIntensityThreshold(dsDeferredLightResolve* resolve,
	float intensityThreshold)
{
	if (!resolve || intensityThreshold <= 0)
	{
		errno = EINVAL;
		return false;
	}

	resolve->intensityThreshold = intensityThreshold;
	return true;
}

void dsDeferredLightResolve_destroy(dsDeferredLightResolve* resolve)
{
	if (!resolve)
		return;

	for (uint32_t i = 0; i < resolve->bufferCount; ++i)
		freeBuffers(resolve->buffers + i);

	dsSceneItemList* itemList = (dsSceneItemList*)resolve;
	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

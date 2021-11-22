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
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Renderer.h>

#include <DeepSea/Scene/ItemLists/SceneItemList.h>

#include <DeepSea/SceneLighting/SceneLightSet.h>
#include <DeepSea/SceneLighting/SceneLight.h>
#include <DeepSea/SceneLighting/SceneLightShadows.h>
#include <DeepSea/SceneLighting/SceneShadowManager.h>

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
	dsDrawGeometry* lightGeometries[dsSceneLightType_Count];
	uint64_t lastUsedFrame;
} BufferInfo;

typedef struct ShadowLightDrawInfo
{
	dsShader* shader;
	dsMaterial* material;
	uint32_t transformGroupID;
	uint32_t textureID;
} ShadowLightDrawInfo;

typedef struct TraverseData
{
	dsDeferredLightResolve* resolve;
	BufferInfo* buffers;
	dsDirectionalLightVertex* directionalVerts;
	dsPointLightVertex* pointVerts;
	dsSpotLightVertex* spotVerts;

	uint16_t* lightIndices[dsSceneLightType_Count];
	uint32_t lightCounts[dsSceneLightType_Count];
	uint32_t shadowLightCounts[dsSceneLightType_Count];
} TraverseData;

struct dsDeferredLightResolve
{
	dsSceneItemList itemList;
	dsAllocator* resourceAllocator;

	const dsSceneLightSet* lightSet;
	const dsSceneShadowManager* shadowManager;
	dsDeferredLightDrawInfo ambientInfo;
	dsDeferredLightDrawInfo lightInfos[dsSceneLightType_Count];
	ShadowLightDrawInfo shadowLightInfos[dsSceneLightType_Count];
	uint32_t maxLights;
	float intensityThreshold;

	size_t bufferSize;

	size_t ambientVertexOffset;
	size_t lightVertexOffsets[dsSceneLightType_Count];

	size_t ambientIndexOffset;
	size_t lightIndexOffsets[dsSceneLightType_Count];
	const dsSceneLightShadows** lightShadows[dsSceneLightType_Count];
	dsSharedMaterialValues* shadowValues;

	BufferInfo* buffers;
	uint32_t bufferCount;
	uint32_t maxBuffers;
};

static void freeBuffers(BufferInfo* buffers)
{
	dsDrawGeometry_destroy(buffers->ambientGometry);
	for (int i = 0; i < dsSceneLightType_Count; ++i)
		dsDrawGeometry_destroy(buffers->lightGeometries[i]);
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

	dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS] = {NULL, NULL, NULL, NULL};

	dsIndexBuffer indexBuffer;
	indexBuffer.buffer = buffers->buffer;
	indexBuffer.indexSize = (uint32_t)sizeof(uint16_t);

	if (resolve->ambientInfo.shader)
	{
		dsVertexBuffer ambientVertices;
		ambientVertices.buffer = buffers->buffer;
		ambientVertices.offset = resolve->ambientVertexOffset;
		ambientVertices.count = DS_AMBIENT_LIGHT_VERTEX_COUNT;
		DS_VERIFY(dsSceneLight_getAmbientLightVertexFormat(&ambientVertices.format));

		vertexBuffers[0] = &ambientVertices;

		indexBuffer.buffer = buffers->buffer;
		indexBuffer.offset = resolve->ambientIndexOffset;
		indexBuffer.count = DS_DIRECTIONAL_LIGHT_INDEX_COUNT;
		indexBuffer.indexSize = (uint32_t)sizeof(uint16_t);
		buffers->ambientGometry = dsDrawGeometry_create(resourceManager, resolve->resourceAllocator,
			vertexBuffers, &indexBuffer);

		if (!buffers->ambientGometry)
		{
			freeBuffers(buffers);
			--resolve->bufferCount;
			return NULL;
		}
	}

	const uint32_t lightVertexCounts[dsSceneLightType_Count] =
	{
		DS_DIRECTIONAL_LIGHT_VERTEX_COUNT, DS_POINT_LIGHT_VERTEX_COUNT, DS_SPOT_LIGHT_VERTEX_COUNT
	};
	const uint32_t lightIndexCounts[dsSceneLightType_Count] =
	{
		DS_DIRECTIONAL_LIGHT_INDEX_COUNT, DS_POINT_LIGHT_INDEX_COUNT, DS_SPOT_LIGHT_INDEX_COUNT
	};
	for (int i = 0; i < dsSceneLightType_Count; ++i)
	{
		if (!resolve->lightInfos[i].shader && !resolve->shadowLightInfos[i].shader)
			continue;

		dsVertexBuffer vertices;
		vertices.buffer = buffers->buffer;
		vertices.offset = resolve->lightVertexOffsets[i];
		vertices.count = lightVertexCounts[i]*maxLights;
		switch (i)
		{
			case dsSceneLightType_Directional:
				DS_VERIFY(dsSceneLight_getDirectionalLightVertexFormat(&vertices.format));
				break;
			case dsSceneLightType_Point:
				DS_VERIFY(dsSceneLight_getPointLightVertexFormat(&vertices.format));
				break;
			case dsSceneLightType_Spot:
				DS_VERIFY(dsSceneLight_getSpotLightVertexFormat(&vertices.format));
				break;
			default:
				DS_ASSERT(false);
		}

		vertexBuffers[0] = &vertices;

		indexBuffer.offset = resolve->lightIndexOffsets[i];
		indexBuffer.count = lightIndexCounts[i]*maxLights;

		buffers->lightGeometries[i] = dsDrawGeometry_create(resourceManager,
			resolve->resourceAllocator, vertexBuffers, &indexBuffer);

		if (!buffers->lightGeometries[i])
		{
			freeBuffers(buffers);
			--resolve->bufferCount;
			return NULL;
		}
	}

	buffers->lastUsedFrame = renderer->frameNumber;
	return buffers;
}

static bool visitLights(void* userData, const dsSceneLightSet* lightSet, const dsSceneLight* light)
{
	DS_UNUSED(lightSet);
	TraverseData* traverseData = (TraverseData*)userData;
	dsDeferredLightResolve* resolve = traverseData->resolve;
	bool drawLight = resolve->lightInfos[light->type].shader != NULL;
	bool drawShadowLight = resolve->shadowLightInfos[light->type].shader != NULL;
	if (!drawLight && !drawShadowLight)
		return true;

	const dsSceneLightShadows* lightShadows = NULL;
	if (resolve->shadowManager)
	{
		lightShadows = dsSceneShadowManager_findShadowsForLightID(resolve->shadowManager,
			light->nameID);
	}

	if ((lightShadows && !drawShadowLight) || (!lightShadows && !drawLight))
		return true;

	uint32_t baseIndex;
	if (lightShadows)
	{
		uint32_t shadowLightIndex = traverseData->shadowLightCounts[light->type]++;
		resolve->lightShadows[light->type][shadowLightIndex] = lightShadows;
		baseIndex = resolve->maxLights - shadowLightIndex - 1;
	}
	else
		baseIndex = traverseData->lightCounts[light->type]++;

	// Store shadowed lights at the end of the respective index buffers so they can be drawn
	// separately.
	switch (light->type)
	{
		case dsSceneLightType_Directional:
		{
			DS_ASSERT(traverseData->directionalVerts);

			uint16_t firstIndex =
				(uint16_t)((baseIndex % MAX_DIRECTIONAL_LIGHTS)*DS_DIRECTIONAL_LIGHT_VERTEX_COUNT);
			DS_VERIFY(dsSceneLight_getDirectionalLightVertices(traverseData->directionalVerts +
					baseIndex*DS_DIRECTIONAL_LIGHT_VERTEX_COUNT,
				DS_DIRECTIONAL_LIGHT_VERTEX_COUNT, traverseData->lightIndices[light->type] +
					baseIndex*DS_DIRECTIONAL_LIGHT_INDEX_COUNT,
				DS_DIRECTIONAL_LIGHT_INDEX_COUNT, light, firstIndex));
			break;
		}
		case dsSceneLightType_Point:
		{
			DS_ASSERT(traverseData->pointVerts);

			uint16_t firstIndex =
				(uint16_t)((baseIndex % MAX_DIRECTIONAL_LIGHTS)*DS_POINT_LIGHT_VERTEX_COUNT);
			DS_VERIFY(dsSceneLight_getPointLightVertices(traverseData->pointVerts +
					baseIndex*DS_POINT_LIGHT_VERTEX_COUNT,
				DS_POINT_LIGHT_VERTEX_COUNT, traverseData->lightIndices[light->type] +
					baseIndex*DS_POINT_LIGHT_INDEX_COUNT,
				DS_POINT_LIGHT_INDEX_COUNT, light, resolve->intensityThreshold, firstIndex));
			break;
		}
		case dsSceneLightType_Spot:
		{
			DS_ASSERT(traverseData->spotVerts);

			uint16_t firstIndex =
				(uint16_t)((baseIndex % MAX_SPOT_LIGHTS)*DS_SPOT_LIGHT_VERTEX_COUNT);
			DS_VERIFY(dsSceneLight_getSpotLightVertices(traverseData->spotVerts +
					baseIndex*DS_SPOT_LIGHT_VERTEX_COUNT,
				DS_SPOT_LIGHT_VERTEX_COUNT, traverseData->lightIndices[light->type] +
					baseIndex*DS_SPOT_LIGHT_INDEX_COUNT,
				DS_SPOT_LIGHT_INDEX_COUNT, light, resolve->intensityThreshold, firstIndex));
			break;
		}
		default:
			DS_ASSERT(false);
			break;
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
	if (resolve->ambientInfo.shader)
	{
		dsColor3f ambientColor;
		DS_VERIFY(dsSceneLightSet_getAmbient(&ambientColor, resolve->lightSet));
		dsAmbientLightVertex* ambientVerts =
			(dsAmbientLightVertex*)(dstData + resolve->ambientVertexOffset);
		uint16_t* ambientIndices = (uint16_t*)(dstData + resolve->ambientIndexOffset);
		DS_VERIFY(dsSceneLight_getAmbientLightVertices(ambientVerts, DS_AMBIENT_LIGHT_VERTEX_COUNT,
			ambientIndices, DS_AMBIENT_LIGHT_INDEX_COUNT, &ambientColor, 0));
	}

	// Populate other light data.
	TraverseData traverseData =
	{
		resolve,
		buffers,
		NULL, NULL, NULL,
		{NULL, NULL, NULL},
		{0, 0, 0},
		{0, 0, 0}
	};

	dsSceneLightType lightType = dsSceneLightType_Directional;
	if (resolve->lightInfos[lightType].shader || resolve->shadowLightInfos[lightType].shader)
	{
		traverseData.directionalVerts = (dsDirectionalLightVertex*)(dstData +
			resolve->lightVertexOffsets[lightType]);
		traverseData.lightIndices[lightType] =
			(uint16_t*)(dstData + resolve->lightIndexOffsets[lightType]);
	}

	lightType = dsSceneLightType_Point;
	if (resolve->lightInfos[lightType].shader || resolve->shadowLightInfos[lightType].shader)
	{
		traverseData.pointVerts = (dsPointLightVertex*)(dstData +
			resolve->lightVertexOffsets[lightType]);
		traverseData.lightIndices[lightType] =
			(uint16_t*)(dstData + resolve->lightIndexOffsets[lightType]);
	}

	lightType = dsSceneLightType_Spot;
	if (resolve->lightInfos[lightType].shader || resolve->shadowLightInfos[lightType].shader)
	{
		traverseData.spotVerts = (dsSpotLightVertex*)(dstData +
			resolve->lightVertexOffsets[lightType]);
		traverseData.lightIndices[lightType] =
			(uint16_t*)(dstData + resolve->lightIndexOffsets[lightType]);
	}

	dsSceneLightSet_forEachLightInFrustum(resolve->lightSet, &view->viewFrustum, &visitLights,
		&traverseData);
	DS_VERIFY(dsGfxBuffer_unmap(buffers->buffer));

	// Draw each set of lights.
	dsDrawIndexedRange drawRange;
	drawRange.firstInstance = 0;
	drawRange.instanceCount = 1;
	if (resolve->ambientInfo.shader)
	{
		dsShader* shader = resolve->ambientInfo.shader;
		if (!DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsShader_bind(shader, commandBuffer,
				resolve->ambientInfo.material, view->globalValues, NULL)))
		{
			return;
		}

		drawRange.firstIndex = 0;
		drawRange.indexCount = DS_AMBIENT_LIGHT_INDEX_COUNT;
		drawRange.vertexOffset = 0;
		DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsRenderer_drawIndexed(renderer, commandBuffer,
			buffers->ambientGometry, &drawRange, dsPrimitiveType_TriangleList));

		DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsShader_unbind(shader, commandBuffer));
	}

	const uint32_t maxLightCounts[dsSceneLightType_Count] =
	{
		MAX_DIRECTIONAL_LIGHTS, MAX_POINT_LIGHTS, MAX_SPOT_LIGHTS
	};
	const uint32_t lightVertexCounts[dsSceneLightType_Count] =
	{
		DS_DIRECTIONAL_LIGHT_VERTEX_COUNT, DS_POINT_LIGHT_VERTEX_COUNT, DS_SPOT_LIGHT_VERTEX_COUNT
	};
	const uint32_t lightIndexCounts[dsSceneLightType_Count] =
	{
		DS_DIRECTIONAL_LIGHT_INDEX_COUNT, DS_POINT_LIGHT_INDEX_COUNT, DS_SPOT_LIGHT_INDEX_COUNT
	};

	// Normal non-shadowed lights.
	for (int i = 0; i < dsSceneLightType_Count; ++i)
	{
		dsShader* shader = resolve->lightInfos[i].shader;
		uint32_t lightCount = traverseData.lightCounts[i];
		if (!shader || lightCount == 0)
			continue;

		if (!DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsShader_bind(shader, commandBuffer,
				resolve->lightInfos[i].material, view->globalValues, NULL)))
		{
			return;
		}

		uint32_t maxLightVerts = maxLightCounts[i]*lightVertexCounts[i];
		uint32_t maxLightIndices = maxLightCounts[i]*lightIndexCounts[i];
		uint32_t indexCount = lightCount*lightIndexCounts[i];
		for (uint32_t vertOffset = 0, indexOffset = 0; indexOffset < indexCount;
			vertOffset += maxLightVerts, indexOffset += maxLightIndices)
		{
			drawRange.firstIndex = indexOffset;
			drawRange.indexCount = dsMin(maxLightIndices, indexCount - indexOffset);
			drawRange.vertexOffset = vertOffset;
			DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsRenderer_drawIndexed(renderer, commandBuffer,
				buffers->lightGeometries[i], &drawRange, dsPrimitiveType_TriangleList));
		}

		DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsShader_unbind(shader, commandBuffer));
	}

	// Shadowed lights. These need to be drawn one by one rather than as a group due to different
	// material values.
	for (int i = 0; i < dsSceneLightType_Count; ++i)
	{
		dsShader* shader = resolve->shadowLightInfos[i].shader;
		uint32_t lightCount = traverseData.shadowLightCounts[i];
		if (!shader || lightCount == 0)
			continue;

		if (!DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsShader_bind(resolve->shadowLightInfos[i].shader,
				commandBuffer, resolve->shadowLightInfos[i].material, view->globalValues, NULL)))
		{
			return;
		}

		DS_VERIFY(dsSharedMaterialValues_clear(resolve->shadowValues));
		uint32_t transformGroupID = resolve->shadowLightInfos[i].transformGroupID;
		uint32_t textureID = resolve->shadowLightInfos[i].textureID;
		uint32_t maxLightVerts = maxLightCounts[i]*lightVertexCounts[i];
		uint32_t maxLightIndices = maxLightCounts[i]*lightIndexCounts[i];
		drawRange.indexCount = lightIndexCounts[i];
		for (uint32_t j = 0; j < lightCount; ++j)
		{
			const dsSceneLightShadows* lightShadows = resolve->lightShadows[i][j];
			DS_ASSERT(lightShadows);
			if (dsSceneLightShadows_getSurfaceCount(lightShadows) == 0)
				continue;

			DS_VERIFY(dsSceneLightShadows_bindTransformGroup(lightShadows, resolve->shadowValues,
				transformGroupID));

			dsTexture* shadowTexture = dsSharedMaterialValues_getTextureID(view->globalValues,
				dsSceneLightShadows_getNameID(lightShadows));
			if (!shadowTexture)
			{
				DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "Couldn't find shadow texture '%s'.",
					dsSceneLightShadows_getName(lightShadows));
				continue;
			}

			DS_VERIFY(dsSharedMaterialValues_setTextureID(resolve->shadowValues, textureID,
				shadowTexture));
			if (!DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsShader_updateInstanceValues(shader,
					commandBuffer, resolve->shadowValues)))
			{
				continue;
			}

			// Shadow lights are at the end of the buffer.
			uint32_t index = resolve->maxLights - j - 1;
			uint32_t blockIndex = index/maxLightIndices;
			uint32_t blockOffset = index % maxLightIndices;
			drawRange.firstIndex = blockIndex*maxLightIndices + blockOffset*drawRange.indexCount;
			drawRange.vertexOffset = blockIndex*maxLightVerts;
			DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsRenderer_drawIndexed(renderer, commandBuffer,
				buffers->lightGeometries[i], &drawRange, dsPrimitiveType_TriangleList));
		}

		DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG, dsShader_unbind(shader, commandBuffer));
	}
}

dsDeferredLightResolve* dsDeferredLightResolve_create(dsAllocator* allocator,
	dsAllocator* resourceAllocator, const char* name, const dsSceneLightSet* lightSet,
	const dsSceneShadowManager* shadowManager, const dsDeferredLightDrawInfo* ambientInfo,
	const dsDeferredLightDrawInfo* lightInfos,
	const dsDeferredShadowLightDrawInfo* shadowLightInfos, float intensityThreshold)
{
	if (!allocator || !name || !lightSet || intensityThreshold <= 0)
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

	uint32_t maxLights = dsSceneLightSet_getMaxLights(lightSet);
	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsDeferredLightResolve)) + DS_ALIGNED_SIZE(nameLen);
	uint32_t maxShadowLights = 0;
	bool hasShadows = false;
	if (shadowManager && shadowLightInfos)
	{
		maxShadowLights = dsSceneShadowManager_getLightShadowsCount(shadowManager);
		maxShadowLights = dsMin(maxLights, maxShadowLights);
		for (uint32_t i = 0; i < dsSceneLightType_Count; ++i)
		{
			const dsDeferredShadowLightDrawInfo* curInfo = shadowLightInfos + i;
			if (!curInfo->shader || !curInfo->material || !curInfo->transformGroupName ||
				!curInfo->shadowTextureName)
			{
				continue;
			}

			fullSize += DS_ALIGNED_SIZE(sizeof(dsSceneLightShadows*)*maxShadowLights);
			hasShadows = true;
		}
	}
	if (hasShadows)
		fullSize += dsSharedMaterialValues_fullAllocSize(2);
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
	resolve->shadowManager = shadowManager;

	if (ambientInfo && ambientInfo->shader && ambientInfo->material)
		resolve->ambientInfo = *ambientInfo;
	else
		memset(&resolve->ambientInfo, 0, sizeof(resolve->ambientInfo));

	if (lightInfos)
	{
		for (int i = 0; i < dsSceneLightType_Count; ++i)
		{
			const dsDeferredLightDrawInfo* curInfo = lightInfos + i;
			if (curInfo->shader && curInfo->material)
				resolve->lightInfos[i] = *curInfo;
			else
				memset(resolve->lightInfos + i, 0, sizeof(dsDeferredLightDrawInfo));
		}
	}
	else
		memset(resolve->lightInfos, 0, sizeof(resolve->lightInfos));

	if (shadowManager && shadowLightInfos)
	{
		for (int i = 0; i < dsSceneLightType_Count; ++i)
		{
			const dsDeferredShadowLightDrawInfo* curInfo = shadowLightInfos + i;
			if (!curInfo->shader || !curInfo->material || !curInfo->transformGroupName ||
				!curInfo->shadowTextureName)
			{
				memset(resolve->shadowLightInfos + i, 0, sizeof(ShadowLightDrawInfo));
				continue;
			}

			ShadowLightDrawInfo* setInfo = resolve->shadowLightInfos + i;
			setInfo->shader = curInfo->shader;
			setInfo->material = curInfo->material;
			setInfo->transformGroupID = dsHashString(curInfo->transformGroupName);
			setInfo->textureID = dsHashString(curInfo->shadowTextureName);

			resolve->lightShadows[i] = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc,
				const dsSceneLightShadows*, maxShadowLights);
			DS_ASSERT(resolve->lightShadows[i]);
		}
	}
	else
		memset(resolve->shadowLightInfos, 0, sizeof(resolve->shadowLightInfos));

	resolve->maxLights = maxLights;
	resolve->intensityThreshold = intensityThreshold;

	// Compute the maximum size of a buffer and the offsets for each light type. This will be based
	// on the worst-case of the maximum number of lights of each light type, plus one ambient light.
	size_t ambientVertexSize = sizeof(dsAmbientLightVertex)*DS_AMBIENT_LIGHT_VERTEX_COUNT;
	size_t lightVertexSizes[dsSceneLightType_Count] =
	{
		sizeof(dsDirectionalLightVertex)*DS_DIRECTIONAL_LIGHT_VERTEX_COUNT,
		sizeof(dsPointLightVertex)*DS_POINT_LIGHT_VERTEX_COUNT,
		sizeof(dsSpotLightVertex)*DS_SPOT_LIGHT_VERTEX_COUNT
	};

	size_t ambientIndexSize = sizeof(uint16_t)*DS_DIRECTIONAL_LIGHT_INDEX_COUNT;
	size_t lightIndexSizes[dsSceneLightType_Count] =
	{
		sizeof(uint16_t)*DS_DIRECTIONAL_LIGHT_INDEX_COUNT,
		sizeof(uint16_t)*DS_POINT_LIGHT_INDEX_COUNT,
		sizeof(uint16_t)*DS_SPOT_LIGHT_INDEX_COUNT
	};

	// Don't allocate space for disabled light types.
	if (!resolve->ambientInfo.shader)
	{
		ambientVertexSize = 0;
		ambientIndexSize = 0;
	}

	for (int i = 0; i < dsSceneLightType_Count; ++i)
	{
		if (resolve->lightInfos[i].shader || resolve->shadowLightInfos[i].shader)
			continue;

		lightVertexSizes[i] = 0;
		lightIndexSizes[i] = 0;
	}

	resolve->bufferSize = ambientVertexSize + ambientIndexSize;
	for (int i = 0; i < dsSceneLightType_Count; ++i)
		resolve->bufferSize += (lightVertexSizes[i] + lightIndexSizes[i])*maxLights;

	size_t curOffset = 0;
	resolve->ambientVertexOffset = curOffset;
	curOffset += ambientVertexSize;
	for (int i = 0; i < dsSceneLightType_Count; ++i)
	{
		resolve->lightVertexOffsets[i] = curOffset;
		curOffset += lightVertexSizes[i]*maxLights;
	}

	resolve->ambientIndexOffset = curOffset;
	curOffset += ambientIndexSize;
	for (int i = 0; i < dsSceneLightType_Count; ++i)
	{
		resolve->lightIndexOffsets[i] = curOffset;
		curOffset += lightIndexSizes[i]*maxLights;
	}

	if (hasShadows)
	{
		resolve->shadowValues = dsSharedMaterialValues_create((dsAllocator*)&bufferAlloc, 2);
		DS_ASSERT(resolve->shadowValues);
	}
	else
		resolve->shadowValues = NULL;

	resolve->buffers = NULL;
	resolve->bufferCount = 0;
	resolve->maxBuffers = 0;

	return resolve;
}

dsShader* dsDeferredLightResolve_getAmbientShader(const dsDeferredLightResolve* resolve)
{
	if (!resolve)
		return NULL;

	return resolve->ambientInfo.shader;
}

bool dsDeferredLightResolve_setAmbientShader(dsDeferredLightResolve* resolve, dsShader* shader)
{
	if (!resolve || !shader || !resolve->ambientInfo.shader)
		return false;

	resolve->ambientInfo.shader = shader;
	return true;
}

dsMaterial* dsDeferredLightResolve_getAmbientMaterial(const dsDeferredLightResolve* resolve)
{
	if (!resolve)
		return NULL;

	return resolve->ambientInfo.material;
}

bool dsDeferredLightResolve_setAmbientMaterial(dsDeferredLightResolve* resolve,
	dsMaterial* material)
{
	if (!resolve || !material)
	{
		errno = EINVAL;
		return false;
	}

	if (!resolve->ambientInfo.shader)
	{
		errno = EPERM;
		return false;
	}

	resolve->ambientInfo.material = material;
	return true;
}

dsShader* dsDeferredLightResolve_getLightShader(const dsDeferredLightResolve* resolve,
	dsSceneLightType lightType)
{
	if (!resolve || lightType < 0 || lightType >= dsSceneLightType_Count)
		return NULL;

	return resolve->lightInfos[lightType].shader;
}

bool dsDeferredLightResolve_setLightShader(dsDeferredLightResolve* resolve,
	dsSceneLightType lightType, dsShader* shader)
{
	if (!resolve || !shader)
	{
		errno = EINVAL;
		return false;
	}

	if (lightType < 0 || lightType >= dsSceneLightType_Count)
	{
		errno = EINDEX;
		return false;
	}

	if (!resolve->lightInfos[lightType].shader)
	{
		errno = EPERM;
		return false;
	}

	resolve->lightInfos[lightType].shader = shader;
	return true;
}

dsMaterial* dsDeferredLightResolve_getLightMaterial(const dsDeferredLightResolve* resolve,
	dsSceneLightType lightType)
{
	if (!resolve || lightType < 0 || lightType >= dsSceneLightType_Count)
		return NULL;

	return resolve->lightInfos[lightType].material;
}

bool dsDeferredLightResolve_setLightMaterial(dsDeferredLightResolve* resolve,
	dsSceneLightType lightType, dsMaterial* material)
{
	if (!resolve || !material)
	{
		errno = EINVAL;
		return false;
	}

	if (lightType < 0 || lightType >= dsSceneLightType_Count)
	{
		errno = EINDEX;
		return false;
	}

	if (!resolve->lightInfos[lightType].shader)
	{
		errno = EPERM;
		return false;
	}

	resolve->lightInfos[lightType].material = material;
	return true;
}

dsShader* dsDeferredLightResolve_getShadowLightShader(const dsDeferredLightResolve* resolve,
	dsSceneLightType lightType)
{
	if (!resolve || lightType < 0 || lightType >= dsSceneLightType_Count)
		return NULL;

	return resolve->shadowLightInfos[lightType].shader;
}

bool dsDeferredLightResolve_setShadowLightShader(dsDeferredLightResolve* resolve,
	dsSceneLightType lightType, dsShader* shader)
{
	if (!resolve || !shader)
	{
		errno = EINVAL;
		return false;
	}

	if (lightType < 0 || lightType >= dsSceneLightType_Count)
	{
		errno = EINDEX;
		return false;
	}

	if (!resolve->shadowLightInfos[lightType].shader)
	{
		errno = EPERM;
		return false;
	}

	resolve->shadowLightInfos[lightType].shader = shader;
	return true;
}

dsMaterial* dsDeferredLightResolve_getShadowLightMaterial(const dsDeferredLightResolve* resolve,
	dsSceneLightType lightType)
{
	if (!resolve || lightType < 0 || lightType >= dsSceneLightType_Count)
		return NULL;

	return resolve->shadowLightInfos[lightType].material;
}

bool dsDeferredLightResolve_setShadowLightMaterial(dsDeferredLightResolve* resolve,
	dsSceneLightType lightType, dsMaterial* material)
{
	if (!resolve || !material)
	{
		errno = EINVAL;
		return false;
	}

	if (lightType < 0 || lightType >= dsSceneLightType_Count)
	{
		errno = EINDEX;
		return false;
	}

	if (!resolve->shadowLightInfos[lightType].shader)
	{
		errno = EPERM;
		return false;
	}

	resolve->shadowLightInfos[lightType].material = material;
	return true;
}

uint32_t dsDeferredLightResolve_getShadowLightTransformGroupID(
	const dsDeferredLightResolve* resolve, dsSceneLightType lightType)
{
	if (!resolve || lightType < 0 || lightType >= dsSceneLightType_Count)
		return 0;

	return resolve->shadowLightInfos[lightType].transformGroupID;
}

bool dsDeferredLightResolve_setShadowLightTransformGroupID(dsDeferredLightResolve* resolve,
	dsSceneLightType lightType, uint32_t groupID)
{
	if (!resolve || !groupID)
	{
		errno = EINVAL;
		return false;
	}

	if (lightType < 0 || lightType >= dsSceneLightType_Count)
	{
		errno = EINDEX;
		return false;
	}

	if (!resolve->shadowLightInfos[lightType].shader)
	{
		errno = EPERM;
		return false;
	}

	resolve->shadowLightInfos[lightType].transformGroupID = groupID;
	return true;
}

bool dsDeferredLightResolve_setShadowLightTransformGroupNameName(dsDeferredLightResolve* resolve,
	dsSceneLightType lightType, const char* groupName)
{
	if (!resolve || !groupName)
	{
		errno = EINVAL;
		return false;
	}

	if (lightType < 0 || lightType >= dsSceneLightType_Count)
	{
		errno = EINDEX;
		return false;
	}

	if (!resolve->shadowLightInfos[lightType].shader)
	{
		errno = EPERM;
		return false;
	}

	resolve->shadowLightInfos[lightType].transformGroupID = dsHashString(groupName);
	return true;
}

uint32_t dsDeferredLightResolve_getShadowLightTextureID(
	const dsDeferredLightResolve* resolve, dsSceneLightType lightType)
{
	if (!resolve || lightType < 0 || lightType >= dsSceneLightType_Count)
		return 0;

	return resolve->shadowLightInfos[lightType].textureID;
}

bool dsDeferredLightResolve_setShadowLightTexturwID(dsDeferredLightResolve* resolve,
	dsSceneLightType lightType, uint32_t textureID)
{
	if (!resolve || !textureID)
	{
		errno = EINVAL;
		return false;
	}

	if (lightType < 0 || lightType >= dsSceneLightType_Count)
	{
		errno = EINDEX;
		return false;
	}

	if (!resolve->shadowLightInfos[lightType].shader)
	{
		errno = EPERM;
		return false;
	}

	resolve->shadowLightInfos[lightType].textureID = textureID;
	return true;
}

bool dsDeferredLightResolve_setShadowLightTextureName(dsDeferredLightResolve* resolve,
	dsSceneLightType lightType, const char* textureName)
{
	if (!resolve || !textureName)
	{
		errno = EINVAL;
		return false;
	}

	if (lightType < 0 || lightType >= dsSceneLightType_Count)
	{
		errno = EINDEX;
		return false;
	}

	if (!resolve->shadowLightInfos[lightType].shader)
	{
		errno = EPERM;
		return false;
	}

	resolve->shadowLightInfos[lightType].textureID = dsHashString(textureName);
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

	dsSceneItemList* itemList = (dsSceneItemList*)resolve;
	dsSharedMaterialValues_destroy(resolve->shadowValues);

	for (uint32_t i = 0; i < resolve->bufferCount; ++i)
		freeBuffers(resolve->buffers + i);
	DS_VERIFY(dsAllocator_free(itemList->allocator, resolve->buffers));

	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

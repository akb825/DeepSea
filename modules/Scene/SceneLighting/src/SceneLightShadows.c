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

#include <DeepSea/SceneLighting/SceneLightShadows.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Shadows/CascadeSplits.h>
#include <DeepSea/Render/Shadows/ShadowProjection.h>
#include <DeepSea/Render/ProjectionParams.h>
#include <DeepSea/Render/Renderer.h>

#include <DeepSea/Scene/Types.h>

#include <DeepSea/SceneLighting/SceneLight.h>
#include <DeepSea/SceneLighting/SceneLightSet.h>

#include <string.h>

#define FRAME_DELAY 3
#define INVALID_INDEX (uint32_t)-1

typedef struct BufferInfo
{
	dsGfxBuffer* buffer;
	uint64_t lastUsedFrame;
} BufferInfo;

struct dsSceneLightShadows
{
	dsAllocator* allocator;
	dsResourceManager* resourceManager;
	const dsSceneLightSet* lightSet;
	dsSceneLightType lightType;
	uint32_t lightNameID;
	bool cascaded;
	bool valid;

	uint32_t committedMatrices;
	uint32_t totalMatrices;

	dsSceneShadowParams shadowParams;
	dsShadowProjection projections[6];

	BufferInfo* buffers;
	uint32_t bufferCount;
	uint32_t maxBuffers;
	uint32_t curBuffer;
	void* curBufferData;

	dsShaderVariableGroup* fallback;

	dsSpinlock lock;
};

typedef struct DirectionalLightData
{
	dsMatrix44f matrix;
	dsVector2f shadowDistance;
	dsVector2f padding0;
} DirectionalLightData;

typedef struct CascadedDirectionalLightData
{
	dsMatrix44f matrices[4];
	dsVector4f splitDistances;
	dsVector2f shadowDistance;
	dsVector2f padding0;
} CascadedDirectionalLightData;

typedef struct PointLightData
{
	dsMatrix44f matrices[6];
	dsVector2f shadowDistance;
	dsVector2f padding0;
} PointLightData;

typedef struct SpotLightData
{
	dsMatrix44f matrix;
	dsVector2f shadowDistance;
	dsVector2f padding0;
} SpotLightData;

static bool matrixGroupValid(const dsShaderVariableGroupDesc* matrixGroupDesc,
	dsSceneLightType lightType)
{
	switch (lightType)
	{
		case dsSceneLightType_Directional:
			if (matrixGroupDesc->elementCount == 2)
			{
				const dsShaderVariableElement* matrixElement = matrixGroupDesc->elements;
				const dsShaderVariableElement* distanceElement = matrixGroupDesc->elements + 1;
				return matrixElement->type == dsMaterialType_Mat4 && matrixElement->count == 0 &&
					distanceElement->type == dsMaterialType_Vec2 && distanceElement->count == 0;
			}
			else if (matrixGroupDesc->elementCount == 3)
			{
				const dsShaderVariableElement* matrixElement = matrixGroupDesc->elements;
				const dsShaderVariableElement* splitElement = matrixGroupDesc->elements + 1;
				const dsShaderVariableElement* distanceElement = matrixGroupDesc->elements + 2;
				return matrixElement->type == dsMaterialType_Mat4 && matrixElement->count == 4 &&
					splitElement->type == dsMaterialType_Vec4 && splitElement->count == 0 &&
					distanceElement->type == dsMaterialType_Vec2 && distanceElement->count == 0;
			}
			return false;
		case dsSceneLightType_Point:
			if (matrixGroupDesc->elementCount == 2)
			{
				const dsShaderVariableElement* matrixElement = matrixGroupDesc->elements;
				const dsShaderVariableElement* distanceElement = matrixGroupDesc->elements + 1;
				return matrixElement->type == dsMaterialType_Mat4 && matrixElement->count == 6 &&
					distanceElement->type == dsMaterialType_Vec2 && distanceElement->count == 0;
			}
			return false;
		case dsSceneLightType_Spot:
			if (matrixGroupDesc->elementCount == 2)
			{
				const dsShaderVariableElement* matrixElement = matrixGroupDesc->elements;
				const dsShaderVariableElement* distanceElement = matrixGroupDesc->elements + 1;
				return matrixElement->type == dsMaterialType_Mat4 && matrixElement->count == 0 &&
					distanceElement->type == dsMaterialType_Vec2 && distanceElement->count == 0;
			}
			return false;
	}

	DS_ASSERT(false);
	return false;
}

static void* getBufferData(dsSceneLightShadows* shadows)
{
	uint64_t frameNumber = shadows->resourceManager->renderer->frameNumber;
	shadows->curBuffer = INVALID_INDEX;

	// Look for any buffer large enough that's FRAME_DELAY number of frames earlier than the
	// current one.
	for (uint32_t i = 0; i < shadows->bufferCount; ++i)
	{
		if (shadows->buffers[i].lastUsedFrame + FRAME_DELAY <= frameNumber)
		{
			shadows->curBuffer = i;
			break;
		}
	}

	// Create a new buffer if one wasn't found.
	if (shadows->curBuffer == INVALID_INDEX)
	{
		if (!DS_RESIZEABLE_ARRAY_ADD(shadows->allocator, shadows->buffers, shadows->bufferCount,
				shadows->maxBuffers, 1))
		{
			return NULL;
		}

		size_t bufferSize;
		switch (shadows->lightType)
		{
			case dsSceneLightType_Directional:
				bufferSize = shadows->cascaded ? sizeof(CascadedDirectionalLightData) :
					sizeof(DirectionalLightData);
				break;
			case dsSceneLightType_Point:
				bufferSize = sizeof(dsMatrix44f)*6;
				break;
			case dsSceneLightType_Spot:
				bufferSize = sizeof(dsMatrix44f);
				break;
			default:
				DS_ASSERT(false);
				return NULL;
		}

		dsGfxBuffer* buffer = dsGfxBuffer_create(shadows->resourceManager, shadows->allocator,
			dsGfxBufferUsage_UniformBlock, dsGfxMemory_Stream | dsGfxMemory_Synchronize, NULL,
			bufferSize);
		if (!buffer)
		{
			--shadows->bufferCount;
			return NULL;
		}

		shadows->curBuffer = shadows->bufferCount - 1;
		shadows->buffers[shadows->curBuffer].buffer = buffer;
	}

	BufferInfo* curBuffer = shadows->buffers + shadows->curBuffer;
	curBuffer->lastUsedFrame = frameNumber;
	shadows->curBufferData = dsGfxBuffer_map(curBuffer->buffer, dsGfxBufferMap_Write, 0,
		DS_MAP_FULL_BUFFER);
	DS_ASSERT(shadows->curBufferData);
	return shadows->curBufferData;
}

dsSceneLightShadows* dsSceneLightShadows_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsSceneLightSet* lightSet, dsSceneLightType lightType,
	const char* lightName, const dsShaderVariableGroupDesc* matrixGroupDesc,
	const dsSceneShadowParams* shadowParams)
{
	if (!allocator || !resourceManager || !lightSet || !matrixGroupDesc || !shadowParams)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG,
			"Scene light shadows allocator must support freeing memory.");
		return NULL;
	}

	if (!matrixGroupValid(matrixGroupDesc, lightType))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG,
			"Scene light shadows allocator must support freeing memory.");
		return NULL;
	}

	bool cascaded = matrixGroupDesc->elements[0].count == 4;
	if (cascaded && (shadowParams->maxCascades < 1 || shadowParams->maxCascades > 4))
	{
		errno = EINVAL;
		return NULL;
	}

	bool needsFallback = dsShaderVariableGroup_useGfxBuffer(resourceManager);
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneLightShadows));
	if (needsFallback)
		fullSize += dsShaderVariableGroup_fullAllocSize(resourceManager, matrixGroupDesc);

	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return false;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneLightShadows* shadows = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneLightShadows);
	DS_ASSERT(shadows);

	shadows->allocator = dsAllocator_keepPointer(allocator);
	shadows->resourceManager = resourceManager;
	shadows->lightSet = lightSet;
	shadows->lightType = lightType;
	shadows->lightNameID = lightName ? dsHashString(lightName) : 0;
	shadows->cascaded = cascaded;
	shadows->valid = false;

	shadows->committedMatrices = 0;
	shadows->totalMatrices = 0;

	shadows->shadowParams = *shadowParams;

	shadows->buffers = NULL;
	shadows->bufferCount = 0;
	shadows->maxBuffers = 0;
	shadows->curBuffer = INVALID_INDEX;
	shadows->curBufferData = NULL;

	if (needsFallback)
	{
		shadows->fallback = dsShaderVariableGroup_create(resourceManager,
			(dsAllocator*)&bufferAlloc, NULL, matrixGroupDesc);
		DS_ASSERT(shadows->fallback);
	}
	else
		shadows->fallback = NULL;

	DS_VERIFY(dsSpinlock_initialize(&shadows->lock));
	return shadows;
}

bool dsSceneLightShadows_prepare(dsSceneLightShadows* shadows, const dsView* view)
{
	if (!shadows || !view)
	{
		errno = EINVAL;
		return false;
	}

	const dsSceneLight* light = dsSceneLightSet_findLightID(shadows->lightSet,
		shadows->lightNameID);
	if (!light || light->type != shadows->lightType)
	{
		shadows->valid = false;
		return true;
	}

	dsRenderer* renderer = shadows->resourceManager->renderer;
	dsProjectionParams shadowedProjection = view->projectionParams;

	const dsSceneShadowParams* shadowParams = &shadows->shadowParams;
	float nearPlane, farPlane;
	if (view->projectionParams.type == dsProjectionType_Perspective)
	{
		nearPlane = view->projectionParams.perspectiveParams.near;
		farPlane = dsMin(view->projectionParams.perspectiveParams.far, shadowParams->maxDistance);
		shadowedProjection.perspectiveParams.far = farPlane;
	}
	else
	{
		nearPlane = view->projectionParams.projectionPlanes.near;
		farPlane = dsMin(view->projectionParams.projectionPlanes.far, shadowParams->maxDistance);
		shadowedProjection.projectionPlanes.far = farPlane;
	}
	dsVector2f shadowDistance = {{shadowParams->fadeStartDistance, shadowParams->maxDistance}};

	// Check if the light is in view based on the max distance to show shadows.
	float intensityThreshold = dsSceneLightSet_getIntensityThreshold(shadows->lightSet);
	dsMatrix44f shadowedProjectionMtx;
	DS_VERIFY(dsProjectionParams_createMatrix(&shadowedProjectionMtx, &shadowedProjection,
		renderer));
	dsFrustum3f shadowedFrustum;
	DS_VERIFY(dsRenderer_frustumFromMatrix(&shadowedFrustum, renderer, &shadowedProjectionMtx));
	if (!dsSceneLight_isInFrustum(light, &shadowedFrustum, intensityThreshold))
	{
		shadows->valid = false;
		return true;
	}

	if (!shadows->fallback)
	{
		if (!getBufferData(shadows))
			return false;
	}

	shadows->valid = true;
	shadows->committedMatrices = 0;
	switch (shadows->lightType)
	{
		case dsSceneLightType_Directional:
		{
			bool uniform = view->projectionParams.type == dsProjectionType_Ortho;

			dsVector3f toLight;
			dsVector3_neg(toLight, light->direction);
			if (shadows->cascaded)
			{
				shadows->totalMatrices = dsComputeCascadeCount(nearPlane, farPlane,
					shadowParams->maxFirstSplitDist,
					shadowParams->cascadedExpFactor, shadowParams->maxCascades);
				if (shadows->totalMatrices == 0)
				{
					shadows->valid = false;
					return false;
				}

				dsVector4f splitDistances = {{farPlane, farPlane, farPlane, farPlane}};
				for (uint32_t i = 0; i < shadows->totalMatrices; ++i)
				{
					splitDistances.values[i] = dsComputeCascadeDistance(nearPlane, farPlane,
						shadowParams->cascadedExpFactor, i, shadows->totalMatrices);
				}

				if (shadows->fallback)
				{
					DS_VERIFY(dsShaderVariableGroup_setElementData(shadows->fallback, 1,
						&splitDistances, dsMaterialType_Vec4, 0, 1));
					DS_VERIFY(dsShaderVariableGroup_setElementData(shadows->fallback, 2,
						&shadowDistance, dsMaterialType_Vec2, 0, 1));
				}
				else
				{
					CascadedDirectionalLightData* data =
						(CascadedDirectionalLightData*)shadows->curBufferData;
					data->splitDistances = splitDistances;
					data->shadowDistance = shadowDistance;;
				}
			}
			else
			{
				shadows->totalMatrices = 1;
				if (shadows->fallback)
				{
					DS_VERIFY(dsShaderVariableGroup_setElementData(shadows->fallback, 1,
						&shadowDistance, dsMaterialType_Vec2, 0, 1));
				}
				else
				{
					DirectionalLightData* data = (DirectionalLightData*)shadows->curBufferData;
					data->shadowDistance = shadowDistance;
				}
			}

			for (uint32_t i = 0; i < shadows->totalMatrices; ++i)
			{
				DS_VERIFY(dsShadowProjection_initialize(shadows->projections + i, renderer,
					&view->cameraMatrix, &toLight, NULL, uniform));
			}
			return true;
		}
		case dsSceneLightType_Point:
		{
			shadows->totalMatrices = 6;
			for (int i = 0; i < 6; ++i)
			{
				dsCubeFace cubeFace = (dsCubeFace)i;

				dsVector3f toLight;
				DS_VERIFY(dsTexture_cubeDirection(&toLight, cubeFace));
				dsVector3_neg(toLight, toLight);

				dsMatrix44f projection;
				DS_VERIFY(dsSceneLight_getPointLightProjection(&projection, light, renderer,
					cubeFace, intensityThreshold));

				DS_VERIFY(dsShadowProjection_initialize(shadows->projections + i, renderer,
					&view->cameraMatrix, &toLight, &projection, false));
			}

			if (shadows->fallback)
			{
				DS_VERIFY(dsShaderVariableGroup_setElementData(shadows->fallback, 1,
					&shadowDistance, dsMaterialType_Vec2, 0, 1));
			}
			else
			{
				PointLightData* data = (PointLightData*)shadows->curBufferData;
				data->shadowDistance = shadowDistance;;
			}
			return true;
		}
		case dsSceneLightType_Spot:
		{
			shadows->totalMatrices = 1;

			dsVector3f toLight;
			dsVector3_neg(toLight, light->direction);

			float intensityThreshold = dsSceneLightSet_getIntensityThreshold(shadows->lightSet);
			dsMatrix44f projection;
			DS_VERIFY(dsSceneLight_getSpotLightProjection(&projection, light, renderer,
				intensityThreshold));

			DS_VERIFY(dsShadowProjection_initialize(shadows->projections, renderer,
				&view->cameraMatrix, &toLight, &projection, false));

			if (shadows->fallback)
			{
				DS_VERIFY(dsShaderVariableGroup_setElementData(shadows->fallback, 1,
					&shadowDistance, dsMaterialType_Vec2, 0, 1));
			}
			else
			{
				SpotLightData* data = (SpotLightData*)shadows->curBufferData;
				data->shadowDistance = shadowDistance;;
			}
			return true;
		}
	}

	DS_ASSERT(false);
	return false;
}

bool dsSceneLightShadows_destroy(dsSceneLightShadows* shadows)
{
	if (!shadows)
		return true;

	for (uint32_t i = 0; i < shadows->bufferCount; ++i)
	{
		if (!dsGfxBuffer_destroy(shadows->buffers[i].buffer))
		{
			DS_ASSERT(i == 0);
			return false;
		}
	}

	dsSpinlock_shutdown(&shadows->lock);
	DS_VERIFY(dsAllocator_free(shadows->allocator, shadows->buffers));
	DS_VERIFY(dsShaderVariableGroup_destroy(shadows->fallback));
	DS_VERIFY(dsAllocator_free(shadows->allocator, shadows));
	return true;
}

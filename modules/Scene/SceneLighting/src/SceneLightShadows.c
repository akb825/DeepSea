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
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector3.h>

#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Shadows/CascadeSplits.h>
#include <DeepSea/Render/Shadows/ShadowCullVolume.h>
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
	uint32_t lightID;
	bool cascaded;

	uint32_t committedMatrices;
	uint32_t totalMatrices;

	dsSceneShadowParams shadowParams;
	dsShadowCullVolume cullVolumes[DS_MAX_SCENE_LIGHT_SHADOWS_SURFACES];
	dsShadowProjection projections[DS_MAX_SCENE_LIGHT_SHADOWS_SURFACES];
	uint32_t projectionSet[DS_MAX_SCENE_LIGHT_SHADOWS_SURFACES];

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

static bool transformGroupValid(const dsShaderVariableGroupDesc* transformGroupDesc,
	dsSceneLightType lightType)
{
	switch (lightType)
	{
		case dsSceneLightType_Directional:
			if (transformGroupDesc->elementCount == 2)
			{
				const dsShaderVariableElement* matrixElement = transformGroupDesc->elements;
				const dsShaderVariableElement* distanceElement = transformGroupDesc->elements + 1;
				return matrixElement->type == dsMaterialType_Mat4 && matrixElement->count == 0 &&
					distanceElement->type == dsMaterialType_Vec2 && distanceElement->count == 0;
			}
			else if (transformGroupDesc->elementCount == 3)
			{
				const dsShaderVariableElement* matrixElement = transformGroupDesc->elements;
				const dsShaderVariableElement* splitElement = transformGroupDesc->elements + 1;
				const dsShaderVariableElement* distanceElement = transformGroupDesc->elements + 2;
				return matrixElement->type == dsMaterialType_Mat4 && matrixElement->count == 4 &&
					splitElement->type == dsMaterialType_Vec4 && splitElement->count == 0 &&
					distanceElement->type == dsMaterialType_Vec2 && distanceElement->count == 0;
			}
			return false;
		case dsSceneLightType_Point:
			if (transformGroupDesc->elementCount == 2)
			{
				const dsShaderVariableElement* matrixElement = transformGroupDesc->elements;
				const dsShaderVariableElement* distanceElement = transformGroupDesc->elements + 1;
				return matrixElement->type == dsMaterialType_Mat4 && matrixElement->count == 6 &&
					distanceElement->type == dsMaterialType_Vec2 && distanceElement->count == 0;
			}
			return false;
		case dsSceneLightType_Spot:
			if (transformGroupDesc->elementCount == 2)
			{
				const dsShaderVariableElement* matrixElement = transformGroupDesc->elements;
				const dsShaderVariableElement* distanceElement = transformGroupDesc->elements + 1;
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

const char* const dsSceneLightShadows_typeName = "LightShadows";

static dsCustomSceneResourceType resourceType;
const dsCustomSceneResourceType* dsSceneLightShadows_type(void)
{
	return &resourceType;
}

dsSceneLightShadows* dsSceneLightShadows_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsSceneLightSet* lightSet, dsSceneLightType lightType,
	const char* lightName, const dsShaderVariableGroupDesc* transformGroupDesc,
	const dsSceneShadowParams* shadowParams)
{
	if (!allocator || !resourceManager || !lightSet || !transformGroupDesc || !shadowParams)
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

	if (!transformGroupValid(transformGroupDesc, lightType))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG,
			"Matrix group isn't valid for scene light shadows.");
		return NULL;
	}

	bool cascaded = transformGroupDesc->elements[0].count == 4;
	if (cascaded && (shadowParams->maxCascades < 1 || shadowParams->maxCascades > 4 ||
		shadowParams->maxFirstSplitDistance <= 0 || shadowParams->cascadeExpFactor < 0 ||
		shadowParams->cascadeExpFactor > 1 || shadowParams->fadeStartDistance < 0 ||
		shadowParams->maxDistance <= 0))
	{
		errno = EINVAL;
		return NULL;
	}

	bool needsFallback = dsShaderVariableGroup_useGfxBuffer(resourceManager);
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneLightShadows));
	if (needsFallback)
		fullSize += dsShaderVariableGroup_fullAllocSize(resourceManager, transformGroupDesc);

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
	shadows->lightID = dsHashString(lightName);
	shadows->cascaded = cascaded;

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
			(dsAllocator*)&bufferAlloc, NULL, transformGroupDesc);
		DS_ASSERT(shadows->fallback);
	}
	else
		shadows->fallback = NULL;

	DS_VERIFY(dsSpinlock_initialize(&shadows->lock));
	return shadows;
}

dsSceneLightType dsSceneLightShadows_getLightType(const dsSceneLightShadows* shadows)
{
	return shadows ? shadows->lightType : dsSceneLightType_Directional;
}

uint32_t dsSceneLightShadows_getLightID(const dsSceneLightShadows* shadows)
{
	return shadows ? shadows->lightID : 0;
}

bool dsSceneLightShadows_setLightID(dsSceneLightShadows* shadows, uint32_t lightID)
{
	if (!shadows)
	{
		errno = EINVAL;
		return false;
	}

	shadows->lightID = lightID;
	return true;
}

bool dsSceneLightShadows_setLightName(dsSceneLightShadows* shadows, const char* light)
{
	if (!shadows)
	{
		errno = EINVAL;
		return false;
	}

	shadows->lightID = dsHashString(light);
	return true;
}

uint32_t dsSceneLightShadows_getMaxCascades(const dsSceneLightShadows* shadows)
{
	if (!shadows || !shadows->cascaded)
		return 0;

	return shadows->shadowParams.maxCascades;
}

bool dsSceneLightShadows_setMaxCascades(dsSceneLightShadows* shadows, uint32_t maxCascades)
{
	if (!shadows || maxCascades < 1 || maxCascades > 4)
	{
		errno = EINVAL;
		return false;
	}

	shadows->shadowParams.maxCascades = maxCascades;
	return true;
}

float dsSceneLightShadows_getMaxFirstSplitDistance(const dsSceneLightShadows* shadows)
{
	return shadows ? shadows->shadowParams.maxFirstSplitDistance : 0.0f;
}

bool dsSceneLightShadows_setMaxFirstSplitDistance(dsSceneLightShadows* shadows, float maxDistance)
{
	if (!shadows || maxDistance <= 1)
	{
		errno = EINVAL;
		return false;
	}

	shadows->shadowParams.maxFirstSplitDistance = maxDistance;
	return true;
}

float dsSceneLightShadows_getCascadedExpFactor(const dsSceneLightShadows* shadows)
{
	return shadows ? shadows->shadowParams.cascadeExpFactor : 0.0f;
}

bool dsSceneLightShadows_setCascadedExpFactor(dsSceneLightShadows* shadows, float expFactor)
{
	if (!shadows || expFactor < 0 || expFactor > 1)
	{
		errno = EINVAL;
		return false;
	}

	shadows->shadowParams.cascadeExpFactor = expFactor;
	return true;
}

float dsSceneLightShadows_getFadeStartDistance(const dsSceneLightShadows* shadows)
{
	return shadows ? shadows->shadowParams.fadeStartDistance : 0.0f;
}

bool dsSceneLightShadows_setFadeStartDistance(dsSceneLightShadows* shadows, float distance)
{
	if (!shadows || distance < 0)
	{
		errno = EINVAL;
		return false;
	}

	shadows->shadowParams.fadeStartDistance = distance;
	return true;
}

float dsSceneLightShadows_getMaxDistance(const dsSceneLightShadows* shadows)
{
	return shadows ? shadows->shadowParams.maxDistance : 0.0f;
}

bool dsSceneLightShadows_setMaxDistance(dsSceneLightShadows* shadows, float distance)
{
	if (!shadows || distance <= 0)
	{
		errno = EINVAL;
		return false;
	}

	shadows->shadowParams.maxDistance = distance;
	return true;
}

bool dsSceneLightShadows_prepare(dsSceneLightShadows* shadows, const dsView* view,
	uint32_t transformGroupID)
{
	if (!shadows || !view)
	{
		errno = EINVAL;
		return false;
	}

	shadows->totalMatrices = 0;
	const dsSceneLight* light = dsSceneLightSet_findLightID(shadows->lightSet, shadows->lightID);
	if (!light || light->type != shadows->lightType)
	{
		dsSharedMaterialValues_removeValueID(view->globalValues, transformGroupID);
		return true;
	}

	dsRenderer* renderer = shadows->resourceManager->renderer;
	dsProjectionParams shadowedProjection = view->projectionParams;

	const dsSceneShadowParams* shadowParams = &shadows->shadowParams;
	float nearPlane = view->projectionParams.near;
	float farPlane = dsMin(view->projectionParams.far, shadowParams->maxDistance);
	shadowedProjection.far = farPlane;
	dsVector2f shadowDistance = {{shadowParams->fadeStartDistance, shadowParams->maxDistance}};

	// Check if the light is in view based on the max distance to show shadows.
	float intensityThreshold = dsSceneLightSet_getIntensityThreshold(shadows->lightSet);
	dsMatrix44f shadowedProjectionMtx;
	DS_VERIFY(dsProjectionParams_createMatrix(&shadowedProjectionMtx, &shadowedProjection,
		renderer));
	dsMatrix44f shadowedCullMtx;
	dsMatrix44_mul(shadowedCullMtx, shadowedProjectionMtx, view->cameraMatrix);
	dsFrustum3f shadowedFrustum;
	DS_VERIFY(dsRenderer_frustumFromMatrix(&shadowedFrustum, renderer, &shadowedCullMtx));
	if (!dsSceneLight_isInFrustum(light, &shadowedFrustum, intensityThreshold))
	{
		dsSharedMaterialValues_removeValueID(view->globalValues, transformGroupID);
		return true;
	}

	if (shadows->fallback)
	{
		if (!dsSharedMaterialValues_setVariableGroupID(view->globalValues, transformGroupID,
				shadows->fallback))
		{
			return false;
		}
	}
	else
	{
		if (!getBufferData(shadows))
		{
			dsSharedMaterialValues_removeValueID(view->globalValues, transformGroupID);
			return false;
		}

		dsGfxBuffer* buffer = shadows->buffers[shadows->curBuffer].buffer;
		if (!dsSharedMaterialValues_setBufferID(view->globalValues, transformGroupID, buffer, 0,
				buffer->size))
		{
			return false;
		}
	}

	shadows->committedMatrices = 0;
	memset(shadows->projectionSet, 0, sizeof(shadows->projectionSet));
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
					shadowParams->maxFirstSplitDistance, shadowParams->cascadeExpFactor,
					shadowParams->maxCascades);
				if (shadows->totalMatrices == 0)
				{
					dsSharedMaterialValues_removeValueID(view->globalValues, transformGroupID);
					return false;
				}

				dsVector4f splitDistances = {{farPlane, farPlane, farPlane, farPlane}};
				for (uint32_t i = 0; i < shadows->totalMatrices; ++i)
				{
					splitDistances.values[i] = dsComputeCascadeDistance(nearPlane, farPlane,
						shadowParams->cascadeExpFactor, i, shadows->totalMatrices);

					dsProjectionParams curProjection = shadowedProjection;
					curProjection.near = i == 0 ? nearPlane : splitDistances.values[i - 1];
					curProjection.far = farPlane;
					dsMatrix44f projectionMtx;
					DS_VERIFY(dsProjectionParams_createMatrix(
						&projectionMtx, &curProjection, renderer));
					dsMatrix44f cullMtx;
					dsMatrix44_mul(cullMtx, projectionMtx, view->cameraMatrix);
					dsFrustum3f frustum;
					DS_VERIFY(dsRenderer_frustumFromMatrix(&frustum, renderer, &cullMtx));
					DS_VERIFY(dsShadowCullVolume_buildDirectional(shadows->cullVolumes + i,
						&frustum, &toLight));
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

				DS_VERIFY(dsShadowCullVolume_buildDirectional(shadows->cullVolumes,
					&shadowedFrustum, &toLight));
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

				dsFrustum3f lightFrustum;
				DS_VERIFY(dsRenderer_frustumFromMatrix(&lightFrustum, renderer, &projection));

				DS_VERIFY(dsShadowCullVolume_buildSpot(shadows->cullVolumes + i, &shadowedFrustum,
					&lightFrustum));
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

			dsFrustum3f lightFrustum;
			DS_VERIFY(dsRenderer_frustumFromMatrix(&lightFrustum, renderer, &projection));

			DS_VERIFY(dsShadowCullVolume_buildSpot(shadows->cullVolumes, &shadowedFrustum,
				&lightFrustum));
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

uint32_t dsSceneLightShadows_getSurfaceCount(const dsSceneLightShadows* shadows)
{
	return shadows ? shadows->totalMatrices : 0;
}

dsIntersectResult dsSceneLightShadows_intersectAlignedBox(dsSceneLightShadows* shadows,
	uint32_t surface, const dsAlignedBox3f* box)
{
	if (!shadows || surface >= shadows->totalMatrices || !box)
		return dsIntersectResult_Outside;

	return dsShadowCullVolume_intersectAlignedBox(shadows->cullVolumes + surface, box,
		shadows->projections + surface);
}

dsIntersectResult dsSceneLightShadows_intersectOrientedBox(dsSceneLightShadows* shadows,
	uint32_t surface, const dsOrientedBox3f* box)
{
	if (!shadows || surface >= shadows->totalMatrices || !box)
		return dsIntersectResult_Outside;

	return dsShadowCullVolume_intersectOrientedBox(shadows->cullVolumes + surface, box,
		shadows->projections + surface);
}

dsIntersectResult dsSceneLightShadows_intersectSphere(dsSceneLightShadows* shadows,
	uint32_t surface, const dsVector3f* center, float radius)
{
	if (!shadows || surface >= shadows->totalMatrices || !center || radius < 0)
		return dsIntersectResult_Outside;

	return dsShadowCullVolume_intersectSphere(shadows->cullVolumes + surface, center, radius,
		shadows->projections + surface);
}

bool dsSceneLightShadows_computeSurfaceProjection(dsSceneLightShadows* shadows, uint32_t surface)
{
	if (!shadows)
	{
		errno = EINVAL;
		return false;
	}

	if (surface >= shadows->totalMatrices)
	{
		errno = EINDEX;
		return false;
	}

	uint32_t expectedSet = false;
	uint32_t isSet = true;
	if (!DS_ATOMIC_COMPARE_EXCHANGE32(shadows->projectionSet + surface, &expectedSet, &isSet,
			false))
	{
		errno = EPERM;
		return false;
	}

	dsMatrix44f shadowMtx;
	if (!dsShadowProjection_computeMatrix(&shadowMtx, shadows->projections + surface))
		dsMatrix44_identity(shadowMtx);

	switch (shadows->lightType)
	{
		case dsSceneLightType_Directional:
			if (shadows->cascaded)
			{
				if (shadows->fallback)
				{
					DS_VERIFY(dsShaderVariableGroup_setElementData(shadows->fallback, 0,
						&shadowMtx, dsMaterialType_Mat4, surface, 1));
				}
				else
				{
					CascadedDirectionalLightData* data =
						(CascadedDirectionalLightData*)shadows->curBufferData;
					data->matrices[surface] = shadowMtx;
				}
			}
			else
			{
				if (shadows->fallback)
				{
					DS_VERIFY(dsShaderVariableGroup_setElementData(shadows->fallback, 0, &shadowMtx,
						dsMaterialType_Mat4, 0, 1));
				}
				else
				{
					DirectionalLightData* data = (DirectionalLightData*)shadows->curBufferData;
					data->matrix = shadowMtx;
				}
			}
			break;
		case dsSceneLightType_Point:
			if (shadows->fallback)
			{
				DS_VERIFY(dsShaderVariableGroup_setElementData(shadows->fallback, 0,
					&shadowMtx, dsMaterialType_Mat4, surface, 1));
			}
			else
			{
				PointLightData* data = (PointLightData*)shadows->curBufferData;
				data->matrices[surface] = shadowMtx;
			}
			break;
		case dsSceneLightType_Spot:
			if (shadows->fallback)
			{
				DS_VERIFY(dsShaderVariableGroup_setElementData(shadows->fallback, 0, &shadowMtx,
					dsMaterialType_Mat4, 0, 1));
			}
			else
			{
				SpotLightData* data = (SpotLightData*)shadows->curBufferData;
				data->matrix = shadowMtx;
			}
			break;
	}

	// Pre-increment, so check against one minus the total value to see when we're finished.
	if (DS_ATOMIC_FETCH_ADD32(&shadows->committedMatrices, 1) == shadows->totalMatrices - 1)
	{
		if (shadows->fallback)
			DS_VERIFY(dsShaderVariableGroup_commitWithoutBuffer(shadows->fallback));
		else
		{
			DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG,
				dsGfxBuffer_unmap(shadows->buffers[shadows->curBuffer].buffer));
		}
	}

	return true;
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

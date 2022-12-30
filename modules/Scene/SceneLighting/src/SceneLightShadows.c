/*
 * Copyright 2021-2022 Aaron Barany
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

#include "SceneLightShadowsInternal.h"

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Geometry/OrientedBox3.h>

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
#define INVALID_INDEX ((uint32_t)-1)

struct ShadowBufferInfo
{
	dsGfxBuffer* buffer;
	uint64_t lastUsedFrame;
};

typedef struct DirectionalLightData
{
	dsMatrix44f matrix;
	dsVector2f shadowDistance;
	dsVector2f padding0;
	dsVector3f texCoordScale;
	float padding1;
	dsVector3f texCoordOffset;
	float padding2;
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
	dsVector3f lightViewPos;
	float padding1;
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
			if (transformGroupDesc->elementCount == 3)
			{
				const dsShaderVariableElement* matrixElement = transformGroupDesc->elements;
				const dsShaderVariableElement* distanceElement = transformGroupDesc->elements + 1;
				const dsShaderVariableElement* positionElement = transformGroupDesc->elements + 2;
				return matrixElement->type == dsMaterialType_Mat4 && matrixElement->count == 6 &&
					distanceElement->type == dsMaterialType_Vec2 && distanceElement->count == 0 &&
					positionElement->type == dsMaterialType_Vec3 && positionElement->count == 0;
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
		default:
			DS_ASSERT(false);
			return false;
	}
}

static void* getBufferData(dsSceneLightShadows* shadows)
{
	// Prevent error situations from never unmapping the buffer.
	if (shadows->curBufferData && shadows->curBuffer != INVALID_INDEX)
	{
		dsGfxBuffer_unmap(shadows->buffers[shadows->curBuffer].buffer);
		shadows->curBufferData = NULL;
	}

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
				bufferSize = sizeof(PointLightData);
				break;
			case dsSceneLightType_Spot:
				bufferSize = sizeof(SpotLightData);
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

	ShadowBufferInfo* curBuffer = shadows->buffers + shadows->curBuffer;
	curBuffer->lastUsedFrame = frameNumber;
	shadows->curBufferData = dsGfxBuffer_map(curBuffer->buffer, dsGfxBufferMap_Write, 0,
		DS_MAP_FULL_BUFFER);
	DS_ASSERT(shadows->curBufferData);
	return shadows->curBufferData;
}

static float getLargeBoxSize(float farPlane)
{
	// Arbitrary ratio to determine a large box that gets clamped to the shadow volume when
	// determining the extent of shadow space. Clamping can be error prone in some situations since
	// it doesn't check *all* intersections, but large boxes can cause the shadow projection to be
	// too large and reduce precision.
	const float ratio = 0.1f;
	return farPlane*ratio;
}

dsSceneLightShadows* dsSceneLightShadows_create(dsAllocator* allocator, const char* name,
	dsResourceManager* resourceManager, const dsSceneLightSet* lightSet, dsSceneLightType lightType,
	const char* lightName, const dsShaderVariableGroupDesc* transformGroupDesc,
	const char* transformGroupName, const dsSceneShadowParams* shadowParams)
{
	if (!allocator || !name || !resourceManager || !lightSet || !transformGroupDesc ||
		!shadowParams)
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
			"Transform group isn't valid for scene light shadows.");
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

	size_t nameLen = strlen(name) + 1;
	bool needsFallback = !dsShaderVariableGroup_useGfxBuffer(resourceManager);
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneLightShadows)) + DS_ALIGNED_SIZE(nameLen);
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

	char* nameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	memcpy(nameCopy, name, nameLen);
	shadows->name = nameCopy;
	shadows->nameID = dsHashString(name);

	shadows->resourceManager = resourceManager;
	shadows->lightSet = lightSet;
	shadows->lightType = lightType;
	shadows->lightID = lightName ? dsHashString(lightName) : 0;
	shadows->transformGroupID = transformGroupName ? dsHashString(transformGroupName) : 0;
	shadows->cascaded = cascaded;

	shadows->view = NULL;
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

const char* dsSceneLightShadows_getName(const dsSceneLightShadows* shadows)
{
	if (!shadows)
	{
		errno = EINVAL;
		return NULL;
	}

	return shadows->name;
}

uint32_t dsSceneLightShadows_getNameID(const dsSceneLightShadows* shadows)
{
	return shadows ? shadows->nameID : 0;
}

dsSceneLightType dsSceneLightShadows_getLightType(const dsSceneLightShadows* shadows)
{
	return shadows ? shadows->lightType : dsSceneLightType_Directional;
}

uint32_t dsSceneLightShadows_getLightID(const dsSceneLightShadows* shadows)
{
	return shadows ? shadows->lightID : 0;
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

bool dsSceneLightShadows_prepare(dsSceneLightShadows* shadows, const dsView* view)
{
	if (!shadows || !view)
	{
		errno = EINVAL;
		return false;
	}

	shadows->totalMatrices = 0;
	const dsSceneLight* light = dsSceneLightSet_findLightID(shadows->lightSet, shadows->lightID);
	if (!light || light->type != shadows->lightType)
		return true;

	dsRenderer* renderer = shadows->resourceManager->renderer;
	dsProjectionParams shadowedProjection = view->projectionParams;

	const dsSceneShadowParams* shadowParams = &shadows->shadowParams;
	float nearPlane = view->projectionParams.near;
	float farPlane = dsMin(view->projectionParams.far, shadowParams->maxDistance);
	shadowedProjection.far = farPlane;
	dsVector2f shadowDistance = {{shadowParams->fadeStartDistance, shadowParams->maxDistance}};
	bool uniform = view->projectionParams.type == dsProjectionType_Ortho;

	// Check if the light is in view based on the max distance to show shadows.
	float intensityThreshold = dsSceneLightSet_getIntensityThreshold(shadows->lightSet);
	dsMatrix44f shadowedProjectionMtx;
	DS_VERIFY(dsProjectionParams_createMatrix(&shadowedProjectionMtx, &shadowedProjection,
		renderer));
	dsMatrix44f shadowedCullMtx;
	dsMatrix44_mul(shadowedCullMtx, shadowedProjectionMtx, view->viewMatrix);
	dsFrustum3f cullFrustum;
	DS_VERIFY(dsRenderer_frustumFromMatrix(&cullFrustum, renderer, &shadowedCullMtx));
	if (!dsSceneLight_isInFrustum(light, &cullFrustum, intensityThreshold))
		return true;

	// Compute matrices in view space to be consistent with other lighting computations.
	dsFrustum3f shadowedFrustum;
	DS_VERIFY(dsRenderer_frustumFromMatrix(&shadowedFrustum, renderer, &shadowedProjectionMtx));
	shadows->view = view;

	if (!shadows->fallback && !getBufferData(shadows))
		return false;

	dsMatrix44f identity;
	dsMatrix44_identity(identity);
	shadows->committedMatrices = 0;
	memset(shadows->projectionSet, 0, sizeof(shadows->projectionSet));
	switch (shadows->lightType)
	{
		case dsSceneLightType_Directional:
		{
			// Compute in view space.
			shadows->largeBoxSize = getLargeBoxSize(farPlane);
			dsVector4f toLightWorld =
				{{-light->direction.x, -light->direction.y, -light->direction.z, 0.0f}};
			dsVector4f toLightView;
			dsMatrix44_transform(toLightView, view->viewMatrix, toLightWorld);
			if (shadows->cascaded)
			{
				shadows->totalMatrices = dsComputeCascadeCount(nearPlane, farPlane,
					shadowParams->maxFirstSplitDistance, shadowParams->cascadeExpFactor,
					shadowParams->maxCascades);
				if (shadows->totalMatrices == 0)
					return false;

				dsVector4f splitDistances = {{farPlane, farPlane, farPlane, farPlane}};
				for (uint32_t i = 0; i < shadows->totalMatrices; ++i)
				{
					splitDistances.values[i] = dsComputeCascadeDistance(nearPlane, farPlane,
						shadowParams->maxFirstSplitDistance, shadowParams->cascadeExpFactor, i,
						shadows->totalMatrices);

					dsProjectionParams curProjection = shadowedProjection;
					curProjection.near = i == 0 ? nearPlane : splitDistances.values[i - 1];
					curProjection.far = splitDistances.values[i];
					dsMatrix44f projectionMtx;
					DS_VERIFY(dsProjectionParams_createMatrix(
						&projectionMtx, &curProjection, renderer));
					dsFrustum3f frustum;
					DS_VERIFY(dsRenderer_frustumFromMatrix(&frustum, renderer, &projectionMtx));
					DS_VERIFY(dsShadowCullVolume_buildDirectional(shadows->cullVolumes + i,
						&frustum, (dsVector3f*)&toLightView));
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
					data->shadowDistance = shadowDistance;
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
					&shadowedFrustum, (dsVector3f*)&toLightView));
			}

			for (uint32_t i = 0; i < shadows->totalMatrices; ++i)
			{
				DS_VERIFY(dsShadowProjection_initialize(shadows->projections + i, renderer,
					&identity, (dsVector3f*)&toLightView, NULL, NULL, uniform));
			}
			break;
		}
		case dsSceneLightType_Point:
		{
			shadows->totalMatrices = 6;
			// Always clamp to the cull volume for point shadows.
			shadows->largeBoxSize = 0.0f;

			dsVector4f lightWorldPos =
				{{light->position.x,light->position.y, light->position.z, 1.0f}};
			dsVector4f lightViewPos;
			dsMatrix44_transform(lightViewPos, view->viewMatrix, lightWorldPos);

			dsMatrix44f projection;
			DS_VERIFY(dsSceneLight_getPointLightProjection(&projection, light, renderer,
				intensityThreshold));
			for (int i = 0; i < 6; ++i)
			{
				dsCubeFace cubeFace = (dsCubeFace)i;

				// Treat the orientation in view space to simplify things since it's arbitrary.
				dsMatrix44f lightWorld;
				DS_VERIFY(dsTexture_cubeOrientation(&lightWorld, cubeFace));
				lightWorld.columns[3] = lightViewPos;

				dsMatrix44f lightSpace;
				dsMatrix44_fastInvert(lightSpace, lightWorld);

				dsMatrix44f lightProjection;
				dsMatrix44_mul(lightProjection, projection, lightSpace);
				dsFrustum3f lightFrustum;
				DS_VERIFY(dsRenderer_frustumFromMatrix(&lightFrustum, renderer, &lightProjection));

				DS_VERIFY(dsShadowCullVolume_buildSpot(shadows->cullVolumes + i, &shadowedFrustum,
					&lightFrustum));
				// Force uniform shadows since they can be hard to tune depth bias with smaller
				// frustums and LiPSM.
				DS_VERIFY(dsShadowProjection_initialize(shadows->projections + i, renderer,
					&identity, (const dsVector3f*)(lightWorld.columns + 2), &lightSpace,
					&projection, true));
			}

			if (shadows->fallback)
			{
				DS_VERIFY(dsShaderVariableGroup_setElementData(shadows->fallback, 1,
					&shadowDistance, dsMaterialType_Vec2, 0, 1));
				DS_VERIFY(dsShaderVariableGroup_setElementData(shadows->fallback, 3,
					&lightViewPos, dsMaterialType_Vec3, 0, 1));
			}
			else
			{
				PointLightData* data = (PointLightData*)shadows->curBufferData;
				data->shadowDistance = shadowDistance;
				data->lightViewPos = *(dsVector3f*)&lightViewPos;
			}
			break;
		}
		case dsSceneLightType_Spot:
		{
			shadows->totalMatrices = 1;
			// Always clamp to the cull volume for spot shadows.
			shadows->largeBoxSize = 0.0f;

			// Compute in view space.
			dsVector4f toLightWorld =
				{{-light->direction.x, -light->direction.y, -light->direction.z, 0.0f}};
			dsVector4f toLightView;
			dsMatrix44_transform(toLightView, view->viewMatrix, toLightWorld);

			float intensityThreshold = dsSceneLightSet_getIntensityThreshold(shadows->lightSet);
			dsMatrix44f transform;
			DS_VERIFY(dsSceneLight_getSpotLightTransform(&transform, light));
			dsMatrix44f lightSpace;
			dsMatrix44_mul(lightSpace, transform, view->cameraMatrix);

			dsMatrix44f projection;
			DS_VERIFY(dsSceneLight_getSpotLightProjection(&projection, light, renderer,
				intensityThreshold));

			dsMatrix44f lightProjection;
			dsMatrix44_mul(lightProjection, projection, lightSpace);
			dsFrustum3f lightFrustum;
			DS_VERIFY(dsRenderer_frustumFromMatrix(&lightFrustum, renderer, &lightProjection));

			DS_VERIFY(dsShadowCullVolume_buildSpot(shadows->cullVolumes, &shadowedFrustum,
				&lightFrustum));
			// Force uniform shadows since they can be hard to tune depth bias with smaller
			// frustums and LiPSM.
			DS_VERIFY(dsShadowProjection_initialize(shadows->projections, renderer, &identity,
				(const dsVector3f*)&toLightView, &lightSpace, &projection, true));

			if (shadows->fallback)
			{
				DS_VERIFY(dsShaderVariableGroup_setElementData(shadows->fallback, 1,
					&shadowDistance, dsMaterialType_Vec2, 0, 1));
			}
			else
			{
				SpotLightData* data = (SpotLightData*)shadows->curBufferData;
				data->shadowDistance = shadowDistance;
			}
			break;
		}
		default:
			DS_ASSERT(false);
			return false;
	}

	if (shadows->transformGroupID)
	{
		return dsSceneLightShadows_bindTransformGroup(shadows, view->globalValues,
			shadows->transformGroupID);
	}
	return true;
}

bool dsSceneLightShadows_bindTransformGroup(
	const dsSceneLightShadows* shadows, dsSharedMaterialValues* materialValues, uint32_t nameID)
{
	if (!shadows || !materialValues)
	{
		errno = EINVAL;
		return false;
	}

	if (shadows->totalMatrices == 0)
	{
		errno = EPERM;
		return false;
	}

	if (shadows->fallback)
	{
		if (!dsSharedMaterialValues_setVariableGroupID(materialValues, nameID, shadows->fallback))
			return false;
	}
	else
	{
		dsGfxBuffer* buffer = shadows->buffers[shadows->curBuffer].buffer;
		if (!dsSharedMaterialValues_setBufferID(materialValues, nameID, buffer, 0, buffer->size))
			return false;
	}

	return true;
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

	dsVector3f size;
	dsAlignedBox3_extents(size, *box);
	float maxSize = dsMax(size.x, size.y);
	maxSize = dsMax(maxSize, size.z);
	bool clampToVolume = maxSize >= shadows->largeBoxSize;

	DS_ASSERT(shadows->view);
	dsOrientedBox3f viewBox;
	dsOrientedBox3_fromAlignedBox(viewBox, *box);
	DS_VERIFY(dsOrientedBox3f_transform(&viewBox, &shadows->view->viewMatrix));
	return dsShadowCullVolume_intersectOrientedBox(shadows->cullVolumes + surface, &viewBox,
		shadows->projections + surface, clampToVolume);
}

dsIntersectResult dsSceneLightShadows_intersectOrientedBox(dsSceneLightShadows* shadows,
	uint32_t surface, const dsOrientedBox3f* box)
{
	if (!shadows || surface >= shadows->totalMatrices || !box)
		return dsIntersectResult_Outside;

	float maxHalfSize = dsMax(box->halfExtents.x, box->halfExtents.y);
	maxHalfSize = dsMax(maxHalfSize, box->halfExtents.z);
	bool clampToVolume = maxHalfSize*2.0f >= shadows->largeBoxSize;

	DS_ASSERT(shadows->view);
	dsOrientedBox3f viewBox = *box;
	DS_VERIFY(dsOrientedBox3f_transform(&viewBox, &shadows->view->viewMatrix));
	return dsShadowCullVolume_intersectOrientedBox(shadows->cullVolumes + surface, &viewBox,
		shadows->projections + surface, clampToVolume);
}

dsIntersectResult dsSceneLightShadows_intersectSphere(dsSceneLightShadows* shadows,
	uint32_t surface, const dsVector3f* center, float radius)
{
	if (!shadows || surface >= shadows->totalMatrices || !center || radius < 0)
		return dsIntersectResult_Outside;

	bool clampToVolume = radius*2.0f >= shadows->largeBoxSize;

	DS_ASSERT(shadows->view);
	dsVector4f worldCenter = {{center->x, center->y, center->z, 1.0f}};
	dsVector4f viewCenter;
	dsMatrix44_transform(viewCenter, shadows->view->viewMatrix, worldCenter);
	return dsShadowCullVolume_intersectSphere(shadows->cullVolumes + surface,
		(dsVector3f*)&viewCenter, radius, shadows->projections + surface, clampToVolume);
}

dsIntersectResult dsSceneLightShadows_intersectViewAlignedBox(dsSceneLightShadows* shadows,
	uint32_t surface, const dsAlignedBox3f* box)
{
	if (!shadows || surface >= shadows->totalMatrices || !box)
		return dsIntersectResult_Outside;

	dsVector3f size;
	dsAlignedBox3_extents(size, *box);
	float maxSize = dsMax(size.x, size.y);
	maxSize = dsMax(maxSize, size.z);
	bool clampToVolume = maxSize >= shadows->largeBoxSize;

	return dsShadowCullVolume_intersectAlignedBox(shadows->cullVolumes + surface, box,
		shadows->projections + surface, clampToVolume);
}

dsIntersectResult dsSceneLightShadows_intersectViewOrientedBox(dsSceneLightShadows* shadows,
	uint32_t surface, const dsOrientedBox3f* box)
{
	if (!shadows || surface >= shadows->totalMatrices || !box)
		return dsIntersectResult_Outside;

	float maxHalfSize = dsMax(box->halfExtents.x, box->halfExtents.y);
	maxHalfSize = dsMax(maxHalfSize, box->halfExtents.z);
	bool clampToVolume = maxHalfSize*2.0f >= shadows->largeBoxSize;

	return dsShadowCullVolume_intersectOrientedBox(shadows->cullVolumes + surface, box,
		shadows->projections + surface, clampToVolume);
}

dsIntersectResult dsSceneLightShadows_intersectViewSphere(dsSceneLightShadows* shadows,
	uint32_t surface, const dsVector3f* center, float radius)
{
	if (!shadows || surface >= shadows->totalMatrices || !center || radius < 0)
		return dsIntersectResult_Outside;

	bool clampToVolume = radius*2.0f >= shadows->largeBoxSize;

	return dsShadowCullVolume_intersectSphere(shadows->cullVolumes + surface, center, radius,
		shadows->projections + surface, clampToVolume);
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

	const float paddingRatio = 0.1f;
	dsMatrix44f* shadowMtx = shadows->projectionMatrices + surface;
	uint32_t depthRangeIndex = shadows->cascaded ? surface : 0U;
	if (!dsShadowProjection_computeMatrix(shadowMtx, shadows->projections + surface, paddingRatio,
			shadows->shadowParams.minDepthRanges[depthRangeIndex]))
	{
		dsMatrix44_identity(*shadowMtx);
	}
;
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
					data->matrices[surface] = *shadowMtx;
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
					data->matrix = *shadowMtx;
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
				data->matrices[surface] = *shadowMtx;
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
				data->matrix = *shadowMtx;
			}
			break;
		default:
			DS_ASSERT(false);
			return false;
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
			shadows->curBufferData = NULL;
		}
	}

	return true;
}

const dsMatrix44fSIMD* dsSceneLightShadows_getSurfaceProjection(const dsSceneLightShadows* shadows,
	uint32_t surface)
{
	if (!shadows)
	{
		errno = EINVAL;
		return NULL;
	}

	if (surface >= shadows->totalMatrices)
	{
		errno = EINDEX;
		return NULL;
	}

	uint32_t committedMatrices;
	DS_ATOMIC_LOAD32(&shadows->committedMatrices, &committedMatrices);
	if (committedMatrices != shadows->totalMatrices)
	{
		errno = EPERM;
		return NULL;
	}

	return shadows->projectionMatrices + surface;
}

bool dsSceneLightShadows_destroy(dsSceneLightShadows* shadows)
{
	if (!shadows)
		return true;

	if (shadows->curBufferData && shadows->curBuffer != INVALID_INDEX)
	{
		if (!dsGfxBuffer_unmap(shadows->buffers[shadows->curBuffer].buffer))
			return false;
	}

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

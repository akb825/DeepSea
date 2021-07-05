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
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Shadows/CascadeSplits.h>
#include <DeepSea/Render/Shadows/ShadowProjection.h>

#include <DeepSea/Scene/Types.h>
#include <DeepSea/SceneLighting/SceneLightSet.h>

#include <string.h>

#define FRAME_DELAY 3

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
	uint32_t committedMatrices;
	uint32_t totalMatrices;

	dsSceneShadowParams shadowParams;
	dsShadowProjection projections[6];
	dsTexture* texture;

	BufferInfo* buffers;
	uint32_t bufferCount;
	uint32_t maxBuffers;
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
			if (matrixGroupDesc->elementCount == 1)
			{
				const dsShaderVariableElement* matrixElement = matrixGroupDesc->elements;
				return matrixElement->type == dsMaterialType_Mat4 && matrixElement->count == 6;
			}
			return false;
		case dsSceneLightType_Spot:
			if (matrixGroupDesc->elementCount == 1)
			{
				const dsShaderVariableElement* matrixElement = matrixGroupDesc->elements;
				return matrixElement->type == dsMaterialType_Mat4 && matrixElement->count == 0;
			}
			return false;
	}

	DS_ASSERT(false);
	return false;
}

static dsTexture* createTexture(dsSceneLightShadows* shadows)
{
	const dsSceneShadowParams* shadowParams = &shadows->shadowParams;
	dsTextureInfo textureInfo =
	{
		dsGfxFormat_D32_Float,
		dsTextureDim_2D,
		shadowParams->resolution,
		shadowParams->resolution,
		shadows->cascaded ? shadowParams->maxCascades : 0,
		0,
		1
	};
	return dsTexture_createOffscreen(shadows->resourceManager, shadows->allocator,
		dsTextureUsage_Texture, dsGfxMemory_GPUOnly, &textureInfo, false);
}

dsSceneLightShadows* dsSceneLightShadows_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsSceneLightSet* lightSet, dsSceneLightType lightType,
	const char* lightName, const dsShaderVariableGroupDesc* matrixGroupDesc,
	const char* matrixGroupName, const char* textureName, const dsSceneShadowParams* shadowParams)
{
	if (!allocator || !resourceManager || !lightSet || !matrixGroupDesc || !matrixGroupName ||
		!textureName || !shadowParams)
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
	shadows->committedMatrices = 0;

	switch (lightType)
	{
		case dsSceneLightType_Directional:
			shadows->totalMatrices = cascaded ? shadowParams->maxCascades : 1;
			break;
		case dsSceneLightType_Point:
			shadows->totalMatrices = 6;
			break;
		case dsSceneLightType_Spot:
			shadows->totalMatrices = 1;
			break;
	}

	shadows->shadowParams = *shadowParams;
	shadows->texture = NULL;

	shadows->buffers = NULL;
	shadows->bufferCount = 0;
	shadows->maxBuffers = 0;
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

	shadows->texture = createTexture(shadows);
	if (!shadows->texture)
	{
		dsSceneLightShadows_destroy(shadows);
		return NULL;
	}

	return shadows;
}

bool dsSceneLightShadows_destroy(dsSceneLightShadows* shadows)
{
	if (!shadows)
		return true;

	if (!dsTexture_destroy(shadows->texture))
		return false;

	for (uint32_t i = 0; i < shadows->bufferCount; ++i)
		DS_VERIFY(dsGfxBuffer_destroy(shadows->buffers[i].buffer));

	dsSpinlock_shutdown(&shadows->lock);
	DS_VERIFY(dsAllocator_free(shadows->allocator, shadows->buffers));
	DS_VERIFY(dsShaderVariableGroup_destroy(shadows->fallback));
	DS_VERIFY(dsAllocator_free(shadows->allocator, shadows));
	return true;
}

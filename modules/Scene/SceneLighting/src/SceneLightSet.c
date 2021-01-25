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

#include <DeepSea/SceneLighting/SceneLightSet.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/PoolAllocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Geometry/BVH.h>
#include <DeepSea/Math/Color.h>
#include <DeepSea/Math/Vector3.h>
#include <DeepSea/SceneLighting/SceneLight.h>

#include <float.h>

typedef struct LightNode
{
	dsHashTableNode node;
	uint32_t id;
	dsSceneLight light;
} LightNode;

struct dsSceneLightSet
{
	dsAllocator* allocator;
	dsPoolAllocator lightAllocator;
	dsHashTable* lightTable;
	dsSceneLight** directionalLights;
	dsBVH* spatialLights;
	uint32_t directionalLightCount;
	dsColor3f ambientColor;
	float ambientIntensity;
	float intensityThreshold;
};

typedef struct FindBrightestData
{
	const dsSceneLight** brightestLights;
	float* intensities;
	uint32_t* lightCount;
	const dsVector3f* position;
	uint32_t maxLights;
	float intensityThreshold;
} FindBrightestData;

typedef struct VisitLightData
{
	dsSceneLightVisitFunction visitFunc;
	const dsSceneLightSet* lightSet;
	void* userData;
	uint32_t count;
} VisitLightData;

static uint32_t identityHash(const void* key)
{
	return *(const uint32_t*)key;
}

static bool destroyResource(void* resource)
{
	dsSceneLightSet_destroy((dsSceneLightSet*)resource);
	return true;
}

static bool getLightBounds(void* outBounds, const dsBVH* bvh, const void* object)
{
	const dsSceneLightSet* lightSet = (const dsSceneLightSet*)dsBVH_getUserData(bvh);
	DS_ASSERT(lightSet);
	return dsSceneLight_computeBounds((dsAlignedBox3f*)outBounds, (const dsSceneLight*)object,
		lightSet->intensityThreshold);
}

static uint32_t findDimmestLight(const float* intensities, uint32_t lightCount)
{
	float dimmest = FLT_MAX;
	uint32_t index = 0;
	for (uint32_t i = 0; i < lightCount; ++i)
	{
		if (intensities[i] < dimmest)
		{
			dimmest = intensities[i];
			index = i;
		}
	}

	return index;
}

static bool visitBrightestLights(void* userData, const dsBVH* bvh, const void* object,
	const void* bounds)
{
	DS_UNUSED(bvh);
	DS_UNUSED(bounds);
	const dsSceneLight* light = (const dsSceneLight*)object;
	const FindBrightestData* data = (const FindBrightestData*)userData;

	float intensity = dsSceneLight_getIntensity(light, data->position);
	if (intensity < data->intensityThreshold)
		return true;

	if (*data->lightCount < data->maxLights)
	{
		data->intensities[*data->lightCount] = intensity;
		data->brightestLights[(*data->lightCount)++] = light;
	}
	else
	{
		uint32_t dimmest = findDimmestLight(data->intensities, data->maxLights);
		if (data->intensities[dimmest] < intensity)
		{
			data->intensities[dimmest] = intensity;
			data->brightestLights[dimmest] = light;
		}
	}

	return true;
}

static bool visitLightFunc(void* userData, const dsBVH* bvh, const void* object,
	const void* frustum)
{
	DS_UNUSED(bvh);
	VisitLightData* lightData = (VisitLightData*)userData;
	const dsSceneLight* light = (const dsSceneLight*)object;
	// Do a more precise check first.
	if (!dsSceneLight_isInFrustum(light, (const dsFrustum3f*)frustum,
			lightData->lightSet->intensityThreshold))
	{
		return true;
	}

	++lightData->count;
	return lightData->visitFunc(lightData->userData, lightData->lightSet, light);
}

const char* const dsSceneLightSet_typeName = "LightSet";

static dsCustomSceneResourceType resourceType;
const dsCustomSceneResourceType* dsSceneLightSet_type(void)
{
	return &resourceType;
}

dsSceneLightSet* dsSceneLightSet_create(dsAllocator* allocator, uint32_t maxLights,
	const dsVector3f* ambientColor, float ambientIntensity)
{
	if (!allocator || maxLights == 0 || !ambientColor)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LIGHTING_LOG_TAG,
			"Scene light set allocator must support freeing memory.");
		return NULL;
	}

	uint32_t lightTableSize = dsHashTable_getTableSize(maxLights);
	size_t lightTableBufferSize = dsHashTable_fullAllocSize(lightTableSize);
	size_t lightPoolSize = dsPoolAllocator_bufferSize(sizeof(LightNode), maxLights);
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneLightSet)) + lightPoolSize +
		lightTableBufferSize + DS_ALIGNED_SIZE(sizeof(dsSceneLight*)*maxLights);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneLightSet* lightSet = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneLightSet);
	DS_ASSERT(lightSet);

	lightSet->allocator = dsAllocator_keepPointer(allocator);

	void* lightPool = dsAllocator_alloc((dsAllocator*)&bufferAlloc, lightPoolSize);
	DS_ASSERT(lightPool);
	DS_VERIFY(dsPoolAllocator_initialize(&lightSet->lightAllocator, sizeof(LightNode), maxLights,
		lightPool, lightPoolSize));

	lightSet->lightTable = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		lightTableBufferSize);
	DS_ASSERT(lightSet->lightTable);
	DS_VERIFY(dsHashTable_initialize(lightSet->lightTable, lightTableSize, &identityHash,
		&dsHash32Equal));

	lightSet->directionalLights = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneLight*, maxLights);
	DS_ASSERT(lightSet->directionalLights);
	lightSet->directionalLightCount = 0;

	lightSet->spatialLights = dsBVH_create(allocator, 3, dsGeometryElement_Float, lightSet);
	if (!lightSet->spatialLights)
	{
		dsSceneLightSet_destroy(lightSet);
		return NULL;
	}

	lightSet->ambientColor = *ambientColor;
	lightSet->ambientIntensity = ambientIntensity;
	lightSet->intensityThreshold = 0.0f;

	return lightSet;
}

dsCustomSceneResource* dsSceneLightSet_createResource(dsAllocator* allocator,
	dsSceneLightSet* lightSet)
{
	if (!allocator || !lightSet)
	{
		errno = EINVAL;
		return NULL;
	}

	dsCustomSceneResource* customResource = DS_ALLOCATE_OBJECT(allocator, dsCustomSceneResource);
	if (!customResource)
		return NULL;

	customResource->allocator = dsAllocator_keepPointer(allocator);
	customResource->type = &resourceType;
	customResource->resource = lightSet;
	customResource->destroyFunc = &destroyResource;
	return customResource;
}

uint32_t dsSceneLightSet_getMaxLights(const dsSceneLightSet* lightSet)
{
	if (!lightSet)
		return 0;

	return (uint32_t)lightSet->lightAllocator.chunkCount;
}

uint32_t dsSceneLightSet_getRemainingLights(const dsSceneLightSet* lightSet)
{
	if (!lightSet)
		return 0;

	return (uint32_t)lightSet->lightAllocator.freeCount;
}

dsSceneLight* dsSceneLightSet_addLightName(dsSceneLightSet* lightSet, const char* name)
{
	if (!lightSet || !name)
	{
		errno = EINVAL;
		return NULL;
	}

	return dsSceneLightSet_addLightID(lightSet, dsHashString(name));
}

dsSceneLight* dsSceneLightSet_addLightID(dsSceneLightSet* lightSet, uint32_t nameID)
{
	if (!lightSet)
	{
		errno = EINVAL;
		return NULL;
	}

	LightNode* node = DS_ALLOCATE_OBJECT(&lightSet->lightAllocator, LightNode);
	if (!node)
		return NULL;

	node->id = nameID;
	if (!dsHashTable_insert(lightSet->lightTable, &node->id, (dsHashTableNode*)node, NULL))
	{
		DS_VERIFY(dsAllocator_free((dsAllocator*)&lightSet->lightAllocator, node));
		return NULL;
	}

	return &node->light;
}

dsSceneLight* dsSceneLightSet_findLightName(const dsSceneLightSet* lightSet, const char* name)
{
	if (!lightSet || !name)
		return NULL;

	return dsSceneLightSet_findLightID(lightSet, dsHashString(name));
}

dsSceneLight* dsSceneLightSet_findLightID(const dsSceneLightSet* lightSet, uint32_t nameID)
{
	if (!lightSet)
		return NULL;

	LightNode* node = (LightNode*)dsHashTable_find(lightSet->lightTable, &nameID);
	if (!node)
		return NULL;

	return &node->light;
}

bool dsSceneLightSet_removeLightName(dsSceneLightSet* lightSet, const char* name)
{
	if (!lightSet || !name)
		return false;

	return dsSceneLightSet_removeLightID(lightSet, dsHashString(name));
}

bool dsSceneLightSet_removeLightID(dsSceneLightSet* lightSet, uint32_t nameID)
{
	if (!lightSet)
		return false;

	LightNode* node = (LightNode*)dsHashTable_remove(lightSet->lightTable, &nameID);
	if (!node)
		return false;

	DS_VERIFY(dsAllocator_free((dsAllocator*)&lightSet->lightAllocator, node));
	return true;
}

bool dsSceneLightSet_clearLights(dsSceneLightSet* lightSet)
{
	if (!lightSet)
	{
		errno = EINVAL;
		return false;
	}

	for (dsListNode* node = lightSet->lightTable->list.head; node; node = node->next)
		DS_VERIFY(dsAllocator_free((dsAllocator*)&lightSet->lightAllocator, node));
	DS_VERIFY(dsHashTable_clear(lightSet->lightTable));
	return true;
}

const dsColor3f* dsSceneLightSet_getAmbientColor(const dsSceneLightSet* lightSet)
{
	if (!lightSet)
	{
		errno = EINVAL;
		return NULL;
	}

	return &lightSet->ambientColor;
}

bool dsSceneLightSet_setAmbientColor(dsSceneLightSet* lightSet, const dsColor3f* color)
{
	if (!lightSet || !color)
	{
		errno = EINVAL;
		return false;
	}

	lightSet->ambientColor = *color;
	return true;
}

float dsSceneLightSet_getAmbientIntensity(const dsSceneLightSet* lightSet)
{
	if (!lightSet)
		return 0.0f;

	return lightSet->ambientIntensity;
}

bool dsSceneLightSet_setAmbientIntensity(dsSceneLightSet* lightSet, float intensity)
{
	if (!lightSet)
	{
		errno = EINVAL;
		return false;
	}

	lightSet->ambientIntensity = intensity;
	return true;
}

bool dsSceneLightSet_getAmbient(dsColor3f* outAmbient, const dsSceneLightSet* lightSet)
{
	if (!outAmbient || !lightSet)
	{
		errno = EINVAL;
		return false;
	}

	dsVector3_scale(*outAmbient, lightSet->ambientColor, lightSet->ambientIntensity);
	return true;
}

bool dsSceneLightSet_setAmbient(dsSceneLightSet* lightSet, const dsColor3f* color,
	float intensity)
{
	if (!lightSet || !color)
	{
		errno = EINVAL;
		return false;
	}

	lightSet->ambientColor = *color;
	lightSet->ambientIntensity = intensity;
	return true;
}

bool dsSceneLightSet_prepare(dsSceneLightSet* lightSet, float intensityThreshold)
{
	if (!lightSet || intensityThreshold <= 0)
		return false;

	lightSet->intensityThreshold = intensityThreshold;
	lightSet->directionalLightCount = 0;

	// Use the remaining space in directionalLights for temporary storage for building the BVH.
	uint32_t maxLights = (uint32_t)lightSet->lightAllocator.chunkCount;
	uint32_t spatialLightCount = 0;
	dsListNode* node = lightSet->lightTable->list.head;
	while (node)
	{
		dsSceneLight* light = &((LightNode*)node)->light;
		node = node->next;

		float intensity = dsColor3f_grayscale(&light->color)*light->intensity;
		if (intensity < intensityThreshold)
			continue;

		if (light->type == dsSceneLightType_Directional)
			lightSet->directionalLights[lightSet->directionalLightCount++] = light;
		else
		{
			uint32_t index = maxLights - (++spatialLightCount);
			lightSet->directionalLights[index] = light;
		}
	}

	// Build a BVH for the spatial (point and spot) lights.
	if (spatialLightCount == 0)
		dsBVH_clear(lightSet->spatialLights);
	else
	{
		dsSceneLight** spatialLights = lightSet->directionalLights + maxLights - spatialLightCount;
		if (!dsBVH_build(lightSet->spatialLights, spatialLights, spatialLightCount,
				DS_GEOMETRY_OBJECT_POINTERS, &getLightBounds, false))
		{
			return false;
		}
	}

	return true;
}

uint32_t dsSceneLightSet_findBrightestLights(const dsSceneLight** outBrightestLights,
	uint32_t outLightCount, const dsSceneLightSet* lightSet, const dsVector3f* position)
{
	if (!outBrightestLights || !lightSet || !position)
	{
		errno = EINVAL;
		return 0;
	}

	uint32_t lightCount = 0;
	float* intensities = DS_ALLOCATE_STACK_OBJECT_ARRAY(float, outLightCount);

	// First check the directional lights.
	for (uint32_t i = 0; i < lightSet->directionalLightCount; ++i)
	{
		const dsSceneLight* light = lightSet->directionalLights[i];
		float intensity = dsColor3f_grayscale(&light->color)*light->intensity;
		if (lightCount < outLightCount)
		{
			intensities[lightCount] = intensity;
			outBrightestLights[lightCount++] = light;
		}
		else
		{
			uint32_t dimmest = findDimmestLight(intensities, lightCount);
			if (intensities[dimmest] < intensity)
			{
				intensities[dimmest] = intensity;
				outBrightestLights[dimmest] = light;
			}
		}
	}

	// Then check the spatial lights.
	dsAlignedBox3f bounds = {*position, *position};
	FindBrightestData visitData = {outBrightestLights, intensities, &lightCount, position,
		outLightCount, lightSet->intensityThreshold};
	dsBVH_intersectBounds(lightSet->spatialLights, &bounds, &visitBrightestLights, &visitData);

	// Set up the final count, nulling out any unset lights.
	for (uint32_t i = lightCount; i < outLightCount; ++i)
		outBrightestLights[i] = NULL;
	return lightCount;
}

uint32_t dsSceneLightSet_forEachLightInFrustum(const dsSceneLightSet* lightSet,
	const dsFrustum3f* frustum, dsSceneLightVisitFunction visitor, void* userData)
{
	if (!lightSet || !frustum)
		return 0;

	uint32_t directionalCount = 0;
	for (uint32_t i = 0; i < lightSet->directionalLightCount; ++i)
	{
		++directionalCount;
		if (visitor && !visitor(userData, lightSet, lightSet->directionalLights[i]))
			return directionalCount;
	}

	VisitLightData lightData = {visitor, lightSet, userData, directionalCount};
	dsBVH_intersectFrustum(lightSet->spatialLights, frustum, visitor ? &visitLightFunc : NULL,
		&lightData);
	return lightData.count;
}

void dsSceneLightSet_destroy(dsSceneLightSet* lightSet)
{
	if (!lightSet)
		return;

	dsBVH_destroy(lightSet->spatialLights);
	DS_VERIFY(dsAllocator_free(lightSet->allocator, lightSet));
}

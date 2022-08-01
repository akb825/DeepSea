/*
 * Copyright 2022 Aaron Barany
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

#include "SceneSSAOShared.h"

#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Random.h>

#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/Texture.h>

#include <DeepSea/SceneLighting/Types.h>

static const char* randomOffsetsName = "RandomOffsets";
static const char* randomRotationsName = "randomRotations";

bool dsSceneSSAO_canUseMaterial(const dsMaterial* material)
{
	const dsMaterialDesc* materialDesc = dsMaterial_getDescription(material);
	uint32_t index = dsMaterialDesc_findElement(materialDesc, randomOffsetsName);
	if (index == DS_MATERIAL_UNKNOWN)
	{
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "SSAO material doesn't contain element '%s'.",
			randomOffsetsName);
		return false;
	}

	const dsMaterialElement* element = materialDesc->elements + index;
	if (element->type != dsMaterialType_UniformBlock)
	{
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
			"SSAO material element '%s' must be a uniform block buffer with a vec3[%u] element "
			"with material binding.", randomOffsetsName, DS_MAX_SCENE_SSAO_SAMPLES);
		return false;
	}

	index = dsMaterialDesc_findElement(materialDesc, randomRotationsName);
	if (index == DS_MATERIAL_UNKNOWN)
	{
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "SSAO material doesn't contain element '%s'.",
			randomRotationsName);
		return false;
	}

	element = materialDesc->elements + index;
	if (element->type != dsMaterialType_Texture || element->binding != dsMaterialBinding_Material)
	{
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
			"SSAO material element '%s' must be a texture with material binding.",
			randomRotationsName);
		return randomRotationsName;
	}

	return true;
}

void dsSceneSSAO_setMaterialValues(dsMaterial* material, dsGfxBuffer* randomOffsets,
	dsTexture* randomRotations)
{
	const dsMaterialDesc* materialDesc = dsMaterial_getDescription(material);
	uint32_t index = dsMaterialDesc_findElement(materialDesc, randomOffsetsName);
	DS_ASSERT(index != DS_MATERIAL_UNKNOWN);
	DS_VERIFY(dsMaterial_setBuffer(material, index, randomOffsets, 0, randomOffsets->size));

	index = dsMaterialDesc_findElement(materialDesc, randomRotationsName);
	DS_ASSERT(index != DS_MATERIAL_UNKNOWN);
	DS_VERIFY(dsMaterial_setTexture(material, index, randomRotations));
}

dsGfxBuffer* dsSceneSSAO_createRandomOffsets(dsResourceManager* resourceManager,
	dsAllocator* allocator)
{
	dsRandom random;
	dsRandom_seed(&random, 0);
	dsVector4f randomOffsets[DS_MAX_SCENE_SSAO_SAMPLES];
	for (unsigned int i = 0; i < DS_MAX_SCENE_SSAO_SAMPLES; ++i)
	{
		// Spherical coordinates for a hemisphere.
		float theta = dsRandom_nextFloatRange(&random, 0.0f, (float)(2*M_PI));
		float phi = dsRandom_nextFloatRange(&random, 0.0f, (float)M_PI_2);

		// Randomly scale within the hemisphere, biasing towards the center with a reasonable
		// minimum.
		float scale = dsRandom_nextFloat(&random);
		scale *= scale;
		scale = dsLerp(0.1f, 1.0f, scale);

		float cosTheta = cosf(theta);
		float sinTheta = sinf(theta);
		float cosPhi = cosf(phi);
		float sinPhi = sinf(phi);

		dsVector4f* curSample = randomOffsets + i;
		curSample->x = cosTheta*cosPhi*scale;
		curSample->y = sinTheta*cosPhi*scale;
		curSample->z = sinPhi*scale;
		curSample->w = 0.0f;
	}

	return dsGfxBuffer_create(resourceManager, allocator, dsGfxBufferUsage_UniformBlock,
		dsGfxMemory_GPUOnly | dsGfxMemory_Static, randomOffsets, sizeof(randomOffsets));
}

dsTexture* dsSceneSSAO_createRandomRotations(dsResourceManager* resourceManager,
	dsAllocator* allocator)
{
	dsRandom random;
	dsRandom_seed(&random, 0);
	uint8_t randomRotations[DS_SCENE_SSAO_ROTATION_SIZE][DS_SCENE_SSAO_ROTATION_SIZE][2];
	for (uint32_t i = 0; i < DS_SCENE_SSAO_ROTATION_SIZE; ++i)
	{
		for (uint32_t j = 0; j < DS_SCENE_SSAO_ROTATION_SIZE; ++j)
		{
			float theta = dsRandom_nextFloatRange(&random, 0.0f, (float)(2*M_PI));
			float x = cosf(theta);
			float y = sinf(theta);
			randomRotations[i][j][0] = (uint8_t)roundf((x*0.5f + 0.5f)*255);
			randomRotations[i][j][1] = (uint8_t)roundf((y*0.5f + 0.5f)*255);
		}
	}
	dsTextureInfo textureInfo =
	{
		dsGfxFormat_decorate(dsGfxFormat_R8G8, dsGfxFormat_UNorm),
		dsTextureDim_2D,
		DS_SCENE_SSAO_ROTATION_SIZE,
		DS_SCENE_SSAO_ROTATION_SIZE,
		0,
		1,
		1
	};
	return dsTexture_create(resourceManager, allocator, dsTextureUsage_Texture,
		dsGfxMemory_GPUOnly | dsGfxMemory_Static, &textureInfo, randomRotations,
		sizeof(randomRotations));
}

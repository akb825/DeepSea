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

#include <DeepSea/SceneLighting/SceneSSAO.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Random.h>

#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Renderer.h>

#include <DeepSea/Scene/ItemLists/SceneFullScreenResolve.h>
#include <DeepSea/Scene/ItemLists/SceneItemList.h>

#include <limits.h>
#include <string.h>

struct dsSceneSSAO
{
	dsSceneItemList itemList;
	dsResourceManager* resourceManager;
	dsAllocator* resourceAllocator;
	dsShader* shader;
	dsMaterial* material;

	dsDrawGeometry* geometry;
	dsShaderVariableGroup* randomOffsets;
	dsTexture* randomRotations;
	dsVector4f randomOffsetValues[DS_MAX_SCENE_SSAO_SAMPLES];
	bool randomOffsetsSet;
};

static const char* randomOffsetsName = "randomOffsets";
static const char* randomRotationsName = "randomRotations";

static bool canUseMaterial(const dsMaterial* material)
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
	if (element->type != dsMaterialType_VariableGroup ||
		element->binding != dsMaterialBinding_Material ||
		element->shaderVariableGroupDesc->elementCount != 1 ||
		element->shaderVariableGroupDesc->elements[0].type != dsMaterialType_Vec3 ||
		element->shaderVariableGroupDesc->elements[0].count != DS_MAX_SCENE_SSAO_SAMPLES)
	{
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
			"SSAO material element '%s' must be a shader variable group with a vec3[%u] element "
			"with material binding.", randomOffsetsName, DS_MAX_SCENE_SSAO_SAMPLES);
		return false;
	}

	index = dsMaterialDesc_findElement(materialDesc, randomRotationsName);
	if (index == DS_MATERIAL_UNKNOWN)
	{
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG, "SSAO material doesn't contain element '%s'.",
			randomOffsetsName);
		return false;
	}

	element = materialDesc->elements + index;
	if (element->type != dsMaterialType_Texture || element->binding != dsMaterialBinding_Material)
	{
		DS_LOG_ERROR_F(DS_SCENE_LIGHTING_LOG_TAG,
			"SSAO material element '%s' must be a texture with material binding.",
			randomOffsetsName);
		return false;
	}

	return true;
}

static void setMaterialValues(dsSceneSSAO* ssao)
{
	const dsMaterialDesc* materialDesc = dsMaterial_getDescription(ssao->material);
	uint32_t index = dsMaterialDesc_findElement(materialDesc, randomOffsetsName);
	DS_ASSERT(index != DS_MATERIAL_UNKNOWN);
	DS_VERIFY(dsMaterial_setVariableGroup(ssao->material, index, ssao->randomOffsets));

	index = dsMaterialDesc_findElement(materialDesc, randomRotationsName);
	DS_ASSERT(index != DS_MATERIAL_UNKNOWN);
	DS_VERIFY(dsMaterial_setTexture(ssao->material, index, ssao->randomRotations));
}

const dsShaderVariableGroupDesc* getOffsetGroupDesc(const dsMaterial* material)
{
	const dsMaterialDesc* materialDesc = dsMaterial_getDescription(material);
	uint32_t index = dsMaterialDesc_findElement(materialDesc, randomOffsetsName);
	DS_ASSERT(index != DS_MATERIAL_UNKNOWN);
	return materialDesc->elements[index].shaderVariableGroupDesc;
}

void dsSceneSSAO_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	dsSceneSSAO* ssao = (dsSceneSSAO*)itemList;
	// Commit offsets when first encountered.
	if (!ssao->randomOffsetsSet)
	{
		DS_VERIFY(dsShaderVariableGroup_commit(ssao->randomOffsets, commandBuffer));
		ssao->randomOffsetsSet = true;
	}

	if (!DS_CHECK(DS_SCENE_LIGHTING_LOG_TAG,
			dsShader_bind(ssao->shader, commandBuffer, ssao->material, view->globalValues, NULL)))
	{
		return;
	}

	dsDrawRange drawRange = {4, 1, 0, 0};
	DS_CHECK(DS_SCENE_LOG_TAG, dsRenderer_draw(commandBuffer->renderer, commandBuffer,
		ssao->geometry, &drawRange, dsPrimitiveType_TriangleStrip));

	DS_CHECK(DS_SCENE_LOG_TAG, dsShader_unbind(ssao->shader, commandBuffer));
}

const char* const dsSceneSSAO_typeName = "SSAO";

dsSceneItemListType dsSceneSSAO_type(void)
{
	static int type;
	return &type;
}

dsSceneSSAO* dsSceneSSAO_create(dsAllocator* allocator, dsResourceManager* resourceManager,
	dsAllocator* resourceAllocator, const char* name, dsShader* shader, dsMaterial* material)
{
	if (!allocator || !resourceManager || !name || !shader || !material ||
		!canUseMaterial(material))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene SSAO allocator must support freeing memory.");
		return NULL;
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneSSAO)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneSSAO* ssao = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneSSAO);
	DS_ASSERT(ssao);

	dsSceneItemList* itemList = (dsSceneItemList*)ssao;
	itemList->allocator = dsAllocator_keepPointer(allocator);
	itemList->type = dsSceneSSAO_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsHashString(name);
	itemList->needsCommandBuffer = true;
	itemList->addNodeFunc = NULL;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = NULL;
	itemList->commitFunc = &dsSceneSSAO_commit;
	itemList->destroyFunc = (dsDestroySceneItemListFunction)&dsSceneSSAO_destroy;

	ssao->resourceManager = resourceManager;
	ssao->resourceAllocator = resourceAllocator;
	ssao->shader = shader;
	ssao->material = material;
	ssao->geometry = NULL;
	ssao->randomOffsets = NULL;
	ssao->randomRotations = NULL;
	ssao->randomOffsetsSet = false;

	ssao->geometry = dsSceneFullScreenResolve_createGeometry(resourceManager);
	if (!ssao->geometry)
	{
		dsSceneSSAO_destroy(ssao);
		return NULL;
	}

	const dsShaderVariableGroupDesc* offsetGroupDesc = getOffsetGroupDesc(material);
	ssao->randomOffsets = dsShaderVariableGroup_create(resourceManager, allocator,
		resourceAllocator, offsetGroupDesc);
	if (!ssao->randomOffsets)
	{
		dsSceneSSAO_destroy(ssao);
		return NULL;
	}

	uint32_t seed = 0;
	for (unsigned int i = 0; i < DS_MAX_SCENE_SSAO_SAMPLES; ++i)
	{
		// Spherical coordinates for a hemisphere.
		float theta = (float)dsRandomDouble(&seed, 0.0, 2*M_PI);
		float phi = (float)dsRandomDouble(&seed, 0.0, M_PI_2);

		// Randomly scale within the hemisphere, biasing towards the center with a reasonable
		// minimum.
		float scale = (float)dsRandomDouble(&seed, 0.0, 1.0);
		scale *= scale;
		scale = dsLerp(0.1f, 1.0f, scale);

		float cosTheta = cosf(theta);
		float sinTheta = sinf(theta);
		float cosPhi = cosf(phi);
		float sinPhi = sinf(phi);

		dsVector4f* curSample = ssao->randomOffsetValues + i;
		curSample->x = cosTheta*cosPhi*scale;
		curSample->y = sinTheta*cosPhi*scale;
		curSample->z = sinPhi*scale;
		curSample->w = 0.0f;
	}
	DS_VERIFY(dsShaderVariableGroup_setElementData(ssao->randomOffsets, 0, ssao->randomOffsetValues,
		dsMaterialType_Vec3, 0, DS_MAX_SCENE_SSAO_SAMPLES));

	uint8_t randomRotations[DS_SCENE_SSAO_ROTATION_SIZE][DS_SCENE_SSAO_ROTATION_SIZE][2];
	for (uint32_t i = 0; i < DS_SCENE_SSAO_ROTATION_SIZE; ++i)
	{
		for (uint32_t j = 0; j < DS_SCENE_SSAO_ROTATION_SIZE; ++j)
		{
			float theta = (float)dsRandomDouble(&seed, 0.0, 2*M_PI);
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
	ssao->randomRotations = dsTexture_create(resourceManager, resourceAllocator,
		dsTextureUsage_Texture, dsGfxMemory_GPUOnly | dsGfxMemory_Static, &textureInfo,
		randomRotations, sizeof(randomRotations));
	if (!ssao->randomRotations)
	{
		dsSceneSSAO_destroy(ssao);
		return NULL;
	}

	setMaterialValues(ssao);
	return ssao;
}

dsShader* dsSceneSSAO_getShader(const dsSceneSSAO* ssao)
{
	if (!ssao)
		return NULL;

	return ssao->shader;
}

bool dsSceneSSSAO_setShader(dsSceneSSAO* ssao, dsShader* shader)
{
	if (!ssao || !shader)
	{
		errno = EINVAL;
		return false;
	}

	ssao->shader = shader;
	return true;
}

dsMaterial* dsSceneSSAO_getMaterial(const dsSceneSSAO* ssao)
{
	if (!ssao)
		return NULL;

	return ssao->material;
}

bool dsSceneSSAO_setMaterial(dsSceneSSAO* ssao, dsMaterial* material)
{
	if (!ssao || !material || !canUseMaterial(material))
	{
		errno = EINVAL;
		return false;
	}

	const dsShaderVariableGroupDesc* offsetGroupDesc = getOffsetGroupDesc(material);
	if (offsetGroupDesc != dsShaderVariableGroup_getDescription(ssao->randomOffsets))
	{
		dsSceneItemList* itemList = (dsSceneItemList*)ssao;
		dsShaderVariableGroup* randomOffsets = dsShaderVariableGroup_create(ssao->resourceManager,
			itemList->allocator, ssao->resourceAllocator, offsetGroupDesc);
		if (!randomOffsets)
			return false;

		DS_VERIFY(dsShaderVariableGroup_setElementData(randomOffsets, 0, ssao->randomOffsetValues,
			dsMaterialType_Vec3, 0, DS_MAX_SCENE_SSAO_SAMPLES));
		DS_VERIFY(dsShaderVariableGroup_destroy(ssao->randomOffsets));
		ssao->randomOffsets = randomOffsets;
		ssao->randomOffsetsSet = false;
	}

	ssao->material = material;
	setMaterialValues(ssao);
	return true;
}

void dsSceneSSAO_destroy(dsSceneSSAO* ssao)
{
	if (!ssao)
		return;

	dsSceneItemList* itemList = (dsSceneItemList*)ssao;

	if (ssao->geometry)
		dsSceneFullScreenResolve_destroyGeometry();
	dsShaderVariableGroup_destroy(ssao->randomOffsets);
	dsTexture_destroy(ssao->randomRotations);

	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

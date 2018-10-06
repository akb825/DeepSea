/*
 * Copyright 2017 Aaron Barany
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

#include <DeepSea/VectorDraw/VectorShaderModule.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <string.h>

#if DS_WINDOWS
#include <malloc.h>
#else
#include <alloca.h>
#endif

// Transform group

// Uniforms
static const char* shapeInfoName = "dsVectorInfoTex";
static const char* materialInfoName = "dsVectorMaterialInfoTex";
static const char* materialColorName = "dsVectorMaterialColorTex";
static const char* textOutlineMaterialInfoName = "dsVectorTextOutlineMaterialInfoTex";
static const char* textOutlineMaterialColorName = "dsVectorTextOutlineMaterialColorTex";
static const char* otherTextureName = "dsVectorOtherTex";
static const char* modelViewProjectionName = "dsVectorModelViewProjection";
static const char* sizeName = "dsVectorImageSize";
static const char* textureSizesName = "dsVectorTextureSizes";

// Shaders
const char* dsDefaultVectorShaderNames[dsVectorShaderType_Count] =
{
	"dsVectorFillColor",          // dsVectorShaderType_FillColor
	"dsVectorFillLinearGradient", // dsVectorShaderType_FillLinearGradient
	"dsVectorFillRadialGradient", // dsVectorShaderType_FillRadialGradient
	"dsVectorLine",               // dsVectorShaderType_Line
	"dsVectorImage",              // dsVectorShaderType_Image
	"dsVectorTextColor",          // dsVectorShaderType_TextColor
	"dsVectorTextColorOutline",   // dsVectorShaderType_TextColorOutline
	"dsVectorTextGradient",       // dsVectorShaderType_TextGradient
	"dsVectorTextGradientOutline" // dsVectorShaderType_TextGradientOutline
};

static bool targetSupported(dsResourceManager* resourceManager)
{
	if (resourceManager->maxVertexSamplers < 3)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
			"Vertex texture lookup is required for vector images.");
		return false;
	}

	return true;
}

static dsVectorShaderModule* createVectorShaderModule(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsShaderModule* module, dsMaterialElement* customElements,
	uint32_t customElementCount)
{
	if (!allocator)
		allocator = resourceManager->allocator;
	DS_ASSERT(allocator);

	dsMaterialElement materialElements[] =
	{
		{shapeInfoName, dsMaterialType_Texture, 0, NULL, false, 0},
		{materialInfoName, dsMaterialType_Texture, 0, NULL, false, 0},
		{materialColorName, dsMaterialType_Texture, 0, NULL, false, 0},
		{textOutlineMaterialInfoName, dsMaterialType_Texture, 0, NULL, false, 0},
		{textOutlineMaterialColorName, dsMaterialType_Texture, 0, NULL, false, 0},
		{otherTextureName, dsMaterialType_Texture, 0, NULL, false, 0},
		{modelViewProjectionName, dsMaterialType_Mat4, 0, NULL, false, 0},
		{sizeName, dsMaterialType_Vec2, 0, NULL, false, 0},
		{textureSizesName, dsMaterialType_Vec3, 0, NULL, false, 0}
	};
	dsMaterialElement* finalMaterialElements = materialElements;
	uint32_t finalMaterialElementCount = DS_ARRAY_SIZE(materialElements);

	if (customElements && customElementCount > 0)
	{
		finalMaterialElementCount += customElementCount;
		finalMaterialElements = (dsMaterialElement*)alloca(
			sizeof(dsMaterialElement)*finalMaterialElementCount);
		memcpy(finalMaterialElements, materialElements,
			sizeof(dsMaterialElement)*DS_ARRAY_SIZE(materialElements));
		memcpy(finalMaterialElements + DS_ARRAY_SIZE(materialElements), customElements,
			sizeof(dsMaterialElement)*customElementCount);
	}

	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, allocator,
		finalMaterialElements, finalMaterialElementCount);
	if (!materialDesc)
	{
		DS_VERIFY(dsShaderModule_destroy(module));
		return NULL;
	}

	uint32_t shaderIndices[dsVectorShaderType_Count];
	uint32_t shaderCount = dsShaderModule_shaderCount(module);
	for (uint32_t i = 0; i < (uint32_t)dsVectorShaderType_Count; ++i)
	{
		shaderIndices[i] = DS_MATERIAL_UNKNOWN;
		for (uint32_t j = 0; j < shaderCount; ++j)
		{
			const char* name = dsShaderModule_shaderName(module, j);
			if (strcmp(name, dsDefaultVectorShaderNames[i]) == 0)
			{
				shaderIndices[i] = j;
				break;
			}
		}
	}

	dsVectorShaderModule* vectorModule = DS_ALLOCATE_OBJECT(allocator, dsVectorShaderModule);
	if (!vectorModule)
	{
		DS_VERIFY(dsShaderModule_destroy(module));
		DS_VERIFY(dsMaterialDesc_destroy(materialDesc));
		return NULL;
	}

	vectorModule->allocator = dsAllocator_keepPointer(allocator);
	vectorModule->shaderModule = module;
	vectorModule->materialDesc = materialDesc;
	vectorModule->shapeInfoTextureElement = dsMaterialDesc_findElement(materialDesc, shapeInfoName);
	DS_ASSERT(vectorModule->shapeInfoTextureElement != DS_MATERIAL_UNKNOWN);
	vectorModule->materialInfoTextureElement = dsMaterialDesc_findElement(materialDesc,
		materialInfoName);
	DS_ASSERT(vectorModule->materialInfoTextureElement != DS_MATERIAL_UNKNOWN);
	vectorModule->materialColorTextureElement = dsMaterialDesc_findElement(materialDesc,
		materialColorName);
	DS_ASSERT(vectorModule->materialColorTextureElement != DS_MATERIAL_UNKNOWN);
	vectorModule->textOutlineMaterialInfoTextureElement = dsMaterialDesc_findElement(materialDesc,
		textOutlineMaterialInfoName);
	DS_ASSERT(vectorModule->textOutlineMaterialInfoTextureElement != DS_MATERIAL_UNKNOWN);
	vectorModule->textOutlineMaterialColorTextureElement = dsMaterialDesc_findElement(materialDesc,
		textOutlineMaterialColorName);
	DS_ASSERT(vectorModule->textOutlineMaterialColorTextureElement != DS_MATERIAL_UNKNOWN);
	vectorModule->otherTextureElement = dsMaterialDesc_findElement(materialDesc, otherTextureName);
	DS_ASSERT(vectorModule->otherTextureElement != DS_MATERIAL_UNKNOWN);
	vectorModule->modelViewProjectionElement = dsMaterialDesc_findElement(materialDesc,
		modelViewProjectionName);
	DS_ASSERT(vectorModule->modelViewProjectionElement != DS_MATERIAL_UNKNOWN);
	vectorModule->sizeElement = dsMaterialDesc_findElement(materialDesc, sizeName);
	DS_ASSERT(vectorModule->sizeElement != DS_MATERIAL_UNKNOWN);
	vectorModule->textureSizesElement = dsMaterialDesc_findElement(materialDesc, textureSizesName);
	DS_ASSERT(vectorModule->textureSizesElement != DS_MATERIAL_UNKNOWN);
	memcpy(vectorModule->shaderIndices, shaderIndices, sizeof(uint32_t)*dsVectorShaderType_Count);

	return vectorModule;
}

dsVectorShaderModule* dsVectorShaderModule_loadFile(dsResourceManager* resourceManager,
	dsAllocator* allocator, const char* filePath, dsMaterialElement* customElements,
	uint32_t customElementCount)
{
	if (!targetSupported(resourceManager))
		return NULL;

	dsShaderModule* module = dsShaderModule_loadFile(resourceManager, allocator, filePath,
		"VectorImage");
	if (!module)
		return NULL;

	return createVectorShaderModule(resourceManager, allocator, module, customElements,
		customElementCount);
}

dsVectorShaderModule* dsVectorShaderModule_loadResource(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsFileResourceType type, const char* filePath,
	dsMaterialElement* customElements, uint32_t customElementCount)
{
	if (!targetSupported(resourceManager))
		return NULL;

	dsShaderModule* module = dsShaderModule_loadResource(resourceManager, allocator, type, filePath,
		"VectorImage");
	if (!module)
		return NULL;

	return createVectorShaderModule(resourceManager, allocator, module, customElements,
		customElementCount);
}

dsVectorShaderModule* dsVectorShaderModule_loadStream(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsStream* stream, dsMaterialElement* customElements,
	uint32_t customElementCount)
{
	if (!targetSupported(resourceManager))
		return NULL;

	dsShaderModule* module = dsShaderModule_loadStream(resourceManager, allocator, stream,
		"VectorImage");
	if (!module)
		return NULL;

	return createVectorShaderModule(resourceManager, allocator, module, customElements,
		customElementCount);
}

dsVectorShaderModule* dsVectorShaderModule_loadData(dsResourceManager* resourceManager,
	dsAllocator* allocator, const void* data, size_t size, dsMaterialElement* customElements,
	uint32_t customElementCount)
{
	if (!targetSupported(resourceManager))
		return NULL;

	dsShaderModule* module = dsShaderModule_loadData(resourceManager, allocator, data, size,
		"VectorImage");
	if (!module)
		return NULL;

	return createVectorShaderModule(resourceManager, allocator, module, customElements,
		customElementCount);
}

dsMaterial* dsVectorShaderModule_createMaterial(dsVectorShaderModule* shaderModule,
	dsAllocator* allocator)
{
	if (!shaderModule || !(allocator && !shaderModule->allocator))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = shaderModule->allocator;

	return dsMaterial_create(allocator, shaderModule->materialDesc);
}

bool dsVectorShaderModule_destroy(dsVectorShaderModule* shaderModule)
{
	if (!shaderModule)
		return true;

	if (!dsShaderModule_destroy(shaderModule->shaderModule))
		return false;
	DS_VERIFY(dsMaterialDesc_destroy(shaderModule->materialDesc));

	if (shaderModule->allocator)
		DS_VERIFY(dsAllocator_free(shaderModule->allocator, shaderModule));
	return true;
}

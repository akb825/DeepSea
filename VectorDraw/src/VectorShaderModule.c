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

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <string.h>

static dsVectorShaderModule* createVectorShaderModule(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsShaderModule* module)
{
	if (!allocator)
		allocator = resourceManager->allocator;
	DS_ASSERT(allocator);

	const char* transformName = "transform";
	const char* modelViewProjectionName = "modelViewProjection";
	const char* sizeName = "size";
	const char* textureSizesName = "textureSizes";
	dsShaderVariableElement transformElements[] =
	{
		{transformName, dsMaterialType_Mat3, 0},
		{modelViewProjectionName, dsMaterialType_Mat4, 0},
		{sizeName, dsMaterialType_Vec2, 0},
		{textureSizesName, dsMaterialType_Vec2, 0}
	};
	dsShaderVariableGroupDesc* transformDesc = dsShaderVariableGroupDesc_create(resourceManager,
		allocator, transformElements, DS_ARRAY_SIZE(transformElements));
	if (!transformDesc)
	{
		DS_VERIFY(dsShaderModule_destroy(module));
		return NULL;
	}

	const char* vectorTransformName = "dsVectorTransform";
	const char* shapeInfoName = "dsvectorInfoTex";
	const char* materialInfoName = "dsVectorMaterialInfoTex";
	const char* materialColorName = "dsVectorMaterialColorTex";
	const char* fontName = "dsVectorFontTex";
	dsMaterialElement materialElements[] =
	{
		{vectorTransformName, dsMaterialType_VariableGroup, 0, transformDesc, true, 0},
		{shapeInfoName, dsMaterialType_Texture, 0, transformDesc, true, 0},
		{materialInfoName, dsMaterialType_Texture, 0, transformDesc, true, 0},
		{materialColorName, dsMaterialType_Texture, 0, transformDesc, true, 0},
		{fontName, dsMaterialType_Texture, 0, transformDesc, true, 0},
	};
	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, allocator,
		materialElements, DS_ARRAY_SIZE(materialElements));
	if (!materialDesc)
	{
		DS_VERIFY(dsShaderModule_destroy(module));
		DS_VERIFY(dsShaderVariableGroupDesc_destroy(transformDesc));
		return NULL;
	}

	const char* shapeName = "dsVectorShape";
	const char* imageName = "dsVectorImage";
	const char* textName = "dsVectorText";
	uint32_t shapeIndex = DS_MATERIAL_UNKNOWN;
	uint32_t imageIndex = DS_MATERIAL_UNKNOWN;
	uint32_t textIndex = DS_MATERIAL_UNKNOWN;
	for (uint32_t i = 0, count = dsShaderModule_shaderCount(module); i < count &&
		(shapeIndex == DS_MATERIAL_UNKNOWN || imageIndex == DS_MATERIAL_UNKNOWN ||
		textIndex == DS_MATERIAL_UNKNOWN); ++i)
	{
		const char* name = dsShaderModule_shaderName(module, i);
		if (strcmp(name, shapeName) == 0)
			shapeIndex = i;
		else if (strcmp(name, imageName) == 0)
			imageIndex = i;
		else if (strcmp(name, textName) == 0)
			textIndex = i;
	}

	bool found = true;
	if (shapeIndex == DS_MATERIAL_UNKNOWN)
	{
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Vector shader module doesn't contain shader '%s'.",
			shapeName);
		found = false;
	}
	else if (imageIndex == DS_MATERIAL_UNKNOWN)
	{
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Vector shader module doesn't contain shader '%s'.",
			imageName);
		found = false;
	}
	if (textIndex == DS_MATERIAL_UNKNOWN)
	{
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Vector shader module doesn't contain shader '%s'.",
			textName);
		found = false;
	}

	if (!found)
	{
		DS_VERIFY(dsShaderModule_destroy(module));
		DS_VERIFY(dsShaderVariableGroupDesc_destroy(transformDesc));
		DS_VERIFY(dsMaterialDesc_destroy(materialDesc));
		return NULL;
	}

	dsVectorShaderModule* vectorModule = DS_ALLOCATE_OBJECT(allocator, dsVectorShaderModule);
	if (!vectorModule)
	{
		DS_VERIFY(dsShaderModule_destroy(module));
		DS_VERIFY(dsShaderVariableGroupDesc_destroy(transformDesc));
		DS_VERIFY(dsMaterialDesc_destroy(materialDesc));
		return NULL;
	}

	vectorModule->allocator = dsAllocator_keepPointer(allocator);
	vectorModule->shaderModule = module;
	vectorModule->transformDesc = transformDesc;
	vectorModule->materialDesc = materialDesc;
	vectorModule->transformElement = dsShaderVariableGroupDesc_findElement(transformDesc,
		transformName);
	DS_ASSERT(vectorModule->sizeElement != DS_MATERIAL_UNKNOWN);
	vectorModule->modelViewProjectionElement = dsShaderVariableGroupDesc_findElement(transformDesc,
		modelViewProjectionName);
	DS_ASSERT(vectorModule->sizeElement != DS_MATERIAL_UNKNOWN);
	vectorModule->sizeElement = dsShaderVariableGroupDesc_findElement(transformDesc, sizeName);
	DS_ASSERT(vectorModule->sizeElement != DS_MATERIAL_UNKNOWN);
	vectorModule->textureSizesElement = dsShaderVariableGroupDesc_findElement(transformDesc,
		textureSizesName);
	DS_ASSERT(vectorModule->textureSizesElement != DS_MATERIAL_UNKNOWN);
	vectorModule->transformId = dsHashString(transformName);
	vectorModule->shapeInfoTextureId = dsHashString(shapeInfoName);
	vectorModule->materialInfoTextureId = dsHashString(materialInfoName);
	vectorModule->materialColorTextureId = dsHashString(materialColorName);
	vectorModule->fontTextureId = dsHashString(fontName);
	vectorModule->shapeShaderIndex = shapeIndex;
	vectorModule->imageShaderIndex = imageIndex;
	vectorModule->textShaderIndex = textIndex;

	return vectorModule;
}

dsVectorShaderModule* dsVectorShaderModule_loadFile(dsResourceManager* resourceManager,
	dsAllocator* allocator, const char* filePath)
{
	dsShaderModule* module = dsShaderModule_loadFile(resourceManager, allocator, filePath,
		"VectorImage");
	if (!module)
		return NULL;

	return createVectorShaderModule(resourceManager, allocator, module);
}

dsVectorShaderModule* dsVectorShaderModule_loadStream(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsStream* stream)
{
	dsShaderModule* module = dsShaderModule_loadStream(resourceManager, allocator, stream,
		"VectorImage");
	if (!module)
		return NULL;

	return createVectorShaderModule(resourceManager, allocator, module);
}

dsVectorShaderModule* dsVectorShaderModule_loadData(dsResourceManager* resourceManager,
	dsAllocator* allocator, const void* data, size_t size)
{
	dsShaderModule* module = dsShaderModule_loadData(resourceManager, allocator, data, size,
		"VectorImage");
	if (!module)
		return NULL;

	return createVectorShaderModule(resourceManager, allocator, module);
}

dsVectorDrawContext* dsVectorShaderModule_createContext(dsVectorShaderModule* shaderModule,
	dsAllocator* allocator)
{
	if (!shaderModule || (!allocator && !shaderModule->allocator))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = shaderModule->allocator;

	return dsShaderVariableGroup_create(shaderModule->shaderModule->resourceManager, allocator,
		allocator, shaderModule->transformDesc);
}

bool dsVectorShaderModule_destroyContext(dsVectorDrawContext* drawContext)
{
	return dsShaderVariableGroup_destroy(drawContext);
}

bool dsVectorShaderModule_destroy(dsVectorShaderModule* shaderModule)
{
	if (!shaderModule)
		return true;

	if (!dsShaderModule_destroy(shaderModule->shaderModule))
		return false;

	if (shaderModule->allocator)
		DS_VERIFY(dsAllocator_free(shaderModule->allocator, shaderModule));
	return true;
}

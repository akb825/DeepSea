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

#include <DeepSea/VectorDraw/VectorShaders.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <string.h>

extern const char* dsDefaultShapeShaderName;
extern const char* dsDefaultImageShaderName;
extern const char* dsDefaultTextShaderName;

static dsVectorShaders* dsVectorShaders_createImpl(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsVectorShaderModule* shaderModule, uint32_t shapeShaderIndex,
	const char* shapeShaderName, uint32_t imageShaderIndex, const char* imageShaderName,
	uint32_t textShaderIndex, const char* textShaderName, uint32_t samples)
{
	if (!resourceManager || (!allocator && !resourceManager->allocator) || !shaderModule)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	if (shapeShaderIndex == DS_MATERIAL_UNKNOWN)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Vector shader module doesn't contain shader '%s'.",
			shapeShaderName);
		return NULL;
	}

	if (imageShaderIndex == DS_MATERIAL_UNKNOWN)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Vector shader module doesn't contain shader '%s'.",
			imageShaderName);
		return NULL;
	}

	// TODO: Implement text.
	/*if (textShaderIndex == DS_MATERIAL_UNKNOWN)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Vector shader module doesn't contain shader '%s'.",
			textShaderName);
		return NULL;
	}*/

	dsShader* shapeShader = dsShader_createIndex(resourceManager, allocator,
		shaderModule->shaderModule, shapeShaderIndex, shaderModule->materialDesc,
		dsPrimitiveType_TriangleList, samples);
	if (!shapeShader)
		return NULL;

	dsShader* imageShader = dsShader_createIndex(resourceManager, allocator,
		shaderModule->shaderModule, imageShaderIndex, shaderModule->materialDesc,
		dsPrimitiveType_TriangleList, samples);
	if (!imageShader)
	{
		DS_VERIFY(dsShader_destroy(shapeShader));
		return NULL;
	}

	// TODO: Implement text.
	DS_UNUSED(textShaderIndex);
	DS_UNUSED(textShaderName);
	dsShader* textShader = NULL;
	/*dsPrimitiveType textType = dsPrimitiveType_TriangleList;
	if (dsShaderModule_shaderIndexHasStage(shaderModule->shaderModule,
		shaderModule->textShaderIndex, dsShaderStage_TessellationEvaluation))
	{
		textType = dsPrimitiveType_PatchList;
	}
	dsShader* textShader = dsShader_createIndex(resourceManager, allocator,
		shaderModule->shaderModule, textShaderIndex, shaderModule->materialDesc, textType, samples);
	if (!textShader)
	{
		DS_VERIFY(dsShader_destroy(shapeShader));
		DS_VERIFY(dsShader_destroy(imageShader));
		return NULL;
	}*/

	dsVectorShaders* shaders = DS_ALLOCATE_OBJECT(allocator, dsVectorShaders);
	if (!shaders)
	{
		DS_VERIFY(dsShader_destroy(shapeShader));
		DS_VERIFY(dsShader_destroy(imageShader));
		DS_VERIFY(dsShader_destroy(textShader));
		return NULL;
	}

	shaders->allocator = dsAllocator_keepPointer(allocator);
	shaders->shaderModule = shaderModule;
	shaders->shapeShader = shapeShader;
	shaders->imageShader = imageShader;
	shaders->textShader = textShader;

	return shaders;
}

dsVectorShaders* dsVectorShaders_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsVectorShaderModule* shaderModule, uint32_t samples)
{
	return dsVectorShaders_createImpl(resourceManager, allocator, shaderModule,
		shaderModule->shapeShaderIndex, dsDefaultShapeShaderName, shaderModule->imageShaderIndex,
		dsDefaultImageShaderName, shaderModule->textShaderIndex, dsDefaultTextShaderName, samples);
}

dsVectorShaders* dsVectorShaders_createCustom(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsVectorShaderModule* shaderModule, const char* shapeShaderName,
	const char* imageShaderName, const char* textShaderName, uint32_t samples)
{
	if (!resourceManager || (!allocator && !resourceManager->allocator) || !shaderModule)
	{
		errno = EINVAL;
		return NULL;
	}

	uint32_t shapeShaderIndex = shapeShaderName ? DS_MATERIAL_UNKNOWN :
		shaderModule->shapeShaderIndex;
	uint32_t imageShaderIndex = imageShaderName ? DS_MATERIAL_UNKNOWN :
		shaderModule->imageShaderIndex;
	uint32_t textShaderIndex = textShaderName ? DS_MATERIAL_UNKNOWN : shaderModule->textShaderIndex;
	for (uint32_t i = 0, count = dsShaderModule_shaderCount(shaderModule->shaderModule);
		i < count && (shapeShaderIndex == DS_MATERIAL_UNKNOWN ||
		imageShaderIndex == DS_MATERIAL_UNKNOWN || textShaderIndex == DS_MATERIAL_UNKNOWN); ++i)
	{
		const char* name = dsShaderModule_shaderName(shaderModule->shaderModule, i);
		if (shapeShaderName && strcmp(name, shapeShaderName) == 0)
			shapeShaderIndex = i;
		else if (imageShaderName && strcmp(name, imageShaderName) == 0)
			imageShaderIndex = i;
		else if (textShaderName && strcmp(name, textShaderName) == 0)
			textShaderIndex = i;
	}

	return dsVectorShaders_createImpl(resourceManager, allocator, shaderModule,
		shapeShaderIndex, shapeShaderName, imageShaderIndex, imageShaderName, textShaderIndex,
		textShaderName, samples);
}

bool dsVectorShaders_destroy(dsVectorShaders* shaders)
{
	if (!shaders)
		return true;

	if (!dsShader_destroy(shaders->shapeShader))
		return false;
	DS_VERIFY(dsShader_destroy(shaders->imageShader));
	DS_VERIFY(dsShader_destroy(shaders->textShader));

	if (shaders->allocator)
		DS_VERIFY(dsAllocator_free(shaders->allocator, shaders));
	return true;
}

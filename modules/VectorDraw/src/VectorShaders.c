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
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderModule.h>

dsVectorShaders* dsVectorShaders_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsVectorShaderModule* shaderModule, uint32_t samples)
{
	if (!resourceManager || (!allocator && !resourceManager->allocator) || !shaderModule)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	dsShader* shapeShader = dsShader_createIndex(resourceManager, allocator,
		shaderModule->shaderModule, shaderModule->shapeShaderIndex, shaderModule->materialDesc,
		dsPrimitiveType_TriangleList, samples);
	if (!shapeShader)
		return NULL;

	dsShader* imageShader = dsShader_createIndex(resourceManager, allocator,
		shaderModule->shaderModule, shaderModule->imageShaderIndex, shaderModule->materialDesc,
		dsPrimitiveType_TriangleList, samples);
	if (!imageShader)
	{
		DS_VERIFY(dsShader_destroy(shapeShader));
		return NULL;
	}

	// TODO: Implement text.
	dsShader* textShader = NULL;
	/*dsPrimitiveType textType = dsPrimitiveType_TriangleList;
	if (dsShaderModule_shaderIndexHasStage(shaderModule->shaderModule,
		shaderModule->textShaderIndex, dsShaderStage_TessellationEvaluation))
	{
		textType = dsPrimitiveType_PatchList;
	}
	dsShader* textShader = dsShader_createIndex(resourceManager, allocator,
		shaderModule->shaderModule, shaderModule->imageShaderIndex, shaderModule->materialDesc,
		textType, samples);
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

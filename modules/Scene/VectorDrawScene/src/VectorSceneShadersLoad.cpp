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

#include "VectorSceneShadersLoad.h"

#include "Flatbuffers/VectorSceneShaders_generated.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/VectorDraw/VectorShaderModule.h>
#include <DeepSea/VectorDraw/VectorShaders.h>

static void setString(const char*& string, const flatbuffers::String* fbString)
{
	if (fbString)
		string = fbString->c_str();
}

void* dsVectorSceneShaders_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator*, dsAllocator* resourceAllocator,
	void*, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaVectorDrawScene::VerifyVectorShadersBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_VECTOR_DRAW_SCENE_LOG_TAG, "Invalid vector shaders flatbuffer format.");
		return nullptr;
	}

	dsResourceManager* resourceManager =
		dsSceneLoadContext_getRenderer(loadContext)->resourceManager;
	auto fbVectorShaders = DeepSeaVectorDrawScene::GetVectorShaders(data);

	auto fbExtraElements = fbVectorShaders->extraElements();
	dsMaterialElement* extraElements = nullptr;
	uint32_t extraElementCount = 0;
	if (!fbExtraElements)
	{
		extraElementCount = fbExtraElements->size();
		extraElements = DS_ALLOCATE_STACK_OBJECT_ARRAY(dsMaterialElement, extraElementCount);
		for (uint32_t i = 0; i < extraElementCount; ++i)
		{
			auto fbExtraElement = (*fbExtraElements)[i];
			dsMaterialElement* extraElement = extraElements + i;
			extraElement->name = fbExtraElement->name()->c_str();
			extraElement->type = DeepSeaScene::convert(fbExtraElement->type());
			extraElement->count = fbExtraElement->count();
			extraElement->binding = DeepSeaScene::convert(fbExtraElement->binding());

			auto fbGroupDescName = fbExtraElement->shaderVariableGroupDesc();
			if (fbGroupDescName)
			{
				dsSceneResourceType resourceType;
				dsShaderVariableGroupDesc* groupDesc;
				if (!dsSceneLoadScratchData_findResource(&resourceType,
						reinterpret_cast<void**>(&groupDesc), scratchData,
						fbGroupDescName->c_str()) ||
					resourceType != dsSceneResourceType_ShaderVariableGroup)
				{
					errno = ENOTFOUND;
					DS_LOG_ERROR_F(DS_VECTOR_DRAW_SCENE_LOG_TAG,
						"Couldn't find shader variable group description '%s'",
						fbGroupDescName->c_str());
					return nullptr;
				}

				extraElement->shaderVariableGroupDesc = groupDesc;
			}
			else
				extraElement->shaderVariableGroupDesc = nullptr;
		}
	}

	dsVectorShaderModule* shaderModule;
	if (auto fileRef = fbVectorShaders->shaderModule_as_FileReference())
	{
		shaderModule = dsVectorShaderModule_loadResource(resourceManager, resourceAllocator,
			DeepSeaScene::convert(fileRef->type()), fileRef->path()->c_str(), extraElements,
			extraElementCount);
	}
	else if (auto rawData = fbVectorShaders->shaderModule_as_RawData())
	{
		auto data = rawData->data();
		shaderModule = dsVectorShaderModule_loadData(resourceManager, resourceAllocator,
			data->data(), data->size(), extraElements, extraElementCount);
	}
	else
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_VECTOR_DRAW_SCENE_LOG_TAG, "No data provided for vector shader module");
		return nullptr;
	}

	if (!shaderModule)
		return nullptr;

	const char* shaderNames[dsVectorShaderType_Count];
	memset(shaderNames, 0, sizeof(shaderNames));
	setString(shaderNames[dsVectorShaderType_FillColor], fbVectorShaders->fillColor());
	setString(shaderNames[dsVectorShaderType_FillLinearGradient],
		fbVectorShaders->fillLinearGradient());
	setString(shaderNames[dsVectorShaderType_FillRadialGradient],
		fbVectorShaders->fillRadialGradient());
	setString(shaderNames[dsVectorShaderType_Line], fbVectorShaders->line());
	setString(shaderNames[dsVectorShaderType_Image], fbVectorShaders->image());
	setString(shaderNames[dsVectorShaderType_TextColor], fbVectorShaders->textColor());
	setString(shaderNames[dsVectorShaderType_TextColorOutline],
		fbVectorShaders->textColorOutline());
	setString(shaderNames[dsVectorShaderType_TextGradient], fbVectorShaders->textGradient());
	setString(shaderNames[dsVectorShaderType_TextGradientOutline],
		fbVectorShaders->textGradientOutline());

	dsVectorShaders* shaders = dsVectorShaders_createCustom(resourceManager, resourceAllocator,
		shaderModule, shaderNames);
	if (!shaders)
	{
		DS_VERIFY(dsVectorShaderModule_destroy(shaderModule));
		return nullptr;
	}

	return shaders;
}

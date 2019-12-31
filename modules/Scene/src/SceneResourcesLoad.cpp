/*
 * Copyright 2019 Aaron Barany
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

#include <DeepSea/Scene/SceneResources.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include "Flatbuffers/BufferMaterialData_generated.h"
#include "Flatbuffers/NamedMaterialData_generated.h"
#include "Flatbuffers/SceneResources_generated.h"
#include "Flatbuffers/TextureBufferMaterialData_generated.h"

#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/MaterialType.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/TextureData.h>
#include <DeepSea/Render/Resources/VertexFormat.h>

#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/Types.h>

#define PRINT_FLATBUFFER_ERROR(message, name) \
	do \
	{ \
		if (name) \
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, message " for '%s'.", name); \
		else \
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, message); \
	} while (false)

#define PRINT_FLATBUFFER_RESOURCE_ERROR(message, resourceName, fileName) \
	do \
	{ \
		if (name) \
		{ \
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, message " for scene resources '%s': %s.", \
				resourceName, fileName,  dsErrorString(errno)); \
		} \
		else \
		{ \
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, message " for scene resources: %s.", resourceName, \
				dsErrorString(errno)); \
		} \
	} while (false)

#define PRINT_FLATBUFFER_RESOURCE_NOT_FOUND(resourceType, resourceName, fileName) \
	do \
	{ \
		if (name) \
		{ \
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, \
				"Couln't find %s '%s' for scene resources '%s'.", resourceType, resourceName, \
				fileName); \
		} \
		else \
		{ \
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, \
				"Couln't find %s '%s' for scene resources.", resourceType, resourceName); \
		} \
	} while (false)

#define PRINT_FLATBUFFER_MATERIAL_ERROR(message, elementName, fileName) \
	do \
	{ \
		if (name) \
		{ \
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, message " for scene resources '%s'.", \
				elementName, fileName); \
		} \
		else \
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, message " for scene resources.", elementName); \
	} while (false)

template <typename T>
using FlatbufferVector = flatbuffers::Vector<flatbuffers::Offset<T>>;

static bool loadBuffers(dsSceneResources* resources, dsResourceManager* resourceManager,
	dsAllocator* allocator, const FlatbufferVector<DeepSeaScene::Buffer>* buffers,
	const char* name)
{
	if (!buffers)
		return true;

	for (auto fbBuffer : *buffers)
	{
		if (!fbBuffer)
			continue;

		const char* bufferName = fbBuffer->name()->c_str();
		uint32_t bufferSize = fbBuffer->size();
		auto bufferData = fbBuffer->data();
		if (bufferData && bufferData->size() != bufferSize)
		{
			errno = EFORMAT;
			PRINT_FLATBUFFER_RESOURCE_ERROR(
				"Mismatch between size and data size for buffer '%s'", bufferName, name);
			return false;
		}

		dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, allocator,
			(dsGfxBufferUsage)fbBuffer->usage(), (dsGfxMemory)fbBuffer->memoryHints(),
			bufferData ? bufferData->data() : nullptr, bufferSize);
		if (!buffer)
		{
			PRINT_FLATBUFFER_RESOURCE_ERROR(
				"Couldn't create buffer '%s'", bufferName, name);
			return false;
		}

		if (!dsSceneResources_addResource(
				resources, bufferName, dsSceneResourceType_Buffer, buffer, true))
		{
			DS_VERIFY(dsGfxBuffer_destroy(buffer));
			return false;
		}
	}

	return true;
}

static bool loadTextures(dsSceneResources* resources, dsResourceManager* resourceManager,
	dsAllocator* allocator, dsAllocator* resourceAllocator,
	const FlatbufferVector<DeepSeaScene::Texture>* textures, const char* name)
{
	if (!textures)
		return true;

	for (auto fbTexture : *textures)
	{
		if (!fbTexture)
			continue;

		const char* textureName = fbTexture->name()->c_str();
		auto texturePath = fbTexture->path();
		auto fbTextureInfo = fbTexture->textureInfo();
		if ((!texturePath && !fbTextureInfo) || (texturePath && fbTextureInfo))
		{
			errno = EFORMAT;
			PRINT_FLATBUFFER_RESOURCE_ERROR(
				"Either texture path or texture info must be provided for texture '%s'",
				textureName, name);
			return false;
		}

		auto usage = static_cast<dsTextureUsage>(fbTexture->usage());
		auto memoryHints = static_cast<dsGfxMemory>(fbTexture->memoryHints());
		dsTexture* texture;
		if (texturePath)
		{
			texture = dsTextureData_loadResourceToTexture(resourceManager, resourceAllocator,
				allocator, DeepSeaScene::convert(texturePath->type()),
				texturePath->path()->c_str(), nullptr, usage, memoryHints);
		}
		else
		{
			dsTextureInfo textureInfo =
			{
				DeepSeaScene::convert(fbTextureInfo->format(), fbTextureInfo->decoration()),
				DeepSeaScene::convert(fbTextureInfo->dimension()),
				fbTextureInfo->width(),
				fbTextureInfo->height(),
				fbTextureInfo->depth(),
				fbTextureInfo->mipLevels(),
				1
			};
			texture = dsTexture_create(resourceManager, resourceAllocator, usage, memoryHints,
				&textureInfo, nullptr, 0);
		}

		if (!texture)
		{
			PRINT_FLATBUFFER_RESOURCE_ERROR(
				"Couldn't create texture '%s'", textureName, name);
			return false;
		}

		if (!dsSceneResources_addResource(
				resources, textureName, dsSceneResourceType_Texture, texture, true))
		{
			DS_VERIFY(dsTexture_destroy(texture));
			return false;
		}
	}

	return true;
}

static bool loadShaderVariableGroupDescs(dsSceneResources* resources,
	dsResourceManager* resourceManager, dsAllocator* allocator, dsSceneLoadScratchData* scratchData,
	const FlatbufferVector<DeepSeaScene::ShaderVariableGroupDesc>* groupDescs, const char* name)
{
	if (!groupDescs)
		return true;

	uint32_t maxElements = 0;
	for (auto fbGroupDesc : *groupDescs)
	{
		if (!fbGroupDesc)
			continue;

		const auto fbElements = fbGroupDesc->elements();
		if (!fbElements)
			continue;

		maxElements = std::max(fbElements->size(), maxElements);
	}
	if (maxElements == 0)
		return true;

	dsAllocator* scratchAllocator = dsSceneLoadScratchData_getAllocator(scratchData);
	auto elements =
		DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, dsShaderVariableElement, maxElements);
	if (!elements)
		return false;

	bool success = true;
	for (auto fbGroupDesc : *groupDescs)
	{
		if (!fbGroupDesc)
			continue;

		const auto fbElements = fbGroupDesc->elements();
		if (!fbElements)
			continue;

		const char* groupDescName = fbGroupDesc->name()->c_str();
		dsShaderVariableElement* curElement = elements;
		for (auto fbElement : *fbElements)
		{
			curElement->name = fbElement->name()->c_str();
			curElement->type = DeepSeaScene::convert(fbElement->type());
			curElement->count = fbElement->count();
			++curElement;
		}

		dsShaderVariableGroupDesc* groupDesc = dsShaderVariableGroupDesc_create(resourceManager,
			allocator, elements, fbElements->size());

		if (!groupDesc)
		{
			PRINT_FLATBUFFER_RESOURCE_ERROR(
				"Couldn't create shader variable group description '%s'", groupDescName, name);
			success = false;
			break;
		}

		if (!dsSceneResources_addResource(resources, groupDescName,
				dsSceneResourceType_ShaderVariableGroupDesc, groupDesc, true))
		{
			DS_VERIFY(dsShaderVariableGroupDesc_destroy(groupDesc));
			success = false;
			break;
		}
	}

	DS_VERIFY(dsAllocator_free(scratchAllocator, elements));
	return success;
}

static bool loadShaderVariableGroups(dsSceneResources* resources,
	dsResourceManager* resourceManager, dsAllocator* allocator, dsSceneLoadScratchData* scratchData,
	const FlatbufferVector<DeepSeaScene::ShaderData>* groups, const char* name)
{
	if (!groups)
		return false;

	dsCommandBuffer* commandBuffer = dsSceneLoadScratchData_getCommandBuffer(scratchData);
	for (auto fbGroup : *groups)
	{
		if (!fbGroup)
			return false;

		const char* groupDescName = fbGroup->description()->c_str();
		dsShaderVariableGroupDesc* groupDesc;
		dsSceneResourceType resourceType;
		if (!dsSceneLoadScratchData_findResource(&resourceType,
				reinterpret_cast<void**>(&groupDesc), scratchData, groupDescName) ||
			resourceType != dsSceneResourceType_ShaderVariableGroupDesc)
		{
			// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
			errno = ENOTFOUND;
			PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("shader variable group", groupDescName, name);
			return false;
		}

		const char* groupName = fbGroup->name()->c_str();
		dsShaderVariableGroup* group = dsShaderVariableGroup_create(resourceManager, allocator,
			nullptr, groupDesc);
		if (!group)
		{
			PRINT_FLATBUFFER_RESOURCE_ERROR(
				"Couldn't create shader variable group '%s'", groupName, name);
			return false;
		}

		// NOTE: This takes ownership on success, so errors after this point won't destroy the
		// variable group.
		if (!dsSceneResources_addResource(
				resources, groupName, dsSceneResourceType_ShaderVariableGroup, groupDesc, true))
		{
			DS_VERIFY(dsShaderVariableGroup_destroy(group));
			return false;
		}

		auto* variableData = fbGroup->data();
		if (!variableData)
			continue;

		if (!commandBuffer)
		{
			PRINT_FLATBUFFER_RESOURCE_ERROR(
				"Command buffer not available to set data on variable group '%s'", groupName, name);
			return false;
		}

		for (auto fbData : *variableData)
		{
			if (!fbData)
				continue;

			const char* dataName = fbData->name()->c_str();
			uint32_t element = dsShaderVariableGroupDesc_findElement(groupDesc, dataName);
			if (element == DS_MATERIAL_UNKNOWN)
			{
				PRINT_FLATBUFFER_MATERIAL_ERROR(
					"Couldn't find shader variable group element '%s'", dataName, name);
				return false;
			}

			auto data = fbData->data();
			dsMaterialType type = DeepSeaScene::convert(fbData->type());
			uint32_t count = fbData->count();
			uint32_t expectedSize = dsMaterialType_cpuSize(type)*count;
			if (data->size() != expectedSize)
			{
				PRINT_FLATBUFFER_MATERIAL_ERROR(
					"Incorrect data size for shader variable group element '%s'", dataName,
					name);
				return false;
			}

			if (!dsShaderVariableGroup_setElementData(group, element, data->data(),
					type, fbData->first(), count))
			{
				PRINT_FLATBUFFER_MATERIAL_ERROR(
					"Couldn't set shader variable group element '%s'", dataName, name);
				return false;
			}
		}

		dsShaderVariableGroup_commit(group, commandBuffer);
	}

	return true;
}

static bool loadMaterialDescs(dsSceneResources* resources, dsResourceManager* resourceManager,
	dsAllocator* allocator, dsSceneLoadScratchData* scratchData,
	const FlatbufferVector<DeepSeaScene::MaterialDesc>* materialDescs, const char* name)
{
	if (!materialDescs)
		return true;

	uint32_t maxElements = 0;
	for (auto fbGroupDesc : *materialDescs)
	{
		if (!fbGroupDesc)
			continue;

		const auto fbElements = fbGroupDesc->elements();
		if (!fbElements)
			continue;

		maxElements = std::max(fbElements->size(), maxElements);
	}
	if (maxElements == 0)
		return true;

	dsAllocator* scratchAllocator = dsSceneLoadScratchData_getAllocator(scratchData);
	auto elements =
		DS_ALLOCATE_OBJECT_ARRAY(scratchAllocator, dsMaterialElement, maxElements);
	if (!elements)
		return false;

	bool success = true;
	for (auto fbMaterialDesc : *materialDescs)
	{
		if (!fbMaterialDesc)
			continue;

		const auto fbElements = fbMaterialDesc->elements();
		if (!fbElements)
			continue;

		const char* materialDescName = fbMaterialDesc->name()->c_str();
		dsMaterialElement* curElement = elements;
		for (auto fbElement : *fbElements)
		{
			curElement->name = fbElement->name()->c_str();
			curElement->type = DeepSeaScene::convert(fbElement->type());
			curElement->count = fbElement->count();
			auto groupDescName = fbElement->shaderVariableGroupDesc();
			if (groupDescName)
			{
				dsShaderVariableGroupDesc* groupDesc;
				dsSceneResourceType resourceType;
				if (!dsSceneLoadScratchData_findResource(&resourceType,
						reinterpret_cast<void**>(&groupDesc), scratchData,
						groupDescName->c_str()) ||
					resourceType != dsSceneResourceType_ShaderVariableGroupDesc)
				{
					errno = ENOTFOUND;
					PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("shader variable group",
						groupDescName->c_str(), name);
					success = false;
					break;
				}
				curElement->shaderVariableGroupDesc = groupDesc;
			}
			else
				curElement->shaderVariableGroupDesc = nullptr;
			curElement->binding = static_cast<dsMaterialBinding>(fbElement->binding());
			++curElement;
		}

		if (!success)
			break;

		dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, allocator, elements,
			fbElements->size());

		if (!materialDesc)
		{
			PRINT_FLATBUFFER_RESOURCE_ERROR(
				"Couldn't create material description '%s'", materialDescName, name);
			success = false;
			break;
		}

		if (!dsSceneResources_addResource(resources, materialDescName,
				dsSceneResourceType_MaterialDesc, materialDesc, true))
		{
			DS_VERIFY(dsMaterialDesc_destroy(materialDesc));
			success = false;
			break;
		}
	}

	DS_VERIFY(dsAllocator_free(scratchAllocator, elements));
	return success;
}

static bool loadMaterialTexture(dsSceneLoadScratchData* scratchData, dsMaterial* material,
	uint32_t element, const uint8_t* data, uint32_t dataSize, const char* dataName,
	const char* name)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScene::VerifyNamedMaterialDataBuffer(verifier))
	{
		errno = EFORMAT;
		PRINT_FLATBUFFER_MATERIAL_ERROR(
			"Invalid NamedMaterialData flatbuffer data for element '%s'", dataName, name);
		return false;
	}

	auto materialData = DeepSeaScene::GetNamedMaterialData(data);
	const char* textureName = materialData->name()->c_str();

	dsTexture* texture;
	dsSceneResourceType resourceType;
	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&texture), scratchData, textureName) ||
		resourceType != dsSceneResourceType_Texture)
	{
		// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
		errno = ENOTFOUND;
		PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("texture", textureName, name);
		return false;
	}

	if (!dsMaterial_setTexture(material, element, texture))
	{
		PRINT_FLATBUFFER_MATERIAL_ERROR("Couldn't set texture '%s'", dataName, name);
		return false;
	}

	return true;
}

static bool loadMaterialTextureBuffer(dsSceneLoadScratchData* scratchData, dsMaterial* material,
	uint32_t element, const uint8_t* data, uint32_t dataSize, const char* dataName,
	const char* name)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScene::VerifyTextureBufferMaterialDataBuffer(verifier))
	{
		errno = EFORMAT;
		PRINT_FLATBUFFER_MATERIAL_ERROR(
			"Invalid TextureBufferMaterialData flatbuffer data for element '%s'", dataName, name);
		return false;
	}

	auto materialData = DeepSeaScene::GetTextureBufferMaterialData(data);
	const char* bufferName = materialData->name()->c_str();

	dsGfxBuffer* buffer;
	dsSceneResourceType resourceType;
	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&buffer), scratchData, bufferName) ||
		resourceType != dsSceneResourceType_Buffer)
	{
		// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
		errno = ENOTFOUND;
		PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("buffer", bufferName, name);
		return false;
	}

	if (!dsMaterial_setTextureBuffer(material, element, buffer,
			DeepSeaScene::convert(materialData->format(), materialData->decoration()),
			materialData->offset(), materialData->count()))
	{
		PRINT_FLATBUFFER_MATERIAL_ERROR("Couldn't set texture buffer '%s'", dataName, name);
		return false;
	}

	return true;
}

static bool loadMaterialVariableGroup(dsSceneLoadScratchData* scratchData, dsMaterial* material,
	uint32_t element, const uint8_t* data, uint32_t dataSize, const char* dataName,
	const char* name)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScene::VerifyNamedMaterialDataBuffer(verifier))
	{
		errno = EFORMAT;
		PRINT_FLATBUFFER_MATERIAL_ERROR(
			"Invalid NamedMaterialData flatbuffer data for element '%s'", dataName, name);
		return false;
	}

	auto materialData = DeepSeaScene::GetNamedMaterialData(data);
	const char* textureName = materialData->name()->c_str();

	dsShaderVariableGroup* variableGroup;
	dsSceneResourceType resourceType;
	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&variableGroup), scratchData, textureName) ||
		resourceType != dsSceneResourceType_ShaderVariableGroup)
	{
		// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
		errno = ENOTFOUND;
		PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("shader variable group", textureName, name);
		return false;
	}

	if (!dsMaterial_setVariableGroup(material, element, variableGroup))
	{
		PRINT_FLATBUFFER_MATERIAL_ERROR("Couldn't set shader variable group '%s'", dataName, name);
		return false;
	}

	return true;
}

static bool loadMaterialBuffer(dsSceneLoadScratchData* scratchData, dsMaterial* material,
	uint32_t element, const uint8_t* data, uint32_t dataSize, const char* dataName,
	const char* name)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScene::VerifyBufferMaterialDataBuffer(verifier))
	{
		errno = EFORMAT;
		PRINT_FLATBUFFER_MATERIAL_ERROR(
			"Invalid BufferMaterialData flatbuffer data for element '%s'", dataName, name);
		return false;
	}

	auto materialData = DeepSeaScene::GetBufferMaterialData(data);
	const char* bufferName = materialData->name()->c_str();

	dsGfxBuffer* buffer;
	dsSceneResourceType resourceType;
	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&buffer), scratchData, bufferName) ||
		resourceType != dsSceneResourceType_Buffer)
	{
		// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
		errno = ENOTFOUND;
		PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("buffer", bufferName, name);
		return false;
	}

	if (!dsMaterial_setBuffer(material, element, buffer, materialData->offset(),
			materialData->size()))
	{
		PRINT_FLATBUFFER_MATERIAL_ERROR("Couldn't set buffer '%s'", dataName, name);
		return false;
	}

	return true;
}

static bool loadMaterialData(dsMaterial* material, uint32_t element, dsMaterialType type,
	uint32_t first, uint32_t count, const uint8_t* data, uint32_t dataSize, const char* dataName,
	const char* name)
{
	uint32_t expectedSize = dsMaterialType_cpuSize(type)*count;
	if (dataSize != expectedSize)
	{
		PRINT_FLATBUFFER_MATERIAL_ERROR(
			"Incorrect data size for material element '%s'", dataName,
			name);
		return false;
	}

	if (!dsMaterial_setElementData(material, element, data, type, first, count))
	{
		PRINT_FLATBUFFER_MATERIAL_ERROR(
			"Couldn't set material element '%s'", dataName, name);
		return false;
	}

	return true;
}

static bool loadMaterials(dsSceneResources* resources, dsResourceManager* resourceManager,
	dsAllocator* allocator, dsSceneLoadScratchData* scratchData,
	const FlatbufferVector<DeepSeaScene::ShaderData>* materials, const char* name)
{
	if (!materials)
		return false;

	for (auto fbMaterial : *materials)
	{
		if (!fbMaterial)
			return false;

		const char* materialDescName = fbMaterial->description()->c_str();
		dsMaterialDesc* materialDesc;
		dsSceneResourceType resourceType;
		if (!dsSceneLoadScratchData_findResource(&resourceType,
				reinterpret_cast<void**>(&materialDesc), scratchData, materialDescName) ||
			resourceType != dsSceneResourceType_MaterialDesc)
		{
			// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
			errno = ENOTFOUND;
			PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("material", materialDescName, name);
			return false;
		}

		const char* materialName = fbMaterial->name()->c_str();
		dsMaterial* material = dsMaterial_create(resourceManager, allocator, materialDesc);
		if (!material)
		{
			PRINT_FLATBUFFER_RESOURCE_ERROR(
				"Couldn't create material '%s'", materialName, name);
			return false;
		}

		// NOTE: This takes ownership on success, so errors after this point won't destroy the
		// material.
		if (!dsSceneResources_addResource(
				resources, materialName, dsSceneResourceType_Material, materialDesc, true))
		{
			dsMaterial_destroy(material);
			return false;
		}

		auto* variableData = fbMaterial->data();
		if (!variableData)
			continue;

		for (auto fbData : *variableData)
		{
			if (!fbData)
				continue;

			const char* dataName = fbData->name()->c_str();
			uint32_t element = dsMaterialDesc_findElement(materialDesc, dataName);
			if (element == DS_MATERIAL_UNKNOWN)
			{
				PRINT_FLATBUFFER_MATERIAL_ERROR(
					"Couldn't find material element '%s'", dataName, name);
				return false;
			}

			auto data = fbData->data();
			dsMaterialType type = DeepSeaScene::convert(fbData->type());
			bool success;
			switch (type)
			{
				case dsMaterialType_Texture:
				case dsMaterialType_Image:
				case dsMaterialType_SubpassInput:
					success = loadMaterialTexture(scratchData, material, element, data->data(),
						data->size(), dataName, name);
					break;
				case dsMaterialType_TextureBuffer:
				case dsMaterialType_ImageBuffer:
					success = loadMaterialTextureBuffer(scratchData, material, element,
						data->data(), data->size(), dataName, name);
					break;
				case dsMaterialType_VariableGroup:
					success = loadMaterialVariableGroup(scratchData, material, element,
						data->data(), data->size(), dataName, name);
					break;
				case dsMaterialType_UniformBlock:
				case dsMaterialType_UniformBuffer:
					success = loadMaterialBuffer(scratchData, material, element, data->data(),
						data->size(), dataName, name);
					break;
				default:
				{
					success = loadMaterialData(material, element, type, fbData->first(),
						fbData->count(), data->data(), data->size(), dataName, name);
					break;
				}
			}

			if (!success)
				return false;
		}
	}

	return true;
}

static bool loadShaderModules(dsSceneResources* resources, dsResourceManager* resourceManager,
	dsAllocator* allocator, const FlatbufferVector<DeepSeaScene::ShaderModule>* shaderModules,
	const char* name)
{
	if (!shaderModules)
		return true;

	for (auto fbShaderModule : *shaderModules)
	{
		if (!fbShaderModule)
			continue;

		const char* shaderModuleName = fbShaderModule->name()->c_str();
		auto file = fbShaderModule->file();
		dsShaderModule* shaderModule = dsShaderModule_loadResource(resourceManager, allocator,
			DeepSeaScene::convert(file->type()), file->path()->c_str(), shaderModuleName);
		if (!shaderModule)
		{
			PRINT_FLATBUFFER_RESOURCE_ERROR(
				"Couldn't load shader module '%s'", shaderModuleName, name);
			return false;
		}

		if (!dsSceneResources_addResource(resources, shaderModuleName,
				dsSceneResourceType_ShaderModule, shaderModule, true))
		{
			DS_VERIFY(dsShaderModule_destroy(shaderModule));
			return false;
		}
	}

	return true;
}

static bool loadShaders(dsSceneResources* resources,
	dsResourceManager* resourceManager, dsAllocator* allocator, dsSceneLoadScratchData* scratchData,
	const FlatbufferVector<DeepSeaScene::Shader>* shaders, const char* name)
{
	if (!shaders)
		return true;

	for (auto fbShader : *shaders)
	{
		if (!fbShader)
			continue;

		// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
		const char* shaderModuleName = fbShader->shaderModule()->c_str();
		dsShaderModule* shaderModule;
		dsSceneResourceType resourceType;
		if (!dsSceneLoadScratchData_findResource(&resourceType,
				reinterpret_cast<void**>(&shaderModule), scratchData, shaderModuleName) ||
			resourceType != dsSceneResourceType_ShaderModule)
		{
			errno = ENOTFOUND;
			PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("shader module", shaderModuleName, name);
			return false;
		}

		const char* materialDescName = fbShader->materialDesc()->c_str();
		dsMaterialDesc* materialDesc;
		if (!dsSceneLoadScratchData_findResource(&resourceType,
				reinterpret_cast<void**>(&materialDesc), scratchData, materialDescName) ||
			resourceType != dsSceneResourceType_MaterialDesc)
		{
			errno = ENOTFOUND;
			PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("material description", materialDescName, name);
			return false;
		}

		const char* shaderName = fbShader->name()->c_str();
		auto fbPipelineName = fbShader->pipelineName();
		const char* pipelineName = fbPipelineName ? fbPipelineName->c_str() : shaderName;
		dsShader* shader = dsShader_createName(resourceManager, allocator, shaderModule,
			pipelineName, materialDesc);
		if (!shader)
		{
			PRINT_FLATBUFFER_RESOURCE_ERROR(
				"Couldn't create shader '%s'", shaderName, name);
			return false;
		}

		if (!dsSceneResources_addResource(
				resources, shaderName, dsSceneResourceType_Shader, shader, true))
		{
			DS_VERIFY(dsShader_destroy(shader));
			return false;
		}
	}

	return true;
}

static bool loadDrawGeometries(dsSceneResources* resources, dsResourceManager* resourceManager,
	dsAllocator* allocator, dsSceneLoadScratchData* scratchData,
	const FlatbufferVector<DeepSeaScene::DrawGeometry>* geometries, const char* name)
{
	if (!geometries)
		return true;

	for (auto fbGeometry : *geometries)
	{
		if (!fbGeometry)
			continue;

		const char* geometryName = fbGeometry->name()->c_str();
		uint32_t vertexBufferIndex = 0;
		dsVertexBuffer vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS];
		dsVertexBuffer* vertexBufferPtrs[DS_MAX_GEOMETRY_VERTEX_BUFFERS];
		for (auto fbVertexBuffer : *fbGeometry->vertexBuffers())
		{
			if (vertexBufferIndex > DS_MAX_GEOMETRY_VERTEX_BUFFERS)
			{
				errno = ESIZE;
				PRINT_FLATBUFFER_RESOURCE_ERROR(
					"Too many vertex buffers for geometry '%s'", geometryName, name);
				return false;
			}

			dsVertexBuffer* vertexBuffer = vertexBuffers + vertexBufferIndex;
			if (!fbVertexBuffer)
			{
				vertexBufferPtrs[vertexBufferIndex] = nullptr;
				++vertexBufferIndex;
				continue;
			}

			const char* bufferName = fbVertexBuffer->name()->c_str();
			dsSceneResourceType resourceType;
			if (!dsSceneLoadScratchData_findResource(&resourceType,
					reinterpret_cast<void**>(&vertexBuffer->buffer), scratchData, bufferName) ||
				resourceType != dsSceneResourceType_Buffer)
			{
				// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
				errno = ENOTFOUND;
				PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("buffer", bufferName, name);
				return false;
			}

			vertexBuffer->offset = fbVertexBuffer->offset();
			vertexBuffer->count = fbVertexBuffer->count();

			auto fbVertexFormat = fbVertexBuffer->format();
			DS_VERIFY(dsVertexFormat_initialize(&vertexBuffer->format));
			vertexBuffer->format.instanced = fbVertexFormat->instanced();
			uint32_t attribIndex = 0;
			for (auto fbAttribute : *fbVertexFormat->attributes())
			{
				if (!fbAttribute)
				{
					++attribIndex;
					continue;
				}

				if (attribIndex > resourceManager->maxVertexAttribs)
				{
					errno = ESIZE;
					PRINT_FLATBUFFER_RESOURCE_ERROR(
						"Too many vertex attributes for vertex buffer '%s'", bufferName, name);
					return false;
				}

				dsVertexFormat_setAttribEnabled(&vertexBuffer->format, attribIndex, true);
				vertexBuffer->format.elements[attribIndex].format = DeepSeaScene::convert(
					fbAttribute->format(), fbAttribute->decoration());
			}
			DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(&vertexBuffer->format));

			++vertexBufferIndex;
		}

		auto fbIndexBuffer = fbGeometry->indexBuffer();
		dsIndexBuffer indexBuffer;
		if (fbIndexBuffer)
		{
			const char* bufferName = fbIndexBuffer->name()->c_str();
			dsSceneResourceType resourceType;
			if (!dsSceneLoadScratchData_findResource(&resourceType,
					reinterpret_cast<void**>(&indexBuffer.buffer), scratchData, bufferName) ||
				resourceType != dsSceneResourceType_Buffer)
			{
				// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
				errno = ENOTFOUND;
				PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("buffer", bufferName, name);
				return false;
			}

			indexBuffer.offset = fbIndexBuffer->offset();
			indexBuffer.count = fbIndexBuffer->count();
			indexBuffer.indexSize = fbIndexBuffer->indexSize();
		}

		dsDrawGeometry* geometry = dsDrawGeometry_create(resourceManager, allocator,
			vertexBufferPtrs, fbIndexBuffer ? &indexBuffer : nullptr);
		if (!geometry)
		{
			PRINT_FLATBUFFER_RESOURCE_ERROR(
				"Couldn't create geometry '%s'", geometryName, name);
			return false;
		}

		if (!dsSceneResources_addResource(
				resources, geometryName, dsSceneResourceType_DrawGeometry, geometry, true))
		{
			DS_VERIFY(dsDrawGeometry_destroy(geometry));
			return false;
		}
	}

	return true;
}

static bool loadNodes(dsSceneResources* resources, dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, const FlatbufferVector<DeepSeaScene::SceneNode>* nodes,
	const char* name)
{
	if (!nodes)
		return true;

	for (auto fbNamedNode : *nodes)
	{
		if (!fbNamedNode)
			continue;

		const char* nodeName = fbNamedNode->name()->c_str();
		auto fbNode = fbNamedNode->node();
		auto data = fbNode->data();
		dsSceneNode* node = dsSceneNode_load(allocator, resourceAllocator, loadContext, scratchData,
			fbNode->type()->c_str(), data->data(), data->size());
		if (!node)
		{
			PRINT_FLATBUFFER_RESOURCE_ERROR("Couldn't load node '%s'", nodeName, name);
			return false;
		}

		bool success = dsSceneResources_addResource(
			resources, nodeName, dsSceneResourceType_SceneNode, node, true);
		dsSceneNode_freeRef(node);
		if (!success)
			return false;
	}

	return true;
}

extern "C"
dsSceneResources* dsSceneResources_loadImpl(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData,
	const void* data, size_t dataSize, const char* name)
{
	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), dataSize);
	if (!DeepSeaScene::VerifySceneResourcesBuffer(verifier))
	{
		errno = EFORMAT;
		PRINT_FLATBUFFER_ERROR("Invalid scene resources flatbuffer format", name);
		return nullptr;
	}

	dsRenderer* renderer = dsSceneLoadContext_getRenderer(loadContext);
	dsResourceManager* resourceManager = renderer->resourceManager;
	if (!resourceAllocator)
		resourceAllocator = allocator;

	auto fbSceneResources = DeepSeaScene::GetSceneResources(data);

	uint32_t totalCount = 0;
	auto buffers = fbSceneResources->buffers();
	if (buffers)
		totalCount += buffers->size();
	auto textures = fbSceneResources->textures();
	if (textures)
		totalCount += textures->size();
	auto groupDescs = fbSceneResources->shaderVariableGroupDescs();
	if (groupDescs)
		totalCount += groupDescs->size();
	auto groups = fbSceneResources->shaderVariableGroups();
	if (groups)
		totalCount += groups->size();
	auto materialDescs = fbSceneResources->materialDescs();
	if (materialDescs)
		totalCount += materialDescs->size();
	auto materials = fbSceneResources->materials();
	if (materials)
		totalCount += materials->size();
	auto shaderModules = fbSceneResources->shaderModules();
	if (shaderModules)
		totalCount += shaderModules->size();
	auto shaders = fbSceneResources->shaders();
	if (shaders)
		totalCount += shaders->size();
	auto geometries = fbSceneResources->drawGeometries();
	if (geometries)
		totalCount += geometries->size();
	auto nodes = fbSceneResources->sceneNodes();
	if (nodes)
		totalCount += nodes->size();

	dsSceneResources* resources = dsSceneResources_create(allocator, totalCount);
	if (!resources)
		return nullptr;

	if (!dsSceneLoadScratchData_pushSceneResources(scratchData, &resources, 1) ||
		!loadBuffers(resources, resourceManager, resourceAllocator, buffers, name) ||
		!loadTextures(resources, resourceManager, allocator, resourceAllocator, textures, name) ||
		!loadShaderVariableGroupDescs(resources, resourceManager, resourceAllocator, scratchData,
			groupDescs, name) ||
		!loadShaderVariableGroups(resources, resourceManager, resourceAllocator, scratchData,
			groups, name) ||
		!loadMaterialDescs(resources, resourceManager, resourceAllocator, scratchData,
			materialDescs, name) ||
		!loadMaterials(
			resources, resourceManager, resourceAllocator, scratchData, materials, name) ||
		!loadShaderModules(resources, resourceManager, resourceAllocator, shaderModules, name) ||
		!loadShaders(resources, resourceManager, resourceAllocator, scratchData, shaders, name) ||
		!loadDrawGeometries(
			resources, resourceManager, resourceAllocator, scratchData, geometries, name) ||
		!loadNodes(resources, allocator, resourceAllocator, loadContext, scratchData, nodes, name))
	{
		dsSceneResources_freeRef(resources);
		DS_VERIFY(dsSceneLoadScratchData_popSceneResources(scratchData, 1));
		return nullptr;
	}

	DS_VERIFY(dsSceneLoadScratchData_popSceneResources(scratchData, 1));
	return resources;
}

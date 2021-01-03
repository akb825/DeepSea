/*
 * Copyright 2019-2020 Aaron Barany
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

#include "Flatbuffers/BufferMaterialData_generated.h"
#include "Flatbuffers/NamedMaterialData_generated.h"
#include "Flatbuffers/SceneResources_generated.h"
#include "Flatbuffers/TextureBufferMaterialData_generated.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Streams/MemoryStream.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

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
#include <DeepSea/Render/Renderer.h>

#include <DeepSea/Scene/Flatbuffers/SceneFlatbufferHelpers.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/CustomSceneResource.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/Types.h>

#define PRINT_FLATBUFFER_ERROR(message, name) \
	do \
	{ \
		if (name) \
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, message " for '%s'.", name); \
		else \
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, message "."); \
	} while (false)

#define PRINT_FLATBUFFER_RESOURCE_ERROR(message, resourceName, fileName) \
	do \
	{ \
		if (fileName) \
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
		if (fileName) \
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
		if (fileName) \
		{ \
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, message " for scene resources '%s'.", \
				elementName, fileName); \
		} \
		else \
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, message " for scene resources.", elementName); \
	} while (false)

template <typename T>
using FlatbufferVector = flatbuffers::Vector<flatbuffers::Offset<T>>;

static bool loadBuffer(dsSceneResources* resources, dsResourceManager* resourceManager,
	dsAllocator* allocator, const DeepSeaScene::Buffer* fbBuffer, const char* fileName,
	dsAllocator* scratchAllocator, void*& tempData, size_t& tempDataSize)
{
	const char* bufferName = fbBuffer->name()->c_str();
	uint32_t bufferSize = fbBuffer->size();
	const void* bufferData = nullptr;
	size_t dataSize = bufferSize;
	if (auto fbFileRef = fbBuffer->data_as_FileReference())
	{
		dsResourceStream stream;
		if (!dsResourceStream_open(&stream, DeepSeaScene::convert(fbFileRef->type()),
				fbFileRef->path()->c_str(), "rb"))
		{
			PRINT_FLATBUFFER_RESOURCE_ERROR("Couldn't open file for buffer '%s'", bufferName,
				fileName);
			return false;
		}

		bool readData = dsStream_readUntilEndReuse(&tempData, &dataSize, &tempDataSize,
			reinterpret_cast<dsStream*>(&stream), scratchAllocator);
		if (!readData)
		{
			PRINT_FLATBUFFER_RESOURCE_ERROR("Couldn't read data for buffer '%s'", bufferName,
				fileName);
			return false;
		}

		bufferData = &tempData;
	}
	else if (auto fbRawData = fbBuffer->data_as_RawData())
	{
		auto fbData = fbRawData->data();
		bufferData = fbData->data();
		dataSize = fbData->size();
	}

	if (dataSize != bufferSize)
	{
		errno = EFORMAT;
		PRINT_FLATBUFFER_RESOURCE_ERROR(
			"Mismatch between size and data size for buffer '%s'", bufferName, fileName);
		return false;
	}

	dsGfxBuffer* buffer = dsGfxBuffer_create(resourceManager, allocator,
		(dsGfxBufferUsage)fbBuffer->usage(), (dsGfxMemory)fbBuffer->memoryHints(), bufferData,
		bufferSize);

	if (!buffer)
	{
		PRINT_FLATBUFFER_RESOURCE_ERROR(
			"Couldn't create buffer '%s'", bufferName, fileName);
		return false;
	}

	if (!dsSceneResources_addResource(
			resources, bufferName, dsSceneResourceType_Buffer, buffer, true))
	{
		DS_VERIFY(dsGfxBuffer_destroy(buffer));
		return false;
	}

	return true;
}

static bool loadTexture(dsSceneResources* resources, dsResourceManager* resourceManager,
	dsAllocator* allocator, dsAllocator* resourceAllocator, const DeepSeaScene::Texture* fbTexture,
	const char* fileName)
{
	const char* textureName = fbTexture->name()->c_str();
	auto usage = static_cast<dsTextureUsage>(fbTexture->usage());
	auto memoryHints = static_cast<dsGfxMemory>(fbTexture->memoryHints());
	dsTexture* texture;
	if (auto fbFileRef = fbTexture->data_as_FileReference())
	{
		texture = dsTextureData_loadResourceToTexture(resourceManager, resourceAllocator,
			allocator, DeepSeaScene::convert(fbFileRef->type()),
			fbFileRef->path()->c_str(), nullptr, usage, memoryHints);
	}
	else if (auto fbRawData = fbTexture->data_as_RawData())
	{
		auto fbData = fbRawData->data();
		dsMemoryStream stream;
		DS_VERIFY(dsMemoryStream_open(&stream, (void*)fbData->data(), fbData->size()));
		texture = dsTextureData_loadStreamToTexture(resourceManager, resourceAllocator,
			allocator, reinterpret_cast<dsStream*>(&stream), nullptr, usage, memoryHints);
		DS_VERIFY(dsMemoryStream_close(&stream));
	}
	else if (auto fbTextureInfo = fbTexture->textureInfo())
	{
		dsTextureInfo textureInfo =
		{
			DeepSeaScene::convert(resourceManager->renderer, fbTextureInfo->format(),
				fbTextureInfo->decoration()),
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
	else
	{
		errno = EFORMAT;
		PRINT_FLATBUFFER_RESOURCE_ERROR(
			"Either texture data or texture info must be provided for texture '%s'",
			textureName, fileName);
		return false;
	}

	if (!texture)
	{
		PRINT_FLATBUFFER_RESOURCE_ERROR(
			"Couldn't create texture '%s'", textureName, fileName);
		return false;
	}

	if (!dsSceneResources_addResource(
			resources, textureName, dsSceneResourceType_Texture, texture, true))
	{
		DS_VERIFY(dsTexture_destroy(texture));
		return false;
	}

	return true;
}

static bool loadShaderVariableGroupDesc(dsSceneResources* resources,
	dsResourceManager* resourceManager, dsAllocator* allocator,
	const DeepSeaScene::ShaderVariableGroupDesc* fbGroupDesc, const char* fileName,
	dsAllocator* scratchAllocator, void*& tempData, size_t& tempDataSize)
{
	const auto fbElements = fbGroupDesc->elements();
	if (!fbElements)
		return true;;

	const char* groupDescName = fbGroupDesc->name()->c_str();
	uint32_t elementCount = fbElements->size();
	if (elementCount == 0)
		return true;

	uint32_t dummyCount = 0;
	uint32_t maxElements = static_cast<uint32_t>(tempDataSize/sizeof(dsShaderVariableElement));
	if (!dsResizeableArray_add(scratchAllocator, &tempData, &dummyCount, &maxElements,
			sizeof(dsShaderVariableElement), elementCount))
	{
		return false;
	}

	tempDataSize = maxElements*sizeof(dsShaderVariableElement);
	auto elements = reinterpret_cast<dsShaderVariableElement*>(tempData);

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
			"Couldn't create shader variable group description '%s'", groupDescName, fileName);
		return false;
	}

	if (!dsSceneResources_addResource(resources, groupDescName,
			dsSceneResourceType_ShaderVariableGroupDesc, groupDesc, true))
	{
		DS_VERIFY(dsShaderVariableGroupDesc_destroy(groupDesc));
		return false;
	}

	return true;
}

static bool loadShaderVariableGroup(dsSceneResources* resources,
	dsResourceManager* resourceManager, dsAllocator* allocator, dsSceneLoadScratchData* scratchData,
	const DeepSeaScene::ShaderVariableGroup* fbGroup, const char* fileName)
{
	dsCommandBuffer* commandBuffer = dsSceneLoadScratchData_getCommandBuffer(scratchData);
	const char* groupName = fbGroup->name()->c_str();
	if (!commandBuffer)
	{
		errno = EPERM;
		PRINT_FLATBUFFER_RESOURCE_ERROR(
			"Command buffer not available to set data on variable group '%s'", groupName, fileName);
		return false;
	}

	const char* groupDescName = fbGroup->description()->c_str();
	dsShaderVariableGroupDesc* groupDesc;
	dsSceneResourceType resourceType;
	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&groupDesc), scratchData, groupDescName) ||
		resourceType != dsSceneResourceType_ShaderVariableGroupDesc)
	{
		// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
		errno = ENOTFOUND;
		PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("shader variable group", groupDescName, fileName);
		return false;
	}

	dsShaderVariableGroup* group = dsShaderVariableGroup_create(resourceManager, allocator,
		nullptr, groupDesc);
	if (!group)
	{
		PRINT_FLATBUFFER_RESOURCE_ERROR(
			"Couldn't create shader variable group '%s'", groupName, fileName);
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
		return true;

	for (auto fbData : *variableData)
	{
		if (!fbData)
			continue;

		const char* dataName = fbData->name()->c_str();
		uint32_t element = dsShaderVariableGroupDesc_findElement(groupDesc, dataName);
		if (element == DS_MATERIAL_UNKNOWN)
		{
			PRINT_FLATBUFFER_MATERIAL_ERROR(
				"Couldn't find shader variable group element '%s'", dataName, fileName);
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
				fileName);
			return false;
		}

		if (!dsShaderVariableGroup_setElementData(group, element, data->data(),
				type, fbData->first(), count))
		{
			PRINT_FLATBUFFER_MATERIAL_ERROR(
				"Couldn't set shader variable group element '%s'", dataName, fileName);
			return false;
		}
	}

	dsShaderVariableGroup_commit(group, commandBuffer);
	return true;
}

static bool loadMaterialDesc(dsSceneResources* resources, dsResourceManager* resourceManager,
	dsAllocator* allocator, dsSceneLoadScratchData* scratchData,
	const DeepSeaScene::MaterialDesc* fbMaterialDesc, const char* fileName,
	dsAllocator* scratchAllocator, void*& tempData, size_t& tempDataSize)
{
	const auto fbElements = fbMaterialDesc->elements();
	if (!fbElements)
		return true;;

	uint32_t elementCount = fbElements->size();
	if (elementCount == 0)
		return true;

	uint32_t dummyCount = 0;
	uint32_t maxElements = static_cast<uint32_t>(tempDataSize/sizeof(dsMaterialElement));
	if (!dsResizeableArray_add(scratchAllocator, &tempData, &dummyCount, &maxElements,
			sizeof(dsMaterialElement), elementCount))
	{
		return false;
	}

	tempDataSize = maxElements*sizeof(dsMaterialElement);
	auto elements = reinterpret_cast<dsMaterialElement*>(tempData);

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
				PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("shader variable group", groupDescName->c_str(),
					fileName);
				return false;
			}
			curElement->shaderVariableGroupDesc = groupDesc;
		}
		else
			curElement->shaderVariableGroupDesc = nullptr;
		curElement->binding = DeepSeaScene::convert(fbElement->binding());
		++curElement;
	}

	dsMaterialDesc* materialDesc = dsMaterialDesc_create(resourceManager, allocator, elements,
		fbElements->size());

	if (!materialDesc)
	{
		PRINT_FLATBUFFER_RESOURCE_ERROR(
			"Couldn't create material description '%s'", materialDescName, fileName);
		return false;
	}

	if (!dsSceneResources_addResource(resources, materialDescName,
			dsSceneResourceType_MaterialDesc, materialDesc, true))
	{
		DS_VERIFY(dsMaterialDesc_destroy(materialDesc));
		return false;
	}

	return true;
}

static bool loadMaterialTexture(dsSceneLoadScratchData* scratchData, dsMaterial* material,
	uint32_t element, const uint8_t* data, uint32_t dataSize, const char* dataName,
	const char* fileName)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScene::VerifyNamedMaterialDataBuffer(verifier))
	{
		errno = EFORMAT;
		PRINT_FLATBUFFER_MATERIAL_ERROR(
			"Invalid NamedMaterialData flatbuffer data for element '%s'", dataName, fileName);
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
		PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("texture", textureName, fileName);
		return false;
	}

	if (!dsMaterial_setTexture(material, element, texture))
	{
		PRINT_FLATBUFFER_MATERIAL_ERROR("Couldn't set texture '%s'", dataName, fileName);
		return false;
	}

	return true;
}

static bool loadMaterialTextureBuffer(dsSceneLoadScratchData* scratchData,
	const dsRenderer* renderer, dsMaterial* material, uint32_t element, const uint8_t* data,
	uint32_t dataSize, const char* dataName, const char* fileName)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScene::VerifyTextureBufferMaterialDataBuffer(verifier))
	{
		errno = EFORMAT;
		PRINT_FLATBUFFER_MATERIAL_ERROR(
			"Invalid TextureBufferMaterialData flatbuffer data for element '%s'", dataName,
			fileName);
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
		PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("buffer", bufferName, fileName);
		return false;
	}

	if (!dsMaterial_setTextureBuffer(material, element, buffer,
			DeepSeaScene::convert(renderer, materialData->format(), materialData->decoration()),
			materialData->offset(), materialData->count()))
	{
		PRINT_FLATBUFFER_MATERIAL_ERROR("Couldn't set texture buffer '%s'", dataName, fileName);
		return false;
	}

	return true;
}

static bool loadMaterialVariableGroup(dsSceneLoadScratchData* scratchData, dsMaterial* material,
	uint32_t element, const uint8_t* data, uint32_t dataSize, const char* dataName,
	const char* fileName)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScene::VerifyNamedMaterialDataBuffer(verifier))
	{
		errno = EFORMAT;
		PRINT_FLATBUFFER_MATERIAL_ERROR(
			"Invalid NamedMaterialData flatbuffer data for element '%s'", dataName, fileName);
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
		PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("shader variable group", textureName, fileName);
		return false;
	}

	if (!dsMaterial_setVariableGroup(material, element, variableGroup))
	{
		PRINT_FLATBUFFER_MATERIAL_ERROR("Couldn't set shader variable group '%s'", dataName,
			fileName);
		return false;
	}

	return true;
}

static bool loadMaterialBuffer(dsSceneLoadScratchData* scratchData, dsMaterial* material,
	uint32_t element, const uint8_t* data, uint32_t dataSize, const char* dataName,
	const char* fileName)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScene::VerifyBufferMaterialDataBuffer(verifier))
	{
		errno = EFORMAT;
		PRINT_FLATBUFFER_MATERIAL_ERROR(
			"Invalid BufferMaterialData flatbuffer data for element '%s'", dataName, fileName);
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
		PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("buffer", bufferName, fileName);
		return false;
	}

	if (!dsMaterial_setBuffer(material, element, buffer, materialData->offset(),
			materialData->size()))
	{
		PRINT_FLATBUFFER_MATERIAL_ERROR("Couldn't set buffer '%s'", dataName, fileName);
		return false;
	}

	return true;
}

static bool loadMaterialData(dsMaterial* material, uint32_t element, dsMaterialType type,
	uint32_t first, uint32_t count, const uint8_t* data, uint32_t dataSize, const char* dataName,
	const char* fileName)
{
	uint32_t expectedSize = dsMaterialType_cpuSize(type)*count;
	if (dataSize != expectedSize)
	{
		PRINT_FLATBUFFER_MATERIAL_ERROR(
			"Incorrect data size for material element '%s'", dataName,
			fileName);
		return false;
	}

	if (!dsMaterial_setElementData(material, element, data, type, first, count))
	{
		PRINT_FLATBUFFER_MATERIAL_ERROR(
			"Couldn't set material element '%s'", dataName, fileName);
		return false;
	}

	return true;
}

static bool loadMaterial(dsSceneResources* resources, dsResourceManager* resourceManager,
	dsAllocator* allocator, dsSceneLoadScratchData* scratchData,
	const DeepSeaScene::Material* fbMaterial, const char* fileName)
{
	const char* materialDescName = fbMaterial->description()->c_str();
	dsMaterialDesc* materialDesc;
	dsSceneResourceType resourceType;
	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&materialDesc), scratchData, materialDescName) ||
		resourceType != dsSceneResourceType_MaterialDesc)
	{
		// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
		errno = ENOTFOUND;
		PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("material desc", materialDescName, fileName);
		return false;
	}

	const char* materialName = fbMaterial->name()->c_str();
	dsMaterial* material = dsMaterial_create(resourceManager, allocator, materialDesc);
	if (!material)
	{
		PRINT_FLATBUFFER_RESOURCE_ERROR(
			"Couldn't create material '%s'", materialName, fileName);
		return false;
	}

	// NOTE: This takes ownership on success, so errors after this point won't destroy the
	// material.
	if (!dsSceneResources_addResource(
			resources, materialName, dsSceneResourceType_Material, material, true))
	{
		dsMaterial_destroy(material);
		return false;
	}

	auto* variableData = fbMaterial->data();
	if (!variableData)
		return true;

	for (auto fbData : *variableData)
	{
		if (!fbData)
			continue;

		const char* dataName = fbData->name()->c_str();
		uint32_t element = dsMaterialDesc_findElement(materialDesc, dataName);
		if (element == DS_MATERIAL_UNKNOWN)
		{
			PRINT_FLATBUFFER_MATERIAL_ERROR(
				"Couldn't find material element '%s'", dataName, fileName);
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
					data->size(), dataName, fileName);
				break;
			case dsMaterialType_TextureBuffer:
			case dsMaterialType_ImageBuffer:
				success = loadMaterialTextureBuffer(scratchData, resourceManager->renderer,
					material, element, data->data(), data->size(), dataName, fileName);
				break;
			case dsMaterialType_VariableGroup:
				success = loadMaterialVariableGroup(scratchData, material, element,
					data->data(), data->size(), dataName, fileName);
				break;
			case dsMaterialType_UniformBlock:
			case dsMaterialType_UniformBuffer:
				success = loadMaterialBuffer(scratchData, material, element, data->data(),
					data->size(), dataName, fileName);
				break;
			default:
			{
				success = loadMaterialData(material, element, type, fbData->first(),
					fbData->count(), data->data(), data->size(), dataName, fileName);
				break;
			}
		}

		if (!success)
			return false;
	}

	return true;
}

static dsShaderModule* loadShaderModule(dsResourceManager* resourceManager, dsAllocator* allocator,
	const FlatbufferVector<DeepSeaScene::VersionedShaderModule>* shaderModules,
	const char* shaderModuleName, const char* fileName)
{
	if (!shaderModules)
		return nullptr;

	uint32_t shaderModuleCount = shaderModules->size();
	auto versionStrings = DS_ALLOCATE_STACK_OBJECT_ARRAY(const char*, shaderModuleCount);
	for (uint32_t i = 0; i < shaderModuleCount; ++i)
	{
		auto fbShaderModule = (*shaderModules)[i];
		versionStrings[i] = fbShaderModule ? fbShaderModule->version()->c_str() : nullptr;
	}

	uint32_t versionIndex;
	const char* versionString = dsRenderer_chooseShaderVersionString(&versionIndex,
		resourceManager->renderer, versionStrings, shaderModuleCount);
	if (!versionString)
	{
		errno = ENOTFOUND;
		PRINT_FLATBUFFER_RESOURCE_ERROR("No supported version found for shader module '%s'",
			shaderModuleName, fileName);
		return nullptr;
	}

	auto fbShaderModule = (*shaderModules)[versionIndex];
	if (auto fbFileRef = fbShaderModule->data_as_FileReference())
	{
		return dsShaderModule_loadResource(resourceManager, allocator,
			DeepSeaScene::convert(fbFileRef->type()), fbFileRef->path()->c_str(),
			shaderModuleName);
	}
	else if (auto fbRawData = fbShaderModule->data_as_RawData())
	{
		auto fbData = fbRawData->data();
		return dsShaderModule_loadData(resourceManager, allocator, fbData->data(), fbData->size(),
			shaderModuleName);
	}
	else
	{
		errno = EFORMAT;
		PRINT_FLATBUFFER_RESOURCE_ERROR("No data provided for shader module '%s'",
			shaderModuleName, fileName);
		return nullptr;
	}
}

static bool loadShaderModule(dsSceneResources* resources, dsResourceManager* resourceManager,
	dsAllocator* allocator, const DeepSeaScene::ShaderModule* fbShaderModule,
	const char* fileName)
{
	const char* shaderModuleName = fbShaderModule->name()->c_str();
	dsShaderModule* shaderModule = loadShaderModule(resourceManager, allocator,
		fbShaderModule->modules(), shaderModuleName, fileName);

	if (!shaderModule)
	{
		PRINT_FLATBUFFER_RESOURCE_ERROR(
			"Couldn't load shader module '%s'", shaderModuleName, fileName);
		return false;
	}

	if (!dsSceneResources_addResource(resources, shaderModuleName,
			dsSceneResourceType_ShaderModule, shaderModule, true))
	{
		DS_VERIFY(dsShaderModule_destroy(shaderModule));
		return false;
	}

	return true;
}

static bool loadShader(dsSceneResources* resources, dsResourceManager* resourceManager,
	dsAllocator* allocator, dsSceneLoadScratchData* scratchData,
	const DeepSeaScene::Shader* fbShader, const char* fileName)
{
	const char* shaderModuleName = fbShader->shaderModule()->c_str();
	dsShaderModule* shaderModule;
	dsSceneResourceType resourceType;
	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&shaderModule), scratchData, shaderModuleName) ||
		resourceType != dsSceneResourceType_ShaderModule)
	{
		// NOTE: ENOTFOUND not set when the type doesn't match, so set it manually.
		errno = ENOTFOUND;
		PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("shader module", shaderModuleName, fileName);
		return false;
	}

	const char* materialDescName = fbShader->materialDesc()->c_str();
	dsMaterialDesc* materialDesc;
	if (!dsSceneLoadScratchData_findResource(&resourceType,
			reinterpret_cast<void**>(&materialDesc), scratchData, materialDescName) ||
		resourceType != dsSceneResourceType_MaterialDesc)
	{
		errno = ENOTFOUND;
		PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("material description", materialDescName, fileName);
		return false;
	}

	const char* shaderName = fbShader->name()->c_str();
	auto fbPipelineName = fbShader->pipeline();
	const char* pipelineName = fbPipelineName ? fbPipelineName->c_str() : shaderName;
	dsShader* shader = dsShader_createName(resourceManager, allocator, shaderModule,
		pipelineName, materialDesc);
	if (!shader)
	{
		PRINT_FLATBUFFER_RESOURCE_ERROR(
			"Couldn't create shader '%s'", shaderName, fileName);
		return false;
	}

	if (!dsSceneResources_addResource(
			resources, shaderName, dsSceneResourceType_Shader, shader, true))
	{
		DS_VERIFY(dsShader_destroy(shader));
		return false;
	}

	return true;
}

static bool loadDrawGeometry(dsSceneResources* resources, dsResourceManager* resourceManager,
	dsAllocator* allocator, dsSceneLoadScratchData* scratchData,
	const DeepSeaScene::DrawGeometry* fbGeometry, const char* fileName)
{
	const char* geometryName = fbGeometry->name()->c_str();
	uint32_t vertexBufferIndex = 0;
	dsVertexBuffer vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS];
	dsVertexBuffer* vertexBufferPtrs[DS_MAX_GEOMETRY_VERTEX_BUFFERS];
	memset(vertexBufferPtrs, 0, sizeof(vertexBufferPtrs));
	for (auto fbVertexBuffer : *fbGeometry->vertexBuffers())
	{
		if (vertexBufferIndex > DS_MAX_GEOMETRY_VERTEX_BUFFERS)
		{
			errno = ESIZE;
			PRINT_FLATBUFFER_RESOURCE_ERROR(
				"Too many vertex buffers for geometry '%s'", geometryName, fileName);
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
			PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("buffer", bufferName, fileName);
			return false;
		}

		vertexBuffer->offset = fbVertexBuffer->offset();
		vertexBuffer->count = fbVertexBuffer->count();

		auto fbVertexFormat = fbVertexBuffer->format();
		DS_VERIFY(dsVertexFormat_initialize(&vertexBuffer->format));
		vertexBuffer->format.instanced = fbVertexFormat->instanced();
		for (auto fbAttribute : *fbVertexFormat->attributes())
		{
			if (!fbAttribute)
				continue;

			uint32_t attribIndex = fbAttribute->attrib();
			if (attribIndex > resourceManager->maxVertexAttribs)
			{
				errno = ESIZE;
				PRINT_FLATBUFFER_RESOURCE_ERROR(
					"Attribute index is out of range for vertex buffer '%s'", bufferName,
					fileName);
				return false;
			}

			dsVertexFormat_setAttribEnabled(&vertexBuffer->format, attribIndex, true);
			vertexBuffer->format.elements[attribIndex].format = DeepSeaScene::convert(
				fbAttribute->format(), fbAttribute->decoration());
		}
		DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(&vertexBuffer->format));

		vertexBufferPtrs[vertexBufferIndex] = vertexBuffers + vertexBufferIndex;
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
			PRINT_FLATBUFFER_RESOURCE_NOT_FOUND("buffer", bufferName, fileName);
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
			"Couldn't create geometry '%s'", geometryName, fileName);
		return false;
	}

	if (!dsSceneResources_addResource(
			resources, geometryName, dsSceneResourceType_DrawGeometry, geometry, true))
	{
		DS_VERIFY(dsDrawGeometry_destroy(geometry));
		return false;
	}

	return true;
}

static bool loadSceneNode(dsSceneResources* resources, dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, const DeepSeaScene::SceneNode* fbNamedNode,
	const char* fileName)
{
	const char* nodeName = fbNamedNode->name()->c_str();
	auto fbNode = fbNamedNode->node();
	auto data = fbNode->data();
	dsSceneNode* node = dsSceneNode_load(allocator, resourceAllocator, loadContext, scratchData,
		fbNode->type()->c_str(), data->data(), data->size());
	if (!node)
	{
		PRINT_FLATBUFFER_RESOURCE_ERROR("Couldn't load scene node '%s'", nodeName, fileName);
		return false;
	}

	bool success = dsSceneResources_addResource(
		resources, nodeName, dsSceneResourceType_SceneNode, node, true);
	dsSceneNode_freeRef(node);
	if (!success)
		return false;

	return true;
}

static bool loadCustomResource(dsSceneResources* resources, dsAllocator* allocator,
	dsAllocator* resourceAllocator, const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData,
	const DeepSeaScene::CustomResource* fbCustoResource, const char* fileName)
{
	const char* resourceName = fbCustoResource->name()->c_str();
	auto fbResource = fbCustoResource->resource();
	auto data = fbResource->data();
	dsCustomSceneResource* customResource = dsCustomSceneResource_load(allocator,
		resourceAllocator, loadContext, scratchData, fbResource->type()->c_str(), data->data(),
		data->size());
	if (!customResource)
	{
		PRINT_FLATBUFFER_RESOURCE_ERROR("Couldn't load custom scene resource '%s'",
			resourceName, fileName);
		return false;
	}

	bool success = dsSceneResources_addResource(
		resources, resourceName, dsSceneResourceType_Custom, customResource, true);
	if (!success)
	{
		DS_VERIFY(dsCustomSceneResource_destroy(customResource));
		return false;
	}

	return true;
}

extern "C"
dsSceneResources* dsSceneResources_loadImpl(dsAllocator* allocator, dsAllocator* resourceAllocator,
	const dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData,
	const void* data, size_t dataSize, const char* fileName)
{
	flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(data), dataSize);
	if (!DeepSeaScene::VerifySceneResourcesBuffer(verifier))
	{
		errno = EFORMAT;
		PRINT_FLATBUFFER_ERROR("Invalid scene resources flatbuffer format", fileName);
		return nullptr;
	}

	dsRenderer* renderer = dsSceneLoadContext_getRenderer(loadContext);
	dsResourceManager* resourceManager = renderer->resourceManager;
	if (!resourceAllocator)
		resourceAllocator = allocator;

	auto fbSceneResources = DeepSeaScene::GetSceneResources(data);

	uint32_t totalCount = 0;
	auto fbResources = fbSceneResources->resources();
	if (fbResources)
	{
		for (auto fbResource : *fbResources)
		{
			if (!fbResource)
				continue;

			++totalCount;
			auto fbCustomResource = fbResource->resource_as_CustomResource();
			if (!fbCustomResource)
				continue;

			auto fbResourceInfo = fbCustomResource->resource();
			if (!fbResourceInfo)
				continue;

			totalCount += dsSceneLoadContext_getCustomResourceAdditionalResources(loadContext,
				fbResourceInfo->type()->c_str());
		}
	}

	dsSceneResources* resources = dsSceneResources_create(allocator, totalCount);
	if (!resources)
		return nullptr;

	if (totalCount == 0)
		return resources;

	if (!dsSceneLoadScratchData_pushSceneResources(scratchData, &resources, 1))
	{
		dsSceneResources_freeRef(resources);
		DS_VERIFY(dsSceneLoadScratchData_popSceneResources(scratchData, 1));
		return nullptr;
	}

	dsAllocator* scratchAllocator = dsSceneLoadScratchData_getAllocator(scratchData);
	void* tempData = nullptr;
	size_t tempDataSize = 0;
	bool success = true;
	for (auto fbResource : *fbResources)
	{
		if (!fbResource)
			continue;

		if (auto fbBuffer = fbResource->resource_as_Buffer())
		{
			success = loadBuffer(resources, resourceManager, resourceAllocator, fbBuffer, fileName,
				scratchAllocator, tempData, tempDataSize);
		}
		else if (auto fbTexture = fbResource->resource_as_Texture())
		{
			success = loadTexture(resources, resourceManager, allocator, resourceAllocator,
				fbTexture, fileName);
		}
		else if (auto fbShaderVariableGroupDesc = fbResource->resource_as_ShaderVariableGroupDesc())
		{
			success = loadShaderVariableGroupDesc(resources, resourceManager, resourceAllocator,
				fbShaderVariableGroupDesc, fileName, scratchAllocator, tempData, tempDataSize);
		}
		else if (auto fbShaderVariableGroup = fbResource->resource_as_ShaderVariableGroup())
		{
			success = loadShaderVariableGroup(resources, resourceManager, allocator,
				scratchData, fbShaderVariableGroup, fileName);
		}
		else if (auto fbMaterialDesc = fbResource->resource_as_MaterialDesc())
		{
			success = loadMaterialDesc(resources, resourceManager, resourceAllocator, scratchData,
				fbMaterialDesc, fileName, scratchAllocator, tempData, tempDataSize);
		}
		else if (auto fbMaterial = fbResource->resource_as_Material())
		{
			success = loadMaterial(resources, resourceManager, allocator, scratchData, fbMaterial,
				fileName);
		}
		else if (auto fbShaderModule = fbResource->resource_as_ShaderModule())
		{
			success = loadShaderModule(resources, resourceManager, resourceAllocator,
				fbShaderModule, fileName);
		}
		else if (auto fbShader = fbResource->resource_as_Shader())
		{
			success = loadShader(resources, resourceManager, resourceAllocator, scratchData,
				fbShader, fileName);
		}
		else if (auto fbDrawGeometry = fbResource->resource_as_DrawGeometry())
		{
			success = loadDrawGeometry(resources, resourceManager, resourceAllocator, scratchData,
				fbDrawGeometry, fileName);
		}
		else if (auto fbSceneNode = fbResource->resource_as_SceneNode())
		{
			success = loadSceneNode(resources, allocator, resourceAllocator, loadContext,
				scratchData, fbSceneNode, fileName);
		}
		else if (auto fbCustomResource = fbResource->resource_as_CustomResource())
		{
			success = loadCustomResource(resources, allocator, resourceAllocator, loadContext,
				scratchData, fbCustomResource, fileName);
		}

		if (!success)
			break;
	}

	DS_VERIFY(dsAllocator_free(scratchAllocator, tempData));
	DS_VERIFY(dsSceneLoadScratchData_popSceneResources(scratchData, 1));
	if (!success)
	{
		dsSceneResources_freeRef(resources);
		return nullptr;
	}

	return resources;
}

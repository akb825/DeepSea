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

#include <DeepSea/Render/Resources/Material.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/MaterialType.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Types.h>
#include <string.h>

struct dsMaterial
{
	dsResourceManager* resourceManager;
	dsAllocator* allocator;
	void* deviceMaterial;
	uint8_t* data;
	const dsMaterialDesc* description;
	uint32_t* offsets;
};

typedef struct ImageBufferData
{
	dsGfxBuffer* buffer;
	dsGfxFormat format;
	size_t offset;
	size_t count;
} ImageBufferData;

typedef struct BufferData
{
	dsGfxBuffer* buffer;
	size_t offset;
	size_t size;
} BufferData;

static size_t addElementSize(size_t* curSize, dsMaterialType type, uint32_t count)
{
	if (type >= dsMaterialType_Texture && type <= dsMaterialType_SubpassInput)
	{
		DS_ASSERT(count == 0);
		size_t alignment = sizeof(void*);
		size_t offset = ((*curSize + alignment - 1)/alignment)*alignment;
		*curSize = offset + sizeof(dsTexture*);
		return offset;
	}
	else if (type >= dsMaterialType_ImageBuffer && type <= dsMaterialType_MutableImageBuffer)
	{
		DS_ASSERT(count == 0);
		size_t alignment = sizeof(void*);
		size_t offset = ((*curSize + alignment - 1)/alignment)*alignment;
		*curSize = offset + sizeof(ImageBufferData);
		return offset;
	}
	else if (type >= dsMaterialType_UniformBlock && type <= dsMaterialType_UniformBuffer)
	{
		DS_ASSERT(count == 0);
		size_t alignment = sizeof(void*);
		size_t offset = ((*curSize + alignment - 1)/alignment)*alignment;
		*curSize = offset + sizeof(BufferData);
		return offset;
	}

	return dsMaterialType_addElementCpuSize(curSize, type, count);
}

static size_t getDataSize(const dsMaterialDesc* description)
{
	size_t dataSize = 0;
	for (uint32_t i = 0; i < description->elementCount; ++i)
	{
		if (description->elements[i].isVolatile)
			continue;

		addElementSize(&dataSize, description->elements[i].type, description->elements[i].count);
	}
	return dataSize;
}

static bool validateGetSetElement(const dsMaterial* material, uint32_t element, const void* data,
	dsMaterialType type, uint32_t firstIndex, uint32_t count)
{
	if (!material || !data || count == 0)
	{
		errno = EINVAL;
		return false;
	}

	if (element >= material->description->elementCount)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid material element.");
		return false;
	}

	if (type != material->description->elements[element].type)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Type doesn't match material element type for %s.",
			material->description->elements[element].name);
		return false;
	}

	if (type >= dsMaterialType_Texture)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
			"Type must be a primitive, vector, or matrix type for %s.",
			material->description->elements[element].name);
		return false;
	}

	uint32_t maxCount = material->description->elements[element].count;
	if (maxCount == 0)
		maxCount = 1;
	if (!DS_IS_BUFFER_RANGE_VALID(firstIndex, count, maxCount))
	{
		errno = EINDEX;
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Attempting to copy too many elements for %s.",
			material->description->elements[element].name);
		return false;
	}

	return true;
}

static size_t fullAllocSize(const dsMaterialDesc* description)
{
	return DS_ALIGNED_SIZE(sizeof(dsMaterial)) + DS_ALIGNED_SIZE(getDataSize(description)) +
		DS_ALIGNED_SIZE(sizeof(uint32_t)*description->elementCount);
}

dsMaterial* dsMaterial_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	const dsMaterialDesc* description)
{
	if (!resourceManager || (!allocator && !resourceManager->allocator) || !description)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	size_t fullSize = fullAllocSize(description);
	void* fullMem = dsAllocator_alloc(allocator, fullSize);
	if (!fullMem)
		return NULL;

	dsBufferAllocator bufferAllocator;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAllocator, fullMem, fullSize));

	dsMaterial* material = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAllocator, dsMaterial);
	DS_ASSERT(material);

	material->resourceManager = resourceManager;
	material->allocator = dsAllocator_keepPointer(allocator);
	material->description = description;

	if (description->elementCount == 0)
	{
		material->data = NULL;
		material->offsets = NULL;
		return material;
	}

	size_t dataSize = getDataSize(description);
	if (dataSize > 0)
	{
		material->data = (uint8_t*)dsAllocator_alloc((dsAllocator*)&bufferAllocator, dataSize);
		DS_ASSERT(material->data);
		memset(material->data, 0, dataSize);
	}
	else
		material->data = NULL;

	material->offsets = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAllocator, uint32_t,
		description->elementCount);
	DS_ASSERT(material->offsets);
	size_t curSize = 0;
	for (uint32_t i = 0; i < description->elementCount; ++i)
	{
		if (description->elements[i].isVolatile)
			material->offsets[i] = DS_MATERIAL_UNKNOWN;
		else
		{
			material->offsets[i] = (uint32_t)addElementSize(&curSize,
				description->elements[i].type, description->elements[i].count);
		}
	}
	DS_ASSERT(curSize == dataSize);

	// Create the device material if needed.
	if (resourceManager->createDeviceMaterialFunc && resourceManager->destroyDeviceMaterialFunc)
	{
		material->deviceMaterial = resourceManager->createDeviceMaterialFunc(resourceManager,
			material, allocator);
		if (!material->deviceMaterial)
		{
			dsMaterial_destroy(material);
			return NULL;
		}
	}
	else
		material->deviceMaterial = NULL;

	return material;
}

dsResourceManager* dsMaterial_getResourceManager(const dsMaterial* material)
{
	if (!material)
	{
		errno = EINVAL;
		return NULL;
	}

	return material->resourceManager;
}

const dsMaterialDesc* dsMaterial_getDescription(const dsMaterial* material)
{
	if (!material)
	{
		errno = EINVAL;
		return NULL;
	}

	return material->description;
}

dsDeviceMaterial* dsMaterial_getDeviceMaterial(const dsMaterial* material)
{
	if (!material)
		return NULL;

	return material->deviceMaterial;
}

bool dsMaterial_getElementData(void* outData, const dsMaterial* material, uint32_t element,
	dsMaterialType type, uint32_t firstIndex, uint32_t count)
{
	if (!validateGetSetElement(material, element, outData, type, firstIndex, count))
		return false;

	// Only volatile elements should have no offset, and only non-primitives should be volatile.
	DS_ASSERT(material->offsets[element] != DS_MATERIAL_UNKNOWN);
	uint16_t stride = dsMaterialType_cpuSize(type);
	memcpy(outData, material->data + material->offsets[element] + firstIndex*stride, count*stride);
	return true;
}

const void* dsMaterial_getRawElementData(const dsMaterial* material, uint32_t element)
{
	if (!material)
	{
		errno = EINVAL;
		return NULL;
	}

	if (element >= material->description->elementCount)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid material element.");
		return NULL;
	}

	if (material->description->elements[element].type >= dsMaterialType_Texture)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Type must be a primitive, vector, or matrix type.");
		return NULL;
	}

	// Only volatile elements should have no offset, and only non-primitives should be volatile.
	DS_ASSERT(material->offsets[element] != DS_MATERIAL_UNKNOWN);
	return material->data + material->offsets[element];
}

bool dsMaterial_setElementData(dsMaterial* material, uint32_t element, const void* data,
	dsMaterialType type, uint32_t firstIndex, uint32_t count)
{
	if (!validateGetSetElement(material, element, data, type, firstIndex, count))
		return false;

	// Only volatile elements should have no offset, and only non-primitives should be volatile.
	DS_ASSERT(material->offsets[element] != DS_MATERIAL_UNKNOWN);
	uint16_t stride = dsMaterialType_cpuSize(type);
	memcpy(material->data + material->offsets[element] + firstIndex*stride, data, count*stride);

	dsResourceManager* resourceManager = material->resourceManager;
	if (material->deviceMaterial && resourceManager->materialElementChangedFunc)
	{
		resourceManager->materialElementChangedFunc(resourceManager, material,
			material->deviceMaterial, element);
	}

	return true;
}

dsTexture* dsMaterial_getTexture(const dsMaterial* material, uint32_t element)
{
	if (!material || element >= material->description->elementCount ||
		material->offsets[element] == DS_MATERIAL_UNKNOWN)
	{
		return NULL;
	}

	dsMaterialType type = material->description->elements[element].type;
	if (type < dsMaterialType_Texture || type > dsMaterialType_SubpassInput)
		return NULL;

	return *(dsTexture**)(material->data + material->offsets[element]);
}

bool dsMaterial_setTexture(dsMaterial* material, uint32_t element, dsTexture* texture)
{
	if (!material)
	{
		errno = EINVAL;
		return false;
	}

	if (element >= material->description->elementCount)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid material element.");
		return false;
	}

	if (material->offsets[element] == DS_MATERIAL_UNKNOWN)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Volatile elements cannot be set on a material.");
		return false;
	}

	dsMaterialType type = material->description->elements[element].type;
	if (type < dsMaterialType_Texture || type > dsMaterialType_SubpassInput)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Element type must be a texture type.");
		return false;
	}

	if (texture)
	{
		if (type == dsMaterialType_Texture && !(texture->usage & dsTextureUsage_Texture))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Texture doesn't support being used as a texture sampler.");
			return false;
		}

		if (type == dsMaterialType_Image && !(texture->usage & dsTextureUsage_Image))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Texture doesn't support being used as an image sampler.");
			return false;
		}

		if (type == dsMaterialType_SubpassInput &&
			!(texture->usage & dsTextureUsage_SubpassInput))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Texture doesn't support being used as a subpass input.");
			return false;
		}
	}

	*(dsTexture**)(material->data + material->offsets[element]) = texture;

	dsResourceManager* resourceManager = material->resourceManager;
	if (material->deviceMaterial && resourceManager->materialElementChangedFunc)
	{
		resourceManager->materialElementChangedFunc(resourceManager, material,
			material->deviceMaterial, element);
	}

	return true;
}

dsGfxBuffer* dsMaterial_getImageBuffer(dsGfxFormat* outFormat, size_t* outOffset,
	size_t* outCount, const dsMaterial* material, uint32_t element)
{
	if (!material || element >= material->description->elementCount ||
		material->offsets[element] == DS_MATERIAL_UNKNOWN)
	{
		return NULL;
	}

	dsMaterialType type = material->description->elements[element].type;
	if (type < dsMaterialType_ImageBuffer || type > dsMaterialType_MutableImageBuffer)
		return NULL;

	const ImageBufferData* imageData = (const ImageBufferData*)(material->data +
		material->offsets[element]);

	if (outFormat)
		*outFormat = imageData->format;
	if (outOffset)
		*outOffset = imageData->offset;
	if (outCount)
		*outCount = imageData->count;
	return imageData->buffer;
}

bool dsMaterial_setImageBuffer(dsMaterial* material, uint32_t element, dsGfxBuffer* buffer,
	dsGfxFormat format, size_t offset, size_t count)
{
	if (!material)
	{
		errno = EINVAL;
		return false;
	}

	if (element >= material->description->elementCount)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid material element.");
		return false;
	}

	if (material->offsets[element] == DS_MATERIAL_UNKNOWN)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Volatile elements cannot be set on a material.");
		return false;
	}

	dsResourceManager* resourceManager = material->resourceManager;
	if (!dsGfxFormat_imageBufferSupported(resourceManager, format))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Format not supported for image buffers.");
		return false;
	}

	dsMaterialType type = material->description->elements[element].type;
	if (type < dsMaterialType_ImageBuffer || type > dsMaterialType_MutableImageBuffer)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Element type must be a image buffer type.");
		return false;
	}

	if (buffer)
	{
		if (!(buffer->usage & (dsGfxBufferUsage_Image | dsGfxBufferUsage_MutableImage)))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Buffer doesn't support being used as a texture.");
			return false;
		}

		unsigned int formatSize = dsGfxFormat_size(format);
		if (!DS_IS_BUFFER_RANGE_VALID(offset, count*formatSize, buffer->size))
		{
			errno = EINDEX;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to bind outside of buffer range.");
			return false;
		}

		if (!resourceManager->hasImageBufferSubrange && (offset != 0 ||
			count*formatSize != buffer->size))
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Current target doesn't support using a subrange of a image buffer.");
			return false;
		}

		if (resourceManager->minImageBufferAlignment > 0 &&
			(offset % resourceManager->minImageBufferAlignment) != 0)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Image buffer offset doesn't match alignment requirements.");
			return false;
		}

		if (count > resourceManager->maxImageBufferElements)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Image buffer elements exceeds the maximum for the current target.");
			return false;
		}
	}

	ImageBufferData* imageData = (ImageBufferData*)(material->data + material->offsets[element]);
	imageData->buffer = buffer;
	imageData->format = format;
	imageData->offset = offset;
	imageData->count = count;

	if (material->deviceMaterial && resourceManager->materialElementChangedFunc)
	{
		resourceManager->materialElementChangedFunc(resourceManager, material,
			material->deviceMaterial, element);
	}

	return true;
}

dsShaderVariableGroup* dsMaterial_getVariableGroup(const dsMaterial* material, uint32_t element)
{
	if (!material || element >= material->description->elementCount ||
		material->offsets[element] == DS_MATERIAL_UNKNOWN ||
		material->description->elements[element].type != dsMaterialType_VariableGroup)
	{
		return NULL;
	}

	return *(dsShaderVariableGroup**)(material->data + material->offsets[element]);
}

bool dsMaterial_setVariableGroup(dsMaterial* material, uint32_t element,
	dsShaderVariableGroup* group)
{
	if (!material)
	{
		errno = EINVAL;
		return false;
	}

	if (element >= material->description->elementCount)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid material element.");
		return false;
	}

	if (material->offsets[element] == DS_MATERIAL_UNKNOWN)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Volatile elements cannot be set on a material.");
		return false;
	}

	if (material->description->elements[element].type != dsMaterialType_VariableGroup)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Element type must be a shader variable group.");
		return false;
	}

	if (group && dsShaderVariableGroup_getDescription(group) !=
		material->description->elements[element].shaderVariableGroupDesc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Shader variable group description doesn't match "
			"description set on material element.");
		return false;
	}

	*(dsShaderVariableGroup**)(material->data + material->offsets[element]) = group;

	dsResourceManager* resourceManager = material->resourceManager;
	if (material->deviceMaterial && resourceManager->materialElementChangedFunc)
	{
		resourceManager->materialElementChangedFunc(resourceManager, material,
			material->deviceMaterial, element);
	}

	return true;
}

dsGfxBuffer* dsMaterial_getBuffer(size_t* outOffset, size_t* outSize, const dsMaterial* material,
	uint32_t element)
{
	if (!material || element >= material->description->elementCount ||
		material->offsets[element] == DS_MATERIAL_UNKNOWN)
	{
		return NULL;
	}

	dsMaterialType type = material->description->elements[element].type;
	if (type < dsMaterialType_UniformBlock || type > dsMaterialType_UniformBuffer)
		return NULL;

	const BufferData* bufferData = (const BufferData*)(material->data + material->offsets[element]);
	if (outOffset)
		*outOffset = bufferData->offset;
	if (outSize)
		*outSize = bufferData->size;
	return bufferData->buffer;
}

bool dsMaterial_setBuffer(dsMaterial* material, uint32_t element, dsGfxBuffer* buffer,
	size_t offset, size_t size)
{
	if (!material)
	{
		errno = EINVAL;
		return false;
	}

	if (element >= material->description->elementCount)
	{
		errno = EINDEX;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid material element.");
		return false;
	}

	if (material->offsets[element] == DS_MATERIAL_UNKNOWN)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Volatile elements cannot be set on a material.");
		return false;
	}

	dsMaterialType type = material->description->elements[element].type;
	if (type < dsMaterialType_UniformBlock || type > dsMaterialType_UniformBuffer)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Element type must be a buffer type.");
		return false;
	}

	if (buffer)
	{
		if (type == dsMaterialType_UniformBlock && !(buffer->usage & dsGfxBufferUsage_UniformBlock))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Buffer doesn't support being used as a uniform block.");
			return false;
		}

		if (type == dsMaterialType_UniformBuffer &&
			!(buffer->usage & dsGfxBufferUsage_UniformBuffer))
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Buffer doesn't support being used as a uniform buffer.");
			return false;
		}

		if (!DS_IS_BUFFER_RANGE_VALID(offset, size, buffer->size))
		{
			errno = EINDEX;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Attempting to bind outside of buffer range.");
			return false;
		}

		if (type == dsMaterialType_UniformBlock &&
			size > buffer->resourceManager->maxUniformBlockSize)
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_RENDER_LOG_TAG,
				"Buffer size exceeds the maximum uniform block size for the current target.");
			return false;
		}
	}

	BufferData* bufferData = (BufferData*)(material->data + material->offsets[element]);
	bufferData->buffer = buffer;
	bufferData->offset = offset;
	bufferData->size = size;

	dsResourceManager* resourceManager = material->resourceManager;
	if (material->deviceMaterial && resourceManager->materialElementChangedFunc)
	{
		resourceManager->materialElementChangedFunc(resourceManager, material,
			material->deviceMaterial, element);
	}

	return true;
}

bool dsMaterial_commit(dsMaterial* material, dsCommandBuffer* commandBuffer)
{
	if (!material || !commandBuffer)
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < material->description->elementCount; ++i)
	{
		dsShaderVariableGroup* variableGroup = dsMaterial_getVariableGroup(material, i);
		if (variableGroup && !dsShaderVariableGroup_commit(variableGroup, commandBuffer))
			return false;
	}

	return true;
}

void dsMaterial_destroy(dsMaterial* material)
{
	if (!material)
		return;

	if (material->deviceMaterial)
	{
		dsResourceManager* resourceManager = material->resourceManager;
		resourceManager->destroyDeviceMaterialFunc(resourceManager, material,
			material->deviceMaterial);
	}

	if (material->allocator)
		DS_VERIFY(dsAllocator_free(material->allocator, material));
}

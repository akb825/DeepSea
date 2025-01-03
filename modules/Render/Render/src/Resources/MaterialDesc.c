/*
 * Copyright 2017-2024 Aaron Barany
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

#include <DeepSea/Render/Resources/MaterialDesc.h>

#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Types.h>

#include <MSL/Client/ModuleC.h>
#include <string.h>

extern const char* dsResourceManager_noContextError;

static const dsMaterialType materialTypeMap[] =
{
	// Scalars and vectors
	dsMaterialType_Float,  // mslType_Float
	dsMaterialType_Vec2,   // mslType_Vec2
	dsMaterialType_Vec3,   // mslType_Vec3
	dsMaterialType_Vec4,   // mslType_Vec4
	dsMaterialType_Double, // mslType_Double
	dsMaterialType_DVec2,  // mslType_DVec2
	dsMaterialType_DVec3,  // mslType_DVec3
	dsMaterialType_DVec4,  // mslType_DVec4
	dsMaterialType_Int,    // mslType_Int
	dsMaterialType_IVec2,  // mslType_IVec2
	dsMaterialType_IVec3,  // mslType_IVec3
	dsMaterialType_IVec4,  // mslType_IVec4
	dsMaterialType_UInt,   // mslType_UInt
	dsMaterialType_UVec2,  // mslType_UVec2
	dsMaterialType_UVec3,  // mslType_UVec3
	dsMaterialType_UVec4,  // mslType_UVec4
	dsMaterialType_Bool,   // mslType_Bool
	dsMaterialType_BVec2,  // mslType_BVec2
	dsMaterialType_BVec3,  // mslType_BVec3
	dsMaterialType_BVec4,  // mslType_BVec4

	// Matrices
	dsMaterialType_Mat2,    // mslType_Mat2
	dsMaterialType_Mat3,    // mslType_Mat3
	dsMaterialType_Mat4,    // mslType_Mat4
	dsMaterialType_Mat2x3,  // mslType_Mat2x3
	dsMaterialType_Mat2x4,  // mslType_Mat2x4
	dsMaterialType_Mat3x2,  // mslType_Mat3x2
	dsMaterialType_Mat3x4,  // mslType_Mat3x4
	dsMaterialType_Mat4x2,  // mslType_Mat4x2
	dsMaterialType_Mat4x3,  // mslType_Mat4x3
	dsMaterialType_DMat2,   // mslType_DMat2
	dsMaterialType_DMat3,   // mslType_DMat3
	dsMaterialType_DMat4,   // mslType_DMat4
	dsMaterialType_DMat2x3, // mslType_DMat2x3
	dsMaterialType_DMat2x4, // mslType_DMat2x4
	dsMaterialType_DMat3x2, // mslType_DMat3x2
	dsMaterialType_DMat3x4, // mslType_DMat3x4
	dsMaterialType_DMat4x2, // mslType_DMat4x2
	dsMaterialType_DMat4x3, // mslType_DMat4x3

	// Samplers
	dsMaterialType_Texture, // mslType_Sampler1D
	dsMaterialType_Texture, // mslType_Sampler2D
	dsMaterialType_Texture, // mslType_Sampler3D
	dsMaterialType_Texture, // mslType_SamplerCube
	dsMaterialType_Texture, // mslType_Sampler1DShadow
	dsMaterialType_Texture, // mslType_Sampler2DShadow
	dsMaterialType_Texture, // mslType_Sampler1DArray
	dsMaterialType_Texture, // mslType_Sampler2DArray
	dsMaterialType_Texture, // mslType_Sampler1DArrayShadow
	dsMaterialType_Texture, // mslType_Sampler2DArrayShadow
	dsMaterialType_Texture, // mslType_Sampler2DMS
	dsMaterialType_Texture, // mslType_Sampler2DMSArray
	dsMaterialType_Texture, // mslType_SamplerCubeShadow
	dsMaterialType_Texture, // mslType_SamplerBuffer
	dsMaterialType_Texture, // mslType_Sampler2DRect
	dsMaterialType_Texture, // mslType_Sampler2DRectShadow
	dsMaterialType_Texture, // mslType_ISampler1D
	dsMaterialType_Texture, // mslType_ISampler2D
	dsMaterialType_Texture, // mslType_ISampler3D
	dsMaterialType_Texture, // mslType_ISamplerCube
	dsMaterialType_Texture, // mslType_ISampler1DArray
	dsMaterialType_Texture, // mslType_ISampler2DArray
	dsMaterialType_Texture, // mslType_ISampler2DMS
	dsMaterialType_Texture, // mslType_ISampler2DMSArray
	dsMaterialType_Texture, // mslType_ISampler2DRect
	dsMaterialType_Texture, // mslType_USampler1D
	dsMaterialType_Texture, // mslType_USampler2D
	dsMaterialType_Texture, // mslType_USampler3D
	dsMaterialType_Texture, // mslType_USamplerCube
	dsMaterialType_Texture, // mslType_USampler1DArray
	dsMaterialType_Texture, // mslType_USampler2DArray
	dsMaterialType_Texture, // mslType_USampler2DMS
	dsMaterialType_Texture, // mslType_USampler2DMSArray
	dsMaterialType_Texture, // mslType_USampler2DRect

	// Images
	dsMaterialType_Image, // mslType_Image1D
	dsMaterialType_Image, // mslType_Image2D
	dsMaterialType_Image, // mslType_Image3D
	dsMaterialType_Image, // mslType_ImageCube
	dsMaterialType_Image, // mslType_Image1DArray
	dsMaterialType_Image, // mslType_Image2DArray
	dsMaterialType_Image, // mslType_Image2DMS
	dsMaterialType_Image, // mslType_Image2DMSArray
	dsMaterialType_Image, // mslType_TextureBuffer
	dsMaterialType_Image, // mslType_Image2DRect
	dsMaterialType_Image, // mslType_IImage1D
	dsMaterialType_Image, // mslType_IImage2D
	dsMaterialType_Image, // mslType_IImage3D
	dsMaterialType_Image, // mslType_IImageCube
	dsMaterialType_Image, // mslType_IImage1DArray
	dsMaterialType_Image, // mslType_IImage2DArray
	dsMaterialType_Image, // mslType_IImage2DMS
	dsMaterialType_Image, // mslType_IImage2DMSArray
	dsMaterialType_Image, // mslType_IImage2DRect
	dsMaterialType_Image, // mslType_UImage1D
	dsMaterialType_Image, // mslType_UImage2D
	dsMaterialType_Image, // mslType_UImage3D
	dsMaterialType_Image, // mslType_UImageCube
	dsMaterialType_Image, // mslType_UImage1DArray
	dsMaterialType_Image, // mslType_UImage2DArray
	dsMaterialType_Image, // mslType_UImage2DMS
	dsMaterialType_Image, // mslType_UImage2DMSArray
	dsMaterialType_Image, // mslType_UImage2DRect

	// Subpass inputs.
	dsMaterialType_SubpassInput, // mslType_SubpassInput
	dsMaterialType_SubpassInput, // mslType_SubpassInputMS
	dsMaterialType_SubpassInput, // mslType_ISubpassInput
	dsMaterialType_SubpassInput, // mslType_ISubpassInputMS
	dsMaterialType_SubpassInput, // mslType_USubpassInput
	dsMaterialType_SubpassInput, // mslType_USubpassInputMS

	// Other.
	dsMaterialType_VariableGroup, // mslType_Struct (also UniformBlock or UniformBuffer)
};

_Static_assert(DS_ARRAY_SIZE(materialTypeMap) == mslType_Count, "Material type map mismatch.");

static bool hasDuplicates(const dsMaterialElement* elements, uint32_t elementCount)
{
	bool hasDuplicate = false;
	for (uint32_t i = 0; i < elementCount; ++i)
	{
		for (uint32_t j = i + 1; j < elementCount; ++j)
		{
			if (strcmp(elements[i].name, elements[j].name) == 0)
			{
				DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Element '%s' specified multiple times.",
					elements[i].name);
				hasDuplicate = true;
				break;
			}
		}
	}

	return hasDuplicate;
}

dsMaterialDesc* dsMaterialDesc_create(dsResourceManager* resourceManager,
	dsAllocator* allocator, const dsMaterialElement* elements, uint32_t elementCount)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createMaterialDescFunc || !resourceManager->destroyMaterialDescFunc ||
		(!elements && elementCount > 0))
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	bool elementsValid = !hasDuplicates(elements, elementCount);
	for (uint32_t i = 0; i < elementCount; ++i)
	{
		if (!elements[i].name)
		{
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Element name not given.");
			elementsValid = false;
			continue;
		}

		if ((unsigned int)elements[i].type >= dsMaterialType_Count)
		{
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Invalid type for element '%s'.", elements[i].name);
			elementsValid = false;
		}

		if (elements[i].binding != dsMaterialBinding_Material &&
			elements[i].type < dsMaterialType_Texture)
		{
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Primitive, vector, and matrix material elements "
				"must use dsMaterialBinding_Material for element '%s'.", elements[i].name);
			elementsValid = false;
		}

		if (elements[i].type == dsMaterialType_VariableGroup &&
			!elements[i].shaderVariableGroupDesc)
		{
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
				"Variable group material element '%s' missing shaderVaraibleGroupDesc.",
				elements[i].name);
			elementsValid = false;
		}

		if (elements[i].type == dsMaterialType_UniformBlock &&
			!(resourceManager->supportedBuffers & dsGfxBufferUsage_UniformBlock))
		{
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
				"Target doesn't support uniform blocks for element '%s'.", elements[i].name);
			elementsValid = false;
		}

		if (elements[i].type == dsMaterialType_UniformBuffer &&
			!(resourceManager->supportedBuffers & dsGfxBufferUsage_UniformBuffer))
		{
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
				"Target doesn't support uniform buffers for element '%s'.", elements[i].name);
			elementsValid = false;
		}

		if (elements[i].type >= dsMaterialType_Texture && elements[i].count > 0)
		{
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG,
				"Only primitive, vector, and matrix types can use arrays for element '%s'.",
				elements[i].name);
			elementsValid = false;
		}
	}

	if (!elementsValid)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsMaterialDesc* materialDesc = resourceManager->createMaterialDescFunc(resourceManager,
		allocator, elements, elementCount);
	if (materialDesc)
	{
		DS_ASSERT(materialDesc->elementCount == elementCount);
		for (uint32_t i = 0; i < elementCount; ++i)
		{
			materialDesc->elements[i].nameID = dsUniqueNameID_create(
				materialDesc->elements[i].name);
		}
		DS_ATOMIC_FETCH_ADD32(&resourceManager->materialDescCount, 1);
	}

	DS_PROFILE_FUNC_RETURN(materialDesc);
}

uint32_t dsMaterialDesc_findElement(const dsMaterialDesc* materialDesc, const char* name)
{
	if (!materialDesc || !name)
		return DS_MATERIAL_UNKNOWN;

	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		if (strcmp(materialDesc->elements[i].name, name) == 0)
			return i;
	}

	return DS_MATERIAL_UNKNOWN;
}

bool dsMaterialDesc_destroy(dsMaterialDesc* materialDesc)
{
	if (!materialDesc)
		return true;

	DS_PROFILE_FUNC_START();

	if (!materialDesc->resourceManager || !materialDesc->resourceManager->destroyMaterialDescFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = materialDesc->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(false);
	}

	bool success = resourceManager->destroyMaterialDescFunc(resourceManager, materialDesc);
	if (success)
		DS_ATOMIC_FETCH_ADD32(&resourceManager->materialDescCount, -1);
	DS_PROFILE_FUNC_RETURN(success);
}

dsMaterialType dsMaterialDesc_convertMaterialType(unsigned int type)
{
	if (type >= mslType_Count)
		return dsMaterialType_Count;

	return materialTypeMap[type];
}

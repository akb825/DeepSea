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

#include <DeepSea/VectorDraw/VectorMaterialSet.h>

#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/PoolAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix33.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/VectorDraw/Gradient.h>
#include <DeepSea/VectorDraw/VectorMaterial.h>
#include <string.h>

typedef enum DirtyType
{
	DirtyType_None,
	DirtyType_ColorGradient,
	DirtyType_All
} DirtyType;

struct dsVectorMaterialSet
{
	dsAllocator* allocator;
	dsTexture* colorTexture;
	dsTexture* infoTexture;
	dsHashTable* materialTable;
	dsPoolAllocator materialPool;
	bool srgb;
};

typedef struct dsMaterialNode
{
	dsHashTableNode node;
	char name[DS_MAX_VECTOR_RESOURCE_NAME_LENGTH];
	dsVectorMaterial material;
	bool owned;
	DirtyType dirtyType;
	uint32_t index;
} dsMaterialNode;

#define TEX_WIDTH 256

size_t dsVectorMaterialSet_fullAllocSize(uint32_t maxMaterials)
{
	return DS_ALIGNED_SIZE(sizeof(dsVectorMaterialSet)) +
		dsHashTable_fullAllocSize(dsHashTable_getTableSize(maxMaterials)) +
		dsPoolAllocator_bufferSize(sizeof(dsMaterialNode), maxMaterials);
}

dsVectorMaterialSet* dsVectorMaterialSet_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, dsAllocator* textureAllocator, uint32_t maxMaterials,
	bool srgb)
{
	if (!allocator || maxMaterials == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	if (maxMaterials > DS_MAX_ALLOWED_VECTOR_MATERIALS)
	{
		errno = ESIZE;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Max vector materials must not exceed %u.",
			DS_MAX_ALLOWED_VECTOR_MATERIALS);
		return NULL;
	}

	dsGfxFormat infoFormat = dsGfxFormat_decorate(dsGfxFormat_R32G32B32A32, dsGfxFormat_Float);
	if (!dsGfxFormat_textureSupported(resourceManager, infoFormat))
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
			"Floating point textures are required for vector images.");
		return NULL;
	}

	dsGfxFormat colorFormat;
	if (srgb)
	{
		colorFormat = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_SRGB);
		if (!dsGfxFormat_textureSupported(resourceManager, colorFormat))
		{
			errno = EPERM;
			DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG,
				"sRGB textures aren't supported on the current target.");
			return NULL;
		}
	}
	else
	{
		colorFormat = dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_UNorm);
		DS_ASSERT(dsGfxFormat_textureSupported(resourceManager, colorFormat));
	}

	if (!textureAllocator)
		textureAllocator = allocator;
	uint32_t texHeight = dsNextPowerOf2(maxMaterials);
	dsTextureInfo colorTexInfo = {colorFormat, dsTextureDim_2D, TEX_WIDTH, texHeight, 0, 1, 1};
	dsTexture* colorTexture = dsTexture_create(resourceManager, textureAllocator,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo, dsGfxMemory_Dynamic, &colorTexInfo, NULL,
		0);
	if (!colorTexture)
		return NULL;

	dsTextureInfo infoTexInfo = {infoFormat, dsTextureDim_2D, 4U, texHeight, 0, 1, 1};
	dsTexture* infoTexture = dsTexture_create(resourceManager, textureAllocator,
		dsTextureUsage_Texture | dsTextureUsage_CopyTo, dsGfxMemory_Dynamic, &infoTexInfo, NULL, 0);
	if (!infoTexture)
	{
		DS_VERIFY(dsTexture_destroy(colorTexture));
		return NULL;
	}

	size_t fullSize = dsVectorMaterialSet_fullAllocSize(maxMaterials);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		DS_VERIFY(dsTexture_destroy(colorTexture));
		DS_VERIFY(dsTexture_destroy(infoTexture));
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsVectorMaterialSet* materials = DS_ALLOCATE_OBJECT(&bufferAlloc, dsVectorMaterialSet);
	DS_ASSERT(materials);

	materials->allocator = dsAllocator_keepPointer(allocator);

	uint32_t materialTableSize = dsHashTable_getTableSize(maxMaterials);
	materials->materialTable = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		dsHashTable_fullAllocSize(materialTableSize));
	DS_ASSERT(materials->materialTable);
	DS_VERIFY(dsHashTable_initialize(materials->materialTable, materialTableSize, &dsHashString,
		&dsHashStringEqual));

	size_t poolSize = dsPoolAllocator_bufferSize(sizeof(dsMaterialNode), maxMaterials);
	void* poolBuffer = dsAllocator_alloc((dsAllocator*)&bufferAlloc, poolSize);
	DS_ASSERT(poolBuffer);
	DS_VERIFY(dsPoolAllocator_initialize(&materials->materialPool, sizeof(dsMaterialNode),
		maxMaterials, poolBuffer, poolSize));

	materials->colorTexture = colorTexture;
	materials->infoTexture = infoTexture;
	materials->srgb = srgb;

	return materials;
}

bool dsVectorMaterialSet_isSRGB(const dsVectorMaterialSet* materials)
{
	return materials && materials->srgb;
}

uint32_t dsVectorMaterialSet_getRemainingMaterials(const dsVectorMaterialSet* materials)
{
	if (!materials)
		return 0;

	return (uint32_t)materials->materialPool.freeCount;
}

bool dsVectorMaterialSet_addMaterial(dsVectorMaterialSet* materials, const char* name,
	const dsVectorMaterial* material, bool ownGradient)
{
	if (!materials || !name || !material)
	{
		errno = EINVAL;
		return false;
	}

	size_t nameLength = strlen(name);
	if (nameLength >= DS_MAX_VECTOR_RESOURCE_NAME_LENGTH)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Material name '%s' exceeds maximum size of %u.",
			name, DS_MAX_VECTOR_RESOURCE_NAME_LENGTH);
		return false;
	}

	dsHashTableNode* foundNode = dsHashTable_find(materials->materialTable, name);
	if (foundNode)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Material '%s' has already been added.", name);
		return false;
	}

	if (!materials->materialTable || materials->materialPool.freeCount == 0)
	{
		errno = ESIZE;
		DS_LOG_ERROR(DS_VECTOR_DRAW_LOG_TAG, "Maximum number of materials has been exceeded.");
		return false;
	}

	dsMaterialNode* node = DS_ALLOCATE_OBJECT(&materials->materialPool, dsMaterialNode);
	DS_ASSERT(node);
	strncpy(node->name, name, nameLength + 1);
	node->material = *material;
	node->owned = ownGradient;
	node->dirtyType = DirtyType_All;
	node->index = (uint32_t)(((uint8_t*)node - (uint8_t*)materials->materialPool.buffer)/
		materials->materialPool.chunkSize);
	DS_VERIFY(dsHashTable_insert(materials->materialTable, node->name, (dsHashTableNode*)node,
		NULL));
	return true;
}

bool dsVectorMaterialSet_setMaterialColor(dsVectorMaterialSet* materials, const char* name,
	dsColor color)
{
	if (!materials || !name)
	{
		errno = EINVAL;
		return false;
	}

	dsMaterialNode* node = (dsMaterialNode*)dsHashTable_find(materials->materialTable, name);
	if (!node)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Material '%s' not found.", name);
		return false;
	}

	if (node->material.materialType != dsVectorMaterialType_Color)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Cannot set a color on gradient material '%s'.",
			name);
		return false;
	}

	node->material.color = color;
	if (node->dirtyType != DirtyType_All)
		node->dirtyType = DirtyType_ColorGradient;
	return true;
}

bool dsVectorMaterialSet_setMaterialGradient(dsVectorMaterialSet* materials,
	const char* name, const dsGradient* gradient, bool own)
{
	if (!materials || !name || dsGradient_isValid(gradient))
	{
		errno = EINVAL;
		return false;
	}

	dsMaterialNode* node = (dsMaterialNode*)dsHashTable_find(materials->materialTable, name);
	if (!node)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Material '%s' not found.", name);
		return false;
	}

	const dsGradient* curGradient = dsVectorMaterial_getGradient(&node->material);
	if (!curGradient)
	{
		errno = EINVAL;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Cannot set a gradient on color material '%s'.",
			name);
		return false;
	}

	if (curGradient != gradient)
	{
		if (node->owned)
			dsGradient_destroy((dsGradient*)curGradient);
		DS_VERIFY(dsVectorMaterial_setGradient(&node->material, gradient));
	}
	node->owned = own;
	if (node->dirtyType != DirtyType_All)
		node->dirtyType = DirtyType_ColorGradient;
	return true;
}

const dsVectorMaterial* dsVectorMaterialSet_findMaterial(const dsVectorMaterialSet* materials,
	const char* name)
{
	if (!materials || !name)
		return NULL;

	dsMaterialNode* node = (dsMaterialNode*)dsHashTable_find(materials->materialTable, name);
	if (!node)
		return NULL;

	return &node->material;
}

uint32_t dsVectorMaterialSet_findMaterialIndex(const dsVectorMaterialSet* materials,
	const char* name)
{
	if (!materials || !name)
		return DS_VECTOR_MATERIAL_NOT_FOUND;

	dsMaterialNode* node = (dsMaterialNode*)dsHashTable_find(materials->materialTable, name);
	if (!node)
		return DS_VECTOR_MATERIAL_NOT_FOUND;

	return node->index;
}

dsVectorMaterialType dsVectorMaterialSet_getMaterialType(const dsVectorMaterialSet* materials,
	const char* name)
{
	if (!materials || !name)
		return dsVectorMaterialType_Color;

	dsMaterialNode* node = (dsMaterialNode*)dsHashTable_find(materials->materialTable, name);
	if (!node)
		return dsVectorMaterialType_Color;

	return node->material.materialType;
}

bool dsVectorMaterialSet_setMaterial(dsVectorMaterialSet* materials,
	const char* name, const dsVectorMaterial* material, bool own)
{
	if (!materials || !name || !material)
	{
		errno = EINVAL;
		return false;
	}

	dsMaterialNode* node = (dsMaterialNode*)dsHashTable_remove(materials->materialTable, name);
	if (!node)
	{
		errno = ENOTFOUND;
		DS_LOG_ERROR_F(DS_VECTOR_DRAW_LOG_TAG, "Material '%s' not found.", name);
		return false;
	}

	const dsGradient* curGradient = dsVectorMaterial_getGradient(&node->material);
	const dsGradient* newGradient = dsVectorMaterial_getGradient(material);
	if (curGradient && curGradient != newGradient)
	{
		if (node->owned)
			dsGradient_destroy((dsGradient*)curGradient);
	}

	node->material = *material;
	node->owned = own;
	node->dirtyType = DirtyType_All;
	return true;
}

bool dsVectorMaterialSet_update(dsVectorMaterialSet* materials, dsCommandBuffer* commandBuffer)
{
	DS_PROFILE_FUNC_START();

	if (!materials || !commandBuffer)
	{
		errno = EINVAL;
		return false;
	}

	dsColor buffer[TEX_WIDTH];
	dsVector4f info[4];
	memset(info, 0, sizeof(info));
	for (dsMaterialNode* node = (dsMaterialNode*)materials->materialTable->list.head;
		node; node = (dsMaterialNode*)node->node.listNode.next)
	{
		if (node->dirtyType == DirtyType_None)
			continue;

		dsTexturePosition texturePos = {dsCubeFace_None, 0, node->index, 0, 0};
		const dsGradient* gradient = dsVectorMaterial_getGradient(&node->material);
		if (gradient)
		{
			for (uint32_t i = 0; i < TEX_WIDTH; ++i)
			{
				buffer[i] = dsGradient_evaluate(gradient, (float)i/(float)(TEX_WIDTH - 1),
					materials->srgb);
			}

			if (!dsTexture_copyData(materials->colorTexture, commandBuffer, &texturePos, TEX_WIDTH,
				1, 1, buffer, sizeof(buffer)))
			{
				DS_PROFILE_FUNC_RETURN(false);
			}
		}
		else
		{
			buffer[0] = node->material.color;
			if (!dsTexture_copyData(materials->colorTexture, commandBuffer, &texturePos, 1, 1, 1,
				buffer, sizeof(*buffer)))
			{
				DS_PROFILE_FUNC_RETURN(false);
			}
		}

		if (node->dirtyType == DirtyType_All)
		{
			info[0].x = (float)node->material.materialType;
			switch (node->material.materialType)
			{
				case dsVectorMaterialType_LinearGradient:
				{
					info[0].y = (float)node->material.linearGradient.edge;
					info[0].z = (float)node->material.linearGradient.coordinateSpace;
					info[1].x = node->material.linearGradient.start.x;
					info[1].y = node->material.linearGradient.start.y;
					info[1].z = node->material.linearGradient.end.x;
					info[1].w = node->material.linearGradient.end.y;

					dsMatrix33f transformInv;
					dsMatrix33f_affineInvert(&transformInv,
						&node->material.linearGradient.transform);
					info[2].x = transformInv.values[0][0];
					info[2].y = transformInv.values[0][1];
					info[2].z = transformInv.values[1][0];
					info[2].w = transformInv.values[1][1];
					info[3].x = transformInv.values[2][0];
					info[3].y = transformInv.values[2][1];
					break;
				}
				case dsVectorMaterialType_RadialGradient:
					info[0].y = (float)node->material.radialGradient.edge;
					info[0].z = (float)node->material.radialGradient.coordinateSpace;
					info[1].x = node->material.radialGradient.center.x;
					info[1].y = node->material.radialGradient.center.y;
					info[1].z = node->material.radialGradient.focus.x;
					info[1].w = node->material.radialGradient.focus.y;

					dsMatrix33f transformInv;
					dsMatrix33f_affineInvert(&transformInv,
						&node->material.radialGradient.transform);
					info[2].x = transformInv.values[0][0];
					info[2].y = transformInv.values[0][1];
					info[2].z = transformInv.values[1][0];
					info[2].w = transformInv.values[1][1];
					info[3].x = transformInv.values[2][0];
					info[3].y = transformInv.values[2][1];

					info[3].z = node->material.radialGradient.radius;
					info[3].w = node->material.radialGradient.focusRadius;
					break;
				default:
					break;
			}

			// Last texture copy had to have succeeded, so this should as well.
			DS_VERIFY(dsTexture_copyData(materials->infoTexture, commandBuffer, &texturePos, 4, 1,
				1, info, sizeof(info)));
		}

		node->dirtyType = DirtyType_None;
	}

	DS_PROFILE_FUNC_RETURN(true);
}

dsTexture* dsVectorMaterialSet_getColorTexture(const dsVectorMaterialSet* materials)
{
	if (!materials)
		return NULL;

	return materials->colorTexture;
}

dsTexture* dsVectorMaterialSet_getInfoTexture(const dsVectorMaterialSet* materials)
{
	if (!materials)
		return NULL;

	return materials->infoTexture;
}

bool dsVectorMaterialSet_destroy(dsVectorMaterialSet* materials)
{
	if (!materials)
		return true;

	if (!dsTexture_destroy(materials->colorTexture))
		return false;
	DS_VERIFY(dsTexture_destroy(materials->infoTexture));

	for (dsMaterialNode* node = (dsMaterialNode*)materials->materialTable->list.head;
		node; node = (dsMaterialNode*)node->node.listNode.next)
	{
		const dsGradient* gradient = dsVectorMaterial_getGradient(&node->material);
		if (node->owned)
			dsGradient_destroy((dsGradient*)gradient);
	}

	dsPoolAllocator_shutdown(&materials->materialPool);
	if (materials->allocator)
		DS_VERIFY(dsAllocator_free(materials->allocator, materials));
	return true;
}

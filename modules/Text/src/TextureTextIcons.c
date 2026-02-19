/*
 * Copyright 2025-2026 Aaron Barany
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

#include <DeepSea/Text/TextureTextIcons.h>

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/Resources/StreamingGfxBufferList.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Renderer.h>

#include <DeepSea/Text/TextIcons.h>

#include <string.h>

#define ENCODE_USER_DATA(userData, takeOwnership) (void*)((size_t)(userData) | !(takeOwnership))
#define HAS_OWNERSHIP(userData) (((size_t)(userData) & 0x1) == 0)
#define EXTRACT_TEXTURE(userData) (dsTexture*)((size_t)(userData) & ~(size_t)0x1)

typedef struct BufferInfo
{
	dsGfxBuffer* buffer;
	uint64_t lastUsedFrame;
} BufferInfo;

typedef struct TextureIcons
{
	dsAllocator* allocator;
	dsResourceManager* resourceManager;
	dsAllocator* resourceAllocator;
	dsSpinlock drawLock;

	const dsShader* shader;
	const dsMaterial* material;
	bool ownsMaterial;
	uint32_t textureNameID;
	uint32_t iconDataNameID;
	uint32_t iconDataStride;
	dsSharedMaterialValues* instanceValues;
	dsShaderVariableGroup* iconDataGroup;

	BufferInfo* iconDataBuffers;
	uint32_t iconDataBufferCount;
	uint32_t maxIconBuffers;

	dsGfxBuffer* vertexBuffer;
	dsDrawGeometry* drawGeometry;
} TextureIcons;

// Clockwise winding order as +y points down for text positions.
static uint8_t vertexData[] =
{
	0x00, 0x00, 0x00, 0x00,
	0x00, 0xFF, 0x00, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0x00, 0xFF, 0x00,
	0x00, 0x00, 0x00, 0x00,
};

static dsShaderVariableElement iconDataElements[] =
{
	{"modelViewProjection", dsMaterialType_Mat4, 0}
};

inline static void createBoundsMatrix(dsMatrix44f* result, const dsAlignedBox2f* bounds)
{
	result->columns[0].x = bounds->max.x - bounds->min.x;
	result->columns[0].y = 0.0f;
	result->columns[0].z = 0.0f;
	result->columns[0].w = 0.0f;
	result->columns[1].x = 0.0f;
	result->columns[1].y = bounds->max.y - bounds->min.y;
	result->columns[1].z = 0.0f;
	result->columns[1].w = 0.0f;
	result->columns[2].x = 0.0f;
	result->columns[2].y = 0.0f;
	result->columns[2].z = 1.0f;
	result->columns[2].w = 0.0f;
	result->columns[3].x = bounds->min.x;
	result->columns[3].y = bounds->min.y;
	result->columns[3].z = 0.0f;
	result->columns[3].w = 1.0f;
}

static void TextureIcons_destroy(void* userData)
{
	TextureIcons* textureIcons = (TextureIcons*)userData;
	dsSpinlock_shutdown(&textureIcons->drawLock);
	dsSharedMaterialValues_destroy(textureIcons->instanceValues);
	dsShaderVariableGroup_destroy(textureIcons->iconDataGroup);
	for (uint32_t i = 0; i < textureIcons->iconDataBufferCount; ++i)
		dsGfxBuffer_destroy(textureIcons->iconDataBuffers[i].buffer);
	dsGfxBuffer_destroy(textureIcons->vertexBuffer);
	dsDrawGeometry_destroy(textureIcons->drawGeometry);
	if (textureIcons->ownsMaterial)
		dsMaterial_destroy((dsMaterial*)textureIcons->material);
	DS_VERIFY(dsAllocator_free(textureIcons->allocator, textureIcons->iconDataBuffers));
	DS_VERIFY(dsAllocator_free(textureIcons->allocator, textureIcons));
}

static dsGfxBuffer* TextureIcons_getIconDataBuffer(TextureIcons* textureIcons, uint32_t glyphCount)
{
	dsRenderer* renderer = textureIcons->resourceManager->renderer;
	uint64_t frameNumber = renderer->frameNumber;
	size_t bufferSize = glyphCount*textureIcons->iconDataStride;

	// Look for an existing buffer we can re-use.
	uint32_t index = dsStreamingGfxBufferList_findNext(textureIcons->iconDataBuffers,
		&textureIcons->iconDataBufferCount, sizeof(BufferInfo), offsetof(BufferInfo, buffer),
		offsetof(BufferInfo, lastUsedFrame), NULL, bufferSize,
		DS_DEFAULT_STREAMING_GFX_BUFFER_FRAME_DELAY, frameNumber);
	if (index != DS_NO_STREAMING_GFX_BUFFER)
		return textureIcons->iconDataBuffers[index].buffer;

	index = textureIcons->iconDataBufferCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(textureIcons->allocator, textureIcons->iconDataBuffers,
			textureIcons->iconDataBufferCount, textureIcons->maxIconBuffers, 1))
	{
		return NULL;
	}

	BufferInfo* bufferInfo = textureIcons->iconDataBuffers + index;
	bufferInfo->buffer = dsGfxBuffer_create(textureIcons->resourceManager,
		textureIcons->resourceAllocator, dsGfxBufferUsage_UniformBlock,
		dsGfxMemory_Stream | dsGfxMemory_Synchronize, NULL, bufferSize);
	if (!bufferInfo->buffer)
	{
		--textureIcons->iconDataBufferCount;
		return false;
	}

	bufferInfo->lastUsedFrame = frameNumber;
	return bufferInfo->buffer;
}

static bool TextureIcons_drawIconDataBuffer(TextureIcons* textureIcons,
	dsCommandBuffer* commandBuffer, const dsIconGlyph* glyphs, uint32_t glyphCount,
	const dsMatrix44f* modelViewProjection, dsSharedMaterialValues* instanceValues)
{
	DS_VERIFY(dsSpinlock_lock(&textureIcons->drawLock));
	dsGfxBuffer* iconDataBuffer = TextureIcons_getIconDataBuffer(textureIcons, glyphCount);
	DS_VERIFY(dsSpinlock_unlock(&textureIcons->drawLock));
	if (!iconDataBuffer)
		return false;

	uint8_t* iconData = (uint8_t*)dsGfxBuffer_map(
		iconDataBuffer, dsGfxBufferMap_Write, 0, DS_MAP_FULL_BUFFER);
	if (!iconData)
		return false;

	for (uint32_t i = 0; i < glyphCount; ++i)
	{
		const dsIconGlyph* glyph = glyphs + i;
		dsMatrix44f* iconModelViewProjection =
			(dsMatrix44f*)(iconData + i*textureIcons->iconDataStride);
		dsMatrix44f boundsMatrix;
		createBoundsMatrix(&boundsMatrix, &glyph->bounds);
		dsMatrix44f_mul(iconModelViewProjection, modelViewProjection, &boundsMatrix);
	}

	DS_VERIFY(dsGfxBuffer_unmap(iconDataBuffer));

	bool needsLock = instanceValues == textureIcons->instanceValues;

	dsDrawRange drawRange = {6, 1, 0, 0};
	for (uint32_t i = 0; i < glyphCount; ++i)
	{
		const dsIconGlyph* glyph = glyphs + i;
		// Minimal lock only when using the internal fallback instance values.
		if (needsLock)
			DS_VERIFY(dsSpinlock_lock(&textureIcons->drawLock));
		bool setInstanceValues = dsSharedMaterialValues_setTextureID(
				instanceValues, textureIcons->textureNameID, EXTRACT_TEXTURE(glyph->userData)) &&
			dsSharedMaterialValues_setBufferID(instanceValues, textureIcons->iconDataNameID,
				iconDataBuffer, i*textureIcons->iconDataStride, sizeof(dsMatrix44f)) &&
			dsShader_updateInstanceValues(
				textureIcons->shader, commandBuffer, textureIcons->instanceValues);
		if (needsLock)
			DS_VERIFY(dsSpinlock_unlock(&textureIcons->drawLock));

		if (!setInstanceValues ||
			!dsRenderer_draw(textureIcons->resourceManager->renderer, commandBuffer,
				textureIcons->drawGeometry, &drawRange, dsPrimitiveType_TriangleList))
		{
			return false;
		}
	}
	return true;
}

static bool TextureIcons_drawIconDataGroup(TextureIcons* textureIcons,
	dsCommandBuffer* commandBuffer, const dsIconGlyph* glyphs, uint32_t glyphCount,
	const dsMatrix44f* modelViewProjection, dsSharedMaterialValues* instanceValues)
{
	dsRenderer* renderer = textureIcons->resourceManager->renderer;
	dsDrawRange drawRange = {6, 1, 0, 0};
	DS_VERIFY(dsSpinlock_lock(&textureIcons->drawLock));
	for (uint32_t i = 0; i < glyphCount; ++i)
	{
		const dsIconGlyph* glyph = glyphs + i;

		dsMatrix44f boundsMatrix, iconModelViewProjection;
		createBoundsMatrix(&boundsMatrix, &glyph->bounds);
		dsMatrix44f_mul(&iconModelViewProjection, modelViewProjection, &boundsMatrix);
		DS_VERIFY(dsShaderVariableGroup_setElementData(
			textureIcons->iconDataGroup, 0, &iconModelViewProjection, dsMaterialType_Mat4, 0, 1));
		DS_VERIFY(dsShaderVariableGroup_commitWithoutBuffer(textureIcons->iconDataGroup));

		if (!dsSharedMaterialValues_setTextureID(
				instanceValues, textureIcons->textureNameID, EXTRACT_TEXTURE(glyph->userData)) ||
			!dsSharedMaterialValues_setVariableGroupID(
				instanceValues, textureIcons->iconDataNameID, textureIcons->iconDataGroup) ||
			!dsShader_updateInstanceValues(
				textureIcons->shader, commandBuffer, instanceValues) ||
			!dsRenderer_draw(renderer, commandBuffer, textureIcons->drawGeometry, &drawRange,
				dsPrimitiveType_TriangleList))
		{
			DS_VERIFY(dsSpinlock_unlock(&textureIcons->drawLock));
			return false;
		}
	}
	DS_VERIFY(dsSpinlock_unlock(&textureIcons->drawLock));
	return true;
}

static void dsTextureTextIcons_destroyTexture(void* userData)
{
	if (HAS_OWNERSHIP(userData))
		dsTexture_destroy(EXTRACT_TEXTURE(userData));
}

static bool dsTextureTextIcons_draw(const dsTextIcons* textIcons, void* userData,
	dsCommandBuffer* commandBuffer, const dsIconGlyph* glyphs, uint32_t glyphCount,
	const dsMatrix44f* modelViewProjection, const dsSharedMaterialValues* globalValues,
	dsSharedMaterialValues* instanceValues, const dsDynamicRenderStates* renderStates)
{
	DS_UNUSED(textIcons);
	TextureIcons* textureIcons = (TextureIcons*)userData;
	if (!dsShader_bind(textureIcons->shader, commandBuffer, textureIcons->material, globalValues,
			renderStates))
	{
		return false;
	}

	if (!instanceValues)
		instanceValues = textureIcons->instanceValues;
	bool success;
	if (textureIcons->iconDataGroup)
	{
		success = TextureIcons_drawIconDataGroup(
			textureIcons, commandBuffer, glyphs, glyphCount, modelViewProjection, instanceValues);
	}
	else
	{
		success = TextureIcons_drawIconDataBuffer(
			textureIcons, commandBuffer, glyphs, glyphCount, modelViewProjection, instanceValues);
	}

	DS_VERIFY(dsShader_unbind(textureIcons->shader, commandBuffer));
	return success;
}

const char* const dsTextureTextIcons_textureName = "dsTextIconTex";
const char* const dsTextureTextIcons_iconDataName = "dsTextureTextIconData";

dsShaderVariableGroupDesc* dsTextureTextIcons_createShaderVariableGroupDesc(
	dsResourceManager* resourceManager, dsAllocator* allocator)
{
	if (!resourceManager)
	{
		errno = EINVAL;
		return NULL;
	}

	return dsShaderVariableGroupDesc_create(resourceManager, allocator, iconDataElements,
		DS_ARRAY_SIZE(iconDataElements));
}

bool dsTextureTextIcons_isShaderVariableGroupCompatible(
	const dsShaderVariableGroupDesc* transformDesc)
{
	return transformDesc &&
		dsShaderVariableGroup_areElementsEqual(iconDataElements, DS_ARRAY_SIZE(iconDataElements),
			transformDesc->elements, transformDesc->elementCount);
}

dsTextIcons* dsTextureTextIcons_create(dsAllocator* allocator, dsResourceManager* resourceManager,
	dsAllocator* resourceAllocator, const dsShader* shader, const dsMaterial* material,
	const dsIndexRange* codepointRanges, uint32_t codepointRangeCount, uint32_t maxIcons)
{
	if (!allocator || !resourceManager || !shader)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Texture text icons allocator must support freeing memory.");
		errno = EINVAL;
		return NULL;
	}

	const dsMaterialDesc* materialDesc = shader->materialDesc;
	uint32_t iconDataElement = dsMaterialDesc_findElement(
		materialDesc, dsTextureTextIcons_iconDataName);
	if (iconDataElement == DS_MATERIAL_UNKNOWN ||
		!dsTextureTextIcons_isShaderVariableGroupCompatible(
			materialDesc->elements[iconDataElement].shaderVariableGroupDesc))
	{
		DS_LOG_ERROR_F(DS_TEXT_LOG_TAG,
			"Icon shader must have shader variable element for '%s' created with "
			"dsTextureTextIcons_createShaderVariableGroupDesc().", dsTextureTextIcons_iconDataName);
		errno = EINVAL;
		return NULL;
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	TextureIcons* textureIcons = DS_ALLOCATE_OBJECT(allocator, TextureIcons);
	if (!textureIcons)
		return NULL;

	memset(textureIcons, 0, sizeof(TextureIcons));
	textureIcons->allocator = allocator;
	textureIcons->resourceManager = resourceManager;
	textureIcons->resourceAllocator = resourceAllocator;
	DS_VERIFY(dsSpinlock_initialize(&textureIcons->drawLock));

	textureIcons->shader = shader;
	if (material)
		textureIcons->material = material;
	else
	{
		textureIcons->material = dsMaterial_create(resourceManager, allocator, materialDesc);
		if (!textureIcons->material)
		{
			TextureIcons_destroy(textureIcons);
			return NULL;
		}
		textureIcons->ownsMaterial = true;
	}
	textureIcons->textureNameID = dsUniqueNameID_create(dsTextureTextIcons_textureName);
	textureIcons->iconDataNameID = dsUniqueNameID_create(dsTextureTextIcons_iconDataName);

	textureIcons->instanceValues = dsSharedMaterialValues_create(
		allocator, DS_TEXTURE_TEXT_ICONS_INSTANCE_VARIABLE_COUNT);
	if (!textureIcons->instanceValues)
	{
		TextureIcons_destroy(textureIcons);
		return NULL;
	}

	if (dsShaderVariableGroup_useGfxBuffer(resourceManager))
	{
		size_t stride = sizeof(dsMatrix44f);
		if (resourceManager->minUniformBlockAlignment > 0)
			stride = DS_CUSTOM_ALIGNED_SIZE(stride, resourceManager->minUniformBlockAlignment);
		textureIcons->iconDataStride = (uint32_t)stride;
	}
	else
	{
		textureIcons->iconDataGroup = dsShaderVariableGroup_create(
			resourceManager, allocator, resourceAllocator,
			materialDesc->elements[iconDataElement].shaderVariableGroupDesc);
		if (!textureIcons->iconDataGroup)
		{
			TextureIcons_destroy(textureIcons);
			return NULL;
		}

		DS_VERIFY(dsSharedMaterialValues_setVariableGroupID(textureIcons->instanceValues,
			textureIcons->iconDataNameID, textureIcons->iconDataGroup));
	}

	textureIcons->vertexBuffer = dsGfxBuffer_create(resourceManager, resourceAllocator,
		dsGfxBufferUsage_Vertex, dsGfxMemory_Static | dsGfxMemory_Draw, vertexData,
		sizeof(vertexData));
	if (!textureIcons->vertexBuffer)
	{
		TextureIcons_destroy(textureIcons);
		return NULL;
	}

	dsVertexBuffer vertexBuffer = {textureIcons->vertexBuffer, 0, 6};
	DS_VERIFY(dsVertexFormat_initialize(&vertexBuffer.format));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexBuffer.format, dsVertexAttrib_Position, true));
	DS_VERIFY(dsVertexFormat_setAttribEnabled(
		&vertexBuffer.format, dsVertexAttrib_TexCoord0, true));
	vertexBuffer.format.elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X8Y8, dsGfxFormat_UNorm);
	vertexBuffer.format.elements[dsVertexAttrib_TexCoord0].format =
		dsGfxFormat_decorate(dsGfxFormat_X8Y8, dsGfxFormat_UNorm);
	DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(&vertexBuffer.format));
	dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS] = {&vertexBuffer};
	textureIcons->drawGeometry = dsDrawGeometry_create(
		resourceManager, resourceAllocator, vertexBuffers, NULL);
	if (!textureIcons->drawGeometry)
	{
		TextureIcons_destroy(textureIcons);
		return NULL;
	}

	return dsTextIcons_create(allocator, codepointRanges, codepointRangeCount, maxIcons,
		DS_TEXTURE_TEXT_ICONS_INSTANCE_VARIABLE_COUNT, textureIcons, &TextureIcons_destroy, NULL,
		&dsTextureTextIcons_draw, &dsTextureTextIcons_destroyTexture);
}

bool dsTextureTextIcons_addIcon(dsTextIcons* icons, uint32_t codepoint, float advance,
	const dsAlignedBox2f* bounds, dsTexture* texture, bool takeOwnership)
{
	if (!icons || !bounds || !texture)
	{
		if (takeOwnership)
			dsTexture_destroy(texture);
		errno = EINVAL;
		return false;
	}

	// Expect that the least significant bit of the texture pointer is always zero.
	DS_ASSERT(((size_t)texture & 0x1) == 0);
	return dsTextIcons_addIcon(icons, codepoint, advance, bounds,
		ENCODE_USER_DATA(texture, takeOwnership));
}

bool dsTextureTextIcons_replaceIcon(
	dsTextIcons* icons, uint32_t codepoint, dsTexture* texture, bool takeOwnership)
{
	if (!icons || !texture)
	{
		if (takeOwnership)
			dsTexture_destroy(texture);
		errno = EINVAL;
		return false;
	}

	// Expect that the least significant bit of the texture pointer is always zero.
	DS_ASSERT(((size_t)texture & 0x1) == 0);
	return dsTextIcons_replaceIcon(icons, codepoint, ENCODE_USER_DATA(texture, takeOwnership));
}

dsTexture* dsTextureTextIcons_getIconTexture(const dsIconGlyph* icon)
{
	if (!icon)
	{
		errno = EINVAL;
		return NULL;
	}

	return EXTRACT_TEXTURE(icon->userData);
}

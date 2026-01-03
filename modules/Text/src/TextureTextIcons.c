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

#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
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

	dsShader* shader;
	dsMaterial* material;
	uint32_t textureNameID;
	uint32_t iconDataNameID;
	dsSharedMaterialValues* instanceValues;
	dsShaderVariableGroup* iconDataGroup;

	BufferInfo* iconDataBuffers;
	uint32_t iconDataBufferCount;
	uint32_t maxIconBuffers;
	uint32_t iconDataStride;

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
	{"positionData", dsMaterialType_Vec4, 0}
};

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
	DS_VERIFY(dsAllocator_free(textureIcons->allocator, textureIcons->iconDataBuffers));
	DS_VERIFY(dsAllocator_free(textureIcons->allocator, textureIcons));
}

static bool TextureIcons_drawIconDataBuffer(TextureIcons* textureIcons,
	dsCommandBuffer* commandBuffer, const dsIconGlyph* glyphs, uint32_t glyphCount)
{
	dsRenderer* renderer = textureIcons->resourceManager->renderer;
	dsDrawRange drawRange = {6, 1, 0, 0};
	uint64_t frameNumber = renderer->frameNumber;
	size_t bufferSize = glyphCount*textureIcons->iconDataStride;;

	// Look for an existing buffer we can re-use.
	uint32_t index = dsStreamingGfxBufferList_findNext(textureIcons->iconDataBuffers,
		&textureIcons->iconDataBufferCount, sizeof(BufferInfo), offsetof(BufferInfo, buffer),
		offsetof(BufferInfo, lastUsedFrame), NULL, bufferSize,
		DS_DEFAULT_STREAMING_GFX_BUFFER_FRAME_DELAY, frameNumber);
	dsGfxBuffer* iconDataBuffer;
	if (index == DS_NO_STREAMING_GFX_BUFFER)
	{
		index = textureIcons->iconDataBufferCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(textureIcons->allocator, textureIcons->iconDataBuffers,
				textureIcons->iconDataBufferCount, textureIcons->maxIconBuffers, 1))
		{
			return false;
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
		iconDataBuffer = bufferInfo->buffer;
	}
	else
		iconDataBuffer = textureIcons->iconDataBuffers[index].buffer;

	uint8_t* iconData = (uint8_t*)dsGfxBuffer_map(
		iconDataBuffer, dsGfxBufferMap_Write, 0, DS_MAP_FULL_BUFFER);
	if (!iconData)
		return false;

	for (uint32_t i = 0; i < glyphCount; ++i)
	{
		const dsIconGlyph* glyph = glyphs + i;
		dsVector4f* iconBounds = (dsVector4f*)(iconData + i*textureIcons->iconDataStride);
		iconBounds->x = glyph->bounds.min.x;
		iconBounds->y = glyph->bounds.min.y;
		iconBounds->z = glyph->bounds.max.x;
		iconBounds->w = glyph->bounds.max.y;
	}

	DS_VERIFY(dsGfxBuffer_unmap(iconDataBuffer));

	for (uint32_t i = 0; i < glyphCount; ++i)
	{
		const dsIconGlyph* glyph = glyphs + i;
		size_t glyphUserDataInt = (size_t)glyph->userData;
		// Strip off last bit indicating whether to take ownership.
		dsTexture* texture = (dsTexture*)(glyphUserDataInt & ~(size_t)0x1);
		DS_VERIFY(dsSharedMaterialValues_setTextureID(textureIcons->instanceValues,
			textureIcons->textureNameID, texture));
		DS_VERIFY(dsSharedMaterialValues_setBufferID(textureIcons->instanceValues,
			textureIcons->iconDataNameID, iconDataBuffer, i*textureIcons->iconDataStride,
			sizeof(dsVector4f)));

		if (!dsShader_updateInstanceValues(textureIcons->shader, commandBuffer,
				textureIcons->instanceValues) ||
			!dsRenderer_draw(renderer, commandBuffer, textureIcons->drawGeometry, &drawRange,
				dsPrimitiveType_TriangleList))
		{
			return false;
		}
	}
	return true;
}

static bool TextureIcons_drawIconDataGroup(TextureIcons* textureIcons,
	dsCommandBuffer* commandBuffer, const dsIconGlyph* glyphs, uint32_t glyphCount)
{
	dsRenderer* renderer = textureIcons->resourceManager->renderer;
	dsDrawRange drawRange = {6, 1, 0, 0};
	for (uint32_t i = 0; i < glyphCount; ++i)
	{
		const dsIconGlyph* glyph = glyphs + i;
		size_t glyphUserDataInt = (size_t)glyph->userData;
		// Strip off last bit indicating whether to take ownership.
		dsTexture* texture = (dsTexture*)(glyphUserDataInt & ~(size_t)0x1);
		DS_VERIFY(dsSharedMaterialValues_setTextureID(textureIcons->instanceValues,
			textureIcons->textureNameID, texture));

		dsVector4f iconBounds =
			{{glyph->bounds.min.x, glyph->bounds.min.y, glyph->bounds.max.x, glyph->bounds.max.y}};
		DS_VERIFY(dsShaderVariableGroup_setElementData(
			textureIcons->iconDataGroup, 0, &iconBounds, dsMaterialType_Vec4, 0, 1));
		DS_VERIFY(dsShaderVariableGroup_commitWithoutBuffer(textureIcons->iconDataGroup));

		if (!dsShader_updateInstanceValues(textureIcons->shader, commandBuffer,
				textureIcons->instanceValues) ||
			!dsRenderer_draw(renderer, commandBuffer, textureIcons->drawGeometry, &drawRange,
				dsPrimitiveType_TriangleList))
		{
			return false;
		}
	}
	return true;
}

static void dsTextureTextIcons_destroyTexture(void* userData)
{
	size_t userDataInt = (size_t)userData;
	if ((userDataInt & 0x1) == 0)
		dsTexture_destroy((dsTexture*)userData);
}

static bool dsTextureTextIcons_draw(const dsTextIcons* textIcons, void* userData,
	dsCommandBuffer* commandBuffer, const dsIconGlyph* glyphs, uint32_t glyphCount,
	const dsSharedMaterialValues* globalValues, const dsDynamicRenderStates* renderStates)
{
	TextureIcons* textureIcons = (TextureIcons*)userData;
	if (!dsShader_bind(textureIcons->shader, commandBuffer, textureIcons->material, globalValues,
			renderStates))
	{
		return false;
	}

	DS_VERIFY(dsSpinlock_lock(&textureIcons->drawLock));
	bool success;
	if (textureIcons->iconDataGroup)
		success = TextureIcons_drawIconDataGroup(textureIcons, commandBuffer, glyphs, glyphCount);
	else
		success = TextureIcons_drawIconDataBuffer(textureIcons, commandBuffer, glyphs, glyphCount);
	DS_VERIFY(dsSpinlock_unlock(&textureIcons->drawLock));

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
	dsAllocator* resourceAllocator, dsShader* shader, dsMaterial* material,
	const dsShaderVariableGroupDesc* iconDataDesc, const dsIndexRange* codepointRanges,
	uint32_t codepointRangeCount, uint32_t maxIcons)
{
	if (!allocator || !resourceManager || !shader || !material)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_TEXT_LOG_TAG, "Texture text icons allocator must support freeing memory.");
		return NULL;
	}

	if (!dsTextureTextIcons_isShaderVariableGroupCompatible(iconDataDesc))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_TEXT_LOG_TAG,
			"Icon data's shader variable group description must have been created with "
			"dsTextureTextIcons_createShaderVariableGroupDesc().");
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
	textureIcons->material = material;
	textureIcons->textureNameID = dsUniqueNameID_create(dsTextureTextIcons_textureName);
	textureIcons->iconDataNameID = dsUniqueNameID_create(dsTextureTextIcons_iconDataName);

	textureIcons->instanceValues = dsSharedMaterialValues_create(allocator, 2);
	if (!textureIcons->instanceValues)
	{
		TextureIcons_destroy(textureIcons);
		return NULL;
	}

	if (dsShaderVariableGroup_useGfxBuffer(resourceManager))
	{
		size_t stride = sizeof(dsVector4f);
		if (resourceManager->minUniformBlockAlignment > 0)
			stride = DS_CUSTOM_ALIGNED_SIZE(stride, resourceManager->minUniformBlockAlignment);
		textureIcons->iconDataStride = (uint32_t)stride;
	}
	else
	{
		textureIcons->iconDataGroup = dsShaderVariableGroup_create(
			resourceManager, allocator, resourceAllocator, iconDataDesc);
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
		textureIcons, &TextureIcons_destroy, NULL, &dsTextureTextIcons_draw,
		&dsTextureTextIcons_destroyTexture);
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
	size_t userDataInt = (size_t)texture | !takeOwnership;
	return dsTextIcons_addIcon(icons, codepoint, advance, bounds, (void*)userDataInt);
}

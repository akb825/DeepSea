/*
 * Copyright 2026 Aaron Barany
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

#include <DeepSea/VectorDraw/VectorTextIcons.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Thread/ThreadObjectStorage.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Render/Resources/Material.h>

#include <DeepSea/Text/TextIcons.h>

#include <DeepSea/VectorDraw/VectorImage.h>

#define ENCODE_USER_DATA(userData, takeOwnership) (void*)((size_t)(userData) | !(takeOwnership))
#define HAS_OWNERSHIP(userData) (((size_t)(userData) & 0x1) == 0)
#define EXTRACT_IMAGE(userData) (dsVectorImage*)((size_t)(userData) & ~(size_t)0x1)

typedef struct VectorIcons
{
	dsAllocator* allocator;
	dsResourceManager* resourceManager;
	const dsVectorShaders* shaders;
	dsThreadObjectStorage* materialStorage;
} VectorIcons;

static void VectorIcons_destroy(void* userData)
{
	VectorIcons* vectorIcons = (VectorIcons*)userData;
	dsThreadObjectStorage_destroy(vectorIcons->materialStorage);
	DS_VERIFY(dsAllocator_free(vectorIcons->allocator, vectorIcons));
}

static void dsTextureTextIcons_destroyImage(void* userData)
{
	if (HAS_OWNERSHIP(userData))
		dsVectorImage_destroy(EXTRACT_IMAGE(userData));
}

static bool dsVectorTextIcons_prepare(const dsTextIcons* textIcons, void* userData,
	dsCommandBuffer* commandBuffer, const dsIconGlyph* glyphs, uint32_t glyphCount)
{
	DS_UNUSED(textIcons);
	DS_UNUSED(userData);
	for (uint32_t i = 0; i < glyphCount; ++i)
	{
		dsVectorImage* image = EXTRACT_IMAGE(glyphs[i].userData);
		if (!dsVectorImage_updateText(image, commandBuffer))
			return false;
	}

	return true;
}

static bool dsVectorTextIcons_draw(const dsTextIcons* textIcons, void* userData,
	dsCommandBuffer* commandBuffer, const dsIconGlyph* glyphs, uint32_t glyphCount,
	const dsMatrix44f* modelViewProjection, const dsSharedMaterialValues* globalValues,
	const dsDynamicRenderStates* renderStates)
{
	DS_UNUSED(textIcons);
	VectorIcons* vectorIcons = (VectorIcons*)userData;
	dsMaterial* material = dsThreadObjectStorage_get(vectorIcons->materialStorage);
	if (!material)
	{
		material = dsMaterial_create(vectorIcons->resourceManager, vectorIcons->allocator,
			vectorIcons->shaders->shaderModule->materialDesc);
		if (!material || !dsThreadObjectStorage_set(vectorIcons->materialStorage, material))
			return false;
	}

	for (uint32_t i = 0; i < glyphCount; ++i)
	{
		const dsIconGlyph* glyph = glyphs + i;
		dsVectorImage* image = EXTRACT_IMAGE(glyph->userData);
		dsVector2f imageSize;
		DS_VERIFY(dsVectorImage_getSize(&imageSize, image));

		dsMatrix44f boundsMatrix =
		{{
			{(glyph->bounds.max.x - glyph->bounds.min.x)/imageSize.x, 0.0f, 0.0f, 0.0f},
			{0.0f, (glyph->bounds.min.y - glyph->bounds.max.y)/imageSize.y, 0.0f, 0.0f},
			{0.0f, 0.0f, 1.0f, 0.0f},
			{glyph->bounds.min.x, glyph->bounds.max.y, 0.0f, 1.0f}
		}};
		dsMatrix44f iconModelViewProjection;
		dsMatrix44f_mul(&iconModelViewProjection, modelViewProjection, &boundsMatrix);

		if (!dsVectorImage_draw(image, commandBuffer, vectorIcons->shaders, material,
				&iconModelViewProjection, globalValues, renderStates))
		{
			return false;
		}
	}

	return true;
}

dsTextIcons* dsVectorTextIcons_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsVectorShaders* shaders,
	const dsIndexRange* codepointRanges, uint32_t codepointRangeCount, uint32_t maxIcons)
{
	if (!allocator || !resourceManager || !shaders)
	{
		errno = EINVAL;
		return NULL;
	}

	VectorIcons* vectorIcons = DS_ALLOCATE_OBJECT(allocator, VectorIcons);
	if (!vectorIcons)
		return NULL;

	vectorIcons->allocator = allocator;
	vectorIcons->resourceManager = resourceManager;
	vectorIcons->shaders = shaders;
	vectorIcons->materialStorage = dsThreadObjectStorage_create(
		allocator, (dsDestroyUserDataFunction)&dsMaterial_destroy);
	if (!vectorIcons->materialStorage)
	{
		VectorIcons_destroy(vectorIcons);
		return NULL;
	}

	return dsTextIcons_create(allocator, codepointRanges, codepointRangeCount, maxIcons,
		vectorIcons, &VectorIcons_destroy, &dsVectorTextIcons_prepare, &dsVectorTextIcons_draw,
		&dsTextureTextIcons_destroyImage);
}

bool dsVectorTextIcons_addIcon(dsTextIcons* icons, uint32_t codepoint, float advance,
	const dsAlignedBox2f* bounds, dsVectorImage* image, bool takeOwnership)
{
	if (!icons || !bounds || !image)
	{
		if (takeOwnership)
			dsVectorImage_destroy(image);
		errno = EINVAL;
		return false;
	}

	// Expect that the least significant bit of the image pointer is always zero.
	DS_ASSERT(((size_t)image& 0x1) == 0);
	return dsTextIcons_addIcon(icons, codepoint, advance, bounds,
		ENCODE_USER_DATA(image, takeOwnership));
}

bool dsVectorTextIcons_replaceIcon(
	dsTextIcons* icons, uint32_t codepoint, dsVectorImage* image, bool takeOwnership)
{
	if (!icons || !image)
	{
		if (takeOwnership)
			dsVectorImage_destroy(image);
		errno = EINVAL;
		return false;
	}

	// Expect that the least significant bit of the image pointer is always zero.
	DS_ASSERT(((size_t)image & 0x1) == 0);
	return dsTextIcons_replaceIcon(icons, codepoint, ENCODE_USER_DATA(image, takeOwnership));
}

dsVectorImage* dsVectorTextIcons_getIconImage(const dsIconGlyph* icon)
{
	if (!icon)
	{
		errno = EINVAL;
		return NULL;
	}

	return EXTRACT_IMAGE(icon->userData);
}

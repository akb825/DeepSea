/*
 * Copyright 2025 Aaron Barany
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

#include <DeepSea/Text/TextIcons.h>

#include "TextInternal.h"

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Geometry/AlignedBox2.h>

size_t dsTextIcons_sizeof(void)
{
	return sizeof(dsTextIcons);
}

size_t dsTextIcons_fullAllocSize(uint32_t maxIcons)
{
	size_t tableSize = dsHashTable_tableSize(maxIcons);
	return DS_ALIGNED_SIZE(sizeof(dsTextIcons)) + DS_ALIGNED_SIZE(sizeof(dsIconGlyph)*maxIcons) +
		DS_ALIGNED_SIZE(sizeof(dsIconGlyphNode)*maxIcons) + dsHashTable_fullAllocSize(tableSize);
}

dsTextIcons* dsTextIcons_create(dsAllocator* allocator, uint32_t maxIcons, void* userData,
	dsDestroyUserDataFunction destroyUserDataFunc, dsPrepareDrawTextIconsFunction prepareFunc,
	dsPrepareDrawTextIconsFunction drawFunc, dsDestroyUserDataFunction destroyGlyphUserDataFunc)
{
	if (!allocator || maxIcons == 0 || !drawFunc)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = dsTextIcons_fullAllocSize(maxIcons);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		if (destroyUserDataFunc)
			destroyUserDataFunc(userData);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_ASSERT(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsTextIcons* icons = DS_ALLOCATE_OBJECT(&bufferAlloc, dsTextIcons);
	DS_ASSERT(icons);

	icons->allocator = dsAllocator_keepPointer(allocator);
	icons->userData = userData;
	icons->destroyUserDataFunc = destroyUserDataFunc;
	icons->prepareFunc = prepareFunc;
	icons->drawFunc = drawFunc;
	icons->destroyGlyphUserDataFunc = destroyGlyphUserDataFunc;

	icons->iconGlyphs = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsIconGlyph, maxIcons);
	DS_ASSERT(icons->iconGlyphs);
	icons->iconNodes = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsIconGlyphNode, maxIcons);
	DS_ASSERT(icons->iconNodes);

	icons->iconCount = 0;
	icons->maxIcons = maxIcons;

	size_t tableSize = dsHashTable_tableSize(maxIcons);
	size_t hashTableSize = dsHashTable_fullAllocSize(tableSize);
	icons->iconTable = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, hashTableSize);
	DS_ASSERT(icons->iconTable);
	DS_VERIFY(dsHashTable_initialize(icons->iconTable, tableSize, &dsHash32, &dsHash32Equal));

	return icons;
}

dsAllocator* dsTextIcons_getAllocator(const dsTextIcons* icons)
{
	if (!icons)
	{
		errno = EINVAL;
		return NULL;
	}

	return icons->allocator;
}

bool dsTextIcons_addIcon(
	dsTextIcons* icons, uint32_t charCode, const dsAlignedBox2f* bounds, void* userData)
{
	if (!icons || !bounds || !dsAlignedBox2_isValid(*bounds))
	{
		if (icons && icons->destroyUserDataFunc)
			icons->destroyGlyphUserDataFunc(userData);
		errno = EINVAL;
		return false;
	}

	uint32_t index = icons->iconCount;
	if (index >= icons->maxIcons)
	{
		if (icons && icons->destroyUserDataFunc)
			icons->destroyGlyphUserDataFunc(userData);
		errno = ENOMEM;
		return false;
	}

	dsIconGlyphNode* node = icons->iconNodes + index;
	node->charCode = charCode;
	node->index = index;
	if (!dsHashTable_insert(icons->iconTable, &node->charCode, (dsHashTableNode*)node, NULL))
	{
		if (icons && icons->destroyUserDataFunc)
			icons->destroyGlyphUserDataFunc(userData);
		errno = EPERM;
		return false;
	}

	dsIconGlyph* iconGlyph = icons->iconGlyphs + index;
	iconGlyph->charCode = charCode;
	iconGlyph->bounds = *bounds;
	iconGlyph->userData = userData;

	++icons->iconCount;
	return true;
}

const dsIconGlyph* dsTextIcons_findIcon(const dsTextIcons* icons, uint32_t charCode)
{
	if (!icons)
	{
		errno = EINVAL;
		return NULL;
	}

	dsIconGlyphNode* foundIcon = (dsIconGlyphNode*)dsHashTable_find(icons->iconTable, &charCode);
	if (!foundIcon)
	{
		errno = ENOTFOUND;
		return NULL;
	}

	return icons->iconGlyphs + foundIcon->index;
}

void dsTextIcons_destroy(dsTextIcons* icons)
{
	if (!icons)
		return;

	if (icons->destroyGlyphUserDataFunc)
	{
		for (uint32_t i = 0; i < icons->iconCount; ++i)
			icons->destroyGlyphUserDataFunc(icons->iconGlyphs[i].userData);
	}

	if (icons->destroyUserDataFunc)
		icons->destroyUserDataFunc(icons->userData);

	if (icons->allocator)
		DS_VERIFY(dsAllocator_free(icons->allocator, icons));
}

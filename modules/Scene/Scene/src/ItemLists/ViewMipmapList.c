/*
 * Copyright 2021-2025 Aaron Barany
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

#include <DeepSea/Scene/ItemLists/ViewMipmapList.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/Resources/Texture.h>

#include <DeepSea/Scene/Types.h>

#include <string.h>

typedef struct dsViewMipmapList
{
	dsSceneItemList itemList;
	uint32_t* textureIDs;
	uint32_t textureCount;
} dsViewMipmapList;

static void dsViewMipmapList_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_ASSERT(itemList);
	dsViewMipmapList* mipmapList = (dsViewMipmapList*)itemList;
	for (uint32_t i = 0; i < mipmapList->textureCount; ++i)
	{
		dsTexture* texture = dsSharedMaterialValues_getTextureID(view->globalValues,
			mipmapList->textureIDs[i]);
		if (texture)
			DS_CHECK(DS_SCENE_LOG_TAG, dsTexture_generateMipmaps(texture, commandBuffer));
	}
}

static void dsViewMipmapList_destroy(dsSceneItemList* itemList)
{
	DS_ASSERT(itemList);
	if (itemList->allocator)
		DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

const char* const dsViewMipmapList_typeName = "ViewMipmapList";

static dsSceneItemListType itemListType =
{
	.commitFunc = &dsViewMipmapList_commit,
	.destroyFunc = &dsViewMipmapList_destroy
};

const dsSceneItemListType* dsViewMipmapList_type(void)
{
	return &itemListType;
}

dsSceneItemList* dsViewMipmapList_create(dsAllocator* allocator, const char* name,
	const char* const* textureNames, uint32_t textureCount)
{
	if (!allocator || !name || !textureNames || textureCount == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	for (uint32_t i = 0; i < textureCount; ++i)
	{
		if (!textureNames[i])
		{
			errno = EINVAL;
			return NULL;
		}
	}

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsViewMipmapList)) + DS_ALIGNED_SIZE(nameLen) +
		DS_ALIGNED_SIZE(textureCount*sizeof(uint32_t));
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsViewMipmapList* mipmapList = DS_ALLOCATE_OBJECT(&bufferAlloc, dsViewMipmapList);
	DS_ASSERT(mipmapList);

	dsSceneItemList* itemList = (dsSceneItemList*)mipmapList;
	itemList->allocator = dsAllocator_keepPointer(allocator);
	itemList->type = dsViewMipmapList_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsUniqueNameID_create(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = true;
	itemList->skipPreRenderPass = false;

	mipmapList->textureIDs = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, uint32_t, textureCount);
	DS_ASSERT(mipmapList->textureIDs);
	for (uint32_t i = 0; i < textureCount; ++i)
		mipmapList->textureIDs[i] = dsUniqueNameID_create(textureNames[i]);
	mipmapList->textureCount = textureCount;

	return itemList;
}

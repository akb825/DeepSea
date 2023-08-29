/*
 * Copyright 2020-2023 Aaron Barany
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

#include <DeepSea/SceneVectorDraw/SceneVectorDrawPrepare.h>

#include "SceneVectorDrawPrepareLoad.h"

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Profile.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <DeepSea/SceneVectorDraw/SceneTextNode.h>
#include <DeepSea/SceneVectorDraw/SceneVectorImageNode.h>
#include <DeepSea/SceneVectorDraw/SceneVectorNode.h>

#include <DeepSea/Text/TextLayout.h>
#include <DeepSea/Text/TextRenderBuffer.h>

#include <DeepSea/VectorDraw/VectorImage.h>

#include <string.h>

typedef struct Entry
{
	dsSceneTextNode* textNode;
	dsSceneVectorImageNode* imageNode;
	uint32_t layoutVersion;
	uint64_t nodeID;
} Entry;

typedef struct dsSceneVectorDrawPrepare
{
	dsSceneItemList itemList;

	Entry* entries;
	uint32_t entryCount;
	uint32_t maxEntries;
	uint64_t nextNodeID;
} dsSceneVectorDrawPrepare;

static uint64_t dsSceneVectorDrawPrepare_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(itemData);
	DS_UNUSED(treeNode);
	DS_UNUSED(thisItemData);
	if (!dsSceneNode_isOfType(node, dsSceneVectorNode_type()))
		return DS_NO_SCENE_NODE;

	dsSceneVectorDrawPrepare* prepareList = (dsSceneVectorDrawPrepare*)itemList;

	uint32_t index = prepareList->entryCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, prepareList->entries, prepareList->entryCount,
			prepareList->maxEntries, 1))
	{
		return DS_NO_SCENE_NODE;
	}

	Entry* entry = prepareList->entries + index;
	if (dsSceneNode_isOfType(node, dsSceneTextNode_type()))
	{
		entry->textNode = (dsSceneTextNode*)node;
		entry->imageNode = NULL;
		// Force a re-layout the first time.
		entry->layoutVersion = entry->textNode->layoutVersion - 1;
	}
	else
	{
		DS_ASSERT(dsSceneNode_isOfType(node, dsSceneVectorImageNode_type()));
		entry->textNode = NULL;
		entry->imageNode = (dsSceneVectorImageNode*)node;
		entry->layoutVersion = 0;
	}
	entry->nodeID = prepareList->nextNodeID++;
	return entry->nodeID;
}

static void dsSceneVectorDrawPrepare_removeNode(dsSceneItemList* itemList, uint64_t nodeID)
{
	dsSceneVectorDrawPrepare* prepareList = (dsSceneVectorDrawPrepare*)itemList;
	for (uint32_t i = 0; i < prepareList->entryCount; ++i)
	{
		if (prepareList->entries[i].nodeID != nodeID)
			continue;

		// Order shouldn't matter, so use constant-time removal.
		prepareList->entries[i] = prepareList->entries[prepareList->entryCount - 1];
		--prepareList->entryCount;
		break;
	}
}

static void dsSceneVectorDrawPrepare_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(view);
	DS_PROFILE_DYNAMIC_SCOPE_START(itemList->name);

	dsSceneVectorDrawPrepare* prepareList = (dsSceneVectorDrawPrepare*)itemList;
	for (uint32_t i = 0; i < prepareList->entryCount; ++i)
	{
		Entry* entry = prepareList->entries + i;
		if (entry->textNode)
		{
			dsSceneTextNode* node = entry->textNode;
			if (entry->layoutVersion == entry->textNode->layoutVersion)
			{
				DS_CHECK(DS_SCENE_VECTOR_DRAW_LOG_TAG,
					dsTextLayout_refresh(node->layout, commandBuffer));
			}
			else
			{
				DS_CHECK(DS_SCENE_VECTOR_DRAW_LOG_TAG,
					dsTextLayout_layout(node->layout, commandBuffer, node->alignment,
						node->maxWidth, node->lineScale));
				DS_VERIFY(dsTextRenderBuffer_clear(node->renderBuffer));
				DS_CHECK(DS_SCENE_VECTOR_DRAW_LOG_TAG,
					dsTextRenderBuffer_addText(node->renderBuffer, node->layout,
						node->textUserData));
				DS_CHECK(DS_SCENE_VECTOR_DRAW_LOG_TAG,
					dsTextRenderBuffer_commit(node->renderBuffer, commandBuffer));
				entry->layoutVersion = node->layoutVersion;
			}
		}
		else
		{
			DS_ASSERT(entry->imageNode);
			dsVectorImage_updateText(entry->imageNode->vectorImage, commandBuffer);
		}
	}

	DS_PROFILE_SCOPE_END();
}

static void dsSceneVectorDrawPrepare_destroy(dsSceneItemList* itemList)
{
	dsSceneVectorDrawPrepare* prepareList = (dsSceneVectorDrawPrepare*)itemList;
	DS_VERIFY(dsAllocator_free(itemList->allocator, prepareList->entries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

dsSceneItemList* dsSceneVectorDrawPrepare_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const char* name, const uint8_t* data, size_t dataSize)
{
	DS_UNUSED(loadContext);
	DS_UNUSED(scratchData);
	DS_UNUSED(resourceAllocator);
	DS_UNUSED(userData);
	DS_UNUSED(data);
	DS_UNUSED(dataSize);
	return dsSceneVectorDrawPrepare_create(allocator, name);
}

const char* const dsSceneVectorDrawPrepare_typeName = "VectorDrawPrepare";

dsSceneItemListType dsSceneVectorDrawPrepare_type(void)
{
	static int type;
	return &type;
}

dsSceneItemList* dsSceneVectorDrawPrepare_create(dsAllocator* allocator, const char* name)
{
	if (!allocator || !name)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_VECTOR_DRAW_LOG_TAG,
			"Vector prepare list allocator must support freeing memory.");
		return NULL;
	}

	size_t nameLen = strlen(name) + 1;
	size_t fullSize =
		DS_ALIGNED_SIZE(sizeof(dsSceneVectorDrawPrepare)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsSceneVectorDrawPrepare* prepareList =
		DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneVectorDrawPrepare);
	DS_ASSERT(prepareList);

	dsSceneItemList* itemList = (dsSceneItemList*)prepareList;
	itemList->allocator = allocator;
	itemList->type = dsSceneVectorDrawPrepare_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsHashString(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = true;
	itemList->addNodeFunc = &dsSceneVectorDrawPrepare_addNode;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = &dsSceneVectorDrawPrepare_removeNode;
	itemList->preTransformUpdateFunc = NULL;
	itemList->updateFunc = NULL;
	itemList->preRenderPassFunc = NULL;
	itemList->commitFunc = &dsSceneVectorDrawPrepare_commit;
	itemList->destroyFunc = &dsSceneVectorDrawPrepare_destroy;

	prepareList->entries = NULL;
	prepareList->entryCount = 0;
	prepareList->maxEntries = 0;
	prepareList->nextNodeID = 0;

	return itemList;
}

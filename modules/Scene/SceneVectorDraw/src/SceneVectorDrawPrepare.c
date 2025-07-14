/*
 * Copyright 2020-2025 Aaron Barany
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

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Scene/ItemLists/SceneItemListEntries.h>
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

	uint64_t* removeEntries;
	uint32_t removeEntryCount;
	uint32_t maxRemoveEntries;
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

static void dsSceneVectorDrawPrepare_removeNode(dsSceneItemList* itemList,
	dsSceneTreeNode* treeNode, uint64_t nodeID)
{
	DS_UNUSED(treeNode);
	dsSceneVectorDrawPrepare* prepareList = (dsSceneVectorDrawPrepare*)itemList;

	uint32_t index = prepareList->removeEntryCount;
	if (DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, prepareList->removeEntries,
			prepareList->removeEntryCount, prepareList->maxRemoveEntries, 1))
	{
		prepareList->removeEntries[index] = nodeID;
	}
	else
	{
		dsSceneItemListEntries_removeSingle(prepareList->entries, &prepareList->entryCount,
			sizeof(Entry), offsetof(Entry, nodeID), nodeID);
	}
}

static void dsSceneVectorDrawPrepare_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	DS_UNUSED(view);
	dsSceneVectorDrawPrepare* prepareList = (dsSceneVectorDrawPrepare*)itemList;

	// Lazily remove entries.
	dsSceneItemListEntries_removeMulti(prepareList->entries, &prepareList->entryCount,
		sizeof(Entry), offsetof(Entry, nodeID), prepareList->removeEntries,
		prepareList->removeEntryCount);
	prepareList->removeEntryCount = 0;

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
}

static void dsSceneVectorDrawPrepare_destroy(dsSceneItemList* itemList)
{
	dsSceneVectorDrawPrepare* prepareList = (dsSceneVectorDrawPrepare*)itemList;
	DS_VERIFY(dsAllocator_free(itemList->allocator, prepareList->entries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, prepareList->removeEntries));
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

const dsSceneItemListType* dsSceneVectorDrawPrepare_type(void)
{
	static dsSceneItemListType type =
	{
		.addNodeFunc = &dsSceneVectorDrawPrepare_addNode,
		.removeNodeFunc = &dsSceneVectorDrawPrepare_removeNode,
		.commitFunc = &dsSceneVectorDrawPrepare_commit,
		.destroyFunc = &dsSceneVectorDrawPrepare_destroy
	};
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
	itemList->nameID = dsUniqueNameID_create(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = true;
	itemList->skipPreRenderPass = false;

	prepareList->entries = NULL;
	prepareList->entryCount = 0;
	prepareList->maxEntries = 0;
	prepareList->nextNodeID = 0;

	return itemList;
}

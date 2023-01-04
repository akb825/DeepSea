/*
 * Copyright 2022-2023 Aaron Barany
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

#include <DeepSea/SceneLighting/SceneLightNode.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <DeepSea/SceneLighting/SceneLightSetPrepare.h>

#include <string.h>

struct dsSceneLightNode
{
	dsSceneNode node;
	dsSceneLight templateLight;
	const char* lightBaseName;
	bool singleInstance;
};

static void dsSceneLightNode_destroy(dsSceneNode* node)
{
	if (node->allocator)
		DS_VERIFY(dsAllocator_free(node->allocator, node));
}

const char* const dsSceneLightNode_typeName = "LightNode";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsSceneLightNode_type(void)
{
	return &nodeType;
}

dsSceneLightNode* dsSceneLightNode_create(dsAllocator* allocator, const dsSceneLight* templateLight,
	const char* lightBaseName, bool singleInstance, const char* const* itemLists,
	uint32_t itemListCount)
{
	if (!allocator || !templateLight || !lightBaseName || (!itemLists && itemListCount > 0))
	{
		errno = EINVAL;
		return NULL;
	}

	size_t itemListsSize = dsSceneNode_itemListsAllocSize(itemLists, itemListCount);
	if (itemListsSize == 0)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t lightBaseNameLen = strlen(lightBaseName) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneLightNode)) + itemListsSize +
		DS_ALIGNED_SIZE(lightBaseNameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneLightNode* lightNode = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneLightNode);
	DS_ASSERT(lightNode);

	const char* const* itemListsCopy = dsSceneNode_copyItemLists(&bufferAlloc, itemLists,
		itemListCount);
	DS_ASSERT(itemListCount == 0 || itemListsCopy);

	if (!dsSceneNode_initialize((dsSceneNode*)lightNode, allocator, dsSceneLightNode_type(),
			itemListsCopy, itemListCount, &dsSceneLightNode_destroy))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, buffer));
		return NULL;
	}

	lightNode->templateLight = *templateLight;
	lightNode->templateLight.nameID = 0;

	char* lightBaseNameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, lightBaseNameLen);
	DS_ASSERT(lightBaseNameCopy);
	memcpy(lightBaseNameCopy, lightBaseName, lightBaseNameLen);
	lightNode->lightBaseName = lightBaseNameCopy;
	lightNode->singleInstance = singleInstance;

	return lightNode;
}

const dsSceneLight* dsSceneLightNode_getTemplateLight(const dsSceneLightNode* lightNode)
{
	if (!lightNode)
	{
		errno = EINVAL;
		return NULL;
	}

	return &lightNode->templateLight;
}

dsSceneLight* dsSceneLightNode_getMutableTemplateLight(dsSceneLightNode* lightNode)
{
	if (!lightNode)
	{
		errno = EINVAL;
		return NULL;
	}

	return &lightNode->templateLight;
}

const char* dsSceneLightNode_getLightBaseName(const dsSceneLightNode* lightNode)
{
	if (!lightNode)
	{
		errno = EINVAL;
		return NULL;
	}

	return lightNode->lightBaseName;
}

bool dsSceneLightNode_getSingleInstance(const dsSceneLightNode* lightNode)
{
	return lightNode ? lightNode->singleInstance : false;
}

dsSceneLight* dsSceneLightNode_getLightForInstance(const dsSceneTreeNode* treeNode)
{
	if (!treeNode)
		return NULL;

	const dsSceneNodeItemData* itemData = &treeNode->itemData;
	DS_ASSERT(itemData->count == treeNode->node->itemListCount);
	for (uint32_t i = 0; i < itemData->count; ++i)
	{
		const dsSceneItemList* itemList = treeNode->itemLists[i].list;
		if (itemList && itemList->type == dsSceneLightSetPrepare_type())
			return (dsSceneLight*)itemData->itemData[i].data;
	}

	return NULL;
}

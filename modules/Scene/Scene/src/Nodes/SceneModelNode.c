/*
 * Copyright 2019 Aaron Barany
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

#include <DeepSea/Scene/Nodes/SceneModelNode.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Geometry/OrientedBox3.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/SceneResources.h>

#include <string.h>

#define EXPECTED_MAX_NODES 256

static size_t fullAllocSize(size_t structSize, const char** drawLists, uint32_t drawListCount,
	const dsSceneModelInitInfo* models, uint32_t modelCount, uint32_t resourceCount)
{
	size_t fullSize = DS_ALIGNED_SIZE(structSize) +
		dsSceneNode_itemListsAllocSize(drawLists, drawListCount) +
		DS_ALIGNED_SIZE(sizeof(dsSceneModelInfo)*modelCount) +
		DS_ALIGNED_SIZE(sizeof(dsSceneResources*)*resourceCount);
	for (uint32_t i = 0; i < modelCount; ++i)
	{
		const char* name = models[i].name;
		if (name)
			fullSize += DS_ALIGNED_SIZE(strlen(name) + 1);
	}

	return fullSize;
}

static size_t cloneFullAllocSize(size_t structSize, const dsSceneModelNode* model)
{
	const dsSceneNode* node = (const dsSceneNode*)model;
	size_t fullSize = DS_ALIGNED_SIZE(structSize) +
		dsSceneNode_itemListsAllocSize(node->itemLists, node->itemListCount) +
		DS_ALIGNED_SIZE(sizeof(dsSceneModelInfo)*model->modelCount) +
		DS_ALIGNED_SIZE(sizeof(dsSceneResources*)*model->resourceCount);
	for (uint32_t i = 0; i < model->modelCount; ++i)
	{
		const char* name = model->models[i].name;
		if (name)
			fullSize += DS_ALIGNED_SIZE(strlen(name) + 1);
	}

	return fullSize;
}

static void populateItemList(const char** itemLists, uint32_t* hashes, uint32_t* itemListCount,
	const dsSceneModelInitInfo* models, uint32_t modelCount, const char** extraItemLists,
	uint32_t extraItemListCount)
{
	// Assume uniqueness for the extra lists. Add extra items first since the data will be more
	// likely to be looked up, making the linear search faster.
	for (uint32_t i = 0; i < extraItemListCount; ++i)
		itemLists[i] = extraItemLists[i];

	for (uint32_t i = 0; i < modelCount; ++i)
		hashes[i] = dsHashString(models[i].listName);

	uint32_t start = extraItemListCount;
	for (uint32_t i = 0; i < modelCount; ++i)
	{
		bool unique = true;
		for (uint32_t j = 0; j < *itemListCount; ++j)
		{
			if (hashes[i] == hashes[j])
			{
				unique = false;
				break;
			}
		}

		if (!unique)
			continue;

		uint32_t index = (*itemListCount)++;
		itemLists[start + index] = models[i].listName;
		// Also make sure the assigned hashes match for faster uniqueness check.
		hashes[index] = hashes[i];
	}

	*itemListCount += extraItemListCount;
}

const char* const dsSceneModelNode_typeName = "ModelNode";
const char* const dsSceneModelNode_cloneTypeName = "ModelNodeClone";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsSceneModelNode_type(void)
{
	return &nodeType;
}

dsSceneModelNode* dsSceneModelNode_create(dsAllocator* allocator,
	const dsSceneModelInitInfo* models, uint32_t modelCount, const char** extraItemLists,
	uint32_t extraItemListCount, dsSceneResources** resources, uint32_t resourceCount,
	const dsOrientedBox3f* bounds)
{
	return dsSceneModelNode_createBase(allocator, sizeof(dsSceneModelNode), models, modelCount,
		extraItemLists, extraItemListCount, resources, resourceCount, bounds);
}

dsSceneModelNode* dsSceneModelNode_createBase(dsAllocator* allocator, size_t structSize,
	const dsSceneModelInitInfo* models, uint32_t modelCount, const char** extraItemLists,
	uint32_t extraItemListCount, dsSceneResources** resources, uint32_t resourceCount,
	const dsOrientedBox3f* bounds)
{
	if (!allocator || structSize < sizeof(dsSceneModelNode) || !models || modelCount == 0 ||
		(!extraItemLists && extraItemListCount > 0) || (!resources && resourceCount > 0))
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Scene node allocator must support freeing memory.");
		return false;
	}

	for (uint32_t i = 0; i < modelCount; ++i)
	{
		const dsSceneModelInitInfo* model = models + i;
		if (!model->shader || !model->material || !model->geometry || !model->listName)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_SCENE_LOG_TAG, "All scene models must have a valid shader, material, "
				"geometry, and draw list name.");
			return NULL;
		}

		if (model->shader->materialDesc != dsMaterial_getDescription(model->material))
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Shader and material are incompatible.");
			return NULL;
		}
	}

	for (uint32_t i = 0; i < resourceCount; ++i)
	{
		if (!resources[i])
		{
			errno = EINVAL;
			return NULL;
		}
	}

	// Get the draw lists from the model nodes.
	const char* tempStringListData[EXPECTED_MAX_NODES];
	uint32_t tempStringHashListData[EXPECTED_MAX_NODES];
	const char** itemLists = tempStringListData;
	uint32_t itemListCount = 0;
	uint32_t* tempStringHashList = tempStringHashListData;
	uint32_t maxItemListCount = modelCount + extraItemListCount;
	if (maxItemListCount > EXPECTED_MAX_NODES)
	{
		// Need to dynamically allocate temp lists if too large.
		itemLists = DS_ALLOCATE_OBJECT_ARRAY(allocator, const char*, maxItemListCount);
		if (!itemLists)
			return NULL;
		if (modelCount > EXPECTED_MAX_NODES)
		{
			tempStringHashList = DS_ALLOCATE_OBJECT_ARRAY(allocator, uint32_t, modelCount);
			if (!tempStringHashList)
			{
				DS_VERIFY(dsAllocator_free(allocator, (void*)itemLists));
				return NULL;
			}
		}
	}

	populateItemList(itemLists, tempStringHashList, &itemListCount, models, modelCount,
		extraItemLists, extraItemListCount);
	if (tempStringHashList != tempStringHashListData)
		DS_VERIFY(dsAllocator_free(allocator, tempStringHashList));

	size_t fullSize = fullAllocSize(structSize, itemLists, itemListCount, models, modelCount,
		resourceCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		if (itemLists != tempStringListData)
			DS_VERIFY(dsAllocator_free(allocator, (void*)itemLists));
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneModelNode* node =
		(dsSceneModelNode*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, structSize);
	DS_ASSERT(node);

	char** itemListsCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char*, itemListCount);
	DS_ASSERT(itemListsCopy);
	for (uint32_t i = 0; i < itemListCount; ++i)
	{
		size_t length = strlen(itemLists[i]);
		itemListsCopy[i] = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, length + 1);
		memcpy(itemListsCopy[i], itemLists[i], length + 1);
	}

	if (itemLists != tempStringListData)
		DS_VERIFY(dsAllocator_free(allocator, (void*)itemLists));

	if (!dsSceneNode_initialize((dsSceneNode*)node, allocator, dsSceneModelNode_type(),
			(const char**)itemListsCopy, itemListCount, &dsSceneModelNode_destroy))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		return NULL;
	}

	node->models = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneModelInfo, modelCount);
	DS_ASSERT(node->models);
	for (uint32_t i = 0; i < modelCount; ++i)
	{
		const dsSceneModelInitInfo* initInfo = models + i;
		dsSceneModelInfo* model = node->models + i;
		if (initInfo->name)
		{
			size_t nameLen = strlen(initInfo->name) + 1;
			char* name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
			memcpy(name, initInfo->name, nameLen);
			model->name = name;
		}
		else
			model->name = NULL;
		model->shader = initInfo->shader;
		model->material = initInfo->material;
		model->geometry = initInfo->geometry;
		model->distanceRange = initInfo->distanceRange;
		if (model->geometry->indexBuffer.buffer)
			model->drawIndexedRange = initInfo->drawIndexedRange;
		else
			model->drawRange = initInfo->drawRange;
		model->primitiveType = initInfo->primitiveType;
		model->listNameID = dsHashString(initInfo->listName);
	}
	node->modelCount = modelCount;

	if (resourceCount > 0)
	{
		node->resources = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneResources*, resourceCount);
		DS_ASSERT(node->resources);
		for (uint32_t i = 0; i < resourceCount; ++i)
			node->resources[i] = dsSceneResources_addRef(resources[i]);
		node->resourceCount = resourceCount;
	}
	else
	{
		node->resources = NULL;
		node->resourceCount = 0;
	}

	if (bounds)
		node->bounds = *bounds;
	else
		dsOrientedBox3_makeInvalid(node->bounds);

	return node;
}

dsSceneModelNode* dsSceneModelNode_clone(dsAllocator* allocator, const dsSceneModelNode* origModel,
	const dsSceneMaterialRemap* remaps, uint32_t remapCount)
{
	return dsSceneModelNode_cloneBase(allocator, sizeof(dsSceneModelNode), origModel, remaps,
		remapCount);
}

dsSceneModelNode* dsSceneModelNode_cloneBase(dsAllocator* allocator, size_t structSize,
	const dsSceneModelNode* origModel, const dsSceneMaterialRemap* remaps, uint32_t remapCount)
{
	if (!allocator || structSize < sizeof(dsSceneModelNode) || !origModel ||
		(!remaps && remapCount > 0))
	{
		errno = EINVAL;
		return NULL;
	}

	for (uint32_t i = 0; i < remapCount; ++i)
	{
		const dsSceneMaterialRemap* remap = remaps + i;
		if (!remap->name)
		{
			errno = EINVAL;
			return NULL;
		}
	}

	size_t fullSize = cloneFullAllocSize(structSize, origModel);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	const dsSceneNode* origNode = (const dsSceneNode*)origModel;
	dsSceneModelNode* node =
		(dsSceneModelNode*)dsAllocator_alloc((dsAllocator*)&bufferAlloc, structSize);
	DS_ASSERT(node);

	uint32_t itemListCount = origNode->itemListCount;
	char** itemListsCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char*, itemListCount);
	DS_ASSERT(itemListsCopy);
	for (uint32_t i = 0; i < itemListCount; ++i)
	{
		const char* origList = origNode->itemLists[i];
		size_t length = strlen(origList);
		itemListsCopy[i] = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, length + 1);
		memcpy(itemListsCopy[i], origList, length + 1);
	}

	if (!dsSceneNode_initialize((dsSceneNode*)node, allocator, dsSceneModelNode_type(),
			(const char**)itemListsCopy, itemListCount, &dsSceneModelNode_destroy))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		return NULL;
	}

	node->models = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneModelInfo, origModel->modelCount);
	DS_ASSERT(node->models);
	for (uint32_t i = 0; i < origModel->modelCount; ++i)
	{
		dsSceneModelInfo* model = node->models + i;
		*model = origModel->models[i];
		if (model->name)
		{
			size_t nameLen = strlen(model->name) + 1;
			char* name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
			memcpy(name, model->name, nameLen);
			model->name = name;
		}
		else
			model->name = NULL;
	}
	node->modelCount = origModel->modelCount;

	if (origModel->resourceCount > 0)
	{
		node->resources = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneResources*,
			origModel->resourceCount);
		DS_ASSERT(node->resources);
		for (uint32_t i = 0; i <origModel-> resourceCount; ++i)
			node->resources[i] = dsSceneResources_addRef(origModel->resources[i]);
		node->resourceCount = origModel->resourceCount;
	}
	else
	{
		node->resources = NULL;
		node->resourceCount = 0;
	}

	node->bounds = origModel->bounds;
	DS_VERIFY(dsSceneModelNode_remapMaterials(node, remaps, remapCount));
	return node;
}

bool dsSceneModelNode_remapMaterials(dsSceneModelNode* node, const dsSceneMaterialRemap* remaps,
	uint32_t remapCount)
{
	if (!node || (!remaps && remapCount > 0))
	{
		errno = EINVAL;
		return false;
	}

	for (uint32_t i = 0; i < remapCount; ++i)
	{
		const dsSceneMaterialRemap* remap = remaps + i;
		if (!remap->name)
		{
			errno = EINVAL;
			return false;
		}

		for (uint32_t j = 0; j < node->modelCount; ++j)
		{
			dsSceneModelInfo* model = node->models + i;
			if (!model->name || strcmp(model->name, remap->name) != 0)
				continue;

			if (remap->shader)
				model->shader = remap->shader;
			if (remap->material)
				model->material = remap->material;
		}
	}

	return true;
}

void dsSceneModelNode_destroy(dsSceneNode* node)
{
	dsSceneModelNode* modelNode = (dsSceneModelNode*)node;
	for (uint32_t i = 0; i < modelNode->resourceCount; ++i)
		dsSceneResources_freeRef(modelNode->resources[i]);
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

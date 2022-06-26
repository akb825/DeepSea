/*
 * Copyright 2019-2022 Aaron Barany
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

#include <DeepSea/Scene/Nodes/SceneCullNode.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneTreeNode.h>
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
	uint32_t drawRangeCount = 0;
	for (uint32_t i = 0; i < modelCount; ++i)
	{
		const char* name = models[i].name;
		if (name)
			fullSize += DS_ALIGNED_SIZE(strlen(name) + 1);
		drawRangeCount += models[i].drawRangeCount;
	}

	return fullSize + DS_ALIGNED_SIZE(sizeof(dsSceneModelDrawRange)*drawRangeCount);
}

static size_t cloneRemapFullAllocSize(size_t structSize, const dsSceneModelNode* model)
{
	const dsSceneNode* node = (const dsSceneNode*)model;
	size_t fullSize = DS_ALIGNED_SIZE(structSize) +
		dsSceneNode_itemListsAllocSize(node->itemLists, node->itemListCount) +
		DS_ALIGNED_SIZE(sizeof(dsSceneModelInfo)*model->modelCount) +
		DS_ALIGNED_SIZE(sizeof(dsSceneResources*)*model->resourceCount);
	uint32_t drawRangeCount = 0;
	for (uint32_t i = 0; i < model->modelCount; ++i)
	{
		const char* name = model->models[i].name;
		if (name)
			fullSize += DS_ALIGNED_SIZE(strlen(name) + 1);
		drawRangeCount += model->models[i].drawRangeCount;
	}

	return fullSize + DS_ALIGNED_SIZE(sizeof(dsSceneModelDrawRange)*drawRangeCount);
}

static void populateItemList(const char** itemLists, uint32_t* hashes, uint32_t* itemListCount,
	const dsSceneModelInitInfo* models, uint32_t modelCount, const char* const* extraItemLists,
	uint32_t extraItemListCount)
{
	// Assume uniqueness for the extra lists. Add extra items first since the data will be more
	// likely to be looked up, making the linear search faster.
	for (uint32_t i = 0; i < extraItemListCount; ++i)
		itemLists[i] = extraItemLists[i];

	for (uint32_t i = 0; i < modelCount; ++i)
		hashes[i] = dsHashString(models[i].modelList);

	uint32_t start = extraItemListCount;
	for (uint32_t i = 0; i < modelCount; ++i)
	{
		if (!models[i].modelList)
			continue;

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
		itemLists[start + index] = models[i].modelList;
		// Also make sure the assigned hashes match for faster uniqueness check.
		hashes[index] = hashes[i];
	}

	*itemListCount += extraItemListCount;
}

static bool dsModelNode_getBounds(dsOrientedBox3f* outBounds, const dsSceneCullNode* node,
	const dsSceneTreeNode* treeNode)
{
	const dsSceneModelNode* modelNode = (const dsSceneModelNode*)node;
	*outBounds = modelNode->bounds;
	const dsMatrix44f* transform = dsSceneTreeNode_getTransform(treeNode);
	DS_ASSERT(transform);
	dsOrientedBox3f_transform(outBounds, transform);
	return true;
}

const char* const dsSceneModelNode_typeName = "ModelNode";
const char* const dsSceneModelNode_remapTypeName = "ModelNodeRemap";
const char* const dsSceneModelNode_reconfigTypeName = "ModelNodeReconfig";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsSceneModelNode_type(void)
{
	return &nodeType;
}

const dsSceneNodeType* dsSceneModelNode_setupParentType(dsSceneNodeType* type)
{
	dsSceneNode_setupParentType(&nodeType, dsSceneCullNode_type());
	return dsSceneNode_setupParentType(type, &nodeType);
}

dsSceneModelNode* dsSceneModelNode_create(dsAllocator* allocator,
	const dsSceneModelInitInfo* models, uint32_t modelCount, const char* const* extraItemLists,
	uint32_t extraItemListCount, dsSceneResources** resources, uint32_t resourceCount,
	const dsOrientedBox3f* bounds)
{
	return dsSceneModelNode_createBase(allocator, sizeof(dsSceneModelNode), models, modelCount,
		extraItemLists, extraItemListCount, resources, resourceCount, bounds);
}

dsSceneModelNode* dsSceneModelNode_createBase(dsAllocator* allocator, size_t structSize,
	const dsSceneModelInitInfo* models, uint32_t modelCount, const char* const* extraItemLists,
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

	uint32_t drawRangeCount = 0;
	for (uint32_t i = 0; i < modelCount; ++i)
	{
		const dsSceneModelInitInfo* model = models + i;
		if (!model->geometry)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_SCENE_LOG_TAG, "All scene models must have valid geometry.");
			return NULL;
		}

		if (model->shader && model->material &&
			model->shader->materialDesc != dsMaterial_getDescription(model->material))
		{
			errno = EPERM;
			DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Model shader and material are incompatible.");
			return NULL;
		}

		if (!model->drawRanges || model->drawRangeCount == 0)
		{
			errno = EINVAL;
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Model doesn't have any draw ranges.");
			return NULL;
		}

		drawRangeCount += model->drawRangeCount;
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

	char** itemListsCopy = NULL;
	if (itemListCount > 0)
	{
		itemListsCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char*, itemListCount);
		DS_ASSERT(itemListsCopy);
		for (uint32_t i = 0; i < itemListCount; ++i)
		{
			size_t length = strlen(itemLists[i]);
			itemListsCopy[i] = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, length + 1);
			memcpy(itemListsCopy[i], itemLists[i], length + 1);
		}
	}

	if (itemLists != tempStringListData)
		DS_VERIFY(dsAllocator_free(allocator, (void*)itemLists));

	if (!dsSceneNode_initialize((dsSceneNode*)node, allocator,
			dsSceneModelNode_setupParentType(NULL), (const char**)itemListsCopy, itemListCount,
			&dsSceneModelNode_destroy))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		return NULL;
	}

	node->models = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneModelInfo, modelCount);
	DS_ASSERT(node->models);
	dsSceneModelDrawRange* drawRanges = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc,
		dsSceneModelDrawRange, drawRangeCount);
	DS_ASSERT(drawRanges);
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
		model->drawRanges = drawRanges;
		model->drawRangeCount = initInfo->drawRangeCount;
		memcpy((void*)model->drawRanges, initInfo->drawRanges,
			sizeof(dsSceneModelDrawRange)*initInfo->drawRangeCount);
		drawRanges += model->drawRangeCount;
		model->primitiveType = initInfo->primitiveType;
		if (initInfo->modelList)
			model->modelListID = dsHashString(initInfo->modelList);
		else
			model->modelListID = 0;
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

	dsSceneCullNode* cullNode = (dsSceneCullNode*)node;
	cullNode->getBoundsFunc = &dsModelNode_getBounds;

	return node;
}

dsSceneModelNode* dsSceneModelNode_cloneRemap(dsAllocator* allocator,
	const dsSceneModelNode* origModel, const dsSceneMaterialRemap* remaps, uint32_t remapCount)
{
	return dsSceneModelNode_cloneRemapBase(allocator, sizeof(dsSceneModelNode), origModel, remaps,
		remapCount);
}

dsSceneModelNode* dsSceneModelNode_cloneRemapBase(dsAllocator* allocator, size_t structSize,
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

	uint32_t drawRangeCount = 0;
	for (uint32_t i = 0; i < origModel->modelCount; ++i)
		drawRangeCount += origModel->models[i].drawRangeCount;

	size_t fullSize = cloneRemapFullAllocSize(structSize, origModel);
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
	char** itemListsCopy = NULL;
	if (itemListCount > 0)
	{
		itemListsCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char*, itemListCount);
		DS_ASSERT(itemListsCopy);
		for (uint32_t i = 0; i < itemListCount; ++i)
		{
			const char* origList = origNode->itemLists[i];
			size_t length = strlen(origList);
			itemListsCopy[i] = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, length + 1);
			memcpy(itemListsCopy[i], origList, length + 1);
		}
	}

	if (!dsSceneNode_initialize((dsSceneNode*)node, allocator,
			dsSceneModelNode_setupParentType(NULL), (const char**)itemListsCopy, itemListCount,
			&dsSceneModelNode_destroy))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		return NULL;
	}

	node->models = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, dsSceneModelInfo, origModel->modelCount);
	DS_ASSERT(node->models);
	dsSceneModelDrawRange* drawRanges = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc,
		dsSceneModelDrawRange, drawRangeCount);
	DS_ASSERT(drawRanges);
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

		model->drawRanges = drawRanges;
		memcpy((void*)model->drawRanges, origModel->models[i].drawRanges,
			model->drawRangeCount*sizeof(dsSceneModelDrawRange));
		drawRanges += model->drawRangeCount;
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

	dsSceneCullNode* cullNode = (dsSceneCullNode*)node;
	cullNode->getBoundsFunc = &dsModelNode_getBounds;

	DS_VERIFY(dsSceneModelNode_remapMaterials(node, remaps, remapCount));
	return node;
}

dsSceneModelNode* dsSceneModelNode_cloneReconfig(dsAllocator* allocator,
	const dsSceneModelNode* origModel, const dsSceneModelReconfig* models, uint32_t modelCount,
	const char* const* extraItemLists, uint32_t extraItemListCount)
{
	return dsSceneModelNode_cloneReconfigBase(allocator, sizeof(dsSceneModelNode), origModel,
		models, modelCount, extraItemLists, extraItemListCount);
}

dsSceneModelNode* dsSceneModelNode_cloneReconfigBase(dsAllocator* allocator,
	size_t structSize, const dsSceneModelNode* origModel, const dsSceneModelReconfig* models,
	uint32_t modelCount, const char* const* extraItemLists, uint32_t extraItemListCount)
{
	if (!allocator || structSize < sizeof(dsSceneModelNode) || !origModel || !models ||
		modelCount == 0 || (!extraItemLists && extraItemListCount > 0))
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

	dsSceneModelInitInfo tempModelInits[EXPECTED_MAX_NODES];
	dsSceneModelInitInfo* modelInits = tempModelInits;
	if (modelCount > EXPECTED_MAX_NODES)
	{
		modelInits = DS_ALLOCATE_OBJECT_ARRAY(allocator, dsSceneModelInitInfo, modelCount);
		if (!modelInits)
			return NULL;
	}

	for (uint32_t i = 0; i < modelCount; ++i)
	{
		dsSceneModelInitInfo* initInfo = modelInits + i;
		const dsSceneModelReconfig* reconfig = models + i;
		if (!reconfig->name || !reconfig->modelList || !reconfig->shader || !reconfig->material)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_SCENE_LOG_TAG,
				"All scene models must have a valid name, draw list name, shader, and material.");
			goto error;
		}

		const dsSceneModelInfo* baseInfo = NULL;
		for (uint32_t j = 0; j < origModel->modelCount; ++j)
		{
			const dsSceneModelInfo* curInfo = origModel->models + j;
			if (curInfo->name && strcmp(curInfo->name, reconfig->name) == 0)
			{
				baseInfo = curInfo;
				break;
			}
		}

		if (!baseInfo)
		{
			errno = ENOTFOUND;
			DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Reconfigured model '%s' not found.", reconfig->name);
			goto error;
		}

		initInfo->name = reconfig->name;
		initInfo->shader = reconfig->shader;
		initInfo->material = reconfig->material;
		initInfo->geometry = baseInfo->geometry;
		initInfo->distanceRange = reconfig->distanceRange;
		initInfo->drawRanges = baseInfo->drawRanges;
		initInfo->drawRangeCount = baseInfo->drawRangeCount;
		initInfo->primitiveType = baseInfo->primitiveType;
		initInfo->modelList = reconfig->modelList;
	}

#if DS_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

	dsSceneModelNode* model = dsSceneModelNode_createBase(allocator, structSize, modelInits,
		modelCount, extraItemLists, extraItemListCount, origModel->resources,
		origModel->resourceCount, &origModel->bounds);
	if (model)
		return model;

#if DS_GCC
#pragma GCC diagnostic push
#endif

error:
	if (modelInits != tempModelInits)
		DS_VERIFY(dsAllocator_free(allocator, modelInits));
	return NULL;
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

		uint32_t modelListID = 0;
		if (remap->modelList)
			modelListID = dsHashString(remap->modelList);

		for (uint32_t j = 0; j < node->modelCount; ++j)
		{
			dsSceneModelInfo* model = node->models + i;
			if (!model->name || strcmp(model->name, remap->name) != 0 ||
				(remap->modelList && model->modelListID != modelListID))
			{
				continue;
			}

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
	DS_ASSERT(dsSceneNode_isOfType(node, dsSceneModelNode_type()));
	dsSceneModelNode* modelNode = (dsSceneModelNode*)node;
	for (uint32_t i = 0; i < modelNode->resourceCount; ++i)
		dsSceneResources_freeRef(modelNode->resources[i]);
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

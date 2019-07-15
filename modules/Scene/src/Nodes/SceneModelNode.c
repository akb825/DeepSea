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

static int nodeType;

static size_t fullAllocSize(const char** drawLists, uint32_t drawListCount, uint32_t modelCount,
	uint32_t resourceCount)
{
	return DS_ALIGNED_SIZE(sizeof(dsSceneModelNode)) +
		dsSceneNode_drawListsAllocSize(drawLists, drawListCount) +
		DS_ALIGNED_SIZE(sizeof(dsSceneModelInfo)*modelCount) +
		DS_ALIGNED_SIZE(sizeof(dsSceneResources*)*resourceCount);
}

static void destroy(dsSceneNode* node)
{
	dsSceneModelNode* modelNode = (dsSceneModelNode*)node;
	for (uint32_t i = 0; i < modelNode->resourceCount; ++i)
		dsSceneResources_freeRef(modelNode->resources[i]);
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

static void populateDrawList(const char** drawLists, uint32_t* hashes, uint32_t* drawListCount,
	const dsSceneModelInitInfo* models, uint32_t modelCount)
{
	for (uint32_t i = 0; i < modelCount; ++i)
		hashes[i] = dsHashString(models[i].listName);

	for (uint32_t i = 0; i < modelCount; ++i)
	{
		bool unique = true;
		for (uint32_t j = 0; j < *drawListCount; ++j)
		{
			if (hashes[i] == hashes[j])
			{
				unique = false;
				break;
			}
		}

		if (!unique)
			continue;

		drawLists[*drawListCount] = models[i].listName;
		// Also make sure the assigned hashes match for faster uniqueness check.
		hashes[*drawListCount] = hashes[i];
		++*drawListCount;
	}
}

dsSceneNodeType dsSceneModelNode_type(void)
{
	return &nodeType;
}

dsSceneModelNode* dsSceneModelNode_create(dsAllocator* allocator,
	const dsSceneModelInitInfo* models, uint32_t modelCount, dsSceneResources** resources,
	uint32_t resourceCount, const dsOrientedBox3f* bounds)
{
	if (!allocator || !models || modelCount == 0 || (!resources && resourceCount > 0))
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
	const char** drawLists = tempStringListData;
	uint32_t drawListCount = 0;
	uint32_t* tempStringHashList = tempStringHashListData;
	if (modelCount > EXPECTED_MAX_NODES)
	{
		// Need to dynamically allocate temp lists if too large.
		drawLists = DS_ALLOCATE_OBJECT_ARRAY(allocator, const char*, modelCount);
		if (!drawLists)
			return NULL;
		tempStringHashList = DS_ALLOCATE_OBJECT_ARRAY(allocator, uint32_t, modelCount);
		if (!tempStringHashList)
		{
			DS_VERIFY(dsAllocator_free(allocator, drawLists));
			return NULL;
		}
	}

	populateDrawList(drawLists, tempStringHashList, &drawListCount, models, modelCount);

	size_t fullSize = fullAllocSize(drawLists, drawListCount, modelCount, resourceCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		if (drawLists != tempStringListData)
		{
			DS_VERIFY(dsAllocator_free(allocator, drawLists));
			DS_VERIFY(dsAllocator_free(allocator, tempStringHashList));
		}
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneModelNode* node = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneModelNode);
	DS_ASSERT(node);

	char** drawListsCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char*, drawListCount);
	DS_ASSERT(drawListsCopy);
	for (uint32_t i = 0; i < drawListCount; ++i)
	{
		size_t length = strlen(drawLists[i]);
		drawListsCopy[i] = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, length + 1);
		memcpy(drawListsCopy[i], drawLists[i], length + 1);
	}

	if (drawLists != tempStringListData)
	{
		DS_VERIFY(dsAllocator_free(allocator, drawLists));
		DS_VERIFY(dsAllocator_free(allocator, tempStringHashList));
	}

	if (!dsSceneNode_initialize((dsSceneNode*)node, allocator, dsSceneModelNode_type(),
			(const char**)drawListsCopy, drawListCount, &destroy))
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

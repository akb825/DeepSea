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

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Geometry/AlignedBox3.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/SceneResources.h>

#include <string.h>

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

dsSceneNodeType dsSceneModelNode_type(void)
{
	return &nodeType;
}

dsSceneModelNode* dsSceneModelNode_create(dsAllocator* allocator, const char** drawLists,
	uint32_t drawListCount, const dsSceneModelInfo* models, uint32_t modelCount,
	dsSceneResources** resources, uint32_t resourceCount, const dsAlignedBox3f* bounds)
{
	if (!allocator || !drawLists || drawListCount == 0 || !models || modelCount == 0 ||
		(!resources && resourceCount > 0))
	{
		errno = EINVAL;
		return NULL;
	}

	for (uint32_t i = 0; i < drawListCount; ++i)
	{
		if (!drawLists[i])
		{
			errno = EINVAL;
			return NULL;
		}
	}

	for (uint32_t i = 0; i < modelCount; ++i)
	{
		const dsSceneModelInfo* model = models + i;
		if (!model->shader || !model->material || !model->geometry)
		{
			errno = EINVAL;
			DS_LOG_ERROR(DS_SCENE_LOG_TAG,
				"All scene models must have a valid shader, material, and geometry.");
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

	size_t fullSize = fullAllocSize(drawLists, drawListCount, modelCount, resourceCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

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
		strcpy(drawListsCopy[i], drawLists[i]);
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
	memcpy(node->models, models, sizeof(dsSceneModelInfo)*modelCount);
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
		dsAlignedBox3f_makeInvalid(&node->bounds);

	return node;
}

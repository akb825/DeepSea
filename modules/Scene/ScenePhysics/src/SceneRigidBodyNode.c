/*
 * Copyright 2024 Aaron Barany
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

#include <DeepSea/ScenePhysics/SceneRigidBodyNode.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Physics/RigidBody.h>
#include <DeepSea/Physics/RigidBodyTemplate.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <DeepSea/ScenePhysics/ScenePhysicsList.h>

#include <string.h>

static void dsSceneRigidBodyNode_destroy(dsSceneNode* node)
{
	dsSceneRigidBodyNode* rigidBodyNode = (dsSceneRigidBodyNode*)node;
	if (rigidBodyNode->ownsPointer)
	{
		dsRigidBody_destroy(rigidBodyNode->rigidBody);
		dsRigidBodyTemplate_destroy(rigidBodyNode->rigidBodyTemplate);
	}
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

const char* const dsSceneRigidBodyNode_typeName = "RigidBodyNode";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsSceneRigidBodyNode_type(void)
{
	return &nodeType;
}

dsSceneRigidBodyNode* dsSceneRigidBodyNode_create(dsAllocator* allocator, const char* rigidBodyName,
	dsRigidBody* rigidBody, dsRigidBodyTemplate* rigidBodyTemplate, bool transferOwnership,
	const char* const* itemLists, uint32_t itemListCount)
{
	if (!allocator || (!rigidBodyName && !rigidBody && !rigidBodyTemplate) ||
		(!itemLists && itemListCount > 0))
	{
		if (transferOwnership)
		{
			dsRigidBody_destroy(rigidBody);
			dsRigidBodyTemplate_destroy(rigidBodyTemplate);
		}
		errno = EINVAL;
		return NULL;
	}

	unsigned int setCount =
		(rigidBodyName != NULL) + (rigidBody != NULL) + (rigidBodyTemplate != NULL);
	if (setCount != 1)
	{
		DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG, "Only one of rigidBodyName, rigidBody, or "
			"rigidBodyTemplate should be set for a rigid body transform node.");
		if (transferOwnership)
		{
			dsRigidBody_destroy(rigidBody);
			dsRigidBodyTemplate_destroy(rigidBodyTemplate);
		}
		errno = EINVAL;
		return NULL;
	}

	size_t nameLen = rigidBodyName ? strlen(rigidBodyName) + 1 : 0;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneRigidBodyNode)) +
		DS_ALIGNED_SIZE(nameLen) + dsSceneNode_itemListsAllocSize(itemLists, itemListCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		if (transferOwnership)
		{
			dsRigidBody_destroy(rigidBody);
			dsRigidBodyTemplate_destroy(rigidBodyTemplate);
		}
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneRigidBodyNode* node = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneRigidBodyNode);

	const char* const* itemListsCopy = dsSceneNode_copyItemLists((dsAllocator*)&bufferAlloc,
		itemLists, itemListCount);
	DS_ASSERT(itemListCount == 0 || itemListsCopy);

	if (!dsSceneNode_initialize((dsSceneNode*)node, allocator, dsSceneRigidBodyNode_type(),
			itemListsCopy, itemListCount, &dsSceneRigidBodyNode_destroy))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		if (transferOwnership)
		{
			dsRigidBody_destroy(rigidBody);
			dsRigidBodyTemplate_destroy(rigidBodyTemplate);
		}
		return NULL;
	}

	if (rigidBodyName)
	{
		char* nameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
		DS_ASSERT(nameCopy);
		memcpy(nameCopy, rigidBodyName, nameLen);
		node->rigidBodyName = nameCopy;
		node->rigidBodyID = dsUniqueNameID_create(rigidBodyName);
		node->rigidBody = NULL;
		node->rigidBodyTemplate = NULL;
		node->ownsPointer = false;
	}
	else
	{
		DS_ASSERT(rigidBody);
		node->rigidBodyName = NULL;
		node->rigidBodyID = 0;
		node->rigidBody = rigidBody;
		node->rigidBodyTemplate = rigidBodyTemplate;
		node->ownsPointer = transferOwnership;
	}

	return node;
}

dsRigidBody* dsSceneRigidBodyNode_getRigidBodyForInstance(const dsSceneTreeNode* treeNode)
{
	while (treeNode && !dsSceneNode_isOfType(treeNode->node, dsSceneRigidBodyNode_type()))
		treeNode = treeNode->parent;
	if (!treeNode)
		return NULL;

	const dsSceneNodeItemData* itemData = &treeNode->itemData;
	DS_ASSERT(itemData->count == treeNode->node->itemListCount);
	for (uint32_t i = 0; i < itemData->count; ++i)
	{
		const dsSceneItemList* itemList = treeNode->itemLists[i].list;
		if (itemList && itemList->type == dsScenePhysicsList_type())
			return (dsRigidBody*)itemData->itemData[i].data;
	}

	return NULL;
}

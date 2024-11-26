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

#include <DeepSea/ScenePhysics/ScenePhysicsConstraintNode.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/ScenePhysics/ScenePhysicsList.h>

#include <string.h>

static bool areParametersValid(dsPhysicsConstraint* constraint,
	const dsScenePhysicsActorReference* firstActor,
	const dsScenePhysicsConstraintReference* firstConnectedConstraint,
	const dsScenePhysicsActorReference* secondActor,
	const dsScenePhysicsConstraintReference* secondConnectedConstraint)
{
	bool hasFirstActor = false;
	if (firstActor)
	{
		if (firstActor->instanceName && firstActor->actor)
		{
			DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG, "First actor reference cannot have both "
				"instance name and actor pointer set for constraint node.");
			return false;
		}
		else if (firstActor->instanceName && !firstActor->rigidBodyGroupNode)
		{
			DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG, "First actor reference must have "
				"rigid body group node set when instance name is set for constraint node.");
			return false;
		}
		hasFirstActor = firstActor->instanceName || firstActor->actor;
	}

	if (!hasFirstActor && !constraint->firstActor)
	{
		DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG, "First actor not provided for constraint node.");
		return false;
	}

	if (firstConnectedConstraint)
	{
		if ((firstConnectedConstraint->instanceName != NULL) +
			(firstConnectedConstraint->constraintNode != NULL) +
			(firstConnectedConstraint->constraint != NULL) > 1)
		{
			DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG,
				"Only one of instance name, constraint node, or constraint pointer may be set for "
				"first connected constraint for constraint node.");
			return false;
		}
		else if (firstConnectedConstraint->instanceName &&
			!firstConnectedConstraint->rigidBodyGroupNode)
		{
			DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG, "First connected constraint reference must have "
				"rigid body group node set when instance name is set for constraint node.");
			return false;
		}
	}

	bool hasSecondActor = false;
	if (secondActor)
	{
		if (secondActor->instanceName && secondActor->actor)
		{
			DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG, "Second actor reference cannot have both "
				"instance name and actor pointer set for constraint node.");
			return false;
		}
		else if (secondActor->instanceName && !secondActor->rigidBodyGroupNode)
		{
			DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG, "Second actor reference must have "
				"rigidBodyGroupNode set when instance name is set for constraint node.");
			return false;
		}
		hasSecondActor = secondActor->instanceName || secondActor->actor;
	}

	if (!hasSecondActor && !constraint->secondActor)
	{
		DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG, "Second actor not provided for constraint node.");
		return false;
	}

	if (secondConnectedConstraint)
	{
		if ((secondConnectedConstraint->instanceName != NULL) +
			(secondConnectedConstraint->constraintNode != NULL) +
			(secondConnectedConstraint->constraint != NULL) > 1)
		{
			DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG,
				"Only one of instance name, constraint node, or constraint pointer may be set for "
				"second connected constraint for constraint node.");
			return false;
		}
		else if (secondConnectedConstraint->instanceName &&
			!secondConnectedConstraint->rigidBodyGroupNode)
		{
			DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG, "Second connected constraint reference must have "
				"rigid body group node set when instance name is set for constraint node.");
			return false;
		}
	}

	return true;
}

static void dsScenePhysicsConstraintNode_destroy(dsSceneNode* node)
{
	dsScenePhysicsConstraintNode* constraintNode = (dsScenePhysicsConstraintNode*)node;
	if (constraintNode->ownsConstraint)
		dsPhysicsConstraint_destroy(constraintNode->constraint);
	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

const char* const dsScenePhysicsConstraintNode_typeName = "PhysicsConstraintNode";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsScenePhysicsConstraintNode_type(void)
{
	return &nodeType;
}

dsScenePhysicsConstraintNode* dsScenePhysicsConstraintNode_create(dsAllocator* allocator,
	dsPhysicsConstraint* constraint, bool takeOwnership,
	const dsScenePhysicsActorReference* firstActor,
	const dsScenePhysicsConstraintReference* firstConnectedConstraint,
	const dsScenePhysicsActorReference* secondActor,
	const dsScenePhysicsConstraintReference* secondConnectedConstraint,
	const char* const* itemLists, uint32_t itemListCount)
{
	if (!allocator || !constraint || (!itemLists && itemListCount > 0) ||
		!areParametersValid(constraint, firstActor, firstConnectedConstraint, secondActor,
			secondConnectedConstraint))
	{
		if (takeOwnership)
			dsPhysicsConstraint_destroy(constraint);
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsScenePhysicsConstraintNode)) +
		dsSceneNode_itemListsAllocSize(itemLists, itemListCount);
	if (firstActor && firstActor->instanceName)
		fullSize += strlen(firstActor->instanceName) + 1;
	if (firstConnectedConstraint && firstConnectedConstraint->instanceName)
		fullSize += strlen(firstConnectedConstraint->instanceName) + 1;
	if (secondActor && secondActor->instanceName)
		fullSize += strlen(secondActor->instanceName) + 1;
	if (secondConnectedConstraint && secondConnectedConstraint->instanceName)
		fullSize += strlen(secondConnectedConstraint->instanceName) + 1;
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		if (takeOwnership)
			dsPhysicsConstraint_destroy(constraint);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsScenePhysicsConstraintNode* node =
		DS_ALLOCATE_OBJECT(&bufferAlloc, dsScenePhysicsConstraintNode);

	const char* const* itemListsCopy = dsSceneNode_copyItemLists((dsAllocator*)&bufferAlloc,
		itemLists, itemListCount);
	DS_ASSERT(itemListCount == 0 || itemListsCopy);

	if (!dsSceneNode_initialize((dsSceneNode*)node, allocator, dsScenePhysicsConstraintNode_type(),
			itemListsCopy, itemListCount, &dsScenePhysicsConstraintNode_destroy))
	{
		if (takeOwnership)
			dsPhysicsConstraint_destroy(constraint);
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		return NULL;
	}

	node->constraint = constraint;
	node->ownsConstraint = takeOwnership;
	if (firstActor)
		memcpy(&node->firstActor, firstActor, sizeof(dsScenePhysicsActorReference));
	else
		memset(&node->firstActor, 0, sizeof(dsScenePhysicsActorReference));
	if (firstConnectedConstraint)
	{
		memcpy(&node->firstConnectedConstraint, firstConnectedConstraint,
			sizeof(dsScenePhysicsConstraintReference));
	}
	else
		memset(&node->firstConnectedConstraint, 0, sizeof(dsScenePhysicsConstraintReference));
	if (secondActor)
		memcpy(&node->secondActor, secondActor, sizeof(dsScenePhysicsActorReference));
	else
		memset(&node->secondActor, 0, sizeof(dsScenePhysicsActorReference));
	if (secondConnectedConstraint)
	{
		memcpy(&node->secondConnectedConstraint, secondConnectedConstraint,
			sizeof(dsScenePhysicsConstraintReference));
	}
	else
		memset(&node->secondConnectedConstraint, 0, sizeof(dsScenePhysicsConstraintReference));

	if (firstActor->instanceName)
	{
		size_t nameLen = strlen(firstActor->instanceName) + 1;
		char* name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
		DS_ASSERT(name);
		memcpy(name, firstActor->instanceName, nameLen);
		node->firstActor.instanceName = name;
		node->firstActorInstanceID = dsHashString(name);
	}
	else
		node->firstActorInstanceID = 0;

	if (firstConnectedConstraint->instanceName)
	{
		size_t nameLen = strlen(firstConnectedConstraint->instanceName) + 1;
		char* name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
		DS_ASSERT(name);
		memcpy(name, firstConnectedConstraint->instanceName, nameLen);
		node->firstConnectedConstraint.instanceName = name;
		node->firstConnectedConstraintInstanceID = dsHashString(name);
	}
	else
		node->firstConnectedConstraintInstanceID = 0;

	if (secondActor->instanceName)
	{
		size_t nameLen = strlen(secondActor->instanceName) + 1;
		char* name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
		DS_ASSERT(name);
		memcpy(name, secondActor->instanceName, nameLen);
		node->secondActor.instanceName = name;
		node->secondActorInstanceID = dsHashString(name);
	}
	else
		node->secondActorInstanceID = 0;

	if (secondConnectedConstraint->instanceName)
	{
		size_t nameLen = strlen(secondConnectedConstraint->instanceName) + 1;
		char* name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
		DS_ASSERT(name);
		memcpy(name, secondConnectedConstraint->instanceName, nameLen);
		node->secondConnectedConstraint.instanceName = name;
		node->secondConnectedConstraintInstanceID = dsHashString(name);
	}
	else
		node->secondConnectedConstraintInstanceID = 0;

	return node;
}

dsPhysicsConstraint* dsScenePhysicsConstraintNode_getConstraintForInstance(
	const dsSceneTreeNode* treeNode)
{
	while (treeNode && !dsSceneNode_isOfType(treeNode->node, dsScenePhysicsConstraintNode_type()))
		treeNode = treeNode->parent;
	if (!treeNode)
		return NULL;

	const dsSceneNodeItemData* itemData = &treeNode->itemData;
	DS_ASSERT(itemData->count == treeNode->node->itemListCount);
	for (uint32_t i = 0; i < itemData->count; ++i)
	{
		const dsSceneItemList* itemList = treeNode->itemLists[i].list;
		if (itemList && itemList->type == dsScenePhysicsList_type())
			return (dsPhysicsConstraint*)itemData->itemData[i].data;
	}

	return NULL;
}

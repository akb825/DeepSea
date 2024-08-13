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

#include <DeepSea/ScenePhysics/SceneRigidBodyGroupNode.h>

#include "SceneRigidBodyGroupNodeData.h"

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>
#include <DeepSea/Physics/RigidBodyGroup.h>
#include <DeepSea/Physics/RigidBodyTemplate.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <DeepSea/ScenePhysics/ScenePhysicsList.h>

#include <string.h>

static void cleanup(const dsNamedSceneRigidBodyTemplate* rigidBodies, uint32_t rigidBodyCount,
	const dsNamedScenePhysicsConstraint* constraints, uint32_t constraintCount)
{
	if (rigidBodies)
	{
		for (uint32_t i = 0; i < rigidBodyCount; ++i)
		{
			const dsNamedSceneRigidBodyTemplate* rigidBody = rigidBodies + i;
			if (rigidBody->transferOwnership)
				dsRigidBodyTemplate_destroy(rigidBody->rigidBodyTemplate);
		}
	}

	if (constraints)
	{
		for (uint32_t i = 0; i < constraintCount; ++i)
		{
			const dsNamedScenePhysicsConstraint* constraint = constraints + i;
			if (constraints->transferOwnership)
				dsPhysicsConstraint_destroy(constraint->constraint);
		}
	}
}

static void dsSceneRigidBodyGroupNode_destroy(dsSceneNode* node)
{
	dsSceneRigidBodyGroupNode* groupNode = (dsSceneRigidBodyGroupNode*)node;
	if (groupNode->rigidBodies)
	{
		for (dsListNode* listNode = groupNode->rigidBodies->list.head; listNode;
			listNode = listNode->next)
		{
			RigidBodyNode* rigidBodyNode = (RigidBodyNode*)listNode;
			if (rigidBodyNode->owned)
				dsRigidBodyTemplate_destroy(rigidBodyNode->rigidBody);
		}
	}

	if (groupNode->constraints)
	{
		for (dsListNode* listNode = groupNode->constraints->list.head; listNode;
			listNode = listNode->next)
		{
			ConstraintNode* constraintNode = (ConstraintNode*)listNode;
			if (constraintNode->owned)
				dsPhysicsConstraint_destroy(constraintNode->constraint);
		}
	}

	DS_VERIFY(dsAllocator_free(node->allocator, node));
}

const char* const dsSceneRigidBodyGroupNode_typeName = "RigidBodyGroupNode";

static dsSceneNodeType nodeType;
const dsSceneNodeType* dsSceneRigidBodyGroupNode_type(void)
{
	return &nodeType;
}

dsSceneRigidBodyGroupNode* dsSceneRigidBodyGroupNode_create(dsAllocator* allocator,
	dsPhysicsMotionType motionType, const dsNamedSceneRigidBodyTemplate* rigidBodies,
	uint32_t rigidBodyCount, const dsNamedScenePhysicsConstraint* constraints,
	uint32_t constraintCount, const char* const* itemLists, uint32_t itemListCount)
{
	if (!allocator || motionType < dsPhysicsMotionType_Static ||
		motionType > dsPhysicsMotionType_Unknown || !rigidBodies || rigidBodyCount == 0 ||
		(!constraints && constraintCount > 0) || (!itemLists && itemListCount > 0))
	{
		cleanup(rigidBodies, rigidBodyCount, constraints, constraintCount);
		errno = EINVAL;
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsSceneRigidBodyNode)) +
		dsSceneNode_itemListsAllocSize(itemLists, itemListCount);

	for (uint32_t i = 0; i < rigidBodyCount; ++i)
	{
		const dsNamedSceneRigidBodyTemplate* rigidBody = rigidBodies + i;
		if (!rigidBody->name || !rigidBody->rigidBodyTemplate)
		{
			cleanup(rigidBodies, rigidBodyCount, constraints, constraintCount);
			errno = EINVAL;
			return NULL;
		}

		if (motionType != dsPhysicsMotionType_Unknown &&
			((rigidBody->rigidBodyTemplate->flags & dsRigidBodyFlags_MutableMotionType) ||
			rigidBody->rigidBodyTemplate->motionType != motionType))
		{
			DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG,
				"Rigid body '%s' doesn't have compatble motion type for rigid body group node.",
				rigidBody->name);
			cleanup(rigidBodies, rigidBodyCount, constraints, constraintCount);
			errno = EINVAL;
			return NULL;
		}

		fullSize += DS_ALIGNED_SIZE(strlen(rigidBody->name) + 1);
	}
	unsigned int rigidBodyTableSize = dsHashTable_tableSize(rigidBodyCount);
	size_t rigidBodyTableAllocSize = dsHashTable_fullAllocSize(rigidBodyTableSize);
	fullSize += rigidBodyTableAllocSize;
	fullSize += DS_ALIGNED_SIZE(sizeof(RigidBodyNode)*rigidBodyCount);

	for (uint32_t i = 0; i < constraintCount; ++i)
	{
		const dsNamedScenePhysicsConstraint* constraint = constraints + i;
		if (!constraint->name || !constraint->constraint)
		{
			cleanup(rigidBodies, rigidBodyCount, constraints, constraintCount);
			errno = EINVAL;
			return NULL;
		}

		fullSize += DS_ALIGNED_SIZE(strlen(constraint->name) + 1);
	}
	unsigned int constraintTableSize = dsHashTable_tableSize(constraintCount);
	size_t constraintTableAllocSize = 0;
	if (constraintCount > 0)
	{
		constraintTableAllocSize = dsHashTable_fullAllocSize(constraintTableSize);
		fullSize += constraintTableAllocSize;
		fullSize += DS_ALIGNED_SIZE(sizeof(ConstraintNode)*constraintCount);
	}

	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		cleanup(rigidBodies, rigidBodyCount, constraints, constraintCount);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneRigidBodyGroupNode* node = DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneRigidBodyGroupNode);

	const char* const* itemListsCopy = dsSceneNode_copyItemLists((dsAllocator*)&bufferAlloc,
		itemLists, itemListCount);
	DS_ASSERT(itemListCount == 0 || itemListsCopy);

	if (!dsSceneNode_initialize((dsSceneNode*)node, allocator, dsSceneRigidBodyGroupNode_type(),
			itemListsCopy, itemListCount, &dsSceneRigidBodyGroupNode_destroy))
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, node));
		cleanup(rigidBodies, rigidBodyCount, constraints, constraintCount);
		return NULL;
	}

	node->rigidBodies = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		rigidBodyTableAllocSize);
	DS_ASSERT(node->rigidBodies);
	DS_VERIFY(dsHashTable_initialize(
		node->rigidBodies, rigidBodyTableSize, &dsHashIdentity, &dsHash32Equal));
	RigidBodyNode* rigidBodyNodes =
		DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, RigidBodyNode, rigidBodyCount);
	DS_ASSERT(rigidBodyNodes);
	for (uint32_t i = 0; i < rigidBodyCount; ++i)
	{
		const dsNamedSceneRigidBodyTemplate* rigidBody = rigidBodies + i;
		RigidBodyNode* rigidBodyNode = rigidBodyNodes + i;
		rigidBodyNode->nameID = dsHashString(rigidBody->name);
		rigidBodyNode->index = i;
		rigidBodyNode->rigidBody = rigidBody->rigidBodyTemplate;
		rigidBodyNode->owned = rigidBody->transferOwnership;

		if (!dsHashTable_insert(node->rigidBodies, &rigidBodyNode->nameID,
				(dsHashTableNode*)rigidBodyNode, NULL))
		{
			DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG,
				"Multiple rigid bodies with name '%s' for scene rigid body group node.",
				rigidBody->name);
			if (allocator->freeFunc)
				DS_VERIFY(dsAllocator_free(allocator, node));
			cleanup(rigidBodies, rigidBodyCount, constraints, constraintCount);
			errno = EINVAL;
			return NULL;
		}
	}
	node->rigidBodyCount = rigidBodyCount;

	if (rigidBodyCount > 0)
	{
		node->constraints = (dsHashTable*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
			constraintTableAllocSize);
		DS_ASSERT(node->constraints);
		DS_VERIFY(dsHashTable_initialize(
			node->constraints, constraintTableSize, &dsHashIdentity, &dsHash32Equal));
		ConstraintNode* constraintNodes =
			DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, ConstraintNode, constraintCount);
		DS_ASSERT(constraintNodes);
		for (uint32_t i = 0; i < constraintCount; ++i)
		{
			const dsNamedScenePhysicsConstraint* constraint = constraints + i;
			ConstraintNode* constraintNode = constraintNodes + i;
			constraintNode->nameID = dsHashString(constraint->name);
			constraintNode->index = i;
			constraintNode->firstRigidBodyID =
				constraint->firstRigidBody ? dsHashString(constraint->firstRigidBody) : 0;
			constraintNode->firstConnectedConstraintID = constraint->firstConnectedConstraint ?
				dsHashString(constraint->firstConnectedConstraint) : 0;
			constraintNode->secondRigidBodyID =
				constraint->secondRigidBody ? dsHashString(constraint->secondRigidBody) : 0;
			constraintNode->secondConnectedConstraintID = constraint->secondConnectedConstraint ?
				dsHashString(constraint->secondConnectedConstraint) : 0;
			constraintNode->constraint = constraint->constraint;
			constraintNode->owned = constraint->transferOwnership;

			if (!dsHashTable_insert(node->rigidBodies, &constraintNode->nameID,
					(dsHashTableNode*)constraintNode, NULL))
			{
				DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG,
					"Multiple constraint with name '%s' for scene rigid body group node.",
					constraint->name);
				if (allocator->freeFunc)
					DS_VERIFY(dsAllocator_free(allocator, node));
				cleanup(rigidBodies, constraintCount, constraints, constraintCount);
				errno = EINVAL;
				return NULL;
			}

			const char* rigidBodyNames[2] =
				{constraint->firstRigidBody, constraint->secondRigidBody};
			uint32_t rigidBodyNameIDs[2] =
				{constraintNode->firstRigidBodyID, constraintNode->secondRigidBodyID};
			for (unsigned int j = 0; j < 2; ++j)
			{
				if (rigidBodyNames[j] && dsHashTable_find(node->rigidBodies, rigidBodyNameIDs + j))
				{
					DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG, "Rigid body '%s' not found for "
						"constraint '%s' for scene rigid body group node.",
						rigidBodyNames[j], constraint->name);
					if (allocator->freeFunc)
						DS_VERIFY(dsAllocator_free(allocator, node));
					cleanup(rigidBodies, constraintCount, constraints, constraintCount);
					errno = EINVAL;
					return NULL;
				}
			}
		}

		// Check for invalid connected constraints.
		for (uint32_t i = 0; i < constraintCount; ++i)
		{
			const dsNamedScenePhysicsConstraint* constraint = constraints + i;
			const char* connectedConstraintNames[2] = {constraint->firstConnectedConstraint,
				constraint->secondConnectedConstraint};
			for (unsigned int j = 0; j < 2; ++j)
			{
				const char* constraintName = connectedConstraintNames[j];
				uint32_t constraintID = dsHashString(constraintName);
				ConstraintNode* foundConstraint = (ConstraintNode*)dsHashTable_find(
					node->constraints, &constraintID);
				bool error = false;
				if (!foundConstraint)
				{
					DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG, "Connected constraint '%s' not found "
						"for constraint '%s' for scene rigid body group node.",
						constraintName, constraint->name);
					error = true;
				}
				else if (foundConstraint->firstConnectedConstraintID ||
					foundConstraint->secondConnectedConstraintID)
				{
					DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG, "Connected constraint '%s' must not "
						"itself have connected constraints for scene rigid body group node.",
						constraintName);
					error = true;
				}

				if (error)
				{
					if (allocator->freeFunc)
						DS_VERIFY(dsAllocator_free(allocator, node));
					cleanup(rigidBodies, constraintCount, constraints, constraintCount);
					errno = EINVAL;
					return NULL;
				}
			}
		}
	}
	else
		node->constraints = NULL;
	node->constraintCount = constraintCount;

	return node;
}

dsRigidBody* dsSceneRigidBodyGroupNode_findRigidBodyForInstanceName(const dsSceneTreeNode* treeNode,
	const char* name)
{
	if (!treeNode || !name)
		return NULL;

	return dsSceneRigidBodyGroupNode_findRigidBodyForInstanceID(treeNode, dsHashString(name));
}

dsRigidBody* dsSceneRigidBodyGroupNode_findRigidBodyForInstanceID(const dsSceneTreeNode* treeNode,
	uint32_t nameID)
{
	while (treeNode && !dsSceneNode_isOfType(treeNode->node, dsSceneRigidBodyGroupNode_type()))
		treeNode = treeNode->parent;
	if (!treeNode)
		return NULL;

	dsSceneRigidBodyGroupNode* groupNode = (dsSceneRigidBodyGroupNode*)treeNode->node;
	const dsSceneNodeItemData* itemData = &treeNode->itemData;
	DS_ASSERT(itemData->count == treeNode->node->itemListCount);
	for (uint32_t i = 0; i < itemData->count; ++i)
	{
		const dsSceneItemList* itemList = treeNode->itemLists[i].list;
		if (!itemList || itemList->type != dsScenePhysicsList_type())
			continue;

		RigidBodyNode* foundRigidBody = (RigidBodyNode*)dsHashTable_find(
			groupNode->rigidBodies, &nameID);
		if (!foundRigidBody)
			return NULL;

		dsSceneRigidBodyGroupNodeData* data =
			(dsSceneRigidBodyGroupNodeData*)itemData->itemData[i].data;
		return data->rigidBodies[foundRigidBody->index];
	}

	return NULL;
}

dsPhysicsConstraint* dsSceneRigidBodyGroupNode_findConstraintForInstanceName(
	const dsSceneTreeNode* treeNode, const char* name)
{
	if (!treeNode || !name)
		return NULL;

	return dsSceneRigidBodyGroupNode_findConstraintForInstanceID(treeNode, dsHashString(name));
}

dsPhysicsConstraint* dsSceneRigidBodyGroupNode_findConstraintForInstanceID(
	const dsSceneTreeNode* treeNode, uint32_t nameID)
{
	while (treeNode && !dsSceneNode_isOfType(treeNode->node, dsSceneRigidBodyGroupNode_type()))
		treeNode = treeNode->parent;
	if (!treeNode)
		return NULL;

	dsSceneRigidBodyGroupNode* groupNode = (dsSceneRigidBodyGroupNode*)treeNode->node;
	const dsSceneNodeItemData* itemData = &treeNode->itemData;
	DS_ASSERT(itemData->count == treeNode->node->itemListCount);
	for (uint32_t i = 0; i < itemData->count; ++i)
	{
		const dsSceneItemList* itemList = treeNode->itemLists[i].list;
		if (!itemList || itemList->type != dsScenePhysicsList_type())
			continue;

		ConstraintNode* foundConstraint = (ConstraintNode*)dsHashTable_find(
			groupNode->constraints, &nameID);
		if (!foundConstraint)
			return NULL;

		dsSceneRigidBodyGroupNodeData* data =
			(dsSceneRigidBodyGroupNodeData*)itemData->itemData[i].data;
		return data->constraints[foundConstraint->index];
	}

	return NULL;
}

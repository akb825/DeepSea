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

#include "SceneRigidBodyGroupNodeData.h"

#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>
#include <DeepSea/Physics/RigidBody.h>
#include <DeepSea/Physics/RigidBodyGroup.h>
#include <DeepSea/Physics/RigidBodyTemplate.h>

#include <string.h>

size_t dsSceneRigidBodyGroupNodeData_fullAllocSize(const dsSceneRigidBodyGroupNode* node)
{
	DS_ASSERT(node);
	return DS_ALIGNED_SIZE(sizeof(dsSceneRigidBodyGroupNodeData)) +
		DS_ALIGNED_SIZE(node->rigidBodyCount*sizeof(dsRigidBody*)) +
		DS_ALIGNED_SIZE(node->constraintCount*sizeof(dsPhysicsConstraint*));
}

dsSceneRigidBodyGroupNodeData* dsSceneRigidBodyGroupNodeData_create(
	dsAllocator* allocator, dsPhysicsEngine* physicsEngine, const dsSceneRigidBodyGroupNode* node)
{
	DS_ASSERT(allocator);
	DS_ASSERT(physicsEngine);
	DS_ASSERT(node);

	size_t fullSize = dsSceneRigidBodyGroupNodeData_fullAllocSize(node);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!fullSize)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsSceneRigidBodyGroupNodeData* data =
		DS_ALLOCATE_OBJECT(&bufferAlloc, dsSceneRigidBodyGroupNodeData);
	DS_ASSERT(data);

	data->allocator = dsAllocator_keepPointer(allocator);
	data->group = dsRigidBodyGroup_create(physicsEngine, allocator, node->motionType);
	if (!data->group)
	{
		if (data->allocator)
			dsAllocator_free(data->allocator, data);
		return NULL;
	}

	DS_ASSERT(node->rigidBodyCount > 0);
	data->rigidBodies = DS_ALLOCATE_OBJECT_ARRAY(allocator, dsRigidBody*, node->rigidBodyCount);
	DS_ASSERT(data->rigidBodies);
	// Initialized to NULL in case we need to clean up on failure since may be created out of order.
	memset(data->rigidBodies, 0, sizeof(dsRigidBody*)*node->rigidBodyCount);
	data->rigidBodyCount = node->rigidBodyCount;

	dsListNode* iterNode = node->rigidBodies->list.head;
	while (iterNode)
	{
		RigidBodyNode* rigidBodyNode = (RigidBodyNode*)iterNode;
		dsRigidBodyTemplate* rigidBodyTemplate = rigidBodyNode->rigidBody;
		// TODO: Generalized user data management.
		dsRigidBody* rigidBody = dsRigidBodyTemplate_instantiate(rigidBodyTemplate,
			allocator, NULL, NULL, data->group, NULL, NULL, NULL, NULL, NULL);
		if (!rigidBody)
		{
			for (uint32_t i = 0; i < data->rigidBodyCount; ++i)
				dsRigidBody_destroy(data->rigidBodies[i]);
			if (data->allocator)
				dsAllocator_free(data->allocator, data);
			return NULL;
		}

		data->rigidBodies[rigidBodyNode->index] = rigidBody;
		iterNode = iterNode->next;
	}

	if (node->constraintCount > 0)
	{
		data->constraints =
			DS_ALLOCATE_OBJECT_ARRAY(allocator, dsPhysicsConstraint*, node->constraintCount);
		DS_ASSERT(data->constraints);
		// Initialized to NULL in case we need to clean up on failure since may be created out of
		// order.
		memset(data->constraints, 0, sizeof(dsRigidBody*)*node->constraintCount);
		data->constraintCount = node->constraintCount;

		// First create constraints without connections.
		iterNode = node->constraints->list.head;
		while (iterNode)
		{
			ConstraintNode* constraintNode = (ConstraintNode*)iterNode;
			if (constraintNode->firstConnectedConstraintID ||
				constraintNode->secondConnectedConstraintID)
			{
				continue;
			}

			dsRigidBody* firstRigidBody = NULL;
			if (constraintNode->firstRigidBodyID)
			{
				RigidBodyNode* foundRigidBody = (RigidBodyNode*)dsHashTable_find(node->rigidBodies,
					&constraintNode->firstRigidBodyID);
				DS_ASSERT(foundRigidBody);
				firstRigidBody = data->rigidBodies[foundRigidBody->index];
			}

			dsRigidBody* secondRigidBody = NULL;
			if (constraintNode->secondRigidBodyID)
			{
				RigidBodyNode* foundRigidBody = (RigidBodyNode*)dsHashTable_find(node->rigidBodies,
					&constraintNode->secondRigidBodyID);
				DS_ASSERT(foundRigidBody);
				secondRigidBody = data->rigidBodies[foundRigidBody->index];
			}

			dsPhysicsConstraint* constraint = dsPhysicsConstraint_clone(constraintNode->constraint,
				allocator, (dsPhysicsActor*)firstRigidBody, NULL, (dsPhysicsActor*)secondRigidBody,
				NULL);
			if (!constraint)
			{
				for (uint32_t i = 0; i < data->rigidBodyCount; ++i)
					dsRigidBody_destroy(data->rigidBodies[i]);
				for (uint32_t i = 0; i < data->constraintCount; ++i)
					dsPhysicsConstraint_destroy(data->constraints[i]);
				if (data->allocator)
					dsAllocator_free(data->allocator, data);
			}

			data->constraints[constraintNode->index] = constraint;
			iterNode = iterNode->next;
		}

		// Then create constraints with connections.
		iterNode = node->constraints->list.head;
		while (iterNode)
		{
			ConstraintNode* constraintNode = (ConstraintNode*)iterNode;
			if (!constraintNode->firstConnectedConstraintID &&
				!constraintNode->secondConnectedConstraintID)
			{
				continue;
			}

			dsRigidBody* firstRigidBody = NULL;
			if (constraintNode->firstRigidBodyID)
			{
				RigidBodyNode* foundRigidBody = (RigidBodyNode*)dsHashTable_find(node->rigidBodies,
					&constraintNode->firstRigidBodyID);
				DS_ASSERT(foundRigidBody);
				firstRigidBody = data->rigidBodies[foundRigidBody->index];
			}

			dsPhysicsConstraint* firstConnectedConstraint = NULL;
			if (constraintNode->firstConnectedConstraintID)
			{
				ConstraintNode* foundConstraint =
					(ConstraintNode*)dsHashTable_find(node->constraints,
						&constraintNode->firstConnectedConstraintID);
				DS_ASSERT(foundConstraint);
				firstConnectedConstraint = data->constraints[foundConstraint->index];
			}

			dsRigidBody* secondRigidBody = NULL;
			if (constraintNode->secondRigidBodyID)
			{
				RigidBodyNode* foundRigidBody = (RigidBodyNode*)dsHashTable_find(node->rigidBodies,
					&constraintNode->secondRigidBodyID);
				DS_ASSERT(foundRigidBody);
				secondRigidBody = data->rigidBodies[foundRigidBody->index];
			}

			dsPhysicsConstraint* secondConnectedConstraint = NULL;
			if (constraintNode->secondConnectedConstraintID)
			{
				ConstraintNode* foundConstraint =
					(ConstraintNode*)dsHashTable_find(node->constraints,
						&constraintNode->secondConnectedConstraintID);
				DS_ASSERT(foundConstraint);
				secondConnectedConstraint = data->constraints[foundConstraint->index];
			}

			dsPhysicsConstraint* constraint = dsPhysicsConstraint_clone(constraintNode->constraint,
				allocator, (dsPhysicsActor*)firstRigidBody, firstConnectedConstraint,
				(dsPhysicsActor*)secondRigidBody, secondConnectedConstraint);
			if (!constraint)
			{
				for (uint32_t i = 0; i < data->rigidBodyCount; ++i)
					dsRigidBody_destroy(data->rigidBodies[i]);
				for (uint32_t i = 0; i < data->constraintCount; ++i)
					dsPhysicsConstraint_destroy(data->constraints[i]);
				if (data->allocator)
					dsAllocator_free(data->allocator, data);
			}

			data->constraints[constraintNode->index] = constraint;
			iterNode = iterNode->next;
		}
	}
	else
	{
		data->constraints = NULL;
		data->constraintCount = 0;
	}

	return data;
}

void dsSceneRigidBodyGroupNodeData_destroy(dsSceneRigidBodyGroupNodeData* data)
{
	if (!data)
		return;

	for (uint32_t i = 0; i < data->constraintCount; ++i)
		dsPhysicsConstraint_destroy(data->constraints[i]);
	for (uint32_t i = 0; i < data->rigidBodyCount; ++i)
		dsRigidBody_destroy(data->rigidBodies[i]);
}

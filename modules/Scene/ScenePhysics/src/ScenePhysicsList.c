/*
 * Copyright 2024-2026 Aaron Barany
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

#include <DeepSea/ScenePhysics/ScenePhysicsList.h>

#include "SceneRigidBodyGroupNodeData.h"

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/HashTable.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/RigidTransform3.h>
#include <DeepSea/Math/Round.h>
#include <DeepSea/Math/Vector3x.h>
#include <DeepSea/Math/Vector4.h>

#include <DeepSea/Physics/Constraints/PhysicsConstraint.h>
#include <DeepSea/Physics/PhysicsScene.h>
#include <DeepSea/Physics/RigidBody.h>
#include <DeepSea/Physics/RigidBodyTemplate.h>

#include <DeepSea/Scene/ItemLists/SceneItemListEntries.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneNodeItemData.h>
#include <DeepSea/Scene/Nodes/SceneShiftNode.h>
#include <DeepSea/Scene/Nodes/SceneTreeNode.h>
#include <DeepSea/Scene/Nodes/SceneUserDataNode.h>
#include <DeepSea/Scene/Scene.h>

#include <DeepSea/ScenePhysics/ScenePhysicsConstraintNode.h>
#include <DeepSea/ScenePhysics/SceneRigidBodyNode.h>
#include <DeepSea/ScenePhysics/SceneRigidBodyGroupNode.h>

#include <limits.h>
#include <string.h>

#define SHIFT_NODE_ENTRY_ID ((uint64_t)-2)
#define MIN_RIGID_BODY_ENTRY_ID (ULLONG_MAX/3)
#define MIN_CONSTRAINT_ENTRY_ID ((ULLONG_MAX/3)*2)
#define SCALE_EPSILON 1e-5f
#define T_EPSILON 1e-3f

typedef struct RigidBodyGroupEntry
{
	dsSceneTreeNode* treeNode;
	dsSceneRigidBodyGroupNodeData* data;
	uint64_t nodeID;
} RigidBodyGroupEntry;

typedef struct RigidBodyEntry
{
	dsSceneTreeNode* treeNode;
	dsRigidBody* rigidBody;

	dsRigidTransform3f prevTransform;
	dsRigidTransform3f targetTransform;
	dsVector3d prevOrigin;

	dsPhysicsMotionType prevMotionType;
	uint64_t nodeID;
} RigidBodyEntry;

typedef struct ConstraintEntry
{
	dsPhysicsConstraint* constraint;
	uint64_t nodeID;
} ConstraintEntry;

typedef struct dsScenePhysicsList
{
	dsSceneItemList itemList;

	dsPhysicsScene* physicsScene;
	bool ownsPhysicsScene;
	float targetStepTime;

	uint32_t preStepListenerID;
	float thisStepTime;
	float updateTime;
	dsVector3d prevOrigin;

	const dsSceneShiftNode* shiftNode;

	RigidBodyGroupEntry* groupEntries;
	uint32_t groupEntryCount;
	uint32_t maxGroupEntries;
	uint64_t nextGroupNodeID;

	uint64_t* removeGroupEntries;
	uint32_t removeGroupEntryCount;
	uint32_t maxRemoveGroupEntries;

	RigidBodyEntry* rigidBodyEntries;
	uint32_t rigidBodyEntryCount;
	uint32_t maxRigidBodyEntries;
	uint64_t nextRigidBodyNodeID;

	uint64_t* removeRigidBodyEntries;
	uint32_t removeRigidBodyEntryCount;
	uint32_t maxRemoveRigidBodyEntries;

	ConstraintEntry* constraintEntries;
	uint32_t constraintEntryCount;
	uint32_t maxConstraintEntries;
	uint64_t nextConstraintNodeID;

	uint64_t* removeConstraintEntries;
	uint32_t removeConstraintEntryCount;
	uint32_t maxRemoveConstraintEntries;
} dsScenePhysicsList;

static bool findPhysicsSceneFunc(dsSceneItemList* itemList, void* userData)
{
	if (itemList->type != dsScenePhysicsList_type())
		return true;

	dsScenePhysicsList* physicsList = (dsScenePhysicsList*)itemList;
	dsPhysicsScene** physicsScenePtr = (dsPhysicsScene**)userData;
	*physicsScenePtr = physicsList->physicsScene;
	return false; // Stop iteration.
}

static void dsScenePhysicsList_preStepUpdate(dsPhysicsScene* scene, float time, unsigned int step,
	unsigned int stepCount, const dsPhysicsSceneLock* lock, void* userData)
{
	DS_UNUSED(scene);
	DS_UNUSED(step);
	DS_UNUSED(stepCount);
	DS_UNUSED(lock);

	dsScenePhysicsList* physicsList = (dsScenePhysicsList*)userData;
	physicsList->thisStepTime += time;
	float t = physicsList->thisStepTime/physicsList->updateTime;
	// Update the target for each kinematic rigid body for this time step.
	if (t >= 1.0f - T_EPSILON)
	{
		// Set to final target.
		for (unsigned int i = 0; i < physicsList->rigidBodyEntryCount; ++i)
		{
			RigidBodyEntry* entry = physicsList->rigidBodyEntries + i;
			dsRigidBody* rigidBody = entry->rigidBody;
			const dsRigidTransform3f* prevTransform = &entry->prevTransform;
			const dsRigidTransform3f* targetTransform = &entry->targetTransform;
			if (rigidBody->motionType != dsPhysicsMotionType_Kinematic ||
				dsRigidTransform3f_equal(prevTransform, targetTransform))
			{
				continue;
			}

			dsRigidBody_setKinematicTarget(rigidBody, time, &targetTransform->position,
				&targetTransform->orientation);

			// Scale can't be updated kinematically, need to update immediately. Skip updating
			// scale if not different.
			if (!dsVector3xf_equal(&prevTransform->scale, &targetTransform->scale))
				dsRigidBody_setTransform(rigidBody, NULL, NULL, &targetTransform->scale, false);
		}
	}
	else
	{
		// Need to interpolate.
		for (unsigned int i = 0; i < physicsList->rigidBodyEntryCount; ++i)
		{
			RigidBodyEntry* entry = physicsList->rigidBodyEntries + i;
			dsRigidBody* rigidBody = entry->rigidBody;
			const dsRigidTransform3f* prevTransform = &entry->prevTransform;
			const dsRigidTransform3f* targetTransform = &entry->targetTransform;
			if (rigidBody->motionType != dsPhysicsMotionType_Kinematic ||
				dsRigidTransform3f_equal(prevTransform, targetTransform))
			{
				continue;
			}

			dsRigidTransform3f interpTransform;
			dsRigidTransform3f_nearLerp(&interpTransform, prevTransform, targetTransform, t);
			dsRigidBody_setKinematicTarget(rigidBody, time, &interpTransform.position,
				&interpTransform.orientation);

			// Scale can't be updated kinematically, need to update immediately. Skip updating
			// scale if not different.
			if (!dsVector3xf_equal(&prevTransform->scale, &targetTransform->scale))
				dsRigidBody_setTransform(rigidBody, NULL, NULL, &interpTransform.scale, false);
		}
	}
}

static uint64_t dsScenePhysicsList_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_ASSERT(itemList);
	DS_UNUSED(itemData);
	DS_UNUSED(treeNode);
	dsScenePhysicsList* physicsList = (dsScenePhysicsList*)itemList;

	if (dsSceneNode_isOfType(node, dsSceneShiftNode_type()))
	{
		if (physicsList->shiftNode)
		{
			DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG,
				"Only one shift node instance is allowed for a physics list.");
			return DS_NO_SCENE_NODE;
		}

		physicsList->shiftNode = (dsSceneShiftNode*)node;
		return SHIFT_NODE_ENTRY_ID;
	}
	if (dsSceneNode_isOfType(node, dsSceneRigidBodyGroupNode_type()))
	{
		const dsSceneRigidBodyGroupNode* groupNode = (const dsSceneRigidBodyGroupNode*)node;
		uint32_t index = physicsList->rigidBodyEntryCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, physicsList->groupEntries,
				physicsList->groupEntryCount, physicsList->maxGroupEntries, 1))
		{
			return DS_NO_SCENE_NODE;
		}

		void* userData = dsSceneUserDataNode_getInstanceData(treeNode);
		dsSceneRigidBodyGroupNodeData* data = dsSceneRigidBodyGroupNodeData_create(
			node->allocator, physicsList->physicsScene->engine, groupNode, userData);
		if (!data)
		{
			--physicsList->groupEntryCount;
			return DS_NO_SCENE_NODE;
		}

		dsPhysicsSceneLock lock;
		DS_VERIFY(dsPhysicsScene_lockWrite(&lock, physicsList->physicsScene));
		// Don't activate rigid bodies yet as the transform will be set for dsRigidBodyNode.
		bool added = dsPhysicsScene_addRigidBodyGroup(
			physicsList->physicsScene, data->group, false, &lock);
		added |= dsPhysicsScene_addConstraints(physicsList->physicsScene,
			data->constraints, data->constraintCount, true, &lock);
		DS_VERIFY(dsPhysicsScene_unlockWrite(&lock, physicsList->physicsScene));
		if (!added)
		{
			dsSceneRigidBodyGroupNodeData_destroy(data);
			--physicsList->groupEntryCount;
			return DS_NO_SCENE_NODE;
		}

		*thisItemData = data;

		RigidBodyGroupEntry* entry = physicsList->groupEntries + index;
		entry->treeNode = treeNode;
		entry->data = data;
		entry->nodeID = physicsList->nextGroupNodeID++;
		return entry->nodeID;
	}
	if (dsSceneNode_isOfType(node, dsSceneRigidBodyNode_type()))
	{
		const dsSceneRigidBodyNode* rigidBodyNode = (const dsSceneRigidBodyNode*)node;
		uint32_t index = physicsList->rigidBodyEntryCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, physicsList->rigidBodyEntries,
				physicsList->rigidBodyEntryCount, physicsList->maxRigidBodyEntries, 1))
		{
			return DS_NO_SCENE_NODE;
		}

		RigidBodyEntry* entry = physicsList->rigidBodyEntries + index;
		dsRigidBody* rigidBody;
		if (rigidBodyNode->rigidBody)
		{
			if (node->treeNodeCount != 1)
			{
				DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG, "Scene rigid body node with explicit rigid "
					"body may only be present once in the scene graph.");
				--physicsList->rigidBodyEntryCount;
				return DS_NO_SCENE_NODE;
			}

			dsPhysicsSceneLock lock;
			DS_VERIFY(dsPhysicsScene_lockWrite(&lock, physicsList->physicsScene));
			rigidBody = rigidBodyNode->rigidBody;
			bool added = dsPhysicsScene_addRigidBodies(
				physicsList->physicsScene, &entry->rigidBody, 1, false, &lock);
			DS_VERIFY(dsPhysicsScene_unlockWrite(&lock, physicsList->physicsScene));
			if (!added)
			{
				--physicsList->rigidBodyEntryCount;
				return DS_NO_SCENE_NODE;
			}
		}
		else if (rigidBodyNode->rigidBodyTemplate)
		{
			rigidBody = dsRigidBodyTemplate_instantiate(rigidBodyNode->rigidBodyTemplate,
				node->allocator, node->userData, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			if (!rigidBody)
			{
				--physicsList->rigidBodyEntryCount;
				return DS_NO_SCENE_NODE;
			}

			dsPhysicsSceneLock lock;
			DS_VERIFY(dsPhysicsScene_lockWrite(&lock, physicsList->physicsScene));
			bool added = dsPhysicsScene_addRigidBodies(
				physicsList->physicsScene, &rigidBody, 1, false, &lock);
			DS_VERIFY(dsPhysicsScene_unlockWrite(&lock, physicsList->physicsScene));
			if (!added)
			{
				dsRigidBody_destroy(rigidBody);
				--physicsList->rigidBodyEntryCount;
				return DS_NO_SCENE_NODE;
			}
		}
		else
		{
			DS_ASSERT(rigidBodyNode->rigidBodyName);

			// Search for a parent dsSceneRigidBodyGroupNode.
			rigidBody = dsSceneRigidBodyGroupNode_findRigidBodyForInstanceID(
				treeNode, rigidBodyNode->rigidBodyID);
			if (!rigidBody)
			{
				DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG,
					"Rigid body node '%s' not found for scene rigid body node.",
					rigidBodyNode->rigidBodyName);
				--physicsList->rigidBodyEntryCount;
				return DS_NO_SCENE_NODE;
			}
		}
		*thisItemData = entry->rigidBody;

		// Set the initial transform based on the current node.
		dsRigidTransform3f transform;
		if (treeNode->parent)
			dsSceneTreeNode_getCurrentStepTransform(&transform, treeNode->parent);
		else
			dsRigidTransform3f_identity(&transform);

		if (entry->rigidBody->motionType == dsPhysicsMotionType_Dynamic)
		{
			// Make sure the base and previous/current step transforms are all set.
			treeNode->baseStepTransform = &rigidBody->transform;
			treeNode->prevStepLocalTransform = treeNode->curStepLocalTransform =
				rigidBody->transform;
			treeNode->noParentTransform = true;
		}

		entry->rigidBody = rigidBody;
		DS_VERIFY(dsRigidBody_setTransform(
			rigidBody, &transform.position, &transform.orientation, &transform.scale, true));
		entry->prevTransform = entry->targetTransform = transform;
		if (physicsList->shiftNode)
			entry->prevOrigin = physicsList->shiftNode->origin;
		else
			entry->prevOrigin.x = entry->prevOrigin.y = entry->prevOrigin.z = 0.0;
		entry->prevMotionType = rigidBody->motionType;

		// Initialize the frame transform matrix based on the rigid body.
		dsRigidTransform3f_toMatrix(&treeNode->prevFrameWorldTransform, &rigidBody->transform);
		treeNode->curFrameWorldTransform = treeNode->prevFrameWorldTransform;

		entry->nodeID = physicsList->nextRigidBodyNodeID++;
		return entry->nodeID;
	}
	if (dsSceneNode_isOfType(node, dsScenePhysicsConstraintNode_type()))
	{
		const dsScenePhysicsConstraintNode* constraintNode =
			(const dsScenePhysicsConstraintNode*)node;
		uint32_t index = physicsList->constraintEntryCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, physicsList->constraintEntries,
				physicsList->constraintEntryCount, physicsList->maxConstraintEntries, 1))
		{
			return DS_NO_SCENE_NODE;
		}

		const dsPhysicsActor* firstActor = NULL;
		const dsScenePhysicsActorReference* firstActorRef = &constraintNode->firstActor;
		if (firstActorRef->actor)
			firstActor = firstActorRef->actor;
		else if (firstActorRef->instanceName)
		{
			const dsSceneTreeNode* treeNode = dsSceneNode_findUniqueTreeNode(
				firstActorRef->rootNode, (const dsSceneNode*)firstActorRef->rigidBodyGroupNode);
			firstActor =
				(const dsPhysicsActor*)dsSceneRigidBodyGroupNode_findRigidBodyForInstanceID(
					treeNode, constraintNode->firstActorInstanceID);
			if (!firstActor)
			{
				DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG,
					"First actor '%s' not found for constraint node.", firstActorRef->instanceName);
				--physicsList->constraintEntryCount;
				return DS_NO_SCENE_NODE;
			}
		}

		const dsPhysicsConstraint* firstConnectedConstraint = NULL;
		const dsScenePhysicsConstraintReference* firstConnectedConstraintRef =
			&constraintNode->firstConnectedConstraint;
		if (firstConnectedConstraintRef->constraint)
			firstConnectedConstraint = firstConnectedConstraintRef->constraint;
		else if (firstConnectedConstraintRef->constraintNode)
		{
			const dsSceneTreeNode* treeNode = dsSceneNode_findUniqueTreeNode(
				firstConnectedConstraintRef->rootNode,
				(const dsSceneNode*)firstConnectedConstraintRef->rigidBodyGroupNode);
			firstConnectedConstraint =
				dsScenePhysicsConstraintNode_getConstraintForInstance(treeNode);
			if (!firstConnectedConstraint)
			{
				DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG,
					"First connected constraint not found for constraint node.");
				--physicsList->constraintEntryCount;
				return DS_NO_SCENE_NODE;
			}
		}
		else if (firstConnectedConstraintRef->instanceName)
		{
			const dsSceneTreeNode* treeNode = dsSceneNode_findUniqueTreeNode(
				firstConnectedConstraintRef->rootNode,
				(const dsSceneNode*)firstConnectedConstraintRef->rigidBodyGroupNode);
			firstConnectedConstraint = dsSceneRigidBodyGroupNode_findConstraintForInstanceID(
				treeNode, constraintNode->firstConnectedConstraintInstanceID);
			if (!firstConnectedConstraint)
			{
				DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG,
					"First connected constraint '%s' not found for constraint node.",
					firstConnectedConstraintRef->instanceName);
				--physicsList->constraintEntryCount;
				return DS_NO_SCENE_NODE;
			}
		}

		const dsPhysicsActor* secondActor = NULL;
		const dsScenePhysicsActorReference* secondActorRef = &constraintNode->secondActor;
		if (secondActorRef->actor)
			secondActor = secondActorRef->actor;
		else if (secondActorRef->instanceName)
		{
			const dsSceneTreeNode* treeNode = dsSceneNode_findUniqueTreeNode(
				secondActorRef->rootNode, (const dsSceneNode*)secondActorRef->rigidBodyGroupNode);
			secondActor =
				(const dsPhysicsActor*)dsSceneRigidBodyGroupNode_findRigidBodyForInstanceID(
					treeNode, constraintNode->secondActorInstanceID);
			if (!secondActor)
			{
				DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG,
					"Second actor '%s' not found for constraint node.",
					secondActorRef->instanceName);
				--physicsList->constraintEntryCount;
				return DS_NO_SCENE_NODE;
			}
		}

		const dsPhysicsConstraint* secondConnectedConstraint = NULL;
		const dsScenePhysicsConstraintReference* secondConnectedConstraintRef =
			&constraintNode->secondConnectedConstraint;
		if (secondConnectedConstraintRef->constraint)
			secondConnectedConstraint = secondConnectedConstraintRef->constraint;
		else if (secondConnectedConstraintRef->constraintNode)
		{
			const dsSceneTreeNode* treeNode = dsSceneNode_findUniqueTreeNode(
				secondConnectedConstraintRef->rootNode,
				(const dsSceneNode*)secondConnectedConstraintRef->rigidBodyGroupNode);
			secondConnectedConstraint =
				dsScenePhysicsConstraintNode_getConstraintForInstance(treeNode);
			if (!secondConnectedConstraint)
			{
				DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG,
					"Second connected constraint not found for constraint node.");
				--physicsList->constraintEntryCount;
				return DS_NO_SCENE_NODE;
			}
		}
		else if (secondConnectedConstraintRef->instanceName)
		{
			const dsSceneTreeNode* treeNode = dsSceneNode_findUniqueTreeNode(
				secondConnectedConstraintRef->rootNode,
				(const dsSceneNode*)secondConnectedConstraintRef->rigidBodyGroupNode);
			secondConnectedConstraint = dsSceneRigidBodyGroupNode_findConstraintForInstanceID(
				treeNode, constraintNode->secondConnectedConstraintInstanceID);
			if (!secondConnectedConstraint)
			{
				DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG,
					"Second connected constraint '%s' not found for constraint node.",
					secondConnectedConstraintRef->instanceName);
				--physicsList->constraintEntryCount;
				return DS_NO_SCENE_NODE;
			}
		}

		ConstraintEntry* entry = physicsList->constraintEntries + index;
		entry->constraint = dsPhysicsConstraint_clone(constraintNode->constraint, node->allocator,
			firstActor, firstConnectedConstraint, secondActor, secondConnectedConstraint);
		if (!entry->constraint)
		{
			--physicsList->constraintEntryCount;
			return DS_NO_SCENE_NODE;
		}

		*thisItemData = entry->constraint;
		entry->nodeID = physicsList->nextConstraintNodeID++;
		return entry->nodeID;
	}
	return DS_NO_SCENE_NODE;
}

static void dsScenePhysicsList_removeNode(
	dsSceneItemList* itemList, dsSceneTreeNode* treeNode, uint64_t nodeID)
{
	DS_ASSERT(itemList);
	DS_UNUSED(treeNode);
	dsScenePhysicsList* physicsList = (dsScenePhysicsList*)itemList;
	if (nodeID == SHIFT_NODE_ENTRY_ID)
		physicsList->shiftNode = NULL;
	else if (nodeID < MIN_RIGID_BODY_ENTRY_ID)
	{
		RigidBodyGroupEntry* entry = (RigidBodyGroupEntry*)dsSceneItemListEntries_findEntry(
			physicsList->groupEntries, physicsList->groupEntryCount, sizeof(RigidBodyGroupEntry),
			offsetof(RigidBodyEntry, nodeID), nodeID);
		if (!entry)
			return;

		dsPhysicsSceneLock lock;
		DS_VERIFY(dsPhysicsScene_lockWrite(&lock, physicsList->physicsScene));
		DS_VERIFY(dsPhysicsScene_removeConstraints(physicsList->physicsScene,
			entry->data->constraints, entry->data->constraintCount, &lock));
		DS_VERIFY(dsPhysicsScene_removeRigidBodyGroup(physicsList->physicsScene,
			entry->data->group, &lock));
		DS_VERIFY(dsPhysicsScene_unlockWrite(&lock, physicsList->physicsScene));

		dsSceneRigidBodyGroupNodeData_destroy(entry->data);

		uint32_t index = physicsList->removeGroupEntryCount;
		if (DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, physicsList->removeGroupEntries,
				physicsList->removeGroupEntryCount, physicsList->maxRemoveGroupEntries, 1))
		{
			physicsList->removeGroupEntries[index] = nodeID;
		}
		else
		{
			dsSceneItemListEntries_removeSingleIndex(physicsList->groupEntries,
				&physicsList->groupEntryCount, sizeof(RigidBodyGroupEntry),
				entry - physicsList->groupEntries);
		}
	}
	else if (nodeID < MIN_CONSTRAINT_ENTRY_ID)
	{
		RigidBodyEntry* entry = (RigidBodyEntry*)dsSceneItemListEntries_findEntry(
			physicsList->rigidBodyEntries, physicsList->rigidBodyEntryCount, sizeof(RigidBodyEntry),
			offsetof(RigidBodyEntry, nodeID), nodeID);
		if (!entry)
			return;

		dsSceneRigidBodyNode* node = (dsSceneRigidBodyNode*)entry->treeNode->node;
		if (!node->rigidBodyName)
		{
			// Should be removed from the physics scene if not part of a rigid body group node.
			dsPhysicsSceneLock lock;
			DS_VERIFY(dsPhysicsScene_lockWrite(&lock, physicsList->physicsScene));
			DS_VERIFY(dsPhysicsScene_removeRigidBodies(physicsList->physicsScene,
				&entry->rigidBody, 1, &lock));
			DS_VERIFY(dsPhysicsScene_unlockWrite(&lock, physicsList->physicsScene));

			// If created from a template, also responsible for deleting the rigid body.
			if (node->rigidBodyTemplate)
				dsRigidBody_destroy(entry->rigidBody);
		}

		uint32_t index = physicsList->removeRigidBodyEntryCount;
		if (DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, physicsList->removeRigidBodyEntries,
				physicsList->removeRigidBodyEntryCount, physicsList->maxRemoveRigidBodyEntries, 1))
		{
			physicsList->removeRigidBodyEntries[index] = nodeID;
		}
		else
		{
			dsSceneItemListEntries_removeSingleIndex(physicsList->rigidBodyEntries,
				&physicsList->rigidBodyEntryCount, sizeof(RigidBodyEntry),
				entry - physicsList->rigidBodyEntries);
		}
	}
	else
	{
		ConstraintEntry* entry = (ConstraintEntry*)dsSceneItemListEntries_findEntry(
			physicsList->constraintEntries, physicsList->constraintEntryCount,
			sizeof(ConstraintEntry), offsetof(ConstraintEntry, nodeID), nodeID);
		if (!entry)
			return;

		dsPhysicsSceneLock lock;
		DS_VERIFY(dsPhysicsScene_lockWrite(&lock, physicsList->physicsScene));
		DS_VERIFY(dsPhysicsScene_removeConstraints(physicsList->physicsScene,
			&entry->constraint, 1, &lock));
		DS_VERIFY(dsPhysicsScene_unlockWrite(&lock, physicsList->physicsScene));

		DS_VERIFY(dsPhysicsConstraint_destroy(entry->constraint));

		uint32_t index = physicsList->removeConstraintEntryCount;
		if (DS_RESIZEABLE_ARRAY_ADD(itemList->allocator, physicsList->removeConstraintEntries,
				physicsList->removeConstraintEntryCount, physicsList->maxRemoveConstraintEntries,
				1))
		{
			physicsList->removeConstraintEntries[index] = nodeID;
		}
		else
		{
			dsSceneItemListEntries_removeSingleIndex(physicsList->constraintEntries,
				&physicsList->constraintEntryCount, sizeof(ConstraintEntry),
				entry - physicsList->constraintEntries);
		}
	}
}

static void dsScenePhysicsList_preTransformUpdate(
	dsSceneItemList* itemList, const dsScene* scene, const dsSceneTick* tick, unsigned int step)
{
	DS_ASSERT(itemList);
	DS_UNUSED(scene);
	DS_UNUSED(step);
	dsScenePhysicsList* physicsList = (dsScenePhysicsList*)itemList;

	// Lazily remove entries.
	dsSceneItemListEntries_removeMulti(physicsList->groupEntries, &physicsList->groupEntryCount,
		sizeof(RigidBodyGroupEntry), offsetof(RigidBodyGroupEntry, nodeID),
		physicsList->removeGroupEntries, physicsList->removeGroupEntryCount);
	physicsList->removeGroupEntryCount = 0;

	dsSceneItemListEntries_removeMulti(physicsList->rigidBodyEntries,
		&physicsList->rigidBodyEntryCount, sizeof(RigidBodyEntry), offsetof(RigidBodyEntry, nodeID),
		physicsList->removeRigidBodyEntries, physicsList->removeRigidBodyEntryCount);
	physicsList->removeRigidBodyEntryCount = 0;

	dsSceneItemListEntries_removeMulti(physicsList->constraintEntries,
		&physicsList->constraintEntryCount, sizeof(ConstraintEntry),
		offsetof(ConstraintEntry, nodeID), physicsList->removeConstraintEntries,
		physicsList->removeConstraintEntryCount);
	physicsList->removeConstraintEntryCount = 0;

	dsVector3d origin;
	if (physicsList->shiftNode)
		origin = physicsList->shiftNode->origin;
	else
		origin.x = origin.y = origin.z = 0.0;

	// Adjust the shift if the origin changed.
	if (!dsVector3d_equal(&origin, &physicsList->prevOrigin))
	{
		physicsList->prevOrigin = origin;
		for (unsigned int i = 0; i < physicsList->rigidBodyEntryCount; ++i)
		{
			RigidBodyEntry* entry = physicsList->rigidBodyEntries + i;
			dsRigidBody* rigidBody = entry->rigidBody;

			dsVector3d offset;
			dsVector3_sub(offset, entry->prevOrigin, origin);
			dsVector3xf offset3f = {{(float)offset.x, (float)offset.y, (float)offset.z}};
			entry->prevOrigin = origin;

			dsVector3xf newPosition;
			dsVector3xf_add(&newPosition, &rigidBody->transform.position, &offset3f);
			dsRigidBody_setTransform(rigidBody, &newPosition, NULL, NULL, false);
		}
	}

	// Update the kinematic and static rigid bodies based on the scene.
	for (unsigned int i = 0; i < physicsList->rigidBodyEntryCount; ++i)
	{
		RigidBodyEntry* entry = physicsList->rigidBodyEntries + i;
		dsRigidBody* rigidBody = entry->rigidBody;
		if (rigidBody->motionType != entry->prevMotionType)
		{
			dsSceneTreeNode* treeNode = entry->treeNode;
			if (rigidBody->motionType == dsPhysicsMotionType_Dynamic)
			{
				treeNode->baseStepTransform = &rigidBody->transform;
				treeNode->noParentTransform = true;
			}
			else if (entry->prevMotionType == dsPhysicsMotionType_Dynamic)
			{
				treeNode->baseStepTransform = NULL;
				treeNode->noParentTransform = false;

				// Need to reset the step transforms to identity.
				dsRigidTransform3f_identity(&treeNode->prevStepLocalTransform);
				treeNode->prevStepLocalTransform = treeNode->curStepLocalTransform;
			}
			entry->prevMotionType = rigidBody->motionType;
		}

		switch (rigidBody->motionType)
		{
			case dsPhysicsMotionType_Static:
			{
				dsRigidTransform3f transform;
				dsSceneTreeNode_getCurrentStepTransform(&transform, entry->treeNode->parent);
				if (!dsRigidTransform3f_equal(&entry->prevTransform, &transform))
				{
					// Static rigid bodies take the new position immediately.
					DS_VERIFY(dsRigidBody_setTransform(rigidBody, &transform.position,
						&transform.orientation, &transform.scale, false));
					entry->prevTransform = rigidBody->transform;
				}
				break;
			}
			case dsPhysicsMotionType_Kinematic:
			{
				dsRigidTransform3f transform;
				dsSceneTreeNode_getCurrentStepTransform(&transform, entry->treeNode->parent);
				if (!dsRigidTransform3f_equal(&entry->prevTransform, &transform))
				{
					// Will need to interpolate kinematic target for each step.
					entry->prevTransform = transform;
					entry->targetTransform.position = rigidBody->transform.position;
					entry->targetTransform.orientation = rigidBody->transform.orientation;
					// Only update scale if sufficiently different.
					if (dsVector3xf_epsilonEqual(&entry->targetTransform.scale,
							&rigidBody->transform.scale, SCALE_EPSILON))
					{
						entry->targetTransform.scale = entry->prevTransform.scale;
					}
					else
						entry->targetTransform.scale = rigidBody->transform.scale;
					// Keep orientations consistent so it can be lerped without checking each time.
					dsRigidTransform3f_makeOrientationConsistent(
						&entry->targetTransform, &transform.orientation);
				}
				break;
			}
			case dsPhysicsMotionType_Dynamic:
			case dsPhysicsMotionType_Unknown:
				break;
		}
	}

	float stepTime;
	unsigned int stepCount;
	if (tick->updatePeriod > 0.0f && physicsList->targetStepTime > 0.0f)
	{
		stepCount = (unsigned int)dsRoundf(tick->stepTime/physicsList->targetStepTime);
		if (stepCount == 0)
			stepCount = 1;
		stepTime = tick->stepTime/(float)stepCount;
	}
	else
	{
		stepTime = tick->stepTime;
		stepCount = 1;
	}
	physicsList->updateTime = tick->stepTime;
	physicsList->thisStepTime = 0.0f;
	DS_VERIFY(dsPhysicsScene_update(physicsList->physicsScene, stepTime, stepCount));

	// Update scene transforms for rigid bodies in motion.
	for (unsigned int i = 0; i < physicsList->rigidBodyEntryCount; ++i)
	{
		RigidBodyEntry* entry = physicsList->rigidBodyEntries + i;
		dsRigidBody* rigidBody = entry->rigidBody;
		if (rigidBody->motionType == dsPhysicsMotionType_Dynamic &&
			!dsRigidTransform3f_equal(&entry->prevTransform, &rigidBody->transform))
		{
			entry->prevTransform= rigidBody->transform;
			dsSceneTreeNode_markDirty(entry->treeNode);
		}
	}
}

static uint32_t dsScenePhysicsList_hash(const dsSceneItemList* itemList, uint32_t combinedHash)
{
	DS_ASSERT(itemList);
	const dsScenePhysicsList* physicsList = (const dsScenePhysicsList*)itemList;
	uint32_t hash = dsHashCombine32(combinedHash, &physicsList->targetStepTime);
	// Assume physics scenes will be equivalent if not explicitly owned.
	if (!physicsList->ownsPhysicsScene)
		hash = dsHashCombinePointer(hash, physicsList->physicsScene);
	return hash;
}

static bool dsScenePhysicsList_equal(const dsSceneItemList* left, const dsSceneItemList* right)
{
	DS_ASSERT(left);
	DS_ASSERT(left->type == dsScenePhysicsList_type());
	DS_ASSERT(right);
	DS_ASSERT(right->type == dsScenePhysicsList_type());

	const dsScenePhysicsList* leftPhysicsList = (const dsScenePhysicsList*)left;
	const dsScenePhysicsList* rightPhysicsList = (const dsScenePhysicsList*)right;
	return leftPhysicsList->ownsPhysicsScene == rightPhysicsList->ownsPhysicsScene &&
		(!leftPhysicsList->ownsPhysicsScene ||
			leftPhysicsList->physicsScene == rightPhysicsList->physicsScene) &&
		leftPhysicsList->targetStepTime == rightPhysicsList->targetStepTime;
}

static void dsScenePhysicsList_destroy(dsSceneItemList* itemList)
{
	DS_ASSERT(itemList);
	dsScenePhysicsList* physicsList = (dsScenePhysicsList*)itemList;

	// Handle removed entries before destroying their resources.
	dsSceneItemListEntries_removeMulti(physicsList->groupEntries, &physicsList->groupEntryCount,
		sizeof(RigidBodyGroupEntry), offsetof(RigidBodyGroupEntry, nodeID),
		physicsList->removeGroupEntries, physicsList->removeGroupEntryCount);
	dsSceneItemListEntries_removeMulti(physicsList->rigidBodyEntries,
		&physicsList->rigidBodyEntryCount, sizeof(RigidBodyEntry), offsetof(RigidBodyEntry, nodeID),
		physicsList->removeRigidBodyEntries, physicsList->removeRigidBodyEntryCount);
	dsSceneItemListEntries_removeMulti(physicsList->constraintEntries,
		&physicsList->constraintEntryCount, sizeof(ConstraintEntry),
		offsetof(ConstraintEntry, nodeID), physicsList->removeConstraintEntries,
		physicsList->removeConstraintEntryCount);

	if (physicsList->ownsPhysicsScene)
	{
		// Don't bother removing the rigid bodies and constraints when destroying the physics scene.
		for (uint32_t i = 0; i < physicsList->groupEntryCount; ++i)
			dsSceneRigidBodyGroupNodeData_destroy(physicsList->groupEntries[i].data);
		for (uint32_t i = 0; i < physicsList->constraintEntryCount; ++i)
			DS_VERIFY(dsPhysicsConstraint_destroy(physicsList->constraintEntries[i].constraint));
		dsPhysicsScene_destroy(physicsList->physicsScene);
	}
	else
	{
		dsPhysicsSceneLock lock;
		DS_VERIFY(dsPhysicsScene_lockWrite(&lock, physicsList->physicsScene));

		for (uint32_t i = 0; i < physicsList->groupEntryCount; ++i)
		{
			RigidBodyGroupEntry* entry = physicsList->groupEntries + i;
			DS_VERIFY(dsPhysicsScene_removeConstraints(physicsList->physicsScene,
				entry->data->constraints, entry->data->constraintCount, &lock));
			DS_VERIFY(dsPhysicsScene_removeRigidBodyGroup(
				physicsList->physicsScene, entry->data->group, &lock));
			dsSceneRigidBodyGroupNodeData_destroy(entry->data);
		}

		for (uint32_t i = 0; i < physicsList->rigidBodyEntryCount; ++i)
		{
			RigidBodyEntry* entry = physicsList->rigidBodyEntries + i;
			dsSceneRigidBodyNode* node = (dsSceneRigidBodyNode*)entry->treeNode->node;
			if (entry->rigidBody == node->rigidBody)
			{
				DS_VERIFY(dsPhysicsScene_removeRigidBodies(physicsList->physicsScene,
					&entry->rigidBody, 1, &lock));
			}
		}

		for (uint32_t i = 0; i < physicsList->constraintEntryCount; ++i)
		{
			ConstraintEntry* entry = physicsList->constraintEntries + i;
			DS_VERIFY(dsPhysicsScene_removeConstraints(physicsList->physicsScene,
				&entry->constraint, 1, &lock));
			DS_VERIFY(dsPhysicsConstraint_destroy(entry->constraint));
		}

		DS_VERIFY(dsPhysicsScene_unlockWrite(&lock, physicsList->physicsScene));

		DS_VERIFY(dsPhysicsScene_removePreStepListener(
			physicsList->physicsScene, physicsList->preStepListenerID));
	}
	DS_VERIFY(dsAllocator_free(itemList->allocator, physicsList->groupEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, physicsList->removeGroupEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, physicsList->rigidBodyEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, physicsList->removeRigidBodyEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, physicsList->constraintEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, physicsList->removeConstraintEntries));

	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

const char* const dsScenePhysicsList_typeName = "PhysicsList";

static dsSceneItemListType itemListType =
{
	.addNodeFunc = &dsScenePhysicsList_addNode,
	.removeNodeFunc = &dsScenePhysicsList_removeNode,
	.preTransformUpdateFunc = &dsScenePhysicsList_preTransformUpdate,
	.hashFunc = &dsScenePhysicsList_hash,
	.equalFunc = &dsScenePhysicsList_equal,
	.destroyFunc = &dsScenePhysicsList_destroy
};

const dsSceneItemListType* dsScenePhysicsList_type(void)
{
	return &itemListType;
}

dsSceneItemList* dsScenePhysicsList_create(dsAllocator* allocator, const char* name,
	dsPhysicsScene* physicsScene, bool takeOwnership, float targetStepTime)
{
	if (!allocator || !name || physicsScene || targetStepTime < 0)
	{
		if (takeOwnership)
			dsPhysicsScene_destroy(physicsScene);
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		if (takeOwnership)
			dsPhysicsScene_destroy(physicsScene);
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG,
			"Scene physics list allocator must support freeing memory.");
		return NULL;
	}

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsScenePhysicsList)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
	{
		if (takeOwnership)
			dsPhysicsScene_destroy(physicsScene);
		return NULL;
	}

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsScenePhysicsList* physicsList = DS_ALLOCATE_OBJECT(&bufferAlloc, dsScenePhysicsList);
	DS_ASSERT(physicsList);

	dsSceneItemList* itemList = (dsSceneItemList*)physicsList;
	itemList->allocator = allocator;
	itemList->type = dsScenePhysicsList_type();
	itemList->viewFilter = NULL;
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsUniqueNameID_create(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = false;
	itemList->skipPreRenderPass = false;

	physicsList->physicsScene = physicsScene;
	physicsList->ownsPhysicsScene = takeOwnership;
	physicsList->targetStepTime = targetStepTime;
	physicsList->preStepListenerID = dsPhysicsScene_addPreStepListener(physicsScene,
		&dsScenePhysicsList_preStepUpdate, physicsList, NULL);
	if (physicsList->preStepListenerID == DS_INVALID_PHYSICS_ID)
	{
		dsAllocator_free(allocator, itemList);
		if (takeOwnership)
			dsPhysicsScene_destroy(physicsScene);
		return NULL;
	}

	physicsList->thisStepTime = 0.0f;
	physicsList->updateTime = 0.0f;

	physicsList->groupEntries = NULL;
	physicsList->groupEntryCount = 0;
	physicsList->maxGroupEntries = 0;
	physicsList->nextGroupNodeID = 0;

	physicsList->removeGroupEntries = NULL;
	physicsList->removeGroupEntryCount = 0;
	physicsList->maxRemoveGroupEntries = 0;

	physicsList->rigidBodyEntries = NULL;
	physicsList->rigidBodyEntryCount = 0;
	physicsList->maxRigidBodyEntries = 0;
	physicsList->nextRigidBodyNodeID = MIN_RIGID_BODY_ENTRY_ID;

	physicsList->removeRigidBodyEntries = NULL;
	physicsList->removeRigidBodyEntryCount = 0;
	physicsList->maxRemoveRigidBodyEntries = 0;

	physicsList->constraintEntries = NULL;
	physicsList->constraintEntryCount = 0;
	physicsList->maxConstraintEntries = 0;
	physicsList->nextConstraintNodeID = MIN_CONSTRAINT_ENTRY_ID;

	physicsList->removeConstraintEntries = NULL;
	physicsList->removeConstraintEntryCount = 0;
	physicsList->maxRemoveConstraintEntries = 0;

	return itemList;
}

dsPhysicsScene* dsScenePhysicsList_getPhysicsScene(dsScene* scene)
{
	if (!scene)
	{
		errno = EINVAL;
		return NULL;
	}

	dsPhysicsScene* physicsScene = NULL;
	dsScene_forEachItemList(scene, &findPhysicsSceneFunc, &physicsScene);
	if (!physicsScene)
		errno = ENOTFOUND;
	return physicsScene;
}

/*
 * Copyright 2023 Aaron Barany
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

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Physics/PhysicsScene.h>
#include <DeepSea/Physics/RigidBody.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>

#include <DeepSea/ScenePhysics/SceneRigidBodyNode.h>

#include <string.h>

typedef struct RigidBodyEntry
{
	dsSceneTreeNode* treeNode;
	dsRigidBody* rigidBody;
	dsMatrix44f transform;
	dsMatrix44f prevTransform;
	dsQuaternion4f prevOrientation;
	dsVector3f prevPosition;
	dsVector3f prevScale;
	dsPhysicsMotionType prevMotionType;
	uint64_t nodeID;
} RigidBodyEntry;

typedef struct dsScenePhysicsList
{
	dsSceneItemList itemList;

	dsPhysicsScene* physicsScene;
	float targetStepTime;

	RigidBodyEntry* rigidBodyEntries;
	uint32_t rigidBodyEntryCount;
	uint32_t maxRigidBodyEntries;
	uint64_t nextRigidBodyNodeID;
} dsScenePhysicsList;

static uint64_t dsScenePhysicsList_addNode(dsSceneItemList* itemList, dsSceneNode* node,
	dsSceneTreeNode* treeNode, const dsSceneNodeItemData* itemData, void** thisItemData)
{
	DS_UNUSED(itemData);
	DS_UNUSED(treeNode);
	dsScenePhysicsList* physicsList = (dsScenePhysicsList*)itemList;

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
		if (rigidBodyNode->rigidBody)
		{
			dsPhysicsSceneLock lock;
			DS_VERIFY(dsPhysicsScene_lockWrite(&lock, physicsList->physicsScene));
			entry->rigidBody = rigidBodyNode->rigidBody;
			bool added = dsPhysicsScene_addRigidBodies(physicsList->physicsScene, &entry->rigidBody,
				1, true, &lock);
			DS_VERIFY(dsPhysicsScene_unlockWrite(&lock, physicsList->physicsScene));
			if (!added)
			{
				--physicsList->rigidBodyEntryCount;
				return DS_NO_SCENE_NODE;
			}
		}
		else
		{
			// TODO: Look up rigid body instance from parent.
		}
		*thisItemData = entry->rigidBody;

		dsMatrix44_identity(treeNode->transform);
		if (entry->rigidBody->motionType == dsPhysicsMotionType_Dynamic)
		{
			treeNode->baseTransform = &entry->transform;
			treeNode->noParentTransform = true;
		}
		entry->prevOrientation = entry->rigidBody->orientation;
		entry->prevPosition = entry->rigidBody->position;
		entry->prevScale = entry->rigidBody->scale;
		entry->prevMotionType = entry->rigidBody->motionType;
		DS_VERIFY(dsRigidBody_getTransformMatrix(&entry->prevTransform, entry->rigidBody));

		entry->nodeID = physicsList->nextRigidBodyNodeID++;
		return entry->nodeID;
	}
	return DS_NO_SCENE_NODE;
}

static void dsScenePhysicsList_removeNode(dsSceneItemList* itemList, uint64_t nodeID)
{
	dsScenePhysicsList* physicsList = (dsScenePhysicsList*)itemList;
	for (uint32_t i = 0; i < physicsList->rigidBodyEntryCount; ++i)
	{
		RigidBodyEntry* entry = physicsList->rigidBodyEntries + i;
		if (entry->nodeID != nodeID)
			continue;

		dsSceneRigidBodyNode* node = (dsSceneRigidBodyNode*)entry->treeNode->node;
		if (entry->rigidBody == node->rigidBody)
		{
			dsPhysicsSceneLock lock;
			DS_VERIFY(dsPhysicsScene_lockWrite(&lock, physicsList->physicsScene));
			DS_VERIFY(dsPhysicsScene_removeRigidBodies(physicsList->physicsScene, &entry->rigidBody,
				1, &lock));
			DS_VERIFY(dsPhysicsScene_unlockWrite(&lock, physicsList->physicsScene));
		}

		// Order shouldn't matter, so use constant-time removal.
		physicsList->rigidBodyEntries[i] =
			physicsList->rigidBodyEntries[physicsList->rigidBodyEntryCount - 1];
		--physicsList->rigidBodyEntryCount;
		break;
	}
}

static void dsScenePhysicsList_preTransformUpdate(dsSceneItemList* itemList, const dsScene* scene,
	float time)
{
	DS_UNUSED(scene);
	dsScenePhysicsList* physicsList = (dsScenePhysicsList*)itemList;

	// Update the kinematic and static rigid bodies based on the scene.
	for (unsigned int i = 0; i < physicsList->rigidBodyEntryCount; ++i)
	{
		RigidBodyEntry* entry = physicsList->rigidBodyEntries + i;
		if (entry->rigidBody->motionType != entry->prevMotionType)
		{
			entry->prevMotionType = entry->rigidBody->motionType;
			if (entry->rigidBody->motionType == dsPhysicsMotionType_Dynamic)
			{
				entry->treeNode->baseTransform = &entry->transform;
				entry->treeNode->noParentTransform = true;
			}
			else
			{
				entry->treeNode->baseTransform = NULL;
				entry->treeNode->noParentTransform = false;
			}
		}

		switch (entry->rigidBody->motionType)
		{
			case dsPhysicsMotionType_Static:
			{
				dsMatrix44f transform;
				dsSceneTreeNode_getCurrentTransform(&transform, entry->treeNode->parent);
				if (memcmp(&transform, &entry->prevTransform, sizeof(dsMatrix44f)) != 0)
				{
					DS_VERIFY(dsRigidBody_setTransformMatrix(entry->rigidBody, &transform, false));
					entry->prevTransform = transform;
					entry->prevOrientation = entry->rigidBody->orientation;
					entry->prevPosition = entry->rigidBody->position;
					entry->prevScale = entry->rigidBody->scale;
				}
				break;
			}
			case dsPhysicsMotionType_Kinematic:
			{
				// TODO: Interpolate for each step.
				break;
			}
			case dsPhysicsMotionType_Dynamic:
				break;
		}
	}

	unsigned int stepCount = (unsigned int)roundf(time/physicsList->targetStepTime);
	if (stepCount == 0)
		stepCount = 1;
	DS_VERIFY(dsPhysicsScene_update(physicsList->physicsScene, time, stepCount));

	// Update scene transforms for dynamic rigid bodies.
	for (unsigned int i = 0; i < physicsList->rigidBodyEntryCount; ++i)
	{
		RigidBodyEntry* entry = physicsList->rigidBodyEntries + i;
		if (entry->rigidBody->motionType == dsPhysicsMotionType_Dynamic &&
			(memcmp(&entry->rigidBody->orientation, &entry->prevOrientation,
				sizeof(dsQuaternion4f)) != 0 ||
			memcmp(&entry->rigidBody->position, &entry->prevPosition, sizeof(dsVector3f)) != 0 ||
			memcmp(&entry->rigidBody->scale, &entry->prevScale, sizeof(dsVector3f)) != 0))
		{
			DS_VERIFY(dsRigidBody_getTransformMatrix(&entry->transform, entry->rigidBody));
			entry->prevTransform = entry->transform;
			entry->prevOrientation = entry->rigidBody->orientation;
			entry->prevPosition = entry->rigidBody->position;
			entry->prevScale = entry->rigidBody->scale;
		}
	}
}

static void dsScenePhysicsList_destroy(dsSceneItemList* itemList)
{
	dsScenePhysicsList* physicsList = (dsScenePhysicsList*)itemList;
	dsPhysicsSceneLock lock;
	DS_VERIFY(dsPhysicsScene_lockWrite(&lock, physicsList->physicsScene));
	for (uint32_t i = 0; i < physicsList->rigidBodyEntryCount; ++i)
	{
		RigidBodyEntry* entry = physicsList->rigidBodyEntries + i;
		dsSceneRigidBodyNode* node = (dsSceneRigidBodyNode*)entry->treeNode->node;
		if (entry->rigidBody == node->rigidBody)
		{
			DS_VERIFY(dsPhysicsScene_removeRigidBodies(physicsList->physicsScene, &entry->rigidBody,
				1, &lock));
		}
	}
	DS_VERIFY(dsAllocator_free(itemList->allocator, physicsList->rigidBodyEntries));
	DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

const char* const dsScenePhysicsList_typeName = "PhysicsList";

dsSceneItemListType dsScenePhysicsList_type(void)
{
	static int type;
	return &type;
}

dsSceneItemList* dsScenePhysicsList_create(dsAllocator* allocator, const char* name,
	dsPhysicsScene* physicsScene, float targetStepTime)
{
	if (!allocator || !name || physicsScene || targetStepTime <= 0)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG,
			"Scene physics list allocator must support freeing memory.");
		return NULL;
	}

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsScenePhysicsList)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsScenePhysicsList* physicsList = DS_ALLOCATE_OBJECT(&bufferAlloc, dsScenePhysicsList);
	DS_ASSERT(physicsList);

	dsSceneItemList* itemList = (dsSceneItemList*)physicsList;
	itemList->allocator = allocator;
	itemList->type = dsScenePhysicsList_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsHashString(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = true;
	itemList->addNodeFunc = &dsScenePhysicsList_addNode;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = &dsScenePhysicsList_removeNode;
	itemList->preTransformUpdateFunc = &dsScenePhysicsList_preTransformUpdate;
	itemList->updateFunc = NULL;
	itemList->commitFunc = NULL;
	itemList->preRenderPassFunc = NULL;
	itemList->destroyFunc = &dsScenePhysicsList_destroy;

	physicsList->physicsScene = physicsScene;
	physicsList->targetStepTime = targetStepTime;
	physicsList->rigidBodyEntries = NULL;
	physicsList->rigidBodyEntryCount = 0;
	physicsList->maxRigidBodyEntries = 0;
	physicsList->nextRigidBodyNodeID = 0;

	return itemList;
}

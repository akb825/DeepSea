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

#include "ScenePhysicsConstraintNodeLoad.h"

#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <DeepSea/ScenePhysics/ScenePhysicsConstraint.h>
#include <DeepSea/ScenePhysics/ScenePhysicsConstraintNode.h>
#include <DeepSea/ScenePhysics/SceneRigidBody.h>
#include <DeepSea/ScenePhysics/SceneRigidBodyGroupNode.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/ScenePhysicsConstraintNode_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

static dsSceneNode* findNode(dsSceneLoadScratchData* scratchData, const char* name)
{
	dsSceneResourceType resourceType;
	dsSceneNode* node;
	if (dsSceneLoadScratchData_findResource(&resourceType, reinterpret_cast<void**>(&node),
			scratchData, name) && resourceType == dsSceneResourceType_SceneNode)
	{
		return node;
	}

	DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG, "Couldn't find scene node '%s'.", name);
	errno = ENOTFOUND;
	return nullptr;
}

static dsSceneRigidBodyGroupNode* findRigidBodyGroupNode(
	dsSceneLoadScratchData* scratchData, const char* name)
{
	dsSceneResourceType resourceType;
	dsSceneNode* node;
	if (dsSceneLoadScratchData_findResource(&resourceType, reinterpret_cast<void**>(&node),
			scratchData, name) && resourceType == dsSceneResourceType_SceneNode &&
			dsSceneNode_isOfType(node, dsSceneRigidBodyGroupNode_type()))
	{
		return reinterpret_cast<dsSceneRigidBodyGroupNode*>(node);
	}

	DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG, "Couldn't find rigid body group node '%s'.", name);
	errno = ENOTFOUND;
	return nullptr;
}

static dsPhysicsActor* findActor(dsSceneLoadScratchData* scratchData, const char* name)
{
	dsSceneResourceType resourceType;
	dsCustomSceneResource* resource;
	if (!dsSceneLoadScratchData_findResource(&resourceType, reinterpret_cast<void**>(&resource),
			scratchData, name) || resourceType != dsSceneResourceType_Custom)
	{
		DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG, "Couldn't find physics actor '%s'.", name);
		errno = ENOTFOUND;
		return nullptr;
	}

	if (resource->type == dsSceneRigidBody_type())
		return reinterpret_cast<dsPhysicsActor*>(resource->resource);

	DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG, "Couldn't find physics actor '%s'.", name);
	errno = ENOTFOUND;
	return nullptr;
}

static dsScenePhysicsConstraintNode* findConstraintNode(
	dsSceneLoadScratchData* scratchData, const char* name)
{
	dsSceneResourceType resourceType;
	dsSceneNode* node;
	if (dsSceneLoadScratchData_findResource(&resourceType, reinterpret_cast<void**>(&node),
			scratchData, name) && resourceType == dsSceneResourceType_SceneNode &&
			dsSceneNode_isOfType(node, dsScenePhysicsConstraintNode_type()))
	{
		return reinterpret_cast<dsScenePhysicsConstraintNode*>(node);
	}

	DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG, "Couldn't find physics constraint node '%s'.", name);
	errno = ENOTFOUND;
	return nullptr;
}

static dsPhysicsConstraint* findConstraint(dsSceneLoadScratchData* scratchData, const char* name)
{
	dsSceneResourceType resourceType;
	dsCustomSceneResource* resource;
	if (dsSceneLoadScratchData_findResource(&resourceType, reinterpret_cast<void**>(&resource),
			scratchData, name) && resourceType == dsSceneResourceType_Custom &&
			resource->type == dsScenePhysicsConstraint_type())
	{
		return reinterpret_cast<dsScenePhysicsConstraint*>(resource->resource)->constraint;
	}

	DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG, "Couldn't find physics constraint '%s'.", name);
	errno = ENOTFOUND;
	return nullptr;
}

dsSceneNode* dsScenePhysicsConstraintNode_load(const dsSceneLoadContext*,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator*,
	void*, const uint8_t* data, size_t dataSize)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScenePhysics::VerifyPhysicsConstraintNodeBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG, "Invalid physics constraint node flatbuffer format.");
		return nullptr;
	}

	auto fbConstraintNode = DeepSeaScenePhysics::GetPhysicsConstraintNode(data);

	const char* constraintName = fbConstraintNode->constraint()->c_str();
	dsPhysicsConstraint* constraint = findConstraint(scratchData, constraintName);
	if (!constraint)
		return nullptr;

	dsScenePhysicsActorReference firstActor = {};
	switch (fbConstraintNode->firstActor_type())
	{
		case DeepSeaScenePhysics::ActorReference::InstanceReference:
		{
			auto fbInstanceRef = fbConstraintNode->firstActor_as_InstanceReference();
			auto fbRootNode = fbInstanceRef->rootNode();
			if (fbRootNode)
			{
				firstActor.rootNode = findNode(scratchData, fbInstanceRef->rootNode()->c_str());
				if (!firstActor.rootNode)
					return nullptr;
			}
			firstActor.rigidBodyGroupNode = findRigidBodyGroupNode(
				scratchData, fbInstanceRef->rigidBodyGroupNode()->c_str());
			if (!firstActor.rigidBodyGroupNode)
				return nullptr;
			firstActor.instanceName = fbInstanceRef->instance()->c_str();
			break;
		}
		case DeepSeaScenePhysics::ActorReference::ActorResourceReference:
		{
			auto fbActorResourceRef = fbConstraintNode->firstActor_as_ActorResourceReference();
			firstActor.actor = findActor(scratchData, fbActorResourceRef->actor()->c_str());
			if (!firstActor.actor)
				return nullptr;
		}
		default:
			break;
	}

	dsScenePhysicsConstraintReference firstConnectedConstraint = {};
	switch (fbConstraintNode->firstConnectedConstraint_type())
	{
		case DeepSeaScenePhysics::ConstraintReference::InstanceReference:
		{
			auto fbInstanceRef = fbConstraintNode->firstConnectedConstraint_as_InstanceReference();
			auto fbRootNode = fbInstanceRef->rootNode();
			if (fbRootNode)
			{
				firstActor.rootNode = findNode(scratchData, fbInstanceRef->rootNode()->c_str());
				if (!firstActor.rootNode)
					return nullptr;
			}
			firstConnectedConstraint.rigidBodyGroupNode = findRigidBodyGroupNode(
				scratchData, fbInstanceRef->rigidBodyGroupNode()->c_str());
			if (!firstConnectedConstraint.rigidBodyGroupNode)
				return nullptr;
			firstConnectedConstraint.instanceName = fbInstanceRef->instance()->c_str();
			break;
		}
		case DeepSeaScenePhysics::ConstraintReference::ConstraintNodeReference:
		{
			auto fbConstraintNodeRef =
				fbConstraintNode->firstConnectedConstraint_as_ConstraintNodeReference();
			firstConnectedConstraint.constraintNode = findConstraintNode(
				scratchData, fbConstraintNodeRef->constraintNode()->c_str());
			if (!firstConnectedConstraint.constraintNode)
				return nullptr;
		}
		case DeepSeaScenePhysics::ConstraintReference::ConstraintResourceReference:
		{
			auto fbConstraintResourceRef =
				fbConstraintNode->firstConnectedConstraint_as_ConstraintResourceReference();
			firstConnectedConstraint.constraint = findConstraint(
				scratchData, fbConstraintResourceRef->constraint()->c_str());
			if (!firstConnectedConstraint.constraint)
				return nullptr;
		}
		default:
			break;
	}

	dsScenePhysicsActorReference secondActor = {};
	switch (fbConstraintNode->secondActor_type())
	{
		case DeepSeaScenePhysics::ActorReference::InstanceReference:
		{
			auto fbInstanceRef = fbConstraintNode->secondActor_as_InstanceReference();
			auto fbRootNode = fbInstanceRef->rootNode();
			if (fbRootNode)
			{
				secondActor.rootNode = findNode(scratchData, fbInstanceRef->rootNode()->c_str());
				if (!secondActor.rootNode)
					return nullptr;
			}
			secondActor.rigidBodyGroupNode = findRigidBodyGroupNode(
				scratchData, fbInstanceRef->rigidBodyGroupNode()->c_str());
			if (!secondActor.rigidBodyGroupNode)
				return nullptr;
			secondActor.instanceName = fbInstanceRef->instance()->c_str();
			break;
		}
		case DeepSeaScenePhysics::ActorReference::ActorResourceReference:
		{
			auto fbActorResourceRef = fbConstraintNode->secondActor_as_ActorResourceReference();
			secondActor.actor = findActor(scratchData, fbActorResourceRef->actor()->c_str());
			if (!secondActor.actor)
				return nullptr;
		}
		default:
			break;
	}

	dsScenePhysicsConstraintReference secondConnectedConstraint = {};
	switch (fbConstraintNode->secondConnectedConstraint_type())
	{
		case DeepSeaScenePhysics::ConstraintReference::InstanceReference:
		{
			auto fbInstanceRef = fbConstraintNode->secondConnectedConstraint_as_InstanceReference();
			auto fbRootNode = fbInstanceRef->rootNode();
			if (fbRootNode)
			{
				secondActor.rootNode = findNode(scratchData, fbInstanceRef->rootNode()->c_str());
				if (!secondActor.rootNode)
					return nullptr;
			}
			secondConnectedConstraint.rigidBodyGroupNode = findRigidBodyGroupNode(
				scratchData, fbInstanceRef->rigidBodyGroupNode()->c_str());
			if (!secondConnectedConstraint.rigidBodyGroupNode)
				return nullptr;
			secondConnectedConstraint.instanceName = fbInstanceRef->instance()->c_str();
			break;
		}
		case DeepSeaScenePhysics::ConstraintReference::ConstraintNodeReference:
		{
			auto fbConstraintNodeRef =
				fbConstraintNode->secondConnectedConstraint_as_ConstraintNodeReference();
			secondConnectedConstraint.constraintNode = findConstraintNode(
				scratchData, fbConstraintNodeRef->constraintNode()->c_str());
			if (!secondConnectedConstraint.constraintNode)
				return nullptr;
		}
		case DeepSeaScenePhysics::ConstraintReference::ConstraintResourceReference:
		{
			auto fbConstraintResourceRef =
				fbConstraintNode->secondConnectedConstraint_as_ConstraintResourceReference();
			secondConnectedConstraint.constraint = findConstraint(
				scratchData, fbConstraintResourceRef->constraint()->c_str());
			if (!secondConnectedConstraint.constraint)
				return nullptr;
		}
		default:
			break;
	}

	auto fbItemLists = fbConstraintNode->itemLists();
	uint32_t itemListCount = fbItemLists ? fbItemLists->size() : 0U;
	const char** itemLists = NULL;
	if (itemListCount > 0)
	{
		itemLists = DS_ALLOCATE_STACK_OBJECT_ARRAY(const char*, itemListCount);
		for (uint32_t i = 0; i < itemListCount; ++i)
		{
			auto fbItemList = (*fbItemLists)[i];
			if (!fbItemList)
			{
				DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG,
					"Physics constraint node item list name is null.");
				errno = EFORMAT;
				return nullptr;
			}

			itemLists[i] = fbItemList->c_str();
		}
	}

	return reinterpret_cast<dsSceneNode*>(dsScenePhysicsConstraintNode_create(
		allocator, constraint, false, &firstActor, &firstConnectedConstraint, &secondActor,
		&secondConnectedConstraint, itemLists, itemListCount));
}

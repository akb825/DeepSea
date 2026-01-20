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

#include "SceneRigidBodyGroupNodeLoad.h"

#include <DeepSea/Core/Memory/StackAllocator.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Physics/Flatbuffers/PhysicsFlatbufferHelpers.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <DeepSea/ScenePhysics/ScenePhysicsConstraint.h>
#include <DeepSea/ScenePhysics/SceneRigidBodyGroupNode.h>
#include <DeepSea/ScenePhysics/SceneRigidBodyTemplate.h>

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#elif DS_MSC
#pragma warning(push)
#pragma warning(disable: 4244)
#endif

#include "Flatbuffers/SceneRigidBodyGroupNode_generated.h"

#if DS_GCC || DS_CLANG
#pragma GCC diagnostic pop
#elif DS_MSC
#pragma warning(pop)
#endif

dsSceneNode* dsSceneRigidBodyGroupNode_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void*, const uint8_t* data, size_t dataSize, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc)
{
	flatbuffers::Verifier verifier(data, dataSize);
	if (!DeepSeaScenePhysics::VerifyRigidBodyGroupNodeBuffer(verifier))
	{
		errno = EFORMAT;
		DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG, "Invalid rigid body group node flatbuffer format.");
		return nullptr;
	}

	auto fbRigidBodyGroupNode = DeepSeaScenePhysics::GetRigidBodyGroupNode(data);

	auto fbRigidBodyTemplates = fbRigidBodyGroupNode->rigidBodyTemplates();
	uint32_t rigidBodyCount = fbRigidBodyTemplates ? fbRigidBodyTemplates->size() : 0;
	dsNamedSceneRigidBodyTemplate* rigidBodyTemplates = NULL;
	if (rigidBodyCount > 0)
	{
		rigidBodyTemplates =  DS_ALLOCATE_STACK_OBJECT_ARRAY(
			dsNamedSceneRigidBodyTemplate, rigidBodyCount);
		for (uint32_t i = 0; i < rigidBodyCount; ++i)
		{
			auto fbRigidBodyTemplateName = (*fbRigidBodyTemplates)[i];
			if (!fbRigidBodyTemplateName)
			{
				DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG,
					"Rigid body group node rigid body template name is null.");
				errno = EFORMAT;
				return nullptr;
			}

			const char* rigidBodyTemplateName = fbRigidBodyTemplateName->c_str();
			dsSceneResourceType resourceType;
			dsCustomSceneResource* resource;
			if (!dsSceneLoadScratchData_findResource(&resourceType,
					reinterpret_cast<void**>(&resource), scratchData, rigidBodyTemplateName) ||
				resourceType != dsSceneResourceType_Custom ||
				resource->type != dsSceneRigidBodyTemplate_type())
			{
				DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG, "Couldn't find rigid body template '%s'.",
					rigidBodyTemplateName);
				errno = ENOTFOUND;
				return nullptr;
			}

			dsNamedSceneRigidBodyTemplate* rigidBodyTemplate = rigidBodyTemplates + i;
			rigidBodyTemplate->name = rigidBodyTemplateName;
			rigidBodyTemplate->rigidBodyTemplate =
				reinterpret_cast<dsRigidBodyTemplate*>(resource->resource);
			rigidBodyTemplate->transferOwnership = false;
		}
	}


	auto fbConstraints = fbRigidBodyGroupNode->constraints();
	uint32_t constraintCount = fbConstraints ? fbConstraints->size() : 0;
	dsNamedScenePhysicsConstraint* constraints = NULL;
	if (constraintCount > 0)
	{
		constraints =  DS_ALLOCATE_STACK_OBJECT_ARRAY(
			dsNamedScenePhysicsConstraint, constraintCount);
		for (uint32_t i = 0; i < constraintCount; ++i)
		{
			auto fbConstraintName = (*fbConstraints)[i];
			if (!fbConstraintName)
			{
				DS_LOG_ERROR(DS_SCENE_PHYSICS_LOG_TAG,
					"Rigid body group node constraint name is null.");
				errno = EFORMAT;
				return nullptr;
			}

			const char* constraintName = fbConstraintName->c_str();
			dsSceneResourceType resourceType;
			dsCustomSceneResource* resource;
			if (!dsSceneLoadScratchData_findResource(&resourceType,
					reinterpret_cast<void**>(&resource), scratchData, constraintName) ||
				resourceType != dsSceneResourceType_Custom ||
				resource->type != dsScenePhysicsConstraint_type())
			{
				DS_LOG_ERROR_F(DS_SCENE_PHYSICS_LOG_TAG, "Couldn't find physics constraint '%s'.",
					constraintName);
				errno = ENOTFOUND;
				return nullptr;
			}

			auto scenePhysicsConstraint =
				reinterpret_cast<dsScenePhysicsConstraint*>(resource->resource);

			dsNamedScenePhysicsConstraint* constraint = constraints + i;
			constraint->name = constraintName;
			constraint->constraint = scenePhysicsConstraint->constraint;
			constraint->firstRigidBody = scenePhysicsConstraint->firstRigidBodyInstance;
			constraint->firstConnectedConstraint =
				scenePhysicsConstraint->firstConnectedConstraintInstance;
			constraint->secondRigidBody = scenePhysicsConstraint->secondRigidBodyInstance;
			constraint->secondConnectedConstraint =
				scenePhysicsConstraint->secondConnectedConstraintInstance;
			constraint->transferOwnership = false;
		}
	}

	auto fbItemLists = fbRigidBodyGroupNode->itemLists();
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
					"Rigid body group node item list name is null.");
				errno = EFORMAT;
				return nullptr;
			}

			itemLists[i] = fbItemList->c_str();
		}
	}

	auto node = reinterpret_cast<dsSceneNode*>(dsSceneRigidBodyGroupNode_create(allocator,
		DeepSeaPhysics::convert(fbRigidBodyGroupNode->motionType()), rigidBodyTemplates,
		rigidBodyCount, constraints, constraintCount, itemLists, itemListCount));
	if (!node)
		return nullptr;

	auto fbChildren = fbRigidBodyGroupNode->children();
	if (fbChildren)
	{
		for (auto fbNode : *fbChildren)
		{
			if (!fbNode)
				continue;

			auto data = fbNode->data();
			dsSceneNode* child = dsSceneNode_load(allocator, resourceAllocator, loadContext,
				scratchData, fbNode->type()->c_str(), data->data(), data->size(),
				relativePathUserData, openRelativePathStreamFunc, closeRelativePathStreamFunc);
			if (!child)
			{
				dsSceneNode_freeRef(node);
				return nullptr;
			}

			bool success = dsSceneNode_addChild(node, child);
			dsSceneNode_freeRef(child);
			if (!success)
			{
				dsSceneNode_freeRef(node);
				return nullptr;
			}
		}
	}

	return node;
}

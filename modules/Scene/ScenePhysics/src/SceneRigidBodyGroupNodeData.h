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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Physics/Types.h>
#include <DeepSea/ScenePhysics/Types.h>

typedef struct RigidBodyNode
{
	dsHashTableNode node;
	uint32_t nameID;
	uint32_t index;
	bool owned;
	dsRigidBodyTemplate* rigidBody;
} RigidBodyNode;

typedef struct ConstraintNode
{
	dsHashTableNode node;
	uint32_t nameID;
	uint32_t index;
	bool owned;
	uint32_t firstRigidBodyID;
	uint32_t firstConnectedConstraintID;
	uint32_t secondRigidBodyID;
	uint32_t secondConnectedConstraintID;
	dsPhysicsConstraint* constraint;
} ConstraintNode;

struct dsSceneRigidBodyGroupNode
{
	dsSceneNode node;
	dsPhysicsMotionType motionType;
	uint32_t rigidBodyCount;
	uint32_t constraintCount;
	dsHashTable* rigidBodies;
	dsHashTable* constraints;
};

typedef struct dsSceneRigidBodyGroupNodeData
{
	dsAllocator* allocator;
	dsRigidBodyGroup* group;
	dsRigidBody** rigidBodies;
	dsPhysicsConstraint** constraints;
	uint32_t rigidBodyCount;
	uint32_t constraintCount;
} dsSceneRigidBodyGroupNodeData;

size_t dsSceneRigidBodyGroupNodeData_fullAllocSize(const dsSceneRigidBodyGroupNode* node);
dsSceneRigidBodyGroupNodeData* dsSceneRigidBodyGroupNodeData_create(dsAllocator* allocator,
	dsPhysicsEngine* physicsEngine, const dsSceneRigidBodyGroupNode* node, void* userData);
void dsSceneRigidBodyGroupNodeData_destroy(dsSceneRigidBodyGroupNodeData* data);

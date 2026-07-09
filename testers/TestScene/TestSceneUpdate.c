/*
 * Copyright 2026 Aaron Barany
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

#include "TestSceneUpdate.h"

#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Quaternion.h>

#include <DeepSea/Scene/ItemLists/SceneItemList.h>
#include <DeepSea/Scene/Nodes/SceneDynamicTransformNode.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <string.h>

#define PRIMARY_TRANSFORM_ID 0
#define SECONDARY_TRANSFORM_ID 1

typedef struct dsTestSceneUpdate
{
	dsSceneItemList itemList;

	dsSceneDynamicTransformNode* primaryTransform;
	dsSceneDynamicTransformNode* secondaryTransform;
	float rotation;
	bool paused;
} dsTestSceneUpdate;

static void setPrimaryRotation(dsSceneDynamicTransformNode* transformNode, float rotation)
{
	dsRigidTransform3f transform = transformNode->transform;
	dsQuaternion4f_fromEulerAngles(&transform.orientation, 0.0f, rotation, 0.0f);
	DS_VERIFY(dsSceneDynamicTransformNode_setTransform(transformNode, &transform));
}

static void setSecondaryRotation(dsSceneDynamicTransformNode* transformNode, float rotation)
{
	dsRigidTransform3f transform = transformNode->transform;
	dsQuaternion4f_fromEulerAngles(&transform.orientation, 0.0f, -2.0f*rotation, 0.0f);
	DS_VERIFY(dsSceneDynamicTransformNode_setTransform(transformNode, &transform));
}

static void dsTestSceneUpdate_preTransformUpdate(
	dsSceneItemList* itemList, const dsScene* scene, const dsSceneTick* tick, unsigned int step)
{
	DS_ASSERT(itemList);
	DS_ASSERT(tick);
	DS_UNUSED(scene);
	DS_UNUSED(step);

	dsTestSceneUpdate* update = (dsTestSceneUpdate*)itemList;
	if (update->paused || tick->stepTime == 0.0f)
		return;

	// radians/s
	const float rate = M_PI_2f;
	update->rotation += tick->stepTime*rate;
	while (update->rotation > 2*M_PIf)
		update->rotation = update->rotation - 2*M_PIf;

	if (update->primaryTransform)
		setPrimaryRotation(update->primaryTransform, update->rotation);
	if (update->secondaryTransform)
		setSecondaryRotation(update->secondaryTransform, update->rotation);
}

static void dsTestSceneUpdate_destroy(dsSceneItemList* itemList)
{
	if (itemList->allocator)
		DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

static dsSceneItemListType itemListType =
{
	.preTransformUpdateFunc = &dsTestSceneUpdate_preTransformUpdate,
	.destroyFunc = &dsTestSceneUpdate_destroy
};

static dsSceneItemList* dsTestSceneUpdate_create(dsAllocator* allocator, const char* name,
	dsSceneDynamicTransformNode* primaryTransform, dsSceneDynamicTransformNode* secondaryTransform)
{
	if (!allocator || !name || !primaryTransform || !secondaryTransform)
	{
		errno = EINVAL;
		return NULL;
	}

	size_t nameLen = strlen(name) + 1;
	size_t fullSize =
		DS_ALIGNED_SIZE(sizeof(dsTestSceneUpdate)) + DS_ALIGNED_SIZE(nameLen);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsTestSceneUpdate* update = DS_ALLOCATE_OBJECT(&bufferAlloc, dsTestSceneUpdate);
	DS_ASSERT(update);

	dsSceneItemList* itemList = (dsSceneItemList*)update;
	itemList->allocator = allocator;
	itemList->type = &itemListType;
	itemList->viewFilter = NULL;
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsUniqueNameID_create(name);
	itemList->globalValueCount = 0;
	itemList->needsCommandBuffer = false;
	itemList->skipPreRenderPass = false;

	update->primaryTransform = primaryTransform;
	update->secondaryTransform = secondaryTransform;
	update->rotation = 0.0f;
	update->paused = false;

	setPrimaryRotation(primaryTransform, update->rotation);
	setSecondaryRotation(secondaryTransform, update->rotation);

	return itemList;
}

dsSceneItemList* dsTestSceneUpdate_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const char* name, const uint8_t* data, size_t dataSize)
{
	DS_UNUSED(loadContext);
	DS_UNUSED(resourceAllocator);
	DS_UNUSED(userData);
	DS_UNUSED(data);
	DS_UNUSED(dataSize);

	dsSceneDynamicTransformNode* primaryTransform;
	dsSceneDynamicTransformNode* secondaryTransform;
	dsSceneResourceType resourceType;
	const dsSceneNodeType* transformNodeType = dsSceneDynamicTransformNode_type();
	if (!dsSceneLoadScratchData_findResource(
			&resourceType, (void**)&primaryTransform, scratchData, "primaryTransform") ||
		resourceType != dsSceneResourceType_SceneNode ||
		!dsSceneNode_isOfType((const dsSceneNode*)primaryTransform, transformNodeType))
	{
		DS_LOG_ERROR("TestScene", "Couldn't find dynamic transform node 'primaryTransform'.");
		return NULL;
	}

	if (!dsSceneLoadScratchData_findResource(
			&resourceType, (void**)&secondaryTransform, scratchData, "secondaryTransform") ||
		resourceType != dsSceneResourceType_SceneNode ||
		!dsSceneNode_isOfType((const dsSceneNode*)secondaryTransform, transformNodeType))
	{
		DS_LOG_ERROR("TestScene", "Couldn't find dynamic transform node 'secondaryTransform'.");
		return NULL;
	}

	return dsTestSceneUpdate_create(allocator, name, primaryTransform, secondaryTransform);
}

void dsTestSceneUpdate_togglePaused(dsSceneItemList* itemList)
{
	if (!itemList || itemList->type != &itemListType)
		return;

	dsTestSceneUpdate* update = (dsTestSceneUpdate*)itemList;
	update->paused = !update->paused;
}

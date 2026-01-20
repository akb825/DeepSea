/*
 * Copyright 2023-2026 Aaron Barany
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

#include <DeepSea/SceneAnimation/SceneAnimationLoadContext.h>

#include "SceneAnimationNodeLoad.h"
#include "SceneAnimationTransformNodeLoad.h"
#include "SceneAnimationTreeNodeLoad.h"

#include <DeepSea/Animation/AnimationNodeMapCache.h>
#include <DeepSea/Animation/AnimationTree.h>
#include <DeepSea/Animation/DirectAnimation.h>
#include <DeepSea/Animation/KeyframeAnimation.h>

#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <DeepSea/SceneAnimation/SceneAnimationNode.h>
#include <DeepSea/SceneAnimation/SceneAnimationList.h>
#include <DeepSea/SceneAnimation/SceneAnimationNodeMapCache.h>
#include <DeepSea/SceneAnimation/SceneAnimationTransformNode.h>
#include <DeepSea/SceneAnimation/SceneAnimationTree.h>
#include <DeepSea/SceneAnimation/SceneAnimationTreeNode.h>
#include <DeepSea/SceneAnimation/SceneDirectAnimation.h>
#include <DeepSea/SceneAnimation/SceneKeyframeAnimation.h>
#include <DeepSea/SceneAnimation/SceneSkinningData.h>

static dsSceneItemList* dsSceneAnimationList_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const char* name, const uint8_t* data, size_t dataSize)
{
	DS_UNUSED(loadContext);
	DS_UNUSED(scratchData);
	DS_UNUSED(resourceAllocator);
	DS_UNUSED(userData);
	DS_UNUSED(data);
	DS_UNUSED(dataSize);

	return (dsSceneItemList*)dsSceneAnimationList_create(allocator, name);
}

static dsSceneInstanceData* dsSceneSkinningData_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize)
{
	DS_UNUSED(scratchData);
	DS_UNUSED(userData);
	DS_UNUSED(data);
	DS_UNUSED(dataSize);

	return dsSceneSkinningData_create(allocator, resourceAllocator,
		dsSceneLoadContext_getRenderer(loadContext)->resourceManager);
}

static void* dsSceneAnimationTree_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc)
{
	DS_UNUSED(loadContext);
	DS_UNUSED(resourceAllocator);
	DS_UNUSED(userData);
	DS_UNUSED(relativePathUserData);
	DS_UNUSED(openRelativePathStreamFunc);
	DS_UNUSED(closeRelativePathStreamFunc);

	return dsAnimationTree_loadData(allocator, dsSceneLoadScratchData_getAllocator(scratchData),
		data, dataSize);
}

static bool dsSceneAnimationTree_destroyResource(void* resource)
{
	dsAnimationTree_destroy((dsAnimationTree*)resource);
	return true;
}

static void* dsSceneAnimationNodeMapCache_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc)
{
	DS_UNUSED(loadContext);
	DS_UNUSED(scratchData);
	DS_UNUSED(resourceAllocator);
	DS_UNUSED(userData);
	DS_UNUSED(data);
	DS_UNUSED(dataSize);
	DS_UNUSED(relativePathUserData);
	DS_UNUSED(openRelativePathStreamFunc);
	DS_UNUSED(closeRelativePathStreamFunc);

	return dsAnimationNodeMapCache_create(allocator);
}

static bool dsSceneAnimationNodeMapCache_destroyResource(void* resource)
{
	dsAnimationNodeMapCache_destroy((dsAnimationNodeMapCache*)resource);
	return true;
}

static void* dsSceneDirectAnimation_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc)
{
	DS_UNUSED(loadContext);
	DS_UNUSED(resourceAllocator);
	DS_UNUSED(userData);
	DS_UNUSED(relativePathUserData);
	DS_UNUSED(openRelativePathStreamFunc);
	DS_UNUSED(closeRelativePathStreamFunc);

	return dsDirectAnimation_loadData(allocator, dsSceneLoadScratchData_getAllocator(scratchData),
		data, dataSize);
}

static bool dsSceneDirectAnimation_destroyResource(void* resource)
{
	dsDirectAnimation_destroy((dsDirectAnimation*)resource);
	return true;
}

static void* dsSceneKeyframeAnimation_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize, void* relativePathUserData,
	dsOpenRelativePathStreamFunction openRelativePathStreamFunc,
	dsCloseRelativePathStreamFunction closeRelativePathStreamFunc)
{
	DS_UNUSED(loadContext);
	DS_UNUSED(resourceAllocator);
	DS_UNUSED(userData);
	DS_UNUSED(relativePathUserData);
	DS_UNUSED(openRelativePathStreamFunc);
	DS_UNUSED(closeRelativePathStreamFunc);

	return dsKeyframeAnimation_loadData(allocator, dsSceneLoadScratchData_getAllocator(scratchData),
		data, dataSize);
}

static bool dsSceneKeyframeAnimation_destroyResource(void* resource)
{
	dsKeyframeAnimation_destroy((dsKeyframeAnimation*)resource);
	return true;
}

bool dsSceneAnimationLoadConext_registerTypes(dsSceneLoadContext* loadContext)
{
	if (!dsSceneLoadContext_registerNodeType(loadContext, dsSceneAnimationNode_typeName,
			&dsSceneAnimationNode_load, NULL, NULL))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerNodeType(loadContext, dsSceneAnimationTransformNode_typeName,
			&dsSceneAnimationTransformNode_load, NULL, NULL))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerNodeType(loadContext, dsSceneAnimationTreeNode_typeName,
			&dsSceneAnimationTreeNode_load, NULL, NULL))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerItemListType(loadContext, dsSceneAnimationList_typeName,
			&dsSceneAnimationList_load, NULL, NULL))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerInstanceDataType(loadContext, dsSceneSkinningData_typeName,
			&dsSceneSkinningData_load, NULL, NULL))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerCustomResourceType(loadContext, dsSceneAnimationTree_typeName,
			dsSceneAnimationTree_type(), &dsSceneAnimationTree_load,
			&dsSceneAnimationTree_destroyResource, NULL, NULL, 0))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerCustomResourceType(loadContext,
			dsSceneAnimationNodeMapCache_typeName, dsSceneAnimationNodeMapCache_type(),
			&dsSceneAnimationNodeMapCache_load, &dsSceneAnimationNodeMapCache_destroyResource, NULL,
			NULL, 0))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerCustomResourceType(loadContext, dsSceneDirectAnimation_typeName,
			dsSceneDirectAnimation_type(), &dsSceneDirectAnimation_load,
			&dsSceneDirectAnimation_destroyResource, NULL, NULL, 0))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerCustomResourceType(loadContext,
			dsSceneKeyframeAnimation_typeName, dsSceneKeyframeAnimation_type(),
			&dsSceneKeyframeAnimation_load, &dsSceneKeyframeAnimation_destroyResource, NULL, NULL,
			0))
	{
		return false;
	}

	return true;
}

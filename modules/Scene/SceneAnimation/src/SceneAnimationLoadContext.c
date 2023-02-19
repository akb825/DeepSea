#include <DeepSea/SceneAnimation/SceneAnimationLoadContext.h>

#include <DeepSea/Animation/AnimationTree.h>
#include <DeepSea/Animation/DirectAnimation.h>
#include <DeepSea/Animation/KeyframeAnimation.h>

#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>

#include <DeepSea/SceneAnimation/SceneAnimationTree.h>
#include <DeepSea/SceneAnimation/SceneDirectAnimation.h>
#include <DeepSea/SceneAnimation/SceneKeyframeAnimation.h>

static void* dsSceneAnimationTree_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize)
{
	dsAnimationTree* animationTree = dsAnimationTree_loadData(allocator,
		dsSceneLoadScratchData_getAllocator(scratchData), data, dataSize);
	if (!animationTree)
		return NULL;

	return dsSceneAnimationTree_create(allocator, animationTree);
}

static bool dsSceneAnimationTree_destroyResource(void* resource)
{
	dsSceneAnimationTree_destroy((dsSceneAnimationTree*)resource);
	return true;
}

static void* dsSceneDirectAnimation_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize)
{
	return dsDirectAnimation_loadData(allocator, dsSceneLoadScratchData_getAllocator(scratchData),
		data, dataSize);
}

static void* dsSceneKeyframeAnimation_load(const dsSceneLoadContext* loadContext,
	dsSceneLoadScratchData* scratchData, dsAllocator* allocator, dsAllocator* resourceAllocator,
	void* userData, const uint8_t* data, size_t dataSize)
{
	return dsKeyframeAnimation_loadData(allocator, dsSceneLoadScratchData_getAllocator(scratchData),
		data, dataSize);
}

bool dsSceneAnimationLoadConext_registerTypes(dsSceneLoadContext* loadContext)
{
	if (!dsSceneLoadContext_registerCustomResourceType(loadContext, dsSceneAnimationTree_typeName,
			dsSceneAnimationTree_type(), &dsSceneAnimationTree_load,
			&dsSceneAnimationTree_destroyResource, NULL, NULL, 0))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerCustomResourceType(loadContext, dsSceneDirectAnimation_typeName,
			dsSceneDirectAnimation_type(), &dsSceneDirectAnimation_load,
			&dsSceneDirectAnimation_destroy, NULL, NULL, 0))
	{
		return false;
	}

	if (!dsSceneLoadContext_registerCustomResourceType(loadContext,
			dsSceneKeyframeAnimation_typeName, dsSceneKeyframeAnimation_type(),
			&dsSceneKeyframeAnimation_load, &dsSceneKeyframeAnimation_destroy, NULL, NULL, 0))
	{
		return false;
	}

	return true;
}

/*
 * Copyright 2019-2023 Aaron Barany
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

#include <DeepSea/Scene/ViewTransformData.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/RenderSurface.h>

#include <DeepSea/Scene/View.h>

#include <string.h>

static dsShaderVariableElement elements[] =
{
	{"view", dsMaterialType_Mat4, 0},
	{"camera", dsMaterialType_Mat4, 0},
	{"projection", dsMaterialType_Mat4, 0},
	{"viewProjection", dsMaterialType_Mat4, 0},
	{"projectionInv", dsMaterialType_Mat4, 0},
	{"screenRotation", dsMaterialType_Vec4, 0},
	{"clipSpaceTexCoordTransform", dsMaterialType_Vec3, 2},
	{"screenSize", dsMaterialType_IVec2, 0}
};

typedef struct dsViewTransformData
{
	dsSceneItemList itemList;
	dsShaderVariableGroup* variableGroup;
	uint32_t nameID;
} dsViewTransformData;

static void dsViewTransformData_commit(dsSceneItemList* itemList, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	dsViewTransformData* viewData = (dsViewTransformData*)itemList;
	dsRenderer* renderer = commandBuffer->renderer;
	unsigned int i = 0;
	DS_VERIFY(dsShaderVariableGroup_setElementData(viewData->variableGroup, i++, &view->viewMatrix,
		dsMaterialType_Mat4, 0, 1));
	DS_VERIFY(dsShaderVariableGroup_setElementData(viewData->variableGroup, i++,
		&view->cameraMatrix, dsMaterialType_Mat4, 0, 1));
	DS_VERIFY(dsShaderVariableGroup_setElementData(viewData->variableGroup, i++,
		&view->projectionMatrix, dsMaterialType_Mat4, 0, 1));
	DS_VERIFY(dsShaderVariableGroup_setElementData(viewData->variableGroup, i++,
		&view->viewProjectionMatrix, dsMaterialType_Mat4, 0, 1));

	dsMatrix44f projectionInv;
	dsMatrix44f_invert(&projectionInv, &view->projectionMatrix);
	DS_VERIFY(dsShaderVariableGroup_setElementData(viewData->variableGroup, i++, &projectionInv,
		dsMaterialType_Mat4, 0, 1));

	dsMatrix22f screenRotation;
	DS_VERIFY(dsRenderSurface_makeRotationMatrix22(&screenRotation, view->rotation));
	DS_VERIFY(dsShaderVariableGroup_setElementData(viewData->variableGroup, i++, &screenRotation,
		dsMaterialType_Vec4, 0, 1));

	int halfDepth = renderer->projectionOptions & dsProjectionMatrixOptions_HalfZRange;
	dsVector3f texCoordTransform[2] =
	{
		{{0.5f, renderer->projectedTexCoordTInverted ? -0.5f : 0.5f, halfDepth ? 1.0f : 0.5f}},
		{{0.5f, 0.5f, halfDepth ? 0.0f : 0.5f}}
	};
	DS_VERIFY(dsShaderVariableGroup_setElementData(viewData->variableGroup, i++, texCoordTransform,
		dsMaterialType_Vec3, 0, 2));

	dsVector2i screenSize;
	if (view->rotation == dsRenderSurfaceRotation_0 ||
		view->rotation == dsRenderSurfaceRotation_180)
	{
		screenSize.x = view->width;
		screenSize.y = view->height;
	}
	else
	{
		screenSize.x = view->height;
		screenSize.y = view->width;
	}
	DS_VERIFY(dsShaderVariableGroup_setElementData(viewData->variableGroup, i++, &screenSize,
		dsMaterialType_IVec2, 0, 1));

	if (DS_CHECK(DS_SCENE_LOG_TAG,
			dsShaderVariableGroup_commit(viewData->variableGroup, commandBuffer)))
	{
		dsSharedMaterialValues* globalValues = dsView_lockGlobalValues(view, itemList);
		DS_ASSERT(globalValues);
		DS_VERIFY(dsSharedMaterialValues_setVariableGroupID(globalValues, viewData->nameID,
			viewData->variableGroup));
		DS_VERIFY(dsView_unlockGlobalValues(view, itemList));
	}
}

static void dsViewTransformData_destroy(dsSceneItemList* itemList)
{
	dsViewTransformData* viewData = (dsViewTransformData*)itemList;
	DS_CHECK(DS_SCENE_LOG_TAG, dsShaderVariableGroup_destroy(viewData->variableGroup));

	if (itemList->allocator)
		DS_VERIFY(dsAllocator_free(itemList->allocator, itemList));
}

const char* const dsViewTransformData_typeName = "ViewTransformData";

dsSceneItemListType dsViewTransformData_type(void)
{
	static int type;
	return &type;
}

dsShaderVariableGroupDesc* dsViewTransformData_createShaderVariableGroupDesc(
	dsResourceManager* resourceManager, dsAllocator* allocator)
{
	if (!resourceManager)
	{
		errno = EINVAL;
		return NULL;
	}

	return dsShaderVariableGroupDesc_create(resourceManager, allocator, elements,
		DS_ARRAY_SIZE(elements));
}

dsSceneItemList* dsViewTransformData_create(dsAllocator* allocator, const char* name,
	dsResourceManager* resourceManager, const dsShaderVariableGroupDesc* transformDesc)
{
	if (!allocator || !name || !transformDesc)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!dsShaderVariableGroup_areElementsEqual(elements, DS_ARRAY_SIZE(elements),
			transformDesc->elements, transformDesc->elementCount))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG,
			"View transform data's shader variable group description must have been created "
			"with dsViewTransformData_createShaderVariableGroupDesc().");
		return NULL;
	}

	size_t nameLen = strlen(name) + 1;
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsViewTransformData)) + DS_ALIGNED_SIZE(nameLen) +
		dsShaderVariableGroup_fullAllocSize(resourceManager, transformDesc);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsViewTransformData* viewData = DS_ALLOCATE_OBJECT(&bufferAlloc, dsViewTransformData);
	DS_VERIFY(viewData);

	dsSceneItemList* itemList = (dsSceneItemList*)viewData;
	itemList->allocator = dsAllocator_keepPointer(allocator);
	itemList->type = dsViewTransformData_type();
	itemList->name = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(itemList->name);
	memcpy((void*)itemList->name, name, nameLen);
	itemList->nameID = dsHashString(name);
	itemList->globalValueCount = 1;
	itemList->needsCommandBuffer = true;
	itemList->addNodeFunc = NULL;
	itemList->updateNodeFunc = NULL;
	itemList->removeNodeFunc = NULL;
	itemList->preTransformUpdateFunc = NULL;
	itemList->updateFunc = NULL;
	itemList->preRenderPassFunc = NULL;
	itemList->commitFunc = &dsViewTransformData_commit;
	itemList->destroyFunc = &dsViewTransformData_destroy;

	viewData->variableGroup = dsShaderVariableGroup_create(resourceManager,
		(dsAllocator*)&bufferAlloc, allocator, transformDesc);
	if (!viewData->variableGroup)
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, buffer));
		return NULL;
	}

	viewData->nameID = dsHashString(dsViewTransformData_typeName);

	return itemList;
}


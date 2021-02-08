/*
 * Copyright 2019 Aaron Barany
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
#include <DeepSea/Scene/SceneGlobalData.h>
#include <string.h>

static dsShaderVariableElement elements[] =
{
	{"view", dsMaterialType_Mat4, 0},
	{"camera", dsMaterialType_Mat4, 0},
	{"projection", dsMaterialType_Mat4, 0},
	{"viewProjection", dsMaterialType_Mat4, 0},
	{"projectionInv", dsMaterialType_Mat4, 0},
	{"screenRotation", dsMaterialType_Vec4, 0},
	{"screenSize", dsMaterialType_IVec2, 0}
};

const char* const dsViewTransformData_typeName = "ViewTransformData";

typedef struct dsViewTransformData
{
	dsSceneGlobalData globalData;
	dsShaderVariableGroup* variableGroup;
	uint32_t nameID;
} dsViewTransformData;

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

bool dsViewTransformData_populateData(dsSceneGlobalData* globalData, const dsView* view,
	dsCommandBuffer* commandBuffer)
{
	dsViewTransformData* viewData = (dsViewTransformData*)globalData;
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

	dsVector2i screenSize = {{view->width, view->height}};
	DS_VERIFY(dsShaderVariableGroup_setElementData(viewData->variableGroup, i++, &screenSize,
		dsMaterialType_IVec2, 0, 1));

	if (!dsShaderVariableGroup_commit(viewData->variableGroup, commandBuffer))
		return false;

	DS_VERIFY(dsSharedMaterialValues_setVariableGroupID(view->globalValues, viewData->nameID,
		viewData->variableGroup));
	return true;
}

bool dsViewTransformData_destroy(dsSceneGlobalData* globalData)
{
	dsViewTransformData* viewData = (dsViewTransformData*)globalData;
	if (!dsShaderVariableGroup_destroy(viewData->variableGroup))
		return false;

	if (globalData->allocator)
		DS_VERIFY(dsAllocator_free(globalData->allocator, globalData));
	return true;
}

dsSceneGlobalData* dsViewTransformData_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, const dsShaderVariableGroupDesc* transformDesc)
{
	if (!allocator || !transformDesc)
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

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsViewTransformData)) +
		dsShaderVariableGroup_fullAllocSize(resourceManager, transformDesc);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsViewTransformData* viewData = DS_ALLOCATE_OBJECT(&bufferAlloc, dsViewTransformData);
	DS_VERIFY(viewData);
	dsSceneGlobalData* globalData = (dsSceneGlobalData*)viewData;

	globalData->allocator = dsAllocator_keepPointer(allocator);
	globalData->valueCount = 1;
	globalData->populateDataFunc = &dsViewTransformData_populateData;
	globalData->finishFunc = NULL;
	globalData->destroyFunc = &dsViewTransformData_destroy;

	viewData->variableGroup = dsShaderVariableGroup_create(resourceManager,
		(dsAllocator*)&bufferAlloc, allocator, transformDesc);
	if (!viewData->variableGroup)
	{
		if (allocator->freeFunc)
			DS_VERIFY(dsAllocator_free(allocator, buffer));
		return NULL;
	}

	viewData->nameID = dsHashString(dsViewTransformData_typeName);

	return globalData;
}


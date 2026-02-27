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

#include <DeepSea/Scene/ItemLists/ViewFramebufferData.h>

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Core/UniqueNameID.h>

#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/SharedMaterialValues.h>
#include <DeepSea/Render/Resources/StreamingGfxBufferList.h>
#include <DeepSea/Render/RenderSurface.h>

typedef struct BufferInfo
{
	dsGfxBuffer* buffer;
	uint64_t lastUsedFrame;
} BufferInfo;

typedef struct dsViewFramebufferData
{
	dsSceneInstanceData instanceData;

	dsAllocator* resourceAllocator;
	dsResourceManager* resourceManager;
	const dsShaderVariableGroupDesc* dataDesc;

	BufferInfo* buffers;
	uint32_t bufferCount;
	uint32_t maxBuffers;

	uint32_t bufferSize;
	uint32_t nameID;

	dsGfxBuffer* curBuffer;
	dsShaderVariableGroup* fallback;
} dsViewFramebufferData;

static dsShaderVariableElement elements[] =
{
	{"framebufferSize", dsMaterialType_IVec4},
	{"framebufferRotation", dsMaterialType_Vec4}
};

typedef struct ViewFramebuffer
{
	dsVector4i framebufferSize;
	dsVector4f framebufferRotation;
} ViewFramebuffer;

static bool dsViewFramebufferData_populateData(dsSceneInstanceData* instanceData,
	const dsView* view, dsCommandBuffer* commandBuffer,
	const dsViewRenderPassParams* renderPassParams, const dsSceneTreeNode* const* instances,
	uint32_t instanceCount)
{
	DS_UNUSED(view);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(renderPassParams);
	DS_UNUSED(instances);
	DS_UNUSED(instanceCount);
	dsViewFramebufferData* framebufferData = (dsViewFramebufferData*)instanceData;
	DS_PROFILE_FUNC_START();

	dsVector4i framebufferSize;
	dsVector4f framebufferRotation;
	DS_ASSERT(renderPassParams);
	if (renderPassParams->rotation == dsRenderSurfaceRotation_0 ||
		renderPassParams->rotation == dsRenderSurfaceRotation_180)
	{
		framebufferSize.x = renderPassParams->framebufferWidth;
		framebufferSize.y = renderPassParams->framebufferHeight;
	}
	else
	{
		framebufferSize.x = renderPassParams->framebufferHeight;
		framebufferSize.y = renderPassParams->framebufferWidth;
	}
	framebufferSize.z = renderPassParams->framebufferWidth;
	framebufferSize.w = renderPassParams->framebufferHeight;

	DS_VERIFY(dsRenderSurface_makeRotationMatrix22(
		(dsMatrix22f*)&framebufferRotation, renderPassParams->rotation));

	if (framebufferData->fallback)
	{
		uint32_t i = 0;
		DS_VERIFY(dsShaderVariableGroup_setElementData(
			framebufferData->fallback, i++, &framebufferSize, dsMaterialType_IVec4, 0, 1));
		DS_VERIFY(dsShaderVariableGroup_setElementData(
			framebufferData->fallback, i++, &framebufferRotation, dsMaterialType_Vec4, 0, 1));
		DS_VERIFY(dsShaderVariableGroup_commitWithoutBuffer(framebufferData->fallback));
		DS_PROFILE_FUNC_RETURN(true);
	}

	if (framebufferData->curBuffer)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG, "Attempting to populate view framebuffer data before "
			"calling dsSceneInstanceData_finish() for the last usage.");
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = framebufferData->resourceManager;
	uint64_t frameNumber = resourceManager->renderer->frameNumber;
	uint32_t bufferSize = framebufferData->bufferSize;

	// Look for an existing buffer we can re-use.
	uint32_t index = dsStreamingGfxBufferList_findNext(framebufferData->buffers,
		&framebufferData->bufferCount, sizeof(BufferInfo), offsetof(BufferInfo, buffer),
		offsetof(BufferInfo, lastUsedFrame), NULL, bufferSize,
		DS_DEFAULT_STREAMING_GFX_BUFFER_FRAME_DELAY, frameNumber);
	if (index == DS_NO_STREAMING_GFX_BUFFER)
	{
		// Not found: need to add a new buffer.
		index = framebufferData->bufferCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(instanceData->allocator, framebufferData->buffers,
				framebufferData->bufferCount, framebufferData->maxBuffers, 1))
		{
			DS_PROFILE_FUNC_RETURN(false);
		}

		dsGfxMemory memoryHints = dsGfxMemory_Stream | dsGfxMemory_Synchronize;
		BufferInfo* bufferInfo = framebufferData->buffers + index;
		bufferInfo->buffer = dsGfxBuffer_create(resourceManager, framebufferData->resourceAllocator,
			dsGfxBufferUsage_UniformBlock, memoryHints, NULL, bufferSize);
		if (!bufferInfo->buffer)
		{
			--framebufferData->bufferCount;
			DS_PROFILE_FUNC_RETURN(false);
		}

		bufferInfo->lastUsedFrame = frameNumber;
		framebufferData->curBuffer = bufferInfo->buffer;
	}
	else
		framebufferData->curBuffer = framebufferData->buffers[index].buffer;

	DS_ASSERT(framebufferData->curBuffer);
	ViewFramebuffer* viewFramebuffer = (ViewFramebuffer*)dsGfxBuffer_map(
		framebufferData->curBuffer, dsGfxBufferMap_Write, 0, DS_MAP_FULL_BUFFER);
	if (!viewFramebuffer)
		DS_PROFILE_FUNC_RETURN(false);

	viewFramebuffer->framebufferSize = framebufferSize;
	viewFramebuffer->framebufferRotation = framebufferRotation;

	DS_VERIFY(dsGfxBuffer_unmap(framebufferData->curBuffer));
	DS_PROFILE_FUNC_RETURN(true);
}

static bool dsViewFramebufferData_bindInstance(
	dsSceneInstanceData* instanceData, uint32_t index, dsSharedMaterialValues* values)
{
	DS_UNUSED(index);
	dsViewFramebufferData* framebufferData = (dsViewFramebufferData*)instanceData;
	DS_ASSERT(framebufferData);
	DS_ASSERT(values);

	if (framebufferData->fallback)
	{
		return dsSharedMaterialValues_setVariableGroupID(
			values, framebufferData->nameID, framebufferData->fallback);
	}

	return dsSharedMaterialValues_setBufferID(
		values, framebufferData->nameID, framebufferData->curBuffer, 0, sizeof(ViewFramebuffer));
}

static bool dsViewFramebufferData_finish(dsSceneInstanceData* instanceData)
{
	dsViewFramebufferData* framebufferData = (dsViewFramebufferData*)instanceData;
	DS_ASSERT(framebufferData);
	framebufferData->curBuffer = NULL;
	return true;
}

static uint32_t dsViewFramebufferData_hash(
	const dsSceneInstanceData* instanceData, uint32_t commonHash)
{
	dsViewFramebufferData* framebufferData = (dsViewFramebufferData*)instanceData;
	DS_ASSERT(framebufferData);

	return dsHashCombinePointer(commonHash, framebufferData->dataDesc);
}

static bool dsViewFramebufferData_equal(
	const dsSceneInstanceData* left, const dsSceneInstanceData* right)
{
	DS_ASSERT(left);
	DS_ASSERT(left->type == dsViewFramebufferData_type());
	DS_ASSERT(right);
	DS_ASSERT(right->type == dsViewFramebufferData_type());

	const dsViewFramebufferData* leftFramebufferData = (const dsViewFramebufferData*)left;
	const dsViewFramebufferData* rightFramebufferData = (const dsViewFramebufferData*)right;
	return leftFramebufferData->dataDesc == rightFramebufferData->dataDesc;
}

static bool dsViewFramebufferData_destroy(dsSceneInstanceData* instanceData)
{
	dsViewFramebufferData* framebufferData = (dsViewFramebufferData*)instanceData;
	DS_ASSERT(framebufferData);

	for (uint32_t i = 0; i < framebufferData->bufferCount; ++i)
	{
		if (!dsGfxBuffer_destroy(framebufferData->buffers[i].buffer))
		{
			DS_ASSERT(i == 0);
			return false;
		}
	}

	if (!dsShaderVariableGroup_destroy(framebufferData->fallback))
		return false;

	DS_VERIFY(dsAllocator_free(instanceData->allocator, framebufferData->buffers));
	DS_VERIFY(dsAllocator_free(instanceData->allocator, framebufferData));
	return true;
}

const char* const dsViewFramebufferData_typeName = "ViewFramebufferData";
const char* const dsViewFramebufferData_uniformName = "dsViewFramebufferData";

static dsSceneInstanceDataType instanceDataType =
{
	&dsViewFramebufferData_populateData,
	&dsViewFramebufferData_bindInstance,
	&dsViewFramebufferData_finish,
	&dsViewFramebufferData_hash,
	&dsViewFramebufferData_equal,
	&dsViewFramebufferData_destroy
};

dsShaderVariableGroupDesc* dsViewFramebufferData_createShaderVariableGroupDesc(
	dsResourceManager* resourceManager, dsAllocator* allocator)
{
	if (!resourceManager)
	{
		errno = EINVAL;
		return NULL;
	}

	return dsShaderVariableGroupDesc_create(
		resourceManager, allocator, elements, DS_ARRAY_SIZE(elements));
}

bool dsViewFramebufferData_isShaderVariableGroupCompatible(
	const dsShaderVariableGroupDesc* transformDesc)
{
	return transformDesc &&
		dsShaderVariableGroup_areElementsEqual(elements, DS_ARRAY_SIZE(elements),
			transformDesc->elements, transformDesc->elementCount);
}

const dsSceneInstanceDataType* dsViewFramebufferData_type(void)
{
	return &instanceDataType;
}

dsSceneInstanceData* dsViewFramebufferData_create(dsAllocator* allocator,
	dsResourceManager* resourceManager, dsAllocator* resourceAllocator,
	const dsShaderVariableGroupDesc* dataDesc)
{
	if (!allocator || !resourceManager || !dataDesc)
	{
		errno = EINVAL;
		return NULL;
	}

	if (!allocator->freeFunc)
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG,
			"View framebuffer data allocator must support freeing memory.");
		return NULL;
	}

	if (!resourceAllocator)
		resourceAllocator = allocator;

	if (!dsViewFramebufferData_isShaderVariableGroupCompatible(dataDesc))
	{
		errno = EINVAL;
		DS_LOG_ERROR(DS_SCENE_LOG_TAG,
			"View framebuffer data's shader variable group description must have been created "
			"with dsViewFramebufferData_createShaderVariableGroupDesc().");
		return NULL;
	}

	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsViewFramebufferData));
	bool needsFallback = !dsShaderVariableGroup_useGfxBuffer(resourceManager);
	if (needsFallback)
		fullSize += dsShaderVariableGroup_fullAllocSize(resourceManager, dataDesc);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));

	dsViewFramebufferData* framebufferData =
		DS_ALLOCATE_OBJECT(&bufferAlloc, dsViewFramebufferData);
	DS_ASSERT(framebufferData);

	dsSceneInstanceData* instanceData = (dsSceneInstanceData*)framebufferData;
	instanceData->allocator = allocator;
	instanceData->type = dsViewFramebufferData_type();
	instanceData->valueCount = 1;
	instanceData->needsCommandBuffer = false;

	framebufferData->resourceAllocator = resourceAllocator;
	framebufferData->resourceManager = resourceManager;
	framebufferData->dataDesc = dataDesc;
	framebufferData->buffers = NULL;
	framebufferData->bufferCount = 0;
	framebufferData->maxBuffers = 0;

	framebufferData->bufferSize = (uint32_t)sizeof(ViewFramebuffer);
	if (resourceManager->minUniformBlockAlignment > 0)
	{
		framebufferData->bufferSize = DS_CUSTOM_ALIGNED_SIZE(
			framebufferData->bufferSize, resourceManager->minUniformBlockAlignment);
	}
	framebufferData->nameID = dsUniqueNameID_create(dsViewFramebufferData_uniformName);

	framebufferData->curBuffer = NULL;
	if (needsFallback)
	{
		framebufferData->fallback = dsShaderVariableGroup_create(
			resourceManager, (dsAllocator*)&bufferAlloc, NULL, dataDesc);
		DS_ASSERT(framebufferData->fallback);
	}
	else
		framebufferData->fallback = NULL;

	return instanceData;
}

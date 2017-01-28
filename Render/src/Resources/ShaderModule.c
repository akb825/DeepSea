/*
 * Copyright 2017 Aaron Barany
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

#include <DeepSea/Render/Resources/ShaderModule.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Types.h>

#include <MSL/Client/ModuleC.h>

extern const char* dsResourceManager_noContextError;

static dsShaderModule* createShaderModule(dsResourceManager* resourceManager,
	dsAllocator* allocator, mslModule* module)
{
	dsShaderModule* shaderModule = resourceManager->createShaderModuleFunc(resourceManager,
		allocator, module);
	if (shaderModule)
		DS_ATOMIC_FETCH_ADD32(&resourceManager->shaderModuleCount, 1);
	else
		mslModule_destroy(module);
	return shaderModule;
}

dsShaderModule* dsShaderModule_loadFile(dsResourceManager* resourceManager, dsAllocator* allocator,
	const char* filePath)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createShaderModuleFunc || !resourceManager->destroyShaderModuleFunc ||
		!filePath)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsFileStream fileStream;
	if (!dsFileStream_openPath(&fileStream, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open shader module file %s", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	DS_VERIFY(dsStream_seek((dsStream*)&fileStream, 0, dsStreamSeekWay_End));
	size_t size = (size_t)dsStream_tell((dsStream*)&fileStream);
	DS_VERIFY(dsStream_seek((dsStream*)&fileStream, 0, dsStreamSeekWay_Beginning));

	mslAllocator allocWrapper;
	allocWrapper.userData = allocator;
	allocWrapper.allocateFunc = (mslAllocateFunction)&dsAllocator_alloc;
	allocWrapper.freeFunc = (mslFreeFunction)&dsAllocator_free;

	mslModule_setInvalidFormatErrno(EFORMAT);
	mslModule* module = mslModule_readStream((mslReadFunction)&dsStream_read, &fileStream, size,
		&allocWrapper);
	if (module)
	{
		if (errno == EFORMAT)
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Invalid shader module file %s", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}


	dsShaderModule* shaderModule = createShaderModule(resourceManager, allocator, module);
	DS_PROFILE_FUNC_RETURN(shaderModule);
}

dsShaderModule* dsShaderModule_loadStream(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsStream* stream)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createShaderModuleFunc || !resourceManager->destroyShaderModuleFunc ||
		!stream)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!stream->seekFunc || !stream->tellFunc)
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Stream for reading shader modules must be seekable.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	uint64_t curPos = dsStream_tell(stream);
	DS_VERIFY(dsStream_seek(stream, 0, dsStreamSeekWay_End));
	size_t size = (size_t)(dsStream_tell(stream) - curPos);
	DS_VERIFY(dsStream_seek(stream, curPos, dsStreamSeekWay_Beginning));

	mslAllocator allocWrapper;
	allocWrapper.userData = allocator;
	allocWrapper.allocateFunc = (mslAllocateFunction)&dsAllocator_alloc;
	allocWrapper.freeFunc = (mslFreeFunction)&dsAllocator_free;

	mslModule_setInvalidFormatErrno(EFORMAT);
	mslModule* module = mslModule_readStream((mslReadFunction)&dsStream_read, stream, size,
		&allocWrapper);
	if (module)
	{
		if (errno == EFORMAT)
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid shader module stream");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsShaderModule* shaderModule = createShaderModule(resourceManager, allocator, module);
	DS_PROFILE_FUNC_RETURN(shaderModule);
}

dsShaderModule* dsShaderModule_loadData(dsResourceManager* resourceManager,
	dsAllocator* allocator, const void* data, size_t size)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createShaderModuleFunc || !resourceManager->destroyShaderModuleFunc ||
		!data)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!allocator)
		allocator = resourceManager->allocator;

	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	mslAllocator allocWrapper;
	allocWrapper.userData = allocator;
	allocWrapper.allocateFunc = (mslAllocateFunction)&dsAllocator_alloc;
	allocWrapper.freeFunc = (mslFreeFunction)&dsAllocator_free;

	mslModule_setInvalidFormatErrno(EFORMAT);
	mslModule* module = mslModule_readData(data, size, &allocWrapper);
	if (module)
	{
		if (errno == EFORMAT)
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid shader module data");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsShaderModule* shaderModule = createShaderModule(resourceManager, allocator, module);
	DS_PROFILE_FUNC_RETURN(shaderModule);
}

uint32_t dsShaderModule_shaderCount(const dsShaderModule* shaderModule)
{
	if (!shaderModule || !shaderModule->module)
		return 0;

	return mslModule_pipelineCount(shaderModule->module);
}

const char* dsShaderModule_shaderName(const dsShaderModule* shaderModule, uint32_t shader)
{
	if (!shaderModule || !shaderModule->module)
		return NULL;

	mslPipeline pipeline;
	if (!mslModule_pipeline(&pipeline, shaderModule->module, shader))
		return NULL;

	return pipeline.name;
}

bool dsShaderModule_destroy(dsShaderModule* shaderModule)
{
	DS_PROFILE_FUNC_START();

	if (!shaderModule || !shaderModule->resourceManager ||
		!shaderModule->resourceManager->destroyShaderModuleFunc)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsResourceManager* resourceManager = shaderModule->resourceManager;
	if (!dsResourceManager_canUseResources(resourceManager))
	{
		errno = EPERM;
		DS_LOG_ERROR(DS_RENDER_LOG_TAG, dsResourceManager_noContextError);
		DS_PROFILE_FUNC_RETURN(false);
	}

	mslModule* module = shaderModule->module;
	bool success = resourceManager->destroyShaderModuleFunc(resourceManager, shaderModule);
	if (success)
	{
		DS_ATOMIC_FETCH_ADD32(&resourceManager->shaderModuleCount, -1);
		mslModule_destroy(module);
	}
	DS_PROFILE_FUNC_RETURN(success);
}

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
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Types.h>

#include <MSL/Client/ModuleC.h>
#include <string.h>

extern const char* dsResourceManager_noContextError;

static dsShaderModule* createShaderModule(dsResourceManager* resourceManager,
	dsAllocator* allocator, mslModule* module, const char* name)
{
	dsShaderModule* shaderModule = resourceManager->createShaderModuleFunc(resourceManager,
		allocator, module, name);
	if (shaderModule)
	{
		DS_ASSERT(shaderModule->module == module);
		DS_ATOMIC_FETCH_ADD32(&resourceManager->shaderModuleCount, 1);
	}
	else
		mslModule_destroy(module);
	return shaderModule;
}

static dsShaderModule* dsShaderModule_loadImpl(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsStream* stream, const char* name)
{
	DS_VERIFY(dsStream_seek(stream, 0, dsStreamSeekWay_End));
	size_t size = (size_t)dsStream_tell(stream);
	DS_VERIFY(dsStream_seek(stream, 0, dsStreamSeekWay_Beginning));

	mslAllocator allocWrapper;
	allocWrapper.userData = allocator;
	allocWrapper.allocateFunc = (mslAllocateFunction)&dsAllocator_alloc;
	allocWrapper.freeFunc = (mslFreeFunction)&dsAllocator_free;

	mslModule_setInvalidFormatErrno(EFORMAT);
	mslModule* module = mslModule_readStream((mslReadFunction)&dsStream_read, stream, size,
		&allocWrapper);
	if (!module)
		return NULL;

	return createShaderModule(resourceManager, allocator, module, name);
}

dsShaderModule* dsShaderModule_loadFile(dsResourceManager* resourceManager, dsAllocator* allocator,
	const char* filePath, const char* name)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createShaderModuleFunc || !resourceManager->destroyShaderModuleFunc ||
		!filePath || !name)
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

	dsFileStream stream;
	if (!dsFileStream_openPath(&stream, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open shader module file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsShaderModule* shaderModule = dsShaderModule_loadImpl(resourceManager, allocator,
		(dsStream*)&stream, name);
	dsStream_close((dsStream*)&stream);
	if (!shaderModule)
	{
		if (errno == EFORMAT)
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Invalid shader module file '%s'.", filePath);
	}
	DS_PROFILE_FUNC_RETURN(shaderModule);
}

dsShaderModule* dsShaderModule_loadResource(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsFileResourceType type, const char* filePath, const char* name)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createShaderModuleFunc || !resourceManager->destroyShaderModuleFunc ||
		!filePath || !name)
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

	dsResourceStream stream;
	if (!dsResourceStream_open(&stream, type, filePath, "rb"))
	{
		DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Couldn't open shader module file '%s'.", filePath);
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsShaderModule* shaderModule = dsShaderModule_loadImpl(resourceManager, allocator,
		(dsStream*)&stream, name);
	dsStream_close((dsStream*)&stream);
	if (!shaderModule)
	{
		if (errno == EFORMAT)
			DS_LOG_ERROR_F(DS_RENDER_LOG_TAG, "Invalid shader module file '%s'.", filePath);
	}
	DS_PROFILE_FUNC_RETURN(shaderModule);
}

dsShaderModule* dsShaderModule_loadStream(dsResourceManager* resourceManager,
	dsAllocator* allocator, dsStream* stream, const char* name)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createShaderModuleFunc || !resourceManager->destroyShaderModuleFunc ||
		!stream || !name)
	{
		errno = EINVAL;
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	if (!stream->seekFunc || !stream->tellFunc)
	{
		errno = EINVAL;
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

	dsShaderModule* shaderModule = dsShaderModule_loadImpl(resourceManager, allocator, stream,
		name);
	if (!shaderModule)
	{
		if (errno == EFORMAT)
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid shader module stream.");
	}
	DS_PROFILE_FUNC_RETURN(shaderModule);
}

dsShaderModule* dsShaderModule_loadData(dsResourceManager* resourceManager,
	dsAllocator* allocator, const void* data, size_t size, const char* name)
{
	DS_PROFILE_FUNC_START();

	if (!resourceManager || (!allocator && !resourceManager->allocator) ||
		!resourceManager->createShaderModuleFunc || !resourceManager->destroyShaderModuleFunc ||
		!data || !name)
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
	if (!module)
	{
		if (errno == EFORMAT)
			DS_LOG_ERROR(DS_RENDER_LOG_TAG, "Invalid shader module data.");
		DS_PROFILE_FUNC_RETURN(NULL);
	}

	dsShaderModule* shaderModule = createShaderModule(resourceManager, allocator, module, name);
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

uint32_t dsShaderModule_shaderIndex(const dsShaderModule* shaderModule, const char* name)
{
	if (!shaderModule || !name)
		return DS_MATERIAL_UNKNOWN;

	uint32_t shaderCount = mslModule_pipelineCount(shaderModule->module);
	for (uint32_t i = 0; i < shaderCount; ++i)
	{
		mslPipeline pipeline;
		if (!mslModule_pipeline(&pipeline, shaderModule->module, i))
			return DS_MATERIAL_UNKNOWN;

		if (strcmp(name, dsShaderModule_shaderName(shaderModule, i)) == 0)
			return i;
	}

	return DS_MATERIAL_UNKNOWN;
}

bool dsShaderModule_shaderNameHasStage(const dsShaderModule* shaderModule, const char* name,
	dsShaderStage stage)
{
	if (!shaderModule || !name || (unsigned int)stage >= (unsigned int)mslStage_Count)
		return false;

	uint32_t shaderCount = mslModule_pipelineCount(shaderModule->module);
	for (uint32_t i = 0; i < shaderCount; ++i)
	{
		mslPipeline pipeline;
		if (!mslModule_pipeline(&pipeline, shaderModule->module, i))
			return false;

		if (strcmp(name, dsShaderModule_shaderName(shaderModule, i)) == 0)
			return pipeline.shaders[stage] != MSL_UNKNOWN;
	}

	return false;
}

bool dsShaderModule_shaderIndexHasStage(const dsShaderModule* shaderModule, uint32_t index,
	dsShaderStage stage)
{
	if (!shaderModule || (unsigned int)stage >= (unsigned int)mslStage_Count)
		return false;

	mslPipeline pipeline;
	if (!mslModule_pipeline(&pipeline, shaderModule->module, index))
		return false;

	return pipeline.shaders[stage] != MSL_UNKNOWN;
}

bool dsShaderModule_destroy(dsShaderModule* shaderModule)
{
	if (!shaderModule)
		return true;

	DS_PROFILE_FUNC_START();

	if (!shaderModule->resourceManager || !shaderModule->resourceManager->destroyShaderModuleFunc)
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

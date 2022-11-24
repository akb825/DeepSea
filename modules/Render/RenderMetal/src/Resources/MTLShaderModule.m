/*
 * Copyright 2019-2022 Aaron Barany
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

#include "Resources/MTLShaderModule.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Log.h>

#include <MSL/Client/ModuleC.h>
#include <string.h>

#define DS_SHADER_ERROR ((CFTypeRef)(size_t)-1)

dsShaderModule* dsMTLShaderModule_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	mslModule* module, const char* name)
{
	size_t nameLen = strlen(name) + 1;
	uint32_t shaderCount = mslModule_shaderCount(module);
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsMTLShaderModule)) + DS_ALIGNED_SIZE(nameLen) +
		DS_ALIGNED_SIZE(sizeof(CFTypeRef)*shaderCount);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsMTLShaderModule* shaderModule = DS_ALLOCATE_OBJECT(&bufferAlloc, dsMTLShaderModule);
	DS_ASSERT(shaderModule);

	dsShaderModule* baseShaderModule = (dsShaderModule*)shaderModule;

	baseShaderModule->resourceManager = resourceManager;
	baseShaderModule->allocator = dsAllocator_keepPointer(allocator);
	baseShaderModule->module = module;

	char* nameCopy = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, char, nameLen);
	DS_ASSERT(nameCopy);
	memcpy(nameCopy, name, nameLen);
	baseShaderModule->name = nameCopy;

	shaderModule->shaders = DS_ALLOCATE_OBJECT_ARRAY(&bufferAlloc, CFTypeRef, shaderCount);
	DS_ASSERT(shaderModule->shaders);
	memset(shaderModule->shaders, 0, sizeof(CFTypeRef)*shaderCount);

	return baseShaderModule;
}

bool dsMTLShaderModule_destroy(dsResourceManager* resourceManager, dsShaderModule* module)
{
	DS_UNUSED(resourceManager);
	dsMTLShaderModule* mtlModule = (dsMTLShaderModule*)module;
	uint32_t shaderCount = mslModule_shaderCount(module->module);
	for (uint32_t i = 0; i < shaderCount; ++i)
	{
		if (mtlModule->shaders[i] && mtlModule->shaders[i] != DS_SHADER_ERROR)
			CFRelease(mtlModule->shaders[i]);
	}

	if (module->allocator)
		DS_VERIFY(dsAllocator_free(module->allocator, module));
	return true;
}

id<MTLFunction> dsMTLShaderModule_getShader(dsShaderModule* module, uint32_t shaderIndex,
	const char* pipelineName)
{
	dsMTLShaderModule* mtlModule = (dsMTLShaderModule*)module;
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)module->resourceManager->renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)mtlRenderer->device;

	// Check if it was already loaded.
	CFTypeRef functionRef;
	DS_ATOMIC_LOAD_PTR(mtlModule->shaders + shaderIndex, &functionRef);
	if (functionRef)
	{
		if (functionRef == DS_SHADER_ERROR)
			return nil;
		return (__bridge id<MTLFunction>)functionRef;
	}

	// Load the shader. If another thread loads the same shader, atomics will be used to return the
	// previous result. Since this should be rare, it is better to pay the extra cost than locking
	// each time.
	dispatch_data_t data = dispatch_data_create(mslModule_shaderData(module->module, shaderIndex),
		mslModule_shaderSize(module->module, shaderIndex), nil, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
	NSError* error = NULL;
	id<MTLLibrary> library = [device newLibraryWithData: data error: &error];
	id<MTLFunction> function = nil;
	if (error)
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_RENDER_METAL_LOG_TAG, "Error loading shader %s.%s: %s",
			module->name, pipelineName, error.localizedDescription.UTF8String);
	}
	else if (library)
	{
		if ([library functionNames].count == 1)
		{
			function = [library newFunctionWithName: library.functionNames[0]];
			if (!function)
				errno = ENOMEM;
		}
		else
		{
			errno = EFORMAT;
			DS_LOG_ERROR_F(DS_RENDER_METAL_LOG_TAG, "Multiple entry points for shader %s.%s.",
				module->name, pipelineName);
		}
	}
	else
	{
		errno = EPERM;
		DS_LOG_ERROR_F(DS_RENDER_METAL_LOG_TAG, "Error loading shader %s.%s.",
			module->name, pipelineName);
	}

	if (function)
		functionRef = CFBridgingRetain(function);
	else
		functionRef = DS_SHADER_ERROR;
	CFTypeRef expected = NULL;
	if (!DS_ATOMIC_COMPARE_EXCHANGE_PTR(mtlModule->shaders + shaderIndex, &expected, &functionRef,
			false))
	{
		if (functionRef && functionRef != DS_SHADER_ERROR)
			CFRelease(functionRef);
		DS_ASSERT(expected);
		if (expected == DS_SHADER_ERROR)
			function = nil;
		else
			function = (__bridge id<MTLFunction>)expected;
		return function;
	}

	return function;
}

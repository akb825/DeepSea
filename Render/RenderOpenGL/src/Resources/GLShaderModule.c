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

#include "Resources/GLShaderModule.h"

#include "AnyGL/AnyGL.h"
#include "Resources/GLResource.h"
#include "GLHelpers.h"
#include "Types.h"
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>

#include <MSL/Client/ModuleC.h>
#include <string.h>

#define DS_SHADER_ERROR (GLuint)-1

dsShaderModule* dsGLShaderModule_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	mslModule* module, const char* name)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);

	uint32_t shaderCount = mslModule_shaderCount(module);
	size_t totalSize = DS_ALIGNED_SIZE(sizeof(dsGLShaderModule)) +
		DS_ALIGNED_SIZE(sizeof(GLuint)*shaderCount);
	void* buffer = dsAllocator_alloc(allocator, totalSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, totalSize));
	dsGLShaderModule* shaderModule = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc,
		dsGLShaderModule);
	DS_ASSERT(shaderModule);

	dsShaderModule* baseShaderModule = (dsShaderModule*)shaderModule;
	baseShaderModule->resourceManager = resourceManager;
	baseShaderModule->allocator = dsAllocator_keepPointer(allocator);
	baseShaderModule->module = module;
	baseShaderModule->name = name;

	dsGLResource_initialize(&shaderModule->resource);
	if (shaderCount > 0)
	{
		shaderModule->shaders = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, GLuint,
			shaderCount);
		DS_ASSERT(shaderModule->shaders);
		memset(shaderModule->shaders, 0, sizeof(GLuint)*shaderCount);
	}
	else
		shaderModule->shaders = NULL;

	return baseShaderModule;
}

bool dsGLShaderModule_compileShader(GLuint* outShader, dsShaderModule* module, uint32_t shaderIndex,
	GLenum stage, const char* pipelineName)
{
	dsGLShaderModule* glModule = (dsGLShaderModule*)module;
	DS_ASSERT(shaderIndex < mslModule_shaderCount(module->module));

	// Check if it was already compiled.
	GLuint shaderId;
	DS_ATOMIC_LOAD32(glModule->shaders + shaderIndex, &shaderId);
	if (shaderId)
	{
		*outShader = shaderId;
		return shaderId != DS_SHADER_ERROR;
	}

	// Compile the shader. If another thread compiles the same shader, atomics will be used to
	// return the previous result. Since this should be rare, it is better to pay the extra cost
	// than locking each time.
	shaderId = glCreateShader(stage);
	if (!shaderId)
	{
		GLenum error = glGetError();
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error creating shader: %s",
			AnyGL_errorString(error));
		errno = dsGetGLErrno(error);
		shaderId = DS_SHADER_ERROR;
		DS_ATOMIC_STORE32(glModule->shaders + shaderIndex, &shaderId);
		return false;
	}

	const char* shaderString = (const char*)mslModule_shaderData(module->module, shaderIndex);
	if (!shaderString)
	{
		errno = EFORMAT;
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "No shader string for shader %s.%s.",
			module->name, pipelineName);
		glDeleteShader(shaderId);
		shaderId = DS_SHADER_ERROR;
		DS_ATOMIC_STORE32(glModule->shaders + shaderIndex, &shaderId);
		return false;
	}

	GLint length = (GLint)mslModule_shaderSize(module->module, shaderIndex);
	while (length > 0 && shaderString[length - 1] == 0)
		--length;
	glShaderSource(shaderId, 1, &shaderString, &length);
	glCompileShader(shaderId);

	GLint compileSuccess = false;
	glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileSuccess);
	if (!compileSuccess)
	{
		errno = EFORMAT;
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error compiling shader %s.%s:",
			module->name, pipelineName);

		GLint logSize = 0;
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logSize);
		if (logSize > 0)
		{
			char* buffer = (char*)dsAllocator_alloc(module->resourceManager->allocator, logSize);
			if (buffer)
			{
				glGetShaderInfoLog(shaderId, logSize, &logSize, buffer);
				DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, buffer);
				DS_VERIFY(dsAllocator_free(module->resourceManager->allocator, buffer));
			}
		}

		glDeleteShader(shaderId);
		shaderId = DS_SHADER_ERROR;
		DS_ATOMIC_STORE32(glModule->shaders + shaderIndex, &shaderId);
		return false;
	}

	GLuint expected = 0;
	if (!DS_ATOMIC_COMPARE_EXCHANGE32(glModule->shaders + shaderIndex, &expected, &shaderId,
		false))
	{
		glDeleteShader(shaderId);
		*outShader = expected;
		return expected != DS_SHADER_ERROR;
	}

	*outShader = shaderId;
	return true;
}

static bool destroyImpl(dsShaderModule* module)
{
	dsGLShaderModule* glModule = (dsGLShaderModule*)module;
	uint32_t shaderCount = mslModule_shaderCount(module->module);
	for (uint32_t i = 0; i < shaderCount; ++i)
	{
		GLuint shaderId = glModule->shaders[i];
		if (shaderId && shaderId != DS_SHADER_ERROR)
			glDeleteShader(shaderId);
	}

	if (module->allocator)
		return dsAllocator_free(module->allocator, module);

	return true;
}

bool dsGLShaderModule_destroy(dsResourceManager* resourceManager, dsShaderModule* module)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(module);

	dsGLShaderModule* glModule = (dsGLShaderModule*)module;
	if (dsGLResource_destroy(&glModule->resource))
		return destroyImpl(module);

	return true;
}

void dsGLShaderModule_addInternalRef(dsShaderModule* module)
{
	DS_ASSERT(module);
	dsGLShaderModule* glModule = (dsGLShaderModule*)module;
	dsGLResource_addRef(&glModule->resource);
}

void dsGLShaderModule_freeInternalRef(dsShaderModule* module)
{
	DS_ASSERT(module);
	dsGLShaderModule* glModule = (dsGLShaderModule*)module;
	if (dsGLResource_freeRef(&glModule->resource))
		destroyImpl(module);
}

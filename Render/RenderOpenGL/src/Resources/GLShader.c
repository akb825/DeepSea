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

#include "Resources/GLShader.h"

#include "AnyGL/AnyGL.h"
#include "AnyGL/gl.h"
#include "GLHelpers.h"
#include "Resources/GLMaterialDesc.h"
#include "Resources/GLResource.h"
#include "Resources/GLShaderModule.h"
#include "Types.h"
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>

#include <MSL/Client/ModuleC.h>
#include <string.h>

#define DS_BUFFER_SIZE 256

static void createSamplers(dsGLShader* shader, mslModule* module, uint32_t shaderIndex)
{
	dsResourceManager* resourceManager = ((dsShader*)shader)->resourceManager;
	for (uint32_t i = 0; i < shader->pipeline.samplerStateCount; ++i)
	{
		mslSamplerState samplerState;
		DS_VERIFY(mslModule_samplerState(&samplerState, module, shaderIndex, i));

		glSamplerParameteri(shader->samplerIds[i], GL_TEXTURE_MIN_FILTER,
			dsGetGLMinFilter(samplerState.minFilter, samplerState.mipFilter));
		glSamplerParameteri(shader->samplerIds[i], GL_TEXTURE_MAG_FILTER,
			dsGetGLMagFilter(samplerState.magFilter));
		glSamplerParameteri(shader->samplerIds[i], GL_TEXTURE_WRAP_S,
			dsGetGLAddressMode(samplerState.addressModeU));
		glSamplerParameteri(shader->samplerIds[i], GL_TEXTURE_WRAP_T,
			dsGetGLAddressMode(samplerState.addressModeV));
		if (resourceManager->maxTextureDepth > 0)
		{
			glSamplerParameteri(shader->samplerIds[i], GL_TEXTURE_WRAP_R,
				dsGetGLAddressMode(samplerState.addressModeW));
		}
		if (AnyGL_EXT_texture_filter_anisotropic)
		{
			glSamplerParameterf(shader->samplerIds[i], GL_TEXTURE_MAX_ANISOTROPY_EXT,
				samplerState.maxAnisotropy == MSL_UNKNOWN_FLOAT ? 1.0f :
				samplerState.maxAnisotropy);
		}

		if (AnyGL_atLeastVersion(2, 0, false) || AnyGL_atLeastVersion(3, 0, true))
		{
			glSamplerParameterf(shader->samplerIds[i], GL_TEXTURE_LOD_BIAS,
				samplerState.mipLodBias == MSL_UNKNOWN_FLOAT ? 0.0f : samplerState.mipLodBias);
			glSamplerParameterf(shader->samplerIds[i], GL_TEXTURE_MIN_LOD,
				samplerState.minLod == MSL_UNKNOWN ? -1000 : samplerState.minLod);
			glSamplerParameterf(shader->samplerIds[i], GL_TEXTURE_MAX_LOD,
				samplerState.maxLod == MSL_UNKNOWN ? 1000 : samplerState.maxLod);
		}

		if (AnyGL_atLeastVersion(1, 0, false) || AnyGL_OES_texture_border_clamp)
		{
			switch (samplerState.borderColor)
			{
				case mslBorderColor_Unset:
				case mslBorderColor_TransparentBlack:
				{
					float color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
					glSamplerParameterfv(shader->samplerIds[i], GL_TEXTURE_BORDER_COLOR, color);
					break;
				}
				case mslBorderColor_TransparentIntZero:
				{
					GLint color[4] = {0, 0, 0, 0};
					glSamplerParameterIiv(shader->samplerIds[i], GL_TEXTURE_BORDER_COLOR, color);
					break;
				}
				case mslBorderColor_OpaqueBlack:
				{
					float color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
					glSamplerParameterfv(shader->samplerIds[i], GL_TEXTURE_BORDER_COLOR, color);
					break;
				}
				case mslBorderColor_OpaqueIntZero:
				{
					GLint color[4] = {0, 0, 0, 1};
					glSamplerParameterIiv(shader->samplerIds[i], GL_TEXTURE_BORDER_COLOR, color);
					break;
				}
				case mslBorderColor_OpaqueWhite:
				{
					float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
					glSamplerParameterfv(shader->samplerIds[i], GL_TEXTURE_BORDER_COLOR, color);
					break;
				}
				case mslBorderColor_OpaqueIntOne:
				{
					GLint color[4] = {1, 1, 1, 1};
					glSamplerParameterIiv(shader->samplerIds[i], GL_TEXTURE_BORDER_COLOR, color);
					break;
				}
			}
		}
	}
}

static uint32_t findUniform(mslModule* module, uint32_t shaderIndex, const mslPipeline* pipeline,
	const char* name)
{
	for (uint32_t i = 0; i < pipeline->uniformCount; ++i)
	{
		mslUniform uniform;
		DS_VERIFY(mslModule_uniform(&uniform, module, shaderIndex, i));
		if (strcmp(uniform.name, name) == 0)
			return i;
	}

	return MSL_UNKNOWN;
}

static uint32_t getUsedTextures(mslModule* module, uint32_t shaderIndex,
	const mslPipeline* pipeline)
{
	uint32_t mask = 0;
	for (uint32_t i = 0; i < pipeline->uniformCount; ++i)
	{
		mslUniform uniform;
		DS_VERIFY(mslModule_uniform(&uniform, module, shaderIndex, i));
		if (uniform.type >= mslType_Sampler1D && uniform.type <= mslType_USubpassInputMS &&
			uniform.binding != MSL_UNKNOWN)
		{
			mask |= 1 << uniform.binding;
		}
	}

	return mask;
}

static bool hookupBindings(dsGLShader* shader, const dsMaterialDesc* materialDesc,
	mslModule* module, uint32_t shaderIndex, bool useGfxBuffers)
{
	glUseProgram(shader->programId);

	uint32_t usedTextures = getUsedTextures(module, shaderIndex, &shader->pipeline);
	char nameBuffer[DS_BUFFER_SIZE];
	for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
	{
		switch (materialDesc->elements[i].type)
		{
			case dsMaterialType_Texture:
			case dsMaterialType_Image:
			case dsMaterialType_SubpassInput:
			{
				int len = snprintf(nameBuffer, DS_BUFFER_SIZE, "Uniforms.%s",
					materialDesc->elements[i].name);
				if (len < 0 || len >= DS_BUFFER_SIZE)
				{
					DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Uniform name '%s' is too long.",
						materialDesc->elements[i].name);
					errno = EINDEX;
					glUseProgram(0);
					return false;
				}

				uint32_t uniformIndex = findUniform(module, shaderIndex, &shader->pipeline,
					nameBuffer);
				if (uniformIndex == MSL_UNKNOWN)
					shader->uniforms[i].location = -1;
				else
				{
					GLint binding = glGetUniformLocation(shader->programId, nameBuffer);
					if (binding < 0)
					{
						shader->uniforms[i].location = -1;
						continue;
					}

					mslUniform uniform;
					DS_VERIFY(mslModule_uniform(&uniform, module, shaderIndex, uniformIndex));
					shader->uniforms[i].samplerIndex = uniform.samplerIndex;

					uint32_t textureIndex = uniform.binding;
					if (textureIndex == MSL_UNKNOWN)
					{
						for (unsigned int i = 0; i < sizeof(usedTextures)*8; ++i)
						{
							if (!(usedTextures & (1 << i)))
							{
								textureIndex = i;
								usedTextures |= 1 << i;
								break;
							}
						}

						if (textureIndex == MSL_UNKNOWN)
						{
							DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG,
								"Ran out of texture indices for shader %s",
								shader->pipeline.name);
							errno = EINDEX;
							glUseProgram(0);
							return false;
						}
					}
					glUniform1i(binding, textureIndex);
					shader->uniforms[i].location = textureIndex;
				}
				break;
			}
			case dsMaterialType_UniformBlock:
			case dsMaterialType_UniformBuffer:
				shader->uniforms[i].location = glGetUniformBlockIndex(shader->programId,
					materialDesc->elements[i].name);
				break;
			case dsMaterialType_VariableGroup:
			{
				if (useGfxBuffers)
				{
					shader->uniforms[i].location = glGetUniformBlockIndex(shader->programId,
						materialDesc->elements[i].name);
				}
				else
				{
					const dsShaderVariableGroupDesc* groupDesc =
						materialDesc->elements[i].shaderVariableGroupDesc;
					DS_ASSERT(groupDesc);
					for (uint32_t j = 0; j < groupDesc->elementCount; ++j)
					{
						int len = snprintf(nameBuffer, DS_BUFFER_SIZE, "Uniforms.%s",
							groupDesc->elements[j].name);
						if (len < 0 || len >= DS_BUFFER_SIZE)
						{
							DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG,
								"Uniform name '%s' is too long.", groupDesc->elements[j].name);
							errno = EINDEX;
							glUseProgram(0);
							return false;
						}

						shader->uniforms[i].groupLocations[j] = glGetUniformLocation(
							shader->programId, nameBuffer);
					}
				}
				break;
			}
			default:
			{
				int len = snprintf(nameBuffer, DS_BUFFER_SIZE, "Uniforms.%s",
					materialDesc->elements[i].name);
				if (len < 0 || len >= DS_BUFFER_SIZE)
				{
					DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Uniform name '%s' is too long.",
						materialDesc->elements[i].name);
					errno = EINDEX;
					glUseProgram(0);
					return false;
				}

				shader->uniforms[i].location = glGetUniformLocation(shader->programId, nameBuffer);
			}
		}
	}

	glUseProgram(0);
	return true;
}

dsShader* dsGLShader_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsShaderModule* module, uint32_t shaderIndex, const dsMaterialDesc* materialDesc,
	dsPrimitiveType primitiveType)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);
	DS_ASSERT(module);
	DS_ASSERT(materialDesc);

	mslPipeline pipeline;
	DS_VERIFY(mslModule_pipeline(&pipeline, module->module, shaderIndex));
	mslStruct pushConstantStruct = {};
	if (pipeline.pushConstantStruct != MSL_UNKNOWN)
	{
		DS_VERIFY(mslModule_struct(&pushConstantStruct, module->module, shaderIndex,
			pipeline.pushConstantStruct));
	}

	bool hasSamplers = ANYGL_SUPPORTED(glGenSamplers);
	bool useGfxBuffers = dsShaderVariableGroup_useGfxBuffer(resourceManager);
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsGLShader));
	if (hasSamplers)
		fullSize += DS_ALIGNED_SIZE(sizeof(GLuint)*pipeline.samplerStateCount);
	fullSize += DS_ALIGNED_SIZE(sizeof(dsGLUniformInfo)*materialDesc->elementCount);
	if (!useGfxBuffers)
	{
		for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
		{
			const dsShaderVariableGroupDesc* groupDesc =
				materialDesc->elements[i].shaderVariableGroupDesc;
			if (groupDesc)
				fullSize += DS_ALIGNED_SIZE(sizeof(GLuint)*groupDesc->elementCount);
		}
	}

	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsGLShader* shader = (dsGLShader*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
		sizeof(dsGLShader));
	DS_ASSERT(shader);

	dsShader* baseShader = (dsShader*)shader;
	baseShader->resourceManager = resourceManager;
	baseShader->allocator = dsAllocator_keepPointer(allocator);
	baseShader->module = module;
	baseShader->pipelineIndex = shaderIndex;
	baseShader->pipeline = &shader->pipeline;
	baseShader->materialDesc = materialDesc;
	baseShader->primitiveType = primitiveType;

	bool prevChecksEnabled = AnyGL_getErrorCheckingEnabled();
	AnyGL_setErrorCheckingEnabled(false);
	dsClearGLErrors();

	dsGLResource_initialize(&shader->resource);
	shader->pipeline = pipeline;
	shader->defaultAnisotropy = resourceManager->renderer->defaultAnisotropy;
	shader->programId = 0;
	if (hasSamplers && pipeline.samplerStateCount > 0)
	{
		shader->samplerIds = (GLuint*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
			sizeof(GLuint)*pipeline.samplerStateCount);
		DS_ASSERT(shader->samplerIds);
		glGenSamplers(shader->pipeline.samplerStateCount, shader->samplerIds);
		if (!*shader->samplerIds)
		{
			GLenum error = glGetError();
			DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error creating samplers: %s",
				AnyGL_errorString(error));
			errno = dsGetGLErrno(error);
			dsGLShader_destroy(resourceManager, baseShader);
			AnyGL_setErrorCheckingEnabled(prevChecksEnabled);
			return NULL;
		}
	}
	else
		shader->samplerIds = NULL;

	if (materialDesc->elementCount > 0)
	{
		shader->uniforms = (dsGLUniformInfo*)dsAllocator_alloc((dsAllocator*)&bufferAlloc,
			sizeof(dsGLUniformInfo)*materialDesc->elementCount);
		DS_ASSERT(shader->uniforms );
		memset(shader->uniforms , 0xFF, sizeof(dsGLUniformInfo)*materialDesc->elementCount);

		if (!useGfxBuffers)
		{
			for (uint32_t i = 0; i < materialDesc->elementCount; ++i)
			{
				const dsShaderVariableGroupDesc* groupDesc =
					materialDesc->elements[i].shaderVariableGroupDesc;
				if (groupDesc)
				{
					shader->uniforms[i].groupLocations = (GLint*)dsAllocator_alloc(
						(dsAllocator*)&bufferAlloc, sizeof(GLint)*groupDesc->elementCount);
					DS_ASSERT(shader->uniforms[i].groupLocations);
					memset(shader->uniforms[i].groupLocations, 0xFF,
						sizeof(GLuint)*groupDesc->elementCount);
				}
			}
		}
	}
	else
		shader->uniforms = NULL;

	shader->programId = glCreateProgram();
	if (!shader->programId)
	{
		GLenum error = glGetError();
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error creating shader %s: %s", pipeline.name,
			AnyGL_errorString(error));
		errno = dsGetGLErrno(error);
		dsGLShader_destroy(resourceManager, baseShader);
		AnyGL_setErrorCheckingEnabled(prevChecksEnabled);
		return NULL;
	}

	static GLenum stageMap[] =
	{
		GL_VERTEX_SHADER,
		GL_TESS_CONTROL_SHADER,
		GL_TESS_EVALUATION_SHADER,
		GL_GEOMETRY_SHADER,
		GL_FRAGMENT_SHADER,
		GL_COMPUTE_SHADER
	};

	// Load and compile the shaders.
	bool success = true;
	GLuint shaderIds[mslStage_Count];
	memset(shaderIds, 0, sizeof(shaderIds));
	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (pipeline.shaders[i] == MSL_UNKNOWN)
			continue;

		shaderIds[i] = glCreateShader(stageMap[i]);
		if (shaderIds[i])
		{
			const char* shaderString = (const char*)mslModule_shaderData(module->module,
				pipeline.shaders[i]);
			if (!shaderString)
			{
				errno = EFORMAT;
				DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "No shader string for shader %s.",
					pipeline.name);
				success = false;
			}
			GLint length = (GLint)mslModule_shaderSize(module->module,
				pipeline.shaders[i]);
			while (length > 0 && shaderString[length - 1] == 0)
				--length;
			glShaderSource(shaderIds[i], 1, &shaderString, &length);

			GLint compileSuccess = false;
			glGetShaderiv(shaderIds[i], GL_COMPILE_STATUS, &compileSuccess);
			if (compileSuccess)
				glAttachShader(shader->programId, shaderIds[i]);
			else
			{
				success = false;
				DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error compiling shader %s:",
					pipeline.name);

				GLint logSize = 0;
				glGetShaderiv(shaderIds[i], GL_INFO_LOG_LENGTH, &logSize);
				char* buffer = (char*)dsAllocator_alloc(resourceManager->allocator, logSize);
				if (buffer)
				{
					glGetShaderInfoLog(shaderIds[i], logSize, &logSize, buffer);
					DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, buffer);
					DS_VERIFY(dsAllocator_free(resourceManager->allocator, buffer));
				}
			}
		}
		else
		{
			GLenum error = glGetError();
			DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error creating shader: %s",
				AnyGL_errorString(error));
			errno = dsGetGLErrno(error);
			success = false;
		}

		if (!success)
			break;
	}

	// Set the input locations.
	if (success && shaderIds[mslStage_Vertex])
	{
		for (uint32_t i = 0; i < pipeline.attributeCount; ++i)
		{
			mslAttribute attribute;
			if (!mslModule_attribute(&attribute, module->module, shaderIndex, i))
			{
				errno = EFORMAT;
				DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Invalid vertex attribute for shader %s.",
					pipeline.name);
				success = false;
				break;
			}

			glBindAttribLocation(shader->programId, attribute.location, attribute.name);
		}
	}

	if (success)
	{
		glLinkProgram(shader->programId);
		GLint linkSuccess = false;
		glGetProgramiv(shader->programId, GL_LINK_STATUS, &linkSuccess);
		if (linkSuccess)
		{
			for (int i = 0; i < mslStage_Count; ++i)
			{
				if (shaderIds[i])
					glDetachShader(shader->programId, shaderIds[i]);
			}
		}
		else
		{
			success = false;
			DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error linking shader %s:",
				pipeline.name);

			GLint logSize = 0;
			glGetProgramiv(shader->programId, GL_INFO_LOG_LENGTH, &logSize);
			char* buffer = (char*)dsAllocator_alloc(resourceManager->allocator, logSize);
			if (buffer)
			{
				glGetProgramInfoLog(shader->programId, logSize, &logSize, buffer);
				DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, buffer);
				DS_VERIFY(dsAllocator_free(resourceManager->allocator, buffer));
			}
		}
	}

	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (shaderIds[i])
			glDeleteShader(shaderIds[i]);
	}

	AnyGL_setErrorCheckingEnabled(prevChecksEnabled);
	if (success)
	{
		if (hasSamplers)
			createSamplers(shader, module->module, shaderIndex);
		success = hookupBindings(shader, materialDesc, module->module, shaderIndex, useGfxBuffers);
	}

	if (!success)
	{
		dsGLShader_destroy(resourceManager, baseShader);
		return NULL;
	}

	return baseShader;
}

static bool destroyImpl(dsShader* shader)
{
	dsGLShader* glShader = (dsGLShader*)shader;
	if (glShader->programId)
		glDeleteProgram(glShader->programId);
	if (glShader->samplerIds && *glShader->samplerIds)
		glDeleteSamplers(glShader->pipeline.samplerStateCount, glShader->samplerIds);
	if (shader->allocator)
		return dsAllocator_free(shader->allocator, shader);

	return true;
}

bool dsGLShader_destroy(dsResourceManager* resourceManager, dsShader* shader)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(shader);

	dsGLShader* glModule = (dsGLShader*)shader;
	if (dsGLResource_destroy(&glModule->resource))
		return destroyImpl(shader);

	return true;
}

void dsGLShader_addInternalRef(dsShader* shader)
{
	DS_ASSERT(shader);
	dsGLShader* glShader = (dsGLShader*)shader;
	dsGLShaderModule_addInternalRef(shader->module);
	dsGLMaterialDesc_addInternalRef((dsMaterialDesc*)shader->materialDesc);
	dsGLResource_addRef(&glShader->resource);
}

void dsGLShader_freeInternalRef(dsShader* shader)
{
	DS_ASSERT(shader);
	dsGLShader* glShader = (dsGLShader*)shader;
	dsGLShaderModule_freeInternalRef(shader->module);
	dsGLMaterialDesc_freeInternalRef((dsMaterialDesc*)shader->materialDesc);
	if (dsGLResource_freeRef(&glShader->resource))
		destroyImpl(shader);
}

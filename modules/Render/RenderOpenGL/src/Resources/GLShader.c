/*
 * Copyright 2017-2018 Aaron Barany
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
#include "GLCommandBuffer.h"
#include "GLHelpers.h"
#include "Resources/GLMaterialDesc.h"
#include "Resources/GLResource.h"
#include "Resources/GLShaderModule.h"
#include "Platform/GLPlatform.h"
#include "GLTypes.h"
#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Streams/FileStream.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Thread/Thread.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>

#include <MSL/Client/ModuleC.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

#if DS_WINDOWS
#include <direct.h>
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif

#define DS_BUFFER_SIZE 256
// DSGL
#define DS_SHADER_MAGIC_NUMBER 0x68837176
#define DS_SHADER_VERSION 0

// "tmp#" where # is up to ~4 billion (10 digits) and null terminator
#define DS_MAX_TEMP_SIZE 14
static uint32_t tempCounter;

static void hashShader(uint64_t shaderHash[2], const mslModule* module, const mslPipeline* pipeline)
{
	shaderHash[0] = 0;
	shaderHash[1] = 0;
	for (int i = 0; i < mslStage_Count; ++i)
	{
		uint32_t shaderIndex = pipeline->shaders[i];
		if (shaderIndex != MSL_UNKNOWN)
		{
			dsHashCombineBytes128(shaderHash, shaderHash, mslModule_shaderData(module, shaderIndex),
				mslModule_shaderSize(module, shaderIndex));
		}
	}
}

static bool makeDir(const char* path)
{
#if DS_WINDOWS
	return mkdir(path) == 0;
#else
	return mkdir(path, 0755);
#endif
}

static bool shaderPath(char outPath[DS_PATH_MAX], const char* shaderCacheDir,
	const char* moduleName, const char* pipelineName)
{
	if (!dsPath_combine(outPath, DS_PATH_MAX, shaderCacheDir, moduleName))
		return false;

	size_t pathLength = strlen(outPath);
	size_t pipelineLength = strlen(pipelineName);
	if (pathLength + pipelineLength + 2 > DS_PATH_MAX)
		return false;

	outPath[pathLength++] = '.';
	memcpy(outPath + pathLength, pipelineName, pipelineLength + 1);
	return true;
}

static bool loadShader(dsResourceManager* resourceManager, const char* shaderCacheDir,
	const char* moduleName, const char* pipelineName, GLuint program, uint64_t shaderHash[2])
{
	static bool printedError = false;
	char path[DS_PATH_MAX];
	if (!shaderPath(path, shaderCacheDir, moduleName, pipelineName))
	{
		if (!printedError)
		{
			DS_LOG_WARNING_F(DS_RENDER_OPENGL_LOG_TAG, "Shader cache path is too long.");
			printedError = true;
		}
		return false;
	}

	dsFileStream stream;
	uint8_t* data = NULL;
	if (!dsFileStream_openPath(&stream, path, "rb"))
		return false;

	uint32_t magicNumber;
	if (!dsFileStream_read(&stream, &magicNumber, sizeof(magicNumber)) ||
		magicNumber != DS_SHADER_MAGIC_NUMBER)
	{
		goto error;
	}

	uint32_t version;
	if (!dsFileStream_read(&stream, &version, sizeof(version)) || version != DS_SHADER_VERSION)
		goto error;

	uint64_t curHash[2];
	if (!dsFileStream_read(&stream, curHash, sizeof(curHash)) || curHash[0] != shaderHash[0] ||
		curHash[1] != shaderHash[1])
	{
		goto error;
	}

	GLenum format;
	if (!dsFileStream_read(&stream, &format, sizeof(format)))
		goto error;

	uint32_t size;
	if (!dsFileStream_read(&stream, &size, sizeof(size)))
		goto error;

	data = (uint8_t*)dsAllocator_alloc(resourceManager->allocator, size);
	if (!data || !dsFileStream_read(&stream, data, size))
		goto error;

	DS_VERIFY(dsFileStream_close(&stream));
	glProgramBinary(program, format, data, size);
	dsAllocator_free(resourceManager->allocator, data);

	GLint linkSuccess = false;
	glGetProgramiv(program, GL_LINK_STATUS, &linkSuccess);
	return linkSuccess;

error:
	DS_VERIFY(dsFileStream_close(&stream));
	dsAllocator_free(resourceManager->allocator, data);
	return false;
}

static bool writeShader(dsResourceManager* resourceManager, const char* shaderCacheDir,
	const char* moduleName, const char* pipelineName, GLuint program, uint64_t shaderHash[2])
{
	static bool printedError = false;
	struct stat statInfo;
	bool exists = stat(shaderCacheDir, &statInfo) == 0;
	if (exists && !S_ISDIR(statInfo.st_mode))
	{
		if (!printedError)
		{
			DS_LOG_WARNING_F(DS_RENDER_OPENGL_LOG_TAG,
				"Shader cache directory '%s' isn't a directory.", shaderCacheDir);
			printedError = true;
		}
		return false;
	}
	else if (!exists)
	{
		if (!makeDir(shaderCacheDir) && errno != EEXIST)
		{
			if (!printedError)
			{
				DS_LOG_WARNING_F(DS_RENDER_OPENGL_LOG_TAG,
					"Couldn't create directory '%s': %s", shaderCacheDir, dsErrorString(errno));
				printedError = true;
			}
			return false;
		}
	}

	char path[DS_PATH_MAX];
	if (!shaderPath(path, shaderCacheDir, moduleName, pipelineName))
	{
		if (!printedError)
		{
			DS_LOG_WARNING_F(DS_RENDER_OPENGL_LOG_TAG, "Shader cache path is too long.");
			printedError = true;
		}
		return false;
	}

	// Write to a temporary path in case two threads try to load the same shader at once.
	uint32_t tempIndex = DS_ATOMIC_FETCH_ADD32(&tempCounter, 1);
	char tempFileName[DS_MAX_TEMP_SIZE];
	DS_VERIFY((unsigned int)snprintf(tempFileName, DS_MAX_TEMP_SIZE, "tmp%u", tempIndex) <
		DS_MAX_TEMP_SIZE);
	char tempPath[DS_PATH_MAX];
	DS_VERIFY(dsPath_combine(tempPath, DS_PATH_MAX, shaderCacheDir, tempFileName));

	GLint size = 0;
	glGetProgramiv(program, GL_PROGRAM_BINARY_LENGTH, &size);
	DS_ASSERT(size > 0);
	uint8_t* data = (uint8_t*)dsAllocator_alloc(resourceManager->allocator, size);
	if (!data)
		return false;

	GLenum format;
	glGetProgramBinary(program, size, NULL, &format, data);

	dsFileStream stream;
	if (!dsFileStream_openPath(&stream, tempPath, "wb"))
	{
		if (!printedError)
		{
			DS_LOG_WARNING_F(DS_RENDER_OPENGL_LOG_TAG,
				"Couldn't write to directory '%s': %s", shaderCacheDir, dsErrorString(errno));
			printedError = true;
		}
		return false;
	}

	uint32_t magicNumber = DS_SHADER_MAGIC_NUMBER;
	if (!dsFileStream_write(&stream, &magicNumber, sizeof(magicNumber)))
		goto error;

	uint32_t version = DS_SHADER_VERSION;
	if (!dsFileStream_write(&stream, &version, sizeof(version)))
		goto error;

	if (!dsFileStream_write(&stream, shaderHash, sizeof(uint64_t)*2))
		goto error;

	if (!dsFileStream_write(&stream, &format, sizeof(format)))
		goto error;

	if (!dsFileStream_write(&stream, &size, sizeof(size)))
		goto error;

	if (!dsFileStream_write(&stream, data, size))
		goto error;

	DS_VERIFY(dsFileStream_close(&stream));

	// Rename the temporary file to the final location once we're done so it's atomic on the
	// filesystem.
	DS_VERIFY(rename(tempPath, path) == 0);
	return true;

error:
	DS_VERIFY(dsFileStream_close(&stream));
	dsAllocator_free(resourceManager->allocator, data);
	return false;
}

static bool compileShaders(GLuint shaderIds[mslStage_Count], dsShaderModule* module,
	const mslPipeline* pipeline)
{
	static GLenum stageMap[] =
	{
		GL_VERTEX_SHADER,
		GL_TESS_CONTROL_SHADER,
		GL_TESS_EVALUATION_SHADER,
		GL_GEOMETRY_SHADER,
		GL_FRAGMENT_SHADER,
		GL_COMPUTE_SHADER
	};

	memset(shaderIds, 0, sizeof(GLuint)*mslStage_Count);
	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (pipeline->shaders[i] == MSL_UNKNOWN)
			continue;

		if (!dsGLShaderModule_compileShader(shaderIds + i, module, pipeline->shaders[i],
			stageMap[i], pipeline->name))
		{
			return false;
		}
	}

	return true;
}

static bool setVertexInputs(dsShaderModule* module, const mslPipeline* pipeline,
	uint32_t pipelineIndex, GLuint programId)
{
	for (uint32_t i = 0; i < pipeline->attributeCount; ++i)
	{
		mslAttribute attribute;
		if (!mslModule_attribute(&attribute, module->module, pipelineIndex, i))
		{
			errno = EFORMAT;
			DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG,
				"Invalid vertex attribute for shader %s.%s.", module->name, pipeline->name);
			return false;
		}

		glBindAttribLocation(programId, attribute.location, attribute.name);
	}

	return true;
}

static bool setFragmentOutputs(dsShaderModule* module, const mslPipeline* pipeline,
	uint32_t pipelineIndex, GLuint programId)
{
	if (!ANYGL_SUPPORTED(glBindFragDataLocation))
		return true;

	for (uint32_t i = 0; i < pipeline->fragmentOutputCount; ++i)
	{
		mslFragmentOutput fragmentOutput;
		if (!mslModule_fragmentOutput(&fragmentOutput, module->module, pipelineIndex, i))
		{
			errno = EFORMAT;
			DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG,
				"Invalid vertex pipeline index for shader %s.%s.", module->name, pipeline->name);
			return false;
		}

		glBindFragDataLocation(programId, fragmentOutput.location, fragmentOutput.name);
	}

	return true;
}

static bool compileAndLinkProgram(dsResourceManager* resourceManager, dsShaderModule* module,
	dsGLShader* shader,  const mslPipeline* pipeline, uint32_t pipelineIndex)
{
	// Compile the shaders.
	GLuint shaderIds[mslStage_Count];
	if (!compileShaders(shaderIds, module, pipeline))
		return false;

	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (pipeline->shaders[i] != MSL_UNKNOWN)
			glAttachShader(shader->programId, shaderIds[i]);
	}

	// Set the input and output locations.
	if (!setVertexInputs(module, pipeline, pipelineIndex, shader->programId))
		return false;
	if (!setFragmentOutputs(module, pipeline, pipelineIndex, shader->programId))
		return false;

	// Link the program.
	glLinkProgram(shader->programId);
	GLint linkSuccess = false;
	glGetProgramiv(shader->programId, GL_LINK_STATUS, &linkSuccess);
	if (!linkSuccess)
	{
		errno = EFORMAT;
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error linking shader %s.%s:",
			module->name, pipeline->name);

		GLint logSize = 0;
		glGetProgramiv(shader->programId, GL_INFO_LOG_LENGTH, &logSize);
		if (logSize > 0)
		{
			char* buffer = (char*)dsAllocator_alloc(resourceManager->allocator, logSize);
			if (buffer)
			{
				glGetProgramInfoLog(shader->programId, logSize, &logSize, buffer);
				DS_LOG_ERROR(DS_RENDER_OPENGL_LOG_TAG, buffer);
				DS_VERIFY(dsAllocator_free(resourceManager->allocator, buffer));
			}
		}

		return false;
	}

	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (shaderIds[i])
			glDetachShader(shader->programId, shaderIds[i]);
	}
	return true;
}

static bool isShadowSampler(mslType type)
{
	switch (type)
	{
		case mslType_Sampler1DShadow:
		case mslType_Sampler2DShadow:
		case mslType_Sampler1DArrayShadow:
		case mslType_Sampler2DArrayShadow:
		case mslType_SamplerCubeShadow:
		case mslType_Sampler2DRectShadow:
			return true;
		default:
			return false;
	}
}

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
			float anisotropy = samplerState.maxAnisotropy == MSL_UNKNOWN_FLOAT ?
				shader->defaultAnisotropy : samplerState.maxAnisotropy;
			glSamplerParameterf(shader->samplerIds[i], GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
		}

		if (AnyGL_atLeastVersion(2, 0, false))
		{
			glSamplerParameterf(shader->samplerIds[i], GL_TEXTURE_LOD_BIAS,
				samplerState.mipLodBias == MSL_UNKNOWN_FLOAT ? 0.0f : samplerState.mipLodBias);
		}

		if (AnyGL_atLeastVersion(2, 0, false) || AnyGL_atLeastVersion(3, 0, true))
		{
			glSamplerParameterf(shader->samplerIds[i], GL_TEXTURE_MIN_LOD,
				samplerState.minLod == MSL_UNKNOWN_FLOAT ? -1000.0f : samplerState.minLod);
			glSamplerParameterf(shader->samplerIds[i], GL_TEXTURE_MAX_LOD,
				samplerState.maxLod == MSL_UNKNOWN_FLOAT ? 1000.0f : samplerState.maxLod);
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

		glSamplerParameteri(shader->samplerIds[i], GL_TEXTURE_COMPARE_FUNC,
			dsGetGLCompareOp(samplerState.compareOp));
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

static void setLocation(dsGLUniformInfo* info, GLint location)
{
#if DS_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#endif

	info->location = location;

#if DS_GCC
#pragma GCC diagnostic pop
#endif
}

static bool hookupBindings(dsGLShader* shader, const dsMaterialDesc* materialDesc,
	mslModule* module, uint32_t shaderIndex, bool useGfxBuffers, const char* moduleName)
{
	GLuint prevProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&prevProgram);
	glUseProgram(shader->programId);

	uint32_t blockBindings = 0;
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
				const char* name = materialDesc->elements[i].name;
				uint32_t uniformIndex = findUniform(module, shaderIndex, &shader->pipeline, name);
				if (uniformIndex == MSL_UNKNOWN)
				{
					shader->uniforms[i].location = -1;
					shader->uniforms[i].isShadowSampler = false;
					shader->uniforms[i].samplerIndex = MSL_UNKNOWN;
				}
				else
				{
					GLint binding = glGetUniformLocation(shader->programId, name);
					if (binding < 0)
					{
						shader->uniforms[i].location = -1;
						shader->uniforms[i].isShadowSampler = false;
						shader->uniforms[i].samplerIndex = MSL_UNKNOWN;
						continue;
					}

					mslUniform uniform;
					DS_VERIFY(mslModule_uniform(&uniform, module, shaderIndex, uniformIndex));
					shader->uniforms[i].samplerIndex = uniform.samplerIndex;
					shader->uniforms[i].isShadowSampler = isShadowSampler(uniform.type);
					if (shader->samplerIds && uniform.samplerIndex != MSL_UNKNOWN &&
						shader->uniforms[i].isShadowSampler)
					{
						glSamplerParameteri(shader->samplerIds[uniform.samplerIndex],
							GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
					}

					// Find a free texture index if not explicitly set.
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
								"Ran out of texture indices for shader %s.%s", moduleName,
								shader->pipeline.name);
							errno = EINDEX;
							glUseProgram(prevProgram);
							return false;
						}
					}
					glUniform1i(binding, textureIndex);
					setLocation(shader->uniforms + i, textureIndex);
				}
				break;
			}
			case dsMaterialType_UniformBlock:
			case dsMaterialType_UniformBuffer:
			{
				GLint blockIndex = glGetUniformBlockIndex(shader->programId,
					materialDesc->elements[i].name);
				if (blockIndex >= 0)
				{
					setLocation(shader->uniforms + i, blockBindings);
					glUniformBlockBinding(shader->programId, blockIndex,
						shader->uniforms[i].location);
				}
				else
					shader->uniforms[i].location = -1;
				++blockBindings;
				break;
			}
			case dsMaterialType_VariableGroup:
			{
				if (useGfxBuffers)
				{
					GLint blockIndex = glGetUniformBlockIndex(shader->programId,
						materialDesc->elements[i].name);
					if (blockIndex >= 0)
					{
						setLocation(shader->uniforms + i, blockBindings);
						glUniformBlockBinding(shader->programId, blockIndex,
							shader->uniforms[i].location);
					}
					else
						shader->uniforms[i].location = -1;
					++blockBindings;
				}
				else
				{
					const dsShaderVariableGroupDesc* groupDesc =
						materialDesc->elements[i].shaderVariableGroupDesc;
					DS_ASSERT(groupDesc);
					for (uint32_t j = 0; j < groupDesc->elementCount; ++j)
					{
						int len = snprintf(nameBuffer, DS_BUFFER_SIZE, "uniforms.%s",
							groupDesc->elements[j].name);
						if (len < 0 || len >= DS_BUFFER_SIZE)
						{
							DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG,
								"Uniform name '%s' is too long.", groupDesc->elements[j].name);
							errno = EINDEX;
							glUseProgram(prevProgram);
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
				int len = snprintf(nameBuffer, DS_BUFFER_SIZE, "uniforms.%s",
					materialDesc->elements[i].name);
				if (len < 0 || len >= DS_BUFFER_SIZE)
				{
					DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Uniform name '%s' is too long.",
						materialDesc->elements[i].name);
					errno = EINDEX;
					glUseProgram(prevProgram);
					return false;
				}

				setLocation(shader->uniforms + i,
					glGetUniformLocation(shader->programId, nameBuffer));
				break;
			}
		}
	}

	shader->internalUniform = glGetUniformLocation(shader->programId, "uniforms._dsInternal");
	glUseProgram(prevProgram);
	return true;
}


static void resolveDefaultRasterizationState(mslRasterizationState* state)
{
	if (state->depthClampEnable == mslBool_Unset)
		state->depthClampEnable = mslBool_False;
	if (state->rasterizerDiscardEnable == mslBool_Unset)
		state->rasterizerDiscardEnable = mslBool_False;
	if (state->polygonMode == mslPolygonMode_Unset)
		state->polygonMode = mslPolygonMode_Fill;
	if (state->cullMode == mslCullMode_Unset)
		state->cullMode = mslCullMode_None;
	if (state->frontFace == mslFrontFace_Unset)
		state->frontFace = mslFrontFace_CounterClockwise;
	if (state->depthBiasEnable == mslBool_Unset)
		state->depthBiasEnable = mslBool_False;
	if (state->lineWidth == MSL_UNKNOWN_FLOAT)
		state->lineWidth = 1.0f;
}

static void resolveDefaultMultisampleState(mslMultisampleState* state)
{
	if (state->sampleShadingEnable == mslBool_Unset)
		state->sampleShadingEnable = mslBool_False;
	if (state->minSampleShading == MSL_UNKNOWN_FLOAT)
		state->minSampleShading = 1.0f;
	if (state->sampleMask == MSL_UNKNOWN)
		state->sampleMask = 0xFFFFFFFF;
	if (state->alphaToCoverageEnable == mslBool_Unset)
		state->alphaToCoverageEnable = mslBool_False;
	if (state->alphaToOneEnable == mslBool_Unset)
		state->alphaToOneEnable = mslBool_False;
}

static void resolveDefaultStencilState(mslStencilOpState* state)
{
	if (state->failOp == mslStencilOp_Unset)
		state->failOp = mslStencilOp_Keep;
	if (state->passOp == mslStencilOp_Unset)
		state->passOp = mslStencilOp_Keep;
	if (state->depthFailOp == mslStencilOp_Unset)
		state->depthFailOp = mslStencilOp_Keep;
	if (state->compareOp == mslCompareOp_Unset)
		state->compareOp = mslCompareOp_Always;
}

static void resolveDefaultDepthStencilState(mslDepthStencilState* state)
{
	if (state->depthBoundsTestEnable == mslBool_Unset)
		state->depthTestEnable = mslBool_False;
	if (state->depthWriteEnable == mslBool_Unset)
		state->depthWriteEnable = mslBool_True;
	if (state->depthCompareOp == mslCompareOp_Unset)
		state->depthCompareOp = mslCompareOp_Less;
	if (state->depthBoundsTestEnable == mslBool_Unset)
		state->depthBoundsTestEnable = mslBool_False;
	if (state->stencilTestEnable == mslBool_Unset)
		state->stencilTestEnable = mslBool_False;

	resolveDefaultStencilState(&state->frontStencil);
	resolveDefaultStencilState(&state->frontStencil);
}

static void resolveDefaultBlendState(mslBlendState* state)
{
	if (state->logicalOpEnable == mslBool_Unset)
		state->logicalOpEnable = mslBool_False;
	if (state->logicalOp == mslLogicOp_Unset)
		state->logicalOp = mslLogicOp_Copy;
	if (state->separateAttachmentBlendingEnable == mslBool_Unset)
		state->separateAttachmentBlendingEnable = mslBool_False;
	for (unsigned int i = 0; i < MSL_MAX_ATTACHMENTS; ++i)
	{
		if (state->blendAttachments[i].blendEnable == mslBool_Unset)
			state->blendAttachments[i].blendEnable = mslBool_False;
		if (state->blendAttachments[i].srcColorBlendFactor == mslBlendFactor_Unset)
			state->blendAttachments[i].srcColorBlendFactor = mslBlendFactor_One;
		if (state->blendAttachments[i].dstColorBlendFactor == mslBlendFactor_Unset)
			state->blendAttachments[i].dstColorBlendFactor = mslBlendFactor_Zero;
		if (state->blendAttachments[i].colorBlendOp == mslBlendOp_Unset)
			state->blendAttachments[i].colorBlendOp = mslBlendOp_Add;
		if (state->blendAttachments[i].srcAlphaBlendFactor == mslBlendFactor_Unset)
			state->blendAttachments[i].srcAlphaBlendFactor = mslBlendFactor_One;
		if (state->blendAttachments[i].dstAlphaBlendFactor == mslBlendFactor_Unset)
			state->blendAttachments[i].dstAlphaBlendFactor = mslBlendFactor_Zero;
		if (state->blendAttachments[i].alphaBlendOp == mslBlendOp_Unset)
			state->blendAttachments[i].alphaBlendOp = mslBlendOp_Add;
		if (state->blendAttachments[i].colorWriteMask == mslColorMask_Unset)
		{
			state->blendAttachments[i].colorWriteMask = (mslColorMask)(mslColorMask_Red |
				mslColorMask_Green | mslColorMask_Blue | mslColorMask_Alpha);
		}
	}
}

static void resolveDefaultStates(mslRenderState* state)
{
	resolveDefaultRasterizationState(&state->rasterizationState);
	resolveDefaultMultisampleState(&state->multisampleState);
	resolveDefaultDepthStencilState(&state->depthStencilState);
	resolveDefaultBlendState(&state->blendState);
}

dsShader* dsGLShader_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	dsShaderModule* module, uint32_t shaderIndex, const dsMaterialDesc* materialDesc)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(allocator);
	DS_ASSERT(module);
	DS_ASSERT(materialDesc);

	mslPipeline pipeline;
	DS_VERIFY(mslModule_pipeline(&pipeline, module->module, shaderIndex));
	mslStruct pushConstantStruct = {NULL, 0, 0};
	if (pipeline.pushConstantStruct != MSL_UNKNOWN)
	{
		DS_VERIFY(mslModule_struct(&pushConstantStruct, module->module, shaderIndex,
			pipeline.pushConstantStruct));
	}

	bool hasSamplers = ANYGL_SUPPORTED(glGenSamplers);
	bool useGfxBuffers = dsShaderVariableGroup_useGfxBuffer(resourceManager);
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsGLShader)) +
		DS_ALIGNED_SIZE(sizeof(mslSamplerState)*pipeline.samplerStateCount) +
		DS_ALIGNED_SIZE(sizeof(dsGLUniformInfo)*materialDesc->elementCount);
	if (hasSamplers)
		fullSize += DS_ALIGNED_SIZE(sizeof(GLuint)*pipeline.samplerStateCount);
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
	dsGLShader* shader = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc, dsGLShader);
	DS_ASSERT(shader);

	dsShader* baseShader = (dsShader*)shader;
	baseShader->resourceManager = resourceManager;
	baseShader->allocator = dsAllocator_keepPointer(allocator);
	baseShader->module = module;
	baseShader->name = pipeline.name;
	baseShader->pipelineIndex = shaderIndex;
	baseShader->pipeline = &shader->pipeline;
	baseShader->materialDesc = materialDesc;

	bool prevChecksEnabled = AnyGL_getErrorCheckingEnabled();
	AnyGL_setErrorCheckingEnabled(false);
	dsClearGLErrors();

	dsGLResource_initialize(&shader->resource);
	shader->pipeline = pipeline;
	shader->defaultAnisotropy = resourceManager->renderer->defaultAnisotropy;
	shader->programId = 0;
	if (hasSamplers && pipeline.samplerStateCount > 0)
	{
		shader->samplerIds = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			GLuint, pipeline.samplerStateCount + 1);
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

	if (pipeline.samplerStateCount > 0)
	{
		shader->samplerStates = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
			mslSamplerState, pipeline.samplerStateCount);
		DS_ASSERT(shader->samplerStates);
		for (uint32_t i = 0; i < pipeline.samplerStateCount; ++i)
		{
			DS_VERIFY(mslModule_samplerState(shader->samplerStates + i, module->module, shaderIndex,
				i));
		}
	}
	else
		shader->samplerStates = NULL;

	if (materialDesc->elementCount > 0)
	{
		shader->uniforms = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, dsGLUniformInfo,
			materialDesc->elementCount);
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
					shader->uniforms[i].groupLocations = DS_ALLOCATE_OBJECT_ARRAY(
						(dsAllocator*)&bufferAlloc, GLint, groupDesc->elementCount);
					DS_ASSERT(shader->uniforms[i].groupLocations);
					memset(shader->uniforms[i].groupLocations, 0xFF,
						sizeof(GLint)*groupDesc->elementCount);
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
		DS_LOG_ERROR_F(DS_RENDER_OPENGL_LOG_TAG, "Error creating program %s.%s: %s", module->name,
			pipeline.name, AnyGL_errorString(error));
		errno = dsGetGLErrno(error);
		dsGLShader_destroy(resourceManager, baseShader);
		AnyGL_setErrorCheckingEnabled(prevChecksEnabled);
		return NULL;
	}

	dsGLRenderer* renderer = (dsGLRenderer*)resourceManager->renderer;
	const char* shaderCacheDir = renderer->options.shaderCacheDir;
	bool readShader = false;
	uint64_t shaderHash[2];
	if (shaderCacheDir && ANYGL_SUPPORTED(glProgramBinary))
	{
		hashShader(shaderHash, module->module, &pipeline);
		int prevErrno = errno;
		readShader = loadShader(resourceManager, shaderCacheDir, module->name, pipeline.name,
			shader->programId, shaderHash);
		errno = prevErrno;
	}

	// Compile and link the shader if it wasn't read.
	bool success = readShader;
	if (!success)
		success = compileAndLinkProgram(resourceManager, module, shader, &pipeline, shaderIndex);

	AnyGL_setErrorCheckingEnabled(prevChecksEnabled);
	if (!success)
	{
		dsGLShader_destroy(resourceManager, baseShader);
		return NULL;
	}

	// Set up the samplers and uniform bindings.
	if (hasSamplers)
		createSamplers(shader, module->module, shaderIndex);
	if (!hookupBindings(shader, materialDesc, module->module, shaderIndex, useGfxBuffers,
		module->name))
	{
		dsGLShader_destroy(resourceManager, baseShader);
		return NULL;
	}

	// Set up the render states.
	DS_VERIFY(mslModule_renderState(&shader->renderState, module->module, shaderIndex));
	resolveDefaultStates(&shader->renderState);

	// Write the shader if caching is enabled and didn't read it before.
	if (shaderCacheDir && ANYGL_SUPPORTED(glProgramBinary) && !readShader)
	{
		int prevErrno = errno;
		writeShader(resourceManager, shaderCacheDir, module->name, pipeline.name, shader->programId,
			shaderHash);
		errno = prevErrno;
	}

	// Make sure it's visible from the main render thread.
	if (!dsThread_equal(resourceManager->renderer->mainThread, dsThread_thisThreadID()))
		glFlush();

	return baseShader;
}

bool dsGLShader_isUniformInternal(dsResourceManager* resourceManager, const char* name)
{
	DS_UNUSED(resourceManager);
	return strcmp(name, "_dsInternal") == 0;
}

bool dsGLShader_bind(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsMaterial* material,
	const dsVolatileMaterialValues* volatileValues, const dsDynamicRenderStates* renderStates)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(shader);
	DS_ASSERT(material);

	return dsGLCommandBuffer_bindShaderAndMaterial(commandBuffer, shader, material, volatileValues,
		renderStates);
}

bool dsGLShader_updateVolatileValues(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsVolatileMaterialValues* volatileValues)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(shader);

	return dsGLCommandBuffer_setVolatileMaterialValues(commandBuffer, shader, volatileValues);
}

bool dsGLShader_unbind(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(shader);

	return dsGLCommandBuffer_unbindShader(commandBuffer, shader);
}

bool dsGLShader_bindCompute(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsMaterial* material,
	const dsVolatileMaterialValues* volatileValues)
{
	DS_ASSERT(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(shader);
	DS_ASSERT(material);

	return dsGLCommandBuffer_bindComputeShaderAndMaterial(commandBuffer, shader, material,
		volatileValues);
}

bool dsGLShader_updateComputeVolatileValues(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsVolatileMaterialValues* volatileValues)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(shader);

	return dsGLCommandBuffer_setComputeVolatileMaterialValues(commandBuffer, shader,
		volatileValues);
}

bool dsGLShader_unbindCompute(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader)
{
	DS_UNUSED(resourceManager);
	DS_ASSERT(commandBuffer);
	DS_ASSERT(shader);

	return dsGLCommandBuffer_unbindComputeShader(commandBuffer, shader);
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

	dsGLShader* glShader = (dsGLShader*)shader;
	if (dsGLResource_destroy(&glShader->resource))
		return destroyImpl(shader);

	return true;
}

void dsGLShader_addInternalRef(dsShader* shader)
{
	DS_ASSERT(shader);
	dsGLShader* glShader = (dsGLShader*)shader;
	dsGLMaterialDesc_addInternalRef((dsMaterialDesc*)shader->materialDesc);
	dsGLResource_addRef(&glShader->resource);
}

void dsGLShader_freeInternalRef(dsShader* shader)
{
	DS_ASSERT(shader);
	dsGLShader* glShader = (dsGLShader*)shader;
	dsGLMaterialDesc_freeInternalRef((dsMaterialDesc*)shader->materialDesc);
	if (dsGLResource_freeRef(&glShader->resource))
		destroyImpl(shader);
}

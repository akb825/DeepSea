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

#pragma once

#include <DeepSea/Core/Config.h>

#include "AnyGL/gl.h"
#include <DeepSea/RenderOpenGL/Types.h>
#include <DeepSea/Core/Types.h>

struct dsResourceContext
{
	void* context;
	void* dummySurface;
	void* dummyOsSurface;
	bool claimed;
};

typedef struct dsGLResource
{
	uint32_t internalRef;
	dsSpinlock lock;
	bool defferDestroy;
} dsGLResource;

typedef struct dsGLGfxBuffer
{
	dsGfxBuffer buffer;
	GLuint bufferId;
	dsGLResource resource;
} dsGLGfxBuffer;

typedef struct dsGLResourceManager
{
	dsResourceManager resourceManager;
	dsResourceContext* resourceContexts;
	dsMutex* mutex;

	int standardFormats[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
	int specialFormats[dsGfxFormat_SpecialCount];
	int compressedFormats[dsGfxFormat_CompressedCount][dsGfxFormat_DecoratorCount];

	GLenum standardInternalFormats[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
	GLenum specialInternalFormats[dsGfxFormat_SpecialCount];
	GLenum compressedInternalFormats[dsGfxFormat_CompressedCount][dsGfxFormat_DecoratorCount];

	GLenum standardGlFormats[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
	GLenum specialGlFormats[dsGfxFormat_SpecialCount];
	GLenum compressedGlFormats[dsGfxFormat_CompressedCount][dsGfxFormat_DecoratorCount];

	GLenum standardTypes[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
	GLenum specialTypes[dsGfxFormat_SpecialCount];
} dsGLResourceManager;

typedef struct dsGLRenderer
{
	dsRenderer renderer;
	dsOpenGLOptions options;
	bool releaseDisplay;

	void* sharedConfig;
	void* sharedContext;
	void* dummySurface;
	void* dummyOsSurface;
	void* renderConfig;
	void* renderContext;
} dsGLRenderer;

typedef bool (*GLCopyGfxBufferDataFunction)(dsCommandBuffer* commandBuffer, dsGfxBuffer* buffer,
	size_t offset, const void* data, size_t size);
typedef bool (*GLCopyGfxBufferFunction)(dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer,
	size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset, size_t size);
typedef bool (*GLSubmitCommandBufferFunction)(dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer);

typedef struct CommandBufferFunctionTable
{
	GLCopyGfxBufferDataFunction copyBufferDataFunc;
	GLCopyGfxBufferFunction copyBufferFunc;
	GLSubmitCommandBufferFunction submitFunc;
} CommandBufferFunctionTable;

typedef struct dsGLCommandBuffer
{
	dsCommandBuffer commandBuffer;
	const CommandBufferFunctionTable* functions;
} dsGLCommandBuffer;

typedef struct dsGLMainCommandBuffer dsGLMainCommandBuffer;
typedef struct dsGLOtherCommandBuffer dsGLOtherCommandBuffer;

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
	dsGLResource resource;
	GLuint bufferId;
} dsGLGfxBuffer;

typedef struct dsGLDrawGeometry
{
	dsDrawGeometry drawGeometry;
	dsGLResource resource;
	GLuint vao;
	uint32_t vaoContext;
} dsGLDrawGeometry;

typedef struct dsGLTexture
{
	dsTexture texture;
	dsGLResource resource;
	GLuint textureId;
	GLuint drawBufferId;
} dsGLTexture;

typedef struct dsGLRenderbuffer
{
	dsRenderbuffer renderbuffer;
	dsGLResource resource;
	GLuint renderbufferId;
} dsGLRenderbuffer;

typedef struct dsGLFramebuffer
{
	dsFramebuffer framebuffer;
	dsGLResource resource;
	GLuint framebufferId;
	uint32_t fboContext;
	bool defaultFramebuffer;
} dsGLFramebuffer;

typedef struct dsGLFenceSync
{
	dsAllocator* allocator;
	GLsync glSync;
	uint32_t refCount;
} dsGLFenceSync;

typedef struct dsGLFenceSyncRef
{
	dsAllocator* allocator;
	dsGLFenceSync* sync;
	uint32_t refCount;
} dsGLFenceSyncRef;

typedef struct dsGLGfxFence
{
	dsGfxFence fence;
	dsSpinlock lock;
	dsGLFenceSyncRef* sync;
} dsGLGfxFence;

typedef struct dsGLResourceManager
{
	dsResourceManager resourceManager;
	dsResourceContext* resourceContexts;
	dsMutex* mutex;

	uint8_t standardFormats[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
	uint8_t specialFormats[dsGfxFormat_SpecialCount];
	uint8_t compressedFormats[dsGfxFormat_CompressedCount][dsGfxFormat_DecoratorCount];

	GLenum standardInternalFormats[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
	GLenum specialInternalFormats[dsGfxFormat_SpecialCount];
	GLenum compressedInternalFormats[dsGfxFormat_CompressedCount][dsGfxFormat_DecoratorCount];

	GLenum standardGlFormats[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
	GLenum specialGlFormats[dsGfxFormat_SpecialCount];
	GLenum compressedGlFormats[dsGfxFormat_CompressedCount][dsGfxFormat_DecoratorCount];

	GLenum standardTypes[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
	GLenum specialTypes[dsGfxFormat_SpecialCount];

	GLenum standardVertexFormats[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
	GLenum specialVertexFormats[dsGfxFormat_SpecialCount];

	GLint standardVertexElements[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
	GLint specialVertexElements[dsGfxFormat_StandardCount];
} dsGLResourceManager;

typedef struct dsGLRenderer
{
	dsRenderer renderer;
	dsOpenGLOptions options;
	bool releaseDisplay;

	bool renderContextBound;
	uint32_t contextCount;
	void* sharedConfig;
	void* sharedContext;
	void* dummySurface;
	void* dummyOsSurface;
	void* renderConfig;
	void* renderContext;
	dsMutex* contextMutex;

	GLuint* destroyVaos;
	size_t maxDestroyVaos;
	size_t curDestroyVaos;
	bool boundAttributes[DS_MAX_ALLOWED_VERTEX_ATTRIBS];

	GLuint* destroyFbos;
	size_t maxDestroyFbos;
	size_t curDestroyFbos;

	GLuint tempFramebuffer;
	GLuint tempCopyFramebuffer;

	dsPoolAllocator* syncPools;
	size_t curSyncPools;
	size_t maxSyncPools;
	dsSpinlock syncPoolLock;

	dsPoolAllocator* syncRefPools;
	size_t curSyncRefPools;
	size_t maxSyncRefPools;
	dsSpinlock syncRefPoolLock;
} dsGLRenderer;

typedef bool (*GLCopyGfxBufferDataFunction)(dsCommandBuffer* commandBuffer, dsGfxBuffer* buffer,
	size_t offset, const void* data, size_t size);
typedef bool (*GLCopyGfxBufferFunction)(dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer,
	size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset, size_t size);

typedef bool (*GLCopyTextureDataFunction)(dsCommandBuffer* commandBuffer, dsTexture* texture,
	const dsTexturePosition* position, uint32_t width, uint32_t height, uint32_t layers,
	const void* data, size_t size);
typedef bool (*GLCopyTextureFunction)(dsCommandBuffer* commandBuffer, dsTexture* srcTexture,
	dsTexture* dstTexture, const dsTextureCopyRegion* regions, size_t regionCount);
typedef bool (*GLBlitTextureFunction)(dsCommandBuffer* commandBuffer, dsTexture* srcTexture,
	dsTexture* dstTexture, const dsTextureBlitRegion* regions,
	size_t regionCount, dsBlitFilter filter);

typedef bool (*GLSetFenceSyncsFunction)(dsCommandBuffer* commandBuffer, dsGLFenceSyncRef** syncs,
	size_t syncCount, bool bufferReadback);

typedef bool (*GLSubmitCommandBufferFunction)(dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer);

typedef struct CommandBufferFunctionTable
{
	GLCopyGfxBufferDataFunction copyBufferDataFunc;
	GLCopyGfxBufferFunction copyBufferFunc;

	GLCopyTextureDataFunction copyTextureDataFunc;
	GLCopyTextureFunction copyTextureFunc;
	GLBlitTextureFunction blitTextureFunc;

	GLSetFenceSyncsFunction setFenceSyncsFunc;

	GLSubmitCommandBufferFunction submitFunc;
} CommandBufferFunctionTable;

typedef struct dsGLCommandBuffer
{
	dsCommandBuffer commandBuffer;
	const CommandBufferFunctionTable* functions;
} dsGLCommandBuffer;

typedef struct dsGLMainCommandBuffer dsGLMainCommandBuffer;
typedef struct dsGLOtherCommandBuffer dsGLOtherCommandBuffer;

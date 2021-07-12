/*
 * Copyright 2017-2021 Aaron Barany
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
#include <MSL/Client/TypesC.h>
#include <DeepSea/Core/Types.h>
#include <DeepSea/Render/Types.h>
#include <DeepSea/RenderOpenGL/GLRenderer.h>

typedef enum GLSurfaceType
{
	GLSurfaceType_None,
	GLSurfaceType_Left,
	GLSurfaceType_Right,
	GLSurfaceType_Framebuffer,
	GLSurfaceType_CubeFramebuffer
} GLSurfaceType;

typedef enum GLFramebufferFlags
{
	GLFramebufferFlags_Default = 0,
	GLFramebufferFlags_Read = 0x1,
	GLFramebufferFlags_Temporary = 0x2
} GLFramebufferFlags;

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
	bool deferDestroy;
} dsGLResource;

typedef struct dsGLGfxBuffer
{
	dsGfxBuffer buffer;
	dsGLResource resource;
	GLuint bufferId;

	dsSpinlock mapLock;
	dsGfxBufferMap mapFlags;
	bool emulatedMap;
	dsAllocator* scratchAllocator;
	void* mappedBuffer;
	size_t mappedOffset;
	size_t mappedSize;
	size_t mappedBufferCapacity;
} dsGLGfxBuffer;

typedef struct dsGLDrawGeometry
{
	dsDrawGeometry drawGeometry;
	dsGLResource resource;
	GLuint vao;
	uint32_t vaoContext;
	int32_t lastBaseVertex;
} dsGLDrawGeometry;

typedef struct dsGLTexture
{
	dsTexture texture;
	dsGLResource resource;
	GLuint textureId;
	GLuint drawBufferId;

	GLenum minFilter;
	GLenum magFilter;
	GLenum addressModeS;
	GLenum addressModeT;
	GLenum addressModeR;
	float anisotropy;
	float mipLodBias;
	float minLod;
	float maxLod;
	mslBorderColor borderColor;
	bool compareEnabled;
	GLenum compareOp;
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
	GLuint curColorAttachments[DS_MAX_ATTACHMENTS];
	GLuint curColorAttachmentCount;
	GLuint curDepthAttachment;
	uint32_t curDefaultSamples;
	bool framebufferError;
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

typedef struct dsGLGfxQueryPool
{
	dsGfxQueryPool queries;
	dsGLResource resource;
	uint32_t queryContext;
	GLuint queryIds[];
} dsGLGfxQueryPool;

typedef struct dsGLShaderModule
{
	dsShaderModule shaderModule;
	GLuint* shaders;
} dsGLShaderModule;

typedef struct dsGLMaterialDesc
{
	dsMaterialDesc materialDesc;
	dsGLResource resource;
} dsGLMaterialDesc;

typedef struct dsGLShaderVariableGroupDesc
{
	dsShaderVariableGroupDesc shaderVariableGroupDesc;
	dsGLResource resource;
} dsGLShaderVariableGroupDesc;

typedef union dsGLUniformInfo
{
	struct
	{
		GLint location : 31;
		uint32_t isShadowSampler : 1;
		GLuint samplerIndex;
	};
	GLint* groupLocations;
} dsGLUniformInfo;

typedef struct dsGLShader
{
	dsShader shader;
	dsGLResource resource;
	mslPipeline pipeline;
	mslRenderState renderState;
	GLuint programId;
	GLint internalUniform;
	GLuint* samplerIds;
	mslSamplerState* samplerStates;
	dsGLUniformInfo* uniforms;
	float defaultAnisotropy;
} dsGLShader;

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
	dsRendererOptions options;
	bool releaseDisplay;

	bool renderContextBound;
	bool renderContextReset;
	uint32_t contextCount;
	void* sharedConfig;
	void* sharedContext;
	void* dummySurface;
	void* dummyOsSurface;
	void* renderConfig;
	void* renderContext;
	dsMutex* contextMutex;

	GLuint* destroyVaos;
	uint32_t maxDestroyVaos;
	uint32_t curDestroyVaos;
	bool boundAttributes[DS_MAX_ALLOWED_VERTEX_ATTRIBS];

	GLuint* destroyFbos;
	uint32_t maxDestroyFbos;
	uint32_t curDestroyFbos;

	GLuint sharedTempFramebuffer;
	GLuint sharedTempCopyFramebuffer;
	GLuint tempFramebuffer;
	GLuint tempCopyFramebuffer;

	dsPoolAllocator* syncPools;
	uint32_t curSyncPools;
	uint32_t maxSyncPools;
	dsSpinlock syncPoolLock;

	dsPoolAllocator* syncRefPools;
	uint32_t curSyncRefPools;
	uint32_t maxSyncRefPools;
	dsSpinlock syncRefPoolLock;

	void* curGLSurface;
	bool curGLSurfaceVSync;
	GLenum curTexture0Target;
	GLuint curTexture0;

	GLSurfaceType curSurfaceType;
	GLuint curFbo;
} dsGLRenderer;

typedef void (*GLResetCommandBufferFunction)(dsCommandBuffer* commandBuffer);

typedef bool (*GLCopyGfxBufferDataFunction)(dsCommandBuffer* commandBuffer, dsGfxBuffer* buffer,
	size_t offset, const void* data, size_t size);
typedef bool (*GLCopyGfxBufferFunction)(dsCommandBuffer* commandBuffer, dsGfxBuffer* srcBuffer,
	size_t srcOffset, dsGfxBuffer* dstBuffer, size_t dstOffset, size_t size);
typedef bool (*GLCopyGfxBufferToTextureFunction)(dsCommandBuffer* commandBuffer,
	dsGfxBuffer* srcBuffer, dsTexture* dstTexture, const dsGfxBufferTextureCopyRegion* regions,
	uint32_t regionCount);

typedef bool (*GLCopyTextureDataFunction)(dsCommandBuffer* commandBuffer, dsTexture* texture,
	const dsTexturePosition* position, uint32_t width, uint32_t height, uint32_t layers,
	const void* data, size_t size);
typedef bool (*GLCopyTextureFunction)(dsCommandBuffer* commandBuffer, dsTexture* srcTexture,
	dsTexture* dstTexture, const dsTextureCopyRegion* regions, uint32_t regionCount);
typedef bool (*GLCopyTextureToGfxBufferFunction)(dsCommandBuffer* commandBuffer,
	dsTexture* srcTexture, dsGfxBuffer* dstBuffer, const dsGfxBufferTextureCopyRegion* regions,
	uint32_t regionCount);
typedef bool (*GLGenerateTextureMipmaps)(dsCommandBuffer* commandBuffer, dsTexture* texture);

typedef bool (*GLSetFenceSyncsFunction)(dsCommandBuffer* commandBuffer, dsGLFenceSyncRef** syncs,
	uint32_t syncCount, bool bufferReadback);

typedef bool (*GLBeginEndQueryFunction)(dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries,
	uint32_t query);
typedef bool (*GLQueryTimestampFunction)(dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries,
	uint32_t query);
typedef bool (*GLCopyQueryValuesFunction)(dsCommandBuffer* commandBuffer, dsGfxQueryPool* queries,
	uint32_t first, uint32_t count, dsGfxBuffer* buffer, size_t offset, size_t stride,
	size_t elementSize, bool checkAvailability);

typedef bool (*GLBindShaderFunctiion)(dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsDynamicRenderStates* renderStates);
typedef bool (*GLSetTextureFunction)(dsCommandBuffer* commandBuffer, const dsShader* shader,
	uint32_t element, dsTexture* texture);
typedef bool (*GLSetTextureBufferFunction)(dsCommandBuffer* commandBuffer, const dsShader* shader,
	uint32_t element, dsGfxBuffer* buffer, dsGfxFormat format, size_t offset, size_t count);
typedef bool (*GLSetShaderBufferFunction)(dsCommandBuffer* commandBuffer, const dsShader* shader,
	uint32_t element, dsGfxBuffer* buffer, size_t offset, size_t size);
typedef bool (*GLSetUniformFunction)(dsCommandBuffer* commandBuffer, GLint location,
	dsMaterialType type, uint32_t count, const void* data);
typedef bool (*GLUpdateDynamicRenderStatesFunctiion)(dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsDynamicRenderStates* renderStates);
typedef bool (*GLUnbindShaderFunction)(dsCommandBuffer* commandBuffer, const dsShader* shader);

typedef bool (*GLBindComputeShaderFunctiion)(dsCommandBuffer* commandBuffer,
	const dsShader* shader);

typedef bool (*GLBeginRenderSurfaceFunction)(dsCommandBuffer* commandBuffer, void* glSurface);
typedef bool (*GLEndRenderSurfaceFunction)(dsCommandBuffer* commandBuffer, void* glSurface);

typedef bool (*GLBeginRenderPassFunction)(dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, const dsFramebuffer* framebuffer,
	const dsAlignedBox3f* viewport, const dsSurfaceClearValue* clearValues,
	uint32_t clearValueCount);
typedef bool (*GLNextRenderSubpassFunction)(dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass, uint32_t subpassIndex);
typedef bool (*GLEndRenderPassFunction)(dsCommandBuffer* commandBuffer,
	const dsRenderPass* renderPass);

typedef bool (*GLSetViewportFunction)(dsCommandBuffer* commandBuffer,
	const dsAlignedBox3f* viewport);
typedef bool (*GLClearAttachmentsFunction)(dsCommandBuffer* commandBuffer,
	const dsClearAttachment* attachments, uint32_t attachmentCount,
	const dsAttachmentClearRegion* regions, uint32_t regionCount);
typedef bool (*GLDrawFunction)(dsCommandBuffer* commandBuffer, const dsDrawGeometry* geometry,
	const dsDrawRange* drawRange, dsPrimitiveType primitiveType);
typedef bool (*GLDrawIndexedFunction)(dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsDrawIndexedRange* drawRange,
	dsPrimitiveType primitiveType);
typedef bool (*GLDrawIndirectFunction)(dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType);
typedef bool (*GLDrawIndexedIndirectFunction)(dsCommandBuffer* commandBuffer,
	const dsDrawGeometry* geometry, const dsGfxBuffer* indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType);
typedef bool (*GLDispatchComputeFunction)(dsCommandBuffer* commandBuffer, uint32_t x, uint32_t y,
	uint32_t z);
typedef bool (*GLDispatchComputeIndirectFunction)(dsCommandBuffer* commandBuffer,
	const dsGfxBuffer* indirectBuffer, size_t offset);
typedef bool (*GLBlitSurfaceFunction)(dsCommandBuffer* commandBuffer,
	dsGfxSurfaceType srcSurfaceType, void* srcSurface, dsGfxSurfaceType dstSurfaceType,
	void* dstSurface, const dsSurfaceBlitRegion* regions, uint32_t regionCount,
	dsBlitFilter filter);

typedef bool (*GLPushDebugGroupFunction)(dsCommandBuffer* commandBuffer, const char* name);
typedef bool (*GLPopDebugGroupFunction)(dsCommandBuffer* commandBuffer);

typedef bool (*GLGfxMemoryBarrierFunction)(dsCommandBuffer* commandBuffer,
	dsGfxPipelineStage beforeStages, dsGfxPipelineStage afterStages,
	const dsGfxMemoryBarrier* barriers, uint32_t barrierCount);

typedef bool (*GLSubmitCommandBufferFunction)(dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer);

typedef struct CommandBufferFunctionTable
{
	GLResetCommandBufferFunction resetCommandBuffer;

	GLCopyGfxBufferDataFunction copyBufferDataFunc;
	GLCopyGfxBufferFunction copyBufferFunc;
	GLCopyGfxBufferToTextureFunction copyBufferToTextureFunc;

	GLCopyTextureDataFunction copyTextureDataFunc;
	GLCopyTextureFunction copyTextureFunc;
	GLCopyTextureToGfxBufferFunction copyTextureToBufferFunc;
	GLGenerateTextureMipmaps generateTextureMipmapsFunc;

	GLSetFenceSyncsFunction setFenceSyncsFunc;

	GLBeginEndQueryFunction beginQueryFunc;
	GLBeginEndQueryFunction endQueryFunc;
	GLQueryTimestampFunction queryTimestampFunc;
	GLCopyQueryValuesFunction copyQueryValuesFunc;

	GLBindShaderFunctiion bindShaderFunc;
	GLSetTextureFunction setTextureFunc;
	GLSetTextureBufferFunction setTextureBufferFunc;
	GLSetShaderBufferFunction setShaderBufferFunc;
	GLSetUniformFunction setUniformFunc;
	GLUpdateDynamicRenderStatesFunctiion updateDynamicRenderStatesFunc;
	GLUnbindShaderFunction unbindShaderFunc;

	GLBindComputeShaderFunctiion bindComputeShaderFunc;
	GLUnbindShaderFunction unbindComputeShaderFunc;

	GLBeginRenderSurfaceFunction beginRenderSurfaceFunc;
	GLEndRenderSurfaceFunction endRenderSurfaceFunc;

	GLBeginRenderPassFunction beginRenderPassFunc;
	GLNextRenderSubpassFunction nextRenderSubpassFunc;
	GLEndRenderPassFunction endRenderPassFunc;

	GLSetViewportFunction setViewportFunc;
	GLClearAttachmentsFunction clearAttachmentsFunc;
	GLDrawFunction drawFunc;
	GLDrawIndexedFunction drawIndexedFunc;
	GLDrawIndirectFunction drawIndirectFunc;
	GLDrawIndexedIndirectFunction drawIndexedIndirectFunc;
	GLDispatchComputeFunction dispatchComputeFunc;
	GLDispatchComputeIndirectFunction dispatchComputeIndirectFunc;
	GLBlitSurfaceFunction blitSurfaceFunc;

	GLPushDebugGroupFunction pushDebugGroupFunc;
	GLPopDebugGroupFunction popDebugGroupFunc;

	GLGfxMemoryBarrierFunction memoryBarrierFunc;

	GLSubmitCommandBufferFunction submitFunc;
} CommandBufferFunctionTable;

typedef struct dsCommitCountInfo
{
	const dsShaderVariableGroup* variableGroup;
	uint64_t commitCount;
} dsCommitCountInfo;

typedef struct dsGLCommandBuffer
{
	dsCommandBuffer commandBuffer;
	const CommandBufferFunctionTable* functions;

	dsCommitCountInfo* commitCounts;
	uint32_t commitCountSize;

	void* boundSurface;
} dsGLCommandBuffer;

typedef struct dsGLMainCommandBuffer dsGLMainCommandBuffer;
typedef struct dsGLOtherCommandBuffer dsGLOtherCommandBuffer;

typedef struct dsGLRenderSurface
{
	dsRenderSurface renderSurface;
	void* glSurface;
} dsGLRenderSurface;

typedef struct dsGLRenderPass
{
	dsRenderPass renderPass;
	dsGLResource resource;
	uint32_t* clearSubpass;
} dsGLRenderPass;

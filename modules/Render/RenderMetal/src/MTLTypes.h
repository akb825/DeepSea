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

#pragma once

#include <DeepSea/Core/Config.h>
#include <DeepSea/Render/Types.h>
#include <DeepSea/RenderMetal/RendererIDs.h>

#include <MSL/Client/TypesC.h>

#import <Foundation/NSObject.h>
#import <Metal/MTLArgument.h>
#import <Metal/MTLCommandBuffer.h>
#import <Metal/MTLDevice.h>
#import <Metal/MTLPixelFormat.h>
#import <Metal/MTLRenderPass.h>
#import <Metal/MTLRenderCommandEncoder.h>
#import <Metal/MTLVertexDescriptor.h>

// This is used by SPIRV-Cross to create the shader.
#define DS_IMAGE_BUFFER_WIDTH 4096
#define DS_NOT_SUBMITTED ((uint64_t)-1)
// 10 seconds in milliseconds
#define DS_DEFAULT_WAIT_TIMEOUT 10000
#define DS_RECENTLY_ADDED_SIZE 10
#define DS_TEMP_BUFFER_CAPACITY 524288
#define DS_MAX_TEMP_BUFFER_ALLOC 262144

typedef struct dsMTLBufferTexture
{
	CFTypeRef mtlTexture;
	dsGfxFormat format;
	size_t offset;
	size_t count;
} dsMTLBufferTexture;

typedef struct dsMTLGfxBufferData
{
	dsResourceManager* resourceManager;
	dsAllocator* allocator;
	dsAllocator* scratchAllocator;
	dsLifetime* lifetime;

	CFTypeRef mtlBuffer;
	CFTypeRef copyBuffer;
	uint64_t lastUsedSubmit;
	uint32_t processed;

	dsSpinlock bufferTextureLock;
	dsGfxBufferUsage usage;
	dsMTLBufferTexture* bufferTextures;
	uint32_t bufferTextureCount;
	uint32_t maxBufferTextures;

	size_t mappedStart;
	size_t mappedSize;
	bool mappedWrite;

	bool managed;
	uint32_t used;
} dsMTLGfxBufferData;

typedef struct dsMTLGfxBuffer
{
	dsGfxBuffer buffer;
	dsSpinlock lock;
	dsMTLGfxBufferData* bufferData;
} dsMTLGfxBuffer;

typedef struct dsMTLDrawGeometry
{
	dsDrawGeometry drawGeometry;
	uint32_t vertexHash;
} dsMTLDrawGeometry;

typedef struct dsMTLTexture
{
	dsTexture texture;
	dsLifetime* lifetime;

	CFTypeRef mtlTexture;
	CFTypeRef copyTexture;
	CFTypeRef resolveTexture;

	CFTypeRef stencilTexture;
	CFTypeRef resolveStencilTexture;

	uint64_t lastUsedSubmit;
	uint32_t processed;
} dsMTLTexture;

typedef struct dsMTLRenderbuffer
{
	dsRenderbuffer renderbuffer;
	CFTypeRef surface;
	CFTypeRef stencilSurface;
} dsMTLRenderbuffer;

typedef struct dsMTLGfxFence
{
	dsGfxFence fence;
	dsLifetime* lifetime;
	uint64_t lastUsedSubmit;
} dsMTLGfxFence;

typedef struct dsMTLShaderModule
{
	dsShaderModule module;
	CFTypeRef* shaders;
} dsMTLShaderModule;

typedef struct dsMTLShaderStageInfo
{
	CFTypeRef function;
	uint32_t* uniformIndices;
	bool hasPushConstants;
} dsMTLShaderStageInfo;

typedef struct dsMTLPipeline
{
	dsAllocator* allocator;
	CFTypeRef pipeline;

	uint32_t hash;
	uint32_t samples;
	dsPrimitiveType primitiveType;
	dsVertexFormat formats[DS_MAX_GEOMETRY_VERTEX_BUFFERS];
	dsLifetime* renderPass;
	uint32_t subpass;
} dsMTLPipeline;

typedef struct dsMTLUniformInfo
{
	uint32_t element;
	uint32_t sampler;
} dsMTLUniformInfo;

typedef struct dsMTLPushConstantInfo
{
	uint32_t element;
	uint32_t offset;
	uint32_t count;
	uint32_t stride;
} dsMTLPushConstantInfo;

typedef struct dsMTLShader
{
	dsShader shader;
	dsAllocator* scratchAllocator;
	dsLifetime* lifetime;

	mslPipeline pipeline;
	dsMTLShaderStageInfo stages[mslStage_Count];
	mslRenderState renderState;
	CFTypeRef depthStencilState;
	CFTypeRef* samplers;
	float defaultAnisotropy;
	uint32_t firstVertexBuffer;
	dsSpinlock samplerLock;

	dsMTLUniformInfo* uniformInfos;
	uint32_t uniformCount;

	dsMTLPushConstantInfo* pushConstantInfos;
	uint32_t pushConstantCount;
	uint32_t pushConstantSize;

	dsLifetime** usedRenderPasses;
	uint32_t usedRenderPassCount;
	uint32_t maxUsedRenderPasses;

	dsMTLPipeline** pipelines;
	uint32_t pipelineCount;
	uint32_t maxPipelines;
	dsSpinlock pipelineLock;

	CFTypeRef computePipeline;
} dsMTLShader;

typedef struct dsMTLAttachmentInfo
{
	MTLLoadAction loadAction;
	MTLStoreAction storeAction;
} dsMTLAttachmentInfo;

typedef struct dsMTLSubpassInfo
{
	dsMTLAttachmentInfo* colorAttachments;
	dsMTLAttachmentInfo depthStencilAttachment;
} dsMTLSubpassInfo;

typedef struct dsMTLRenderPass
{
	dsRenderPass renderPass;
	dsAllocator* scratchAllocator;
	dsLifetime* lifetime;

	dsMTLSubpassInfo* subpassInfos;

	dsLifetime** usedShaders;
	uint32_t usedShaderCount;
	uint32_t maxUsedShaders;
	dsSpinlock shaderLock;
} dsMTLRenderPass;

typedef struct dsMTLTempBuffer
{
	dsAllocator* allocator;
	dsLifetime* lifetime;

	CFTypeRef mtlBuffer;
	uint8_t* contents;
	uint64_t lastUsedSubmit;
	uint32_t size;
} dsMTLTempBuffer;

typedef void (*ClearCommandBufferFunction)(dsCommandBuffer* commandBuffer);
typedef void (*EndCommandBufferFunction)(dsCommandBuffer* commandBuffer);
typedef bool (*SubmitCommandBufferFunction)(dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer);

typedef bool (*CopyBufferDataFunction)(dsCommandBuffer* commandBuffer,
	id<MTLBuffer> buffer, size_t offset, const void* data, size_t size);
typedef bool (*CopyBufferFunction)(dsCommandBuffer* commandBuffer, id<MTLBuffer> srcBuffer,
	size_t srcOffset, id<MTLBuffer> dstBuffer, size_t dstOffset, size_t size);
typedef bool (*CopyBufferToTextureFunction)(dsCommandBuffer* commandBuffer, id<MTLBuffer> srcBuffer,
	id<MTLTexture> dstTexture, dsGfxFormat format, const dsGfxBufferTextureCopyRegion* regions,
	uint32_t regionCount);

typedef bool (*CopyTextureDataFunction)(dsCommandBuffer* commandBuffer, id<MTLTexture> texture,
	const dsTextureInfo* textureInfo, const dsTexturePosition* position, uint32_t width,
	uint32_t height, uint32_t layers, const void* data, size_t size);
typedef bool (*CopyTextureFunction)(dsCommandBuffer* commandBuffer, id<MTLTexture> srcTexture,
	id<MTLTexture> dstTexture, const dsTextureCopyRegion* regions, uint32_t regionCount);
typedef bool (*CopyTextureToBufferFunction)(dsCommandBuffer* commandBuffer,
	id<MTLTexture> srcTexture, id<MTLBuffer> dstBuffer, dsGfxFormat format,
	const dsGfxBufferTextureCopyRegion* regions, uint32_t regionCount);
typedef bool (*GenerateMipmapsFunction)(dsCommandBuffer* commandBuffer, id<MTLTexture> texture);

typedef bool (*BindPushConstantsFunction)(dsCommandBuffer* commandBuffer, const void* data,
	uint32_t size, bool vertex, bool fragment);
typedef bool (*BindBufferUniformFunction)(dsCommandBuffer* commandBuffer, id<MTLBuffer> buffer,
	size_t offset, uint32_t vertexIndex, uint32_t fragmentIndex);
typedef bool (*BindTextureUniformFunction)(dsCommandBuffer* commandBuffer, id<MTLTexture> texture,
	id<MTLSamplerState> sampler, uint32_t vertexIndex, uint32_t fragmentIndex);
typedef bool (*SetRenderStatesFunction)(dsCommandBuffer* commandBuffer,
	const mslRenderState* renderStates, id<MTLDepthStencilState> depthStencilState,
	const dsDynamicRenderStates* dynamicStates, bool dynamicOnly);

typedef bool (*BeginComputeShaderFunction)(dsCommandBuffer* commandBuffer);
typedef bool (*BindComputePushConstantsFunction)(dsCommandBuffer* commandBuffer, const void* data,
	uint32_t size);
typedef bool (*BindComputeBufferUniformFunction)(dsCommandBuffer* commandBuffer,
	id<MTLBuffer> buffer, size_t offset, uint32_t index);
typedef bool (*BindComputeTextureUniformFunction)(dsCommandBuffer* commandBuffer,
	id<MTLTexture> texture, id<MTLSamplerState> sampler, uint32_t index);

typedef bool (*BeginRenderPassFunction)(dsCommandBuffer* commandBuffer,
	MTLRenderPassDescriptor* renderPass, const dsAlignedBox3f* viewport);
typedef bool (*EndRenderPassFunction)(dsCommandBuffer* commandBuffer);

typedef bool (*SetViewportFunction)(dsCommandBuffer* commandBuffer,
	const dsAlignedBox3f* viewport);
typedef bool (*ClearAttachmentsFunction)(dsCommandBuffer* commandBuffer,
	const dsClearAttachment* attachments, uint32_t attachmentCount,
	const dsAttachmentClearRegion* regions, uint32_t regionCount);
typedef bool (*DrawFunction)(dsCommandBuffer* commandBuffer, id<MTLRenderPipelineState> pipeline,
	const dsDrawRange* drawRange, dsPrimitiveType primitiveType);
typedef bool (*DrawIndexedFunction)(dsCommandBuffer* commandBuffer,
	id<MTLRenderPipelineState> pipeline, id<MTLBuffer> indexBuffer, size_t indexOffset,
	uint32_t indexSize, const dsDrawIndexedRange* drawRange, dsPrimitiveType primitiveType);
typedef bool (*DrawIndirectFunction)(dsCommandBuffer* commandBuffer,
	id<MTLRenderPipelineState> pipeline, id<MTLBuffer> indirectBuffer, size_t offset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType);
typedef bool (*DrawIndexedIndirectFunction)(dsCommandBuffer* commandBuffer,
	id<MTLRenderPipelineState> pipeline, id<MTLBuffer> indexBuffer, size_t indexOffset,
	uint32_t indexSize, id<MTLBuffer> indirectBuffer, size_t indirectOffset,
	uint32_t count, uint32_t stride, dsPrimitiveType primitiveType);

typedef bool (*DispatchComputeFunction)(dsCommandBuffer* commandBuffer,
	id<MTLComputePipelineState> computePipeline, uint32_t x, uint32_t y, uint32_t z,
	uint32_t groupX, uint32_t groupY, uint32_t groupZ);
typedef bool (*DispatchComputeIndirectFunction)(dsCommandBuffer* commandBuffer,
	id<MTLComputePipelineState> computePipeline, id<MTLBuffer> buffer, size_t offset,
	uint32_t groupX, uint32_t groupY, uint32_t groupZ);

typedef bool (*PushDebugGroupFunction)(dsCommandBuffer* commandBuffer, const char* name);
typedef bool (*PopDebugGroupFunction)(dsCommandBuffer* commandBuffer);

typedef struct dsMTLCommandBufferFunctionTable
{
	ClearCommandBufferFunction clearFunc;
	EndCommandBufferFunction endFunc;
	SubmitCommandBufferFunction submitFunc;

	CopyBufferDataFunction copyBufferDataFunc;
	CopyBufferFunction copyBufferFunc;
	CopyBufferToTextureFunction copyBufferToTextureFunc;

	CopyTextureDataFunction copyTextureDataFunc;
	CopyTextureFunction copyTextureFunc;
	CopyTextureToBufferFunction copyTextureToBufferFunc;
	GenerateMipmapsFunction generateMipmapsFunc;

	BindPushConstantsFunction bindPushConstantsFunc;
	BindBufferUniformFunction bindBufferUniformFunc;
	BindTextureUniformFunction bindTextureUniformFunc;
	SetRenderStatesFunction setRenderStatesFunc;

	BeginComputeShaderFunction beginComputeShaderFunc;
	BindComputePushConstantsFunction bindComputePushConstantsFunc;
	BindComputeBufferUniformFunction bindComputeBufferUniformFunc;
	BindComputeTextureUniformFunction bindComputeTextureUniformFunc;

	BeginRenderPassFunction beginRenderPassFunc;
	EndRenderPassFunction endRenderPassFunc;

	SetViewportFunction setViewportFunc;
	ClearAttachmentsFunction clearAttachmentsFunc;
	DrawFunction drawFunc;
	DrawIndexedFunction drawIndexedFunc;
	DrawIndirectFunction drawIndirectFunc;
	DrawIndexedIndirectFunction drawIndexedIndirectFunc;

	DispatchComputeFunction dispatchComputeFunc;
	DispatchComputeIndirectFunction dispatchComputeIndirectFunc;

	PushDebugGroupFunction pushDebugGroupFunc;
	PopDebugGroupFunction popDebugGroupFunc;
} dsMTLCommandBufferFunctionTable;

typedef struct dsMTLCommandBuffer
{
	dsCommandBuffer commandBuffer;
	const dsMTLCommandBufferFunctionTable* functions;

	dsLifetime** gfxBuffers;
	uint32_t gfxBufferCount;
	uint32_t maxGfxBuffers;

	dsLifetime** tempBuffers;
	uint32_t tempBufferCount;
	uint32_t maxTempBuffers;

	dsLifetime** readbackOffscreens;
	uint32_t readbackOffscreenCount;
	uint32_t maxReadbackOffscreens;

	dsLifetime** fences;
	uint32_t fenceCount;
	uint32_t maxFences;

	uint8_t* pushConstantData;
	uint32_t maxPushConstantDataSize;

	dsSurfaceClearValue* clearValues;
	uint32_t clearValueCount;
	uint32_t maxClearValues;

	dsAlignedBox3f viewport;
	const dsDrawGeometry* boundGeometry;
	uint32_t firstVertexBuffer;
	int32_t vertexOffset;

	bool fenceSet;
} dsMTLCommandBuffer;

typedef struct dsMTLBoundTexture
{
	CFTypeRef texture;
	CFTypeRef sampler;
} dsMTLBoundTexture;

typedef struct dsMTLBoundTextureSet
{
	dsMTLBoundTexture* textures;
	uint32_t textureCount;
	uint32_t maxTextures;
} dsMTLBoundTextureSet;

typedef struct dsMTLBoundBuffer
{
	CFTypeRef buffer;
	size_t offset;
} dsMTLBoundBuffer;

typedef struct dsMTLBoundBufferSet
{
	dsMTLBoundBuffer* buffers;
	uint32_t bufferCount;
	uint32_t maxBuffers;
} dsMTLBoundBufferSet;

typedef struct dsMTLHardwareCommandBuffer
{
	dsMTLCommandBuffer commandBuffer;

	CFTypeRef mtlCommandBuffer;

	CFTypeRef renderCommandEncoder;
	CFTypeRef blitCommandEncoder;
	CFTypeRef computeCommandEncoder;

	CFTypeRef* submitBuffers;
	uint32_t submitBufferCount;
	uint32_t maxSubmitBuffers;

	dsMTLBoundTextureSet boundTextures[2];
	dsMTLBoundBufferSet boundBuffers[2];

	dsMTLBoundTextureSet boundComputeTextures;
	dsMTLBoundBufferSet boundComputeBuffers;
	CFTypeRef boundComputePipeline;

	CFTypeRef boundPipeline;
	CFTypeRef boundDepthStencil;
	MTLViewport curViewport;
	uint32_t curFrontStencilRef;
	uint32_t curBackStencilRef;

	dsMTLTempBuffer* curTempBuffer;
	dsMTLTempBuffer** tempBufferPool;
	uint32_t tempBufferPoolCount;
	uint32_t maxTempBufferPools;
} dsMTLHardwareCommandBuffer;

typedef struct dsMTLSoftwareCommandBuffer
{
	dsMTLCommandBuffer commandBuffer;
	dsBufferAllocator commands;
} dsMTLSoftwareCommandBuffer;

typedef struct dsMTLRenderSurface
{
	dsRenderSurface renderSurface;
	dsSpinlock lock;
	CFTypeRef view;
	CFTypeRef layer;
	CFTypeRef drawable;
	CFTypeRef resolveSurface;
	CFTypeRef depthSurface;
	CFTypeRef stencilSurface;
} dsMTLRenderSurface;

typedef struct dsMTLResourceManager
{
	dsResourceManager resourceManager;

	MTLPixelFormat standardPixelFormats[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
	MTLPixelFormat specialPixelFormats[dsGfxFormat_SpecialCount];
	MTLPixelFormat compressedPixelFormats[dsGfxFormat_CompressedCount][dsGfxFormat_DecoratorCount];

	MTLVertexFormat vertexFormats[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];

	bool appleGpu;

	CFTypeRef defaultSampler;
} dsMTLResourceManager;

typedef struct dsMTLClearPipeline
{
	MTLPixelFormat colorFormats[DS_MAX_ATTACHMENTS];
	uint32_t colorMask;
	MTLPixelFormat depthFormat;
	MTLPixelFormat stencilFormat;
	bool layered;
	uint8_t samples;
	CFTypeRef pipeline;
} dsMTLClearPipeline;

typedef struct dsMTLRenderer
{
	dsRenderer renderer;

	CFTypeRef device;
	CFTypeRef commandQueue;

	CFTypeRef clearNoDepthStencilState;
	CFTypeRef clearDepthState;
	CFTypeRef clearStencilState;
	CFTypeRef clearDepthStencilState;
	CFTypeRef clearVertices;

	dsMTLHardwareCommandBuffer mainCommandBuffer;

	uint64_t submitCount;
	uint64_t finishedSubmitCount;

	dsConditionVariable* submitCondition;
	dsMutex* submitMutex;

	dsLifetime** processBuffers;
	uint32_t processBufferCount;
	uint32_t maxProcessBuffers;

	dsLifetime** processTextures;
	uint32_t processTextureCount;
	uint32_t maxProcessTextures;

	dsMTLClearPipeline* clearPipelines;
	uint32_t clearPipelineCount;
	uint32_t maxClearPipelines;

	dsSpinlock processBuffersLock;
	dsSpinlock processTexturesLock;
	dsSpinlock clearPipelinesLock;
} dsMTLRenderer;

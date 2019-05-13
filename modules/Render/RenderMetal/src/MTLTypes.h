/*
 * Copyright 2019 Aaron Barany
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
#import <Metal/MTLVertexDescriptor.h>

// This is used by SPIRV-Cross to create the shader.
#define DS_IMAGE_BUFFER_WIDTH 4096
#define DS_NOT_SUBMITTED (uint64_t)-1
// 10 seconds in milliseconds
#define DS_DEFAULT_WAIT_TIMEOUT 10000
#define DS_RECENTLY_ADDED_SIZE 10

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
	uint64_t lastUsedSubmit;

	dsSpinlock bufferTextureLock;
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
	CFTypeRef resolveTexture;
	uint32_t processed;
} dsMTLTexture;

typedef struct dsMTLRenderbuffer
{
	dsRenderbuffer renderbuffer;
	CFTypeRef surface;
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

typedef struct dsMTLRenderPass
{
	dsRenderPass renderPass;
	dsLifetime* lifetime;

	dsLifetime** usedShaders;
	uint32_t usedShaderCount;
	uint32_t maxUsedShaders;
	dsSpinlock shaderLock;
} dsMTLRenderPass;

typedef void (*ClearCommandBufferFunction)(dsCommandBuffer* commandBuffer);
typedef void (*EndCommandBufferFunction)(dsCommandBuffer* commandBuffer);
typedef bool (*SubmitCommandBufferFunction)(dsCommandBuffer* commandBuffer,
	dsCommandBuffer* submitBuffer);

typedef bool (*CopyBufferDataFunction)(dsCommandBuffer* commandBuffer,
	id<MTLBuffer> buffer, size_t offset, const void* data, size_t size);
typedef bool (*CopyBufferFunction)(dsCommandBuffer* commandBuffer, id<MTLBuffer> srcBuffer,
	size_t srcOffset, id<MTLBuffer> dstBuffer, size_t dstOffset, size_t size);

typedef bool (*CopyTextureDataFunction)(dsCommandBuffer* commandBuffer, id<MTLTexture> texture,
	const dsTextureInfo* textureInfo, const dsTexturePosition* position, uint32_t width,
	uint32_t height, uint32_t layers, const void* data, size_t size);
typedef bool (*CopyTextureFunction)(dsCommandBuffer* commandBuffer, id<MTLTexture> srcTexture,
	id<MTLTexture> dstTexture, const dsTextureCopyRegion* regions, uint32_t regionCount);
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

typedef bool (*BindComputePushConstantsFunction)(dsCommandBuffer* commandBuffer, const void* data,
	uint32_t size);
typedef bool (*BindComputeBufferUniformFunction)(dsCommandBuffer* commandBuffer,
	id<MTLBuffer> buffer, size_t offset, uint32_t index);
typedef bool (*BindComputeTextureUniformFunction)(dsCommandBuffer* commandBuffer,
	id<MTLTexture> texture, id<MTLSamplerState> sampler, uint32_t index);

typedef struct dsMTLCommandBufferFunctionTable
{
	ClearCommandBufferFunction clearFunc;
	EndCommandBufferFunction endFunc;
	SubmitCommandBufferFunction submitFunc;

	CopyBufferDataFunction copyBufferDataFunc;
	CopyBufferFunction copyBufferFunc;

	CopyTextureDataFunction copyTextureDataFunc;
	CopyTextureFunction copyTextureFunc;
	GenerateMipmapsFunction generateMipmapsFunc;

	BindPushConstantsFunction bindPushConstantsFunc;
	BindBufferUniformFunction bindBufferUniformFunc;
	BindTextureUniformFunction bindTextureUniformFunc;
	SetRenderStatesFunction setRenderStatesFunc;

	BindComputePushConstantsFunction bindComputePushConstantsFunc;
	BindComputeBufferUniformFunction bindComputeBufferUniformFunc;
	BindComputeTextureUniformFunction bindComputeTextureUniformFunc;
} dsMTLCommandBufferFunctionTable;

typedef struct dsMTLCommandBuffer
{
	dsCommandBuffer commandBuffer;
	const dsMTLCommandBufferFunctionTable* functions;

	dsLifetime** gfxBuffers;
	uint32_t gfxBufferCount;
	uint32_t maxGfxBuffers;

	dsLifetime** fences;
	uint32_t fenceCount;
	uint32_t maxFences;

	uint8_t* pushConstantData;
	uint32_t maxPushConstantDataSize;

	bool fenceSet;
} dsMTLCommandBuffer;

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
} dsMTLHardwareCommandBuffer;

typedef struct dsMTLSoftwareCommandBuffer
{
	dsMTLCommandBuffer commandBuffer;
	dsBufferAllocator commands;
} dsMTLSoftwareCommandBuffer;

typedef struct dsMTLRenderSurface
{
	dsRenderSurface renderSurface;
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

	CFTypeRef defaultSampler;
} dsMTLResourceManager;

typedef struct dsMTLRenderer
{
	dsRenderer renderer;

	CFTypeRef device;
	CFTypeRef commandQueue;

	dsMTLHardwareCommandBuffer mainCommandBuffer;

	uint64_t submitCount;
	uint64_t finishedSubmitCount;

	dsConditionVariable* submitCondition;
	dsMutex* submitMutex;

	dsLifetime** processTextures;
	uint32_t processTextureCount;
	uint32_t maxProcessTextures;

	dsSpinlock processTexturesLock;
} dsMTLRenderer;

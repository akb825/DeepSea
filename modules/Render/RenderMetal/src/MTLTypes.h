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

#import <Foundation/NSObject.h>
#import <Metal/MTLArgument.h>
#import <Metal/MTLCommandBuffer.h>
#import <Metal/MTLDevice.h>
#import <Metal/MTLPixelFormat.h>
#import <Metal/MTLVertexDescriptor.h>

// This is used by SPIRV-Cross to create the shader.
#define DS_IMAGE_BUFFER_WIDTH 4096
#define DS_NOT_SUBMITTED (uint64_t)-1
#define DS_DELAY_FRAMES 3
#define DS_EXPECTED_FRAME_FLUSHES 10
#define DS_MAX_SUBMITS (DS_DELAY_FRAMES*DS_EXPECTED_FRAME_FLUSHES)
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

typedef struct dsMTLCommandBuffer
{
	dsCommandBuffer commandBuffer;

	CFTypeRef mtlCommandBuffer;

	CFTypeRef renderCommandEncoder;
	CFTypeRef blitCommandEncoder;
	CFTypeRef computeCommandEncoder;

	CFTypeRef* submitBuffers;
	uint32_t submitBufferCount;
	uint32_t maxSubmitBuffers;

	dsLifetime** gfxBuffers;
	uint32_t gfxBufferCount;
	uint32_t maxGfxBuffers;
} dsMTLCommandBuffer;

typedef struct dsMTLResourceManager
{
	dsResourceManager resourceManager;

	MTLPixelFormat standardPixelFormats[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
	MTLPixelFormat specialPixelFormats[dsGfxFormat_SpecialCount];
	MTLPixelFormat compressedPixelFormats[dsGfxFormat_CompressedCount][dsGfxFormat_DecoratorCount];

	MTLVertexFormat vertexFormats[dsGfxFormat_StandardCount][dsGfxFormat_DecoratorCount];
} dsMTLResourceManager;

typedef struct dsMTLSubmitInfo
{
	CFTypeRef commandBuffer;
	uint64_t submitCount;
} dsMTLSubmitInfo;

typedef struct dsMTLRenderer
{
	dsRenderer renderer;

	CFTypeRef device;
	CFTypeRef commandQueue;
	MTLFeatureSet featureSet;

	uint32_t curSubmit;
	dsMTLSubmitInfo submits[DS_MAX_SUBMITS];
	uint64_t submitCount;
	uint64_t finishedSubmitCount;

	dsConditionVariable* submitCondition;
	dsMutex* submitMutex;

	dsLifetime** processTextures;
	uint32_t processTextureCount;
	uint32_t maxProcessTextures;

	dsSpinlock processTexturesLock;
} dsMTLRenderer;

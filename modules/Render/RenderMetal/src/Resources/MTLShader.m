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

#include "Resources/MTLShader.h"

#include "Resources/MTLPipeline.h"
#include "Resources/MTLShaderModule.h"
#include "MTLRenderPass.h"

#include <DeepSea/Core/Containers/ResizeableArray.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/BufferAllocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Thread/Spinlock.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Atomic.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>

#include <MSL/Client/ModuleC.h>
#include <string.h>

#import <Metal/MTLSampler.h>

static size_t fullAllocSize(const mslPipeline* pipeline)
{
	size_t fullSize = DS_ALIGNED_SIZE(sizeof(dsMTLShader));

	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (pipeline->shaders[i] != DS_MATERIAL_UNKNOWN)
			fullSize += DS_ALIGNED_SIZE(sizeof(uint32_t)*pipeline->uniformCount);
	}

	fullSize += DS_ALIGNED_SIZE(sizeof(CFTypeRef)*pipeline->samplerStateCount);
	return fullSize;
}

static MTLSamplerMinMagFilter getFilter(mslFilter filter)
{
	switch (filter)
	{
		case mslFilter_Linear:
			return MTLSamplerMinMagFilterLinear;
		case mslFilter_Nearest:
		default:
			return MTLSamplerMinMagFilterNearest;
	}
}

static MTLSamplerMipFilter getMipFilter(mslMipFilter filter)
{
	switch (filter)
	{
		case mslMipFilter_Nearest:
			return MTLSamplerMipFilterNearest;
		case mslMipFilter_Linear:
		case mslMipFilter_Anisotropic:
			return MTLSamplerMipFilterLinear;
		case mslMipFilter_None:
		default:
			return MTLSamplerMipFilterNotMipmapped;
	}
}

static MTLSamplerAddressMode getAddressMode(mslAddressMode mode)
{
	switch (mode)
	{
		case mslAddressMode_MirroredRepeat:
			return MTLSamplerAddressModeMirrorRepeat;
		case mslAddressMode_ClampToEdge:
			return MTLSamplerAddressModeClampToEdge;
		case mslAddressMode_ClampToBorder:
			return MTLSamplerAddressModeClampToBorderColor;
		case mslAddressMode_MirrorOnce:
			return MTLSamplerAddressModeMirrorClampToEdge;
		case mslAddressMode_Repeat:
		default:
			return MTLSamplerAddressModeRepeat;
	}
}

static MTLSamplerBorderColor getBorderColor(mslBorderColor color)
{
	switch (color)
	{
		case mslBorderColor_OpaqueBlack:
		case mslBorderColor_OpaqueIntZero:
			return MTLSamplerBorderColorOpaqueBlack;
		case mslBorderColor_OpaqueWhite:
		case mslBorderColor_OpaqueIntOne:
			return MTLSamplerBorderColorOpaqueWhite;
		case mslBorderColor_TransparentBlack:
		case mslBorderColor_TransparentIntZero:
		default:
			return MTLSamplerBorderColorTransparentBlack;
	}
}

#if !DS_IOS || IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
MTLCompareFunction getCompareFunction(mslCompareOp compare)
{
	switch (compare)
	{
		case mslCompareOp_Less:
			return MTLCompareFunctionLess;
		case mslCompareOp_Equal:
			return MTLCompareFunctionEqual;
		case mslCompareOp_LessOrEqual:
			return MTLCompareFunctionLessEqual;
		case mslCompareOp_Greater:
			return MTLCompareFunctionGreater;
		case mslCompareOp_NotEqual:
			return MTLCompareFunctionNotEqual;
		case mslCompareOp_GreaterOrEqual:
			return MTLCompareFunctionGreaterEqual;
		case mslCompareOp_Always:
			return MTLCompareFunctionAlways;
		case mslCompareOp_Never:
		default:
			return MTLCompareFunctionNever;
	}
}
#endif

static id<MTLSamplerState> createSampler(dsRenderer* renderer, const mslSamplerState* samplerState)
{
	dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)mtlRenderer;
	MTLSamplerDescriptor* descriptor = [MTLSamplerDescriptor new];
	if (!descriptor)
	{
		errno = ENOMEM;
		return NULL;
	}
	descriptor.rAddressMode = getAddressMode(samplerState->addressModeU);
	descriptor.sAddressMode = getAddressMode(samplerState->addressModeV);
	descriptor.tAddressMode = getAddressMode(samplerState->addressModeW);
	descriptor.borderColor = getBorderColor(samplerState->borderColor);

	descriptor.minFilter = getFilter(samplerState->minFilter);
	descriptor.magFilter = getFilter(samplerState->magFilter);
	descriptor.mipFilter = getMipFilter(samplerState->mipFilter);
	if (samplerState->minLod != MSL_UNKNOWN_FLOAT)
		descriptor.lodMinClamp = samplerState->minLod;
	if (samplerState->maxLod != MSL_UNKNOWN_FLOAT)
		descriptor.lodMaxClamp = samplerState->maxLod;
	if (samplerState->mipFilter == mslMipFilter_Anisotropic)
	{
		if (samplerState->maxAnisotropy == MSL_UNKNOWN)
			descriptor.maxAnisotropy = (NSUInteger)roundf(samplerState->maxAnisotropy);
		else
			descriptor.maxAnisotropy = (NSUInteger)roundf(renderer->defaultAnisotropy);
	}

#if !DS_IOS || IPHONE_OS_VERSION_MIN_REQUIRED >= 90000
	descriptor.compareFunction = getCompareFunction(samplerState->compareOp);
#endif

	id<MTLSamplerState> sampler = [device newSamplerStateWithDescriptor: descriptor];
	if (!sampler)
		errno = ENOMEM;
	return sampler;
}

static bool createSamplers(dsMTLShader* shader, mslModule* module, uint32_t shaderIndex)
{
	bool hasDefaultAnisotropy = false;
	dsRenderer* renderer = ((dsShader*)shader)->resourceManager->renderer;
	for (uint32_t i = 0; i < shader->pipeline.samplerStateCount; ++i)
	{
		mslSamplerState samplerState;
		DS_VERIFY(mslModule_samplerState(&samplerState, module, shaderIndex, i));

		if (samplerState.mipFilter == mslMipFilter_Anisotropic &&
			samplerState.maxAnisotropy == MSL_UNKNOWN_FLOAT)
		{
			hasDefaultAnisotropy = true;
		}

		id<MTLSamplerState> sampler = createSampler(renderer, &samplerState);
		if (!sampler)
			return false;

		shader->samplers[i] = CFBridgingRetain(sampler);
	}

	if (hasDefaultAnisotropy)
		shader->defaultAnisotropy = renderer->defaultAnisotropy;
	else
		shader->defaultAnisotropy = MSL_UNKNOWN_FLOAT;
	return true;
}

static void setupUniformIndices(dsMTLShader* shader, mslModule* module, uint32_t shaderIndex)
{
	// TODO once implemented in MSL
	DS_UNUSED(shader);
	DS_UNUSED(module);
	DS_UNUSED(shaderIndex);
}

static bool setupShaders(dsShader* shader, mslModule* module, uint32_t shaderIndex)
{
	dsMTLRenderer* renderer = (dsMTLRenderer*)shader->resourceManager->renderer;
	id<MTLDevice> device = (__bridge id<MTLDevice>)renderer->device;
	dsMTLShader* mtlShader = (dsMTLShader*)shader;
	mslPipeline* pipeline = &mtlShader->pipeline;
	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (pipeline->shaders[i] == MSL_UNKNOWN)
			continue;

		id<MTLFunction> function = dsMTLShaderModule_getShader(shader->module, pipeline->shaders[i],
			pipeline->name);
		if (!function)
			return false;

		mtlShader->stages[i].function = CFBridgingRetain(function);
	}
	setupUniformIndices(mtlShader, module, shaderIndex);

	if (pipeline->shaders[mslStage_Compute] != MSL_UNKNOWN)
	{
		id<MTLFunction> function = dsMTLShaderModule_getShader(shader->module,
			pipeline->shaders[mslStage_Compute], pipeline->name);
		if (!function)
			return false;

		NSError* error = NULL;
		id<MTLComputePipelineState> computePipeline =
			[device newComputePipelineStateWithFunction: function error: &error];
		if (!computePipeline)
		{
			if (error)
			{
				DS_LOG_ERROR_F(DS_RENDER_METAL_LOG_TAG,
					"Error creating compute pipeline for shader %s.%s: %s",
					shader->module->name, shader->name, [[error localizedDescription] UTF8String]);
			}
			else
			{
				DS_LOG_ERROR_F(DS_RENDER_METAL_LOG_TAG,
					"Error creating compute pipeline for shader %s.%s.",
					shader->module->name, shader->name);
			}
			return false;
		}

		mtlShader->computePipeline = CFBridgingRetain(computePipeline);
	}
	return true;
}

dsShader* dsMTLShader_create(dsResourceManager* resourceManager, dsAllocator* allocator,
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

	size_t fullSize = fullAllocSize(&pipeline);
	void* buffer = dsAllocator_alloc(allocator, fullSize);
	if (!buffer)
		return NULL;

	dsBufferAllocator bufferAlloc;
	DS_VERIFY(dsBufferAllocator_initialize(&bufferAlloc, buffer, fullSize));
	dsMTLShader* shader = DS_ALLOCATE_OBJECT((dsAllocator*)&bufferAlloc, dsMTLShader);
	DS_ASSERT(shader);

	dsShader* baseShader = (dsShader*)shader;
	baseShader->resourceManager = resourceManager;
	baseShader->allocator = dsAllocator_keepPointer(allocator);
	baseShader->module = module;
	baseShader->name = pipeline.name;
	baseShader->pipelineIndex = shaderIndex;
	baseShader->pipeline = &shader->pipeline;
	baseShader->materialDesc = materialDesc;

	shader->scratchAllocator = resourceManager->allocator;
	shader->lifetime = NULL;
	shader->pipeline = pipeline;

	for (int i = 0; i < mslStage_Count; ++i)
	{
		shader->stages[i].function = NULL;
		if (pipeline.shaders[i] != MSL_UNKNOWN)
		{
			shader->stages[i].uniformIndices = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc,
				uint32_t, pipeline.uniformCount);
			DS_ASSERT(shader->stages[i].uniformIndices);
			memset(shader->stages[i].uniformIndices, 0xFF, sizeof(CFTypeRef)*pipeline.uniformCount);
		}
		else
			shader->stages[i].uniformIndices = NULL;
	}

	if (pipeline.samplerStateCount > 0)
	{
		shader->samplers = DS_ALLOCATE_OBJECT_ARRAY((dsAllocator*)&bufferAlloc, CFTypeRef,
			pipeline.samplerStateCount);
		DS_ASSERT(shader->samplers);
		memset(shader->samplers, 0, sizeof(CFTypeRef)*pipeline.samplerStateCount);
	}
	DS_VERIFY(dsSpinlock_initialize(&shader->samplerLock));

	shader->pipelines = NULL;
	shader->pipelineCount = 0;
	shader->maxPipelines = 0;
	DS_VERIFY(dsSpinlock_initialize(&shader->pipelineLock));

	shader->computePipeline = NULL;

	shader->lifetime = dsLifetime_create(allocator, shader);
	if (!shader->lifetime)
	{
		dsMTLShader_destroy(resourceManager, baseShader);
		return NULL;
	}

	if (!setupShaders(baseShader, module->module, shaderIndex))
	{
		dsMTLShader_destroy(resourceManager, baseShader);
		return NULL;
	}

	DS_VERIFY(mslModule_renderState(&shader->renderState, module->module, shaderIndex));
	if (!createSamplers(shader, module->module, shaderIndex))
	{
		dsMTLShader_destroy(resourceManager, baseShader);
		return NULL;
	}
	return baseShader;
}

bool dsMTLShader_bind(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsMaterial* material,
	const dsSharedMaterialValues* sharedValues, const dsDynamicRenderStates* renderStates)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(shader);
	DS_UNUSED(material);
	DS_UNUSED(sharedValues);
	DS_UNUSED(renderStates);
	return false;
}

bool dsMTLShader_updateSharedValues(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsSharedMaterialValues* sharedValues)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(shader);
	DS_UNUSED(sharedValues);
	return false;
}

bool dsMTLShader_unbind(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(shader);
	return false;
}

bool dsMTLShader_bindCompute(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader, const dsMaterial* material,
	const dsSharedMaterialValues* sharedValues)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(shader);
	DS_UNUSED(material);
	DS_UNUSED(sharedValues);
	return false;
}

bool dsMTLShader_updateComputeSharedValues(dsResourceManager* resourceManager,
	dsCommandBuffer* commandBuffer, const dsShader* shader,
	const dsSharedMaterialValues* sharedValues)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(shader);
	DS_UNUSED(sharedValues);
	return false;
}

bool dsMTLShader_unbindCompute(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	const dsShader* shader)
{
	DS_UNUSED(resourceManager);
	DS_UNUSED(commandBuffer);
	DS_UNUSED(shader);
	return false;
}

bool dsMTLShader_destroy(dsResourceManager* resourceManager, dsShader* shader)
{
	DS_UNUSED(resourceManager);

	dsMTLShader* mtlShader = (dsMTLShader*)shader;

	// Clear out the array inside the lock, then destroy the objects outside to avoid nested locks
	// that can deadlock. The lifetime object protects against shaders being destroyed concurrently
	// when unregistering the material.
	DS_VERIFY(dsSpinlock_lock(&mtlShader->pipelineLock));
	dsLifetime** usedRenderPasses = mtlShader->usedRenderPasses;
	uint32_t usedRenderPassCount = mtlShader->usedRenderPassCount;
	mtlShader->usedRenderPasses = NULL;
	mtlShader->usedRenderPassCount = 0;
	mtlShader->maxUsedRenderPasses = 0;

	dsMTLPipeline** pipelines = mtlShader->pipelines;
	uint32_t pipelineCount = mtlShader->pipelineCount;
	mtlShader->pipelines = NULL;
	mtlShader->pipelineCount = 0;
	mtlShader->maxPipelines = 0;
	DS_VERIFY(dsSpinlock_unlock(&mtlShader->pipelineLock));

	for (uint32_t i = 0; i < usedRenderPassCount; ++i)
	{
		dsRenderPass* renderPass = (dsRenderPass*)dsLifetime_acquire(usedRenderPasses[i]);
		if (renderPass)
		{
			dsMTLRenderPass_removeShader(renderPass, shader);
			dsLifetime_release(usedRenderPasses[i]);
		}
		dsLifetime_freeRef(usedRenderPasses[i]);
	}
	DS_VERIFY(dsAllocator_free(mtlShader->scratchAllocator, usedRenderPasses));
	DS_ASSERT(!mtlShader->usedRenderPasses);

	dsLifetime_destroy(mtlShader->lifetime);

	for (int i = 0; i < mslStage_Count; ++i)
	{
		if (mtlShader->stages[i].function)
			CFRelease(mtlShader->stages[i].function);
	}

	for (uint32_t i = 0; i < shader->pipeline->samplerStateCount; ++i)
	{
		if (mtlShader->samplers[i])
			CFRelease(mtlShader->samplers[i]);
	}

	for (uint32_t i = 0; i < pipelineCount; ++i)
		dsMTLPipeline_destroy(pipelines[i]);
	DS_VERIFY(dsAllocator_free(mtlShader->scratchAllocator, pipelines));

	if (mtlShader->computePipeline)
		CFRelease(mtlShader->computePipeline);

	return true;
}

id<MTLRenderPipelineState> dsMTLShader_getPipeline(dsShader* shader, dsCommandBuffer* commandBuffer,
	dsPrimitiveType primitiveType, const dsDrawGeometry* geometry)
{
	dsRenderPass* renderPass = (dsRenderPass*)commandBuffer->boundRenderPass;
	if (!renderPass)
		return NULL;

	const dsMTLRenderPass* mtlRenderPass = (const dsMTLRenderPass*)renderPass;;
	dsMTLShader* mtlShader = (dsMTLShader*)shader;
	if (!mtlShader->stages[mslStage_Vertex].function)
		return NULL;

	dsRenderer* renderer = commandBuffer->renderer;
	uint32_t subpassIndex = commandBuffer->activeRenderSubpass;
	const dsRenderSubpassInfo* subpass = renderPass->subpasses + subpassIndex;

	// Get the number of samples based on the attachments.
	const dsAttachmentInfo* attachments = renderPass->attachments;
	uint32_t referenceAttachment = DS_NO_ATTACHMENT;
	for (uint32_t i = 0; i < subpass->colorAttachmentCount; ++i)
	{
		uint32_t colorAttachment = subpass->colorAttachments[i].attachmentIndex;
		if (colorAttachment != DS_NO_ATTACHMENT)
		{
			referenceAttachment = colorAttachment;
			break;
		}
	}
	if (referenceAttachment == DS_NO_ATTACHMENT)
		referenceAttachment = subpass->depthStencilAttachment;

	uint32_t samples = DS_DEFAULT_ANTIALIAS_SAMPLES;
	if (referenceAttachment != DS_NO_ATTACHMENT)
		samples = attachments[referenceAttachment].samples;

	if (samples == DS_DEFAULT_ANTIALIAS_SAMPLES)
		samples = renderer->surfaceSamples;

	const dsMTLDrawGeometry* mtlDrawGeometry = (dsMTLDrawGeometry*)geometry;
	uint32_t hash = dsMTLPipeline_hash(samples, primitiveType, mtlDrawGeometry->vertexHash,
		renderPass, subpassIndex);

	DS_VERIFY(dsSpinlock_lock(&mtlShader->pipelineLock));

	// Search for an existing pipeline
	for (uint32_t i = 0; i < mtlShader->pipelineCount; ++i)
	{
		dsMTLPipeline* pipeline = mtlShader->pipelines[i];
		if (dsMTLPipeline_isEquivalent(pipeline, hash, samples, primitiveType, geometry, renderPass,
				subpassIndex))
		{
			id<MTLRenderPipelineState> mtlPipeline =
				(__bridge id<MTLRenderPipelineState>)pipeline->pipeline;
			DS_VERIFY(dsSpinlock_unlock(&mtlShader->pipelineLock));
			return mtlPipeline;
		}
	}

	// Add a new pipeline if not present.
	uint32_t index = mtlShader->pipelineCount;
	if (!DS_RESIZEABLE_ARRAY_ADD(mtlShader->scratchAllocator, mtlShader->pipelines,
		mtlShader->pipelineCount, mtlShader->maxPipelines, 1))
	{
		DS_VERIFY(dsSpinlock_unlock(&mtlShader->pipelineLock));
		return 0;
	}

	mtlShader->pipelines[index] = dsMTLPipeline_create(mtlShader->scratchAllocator, shader,
		hash, samples, primitiveType, geometry, renderPass, subpassIndex);
	if (!mtlShader->pipelines[index])
	{
		--mtlShader->pipelineCount;
		DS_VERIFY(dsSpinlock_unlock(&mtlShader->pipelineLock));
		return 0;
	}

	// Register the render pass.
	bool hasRenderPass = false;
	for (uint32_t i = 0; i < mtlShader->usedRenderPassCount; ++i)
	{
		if (mtlShader->usedRenderPasses[i] == mtlRenderPass->lifetime)
		{
			hasRenderPass = true;
			break;
		}
	}

	if (!hasRenderPass)
	{
		uint32_t passIndex = mtlShader->usedRenderPassCount;
		if (!DS_RESIZEABLE_ARRAY_ADD(mtlShader->scratchAllocator, mtlShader->usedRenderPasses,
			mtlShader->usedRenderPassCount, mtlShader->maxUsedRenderPasses, 1))
		{
			dsMTLPipeline_destroy(mtlShader->pipelines[index]);
			--mtlShader->pipelineCount;
			DS_VERIFY(dsSpinlock_unlock(&mtlShader->pipelineLock));
			return 0;
		}

		mtlShader->usedRenderPasses[passIndex] = dsLifetime_addRef(mtlRenderPass->lifetime);
	}

	id<MTLRenderPipelineState> mtlPipeline =
		(__bridge id<MTLRenderPipelineState>)mtlShader->pipelines[index]->pipeline;
	DS_VERIFY(dsSpinlock_unlock(&mtlShader->pipelineLock));

	if (!hasRenderPass)
		dsMTLRenderPass_addShader(renderPass, shader);

	return mtlPipeline;
}

void dsMTLShader_removeRenderPass(dsShader* shader, dsRenderPass* renderPass)
{
	dsMTLShader* mtlShader = (dsMTLShader*)shader;
	dsMTLRenderPass* mtlRenderPass = (dsMTLRenderPass*)renderPass;
	DS_VERIFY(dsSpinlock_lock(&mtlShader->pipelineLock));

	// Unregister the render pass.
	bool wasRegistered = false;
	for (uint32_t i = 0; i < mtlShader->usedRenderPassCount; ++i)
	{
		dsLifetime* renderPassLifetime = mtlShader->usedRenderPasses[i];
		if (renderPassLifetime == mtlRenderPass->lifetime)
		{
			DS_VERIFY(DS_RESIZEABLE_ARRAY_REMOVE(mtlShader->usedRenderPasses,
				mtlShader->usedRenderPassCount, i, 1));
			dsLifetime_freeRef(renderPassLifetime);
			wasRegistered = true;
			break;
		}
	}

	if (!wasRegistered)
	{
		DS_VERIFY(dsSpinlock_unlock(&mtlShader->pipelineLock));
		return;
	}

	// Remove all pipelines for the render pass.
	for (uint32_t i = 0; i < mtlShader->pipelineCount;)
	{
		if (mtlShader->pipelines[i]->renderPass == mtlRenderPass->lifetime)
		{
			dsMTLPipeline_destroy(mtlShader->pipelines[i]);
			DS_VERIFY(DS_RESIZEABLE_ARRAY_REMOVE(mtlShader->pipelines, mtlShader->pipelineCount, i,
				1));
		}
		else
			++i;
	}

	DS_VERIFY(dsSpinlock_unlock(&mtlShader->pipelineLock));
}

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

#include "Resources/MTLPipeline.h"
#include "Resources/MTLResourceManager.h"

#include <DeepSea/Core/Containers/Hash.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/Lifetime.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Bits.h>
#include <DeepSea/Core/Log.h>
#include <string.h>

#import <Metal/MTLRenderPipeline.h>

static dsPrimitiveType normalizePrimitiveType(dsPrimitiveType type)
{
	switch (type)
	{
		case dsPrimitiveType_PointList:
			return dsPrimitiveType_PointList;
		case dsPrimitiveType_LineList:
		case dsPrimitiveType_LineStrip:
		case dsPrimitiveType_LineListAdjacency:
			return dsPrimitiveType_LineList;
		case dsPrimitiveType_TriangleList:
		case dsPrimitiveType_TriangleStrip:
		case dsPrimitiveType_TriangleFan:
		case dsPrimitiveType_TriangleStripAdjacency:
		case dsPrimitiveType_PatchList:
			return dsPrimitiveType_TriangleList;
		default:
			DS_ASSERT(false);
			return dsPrimitiveType_TriangleList;
	}
}

static MTLColorWriteMask convertColorWriteMask(mslColorMask mask)
{
	MTLColorWriteMask outMask = MTLColorWriteMaskNone;
	if (mask & mslColorMask_Red)
		outMask |= MTLColorWriteMaskRed;
	if (mask & mslColorMask_Green)
		outMask |= MTLColorWriteMaskGreen;
	if (mask & mslColorMask_Blue)
		outMask |= MTLColorWriteMaskBlue;
	if (mask & mslColorMask_Alpha)
		outMask |= MTLColorWriteMaskAlpha;
	return outMask;
}

static MTLBlendOperation convertBlendOp(mslBlendOp op)
{
	switch (op)
	{
		case mslBlendOp_Subtract:
			return MTLBlendOperationSubtract;
		case mslBlendOp_ReverseSubtract:
			return MTLBlendOperationReverseSubtract;
		case mslBlendOp_Min:
			return MTLBlendOperationMin;
		case mslBlendOp_Max:
			return MTLBlendOperationMax;
		case mslBlendOp_Add:
		default:
			return MTLBlendOperationAdd;
	}
}

static MTLBlendFactor convertBlendFactor(mslBlendFactor factor, MTLBlendFactor defaultValue)
{
	switch (factor)
	{
		case mslBlendFactor_Zero:
			return MTLBlendFactorZero;
		case mslBlendFactor_One:
			return MTLBlendFactorOne;
		case mslBlendFactor_SrcColor:
			return MTLBlendFactorSourceColor;
		case mslBlendFactor_OneMinusSrcColor:
			return MTLBlendFactorOneMinusSourceColor;
		case mslBlendFactor_DstColor:
			return MTLBlendFactorDestinationColor;
		case mslBlendFactor_OneMinusDstColor:
			return MTLBlendFactorOneMinusDestinationColor;
		case mslBlendFactor_SrcAlpha:
			return MTLBlendFactorSourceAlpha;
		case mslBlendFactor_OneMinusSrcAlpha:
			return MTLBlendFactorOneMinusSourceAlpha;
		case mslBlendFactor_DstAlpha:
			return MTLBlendFactorDestinationAlpha;
		case mslBlendFactor_OneMinusDstAlpha:
			return MTLBlendFactorOneMinusDestinationAlpha;
		case mslBlendFactor_ConstColor:
			return MTLBlendFactorBlendColor;
		case mslBlendFactor_OneMinusConstColor:
			return MTLBlendFactorOneMinusBlendColor;
		case mslBlendFactor_ConstAlpha:
			return MTLBlendFactorBlendAlpha;
		case mslBlendFactor_OneMinusConstAlpha:
			return MTLBlendFactorOneMinusBlendAlpha;
		case mslBlendFactor_SrcAlphaSaturate:
			return MTLBlendFactorSourceAlphaSaturated;
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 110000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101200
		case mslBlendFactor_Src1Color:
			return MTLBlendFactorSource1Color;
		case mslBlendFactor_OneMinusSrc1Color:
			return MTLBlendFactorOneMinusSource1Color;
		case mslBlendFactor_Src1Alpha:
			return MTLBlendFactorSource1Alpha;
		case mslBlendFactor_OneMinusSrc1Alpha:
			return MTLBlendFactorOneMinusSource1Alpha;
#else
		case mslBlendFactor_Src1Color:
			return MTLBlendFactorSourceColor;
		case mslBlendFactor_OneMinusSrc1Color:
			return MTLBlendFactorOneMinusSourceColor;
		case mslBlendFactor_Src1Alpha:
			return MTLBlendFactorSourceAlpha;
		case mslBlendFactor_OneMinusSrc1Alpha:
			return MTLBlendFactorOneMinusSourceAlpha;
#endif
		default:
			return defaultValue;
	}
}

static MTLPixelFormat getDepthFormat(const dsResourceManager* resourceManager, dsGfxFormat format)
{
	switch (format)
	{
		case dsGfxFormat_D16:
		case dsGfxFormat_X8D24:
		case dsGfxFormat_D32_Float:
		case dsGfxFormat_D24S8:
			return dsMTLResourceManager_getPixelFormat(resourceManager, format);
		case dsGfxFormat_D16S8:
		{
			MTLPixelFormat pixelFormat = dsMTLResourceManager_getPixelFormat(resourceManager,
				format);
			if (pixelFormat == MTLPixelFormatInvalid)
				pixelFormat = dsMTLResourceManager_getPixelFormat(resourceManager, dsGfxFormat_D16);
			return pixelFormat;
		}
		case dsGfxFormat_D32S8_Float:
		{
			MTLPixelFormat pixelFormat = dsMTLResourceManager_getPixelFormat(resourceManager,
				format);
			if (pixelFormat == MTLPixelFormatInvalid)
			{
				pixelFormat = dsMTLResourceManager_getPixelFormat(resourceManager,
					dsGfxFormat_D32_Float);
			}
			return pixelFormat;
		}
		default:
			return MTLPixelFormatInvalid;
	}
}

static MTLPixelFormat getStencilFormat(const dsResourceManager* resourceManager, dsGfxFormat format)
{
	switch (format)
	{
		case dsGfxFormat_S8:
		case dsGfxFormat_D24S8:
			return dsMTLResourceManager_getPixelFormat(resourceManager, format);
		case dsGfxFormat_D16S8:
		case dsGfxFormat_D32S8_Float:
		{
			MTLPixelFormat pixelFormat = dsMTLResourceManager_getPixelFormat(resourceManager,
				format);
			if (pixelFormat == MTLPixelFormatInvalid)
				pixelFormat = dsMTLResourceManager_getPixelFormat(resourceManager, dsGfxFormat_S8);
			return pixelFormat;
		}
		default:
			return MTLPixelFormatInvalid;
	}
}

#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 120000
static MTLPrimitiveTopologyClass getPrimitiveTopology(dsPrimitiveType type)
{
	switch (type)
	{
		case dsPrimitiveType_PointList:
			return MTLPrimitiveTopologyClassPoint;
		case dsPrimitiveType_LineList:
		case dsPrimitiveType_LineStrip:
		case dsPrimitiveType_LineListAdjacency:
			return MTLPrimitiveTopologyClassLine;
		case dsPrimitiveType_TriangleList:
		case dsPrimitiveType_TriangleStrip:
		case dsPrimitiveType_TriangleFan:
		case dsPrimitiveType_TriangleStripAdjacency:
		case dsPrimitiveType_PatchList:
			return MTLPrimitiveTopologyClassTriangle;
		default:
			DS_ASSERT(false);
			return MTLPrimitiveTopologyClassTriangle;
	}
}
#endif

static bool setupVertexState(dsShader* shader, MTLRenderPipelineDescriptor* descriptor,
	const dsVertexFormat formats[DS_MAX_GEOMETRY_VERTEX_BUFFERS])
{
	dsMTLShader* mtlShader = (dsMTLShader*)shader;
	dsResourceManager* resourceManager = shader->resourceManager;
	MTLVertexDescriptor* vertexDescriptor = [MTLVertexDescriptor new];
	if (!vertexDescriptor)
	{
		errno = ENOMEM;
		return false;
	}

	for (uint32_t i = 0; i < DS_MAX_GEOMETRY_VERTEX_BUFFERS; ++i)
	{
		if (formats[i].enabledMask == 0)
			continue;

		uint32_t bufferIndex = mtlShader->firstVertexBuffer + i;
		MTLVertexBufferLayoutDescriptor* layout = vertexDescriptor.layouts[bufferIndex];
		if (!layout)
		{
			errno = ENOMEM;
			return false;
		}

		if (formats[i].instanced)
		{
			layout.stepFunction = MTLVertexStepFunctionPerInstance;
			layout.stepRate = 1;
		}
		layout.stride = formats[i].size;

		for (uint32_t curBitmask = formats[i].enabledMask; curBitmask;
			curBitmask = dsRemoveLastBit(curBitmask))
		{
			uint32_t j = dsBitmaskIndex(curBitmask);
			MTLVertexAttributeDescriptor* attribute = vertexDescriptor.attributes[j];
			if (!attribute)
			{
				errno = ENOMEM;
				return false;
			}

			const dsVertexElement* element = formats[i].elements + j;
			MTLVertexFormat format = dsMTLResourceManager_getVertexFormat(resourceManager,
				element->format);
			if (format == MTLVertexFormatInvalid)
			{
				DS_LOG_ERROR(DS_RENDER_METAL_LOG_TAG, "Unkonwn vertex format.");
				errno = EPERM;
				return false;
			}

			attribute.format = format;
			attribute.offset = element->offset;
			attribute.bufferIndex = bufferIndex;
		}
	}

	descriptor.vertexDescriptor = vertexDescriptor;
	return true;
}

static void setupColorAttachments(dsShader* shader, MTLRenderPipelineDescriptor* descriptor,
	const dsRenderPass* renderPass, uint32_t subpass)
{
	dsResourceManager* resourceManager = shader->resourceManager;
	dsMTLShader* mtlShader = (dsMTLShader*)shader;
	const mslBlendState* blendState = &mtlShader->renderState.blendState;
	const dsRenderSubpassInfo* subpassInfo = renderPass->subpasses + subpass;
	for (uint32_t i = 0; i < subpassInfo->colorAttachmentCount; ++i)
	{
		uint32_t attachmentIndex = subpassInfo->colorAttachments[i].attachmentIndex;
		if (attachmentIndex == DS_NO_ATTACHMENT)
			continue;

		DS_ASSERT(i < MSL_MAX_ATTACHMENTS);
		const mslBlendAttachmentState* attachmentState = blendState->blendAttachments + i;
		const dsAttachmentInfo* attachment = renderPass->attachments + attachmentIndex;
		MTLRenderPipelineColorAttachmentDescriptor* colorDescriptor =
			descriptor.colorAttachments[i];
		colorDescriptor.pixelFormat = dsMTLResourceManager_getPixelFormat(resourceManager,
			attachment->format);
		colorDescriptor.writeMask = convertColorWriteMask(attachmentState->colorWriteMask);
		colorDescriptor.blendingEnabled = attachmentState->blendEnable == mslBool_True;
		colorDescriptor.alphaBlendOperation = convertBlendOp(attachmentState->alphaBlendOp);
		colorDescriptor.rgbBlendOperation = convertBlendOp(attachmentState->colorBlendOp);
		colorDescriptor.destinationAlphaBlendFactor = convertBlendFactor(
			attachmentState->dstAlphaBlendFactor, MTLBlendFactorZero);
		colorDescriptor.destinationRGBBlendFactor = convertBlendFactor(
			attachmentState->dstColorBlendFactor, MTLBlendFactorZero);
		colorDescriptor.sourceAlphaBlendFactor = convertBlendFactor(
			attachmentState->srcAlphaBlendFactor, MTLBlendFactorOne);
		colorDescriptor.sourceRGBBlendFactor = convertBlendFactor(
			attachmentState->srcColorBlendFactor, MTLBlendFactorOne);
	}
}

static void setupDepthStencilAttachment(dsShader* shader, MTLRenderPipelineDescriptor* descriptor,
	const dsRenderPass* renderPass, uint32_t subpass)
{
	dsResourceManager* resourceManager = shader->resourceManager;
	const dsRenderSubpassInfo* subpassInfo = renderPass->subpasses + subpass;
	if (subpassInfo->depthStencilAttachment == DS_NO_ATTACHMENT)
		return;

	const dsAttachmentInfo* attachment = renderPass->attachments +
		subpassInfo->depthStencilAttachment;
	descriptor.depthAttachmentPixelFormat = getDepthFormat(resourceManager, attachment->format);
	descriptor.stencilAttachmentPixelFormat = getStencilFormat(resourceManager, attachment->format);
}

static void setupRasterState(dsShader* shader, MTLRenderPipelineDescriptor* descriptor,
	uint32_t samples, dsPrimitiveType primitiveType)
{
	dsMTLShader* mtlShader = (dsMTLShader*)shader;
	const mslRasterizationState* rasterState = &mtlShader->renderState.rasterizationState;
	const mslMultisampleState* multisampleState = &mtlShader->renderState.multisampleState;
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 110000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
	descriptor.rasterSampleCount = samples;
#else
	descriptor.sampleCount = samples;
#endif
	descriptor.alphaToCoverageEnabled = multisampleState->alphaToCoverageEnable == mslBool_True;
	descriptor.alphaToOneEnabled = multisampleState->alphaToOneEnable == mslBool_True;
	descriptor.rasterizationEnabled = rasterState->rasterizerDiscardEnable != mslBool_False;
#if DS_MAC || __IPHONE_OS_VERSION_MIN_REQUIRED >= 120000
	descriptor.inputPrimitiveTopology = getPrimitiveTopology(primitiveType);
#else
	DS_UNUSED(primitiveType);
#endif
}

static void setupBufferInfos(dsShader* shader, MTLRenderPipelineDescriptor* descriptor)
{
#if __IPHONE_OS_VERSION_MIN_REQUIRED >= 110000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101300
	dsMTLShader* mtlShader = (dsMTLShader*)shader;
	if (mtlShader->stages[mslStage_Vertex].hasPushConstants)
		descriptor.vertexBuffers[0].mutability = MTLMutabilityImmutable;
	if (mtlShader->stages[mslStage_Fragment].hasPushConstants)
		descriptor.fragmentBuffers[0].mutability = MTLMutabilityImmutable;

	const dsMaterialDesc* materialDesc = shader->materialDesc;
	for (uint32_t i = 0; i < mtlShader->uniformCount; ++i)
	{
		uint32_t vertexIndex = mtlShader->stages[mslStage_Vertex].uniformIndices[i];
		uint32_t fragmentIndex = mtlShader->stages[mslStage_Fragment].uniformIndices[i];
		if (vertexIndex == DS_MATERIAL_UNKNOWN && fragmentIndex == DS_MATERIAL_UNKNOWN)
			continue;

		const dsMTLUniformInfo* info = mtlShader->uniformInfos + i;
		if (info->element == DS_MATERIAL_UNKNOWN)
			continue;

		const dsMaterialElement* element = materialDesc->elements + info->element;
		if (element->type == dsMaterialType_VariableGroup ||
			element->type == dsMaterialType_UniformBlock)
		{
			if (vertexIndex != DS_MATERIAL_UNKNOWN)
				descriptor.vertexBuffers[vertexIndex].mutability = MTLMutabilityImmutable;
			if (fragmentIndex != DS_MATERIAL_UNKNOWN)
				descriptor.fragmentBuffers[fragmentIndex].mutability = MTLMutabilityImmutable;
		}
	}
#else
	DS_UNUSED(shader);
	DS_UNUSED(descriptor);
#endif
}

uint32_t dsMTLPipeline_hash(uint32_t samples, dsPrimitiveType primitiveType,
	uint32_t vertexFormatHash, const dsRenderPass* renderPass, uint32_t subpass)
{
	const dsMTLRenderPass* mtlRenderPass = (const dsMTLRenderPass*)renderPass;
	uint32_t hash = dsHash32(&samples);
	uint32_t primitiveType32 = normalizePrimitiveType(primitiveType);
	hash = dsHashCombine32(hash, &primitiveType32);
	hash = dsHashCombine(hash, vertexFormatHash);
	hash = dsHashCombinePointer(hash, mtlRenderPass->lifetime);
	return dsHashCombine32(hash, &subpass);
}

dsMTLPipeline* dsMTLPipeline_create(dsAllocator* allocator, dsShader* shader, uint32_t hash,
	uint32_t samples, dsPrimitiveType primitiveType, const dsDrawGeometry* geometry,
	const dsRenderPass* renderPass, uint32_t subpass)
{
	@autoreleasepool
	{
		dsMTLPipeline* pipeline = DS_ALLOCATE_OBJECT(allocator, dsMTLPipeline);
		if (!pipeline)
			return NULL;

		dsMTLShader* mtlShader = (dsMTLShader*)shader;
		dsResourceManager* resourceManager = shader->resourceManager;
		dsMTLRenderer* mtlRenderer = (dsMTLRenderer*)resourceManager->renderer;
		id<MTLDevice> device = (__bridge id<MTLDevice>)mtlRenderer->device;
		const dsMTLRenderPass* mtlRenderPass = (const dsMTLRenderPass*)renderPass;

		pipeline->allocator = dsAllocator_keepPointer(allocator);
		pipeline->pipeline = NULL;
		pipeline->hash = hash;
		pipeline->samples = samples;
		pipeline->primitiveType = normalizePrimitiveType(primitiveType);
		pipeline->renderPass = dsLifetime_addRef(mtlRenderPass->lifetime);
		pipeline->subpass = subpass;
		for (uint32_t i = 0; i < DS_MAX_GEOMETRY_VERTEX_BUFFERS; ++i)
		{
			memcpy(pipeline->formats + i, &geometry->vertexBuffers[i].format,
				sizeof(dsVertexFormat));
		}

		// Maybe add support for tessellation shaders some day. See rant in hasTessellationShaders()
		// function in MTLRenderer.m for why this isn't implemented for now.
		MTLRenderPipelineDescriptor* descriptor = [MTLRenderPipelineDescriptor new];
		if (!descriptor)
		{
			errno = ENOMEM;
			dsMTLPipeline_destroy(pipeline);
			return NULL;
		}

		if (!setupVertexState(shader, descriptor, pipeline->formats))
		{
			dsMTLPipeline_destroy(pipeline);
			return NULL;
		}

		descriptor.vertexFunction =
			(__bridge id<MTLFunction>)mtlShader->stages[mslStage_Vertex].function;
		descriptor.fragmentFunction =
			(__bridge id<MTLFunction>)mtlShader->stages[mslStage_Fragment].function;
		setupColorAttachments(shader, descriptor, renderPass, subpass);
		setupDepthStencilAttachment(shader, descriptor, renderPass, subpass);
		setupRasterState(shader, descriptor, samples, primitiveType);
		setupBufferInfos(shader, descriptor);

		NSError* error = NULL;
		id<MTLRenderPipelineState> renderPipeline =
			[device newRenderPipelineStateWithDescriptor: descriptor error: &error];
		if (error)
		{
			errno = EPERM;
			DS_LOG_ERROR_F(DS_RENDER_METAL_LOG_TAG,
				"Error creating pipeline for shader %s.%s: %s", shader->module->name,
				shader->name, error.localizedDescription.UTF8String);
			dsMTLPipeline_destroy(pipeline);
			return NULL;
		}
		else if (!renderPipeline)
		{
			DS_LOG_ERROR_F(DS_RENDER_METAL_LOG_TAG,
				"Error creating pipeline for shader %s.%s.", shader->module->name,
				shader->name);
			dsMTLPipeline_destroy(pipeline);
			return NULL;
		}

		pipeline->pipeline = CFBridgingRetain(renderPipeline);

		return pipeline;
	}
}

bool dsMTLPipeline_isEquivalent(const dsMTLPipeline* pipeline, uint32_t hash, uint32_t samples,
	dsPrimitiveType primitiveType, const dsDrawGeometry* geometry, const dsRenderPass* renderPass,
	uint32_t subpass)
{
	if (pipeline->hash != hash)
		return false;

	const dsMTLRenderPass* mtlRenderPass = (const dsMTLRenderPass*)renderPass;
	if (pipeline->samples != samples ||
		pipeline->primitiveType != normalizePrimitiveType(primitiveType) ||
		pipeline->renderPass != mtlRenderPass->lifetime || pipeline->subpass != subpass)
	{
		return false;
	}

	for (uint32_t i = 0; i < DS_MAX_GEOMETRY_VERTEX_BUFFERS; ++i)
	{
		if (memcmp(pipeline->formats + i, &geometry->vertexBuffers[i].format,
			sizeof(dsVertexFormat)) != 0)
		{
			return false;
		}
	}

	return true;
}

void dsMTLPipeline_destroy(dsMTLPipeline* pipeline)
{
	if (!pipeline)
		return;

	if (pipeline->pipeline)
		CFRelease(pipeline->pipeline);
	dsLifetime_freeRef(pipeline->renderPass);
	if (pipeline->allocator)
		DS_VERIFY(dsAllocator_free(pipeline->allocator, pipeline));
}

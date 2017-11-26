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
#include <DeepSea/Render/Resources/Types.h>
#include <MSL/Client/TypesC.h>

dsTexture* dsGLTexture_create(dsResourceManager* resourceManager, dsAllocator* allocator,
	unsigned int usage, unsigned int memoryHints, dsGfxFormat format, dsTextureDim dimension,
	uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, const void* data,
	size_t size);
dsOffscreen* dsGLTexture_createOffscreen(dsResourceManager* resourceManager, dsAllocator* allocator,
	unsigned int usage, unsigned int memoryHints, dsGfxFormat format, dsTextureDim dimension,
	uint32_t width, uint32_t height, uint32_t depth, uint32_t mipLevels, uint32_t samples,
	bool resolve);
bool dsGLTexture_copyData(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height,
	uint32_t layers, const void* data, size_t size);
bool dsGLTexture_copy(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* srcTexture, dsTexture* dstTexture, const dsTextureCopyRegion* regions,
	size_t regionCount);
bool dsGLTexture_blit(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* srcTexture, dsTexture* dstTexture, const dsTextureBlitRegion* regions,
	size_t regionCount, dsBlitFilter filter);
bool dsGLTexture_generateMipmaps(dsResourceManager* resourceManager, dsCommandBuffer* commandBuffer,
	dsTexture* texture);
bool dsGLTexture_getData(void* result, size_t size, dsResourceManager* resourceManager,
	dsTexture* texture, const dsTexturePosition* position, uint32_t width, uint32_t height);
bool dsGLTexture_destroy(dsResourceManager* resourceManager, dsTexture* texture);

GLenum dsGLTexture_target(const dsTexture* texture);
GLenum dsGLTexture_attachment(dsGfxFormat format);
GLbitfield dsGLTexture_buffers(const dsTexture* texture);
void dsGLTexture_bindFramebuffer(dsTexture* texture, GLenum framebuffer, uint32_t mipLevel,
	uint32_t layer);
void dsGLTexture_bindFramebufferAttachment(dsTexture* texture, GLenum framebuffer,
	GLenum attachment, uint32_t mipLevel, uint32_t layer);
void dsGLTexture_bindFramebufferTexture(dsTexture* texture, GLenum framebuffer, uint32_t mipLevel,
	uint32_t layer);
void dsGLTexture_bindFramebufferTextureAttachment(dsTexture* texture, GLenum framebuffer,
	GLenum attachment, uint32_t mipLevel, uint32_t layer);
void dsGLTexture_unbindFramebuffer(dsTexture* texture, GLenum framebuffer);
void dsGLTexture_setState(dsTexture* texture, const mslSamplerState* samplerState,
	bool isShadowSampler);
void dsGLTexture_addInternalRef(dsTexture* texture);
void dsGLTexture_freeInternalRef(dsTexture* texture);

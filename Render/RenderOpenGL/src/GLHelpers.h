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

#include "AnyGL/gl.h"
#include <DeepSea/Render/Types.h>

void dsCheckGLErrors(void);
void dsClearGLErrors(void);
GLenum dsGetLastGLError(void);
int dsGetGLErrno(GLenum error);

void dsGLBindFramebufferTexture(GLenum framebuffer, dsTexture* texture, uint32_t mipLevel,
	uint32_t layer);

void dsGLUnbindFramebufferTexture(GLenum framebuffer, dsTexture* texture);

bool dsGLAddToBuffer(dsAllocator* allocator, void** buffer, size_t* curElems,
	size_t* maxElems, size_t elemSize, size_t addElems);

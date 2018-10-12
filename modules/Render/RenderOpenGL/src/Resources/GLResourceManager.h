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
#include "GLTypes.h"

dsGLResourceManager* dsGLResourceManager_create(dsAllocator* allocator, dsGLRenderer* renderer);

bool dsGLResourceManager_getVertexFormatInfo(GLenum* outFormat, GLint* outElements,
	bool* outNormalized, const dsResourceManager* resourceManager, dsGfxFormat format);
bool dsGLResourceManager_getTextureFormatInfo(GLenum* outInternalFormat, GLenum* outFormat,
	GLenum* outType, const dsResourceManager* resourceManager, dsGfxFormat format);

void dsGLResourceManager_destroy(dsGLResourceManager* resourceManager);

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
#include "Types.h"

bool dsGLRenderer_bindSurface(dsRenderer* renderer, void* glSurface);
void dsGLRenderer_destroySurface(dsRenderer* renderer, void* glSurface);

void dsGLRenderer_destroyVao(dsRenderer* renderer, GLuint vao, uint32_t contextCount);
void dsGLRenderer_destroyFbo(dsRenderer* renderer, GLuint fbo, uint32_t contextCount);
void dsGLRenderer_destroyTexture(dsRenderer* renderer, GLuint texture);

GLuint dsGLRenderer_tempFramebuffer(dsRenderer* renderer);
GLuint dsGLRenderer_tempCopyFramebuffer(dsRenderer* renderer);
void dsGLRenderer_restoreFramebuffer(dsRenderer* renderer);

dsGLFenceSync* dsGLRenderer_createSync(dsRenderer* renderer, GLsync sync);
dsGLFenceSyncRef* dsGLRenderer_createSyncRef(dsRenderer* renderer);

void dsGLRenderer_bindTexture(dsRenderer* renderer, unsigned int unit, GLenum target,
	GLuint texture);
void dsGLRenderer_beginTextureOp(dsRenderer* renderer, GLenum target, GLuint texture);
void dsGLRenderer_endTextureOp(dsRenderer* renderer);

void dsGLRenderer_bindFramebuffer(dsRenderer* renderer, GLSurfaceType surfaceType,
	GLuint framebuffer);

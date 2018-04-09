/*
 * Copyright 2018 Aaron Barany
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

#include "SetupOpenGL.h"
#include <DeepSea/RenderOpenGL/GLRenderer.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

dsRenderer* dsTestVectorDraw_createGLRenderer(dsAllocator* allocator)
{
	dsOpenGLOptions options;
	dsGLRenderer_defaultOptions(&options);
	options.depthBits = 0;
	options.stencilBits = 0;
	dsRenderer* renderer = dsGLRenderer_create(allocator, &options);

#ifndef NDEBUG
	if (renderer)
		dsGLRenderer_setEnableErrorChecking(renderer, true);
#endif

	return renderer;
}

void dsTestVectorDraw_destroyGLRenderer(dsRenderer* renderer)
{
	dsGLRenderer_destroy(renderer);
}

const char* dsTestVectorDraw_getGLShaderDir(dsRenderer* renderer)
{
	uint32_t version;
	bool gles;
	DS_VERIFY(dsGLRenderer_getShaderVersion(&version, &gles, renderer));
	if (gles)
	{
		if (version >= 320)
			return "glsl-es-3.2";
		else if (version >= 300)
			return "glsl-es-3.0";
		return "glsl-es-1.0";
	}
	else
	{
		if (version >= 400)
			return "glsl-4.0";
		else if (version >= 150)
			return "glsl-1.5";
		return "glsl-1.1";
	}
}

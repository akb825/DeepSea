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

#include "SetupOpenGL.h"
#include <DeepSea/RenderOpenGL/GLRenderer.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>

dsRenderer* dsTestCube_createGLRenderer(dsAllocator* allocator)
{
	dsOpenGLOptions options;
	dsGLRenderer_defaultOptions(&options);
	dsRenderer* renderer = dsGLRenderer_create(allocator, &options);
	if (!renderer && errno == EPERM)
	{
		DS_LOG_INFO("TestCube",
			"Failed creating OpenGL renderer. Trying again without anti-aliasing.");
		renderer = dsGLRenderer_create(allocator, &options);
	}

#ifndef NDEBUG
	if (renderer)
		dsGLRenderer_setEnableErrorChecking(renderer, true);
#endif

	return renderer;
}

void dsTestCube_destroyGLRenderer(dsRenderer* renderer)
{
	dsGLRenderer_destroy(renderer);
}

const char* dsTestCube_getGLShaderDir(dsRenderer* renderer)
{
	bool gles;
	DS_VERIFY(dsGLRenderer_getShaderVersion(NULL, &gles, renderer));
	if (gles)
		return "glsl-es-1.0";
	else
		return "glsl-1.1";
}

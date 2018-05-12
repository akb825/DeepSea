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
#include <DeepSea/Application/Application.h>
#include <DeepSea/Application/Window.h>
#include <DeepSea/ApplicationSDL/SDLApplication.h>
#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Math/Vector2.h>
#include <DeepSea/Render/Resources/Framebuffer.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <DeepSea/Render/Resources/ShaderVariableGroup.h>
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Render/RenderPass.h>
#include <DeepSea/VectorDraw/VectorImage.h>
#include <DeepSea/VectorDraw/VectorResources.h>
#include <DeepSea/VectorDraw/VectorScratchData.h>
#include <DeepSea/VectorDraw/VectorShaderModule.h>
#include <DeepSea/VectorDraw/VectorShaders.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum dsRenderType
{
	dsRenderType_OpenGL,
	dsRenderType_Count
} dsRenderType;

typedef struct TestVectorDraw
{
	dsAllocator* allocator;
	dsRenderer* renderer;
	dsWindow* window;
	dsFramebuffer* framebuffer;
	dsRenderPass* renderPass;
	dsVectorShaderModule* shaderModule;
	dsVectorShaders* shaders;
	dsVectorShaders* wireframeShaders;
	dsMaterial* material;
	dsVectorResources* vectorResources;
	dsVectorImage** vectorImages;

	uint32_t vectorImageCount;
	uint32_t curVectorImage;
	bool wireframe;
} TestVectorDraw;

static const char* renderTypeNames[] =
{
	"OpenGL"
};

DS_STATIC_ASSERT(DS_ARRAY_SIZE(renderTypeNames) == dsRenderType_Count, renderer_type_mismatch);

#if DS_HAS_OPENGL
static dsRenderType defaultRenderType = dsRenderType_OpenGL;
#else
#error No renderer type available
#endif

#define TARGET_SIZE 800

static char assetsDir[DS_PATH_MAX];
static const char* shaderDir;

const char* vectorImageFiles[] =
{
	"polygon.dsvi",
	"line.dsvi",
	"polyline.dsvi",
	"polyline-miter-square.dsvi",
	"polyline-bevel-butt.dsvi",
	"polyline-round.dsvi",
	"polyline-dashed.dsvi",
	"circle.dsvi",
	"ellipse.dsvi",
	"rectangle.dsvi",
	"rectangle-rounded.dsvi",
	"path.dsvi",
	"curve.dsvi",
	"quadratic.dsvi",
	"arc-mixed-path.dsvi",
	"arc.dsvi",
	"linear-gradient.dsvi",
	"linear-gradient-repeat.dsvi",
	"radial-gradient.dsvi",
	"radial-gradient-focus.dsvi",
	"radial-gradient-repeat.dsvi"
};

typedef dsRenderer* (*CreateRendererFunction)(dsAllocator* allocator);
typedef void (*DestroyRendererFunction)(dsRenderer* renderer);
typedef const char* (*GetShaderDirFunction)(dsRenderer* renderer);

static void printHelp(const char* programPath)
{
	printf("usage: %s [OPTIONS]\n", dsPath_getFileName(programPath));
	printf("Use left/right arrows or tap on touchscreen to cyle images.\n");
	printf("Press 'w' to toggle wireframe.\n\n");
	printf("options:\n");
	printf("--srgb      use sRGB-correct drawing\n");
#if DS_HAS_OPENGL
	printf("--opengl    render using OpenGL\n");
#endif
	printf("default renderer: %s\n", renderTypeNames[defaultRenderType]);
}

static bool validateAllocator(dsAllocator* allocator, const char* name)
{
	if (allocator->size == 0)
		return true;

	DS_LOG_ERROR_F("TestVectorDraw", "Allocator '%s' has %llu bytes allocated with %u allocations.",
		name, (unsigned long long)allocator->size, allocator->currentAllocations);
	return false;
}

static bool createFramebuffer(TestVectorDraw* testVectorDraw)
{
	uint32_t width, height;
	if (!dsWindow_getPixelSize(&width, &height, testVectorDraw->window))
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't get window size: %s", dsErrorString(errno));
		return false;
	}

	dsFramebuffer_destroy(testVectorDraw->framebuffer);

	dsRenderSurface* surface = testVectorDraw->window->surface;
	dsFramebufferSurface surfaces[] =
	{
		{dsGfxSurfaceType_ColorRenderSurface, dsCubeFace_None, 0, 0, surface}
	};
	testVectorDraw->framebuffer = dsFramebuffer_create(testVectorDraw->renderer->resourceManager,
		testVectorDraw->allocator, surfaces, 1, width, height, 1);

	if (!testVectorDraw->framebuffer)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create framebuffer: %s", dsErrorString(errno));
		return false;
	}

	return true;
}

static void nextImage(TestVectorDraw* testVectorDraw)
{
	++testVectorDraw->curVectorImage;
	if (testVectorDraw->curVectorImage >= testVectorDraw->vectorImageCount)
		testVectorDraw->curVectorImage = 0;
}

static void prevImage(TestVectorDraw* testVectorDraw)
{
	if (testVectorDraw->curVectorImage == 0)
		testVectorDraw->curVectorImage = testVectorDraw->vectorImageCount - 1;
	else
		--testVectorDraw->curVectorImage;
}

static bool processEvent(dsApplication* application, dsWindow* window, const dsEvent* event,
	void* userData)
{
	DS_UNUSED(application);

	TestVectorDraw* testVectorDraw = (TestVectorDraw*)userData;
	DS_ASSERT(!window || window == testVectorDraw->window);
	switch (event->type)
	{
		case dsEventType_WindowClosed:
			DS_VERIFY(dsWindow_destroy(window));
			testVectorDraw->window = NULL;
			return false;
		case dsEventType_WindowResized:
			if (!createFramebuffer(testVectorDraw))
				abort();
			return true;
		case dsEventType_KeyDown:
			switch (event->key.key)
			{
				case dsKeyCode_Right:
					nextImage(testVectorDraw);
					return false;
				case dsKeyCode_Left:
					prevImage(testVectorDraw);
					return false;
				case dsKeyCode_W:
					testVectorDraw->wireframe = !testVectorDraw->wireframe;
					return false;
				default:
					return true;
			}
		case dsEventType_TouchFingerDown:
			nextImage(testVectorDraw);
			return true;
		default:
			return true;
	}
}

static void draw(dsApplication* application, dsWindow* window, void* userData)
{
	DS_UNUSED(application);
	DS_UNUSED(window);
	TestVectorDraw* testVectorDraw = (TestVectorDraw*)userData;
	DS_ASSERT(testVectorDraw->window == window);
	dsRenderer* renderer = testVectorDraw->renderer;
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsSurfaceClearValue clearValue;
	clearValue.colorValue.floatValue.r = 1.0f;
	clearValue.colorValue.floatValue.g = 1.0f;
	clearValue.colorValue.floatValue.b = 1.0f;
	clearValue.colorValue.floatValue.a = 1.0f;
	DS_VERIFY(dsRenderPass_begin(testVectorDraw->renderPass, commandBuffer,
		testVectorDraw->framebuffer, NULL, &clearValue, 1, false));

	dsVectorImage* image = testVectorDraw->vectorImages[testVectorDraw->curVectorImage];

	dsVector2f size;
	DS_VERIFY(dsVectorImage_getSize(&size, image));
	float windowAspect = (float)window->surface->width/(float)window->surface->height;
	float imageAspect = size.x/size.y;
	float imageToWindowAspect = windowAspect/imageAspect;
	if (imageToWindowAspect < 1.0f)
		size.y = size.x/windowAspect;
	else
		size.x = size.y*windowAspect;

	dsMatrix44f matrix;
	DS_VERIFY(dsRenderer_makeOrtho(&matrix, renderer, 0.0f, size.x, 0.0f, size.y, 0.0f, 1.0f));
	dsVectorShaders* shaders;
	if (testVectorDraw->wireframe)
		shaders = testVectorDraw->wireframeShaders;
	else
		shaders = testVectorDraw->shaders;
	DS_VERIFY(dsVectorImage_draw(image, commandBuffer, shaders, testVectorDraw->material, &matrix,
		NULL, NULL));

	DS_VERIFY(dsRenderPass_end(testVectorDraw->renderPass, commandBuffer));
}

static bool setup(TestVectorDraw* testVectorDraw, dsApplication* application,
	dsAllocator* allocator, bool srgb)
{
	dsRenderer* renderer = application->renderer;
	dsResourceManager* resourceManager = renderer->resourceManager;
	testVectorDraw->allocator = allocator;
	testVectorDraw->renderer = renderer;

	dsEventResponder responder = {&processEvent, testVectorDraw, 0, 0};
	DS_VERIFY(dsApplication_addEventResponder(application, &responder));

	testVectorDraw->window = dsWindow_create(application, allocator, "Test Vector Draw",
		NULL, TARGET_SIZE, TARGET_SIZE, dsWindowFlags_Resizeable);
	if (!testVectorDraw->window)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create window: %s", dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsWindow_setDrawFunction(testVectorDraw->window, &draw, testVectorDraw));

	if (!createFramebuffer(testVectorDraw))
		return false;

	dsAttachmentInfo attachment = {dsAttachmentUsage_Clear, renderer->surfaceColorFormat,
		DS_DEFAULT_ANTIALIAS_SAMPLES};

	dsColorAttachmentRef colorAttachment = {0, false};
	uint32_t depthStencilAttachment = DS_NO_ATTACHMENT;
	dsRenderSubpassInfo subpass =
	{
		"TestVectorDraw", NULL, &colorAttachment, 0, 1, depthStencilAttachment
	};
	testVectorDraw->renderPass = dsRenderPass_create(renderer, allocator, &attachment, 1, &subpass,
		1, NULL, 0);
	if (!testVectorDraw->renderPass)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create render pass: %s", dsErrorString(errno));
		return false;
	}

	DS_ASSERT(shaderDir);
	char path[DS_PATH_MAX];
	const char* shaderFilename = srgb ? "TestVectorDrawSRGB.mslb" : "TestVectorDraw.mslb";
	if (!dsPath_combine(path, sizeof(path), assetsDir, shaderDir) ||
		!dsPath_combine(path, sizeof(path), path, shaderFilename))
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create shader path: %s", dsErrorString(errno));
		return false;
	}

	testVectorDraw->shaderModule = dsVectorShaderModule_loadFile(resourceManager, allocator, path,
		NULL, 0);
	if (!testVectorDraw->shaderModule)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't load shader module: %s", dsErrorString(errno));
		return false;
	}

	testVectorDraw->shaders = dsVectorShaders_create(resourceManager, allocator,
		testVectorDraw->shaderModule, DS_DEFAULT_ANTIALIAS_SAMPLES);
	if (!testVectorDraw->shaders)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create shaders: %s", dsErrorString(errno));
		return false;
	}

	testVectorDraw->wireframeShaders = dsVectorShaders_createCustom(resourceManager, allocator,
		testVectorDraw->shaderModule, "dsVectorShapeWireframe", NULL, NULL,
		DS_DEFAULT_ANTIALIAS_SAMPLES);
	if (!testVectorDraw->shaders)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create shaders: %s", dsErrorString(errno));
		return false;
	}

	testVectorDraw->material = dsMaterial_create(allocator,
		testVectorDraw->shaderModule->materialDesc);
	if (!testVectorDraw->material)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create material: %s", dsErrorString(errno));
		return false;
	}

	testVectorDraw->vectorResources = NULL;
	testVectorDraw->vectorImageCount = DS_ARRAY_SIZE(vectorImageFiles);
	testVectorDraw->vectorImages = DS_ALLOCATE_OBJECT_ARRAY(allocator, dsVectorImage*,
		testVectorDraw->vectorImageCount);
	if (!testVectorDraw->vectorImages)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't allocate vector image array: %s",
			dsErrorString(errno));
		return false;
	}
	memset(testVectorDraw->vectorImages, 0,
		sizeof(dsVectorImage*)*testVectorDraw->vectorImageCount);

	dsVectorScratchData* scratchData = dsVectorScratchData_create(allocator);
	if (!scratchData)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create vector scratch data: %s",
			dsErrorString(errno));
		return false;
	}

	dsVector2f targetSize = {{(float)TARGET_SIZE, (float)TARGET_SIZE}};
	dsVectorImageInitResources initResources = {resourceManager, scratchData, NULL,
		testVectorDraw->shaderModule, NULL, 0, srgb, renderer->mainCommandBuffer};
	for (uint32_t i = 0; i < testVectorDraw->vectorImageCount; ++i)
	{
		if (!dsPath_combine(path, sizeof(path), assetsDir, vectorImageFiles[i]))
		{
			DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create vector image path: %s",
				dsErrorString(errno));
			dsVectorScratchData_destroy(scratchData);
			return false;
		}

		testVectorDraw->vectorImages[i] = dsVectorImage_loadFile(allocator, NULL, &initResources,
			path, 1.0f, &targetSize);
		if (!testVectorDraw->vectorImages[i])
		{
			DS_LOG_ERROR_F("TestVectorDraw", "Couldn't load vector image: %s",
				dsErrorString(errno));
			dsVectorScratchData_destroy(scratchData);
			return false;
		}
	}

	dsVectorScratchData_destroy(scratchData);
	return true;
}

static void shutdown(TestVectorDraw* testVectorDraw)
{
	if (testVectorDraw->vectorImages)
	{
		for (uint32_t i = 0; i < testVectorDraw->vectorImageCount; ++i)
			DS_VERIFY(dsVectorImage_destroy(testVectorDraw->vectorImages[i]));
		DS_VERIFY(dsAllocator_free(testVectorDraw->allocator, testVectorDraw->vectorImages));
	}
	DS_VERIFY(dsVectorResources_destroy(testVectorDraw->vectorResources));
	dsMaterial_destroy(testVectorDraw->material);
	DS_VERIFY(dsVectorShaders_destroy(testVectorDraw->wireframeShaders));
	DS_VERIFY(dsVectorShaders_destroy(testVectorDraw->shaders));
	DS_VERIFY(dsVectorShaderModule_destroy(testVectorDraw->shaderModule));
	DS_VERIFY(dsRenderPass_destroy(testVectorDraw->renderPass));
	DS_VERIFY(dsFramebuffer_destroy(testVectorDraw->framebuffer));
	DS_VERIFY(dsWindow_destroy(testVectorDraw->window));
}

int dsMain(int argc, const char** argv)
{
	dsRenderType renderType = defaultRenderType;
	bool srgb = false;
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			printHelp(argv[0]);
			return 0;
		}
		else if (strcmp(argv[i], "--srgb") == 0)
			srgb = true;
#if DS_HAS_OPENGL
		else if (strcmp(argv[i], "--opengl") == 0)
			renderType = dsRenderType_OpenGL;
#endif
		else
		{
			printf("Unknown option: %s\n", argv[i]);
			printHelp(argv[0]);
			return 1;
		}
	}

	DS_VERIFY(dsPath_getDirectoryName(assetsDir, sizeof(assetsDir), argv[0]));
	DS_VERIFY(dsPath_combine(assetsDir, sizeof(assetsDir), assetsDir, "TestVectorDraw-assets"));

	DS_LOG_INFO_F("TestVectorDraw", "Render using %s", renderTypeNames[renderType]);

	CreateRendererFunction createRendererFunc = NULL;
	DestroyRendererFunction destroyRendererFunc = NULL;
	GetShaderDirFunction getShaderDirFunc = NULL;
	switch (renderType)
	{
#if DS_HAS_OPENGL
		case dsRenderType_OpenGL:
			createRendererFunc = &dsTestVectorDraw_createGLRenderer;
			destroyRendererFunc = &dsTestVectorDraw_destroyGLRenderer;
			getShaderDirFunc = &dsTestVectorDraw_getGLShaderDir;
			break;
#endif
		default:
			DS_ASSERT(false);
			break;
	}

	dsSystemAllocator renderAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&renderAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator applicationAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&applicationAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator testVectorDrawAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&testVectorDrawAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsRenderer* renderer = createRendererFunc((dsAllocator*)&renderAllocator);
	if (!renderer)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create renderer: %s", dsErrorString(errno));
		return 2;
	}
	dsRenderer_setVsync(renderer, true);
	dsRenderer_setDefaultAnisotropy(renderer, renderer->maxAnisotropy);

	if (srgb && !dsGfxFormat_textureSupported(renderer->resourceManager,
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_SRGB)))
	{
		DS_LOG_ERROR_F("TestVectorDraw", "sRGB requested but not supported by the current target.");
		return 2;
	}

	shaderDir = getShaderDirFunc(renderer);

	dsApplication* application = dsSDLApplication_create((dsAllocator*)&applicationAllocator,
		renderer);
	if (!application)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create application: %s", dsErrorString(errno));
		destroyRendererFunc(renderer);
		return 2;
	}

	TestVectorDraw testVectorDraw;
	memset(&testVectorDraw, 0, sizeof(testVectorDraw));
	if (!setup(&testVectorDraw, application, (dsAllocator*)&testVectorDrawAllocator, srgb))
	{
		shutdown(&testVectorDraw);
		return 3;
	}

	int exitCode = dsApplication_run(application);

	shutdown(&testVectorDraw);
	dsSDLApplication_destroy(application);
	destroyRendererFunc(renderer);

	if (!validateAllocator((dsAllocator*)&renderAllocator, "render"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&applicationAllocator, "application"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&testVectorDrawAllocator, "TestVectorDraw"))
		exitCode = 4;

	return exitCode;
}

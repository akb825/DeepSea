/*
 * Copyright 2018-2026 Aaron Barany
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

#include <DeepSea/Application/Application.h>
#include <DeepSea/Application/Window.h>
#include <DeepSea/ApplicationSDL/SDLApplication.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Core/Profile.h>
#include <DeepSea/Core/Timer.h>

#include <DeepSea/Geometry/AlignedBox2.h>

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
#include <DeepSea/Render/Resources/ShaderVariableGroupDesc.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Render/RenderPass.h>
#include <DeepSea/Render/RenderSurface.h>

#include <DeepSea/RenderBootstrap/RenderBootstrap.h>

#include <DeepSea/Text/TextureTextIcons.h>

#include <DeepSea/VectorDraw/VectorImage.h>
#include <DeepSea/VectorDraw/VectorResources.h>
#include <DeepSea/VectorDraw/VectorScratchData.h>
#include <DeepSea/VectorDraw/VectorShaderModule.h>
#include <DeepSea/VectorDraw/VectorShaders.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if DS_HAS_EASY_PROFILER
#include <DeepSea/EasyProfiler/EasyProfiler.h>
#endif

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
	dsShaderVariableGroupDesc* textureIconDataDesc;
	dsMaterialDesc* textureIconMaterialDesc;
	dsShader* textureIconShader;
	dsVectorResources* vectorResources;
	dsVectorImage** vectorImages;

	uint32_t vectorImageCount;
	uint32_t curVectorImage;
	uint32_t fingerCount;
	uint32_t maxFingers;
	bool updateImage;
	bool wireframe;
} TestVectorDraw;

#define TARGET_SIZE 600

static const char* assetsDir = "TestVectorDraw-assets";
static char shaderDir[100];

static dsSystemAllocator renderAllocator;
static dsSystemAllocator applicationAllocator;
static dsSystemAllocator testVectorDrawAllocator;

static const char* vectorImageFiles[] =
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
	"radial-gradient-repeat.dsvi",
	"icon.dsvi",
	"evenodd.dsvi",
	"nonzero.dsvi",
	"holes.dsvi",
	"Ghostscript_Tiger.dsvi",
	"st_ellipse_fan.dsvi",
	"st_complex.dsvi",
	"texture.dsvi",
	"text.dsvi",
	"text-preformatted.dsvi",
	"text-autoformat.dsvi",
	"tspan.dsvi",
	"text-materials.dsvi",
	"text-materials-compare.dsvi",
	"text-icons.dsvi"
};

static void printHelp(const char* programPath)
{
	printf("usage: %s [OPTIONS]\n", dsPath_getFileName(programPath));
	printf("Use left/right arrows or tap on touchscreen to cyle images.\n");
	printf("Press 'w' to toggle wireframe.\n\n");
	printf("options:\n");
	printf("  -h, --help                   print this help message and exit\n");
	printf("--srgb                         use sRGB-correct drawing\n");
	printf("  -r, --renderer <renderer>    explicitly use a renderer; options are:\n");
	for (int i = 0; i < dsRendererType_Default; ++i)
	{
		printf("                                 %s\n",
			dsRenderBootstrap_rendererName((dsRendererType)i));
	}
	printf("  -d, --device <device>        use a graphics device by name\n");
}

static bool validateAllocator(dsAllocator* allocator, const char* name)
{
	if (allocator->size == 0)
		return true;

	DS_LOG_ERROR_F("TestVectorDraw", "Allocator '%s' has %llu bytes allocated with %u allocations.",
		name, (unsigned long long)allocator->size, allocator->currentAllocations);
	return false;
}

static void validateAllocators(void* userData)
{
	DS_UNUSED(userData);
	validateAllocator((dsAllocator*)&renderAllocator, "render");
	validateAllocator((dsAllocator*)&applicationAllocator, "application");
	validateAllocator((dsAllocator*)&testVectorDrawAllocator, "TestVectorDraw");
}

static bool createFramebuffer(TestVectorDraw* testVectorDraw)
{
	uint32_t width = testVectorDraw->window->surface->preRotateWidth;
	uint32_t height = testVectorDraw->window->surface->preRotateHeight;

	dsFramebuffer_destroy(testVectorDraw->framebuffer);

	dsRenderSurface* surface = testVectorDraw->window->surface;
	dsFramebufferSurface surfaces[] =
	{
		{dsGfxSurfaceType_ColorRenderSurface, dsCubeFace_None, 0, 0, surface}
	};
	testVectorDraw->framebuffer = dsFramebuffer_create(testVectorDraw->renderer->resourceManager,
		testVectorDraw->allocator, "Main", surfaces, 1, width, height, 1);

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
	testVectorDraw->updateImage = true;
}

static void prevImage(TestVectorDraw* testVectorDraw)
{
	if (testVectorDraw->curVectorImage == 0)
		testVectorDraw->curVectorImage = testVectorDraw->vectorImageCount - 1;
	else
		--testVectorDraw->curVectorImage;
	testVectorDraw->updateImage = true;
}

static bool processEvent(dsApplication* application, const dsEvent* event, void* userData)
{
	DS_UNUSED(application);

	TestVectorDraw* testVectorDraw = (TestVectorDraw*)userData;
	switch (event->type)
	{
		case dsAppEventType_WindowClosed:
		case dsAppEventType_WindowDestroyed:
			DS_ASSERT(event->window == testVectorDraw->window);
			DS_VERIFY(dsWindow_destroy(testVectorDraw->window));
			testVectorDraw->window = NULL;
			return false;
		case dsAppEventType_WindowChanged:
			DS_ASSERT(event->windowChange.window == testVectorDraw->window);
			if (event->windowChange.flags & dsWindowChangeFlags_SurfaceSize)
			{
				if (!createFramebuffer(testVectorDraw))
					abort();

				// Touch events might be lost, so clear out state to avoid never reaching 0 again.
				testVectorDraw->fingerCount = 0;
				testVectorDraw->maxFingers = 0;
			}
			return true;
		case dsAppEventType_SurfaceInvalidated:
			DS_ASSERT(event->window == testVectorDraw->window);
			if (!createFramebuffer(testVectorDraw))
				abort();
			return true;
		case dsAppEventType_KeyDown:
			if (event->key.window != testVectorDraw->window)
				return true;

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
				case dsKeyCode_V:
					if (testVectorDraw->renderer->vsync == dsVSync_Disabled)
						dsRenderer_setVSync(testVectorDraw->renderer, dsVSync_TripleBuffer);
					else
						dsRenderer_setVSync(testVectorDraw->renderer, dsVSync_Disabled);
					return false;
				case dsKeyCode_ACBack:
				case dsKeyCode_ACExit:
					dsApplication_quit(application, 0);
					return false;
				default:
					return true;
			}
		case dsAppEventType_TouchFingerDown:
			if (event->touch.window != testVectorDraw->window)
				return true;

			++testVectorDraw->fingerCount;
			testVectorDraw->maxFingers = dsMax(
				testVectorDraw->fingerCount, testVectorDraw->maxFingers);
			return false;
		case dsAppEventType_TouchFingerUp:
			if (event->touch.window != testVectorDraw->window || testVectorDraw->fingerCount == 0)
				return true;

			--testVectorDraw->fingerCount;
			if (testVectorDraw->fingerCount == 0)
			{
				switch (testVectorDraw->maxFingers)
				{
					case 1:
						nextImage(testVectorDraw);
						break;
					case 2:
						prevImage(testVectorDraw);
						break;
					case 3:
						testVectorDraw->wireframe = !testVectorDraw->wireframe;
						break;
					default:
						break;
				}
				testVectorDraw->maxFingers = 0;
			}
			return false;
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
	const dsRenderSurface* surface = window->surface;

	if (testVectorDraw->updateImage)
	{
		DS_VERIFY(dsVectorImage_updateText(
			testVectorDraw->vectorImages[testVectorDraw->curVectorImage], commandBuffer));
		testVectorDraw->updateImage = false;
	}

	dsSurfaceClearValue clearValue;
	clearValue.colorValue.floatValue.r = 1.0f;
	clearValue.colorValue.floatValue.g = 1.0f;
	clearValue.colorValue.floatValue.b = 1.0f;
	clearValue.colorValue.floatValue.a = 1.0f;
	DS_VERIFY(dsRenderPass_begin(testVectorDraw->renderPass, commandBuffer,
		testVectorDraw->framebuffer, NULL, NULL, &clearValue, 1, false));

	dsVectorImage* image = testVectorDraw->vectorImages[testVectorDraw->curVectorImage];

	float windowWidth = (float)window->width;
	float windowHeight = (float)window->height;
	float surfaceWidth = (float)surface->width;
	float surfaceHeight = (float)surface->height;
	float pixelScale = surfaceWidth/windowWidth;
	dsAlignedBox2f safeBounds;
	safeBounds.min.x = (float)window->safeArea.min.x*pixelScale;
	safeBounds.min.y = (float)window->safeArea.min.y*pixelScale;
	safeBounds.max.x = (float)window->safeArea.max.x*pixelScale;
	safeBounds.max.y = (float)window->safeArea.max.y*pixelScale;
	dsVector2f safeSize;
	dsAlignedBox2_extents(safeSize, safeBounds);

	// Logical size within the safe area.
	dsVector2f size;
	DS_VERIFY(dsVectorImage_getSize(&size, image));
	float windowAspect = safeSize.x/safeSize.y;
	float imageAspect = size.x/size.y;
	float imageToWindowAspect = windowAspect/imageAspect;
	if (imageToWindowAspect < 1.0f)
		size.y = size.x/windowAspect;
	else
		size.x = size.y*windowAspect;

	// Add padding as necessary to encompass the full window outside of the safe area.
	dsVector2f fullSize;
	fullSize.x = size.x*surfaceWidth/safeSize.x;
	fullSize.y = size.y*surfaceHeight/safeSize.y;

	dsMatrix44f temp, projection, surfaceRotation, safeAreaTransform, modelViewProjection;
	DS_VERIFY(dsRenderer_makeOrtho(
		&projection, renderer, 0.0f, fullSize.x, 0.0f, fullSize.y, 0.0f, 1.0f));
	DS_VERIFY(dsRenderSurface_makeRotationMatrix44(
		&surfaceRotation, testVectorDraw->window->surface->rotation));
	// Shift the lower-left to the safe area.
	dsMatrix44f_makeTranslate(&safeAreaTransform, safeBounds.min.x*fullSize.x/windowWidth,
		(surfaceHeight - safeBounds.max.y)*fullSize.y/windowHeight, 0.0f);
	dsMatrix44f_mul(&temp, &projection, &safeAreaTransform);
	dsMatrix44f_mul(&modelViewProjection, &surfaceRotation, &temp);

	dsVectorShaders* shaders;
	if (testVectorDraw->wireframe)
		shaders = testVectorDraw->wireframeShaders;
	else
		shaders = testVectorDraw->shaders;
	DS_VERIFY(dsVectorImage_draw(image, commandBuffer, shaders, testVectorDraw->material,
		&modelViewProjection, NULL, NULL, NULL));

	DS_VERIFY(dsRenderPass_end(testVectorDraw->renderPass, commandBuffer));
}

static void shutdown(void* userData)
{
	TestVectorDraw* testVectorDraw = (TestVectorDraw*)userData;
	if (testVectorDraw->vectorImages)
	{
		for (uint32_t i = 0; i < testVectorDraw->vectorImageCount; ++i)
			DS_VERIFY(dsVectorImage_destroy(testVectorDraw->vectorImages[i]));
		DS_VERIFY(dsAllocator_free(testVectorDraw->allocator, testVectorDraw->vectorImages));
	}
	DS_VERIFY(dsVectorResources_destroy(testVectorDraw->vectorResources));
	dsMaterial_destroy(testVectorDraw->material);
	DS_VERIFY(dsShader_destroy(testVectorDraw->textureIconShader));
	DS_VERIFY(dsMaterialDesc_destroy(testVectorDraw->textureIconMaterialDesc));
	DS_VERIFY(dsShaderVariableGroupDesc_destroy(testVectorDraw->textureIconDataDesc));
	DS_VERIFY(dsVectorShaders_destroy(testVectorDraw->wireframeShaders));
	DS_VERIFY(dsVectorShaders_destroy(testVectorDraw->shaders));
	DS_VERIFY(dsVectorShaderModule_destroy(testVectorDraw->shaderModule));
	DS_VERIFY(dsRenderPass_destroy(testVectorDraw->renderPass));
	DS_VERIFY(dsFramebuffer_destroy(testVectorDraw->framebuffer));
	DS_VERIFY(dsWindow_destroy(testVectorDraw->window));
	DS_VERIFY(dsAllocator_free(testVectorDraw->allocator, testVectorDraw));
}

static bool setup(dsApplication* application, dsAllocator* allocator, bool srgb)
{
	DS_PROFILE_FUNC_START();

	dsRenderer* renderer = application->renderer;
	dsResourceManager* resourceManager = renderer->resourceManager;
	DS_VERIFY(dsApplication_setFinalizer(application, &validateAllocators, NULL));

	TestVectorDraw* testVectorDraw = DS_ALLOCATE_OBJECT(allocator, TestVectorDraw);
	if (!testVectorDraw)
		DS_PROFILE_FUNC_RETURN(false);

	memset(testVectorDraw, 0, sizeof(TestVectorDraw));
	testVectorDraw->allocator = allocator;
	testVectorDraw->renderer = renderer;
	DS_VERIFY(dsApplication_setUserData(application, testVectorDraw, &shutdown));

	dsEventResponder responder = {&processEvent, testVectorDraw, 0, 0};
	DS_VERIFY(dsApplication_addEventResponder(application, &responder));

	uint32_t targetWindowSize = dsApplication_adjustWindowSize(application, NULL, TARGET_SIZE);
	testVectorDraw->window = dsWindow_create(application, allocator, "Test Vector Draw", NULL,
		NULL, targetWindowSize, targetWindowSize, dsWindowFlags_Resizable,
		dsRenderSurfaceUsage_ClientRotations);
	if (!testVectorDraw->window)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create window: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	DS_VERIFY(dsWindow_setDrawFunction(testVectorDraw->window, &draw, testVectorDraw, NULL));

	if (!createFramebuffer(testVectorDraw))
		DS_PROFILE_FUNC_RETURN(false);

	dsAttachmentInfo attachment = {dsAttachmentUsage_Clear | dsAttachmentUsage_KeepAfter,
		renderer->surfaceColorFormat, DS_SURFACE_ANTIALIAS_SAMPLES};

	dsAttachmentRef colorAttachment = {0, true};
	uint32_t depthStencilAttachment = DS_NO_ATTACHMENT;
	dsRenderSubpassInfo subpass =
	{
		"TestVectorDraw", NULL, &colorAttachment, {depthStencilAttachment, false}, 0, 1
	};
	testVectorDraw->renderPass = dsRenderPass_create(renderer, allocator, &attachment, 1, &subpass,
		1, NULL, DS_DEFAULT_SUBPASS_DEPENDENCIES);
	if (!testVectorDraw->renderPass)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create render pass: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	char path[DS_PATH_MAX];
	const char* shaderFilename = srgb ? "TestVectorDrawSRGB.mslb" : "TestVectorDraw.mslb";
	if (!dsPath_combine(path, sizeof(path), assetsDir, shaderDir) ||
		!dsPath_combine(path, sizeof(path), path, shaderFilename))
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create shader path: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	testVectorDraw->shaderModule = dsVectorShaderModule_loadResource(resourceManager, allocator,
		dsFileResourceType_Embedded, path, NULL, 0);
	if (!testVectorDraw->shaderModule)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't load shader module: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	testVectorDraw->shaders = dsVectorShaders_create(resourceManager, allocator,
		testVectorDraw->shaderModule);
	if (!testVectorDraw->shaders)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create shaders: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	const char* shaderNames[dsVectorShaderType_Count] =
	{
		"dsVectorFillColorWireframe",
		"dsVectorFillLinearGradientWireframe",
		"dsVectorFillRadialGradientWireframe",
		"dsVectorLineWireframe",
		NULL,
		NULL,
		NULL,
		NULL,
		NULL
	};
	testVectorDraw->wireframeShaders = dsVectorShaders_createCustom(resourceManager, allocator,
		testVectorDraw->shaderModule, shaderNames);
	if (!testVectorDraw->shaders)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create shaders: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	testVectorDraw->textureIconDataDesc = dsTextureTextIcons_createShaderVariableGroupDesc(
		resourceManager, allocator);
	if (!testVectorDraw->textureIconDataDesc)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create shader variable group description: %s",
			dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsMaterialElement textureIconMaterialElems[] =
	{
		{dsTextureTextIcons_textureName, dsMaterialType_Texture, 0, NULL,
			dsMaterialBinding_Instance, 0},
		{dsTextureTextIcons_iconDataName, dsMaterialType_VariableGroup, 0,
			testVectorDraw->textureIconDataDesc, dsMaterialBinding_Instance, 0}
	};
	testVectorDraw->textureIconMaterialDesc = dsMaterialDesc_create(resourceManager, allocator,
		textureIconMaterialElems, DS_ARRAY_SIZE(textureIconMaterialElems));
	if (!testVectorDraw->textureIconMaterialDesc)
	{
		DS_LOG_ERROR_F(
			"TestVectorDraw", "Couldn't create material description: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	testVectorDraw->textureIconShader = dsShader_createName(resourceManager, allocator,
		testVectorDraw->shaderModule->shaderModule, "TextureIcon",
		testVectorDraw->textureIconMaterialDesc);
	if (!testVectorDraw->textureIconShader)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create shader: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	testVectorDraw->material = dsMaterial_create(resourceManager, allocator,
		testVectorDraw->shaderModule->materialDesc);
	if (!testVectorDraw->material)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create material: %s", dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	testVectorDraw->vectorResources = NULL;
	testVectorDraw->vectorImageCount = DS_ARRAY_SIZE(vectorImageFiles);
	testVectorDraw->vectorImages = DS_ALLOCATE_OBJECT_ARRAY(allocator, dsVectorImage*,
		testVectorDraw->vectorImageCount);
	if (!testVectorDraw->vectorImages)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't allocate vector image array: %s",
			dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}
	memset(testVectorDraw->vectorImages, 0,
		sizeof(dsVectorImage*)*testVectorDraw->vectorImageCount);

	if (!dsPath_combine(path, sizeof(path), assetsDir, "resources.dsvr"))
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create vector resources path: %s",
			dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsVectorScratchData* scratchData = dsVectorScratchData_create(allocator);
	if (!scratchData)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create vector scratch data: %s",
			dsErrorString(errno));
		DS_PROFILE_FUNC_RETURN(false);
	}

	dsVectorImageInitResources initResources = {resourceManager, NULL, scratchData, NULL,
		testVectorDraw->shaderModule, NULL, NULL, 0, srgb};

	testVectorDraw->vectorResources = dsVectorResources_loadResource(allocator, NULL,
		resourceManager, NULL, dsFileResourceType_Embedded, path, NULL, &initResources, 1.0f,
		testVectorDraw->shaders, testVectorDraw->textureIconShader, NULL);
	if (!testVectorDraw->vectorResources)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't load vector resources: %s",
			dsErrorString(errno));
		dsVectorScratchData_destroy(scratchData);
		DS_PROFILE_FUNC_RETURN(false);
	}

	initResources.resources = &testVectorDraw->vectorResources;
	initResources.resourceCount = 1;

	float targetImageSize = testVectorDraw->window->contentScale*(float)TARGET_SIZE;
	dsVector2f targetImageSize2f = {{targetImageSize, targetImageSize}};
	for (uint32_t i = 0; i < testVectorDraw->vectorImageCount; ++i)
	{
		if (!dsPath_combine(path, sizeof(path), assetsDir, vectorImageFiles[i]))
		{
			DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create vector image path: %s",
				dsErrorString(errno));
			dsVectorScratchData_destroy(scratchData);
			DS_PROFILE_FUNC_RETURN(false);
		}

		uint64_t start = dsTimer_currentTicks();
		DS_PROFILE_DYNAMIC_SCOPE_START(vectorImageFiles[i]);
		testVectorDraw->vectorImages[i] = dsVectorImage_loadResource(allocator, NULL,
			&initResources, dsFileResourceType_Embedded, path, 1.0f, &targetImageSize2f);
		DS_PROFILE_SCOPE_END();
		if (!testVectorDraw->vectorImages[i])
		{
			DS_LOG_ERROR_F("TestVectorDraw", "Couldn't load vector image %s: %s",
				vectorImageFiles[i], dsErrorString(errno));
			dsVectorScratchData_destroy(scratchData);
			DS_PROFILE_FUNC_RETURN(false);
		}
		DS_LOG_INFO_F("TestVectorDraw", "Loaded %s in %g s", vectorImageFiles[i],
			dsTimer_ticksToSeconds(application->timer, dsTimer_currentTicks() - start));
	}

	dsVectorScratchData_destroy(scratchData);
	DS_PROFILE_FUNC_RETURN(true);
}

#if DS_ANDROID && DS_HAS_EASY_PROFILER
static void startEasyProfilerOnPermission(void* userData, const char* permission, bool granted)
{
	DS_UNUSED(userData);
	DS_UNUSED(permission);
	if (granted)
		dsEasyProfiler_startListening(DS_DEFAULT_EASY_PROFILER_PORT);
}
#endif

dsApplication* dsMain(int argc, const char* const* argv)
{
	dsRendererType rendererType = dsRendererType_Default;
	const char* deviceName = NULL;
	bool srgb = false;
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			printHelp(argv[0]);
			return NULL;
		}
		else if (strcmp(argv[i], "--srgb") == 0)
			srgb = true;
		else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--renderer") == 0)
		{
			if (i == argc - 1)
			{
				printf("--renderer option requires an argument\n");
				printHelp(argv[0]);
				return NULL;
			}
			rendererType = dsRenderBootstrap_rendererTypeFromName(argv[++i]);
			if (rendererType == dsRendererType_Default)
			{
				printf("Unknown renderer type: %s\n", argv[i]);
				printHelp(argv[0]);
				return NULL;
			}
		}
		else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--device") == 0)
		{
			if (i == argc - 1)
			{
				printf("--device option requires an argument\n");
				printHelp(argv[0]);
				return NULL;
			}
			deviceName = argv[++i];
		}
		else if (*argv[i])
		{
			printf("Unknown option: %s\n", argv[i]);
			printHelp(argv[0]);
			return NULL;
		}
	}

	DS_LOG_INFO_F("TestVectorDraw", "Render using %s",
		dsRenderBootstrap_rendererName(rendererType));
	DS_LOG_INFO("TestVectorDraw", "Use left and right arrows to cycle through test cases.");
	DS_LOG_INFO("TestVectorDraw", "Press 'W' to toggle wireframe.");
	DS_LOG_INFO("TestVectorDraw", "Press 'V' to toggle vsync.");

	DS_VERIFY(dsSystemAllocator_initialize(&renderAllocator, DS_ALLOCATOR_NO_LIMIT));
	DS_VERIFY(dsSystemAllocator_initialize(&applicationAllocator, DS_ALLOCATOR_NO_LIMIT));
	DS_VERIFY(dsSystemAllocator_initialize(&testVectorDrawAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsRendererOptions rendererOptions;
	dsRenderer_defaultOptions(&rendererOptions, "TestVectorDraw", 0);
	rendererOptions.deviceName = deviceName;
	rendererOptions.depthBits = 0;
	rendererOptions.stencilBits = 0;
	rendererOptions.surfaceSamples = 4;
	if (!dsSDLApplication_prepareRendererOptions(
			&rendererOptions, dsRenderBootstrap_rendererID(rendererType)))
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't setup renderer options.");
		return NULL;
	}

	dsRenderer* renderer = dsRenderBootstrap_createRenderer(rendererType,
		(dsAllocator*)&renderAllocator, &rendererOptions);
	if (!renderer)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create renderer: %s", dsErrorString(errno));
		return NULL;
	}

	dsRenderer_setVSync(renderer, dsVSync_TripleBuffer);
#if DS_DEBUG
	dsRenderer_setExtraDebugging(renderer, true);
#endif

	if (srgb && !dsGfxFormat_textureSupported(renderer->resourceManager,
		dsGfxFormat_decorate(dsGfxFormat_R8G8B8A8, dsGfxFormat_SRGB)))
	{
		DS_LOG_ERROR_F("TestVectorDraw", "sRGB requested but not supported by the current target.");
		return NULL;
	}

	dsShaderVersion shaderVersions[] =
	{
		{DS_VK_RENDERER_ID, DS_ENCODE_VERSION(1, 0, 0)},
		{DS_MTL_RENDERER_ID, DS_ENCODE_VERSION(1, 1, 0)},
		{DS_GL_RENDERER_ID, DS_ENCODE_VERSION(1, 1, 0)},
		{DS_GL_RENDERER_ID, DS_ENCODE_VERSION(1, 5, 0)},
		{DS_GL_RENDERER_ID, DS_ENCODE_VERSION(4, 0, 0)},
		{DS_GLES_RENDERER_ID, DS_ENCODE_VERSION(1, 0, 0)},
		{DS_GLES_RENDERER_ID, DS_ENCODE_VERSION(3, 0, 0)},
		{DS_GLES_RENDERER_ID, DS_ENCODE_VERSION(3, 2, 0)},
	};
	DS_VERIFY(dsRenderer_shaderVersionToString(shaderDir, DS_ARRAY_SIZE(shaderDir), renderer,
		dsRenderer_chooseShaderVersion(renderer, shaderVersions, DS_ARRAY_SIZE(shaderVersions))));

	dsApplication* application = dsSDLApplication_create((dsAllocator*)&applicationAllocator,
		renderer, argc, argv, "DeepSea", "TestVectorDraw", dsSDLApplicationFlags_None);
	if (!application)
	{
		DS_LOG_ERROR_F("TestVectorDraw", "Couldn't create application: %s", dsErrorString(errno));
		return NULL;
	}

#if DS_HAS_EASY_PROFILER
	dsEasyProfiler_start(false);
#if DS_ANDROID
	dsApplication_requestAndroidPermission(application, "android.permission.ACCESS_LOCAL_NETWORK",
		&startEasyProfilerOnPermission, NULL);
#else
	dsEasyProfiler_startListening(DS_DEFAULT_EASY_PROFILER_PORT);
#endif
#endif

	if (!setup(application, (dsAllocator*)&testVectorDrawAllocator, srgb))
	{
		dsApplication_destroy(application);
		return NULL;
	}
	return application;
}

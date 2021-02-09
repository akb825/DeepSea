/*
 * Copyright 2019-2020 Aaron Barany
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
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Render/RenderSurface.h>
#include <DeepSea/RenderBootstrap/RenderBootstrap.h>

#include <DeepSea/Scene/ItemLists/InstanceTransformData.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Scene.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/SceneResources.h>
#include <DeepSea/Scene/View.h>
#include <DeepSea/Scene/ViewTransformData.h>

#include <DeepSea/SceneLighting/InstanceForwardLightData.h>
#include <DeepSea/SceneLighting/SceneLightingLoadContext.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if DS_HAS_EASY_PROFILER
#include <DeepSea/EasyProfiler/EasyProfiler.h>
#endif

typedef struct TestLighting
{
	dsAllocator* allocator;
	dsRenderer* renderer;
	dsWindow* window;
	dsSceneResources* builtinResources;
	dsSceneResources* baseResources;

	dsSceneResources* forwardLightShaders;
	dsSceneResources* forwardLightMaterials;
	dsSceneResources* forwardLightModels;
	dsSceneResources* forwardLightSceneResources;
	dsScene* forwardLightScene;
	dsView* forwardLightView;

	dsSceneResources* deferredLightShaders;
	dsSceneResources* deferredLightMaterials;
	dsSceneResources* deferredLightModels;
	dsSceneResources* deferredLightSceneResources;
	dsScene* deferredLightScene;
	dsView* deferredLightView;

	dsScene* curScene;
	dsView* curView;
	float rotation;
	bool ignoreTime;
	bool stop;
} TestLighting;

static void printHelp(const char* programPath)
{
	printf("usage: %s [OPTIONS]\n", dsPath_getFileName(programPath));
	printf("options:\n");
	printf("  -h, --help                   print this help message and exit\n");
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

	DS_LOG_ERROR_F("TestLighting", "Allocator '%s' has %llu bytes allocated with %u allocations.",
		name, (unsigned long long)allocator->size, allocator->currentAllocations);
	return false;
}

static void updateProjectionMatrix(dsView* view)
{
	dsMatrix44f projection;
	DS_VERIFY(dsRenderer_makePerspective(&projection, dsScene_getRenderer(view->scene),
		(float)dsDegreesToRadians(45.0f), (float)view->width/(float)view->height, 0.1f, 100.0f));
	DS_VERIFY(dsView_setProjectionMatrix(view, &projection));
}

static bool processEvent(dsApplication* application, dsWindow* window, const dsEvent* event,
	void* userData)
{
	TestLighting* testLighting = (TestLighting*)userData;
	dsRenderer* renderer = testLighting->renderer;
	DS_ASSERT(!window || window == testLighting->window);
	switch (event->type)
	{
		case dsAppEventType_WindowClosed:
			DS_VERIFY(dsWindow_destroy(window));
			testLighting->window = NULL;
			return false;
		case dsAppEventType_SurfaceInvalidated:
			DS_VERIFY(dsView_setSurface(testLighting->forwardLightView, "windowColor",
				testLighting->window->surface, dsGfxSurfaceType_ColorRenderSurface));
			// Fall through
		case dsAppEventType_WindowResized:
			DS_VERIFY(dsView_setDimensions(testLighting->forwardLightView,
				testLighting->window->surface->width, testLighting->window->surface->height,
				testLighting->window->surface->rotation));
			DS_VERIFY(dsView_setDimensions(testLighting->deferredLightView,
				testLighting->window->surface->width, testLighting->window->surface->height,
				testLighting->window->surface->rotation));
			updateProjectionMatrix(testLighting->forwardLightView);
			updateProjectionMatrix(testLighting->deferredLightView);
			// Need to update the view again if the surfaces have been set.
			if (event->type == dsAppEventType_SurfaceInvalidated)
			{
				dsView_update(testLighting->forwardLightView);
				dsView_update(testLighting->deferredLightView);
			}
			return true;
		case dsAppEventType_WillEnterForeground:
			testLighting->ignoreTime = true;
			return true;
		case dsAppEventType_KeyDown:
			if (event->key.repeat)
				return false;

			if (event->key.key == dsKeyCode_ACBack)
				dsApplication_quit(application, 0);
			else if (event->key.key == dsKeyCode_Space)
				testLighting->stop = !testLighting->stop;
			else if (event->key.key == dsKeyCode_Enter)
			{
				if (testLighting->curScene == testLighting->forwardLightScene)
				{
					testLighting->curScene = testLighting->deferredLightScene;
					testLighting->curView = testLighting->deferredLightView;
				}
				else
				{
					testLighting->curScene = testLighting->forwardLightScene;
					testLighting->curView = testLighting->forwardLightView;
				}
			}
			else if (event->key.key == dsKeyCode_1)
			{
				uint32_t samples = renderer->defaultSamples;
				if (samples == 1)
					samples = 4;
				else
					samples = 1;
				dsRenderer_setDefaultSamples(renderer, samples);
				DS_LOG_INFO_F("TestLighting", "Togging anti-aliasing: %s",
					samples == 1 ? "off" : "on");
			}
			return false;
		default:
			return true;
	}
}

static void update(dsApplication* application, double lastFrameTime, void* userData)
{
	DS_UNUSED(application);
	DS_UNUSED(lastFrameTime);

	TestLighting* testLighting = (TestLighting*)userData;

	const float speed = 0.4f;
	const float xyDist = 7.0f;
	const float height = 9.0f;
	if (!testLighting->stop && !testLighting->ignoreTime)
	{
		testLighting->rotation = dsWrapf(testLighting->rotation + (float)(lastFrameTime*speed),
			0.0f, (float)(2*M_PI));
	}
	if (testLighting->ignoreTime)
		testLighting->ignoreTime = false;

	dsVector3f eyePos = {{sinf(testLighting->rotation)*xyDist, -cosf(testLighting->rotation)*xyDist,
		height}};
	dsVector3f lookAtPos = {{0.0f, 0.0f, 0.0f}};
	dsVector3f upDir = {{0.0f, 0.0f, 1.0f}};
	dsMatrix44f camera;
	dsMatrix44f_lookAt(&camera, &eyePos, &lookAtPos, &upDir);
	dsView_setCameraMatrix(testLighting->curView, &camera);
	updateProjectionMatrix(testLighting->curView);

	DS_VERIFY(dsScene_update(testLighting->curScene));
	DS_VERIFY(dsView_update(testLighting->curView));
}

static void draw(dsApplication* application, dsWindow* window, void* userData)
{
	DS_UNUSED(application);
	DS_UNUSED(window);
	TestLighting* testLighting = (TestLighting*)userData;
	DS_ASSERT(testLighting->window == window);
	dsRenderer* renderer = testLighting->renderer;
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	DS_VERIFY(dsView_draw(testLighting->curView, commandBuffer, NULL));
}

static bool setupForwardLighitng(TestLighting* testLighting, dsAllocator* allocator,
	dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData)
{
	testLighting->forwardLightShaders = dsSceneResources_loadResource(allocator, NULL,
		loadContext, scratchData, dsFileResourceType_Embedded, "ForwardLightShaders.dssr");
	if (!testLighting->forwardLightShaders)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load forward lighting shaders: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData,
		&testLighting->forwardLightShaders, 1));

	testLighting->forwardLightMaterials = dsSceneResources_loadResource(allocator, NULL,
		loadContext, scratchData, dsFileResourceType_Embedded, "Materials.dssr");
	if (!testLighting->forwardLightMaterials)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load forward lighting materials: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData,
		&testLighting->forwardLightMaterials, 1));

	testLighting->forwardLightModels = dsSceneResources_loadResource(allocator, NULL,
		loadContext, scratchData, dsFileResourceType_Embedded, "Models.dssr");
	if (!testLighting->forwardLightModels)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load forward lighting models: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData,
		&testLighting->forwardLightModels, 1));

	testLighting->forwardLightSceneResources = dsSceneResources_loadResource(allocator, NULL,
		loadContext, scratchData, dsFileResourceType_Embedded, "SceneGraph.dssr");
	if (!testLighting->forwardLightSceneResources)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load forward lighting scene graph: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData,
		&testLighting->forwardLightSceneResources, 1));

	testLighting->forwardLightScene = dsScene_loadResource(allocator, NULL, loadContext,
		scratchData, NULL, NULL, dsFileResourceType_Embedded, "ForwardLightScene.dss");
	if (!testLighting->forwardLightScene)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load forward light scene: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	dsRenderSurface* surface = testLighting->window->surface;
	dsViewSurfaceInfo viewSurface;
	viewSurface.name = "windowColor";
	viewSurface.surfaceType = dsGfxSurfaceType_ColorRenderSurface;
	viewSurface.surface = surface;
	viewSurface.windowFramebuffer = true;

	testLighting->forwardLightView = dsView_loadResource(testLighting->forwardLightScene, allocator,
		NULL, scratchData, &viewSurface, 1, surface->width, surface->height,
		surface->rotation, NULL, NULL, dsFileResourceType_Embedded, "ForwardLightView.dsv");
	if (!testLighting->forwardLightView)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load forward light view: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	DS_VERIFY(dsSceneLoadScratchData_popSceneResources(scratchData, 4));
	return true;
}

static bool setupDeferredLighitng(TestLighting* testLighting, dsAllocator* allocator,
	dsSceneLoadContext* loadContext, dsSceneLoadScratchData* scratchData)
{
	testLighting->deferredLightShaders = dsSceneResources_loadResource(allocator, NULL,
		loadContext, scratchData, dsFileResourceType_Embedded, "DeferredLightShaders.dssr");
	if (!testLighting->deferredLightShaders)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load deferred lighting shaders: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData,
		&testLighting->deferredLightShaders, 1));

	testLighting->deferredLightMaterials = dsSceneResources_loadResource(allocator, NULL,
		loadContext, scratchData, dsFileResourceType_Embedded, "Materials.dssr");
	if (!testLighting->deferredLightMaterials)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load deferred lighting materials: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData,
		&testLighting->deferredLightMaterials, 1));

	testLighting->deferredLightModels = dsSceneResources_loadResource(allocator, NULL,
		loadContext, scratchData, dsFileResourceType_Embedded, "Models.dssr");
	if (!testLighting->deferredLightModels)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load deferred lighting models: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData,
		&testLighting->deferredLightModels, 1));

	testLighting->deferredLightSceneResources = dsSceneResources_loadResource(allocator, NULL,
		loadContext, scratchData, dsFileResourceType_Embedded, "SceneGraph.dssr");
	if (!testLighting->deferredLightSceneResources)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load deferred lighting scene graph: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData,
		&testLighting->deferredLightSceneResources, 1));

	testLighting->deferredLightScene = dsScene_loadResource(allocator, NULL, loadContext,
		scratchData, NULL, NULL, dsFileResourceType_Embedded, "DeferredLightScene.dss");
	if (!testLighting->deferredLightScene)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load deferred light scene: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	dsRenderSurface* surface = testLighting->window->surface;
	dsViewSurfaceInfo viewSurface;
	viewSurface.name = "windowColor";
	viewSurface.surfaceType = dsGfxSurfaceType_ColorRenderSurface;
	viewSurface.surface = surface;
	viewSurface.windowFramebuffer = true;

	testLighting->deferredLightView = dsView_loadResource(testLighting->deferredLightScene, allocator,
		NULL, scratchData, &viewSurface, 1, surface->width, surface->height,
		surface->rotation, NULL, NULL, dsFileResourceType_Embedded, "DeferredLightView.dsv");
	if (!testLighting->deferredLightView)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load deferred light view: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	DS_VERIFY(dsSceneLoadScratchData_popSceneResources(scratchData, 4));
	return true;
}

static bool setup(TestLighting* testLighting, dsApplication* application, dsAllocator* allocator)
{
	dsRenderer* renderer = application->renderer;
	dsResourceManager* resourceManager = renderer->resourceManager;
	testLighting->allocator = allocator;
	testLighting->renderer = renderer;

	dsEventResponder responder = {&processEvent, testLighting, 0, 0};
	DS_VERIFY(dsApplication_addEventResponder(application, &responder));
	DS_VERIFY(dsApplication_setUpdateFunction(application, &update, testLighting));

	uint32_t width = dsApplication_adjustWindowSize(application, 0, 800);
	uint32_t height = dsApplication_adjustWindowSize(application, 0, 600);
	testLighting->window = dsWindow_create(application, allocator, "Test Scene", NULL,
		NULL, width, height, dsWindowFlags_Resizeable | dsWindowFlags_DelaySurfaceCreate,
		dsRenderSurfaceUsage_ClientRotations);
	if (!testLighting->window)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't create window: %s", dsErrorString(errno));
		return false;
	}

	if (DS_ANDROID || DS_IOS)
		dsWindow_setStyle(testLighting->window, dsWindowStyle_FullScreen);

	if (!dsWindow_createSurface(testLighting->window))
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't create window surface: %s", dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsWindow_setDrawFunction(testLighting->window, &draw, testLighting));

	dsSceneLoadContext* loadContext = dsSceneLoadContext_create(allocator, renderer);
	if (!loadContext)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't create load context: %s", dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsSceneLightingLoadConext_registerTypes(loadContext));

	dsSceneLoadScratchData* scratchData = dsSceneLoadScratchData_create(allocator,
		renderer->mainCommandBuffer);
	if (!scratchData)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't create load scratch data: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		return false;
	}

	testLighting->builtinResources = dsSceneResources_create(allocator, 3);
	if (!testLighting->builtinResources)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't create scene resources: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	dsShaderVariableGroupDesc* groupDesc =
		dsInstanceTransformData_createShaderVariableGroupDesc(resourceManager, allocator);
	if (!groupDesc)
	{
		DS_LOG_ERROR_F("TestLighting",
			"Couldn't create instance transform shader variable desc: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneResources_addResource(testLighting->builtinResources,
		"instanceTransformDesc", dsSceneResourceType_ShaderVariableGroupDesc, groupDesc, true));

	groupDesc = dsViewTransformData_createShaderVariableGroupDesc(resourceManager, allocator);
	if (!groupDesc)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't create view transform shader variable desc: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneResources_addResource(testLighting->builtinResources,
		"viewTransformDesc", dsSceneResourceType_ShaderVariableGroupDesc, groupDesc, true));

	groupDesc = dsInstanceForwardLightData_createShaderVariableGroupDesc(resourceManager, allocator,
		DS_DEFAULT_FORWARD_LIGHT_COUNT);
	if (!groupDesc)
	{
		DS_LOG_ERROR_F("TestLighting",
			"Couldn't create instance forward light shader variable desc: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneResources_addResource(testLighting->builtinResources,
		"instanceForwardLightDesc", dsSceneResourceType_ShaderVariableGroupDesc, groupDesc, true));

	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData,
		&testLighting->builtinResources, 1));

	testLighting->baseResources = dsSceneResources_loadResource(allocator, NULL, loadContext,
		scratchData, dsFileResourceType_Embedded, "BaseResources.dssr");
	if (!testLighting->baseResources)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load base scene resources: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData,
		&testLighting->baseResources, 1));

	if (!setupForwardLighitng(testLighting, allocator, loadContext, scratchData) ||
		!setupDeferredLighitng(testLighting, allocator, loadContext, scratchData))
	{
		return false;
	}

	dsSceneLoadContext_destroy(loadContext);
	dsSceneLoadScratchData_destroy(scratchData);

	testLighting->curScene = testLighting->forwardLightScene;
	testLighting->curView = testLighting->forwardLightView;

	return true;
}

static void shutdown(TestLighting* testLighting)
{
	DS_VERIFY(dsView_destroy(testLighting->forwardLightView));
	dsScene_destroy(testLighting->forwardLightScene);
	dsSceneResources_freeRef(testLighting->forwardLightSceneResources);
	dsSceneResources_freeRef(testLighting->forwardLightModels);
	dsSceneResources_freeRef(testLighting->forwardLightShaders);
	dsSceneResources_freeRef(testLighting->forwardLightMaterials);

	DS_VERIFY(dsView_destroy(testLighting->deferredLightView));
	dsScene_destroy(testLighting->deferredLightScene);
	dsSceneResources_freeRef(testLighting->deferredLightSceneResources);
	dsSceneResources_freeRef(testLighting->deferredLightModels);
	dsSceneResources_freeRef(testLighting->deferredLightShaders);
	dsSceneResources_freeRef(testLighting->deferredLightMaterials);

	dsSceneResources_freeRef(testLighting->baseResources);
	dsSceneResources_freeRef(testLighting->builtinResources);
	DS_VERIFY(dsWindow_destroy(testLighting->window));
}

int dsMain(int argc, const char** argv)
{
#if DS_HAS_EASY_PROFILER
	dsEasyProfiler_start(false);
	dsEasyProfiler_startListening(DS_DEFAULT_EASY_PROFILER_PORT);
#endif

	dsRendererType rendererType = dsRendererType_Default;
	const char* deviceName = NULL;
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			printHelp(argv[0]);
			return 0;
		}
		else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--renderer") == 0)
		{
			if (i == argc - 1)
			{
				printf("--renderer option requires an argument\n");
				printHelp(argv[0]);
				return 1;
			}
			rendererType = dsRenderBootstrap_rendererTypeFromName(argv[++i]);
			if (rendererType == dsRendererType_Default)
			{
				printf("Unknown renderer type: %s\n", argv[i]);
				printHelp(argv[0]);
				return 1;
			}
		}
		else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--device") == 0)
		{
			if (i == argc - 1)
			{
				printf("--device option requires an argument\n");
				printHelp(argv[0]);
				return 1;
			}
			deviceName = argv[++i];
		}
		else if (*argv[i])
		{
			printf("Unknown option: %s\n", argv[i]);
			printHelp(argv[0]);
			return 1;
		}
	}

	DS_LOG_INFO_F("TestLighting", "Render using %s", dsRenderBootstrap_rendererName(rendererType));
	DS_LOG_INFO("TestLighting", "Press space to pause/unpause.");
	DS_LOG_INFO("TestLighting", "Press enter to toggle forward and deferred lighting.");
	DS_LOG_INFO("TestLighting", "Press '1' to toggle anti-aliasing for forward lighting.");

	dsSystemAllocator renderAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&renderAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator applicationAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&applicationAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator testLightingAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&testLightingAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsRendererOptions rendererOptions;
	dsRenderer_defaultOptions(&rendererOptions, "TestLighting", 0);
	rendererOptions.depthBits = 0;
	rendererOptions.stencilBits = 0;
	rendererOptions.defaultSamples = 4;
	rendererOptions.deviceName = deviceName;
	dsRenderer* renderer = dsRenderBootstrap_createRenderer(rendererType,
		(dsAllocator*)&renderAllocator, &rendererOptions);
	if (!renderer)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't create renderer: %s", dsErrorString(errno));
		return 2;
	}

	dsRenderer_setVsync(renderer, true);
	dsRenderer_setDefaultAnisotropy(renderer, dsMin(4.0f, renderer->maxAnisotropy));
#if DS_DEBUG
	dsRenderer_setExtraDebugging(renderer, true);
#endif

	dsApplication* application = dsSDLApplication_create((dsAllocator*)&applicationAllocator,
		renderer, argc, argv, "DeepSea", "TestLighting");
	if (!application)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't create application: %s", dsErrorString(errno));
		dsRenderer_destroy(renderer);
		return 2;
	}

	char assetsPath[DS_PATH_MAX];
	DS_VERIFY(dsPath_combine(assetsPath, sizeof(assetsPath), dsResourceStream_getEmbeddedDir(),
		"TestLighting-assets"));
	dsResourceStream_setEmbeddedDir(assetsPath);

	TestLighting testLighting;
	memset(&testLighting, 0, sizeof(testLighting));
	if (!setup(&testLighting, application, (dsAllocator*)&testLightingAllocator))
	{
		shutdown(&testLighting);
		return 3;
	}

	int exitCode = dsApplication_run(application);

	shutdown(&testLighting);
	dsSDLApplication_destroy(application);
	dsRenderer_destroy(renderer);

	if (!validateAllocator((dsAllocator*)&renderAllocator, "render"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&applicationAllocator, "application"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&testLightingAllocator, "TestLighting"))
		exitCode = 4;

	return exitCode;
}

/*
 * Copyright 2019-2021 Aaron Barany
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

#include "LightData.h"
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

#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneTransformNode.h>
#include <DeepSea/Scene/Scene.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/SceneResources.h>
#include <DeepSea/Scene/SceneThreadManager.h>
#include <DeepSea/Scene/View.h>
#include <DeepSea/Scene/ViewTransformData.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if DS_HAS_EASY_PROFILER
#include <DeepSea/EasyProfiler/EasyProfiler.h>
#endif

typedef struct TestScene
{
	dsAllocator* allocator;
	dsRenderer* renderer;
	dsWindow* window;
	dsSceneResources* resources;
	dsSceneTransformNode* primaryTransform;
	dsSceneTransformNode* secondaryTransform;
	dsScene* scene;
	dsView* view;
	dsSceneThreadManager* threadManager;

	uint64_t invalidatedFrame;
	bool secondarySceneSet;
	bool multithreadedRendering;
	float rotation;
} TestScene;

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

	DS_LOG_ERROR_F("TestScene", "Allocator '%s' has %llu bytes allocated with %u allocations.",
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
	TestScene* testScene = (TestScene*)userData;
	dsRenderer* renderer = application->renderer;
	DS_ASSERT(!window || window == testScene->window);
	switch (event->type)
	{
		case dsAppEventType_WindowClosed:
			DS_VERIFY(dsWindow_destroy(window));
			testScene->window = NULL;
			return false;
		case dsAppEventType_SurfaceInvalidated:
			DS_VERIFY(dsView_setSurface(testScene->view, "windowColor", testScene->window->surface,
				dsGfxSurfaceType_ColorRenderSurface));
			DS_VERIFY(dsView_setSurface(testScene->view, "windowDepth", testScene->window->surface,
				dsGfxSurfaceType_DepthRenderSurface));
			testScene->invalidatedFrame = renderer->frameNumber;
			// Fall through
		case dsAppEventType_WindowResized:
			DS_VERIFY(dsView_setDimensions(testScene->view, testScene->window->surface->width,
				testScene->window->surface->height, testScene->window->surface->rotation));
			updateProjectionMatrix(testScene->view);
			// Need to update the view again if the surfaces have been set.
			if (event->type == dsAppEventType_SurfaceInvalidated)
				dsView_update(testScene->view);
			return true;
		case dsAppEventType_KeyDown:
			if (event->key.repeat)
				return false;

			if (event->key.key == dsKeyCode_ACBack)
				dsApplication_quit(application, 0);
			else if (event->key.key == dsKeyCode_1)
			{
				// The key down will be re-sent when re-creating the window.
				if (testScene->invalidatedFrame + 2 > renderer->frameNumber)
					return false;

				uint32_t samples = renderer->surfaceSamples;
				if (samples == 1)
					samples = 4;
				else
					samples = 1;
				dsRenderer_setSamples(renderer, samples);
				DS_LOG_INFO_F("TestScene", "Togging anti-aliasing: %s",
					samples == 1 ? "off" : "on");
			}
			else if (event->key.key == dsKeyCode_2)
			{
				if (testScene->secondarySceneSet)
				{
					DS_VERIFY(dsSceneNode_removeChildNode((dsSceneNode*)testScene->primaryTransform,
						(dsSceneNode*)testScene->secondaryTransform));
					testScene->secondarySceneSet = false;
				}
				else
				{
					DS_VERIFY(dsSceneNode_addChild((dsSceneNode*)testScene->primaryTransform,
						(dsSceneNode*)testScene->secondaryTransform));
					testScene->secondarySceneSet = true;
				}
				DS_LOG_INFO_F("TestScene", "Togging secondary scene: %s",
					testScene->secondarySceneSet ? "on" : "off");
			}
			else if (event->key.key == dsKeyCode_3)
			{
				testScene->multithreadedRendering = !testScene->multithreadedRendering;
				DS_LOG_INFO_F("TestScene", "Togging multi-threaded rendering: %s",
					testScene->multithreadedRendering ? "on" : "off");
			}
			return false;
		default:
			return true;
	}
}

static void update(dsApplication* application, double lastFrameTime, void* userData)
{
	DS_UNUSED(application);

	TestScene* testScene = (TestScene*)userData;

	// radians/s
	const double rate = M_PI_2;
	testScene->rotation += (float)(lastFrameTime*rate);
	while (testScene->rotation > 2*M_PI)
		testScene->rotation = (float)(testScene->rotation - 2*M_PI);

	dsMatrix44f transform;
	dsMatrix44f_makeRotate(&transform, 0, testScene->rotation, 0);
	DS_VERIFY(dsSceneTransformNode_setTransform(testScene->primaryTransform, &transform));

	dsMatrix44f_makeRotate(&transform, 0, -2.0f*testScene->rotation, 0);
	transform.columns[3].x = -3.0f;
	transform.columns[3].y = 2.0f;
	transform.columns[3].z = 5.0f;
	DS_VERIFY(dsSceneTransformNode_setTransform(testScene->secondaryTransform, &transform));

	DS_VERIFY(dsScene_update(testScene->scene));
	DS_VERIFY(dsView_update(testScene->view));
}

static void draw(dsApplication* application, dsWindow* window, void* userData)
{
	DS_UNUSED(application);
	DS_UNUSED(window);
	TestScene* testScene = (TestScene*)userData;
	DS_ASSERT(testScene->window == window);
	dsRenderer* renderer = testScene->renderer;
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	DS_VERIFY(dsView_draw(testScene->view, commandBuffer,
		testScene->multithreadedRendering ? testScene->threadManager : NULL));
}

static bool setup(TestScene* testScene, dsApplication* application, dsAllocator* allocator)
{
	dsRenderer* renderer = application->renderer;
	testScene->allocator = allocator;
	testScene->renderer = renderer;

	dsEventResponder responder = {&processEvent, testScene, 0, 0};
	DS_VERIFY(dsApplication_addEventResponder(application, &responder));
	DS_VERIFY(dsApplication_setUpdateFunction(application, &update, testScene));

	uint32_t width = dsApplication_adjustWindowSize(application, 0, 800);
	uint32_t height = dsApplication_adjustWindowSize(application, 0, 600);
	testScene->window = dsWindow_create(application, allocator, "Test Scene", NULL,
		NULL, width, height, dsWindowFlags_Resizeable | dsWindowFlags_DelaySurfaceCreate,
		dsRenderSurfaceUsage_ClientRotations);
	if (!testScene->window)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create window: %s", dsErrorString(errno));
		return false;
	}

	if (DS_ANDROID || DS_IOS)
		dsWindow_setStyle(testScene->window, dsWindowStyle_FullScreen);

	if (!dsWindow_createSurface(testScene->window))
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create window surface: %s", dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsWindow_setDrawFunction(testScene->window, &draw, testScene));

	dsSceneLoadContext* loadContext = dsSceneLoadContext_create(allocator, renderer);
	if (!loadContext)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create load context: %s", dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsSceneLoadContext_registerGlobalDataType(loadContext, "LightData", &dsLightData_load,
		NULL, NULL));

	dsSceneLoadScratchData* scratchData = dsSceneLoadScratchData_create(allocator,
		renderer->mainCommandBuffer);
	if (!scratchData)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create load scratch data: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		return false;
	}

	testScene->resources = dsSceneResources_loadResource(allocator, NULL, loadContext, scratchData,
		dsFileResourceType_Embedded, "resources.dssr");
	if (!testScene->resources)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't load scene resources: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData, &testScene->resources, 1));

	dsSceneResourceType resourceType;
	const dsSceneNodeType* transformNodeType = dsSceneTransformNode_type();
	if (!dsSceneResources_findResource(&resourceType, (void**)&testScene->primaryTransform,
			testScene->resources, "primaryTransform") ||
		resourceType != dsSceneResourceType_SceneNode ||
		!dsSceneNode_isOfType((dsSceneNode*)testScene->primaryTransform, transformNodeType))
	{
		DS_LOG_ERROR("TestScene", "Couldn't find transform node 'primaryTransform'.");
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	if (!dsSceneResources_findResource(&resourceType, (void**)&testScene->secondaryTransform,
			testScene->resources, "secondaryTransform") ||
		resourceType != dsSceneResourceType_SceneNode ||
		!dsSceneNode_isOfType((dsSceneNode*)testScene->secondaryTransform, transformNodeType))
	{
		DS_LOG_ERROR("TestScene", "Couldn't find transform node 'secondaryTransform'.");
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	testScene->scene = dsScene_loadResource(allocator, NULL, loadContext, scratchData, NULL, NULL,
		dsFileResourceType_Embedded, "scene.dss");
	if (!testScene->scene)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't load scene: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	dsRenderSurface* surface = testScene->window->surface;
	dsViewSurfaceInfo viewSurfaces[2];
	viewSurfaces[0].name = "windowColor";
	viewSurfaces[0].surfaceType = dsGfxSurfaceType_ColorRenderSurface;
	viewSurfaces[0].surface = surface;
	viewSurfaces[0].windowFramebuffer = true;
	viewSurfaces[1].name = "windowDepth";
	viewSurfaces[1].surfaceType = dsGfxSurfaceType_DepthRenderSurface;
	viewSurfaces[1].surface = surface;
	viewSurfaces[1].windowFramebuffer = true;

	testScene->view = dsView_loadResource(testScene->scene, allocator, NULL, scratchData,
		viewSurfaces, DS_ARRAY_SIZE(viewSurfaces), surface->width, surface->height,
		surface->rotation, NULL, NULL, dsFileResourceType_Embedded, "view.dsv");
	dsSceneLoadContext_destroy(loadContext);
	dsSceneLoadScratchData_destroy(scratchData);
	if (!testScene->view)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't load view: %s", dsErrorString(errno));
		return false;
	}

	dsVector3f eyePos = {{0.0f, 20.0f, 20.0f}};
	dsVector3f lookAtPos = {{0.0f, 0.0f, 0.0f}};
	dsVector3f upDir = {{0.0f, 1.0f, 0.0f}};
	dsMatrix44f camera;
	dsMatrix44f_lookAt(&camera, &eyePos, &lookAtPos, &upDir);
	dsView_setCameraMatrix(testScene->view, &camera);
	updateProjectionMatrix(testScene->view);
	testScene->secondarySceneSet = true;

	testScene->threadManager = dsSceneThreadManager_create(allocator, renderer, 1);
	if (!testScene->threadManager)
		return false;

	return true;
}

static void shutdown(TestScene* testScene)
{
	DS_VERIFY(dsView_destroy(testScene->view));
	dsScene_destroy(testScene->scene);
	dsSceneThreadManager_destroy(testScene->threadManager);
	dsSceneResources_freeRef(testScene->resources);
	DS_VERIFY(dsWindow_destroy(testScene->window));
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

	DS_LOG_INFO_F("TestScene", "Render using %s", dsRenderBootstrap_rendererName(rendererType));
	DS_LOG_INFO("TestScene", "Press '1' to toggle anti-aliasing.");
	DS_LOG_INFO("TestScene", "Press '2' to toggle sub-scene.");
	DS_LOG_INFO("TestScene", "Press '3' to toggle multi-threaded rendering.");

	dsSystemAllocator renderAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&renderAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator applicationAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&applicationAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator testSceneAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&testSceneAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsRendererOptions rendererOptions;
	dsRenderer_defaultOptions(&rendererOptions, "TestScene", 0);
	rendererOptions.surfaceSamples = 4;
	rendererOptions.maxResourceThreads = 1;
	rendererOptions.deviceName = deviceName;
	dsRenderer* renderer = dsRenderBootstrap_createRenderer(rendererType,
		(dsAllocator*)&renderAllocator, &rendererOptions);
	if (!renderer)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create renderer: %s", dsErrorString(errno));
		return 2;
	}

	dsRenderer_setVsync(renderer, true);
	dsRenderer_setDefaultAnisotropy(renderer, renderer->maxAnisotropy);
#if DS_DEBUG
	dsRenderer_setExtraDebugging(renderer, true);
#endif

	dsApplication* application = dsSDLApplication_create((dsAllocator*)&applicationAllocator,
		renderer, argc, argv, "DeepSea", "TestScene");
	if (!application)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create application: %s", dsErrorString(errno));
		dsRenderer_destroy(renderer);
		return 2;
	}

	char assetsPath[DS_PATH_MAX];
	DS_VERIFY(dsPath_combine(assetsPath, sizeof(assetsPath), dsResourceStream_getEmbeddedDir(),
		"TestScene-assets"));
	dsResourceStream_setEmbeddedDir(assetsPath);

	TestScene testScene;
	memset(&testScene, 0, sizeof(testScene));
	if (!setup(&testScene, application, (dsAllocator*)&testSceneAllocator))
	{
		shutdown(&testScene);
		return 3;
	}

	int exitCode = dsApplication_run(application);

	shutdown(&testScene);
	dsSDLApplication_destroy(application);
	dsRenderer_destroy(renderer);

	if (!validateAllocator((dsAllocator*)&renderAllocator, "render"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&applicationAllocator, "application"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&testSceneAllocator, "TestScene"))
		exitCode = 4;

	return exitCode;
}

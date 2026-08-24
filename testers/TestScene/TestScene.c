/*
 * Copyright 2019-2026 Aaron Barany
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
#include "TestSceneUpdate.h"

#include <DeepSea/Application/Application.h>
#include <DeepSea/Application/Window.h>
#include <DeepSea/ApplicationSDL/SDLApplication.h>

#include <DeepSea/Core/Memory/Allocator.h>
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Streams/ResourceStream.h>
#include <DeepSea/Core/Thread/ThreadPool.h>
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
#include <DeepSea/Scene/Scene.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/SceneResources.h>
#include <DeepSea/Scene/SceneThreadManager.h>
#include <DeepSea/Scene/SceneTick.h>
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
	dsSceneNode* primaryTransform;
	dsSceneNode* secondaryTransform;
	dsScene* scene;
	dsSceneItemList* update;
	dsView* view;
	dsThreadPool* threadPool;
	dsSceneThreadManager* threadManager;

	dsSceneTick tick;
	uint64_t invalidatedFrame;
	bool secondarySceneSet;
	bool multithreadedRendering;
	float rotation;
} TestScene;

static dsSystemAllocator renderAllocator;
static dsSystemAllocator applicationAllocator;
static dsSystemAllocator testSceneAllocator;

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

static void validateAllocators(void* userData)
{
	DS_UNUSED(userData);
	validateAllocator((dsAllocator*)&renderAllocator, "render");
	validateAllocator((dsAllocator*)&applicationAllocator, "application");
	validateAllocator((dsAllocator*)&testSceneAllocator, "TestScene");
}

static bool processEvent(dsApplication* application, const dsEvent* event, void* userData)
{
	TestScene* testScene = (TestScene*)userData;
	dsRenderer* renderer = application->renderer;
	switch (event->type)
	{
		case dsAppEventType_WindowClosed:
		case dsAppEventType_WindowDestroyed:
			DS_ASSERT(event->window == testScene->window);
			DS_VERIFY(dsWindow_destroy(testScene->window));
			testScene->window = NULL;
			return false;
		case dsAppEventType_WindowChanged:
			DS_ASSERT(event->windowChange.window == testScene->window);
			if (event->windowChange.flags & dsWindowChangeFlags_SurfaceSize)
			{
				DS_VERIFY(dsView_setDimensions(testScene->view, testScene->window->surface->width,
					testScene->window->surface->height, testScene->window->surface->rotation));
			}
			return true;
		case dsAppEventType_SurfaceInvalidated:
			DS_VERIFY(dsView_setSurface(testScene->view, "windowColor", testScene->window->surface,
				dsGfxSurfaceType_ColorRenderSurface));
			DS_VERIFY(dsView_setSurface(testScene->view, "windowDepth", testScene->window->surface,
				dsGfxSurfaceType_DepthRenderSurface));
			testScene->invalidatedFrame = renderer->frameNumber;
			DS_VERIFY(dsView_setDimensions(testScene->view, testScene->window->surface->width,
				testScene->window->surface->height, testScene->window->surface->rotation));
			// Need to update the view again if the surfaces have been set.
			dsView_update(testScene->view);
			return true;
		case dsAppEventType_KeyDown:
			if (event->key.window != testScene->window)
				return true;

			switch (event->key.key)
			{
				case dsKeyCode_Space:
					dsTestSceneUpdate_togglePaused(testScene->update);
					return false;
				case dsKeyCode_1:
				{
					// The key down will be re-sent when re-creating the window.
					if (testScene->invalidatedFrame + 2 > renderer->frameNumber)
						return true;

					uint32_t samples = renderer->surfaceSamples;
					if (samples == 1)
						samples = 4;
					else
						samples = 1;
					dsRenderer_setSamples(renderer, samples);
					DS_LOG_INFO_F(
						"TestScene", "Togging anti-aliasing: %s", samples == 1 ? "off" : "on");
					return false;
				}
				case dsKeyCode_2:
					if (testScene->secondarySceneSet)
					{
						DS_VERIFY(dsSceneNode_removeChildNode(
							testScene->primaryTransform, testScene->secondaryTransform));
						testScene->secondarySceneSet = false;
					}
					else
					{
						DS_VERIFY(dsSceneNode_addChild(
							testScene->primaryTransform, testScene->secondaryTransform));
						testScene->secondarySceneSet = true;
					}
					DS_LOG_INFO_F("TestScene", "Togging secondary scene: %s",
						testScene->secondarySceneSet ? "on" : "off");
					return false;
				case dsKeyCode_3:
					testScene->multithreadedRendering = !testScene->multithreadedRendering;
					DS_LOG_INFO_F("TestScene", "Togging multi-threaded rendering: %s",
						testScene->multithreadedRendering ? "on" : "off");
					return false;
				case dsKeyCode_V:
					if (testScene->renderer->vsync == dsVSync_Disabled)
						dsRenderer_setVSync(testScene->renderer, dsVSync_TripleBuffer);
					else
						dsRenderer_setVSync(testScene->renderer, dsVSync_Disabled);
					return false;
				case dsKeyCode_ACBack:
				case dsKeyCode_ACExit:
					dsApplication_quit(application, 0);
					return false;
				default:
					return true;
			}
		default:
			return true;
	}
}

static void update(
	dsApplication* application, uint64_t absoluteTime, uint64_t lastFrameTime, void* userData)
{
	DS_UNUSED(application);

	TestScene* testScene = (TestScene*)userData;
	DS_VERIFY(dsSceneTick_update(&testScene->tick, absoluteTime, lastFrameTime));
	DS_VERIFY(dsScene_update(testScene->scene, &testScene->tick));
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

static void shutdown(void* userData)
{
	TestScene* testScene = (TestScene*)userData;
	DS_VERIFY(dsView_destroy(testScene->view));
	dsScene_destroy(testScene->scene);
	dsSceneThreadManager_destroy(testScene->threadManager);
	dsThreadPool_destroy(testScene->threadPool);
	dsSceneResources_freeRef(testScene->resources);
	DS_VERIFY(dsWindow_destroy(testScene->window));
	DS_VERIFY(dsAllocator_free(testScene->allocator, testScene));
}

static bool setup(dsApplication* application, dsAllocator* allocator, float updateFps)
{
	dsRenderer* renderer = application->renderer;
	DS_VERIFY(dsApplication_setFinalizer(application, &validateAllocators, NULL));

	TestScene* testScene = DS_ALLOCATE_OBJECT(allocator, TestScene);
	if (!testScene)
		return false;

	memset(testScene, 0, sizeof(TestScene));
	testScene->allocator = allocator;
	testScene->renderer = renderer;
	DS_VERIFY(dsApplication_setUserData(application, testScene, &shutdown));

	dsEventResponder responder = {&processEvent, testScene, 0, 0};
	DS_VERIFY(dsApplication_addEventResponder(application, &responder));
	DS_VERIFY(dsApplication_setUpdateFunction(application, &update, testScene, NULL));

	uint32_t width = dsApplication_adjustWindowSize(application, NULL, 800);
	uint32_t height = dsApplication_adjustWindowSize(application, NULL, 600);
	testScene->window = dsWindow_create(application, allocator, "Test Scene", NULL, NULL, width,
		height, dsWindowFlags_Resizable, dsRenderSurfaceUsage_ClientRotations);
	if (!testScene->window)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create window: %s", dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsWindow_setDrawFunction(testScene->window, &draw, testScene, NULL));

	dsSceneLoadContext* loadContext = dsSceneLoadContext_create(allocator, renderer);
	if (!loadContext)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create load context: %s", dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsSceneLoadContext_registerItemListType(
		loadContext, "LightData", &dsLightData_load, NULL, NULL));
	DS_VERIFY(dsSceneLoadContext_registerItemListType(
		loadContext, "TestSceneUpdate", &dsTestSceneUpdate_load, NULL, NULL));

	dsSceneLoadScratchData* scratchData = dsSceneLoadScratchData_create(
		allocator, renderer->mainCommandBuffer);
	if (!scratchData)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create load scratch data: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		return false;
	}

	testScene->resources = dsSceneResources_loadResource(
		allocator, NULL, loadContext, scratchData, dsFileResourceType_Embedded, "resources.dssr");
	if (!testScene->resources)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't load scene resources: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData, &testScene->resources, 1));

	dsSceneResourceType resourceType;
	if (!dsSceneResources_findResource(&resourceType, (void**)&testScene->primaryTransform,
			testScene->resources, "primaryTransform") ||
		resourceType != dsSceneResourceType_SceneNode)
	{
		DS_LOG_ERROR("TestScene", "Couldn't find node 'primaryTransform'.");
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	if (!dsSceneResources_findResource(&resourceType, (void**)&testScene->secondaryTransform,
			testScene->resources, "secondaryTransform") ||
		resourceType != dsSceneResourceType_SceneNode)
	{
		DS_LOG_ERROR("TestScene", "Couldn't find node 'secondaryTransform'.");
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	testScene->scene = dsScene_loadResource(allocator, NULL, loadContext, scratchData, NULL, NULL,
		NULL, dsFileResourceType_Embedded, "scene.dss");
	if (!testScene->scene)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't load scene: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	testScene->update = dsScene_findItemList(testScene->scene, "update");
	if (!testScene->update)
	{
		DS_LOG_ERROR("TestScene", "Couldn't find scene item list 'update'.");
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

	testScene->view = dsView_loadResource(allocator, "window", testScene->scene, NULL, scratchData,
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
	dsView_setPerspectiveProjection(testScene->view, dsDegreesToRadiansf(45.0f), 0.1f, 100.0f);
	testScene->secondarySceneSet = true;

	testScene->threadPool = dsResourceManager_createThreadPool(
		allocator, renderer->resourceManager, dsThreadPool_defaultThreadCount(), 0);
	if (!testScene->threadPool)
		return false;

	testScene->threadManager = dsSceneThreadManager_create(
		allocator, renderer, testScene->threadPool);
	if (!testScene->threadManager)
		return false;

	float updatePeriod = updateFps > 0.0f ? 1.0f/updateFps : 0.0f;
	DS_VERIFY(dsSceneTick_initialize(&testScene->tick, updatePeriod, 1.0f));
	return true;
}

#if DS_ANDROID
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
	float updateFps = 0.0f;
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			printHelp(argv[0]);
			return NULL;
		}
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
		else if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--update-fps") == 0)
		{
			if (i == argc - 1)
			{
				printf("--update-fps option requires an argument\n");
				printHelp(argv[0]);
				return NULL;
			}

			char* endPtr;
			updateFps = strtof(argv[++i], &endPtr);
			if (updateFps < 0.0f || *endPtr)
			{
				printf("--update-fps option must be a float >= 0\n");
				printHelp(argv[0]);
				return NULL;
			}
		}
		else if (*argv[i])
		{
			printf("Unknown option: %s\n", argv[i]);
			printHelp(argv[0]);
			return NULL;
		}
	}

	DS_LOG_INFO_F("TestScene", "Render using %s", dsRenderBootstrap_rendererName(rendererType));
	DS_LOG_INFO("TestScene", "Press space to pause/unpause.");
	DS_LOG_INFO("TestScene", "Press '1' to toggle anti-aliasing.");
	DS_LOG_INFO("TestScene", "Press '2' to toggle sub-scene.");
	DS_LOG_INFO("TestScene", "Press '3' to toggle multi-threaded rendering.");
	DS_LOG_INFO("TestScene", "Press 'V' to toggle vsync.");

	DS_VERIFY(dsSystemAllocator_initialize(&renderAllocator, DS_ALLOCATOR_NO_LIMIT));
	DS_VERIFY(dsSystemAllocator_initialize(&applicationAllocator, DS_ALLOCATOR_NO_LIMIT));
	DS_VERIFY(dsSystemAllocator_initialize(&testSceneAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsRendererOptions rendererOptions;
	dsRenderer_defaultOptions(&rendererOptions, "TestScene", 0);
	rendererOptions.surfaceSamples = 4;
	rendererOptions.maxResourceThreads = dsThreadPool_defaultThreadCount();
	rendererOptions.deviceName = deviceName;
	if (!dsSDLApplication_prepareRendererOptions(
			&rendererOptions, dsRenderBootstrap_rendererID(rendererType)))
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't setup renderer options.");
		return NULL;
	}

	dsRenderer* renderer = dsRenderBootstrap_createRenderer(rendererType,
		(dsAllocator*)&renderAllocator, &rendererOptions);
	if (!renderer)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create renderer: %s", dsErrorString(errno));
		return NULL;
	}

	dsRenderer_setVSync(renderer, dsVSync_TripleBuffer);
#if DS_DEBUG
	dsRenderer_setExtraDebugging(renderer, true);
#endif

	dsApplication* application = dsSDLApplication_create((dsAllocator*)&applicationAllocator,
		renderer, argc, argv, "DeepSea", "TestScene", dsSDLApplicationFlags_None);
	if (!application)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create application: %s", dsErrorString(errno));
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

	char assetsPath[DS_PATH_MAX];
	DS_VERIFY(dsResourceStream_getPath(assetsPath, sizeof(assetsPath), dsFileResourceType_Embedded,
		"TestScene-assets"));
	dsResourceStream_setEmbeddedDirectory(assetsPath);

	if (!setup(application, (dsAllocator*)&testSceneAllocator, updateFps))
	{
		dsApplication_destroy(application);
		return NULL;
	}
	return application;
}

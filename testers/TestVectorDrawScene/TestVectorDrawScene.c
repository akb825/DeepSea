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
#include <DeepSea/Render/CommandBuffer.h>
#include <DeepSea/Render/CommandBufferPool.h>
#include <DeepSea/RenderBootstrap/RenderBootstrap.h>

#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Scene.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/SceneResources.h>
#include <DeepSea/Scene/View.h>
#include <DeepSea/Scene/ViewTransformData.h>

#include <DeepSea/Text/TextSubstitutionTable.h>

#include <DeepSea/VectorDrawScene/SceneTextNode.h>
#include <DeepSea/VectorDrawScene/SceneVectorItemList.h>
#include <DeepSea/VectorDrawScene/VectorSceneLoadContext.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if DS_HAS_EASY_PROFILER
#include <DeepSea/EasyProfiler/EasyProfiler.h>
#endif

typedef struct TestVectorDrawScene
{
	dsAllocator* allocator;
	dsRenderer* renderer;
	dsWindow* window;
	dsSceneResources* resources;
	dsSceneTextNode* figureNode;
	dsCommandBufferPool* initCommandBufferPool;
	dsCommandBuffer* initCommandBuffer;
	dsScene* scene;
	dsView* view;

	double changeTime;
	unsigned int skipCount;
} TestVectorDrawScene;

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

	DS_LOG_ERROR_F("TestVectorDrawScene",
		"Allocator '%s' has %llu bytes allocated with %u allocations.", name,
		(unsigned long long)allocator->size, allocator->currentAllocations);
	return false;
}

static void updateProjectionMatrix(dsView* view)
{
	float aspect = (float)view->width/(float)view->height;
	dsMatrix44f projection;
	DS_VERIFY(dsRenderer_makeOrtho(&projection, dsScene_getRenderer(view->scene),
		-aspect*100.0f, aspect*100.0f, -100.0f, 100.0f, -1.0f, 1.0f));
	DS_VERIFY(dsView_setProjectionMatrix(view, &projection));
}

static bool processEvent(dsApplication* application, dsWindow* window, const dsEvent* event,
	void* userData)
{
	TestVectorDrawScene* testVectorDrawScene = (TestVectorDrawScene*)userData;
	DS_ASSERT(!window || window == testVectorDrawScene->window);
	switch (event->type)
	{
		case dsAppEventType_WindowClosed:
			DS_VERIFY(dsWindow_destroy(window));
			testVectorDrawScene->window = NULL;
			return false;
		case dsAppEventType_SurfaceInvalidated:
			DS_VERIFY(dsView_setSurface(testVectorDrawScene->view, "windowColor",
				testVectorDrawScene->window->surface, dsGfxSurfaceType_ColorRenderSurface));
			// Fall through
		case dsAppEventType_WindowResized:
			DS_VERIFY(dsView_setDimensions(testVectorDrawScene->view,
				testVectorDrawScene->window->surface->width,
				testVectorDrawScene->window->surface->height,
				testVectorDrawScene->window->surface->rotation));
			updateProjectionMatrix(testVectorDrawScene->view);
			// Need to update the view again if the surfaces have been set.
			if (event->type == dsAppEventType_SurfaceInvalidated)
				dsView_update(testVectorDrawScene->view);
			return true;
		case dsAppEventType_WillEnterForeground:
			// Takes two frames to go through the bad frame time values.
			testVectorDrawScene->skipCount = 2;
			return false;
		case dsAppEventType_KeyDown:
			if (event->key.repeat)
				return false;

			if (event->key.key == dsKeyCode_ACBack)
				dsApplication_quit(application, 0);
			return false;
		default:
			return true;
	}
}

static void update(dsApplication* application, double lastFrameTime, void* userData)
{
	DS_UNUSED(application);

	TestVectorDrawScene* testVectorDrawScene = (TestVectorDrawScene*)userData;

	if (testVectorDrawScene->initCommandBuffer)
	{
		DS_VERIFY(dsCommandBuffer_submit(testVectorDrawScene->renderer->mainCommandBuffer,
			testVectorDrawScene->initCommandBuffer));
		DS_VERIFY(dsCommandBufferPool_destroy(testVectorDrawScene->initCommandBufferPool));
		testVectorDrawScene->initCommandBufferPool = NULL;
		testVectorDrawScene->initCommandBuffer = NULL;
	}

	const double newCharTime = 0.1;
	const double clearTime = 1.0;
	if (testVectorDrawScene->skipCount > 0)
		--testVectorDrawScene->skipCount;
	else
		testVectorDrawScene->changeTime += lastFrameTime;
	if (testVectorDrawScene->figureNode->charCount >=
		testVectorDrawScene->figureNode->layout->text->characterCount)
	{
		if (testVectorDrawScene->changeTime >= clearTime)
		{
			testVectorDrawScene->figureNode->charCount = 0;
			testVectorDrawScene->changeTime -= clearTime;
		}
	}
	else
	{
		if (testVectorDrawScene->changeTime >= newCharTime)
		{
			++testVectorDrawScene->figureNode->charCount;
			testVectorDrawScene->changeTime -= newCharTime;
		}
	}

	DS_VERIFY(dsScene_update(testVectorDrawScene->scene));
	DS_VERIFY(dsView_update(testVectorDrawScene->view));
}

static void draw(dsApplication* application, dsWindow* window, void* userData)
{
	DS_UNUSED(application);
	DS_UNUSED(window);
	TestVectorDrawScene* testVectorDrawScene = (TestVectorDrawScene*)userData;
	DS_ASSERT(testVectorDrawScene->window == window);
	dsRenderer* renderer = testVectorDrawScene->renderer;
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	DS_VERIFY(dsView_draw(testVectorDrawScene->view, commandBuffer, NULL));
}

static bool setup(TestVectorDrawScene* testVectorDrawScene, dsApplication* application,
	dsAllocator* allocator)
{
	dsRenderer* renderer = application->renderer;
	testVectorDrawScene->allocator = allocator;
	testVectorDrawScene->renderer = renderer;

	dsEventResponder responder = {&processEvent, testVectorDrawScene, 0, 0};
	DS_VERIFY(dsApplication_addEventResponder(application, &responder));
	DS_VERIFY(dsApplication_setUpdateFunction(application, &update, testVectorDrawScene));

	uint32_t width = dsApplication_adjustWindowSize(application, 0, 800);
	uint32_t height = dsApplication_adjustWindowSize(application, 0, 600);
	testVectorDrawScene->window = dsWindow_create(application, allocator, "Test Scene", NULL,
		NULL, width, height, dsWindowFlags_Resizeable | dsWindowFlags_DelaySurfaceCreate,
		dsRenderSurfaceUsage_ClientRotations);
	if (!testVectorDrawScene->window)
	{
		DS_LOG_ERROR_F("TestVectorDrawScene", "Couldn't create window: %s", dsErrorString(errno));
		return false;
	}

	if (DS_ANDROID || DS_IOS)
		dsWindow_setStyle(testVectorDrawScene->window, dsWindowStyle_FullScreen);

	if (!dsWindow_createSurface(testVectorDrawScene->window))
	{
		DS_LOG_ERROR_F("TestVectorDrawScene", "Couldn't create window surface: %s",
			dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsWindow_setDrawFunction(testVectorDrawScene->window, &draw, testVectorDrawScene));

	testVectorDrawScene->initCommandBufferPool = dsCommandBufferPool_create(renderer, allocator,
		dsCommandBufferUsage_Standard);
	if (!testVectorDrawScene->initCommandBufferPool)
	{
		DS_LOG_ERROR_F("TestVectorDrawScene", "Couldn't create command buffer pool: %s",
			dsErrorString(errno));
		return false;
	}

	dsCommandBuffer** commandBuffers =
		dsCommandBufferPool_createCommandBuffers(testVectorDrawScene->initCommandBufferPool, 1);
	if (!commandBuffers)
	{
		DS_LOG_ERROR_F("TestVectorDrawScene", "Couldn't create command buffer: %s",
			dsErrorString(errno));
		return false;
	}

	testVectorDrawScene->initCommandBuffer = commandBuffers[0];

	dsSceneLoadContext* loadContext = dsSceneLoadContext_create(allocator, renderer);
	if (!loadContext)
	{
		DS_LOG_ERROR_F("TestVectorDrawScene", "Couldn't create load context: %s",
			dsErrorString(errno));
		return false;
	}

	dsTextSubstitutionTable* substitutionTable = dsTextSubstitutionTable_create(allocator, 1);
	if (!substitutionTable)
	{
		DS_LOG_ERROR_F("TestVectorDrawScene", "Couldn't create text substitution table: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		return false;
	}

	if (!dsTextSubstitutionTable_setString(substitutionTable, "tigerNum", "1"))
	{
		DS_LOG_ERROR_F("TestVectorDrawScene", "Couldn't register text subtitution: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsTextSubstitutionTable_destroy(substitutionTable);
		return false;
	}

	dsVertexFormat textVertexFormat;
	DS_VERIFY(dsSceneTextNode_defaultTextVertexFormat(&textVertexFormat));
	dsSceneTextRenderBufferInfo textRenderInfo =
	{
		&textVertexFormat,
		dsSceneTextNode_defaultGlyphDataFunc,
		NULL
	};

	float pixelSize = 200.0f/(float)testVectorDrawScene->window->surface->height;
	if (!dsVectorSceneLoadConext_registerTypes(loadContext, allocator,
			testVectorDrawScene->initCommandBuffer, NULL, substitutionTable, &textRenderInfo,
			pixelSize))
	{
		DS_LOG_ERROR_F("TestVectorDrawScene", "Couldn't register vector scene types: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsTextSubstitutionTable_destroy(substitutionTable);
		return false;
	}

	dsSceneLoadScratchData* scratchData = dsSceneLoadScratchData_create(allocator,
		renderer->mainCommandBuffer);
	if (!scratchData)
	{
		DS_LOG_ERROR_F("TestVectorDrawScene", "Couldn't create load scratch data: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsTextSubstitutionTable_destroy(substitutionTable);
		return false;
	}

	DS_VERIFY(dsCommandBuffer_begin(testVectorDrawScene->initCommandBuffer));
	testVectorDrawScene->resources = dsSceneResources_loadResource(allocator, NULL, loadContext,
		scratchData, dsFileResourceType_Embedded, "SceneResources.dssr");
	DS_VERIFY(dsCommandBuffer_end(testVectorDrawScene->initCommandBuffer));
	if (!testVectorDrawScene->resources)
	{
		DS_LOG_ERROR_F("TestVectorDrawScene", "Couldn't load scene resources: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsTextSubstitutionTable_destroy(substitutionTable);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData,
		&testVectorDrawScene->resources, 1));

	dsSceneResourceType resourceType;
	const dsSceneNodeType* textNodeType = dsSceneTextNode_type();
	if (!dsSceneResources_findResource(&resourceType, (void**)&testVectorDrawScene->figureNode,
			testVectorDrawScene->resources, "figureNode") ||
		resourceType != dsSceneResourceType_SceneNode ||
		!dsSceneNode_isOfType((dsSceneNode*)testVectorDrawScene->figureNode, textNodeType))
	{
		DS_LOG_ERROR("TestVectorDrawScene", "Couldn't find text node 'figureNode'.");
		dsSceneLoadContext_destroy(loadContext);
		dsTextSubstitutionTable_destroy(substitutionTable);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	testVectorDrawScene->scene = dsScene_loadResource(allocator, NULL, loadContext, scratchData,
		NULL, NULL, dsFileResourceType_Embedded, "Scene.dss");
	if (!testVectorDrawScene->scene)
	{
		DS_LOG_ERROR_F("TestVectorDrawScene", "Couldn't load scene: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsTextSubstitutionTable_destroy(substitutionTable);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	dsRenderSurface* surface = testVectorDrawScene->window->surface;
	dsViewSurfaceInfo viewSurface;
	viewSurface.name = "windowColor";
	viewSurface.surfaceType = dsGfxSurfaceType_ColorRenderSurface;
	viewSurface.surface = surface;
	viewSurface.windowFramebuffer = true;

	testVectorDrawScene->view = dsView_loadResource(testVectorDrawScene->scene, allocator, NULL,
		scratchData, &viewSurface, 1, surface->width, surface->height, surface->rotation, NULL,
		NULL, dsFileResourceType_Embedded, "View.dsv");
	dsSceneLoadContext_destroy(loadContext);
	dsTextSubstitutionTable_destroy(substitutionTable);
	dsSceneLoadScratchData_destroy(scratchData);
	if (!testVectorDrawScene->view)
	{
		DS_LOG_ERROR_F("TestVectorDrawScene", "Couldn't load view: %s", dsErrorString(errno));
		return false;
	}

	updateProjectionMatrix(testVectorDrawScene->view);
	return true;
}

static void shutdown(TestVectorDrawScene* testVectorDrawScene)
{
	DS_VERIFY(dsView_destroy(testVectorDrawScene->view));
	dsScene_destroy(testVectorDrawScene->scene);
	dsSceneResources_freeRef(testVectorDrawScene->resources);
	DS_VERIFY(dsCommandBufferPool_destroy(testVectorDrawScene->initCommandBufferPool));
	DS_VERIFY(dsWindow_destroy(testVectorDrawScene->window));
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

	DS_LOG_INFO_F("TestVectorDrawScene", "Render using %s",
		dsRenderBootstrap_rendererName(rendererType));

	dsSystemAllocator renderAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&renderAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator applicationAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&applicationAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator testVectorDrawSceneAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&testVectorDrawSceneAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsRendererOptions rendererOptions;
	dsRenderer_defaultOptions(&rendererOptions, "TestVectorDrawScene", 0);
	rendererOptions.deviceName = deviceName;
	rendererOptions.depthBits = 0;
	rendererOptions.stencilBits = 0;
	rendererOptions.samples = 4;
	rendererOptions.maxResourceThreads = 1;
	dsRenderer* renderer = dsRenderBootstrap_createRenderer(rendererType,
		(dsAllocator*)&renderAllocator, &rendererOptions);
	if (!renderer)
	{
		DS_LOG_ERROR_F("TestVectorDrawScene", "Couldn't create renderer: %s", dsErrorString(errno));
		return 2;
	}

	dsRenderer_setVsync(renderer, true);
	dsRenderer_setDefaultAnisotropy(renderer, renderer->maxAnisotropy);
#if DS_DEBUG
	dsRenderer_setExtraDebugging(renderer, true);
#endif

	dsApplication* application = dsSDLApplication_create((dsAllocator*)&applicationAllocator,
		renderer, argc, argv, "DeepSea", "TestVectorDrawScene");
	if (!application)
	{
		DS_LOG_ERROR_F("TestVectorDrawScene", "Couldn't create application: %s",
			dsErrorString(errno));
		dsRenderer_destroy(renderer);
		return 2;
	}

	char assetsPath[DS_PATH_MAX];
	DS_VERIFY(dsPath_combine(assetsPath, sizeof(assetsPath), dsResourceStream_getEmbeddedDir(),
		"TestVectorDrawScene-assets"));
	dsResourceStream_setEmbeddedDir(assetsPath);

	TestVectorDrawScene testVectorDrawScene;
	memset(&testVectorDrawScene, 0, sizeof(testVectorDrawScene));
	if (!setup(&testVectorDrawScene, application, (dsAllocator*)&testVectorDrawSceneAllocator))
	{
		shutdown(&testVectorDrawScene);
		return 3;
	}

	int exitCode = dsApplication_run(application);

	shutdown(&testVectorDrawScene);
	dsSDLApplication_destroy(application);
	dsRenderer_destroy(renderer);

	if (!validateAllocator((dsAllocator*)&renderAllocator, "render"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&applicationAllocator, "application"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&testVectorDrawSceneAllocator, "TestVectorDrawScene"))
		exitCode = 4;

	return exitCode;
}

/*
 * Copyright 2019-2025 Aaron Barany
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
#include <DeepSea/Core/Streams/FileArchive.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Streams/Stream.h>
#include <DeepSea/Core/Streams/ZipArchive.h>
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

#include <DeepSea/Scene/ItemLists/InstanceTransformData.h>
#include <DeepSea/Scene/ItemLists/SceneModelList.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Scene.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/SceneResources.h>
#include <DeepSea/Scene/SceneThreadManager.h>
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

typedef enum LightingType
{
	LightingType_Forward,
	LightingType_Deferred,
	LightingType_SSAODeferred,
	LightingType_Count
} LightingType;

typedef struct TestLighting
{
	dsAllocator* allocator;
	dsRenderer* renderer;
	dsWindow* window;
	dsThreadPool* threadPool;
	dsSceneThreadManager* threadManager;

	void* lightSceneData[LightingType_Count];
	size_t lightSceneDataSizes[LightingType_Count];

	dsSceneLoadContext* loadContext;
	dsSceneLoadScratchData* scratchData;
	dsSceneResources* builtinResources;
	dsSceneResources* baseResources;
	dsSceneResources* models;
	dsSceneResources* lightShaderResources;
	dsSceneResources* sceneGraphResources;
	dsScene* scene;
	dsView* view;

	uint32_t aaSamples;
	LightingType lightingType;
	float rotation;
	uint32_t fingerCount;
	uint32_t maxFingers;
	bool ignoreTime;
	bool stop;
} TestLighting;

static const char* lightingTypeNames[LightingType_Count] = {"forward", "deferred", "SSAO"};

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

static void setDirectionalShadowBias(dsDynamicRenderStates* outRenderStates,
	const dsRenderer* renderer, const dsDynamicRenderStates* curRenderStates)
{
	DS_UNUSED(renderer);
	if (curRenderStates)
		*outRenderStates = *curRenderStates;

	outRenderStates->depthBiasConstantFactor = -1.0f;
	outRenderStates->depthBiasSlopeFactor = -2.0f;
	outRenderStates->depthBiasClamp = 0.0f;
}

static void setPointShadowBias(dsDynamicRenderStates* outRenderStates, const dsRenderer* renderer,
	const dsDynamicRenderStates* curRenderStates)
{
	DS_UNUSED(renderer);
	if (curRenderStates)
		*outRenderStates = *curRenderStates;

	if (renderer->rendererID == DS_VK_RENDERER_ID)
	{
		outRenderStates->depthBiasConstantFactor = 0.0f;
		outRenderStates->depthBiasSlopeFactor = 1.0f;
		outRenderStates->depthBiasClamp = 0.0f;
	}
	else
	{
		outRenderStates->depthBiasConstantFactor = -0.2f;
		outRenderStates->depthBiasSlopeFactor = -2.0f;
		outRenderStates->depthBiasClamp = 0.0f;
	}
}

static void setSpotShadowBias(dsDynamicRenderStates* outRenderStates, const dsRenderer* renderer,
	const dsDynamicRenderStates* curRenderStates)
{
	DS_UNUSED(renderer);
	if (curRenderStates)
		*outRenderStates = *curRenderStates;

	if (renderer->rendererID == DS_VK_RENDERER_ID)
	{
		outRenderStates->depthBiasConstantFactor = -0.5f;
		outRenderStates->depthBiasSlopeFactor = 2.0f;
		outRenderStates->depthBiasClamp = 0.0f;
	}
	else
	{
		outRenderStates->depthBiasConstantFactor = -0.2f;
		outRenderStates->depthBiasSlopeFactor = -2.0f;
		outRenderStates->depthBiasClamp = 0.0f;
	}
}

static bool updateItemListShadowBias(dsSceneItemList* itemList, void* userData)
{
	const char* mainShadowPrefix = "mainShadowCastList";
	const char* pointShadowPrefix = "pointShadowCastList";
	const char* spotShadowPrefix = "spotShadowCastList";
	const dsRenderer* renderer = (const dsRenderer*)userData;

	if (itemList->type != dsSceneModelList_type())
		return true;

	dsSceneModelList* modelList = (dsSceneModelList*)itemList;
	const dsDynamicRenderStates* curRenderStates = dsSceneModelList_getRenderStates(modelList);
	dsDynamicRenderStates newRenderStates;
	if (strncmp(itemList->name, mainShadowPrefix, strlen(mainShadowPrefix)) == 0)
	{
		setDirectionalShadowBias(&newRenderStates, renderer, curRenderStates);
		dsSceneModelList_setRenderStates(modelList, &newRenderStates);
	}
	else if (strncmp(itemList->name, pointShadowPrefix, strlen(pointShadowPrefix)) == 0)
	{
		setPointShadowBias(&newRenderStates, renderer, curRenderStates);
		dsSceneModelList_setRenderStates(modelList, &newRenderStates);
	}
	else if (strncmp(itemList->name, spotShadowPrefix, strlen(spotShadowPrefix)) == 0)
	{
		setSpotShadowBias(&newRenderStates, renderer, curRenderStates);
		dsSceneModelList_setRenderStates(modelList, &newRenderStates);
	}

	return true;
}

static bool loadLighting(TestLighting* testLighting, LightingType lightingType)
{
	// Set samples first so it will see the correct number of samples at load time.
	uint32_t samples = lightingType == LightingType_Forward ? testLighting->aaSamples : 1;
	dsRenderer_setDefaultSamples(testLighting->renderer, samples);

	testLighting->scene = dsScene_loadData(testLighting->allocator, testLighting->allocator,
		testLighting->loadContext, testLighting->scratchData, NULL, NULL, testLighting->scene,
		testLighting->lightSceneData[lightingType],
		testLighting->lightSceneDataSizes[lightingType]);
	if (!testLighting->scene)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load %s light scene: %s",
			lightingTypeNames[lightingType], dsErrorString(errno));
		return false;
	}
	DS_VERIFY(dsScene_forEachItemList(
		testLighting->scene, &updateItemListShadowBias, testLighting->renderer));

	if (testLighting->view && !dsView_setScene(testLighting->view, testLighting->scene))
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't set %s light scene on view: %s",
			lightingTypeNames[lightingType], dsErrorString(errno));
	}
	testLighting->lightingType = lightingType;
	return true;
}

static void nextLightingType(TestLighting* testLighting)
{
	if (!loadLighting(testLighting,
			(LightingType)((testLighting->lightingType + 1) % LightingType_Count)))
	{
		DS_LOG_FATAL("TestLighting", "Couldn't change lighting types.");
		abort();
	}
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
			DS_VERIFY(dsView_setSurface(testLighting->view, "windowColor",
				testLighting->window->surface, dsGfxSurfaceType_ColorRenderSurface));
			// Fall through
		case dsAppEventType_WindowResized:
			DS_VERIFY(dsView_setDimensions(testLighting->view, testLighting->window->surface->width,
				testLighting->window->surface->height, testLighting->window->surface->rotation));
			// Need to update the view again if the surfaces have been set.
			if (event->type == dsAppEventType_SurfaceInvalidated)
				dsView_update(testLighting->view);
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
				nextLightingType(testLighting);
			else if (event->key.key == dsKeyCode_1)
			{
				if (testLighting->aaSamples == 1)
					testLighting->aaSamples = 4;
				else
					testLighting->aaSamples = 1;
				if (testLighting->lightingType == LightingType_Forward)
					dsRenderer_setDefaultSamples(renderer, testLighting->aaSamples);
				DS_LOG_INFO_F("TestLighting", "Togging anti-aliasing: %s",
					testLighting->aaSamples == 1 ? "off" : "on");
			}
			return false;
		case dsAppEventType_TouchFingerDown:
			++testLighting->fingerCount;
			testLighting->maxFingers = dsMax(testLighting->fingerCount, testLighting->maxFingers);
			return true;
		case dsAppEventType_TouchFingerUp:
			if (testLighting->fingerCount == 0)
				return true;

			--testLighting->fingerCount;
			if (testLighting->fingerCount == 0)
			{
				switch (testLighting->maxFingers)
				{
					case 1:
						testLighting->stop = !testLighting->stop;
						break;
					case 2:
						nextLightingType(testLighting);
						break;
					default:
						break;
				}
				testLighting->maxFingers = 0;
			}
			return true;
		default:
			return true;
	}
}

static void update(dsApplication* application, float lastFrameTime, void* userData)
{
	DS_UNUSED(application);
	DS_UNUSED(lastFrameTime);

	TestLighting* testLighting = (TestLighting*)userData;

	const float speed = 0.4f;
	const float xyDist = 7.0f;
	const float height = 9.0f;
	if (!testLighting->stop && !testLighting->ignoreTime)
	{
		testLighting->rotation = dsWrapf(testLighting->rotation + lastFrameTime*speed,
			0.0f, 2*M_PIf);
	}
	if (testLighting->ignoreTime)
		testLighting->ignoreTime = false;

	dsVector3f eyePos = {{sinf(testLighting->rotation)*xyDist, -cosf(testLighting->rotation)*xyDist,
		height}};
	dsVector3f lookAtPos = {{0.0f, 0.0f, 0.0f}};
	dsVector3f upDir = {{0.0f, 0.0f, 1.0f}};
	dsMatrix44f camera;
	dsMatrix44f_lookAt(&camera, &eyePos, &lookAtPos, &upDir);
	dsView_setCameraMatrix(testLighting->view, &camera);

	DS_VERIFY(dsScene_update(testLighting->scene, lastFrameTime));
	DS_VERIFY(dsView_update(testLighting->view));
}

static void draw(dsApplication* application, dsWindow* window, void* userData)
{
	DS_UNUSED(application);
	DS_UNUSED(window);
	TestLighting* testLighting = (TestLighting*)userData;
	DS_ASSERT(testLighting->window == window);
	dsRenderer* renderer = testLighting->renderer;
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	DS_VERIFY(dsView_draw(testLighting->view, commandBuffer, testLighting->threadManager));
}

static void* loadFileData(
	size_t* outSize, dsFileArchive* archive, dsAllocator* allocator, const char* fileName)
{
	dsStream* stream = dsFileArchive_openFile(archive, fileName);
	if (!stream)
		return NULL;

	void* data =  dsStream_readUntilEnd(outSize, stream, allocator);
	DS_VERIFY(dsStream_close(stream));
	return data;
}

static bool setup(TestLighting* testLighting, dsApplication* application, dsAllocator* allocator)
{
	dsRenderer* renderer = application->renderer;
	dsResourceManager* resourceManager = renderer->resourceManager;
	testLighting->allocator = allocator;
	testLighting->renderer = renderer;

	testLighting->threadPool = dsResourceManager_createThreadPool(allocator, resourceManager,
		dsThreadPool_defaultThreadCount(), 0);
	if (!testLighting->threadPool)
		return false;

	testLighting->threadManager =
		dsSceneThreadManager_create(allocator, renderer, testLighting->threadPool);
	if (!testLighting->threadManager)
		return false;

	dsEventResponder responder = {&processEvent, testLighting, 0, 0};
	DS_VERIFY(dsApplication_addEventResponder(application, &responder));
	DS_VERIFY(dsApplication_setUpdateFunction(application, &update, testLighting, NULL));

	uint32_t width = dsApplication_adjustWindowSize(application, 0, 800);
	uint32_t height = dsApplication_adjustWindowSize(application, 0, 600);
	testLighting->window = dsWindow_create(application, allocator, "Test Lighting", NULL,
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

	DS_VERIFY(dsWindow_setDrawFunction(testLighting->window, &draw, testLighting, NULL));

	dsSceneLoadContext* loadContext = dsSceneLoadContext_create(allocator, renderer);
	if (!loadContext)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't create load context: %s", dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsSceneLightingLoadConext_registerTypes(loadContext));
	testLighting->loadContext = loadContext;

	dsSceneLoadScratchData* scratchData = dsSceneLoadScratchData_create(allocator,
		renderer->mainCommandBuffer);
	if (!scratchData)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't create load scratch data: %s",
			dsErrorString(errno));
		return false;
	}

	testLighting->scratchData = scratchData;
	testLighting->builtinResources = dsSceneResources_create(allocator, 3);
	if (!testLighting->builtinResources)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't create scene resources: %s", dsErrorString(errno));
		return false;
	}

	dsShaderVariableGroupDesc* groupDesc =
		dsInstanceTransformData_createShaderVariableGroupDesc(resourceManager, allocator);
	if (!groupDesc)
	{
		DS_LOG_ERROR_F("TestLighting",
			"Couldn't create instance transform shader variable desc: %s", dsErrorString(errno));
		return false;
	}
	DS_VERIFY(dsSceneResources_addResource(testLighting->builtinResources,
		"instanceTransformDesc", dsSceneResourceType_ShaderVariableGroupDesc, groupDesc, true));

	groupDesc = dsViewTransformData_createShaderVariableGroupDesc(resourceManager, allocator);
	if (!groupDesc)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't create view transform shader variable desc: %s",
			dsErrorString(errno));
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
		return false;
	}
	DS_VERIFY(dsSceneResources_addResource(testLighting->builtinResources,
		"instanceForwardLightDesc", dsSceneResourceType_ShaderVariableGroupDesc, groupDesc, true));

	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData,
		&testLighting->builtinResources, 1));

	dsFileArchive* archive = (dsFileArchive*)dsZipArchive_openResource(
		allocator, dsFileResourceType_Embedded, "TestLighting-assets.zip", 0);
	if (!archive)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't open assets zip: %s", dsErrorString(errno));
		return false;
	}

	testLighting->baseResources = dsSceneResources_loadArchive(allocator, NULL, loadContext,
		scratchData, archive, "BaseResources.dssr");
	if (!testLighting->baseResources)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load base scene resources: %s",
			dsErrorString(errno));
		dsFileArchive_close(archive);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData,
		&testLighting->baseResources, 1));

	const char* sceneFileNames[LightingType_Count];
	const char* shadersFileName;
	const char* viewFileName;
	if (testLighting->renderer->hasFragmentInputs)
	{
		sceneFileNames[LightingType_Forward] = "ForwardLightFIScene.dss";
		sceneFileNames[LightingType_Deferred] = "DeferredLightFIScene.dss";
		sceneFileNames[LightingType_SSAODeferred] = "SSAOFIScene.dss";
		shadersFileName = "LightShadersFI.dssr";
		viewFileName = "ViewFI.dsv";
	}
	else
	{
		sceneFileNames[LightingType_Forward] = "ForwardLightScene.dss";
		sceneFileNames[LightingType_Deferred] = "DeferredLightScene.dss";
		sceneFileNames[LightingType_SSAODeferred] = "SSAOScene.dss";
		shadersFileName = "LightShaders.dssr";
		viewFileName = "View.dsv";
	}

	for (int i = 0; i < LightingType_Count; ++i)
	{
		testLighting->lightSceneData[i] = loadFileData(testLighting->lightSceneDataSizes + i,
			archive, allocator, sceneFileNames[i]);
		if (!testLighting->lightSceneData[i])
		{
			DS_LOG_ERROR_F("TestLighting", "Couldn't load %s light scene: %s",
				lightingTypeNames[i], dsErrorString(errno));
			dsFileArchive_close(archive);
			return false;
		}
	}

	testLighting->lightShaderResources = dsSceneResources_loadArchive(allocator, NULL, loadContext,
		scratchData, archive, shadersFileName);
	if (!testLighting->lightShaderResources)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load light shader scene resources: %s",
			dsErrorString(errno));
		dsFileArchive_close(archive);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData,
		&testLighting->lightShaderResources, 1));

	testLighting->models = dsSceneResources_loadArchive(allocator, NULL, loadContext, scratchData,
		archive, "Models.dssr");
	if (!testLighting->models)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load model scene resources: %s",
			dsErrorString(errno));
		dsFileArchive_close(archive);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData, &testLighting->models, 1));

	testLighting->sceneGraphResources = dsSceneResources_loadArchive(allocator, NULL,
		loadContext, scratchData, archive, "SceneGraph.dssr");
	if (!testLighting->sceneGraphResources)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load scene graph resources: %s",
			dsErrorString(errno));
		dsFileArchive_close(archive);
		return false;
	}

	dsSceneResourceType rootNodeType;
	dsSceneNode* rootNode;
	if (!dsSceneResources_findResource(&rootNodeType, (void**)&rootNode,
			testLighting->sceneGraphResources, "rootNode") ||
		rootNodeType != dsSceneResourceType_SceneNode)
	{
		DS_LOG_ERROR("TestLighting", "Couldn't find root node in scene graph.");
		dsFileArchive_close(archive);
		return false;
	}

	testLighting->aaSamples = renderer->defaultSamples;
	if (!loadLighting(testLighting, LightingType_Forward))
	{
		dsFileArchive_close(archive);
		return false;
	}
	dsScene_addNode(testLighting->scene, rootNode);

	dsRenderSurface* surface = testLighting->window->surface;
	dsViewSurfaceInfo viewSurface;
	viewSurface.name = "windowColor";
	viewSurface.surfaceType = dsGfxSurfaceType_ColorRenderSurface;
	viewSurface.surface = surface;
	viewSurface.windowFramebuffer = true;

	testLighting->view = dsView_loadArchive(allocator, testLighting->scene, NULL, scratchData,
		&viewSurface, 1, surface->width, surface->height, surface->rotation, NULL, NULL, archive,
		viewFileName);
	if (!testLighting->view)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't load view: %s", dsErrorString(errno));
		dsFileArchive_close(archive);
		return false;
	}

	dsFileArchive_close(archive);
	dsView_setPerspectiveProjection(testLighting->view, dsDegreesToRadiansf(45.0f), 0.1f, 100.0f);
	return true;
}

static void shutdown(TestLighting* testLighting)
{
	dsSceneLoadContext_destroy(testLighting->loadContext);
	dsSceneLoadScratchData_destroy(testLighting->scratchData);
	for (int i = 0; i < LightingType_Count; ++i)
		DS_VERIFY(dsAllocator_free(testLighting->allocator, testLighting->lightSceneData[i]));

	DS_VERIFY(dsView_destroy(testLighting->view));
	dsScene_destroy(testLighting->scene);

	dsSceneResources_freeRef(testLighting->sceneGraphResources);
	dsSceneResources_freeRef(testLighting->lightShaderResources);
	dsSceneResources_freeRef(testLighting->models);
	dsSceneResources_freeRef(testLighting->baseResources);
	dsSceneResources_freeRef(testLighting->builtinResources);
	DS_VERIFY(dsWindow_destroy(testLighting->window));

	dsSceneThreadManager_destroy(testLighting->threadManager);
	DS_VERIFY(dsThreadPool_destroy(testLighting->threadPool));
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
	DS_LOG_INFO("TestLighting", "Press enter to cycle lighting types.");
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
	rendererOptions.surfaceSamples = 1;
	rendererOptions.reverseZ = true;
	rendererOptions.preferHalfDepthRange = true;
	rendererOptions.deviceName = deviceName;
	rendererOptions.maxResourceThreads = dsThreadPool_defaultThreadCount();
	if (!dsSDLApplication_prepareRendererOptions(
			&rendererOptions, dsRenderBootstrap_rendererID(rendererType)))
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't setup renderer options.");
		return 0;
	}

	dsRenderer* renderer = dsRenderBootstrap_createRenderer(rendererType,
		(dsAllocator*)&renderAllocator, &rendererOptions);
	if (!renderer)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't create renderer: %s", dsErrorString(errno));
		return 2;
	}

	dsRenderer_setVSync(renderer, dsVSync_TripleBuffer);
	dsRenderer_setDefaultAnisotropy(renderer, dsMin(4.0f, renderer->maxAnisotropy));
#if DS_DEBUG
	dsRenderer_setExtraDebugging(renderer, true);
#endif

	dsApplication* application = dsSDLApplication_create((dsAllocator*)&applicationAllocator,
		renderer, argc, argv, "DeepSea", "TestLighting", dsSDLApplicationFlags_None);
	if (!application)
	{
		DS_LOG_ERROR_F("TestLighting", "Couldn't create application: %s", dsErrorString(errno));
		dsRenderer_destroy(renderer);
		return 2;
	}

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

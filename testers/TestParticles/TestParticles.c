/*
 * Copyright 2022 Aaron Barany
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

#include "LightFlicker.h"

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
#include <DeepSea/Scene/ItemLists/SceneModelList.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneTransformNode.h>
#include <DeepSea/Scene/Scene.h>
#include <DeepSea/Scene/SceneLoadContext.h>
#include <DeepSea/Scene/SceneLoadScratchData.h>
#include <DeepSea/Scene/SceneResources.h>
#include <DeepSea/Scene/View.h>
#include <DeepSea/Scene/ViewTransformData.h>

#include <DeepSea/SceneLighting/InstanceForwardLightData.h>
#include <DeepSea/SceneLighting/SceneLightingLoadContext.h>
#include <DeepSea/SceneLighting/SceneLightNode.h>

#include <DeepSea/SceneParticle/ParticleTransformData.h>
#include <DeepSea/SceneParticle/SceneParticleLoadContext.h>
#include <DeepSea/SceneParticle/SceneParticleNode.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if DS_HAS_EASY_PROFILER
#include <DeepSea/EasyProfiler/EasyProfiler.h>
#endif

typedef struct TestParticles
{
	dsAllocator* allocator;
	dsRenderer* renderer;
	dsWindow* window;
	dsSceneResources* builtinResources;
	dsSceneResources* baseResources;
	dsSceneResources* materials;
	dsSceneResources* sceneGraph;
	dsSceneTransformNode* rootNode;
	dsSceneTransformNode* rotatingTorches[2];
	dsSceneNode* staticTorch;
	dsSceneNode* staticTorchLight;
	dsScene* scene;
	dsView* view;

	uint32_t fingerCount;
	uint32_t maxFingers;
	bool ignoreTime;
	bool stop;
} TestParticles;

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

	DS_LOG_ERROR_F("TestParticles", "Allocator '%s' has %llu bytes allocated with %u allocations.",
		name, (unsigned long long)allocator->size, allocator->currentAllocations);
	return false;
}

static void toggleStaticTorch(TestParticles* testParticles)
{
	dsSceneNode* staticTorch = testParticles->staticTorch;
	bool enable;
	if (testParticles->staticTorchLight)
	{
		enable = true;
		DS_CHECK("TestParticles",
			dsSceneNode_addChild(staticTorch, testParticles->staticTorchLight));
		dsSceneNode_freeRef(testParticles->staticTorchLight);
		testParticles->staticTorchLight = NULL;
	}
	else
	{
		enable = false;
		uint32_t lightNodeIndex = staticTorch->childCount;
		for (uint32_t i = 0; i < staticTorch->childCount; ++i)
		{
			dsSceneNode* curNode = staticTorch->children[i];
			if (!dsSceneNode_isOfType(curNode, dsSceneLightNode_type()))
				continue;

			lightNodeIndex = i;
			break;
		}

		if (lightNodeIndex == staticTorch->childCount)
		{
			DS_LOG_ERROR("TestParticles", "No light node under static torch.");
			return;
		}

		testParticles->staticTorchLight = dsSceneNode_addRef(staticTorch->children[lightNodeIndex]);
		DS_VERIFY(dsSceneNode_removeChildIndex(staticTorch, lightNodeIndex));
	}

	for (uint32_t i = 0; i < staticTorch->childCount; ++i)
	{
		dsSceneNode* curNode = staticTorch->children[i];
		if (!dsSceneNode_isOfType(curNode, dsSceneParticleNode_type()))
			continue;

		for (uint32_t j = 0; j < curNode->treeNodeCount; ++j)
		{
			dsParticleEmitter* emitter = dsSceneParticleNode_getEmitterForInstance(
				curNode->treeNodes[j]);
			if (emitter)
				emitter->enabled = enable;
		}
	}
}

static void toggleRotatingTorch(TestParticles* testParticles)
{
	dsSceneNode* rootNode = (dsSceneNode*)testParticles->rootNode;
	dsSceneNode* rotatingTorch = (dsSceneNode*)testParticles->rotatingTorches[1];
	if (!dsSceneNode_removeChildNode(rootNode, rotatingTorch))
		DS_CHECK("TestParticles", dsSceneNode_addChild(rootNode, rotatingTorch));
}

static bool processEvent(dsApplication* application, dsWindow* window, const dsEvent* event,
	void* userData)
{
	TestParticles* testParticles = (TestParticles*)userData;
	DS_ASSERT(!window || window == testParticles->window);
	switch (event->type)
	{
		case dsAppEventType_WindowClosed:
			DS_VERIFY(dsWindow_destroy(window));
			testParticles->window = NULL;
			return false;
		case dsAppEventType_SurfaceInvalidated:
			DS_VERIFY(dsView_setSurface(testParticles->view, "windowColor",
				testParticles->window->surface, dsGfxSurfaceType_ColorRenderSurface));
			// Fall through
		case dsAppEventType_WindowResized:
			DS_VERIFY(dsView_setDimensions(testParticles->view,
				testParticles->window->surface->width, testParticles->window->surface->height,
				testParticles->window->surface->rotation));
			// Need to update the view again if the surfaces have been set.
			if (event->type == dsAppEventType_SurfaceInvalidated)
				dsView_update(testParticles->view);
			return true;
		case dsAppEventType_WillEnterForeground:
			testParticles->ignoreTime = true;
			return true;
		case dsAppEventType_KeyDown:
			if (event->key.repeat)
				return false;

			if (event->key.key == dsKeyCode_ACBack)
				dsApplication_quit(application, 0);
			else if (event->key.key == dsKeyCode_Space)
				testParticles->stop = !testParticles->stop;
			else if (event->key.key == dsKeyCode_1)
				toggleStaticTorch(testParticles);
			else if (event->key.key == dsKeyCode_2)
				toggleRotatingTorch(testParticles);
			return false;
		case dsAppEventType_TouchFingerDown:
			++testParticles->fingerCount;
			testParticles->maxFingers = dsMax(testParticles->fingerCount, testParticles->maxFingers);
			return true;
		case dsAppEventType_TouchFingerUp:
			if (testParticles->fingerCount == 0)
				return true;

			--testParticles->fingerCount;
			if (testParticles->fingerCount == 0)
			{
				switch (testParticles->maxFingers)
				{
					case 1:
						testParticles->stop = !testParticles->stop;
						break;
					case 2:
						toggleStaticTorch(testParticles);
						break;
					case 3:
						toggleRotatingTorch(testParticles);
						break;
					default:
						break;
				}
				testParticles->maxFingers = 0;
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

	TestParticles* testParticles = (TestParticles*)userData;
	if (!testParticles->stop && !testParticles->ignoreTime)
	{
		const float speed = 0.4f;
		dsMatrix44f rotate;
		dsMatrix44f_makeRotate(&rotate, 0.0f, 0.0f, speed*lastFrameTime);
		for (unsigned int i = 0; i < DS_ARRAY_SIZE(testParticles->rotatingTorches); ++i)
		{
			dsSceneTransformNode* transformNode = testParticles->rotatingTorches[i];
			dsMatrix44f updatedTransform;
			dsMatrix44_affineMul(updatedTransform, rotate, transformNode->transform);
			dsSceneTransformNode_setTransform(transformNode, &updatedTransform);
		}
	}
	if (testParticles->ignoreTime)
		testParticles->ignoreTime = false;

	DS_VERIFY(dsScene_update(testParticles->scene, lastFrameTime));
	DS_VERIFY(dsView_update(testParticles->view));
}

static void draw(dsApplication* application, dsWindow* window, void* userData)
{
	DS_UNUSED(application);
	DS_UNUSED(window);
	TestParticles* testParticles = (TestParticles*)userData;
	DS_ASSERT(testParticles->window == window);
	dsRenderer* renderer = testParticles->renderer;
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	DS_VERIFY(dsView_draw(testParticles->view, commandBuffer, NULL));
}

static bool setup(TestParticles* testParticles, dsApplication* application, dsAllocator* allocator)
{
	dsRenderer* renderer = application->renderer;
	dsResourceManager* resourceManager = renderer->resourceManager;
	testParticles->allocator = allocator;
	testParticles->renderer = renderer;

	dsEventResponder responder = {&processEvent, testParticles, 0, 0};
	DS_VERIFY(dsApplication_addEventResponder(application, &responder));
	DS_VERIFY(dsApplication_setUpdateFunction(application, &update, testParticles));

	uint32_t width = dsApplication_adjustWindowSize(application, 0, 800);
	uint32_t height = dsApplication_adjustWindowSize(application, 0, 600);
	testParticles->window = dsWindow_create(application, allocator, "Test Particles", NULL,
		NULL, width, height, dsWindowFlags_Resizeable | dsWindowFlags_DelaySurfaceCreate,
		dsRenderSurfaceUsage_ClientRotations);
	if (!testParticles->window)
	{
		DS_LOG_ERROR_F("TestParticles", "Couldn't create window: %s", dsErrorString(errno));
		return false;
	}

	if (DS_ANDROID || DS_IOS)
		dsWindow_setStyle(testParticles->window, dsWindowStyle_FullScreen);

	if (!dsWindow_createSurface(testParticles->window))
	{
		DS_LOG_ERROR_F("TestParticles", "Couldn't create window surface: %s", dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsWindow_setDrawFunction(testParticles->window, &draw, testParticles));

	dsSceneLoadContext* loadContext = dsSceneLoadContext_create(allocator, renderer);
	if (!loadContext)
	{
		DS_LOG_ERROR_F("TestParticles", "Couldn't create load context: %s", dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsSceneLightingLoadConext_registerTypes(loadContext));
	DS_VERIFY(dsSceneParticleLoadConext_registerTypes(loadContext));
	DS_VERIFY(dsSceneLoadContext_registerItemListType(loadContext, "LightFlicker",
		&dsLightFlicker_load, NULL, NULL));

	dsSceneLoadScratchData* scratchData = dsSceneLoadScratchData_create(allocator,
		renderer->mainCommandBuffer);
	if (!scratchData)
	{
		DS_LOG_ERROR_F("TestParticles", "Couldn't create load scratch data: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		return false;
	}

	testParticles->builtinResources = dsSceneResources_create(allocator, 4);
	if (!testParticles->builtinResources)
	{
		DS_LOG_ERROR_F("TestParticles", "Couldn't create scene resources: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	dsShaderVariableGroupDesc* groupDesc =
		dsInstanceTransformData_createShaderVariableGroupDesc(resourceManager, allocator);
	if (!groupDesc)
	{
		DS_LOG_ERROR_F("TestParticles",
			"Couldn't create instance transform shader variable desc: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneResources_addResource(testParticles->builtinResources,
		"instanceTransformDesc", dsSceneResourceType_ShaderVariableGroupDesc, groupDesc, true));

	groupDesc = dsParticleTransformData_createShaderVariableGroupDesc(resourceManager, allocator);
	if (!groupDesc)
	{
		DS_LOG_ERROR_F("TestParticles",
			"Couldn't create particle transform shader variable desc: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneResources_addResource(testParticles->builtinResources,
		"particleTransformDesc", dsSceneResourceType_ShaderVariableGroupDesc, groupDesc, true));

	groupDesc = dsViewTransformData_createShaderVariableGroupDesc(resourceManager, allocator);
	if (!groupDesc)
	{
		DS_LOG_ERROR_F("TestParticles", "Couldn't create view transform shader variable desc: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneResources_addResource(testParticles->builtinResources,
		"viewTransformDesc", dsSceneResourceType_ShaderVariableGroupDesc, groupDesc, true));

	groupDesc = dsInstanceForwardLightData_createShaderVariableGroupDesc(resourceManager, allocator,
		DS_DEFAULT_FORWARD_LIGHT_COUNT);
	if (!groupDesc)
	{
		DS_LOG_ERROR_F("TestParticles",
			"Couldn't create instance forward light shader variable desc: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneResources_addResource(testParticles->builtinResources,
		"instanceForwardLightDesc", dsSceneResourceType_ShaderVariableGroupDesc, groupDesc, true));

	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData,
		&testParticles->builtinResources, 1));

	testParticles->baseResources = dsSceneResources_loadResource(allocator, NULL, loadContext,
		scratchData, dsFileResourceType_Embedded, "BaseResources.dssr");
	if (!testParticles->baseResources)
	{
		DS_LOG_ERROR_F("TestParticles", "Couldn't load base scene resources: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData,
		&testParticles->baseResources, 1));

	testParticles->materials = dsSceneResources_loadResource(allocator, NULL, loadContext,
		scratchData, dsFileResourceType_Embedded, "Materials.dssr");
	if (!testParticles->materials)
	{
		DS_LOG_ERROR_F("TestParticles", "Couldn't load material scene resources: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(
		scratchData, &testParticles->materials, 1));

	testParticles->sceneGraph = dsSceneResources_loadResource(allocator, NULL, loadContext,
		scratchData, dsFileResourceType_Embedded, "SceneGraph.dssr");
	if (!testParticles->baseResources)
	{
		DS_LOG_ERROR_F("TestParticles", "Couldn't load scene graph: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(
		scratchData, &testParticles->sceneGraph, 1));

	dsSceneResourceType curType;
	dsSceneNode* curNode;
	if (!dsSceneResources_findResource(&curType, (void**)&curNode, testParticles->sceneGraph,
			"rootNode") ||
		curType != dsSceneResourceType_SceneNode ||
		!dsSceneNode_isOfType(curNode, dsSceneTransformNode_type()))
	{
		DS_LOG_ERROR("TestParticles", "Couldn't find rootNode.");
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	testParticles->rootNode = (dsSceneTransformNode*)dsSceneNode_addRef(curNode);

	const char* nodeNames[] = {"rotatingTorch1", "rotatingTorch2"};
	_Static_assert(DS_ARRAY_SIZE(nodeNames) == DS_ARRAY_SIZE(testParticles->rotatingTorches),
		"Node array sizes don't match.");
	for (unsigned int i = 0; i < DS_ARRAY_SIZE(nodeNames); ++i)
	{
		if (!dsSceneResources_findResource(&curType, (void**)&curNode, testParticles->sceneGraph,
				nodeNames[i]) ||
			curType != dsSceneResourceType_SceneNode ||
			!dsSceneNode_isOfType(curNode, dsSceneTransformNode_type()))
		{
			DS_LOG_ERROR_F("TestParticles", "Couldn't find %s.", nodeNames[i]);
			dsSceneLoadContext_destroy(loadContext);
			dsSceneLoadScratchData_destroy(scratchData);
			return false;
		}
		testParticles->rotatingTorches[i] = (dsSceneTransformNode*)dsSceneNode_addRef(curNode);
	}

	if (!dsSceneResources_findResource(&curType, (void**)&curNode, testParticles->sceneGraph,
			"staticTorch") ||
		curType != dsSceneResourceType_SceneNode ||
		!dsSceneNode_isOfType(curNode, dsSceneTransformNode_type()))
	{
		DS_LOG_ERROR("TestParticles", "Couldn't find statictorch.");
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	testParticles->staticTorch = dsSceneNode_addRef(curNode);

	testParticles->scene = dsScene_loadResource(allocator, NULL, loadContext, scratchData, NULL, NULL,
		dsFileResourceType_Embedded, "Scene.dss");
	if (!testParticles->scene)
	{
		DS_LOG_ERROR_F("TestParticles", "Couldn't load scene: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	dsRenderSurface* surface = testParticles->window->surface;
	dsViewSurfaceInfo viewSurface;
	viewSurface.name = "windowColor";
	viewSurface.surfaceType = dsGfxSurfaceType_ColorRenderSurface;
	viewSurface.surface = surface;
	viewSurface.windowFramebuffer = true;

	testParticles->view = dsView_loadResource(testParticles->scene, allocator, NULL, scratchData,
		&viewSurface, 1, surface->width, surface->height, surface->rotation, NULL, NULL,
		dsFileResourceType_Embedded, "View.dsv");
	if (!testParticles->view)
	{
		DS_LOG_ERROR_F("TestParticles", "Couldn't load view: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	dsSceneLoadContext_destroy(loadContext);
	dsSceneLoadScratchData_destroy(scratchData);

	dsView_setPerspectiveProjection(testParticles->view, dsDegreesToRadiansf(45.0f), 0.1f, 100.0f);

	dsVector3f eyePos = {{0.0f, -5.0f, 7.0f}};
	dsVector3f lookAtPos = {{0.0f, 0.0f, 0.0f}};
	dsVector3f upDir = {{0.0f, 0.0f, 1.0f}};
	dsMatrix44f camera;
	dsMatrix44f_lookAt(&camera, &eyePos, &lookAtPos, &upDir);
	dsView_setCameraMatrix(testParticles->view, &camera);

	return true;
}

static void shutdown(TestParticles* testParticles)
{
	DS_VERIFY(dsView_destroy(testParticles->view));
	dsScene_destroy(testParticles->scene);

	dsSceneNode_freeRef((dsSceneNode*)testParticles->rootNode);
	for (unsigned int i = 0; i < DS_ARRAY_SIZE(testParticles->rotatingTorches); ++i)
		dsSceneNode_freeRef((dsSceneNode*)testParticles->rotatingTorches[i]);
	dsSceneNode_freeRef(testParticles->staticTorch);
	dsSceneNode_freeRef(testParticles->staticTorchLight);

	dsSceneResources_freeRef(testParticles->sceneGraph);
	dsSceneResources_freeRef(testParticles->materials);
	dsSceneResources_freeRef(testParticles->baseResources);
	dsSceneResources_freeRef(testParticles->builtinResources);
	DS_VERIFY(dsWindow_destroy(testParticles->window));
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

	DS_LOG_INFO_F("TestParticles", "Render using %s", dsRenderBootstrap_rendererName(rendererType));
	DS_LOG_INFO("TestParticles", "Press space to pause/unpause.");
	DS_LOG_INFO("TestParticles", "Press 1 to extinguish/light the middle torch.");
	DS_LOG_INFO("TestParticles", "Press 2 to toggle one of the moving torches.");

	dsSystemAllocator renderAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&renderAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator applicationAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&applicationAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator testParticlesAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&testParticlesAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsRendererOptions rendererOptions;
	dsRenderer_defaultOptions(&rendererOptions, "TestParticles", 0);
	rendererOptions.depthBits = 0;
	rendererOptions.stencilBits = 0;
	rendererOptions.surfaceSamples = 1;
	rendererOptions.reverseZ = true;
	rendererOptions.preferHalfDepthRange = true;
	rendererOptions.deviceName = deviceName;
	dsRenderer* renderer = dsRenderBootstrap_createRenderer(rendererType,
		(dsAllocator*)&renderAllocator, &rendererOptions);
	if (!renderer)
	{
		DS_LOG_ERROR_F("TestParticles", "Couldn't create renderer: %s", dsErrorString(errno));
		return 2;
	}

	dsRenderer_setVsync(renderer, true);
	dsRenderer_setDefaultAnisotropy(renderer, dsMin(4.0f, renderer->maxAnisotropy));
#if DS_DEBUG
	dsRenderer_setExtraDebugging(renderer, true);
#endif

	dsApplication* application = dsSDLApplication_create((dsAllocator*)&applicationAllocator,
		renderer, argc, argv, "DeepSea", "TestParticles", dsSDLApplicationFlags_None);
	if (!application)
	{
		DS_LOG_ERROR_F("TestParticles", "Couldn't create application: %s", dsErrorString(errno));
		dsRenderer_destroy(renderer);
		return 2;
	}

	char assetsPath[DS_PATH_MAX];
	DS_VERIFY(dsPath_combine(assetsPath, sizeof(assetsPath), dsResourceStream_getEmbeddedDir(),
		"TestParticles-assets"));
	dsResourceStream_setEmbeddedDir(assetsPath);

	TestParticles testParticles;
	memset(&testParticles, 0, sizeof(testParticles));
	if (!setup(&testParticles, application, (dsAllocator*)&testParticlesAllocator))
	{
		shutdown(&testParticles);
		return 3;
	}

	int exitCode = dsApplication_run(application);

	shutdown(&testParticles);
	dsSDLApplication_destroy(application);
	dsRenderer_destroy(renderer);

	if (!validateAllocator((dsAllocator*)&renderAllocator, "render"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&applicationAllocator, "application"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&testParticlesAllocator, "TestParticles"))
		exitCode = 4;

	return exitCode;
}

/*
 * Copyright 2023 Aaron Barany
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

#include <DeepSea/Animation/Animation.h>
#include <DeepSea/Animation/DirectAnimation.h>
#include <DeepSea/Animation/KeyframeAnimation.h>

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

#include <DeepSea/SceneAnimation/SceneAnimationLoadContext.h>
#include <DeepSea/SceneAnimation/SceneAnimationNode.h>
#include <DeepSea/SceneAnimation/SceneDirectAnimation.h>
#include <DeepSea/SceneAnimation/SceneKeyframeAnimation.h>
#include <DeepSea/SceneAnimation/SceneSkinningData.h>

#include <DeepSea/SceneLighting/InstanceForwardLightData.h>
#include <DeepSea/SceneLighting/SceneLightingLoadContext.h>
#include <DeepSea/SceneLighting/SceneLightNode.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if DS_HAS_EASY_PROFILER
#include <DeepSea/EasyProfiler/EasyProfiler.h>
#endif

#define HOLD_TORCH_WEIGHT 50.0f
#define ACTIVE_WEIGHT 1.0f
#define IDLE_SPEED 0.0f
#define IDLE_SCALE 1.0f
#define WALK_SPEED 1.0f
#define WALK_SCALE 1.5f
#define RUN_SPEED 2.0f
#define RUN_SCALE 2.0f
#define UPDATE_STEP (1.0f/60.0f)

typedef struct AnimationState
{
	dsAnimation* animation;
	float speed;
	float targetSpeed;
} AnimationState;

typedef struct TestAnimation
{
	dsAllocator* allocator;
	dsRenderer* renderer;
	dsWindow* window;
	dsSceneResources* builtinResources;
	dsSceneResources* baseResources;
	dsSceneResources* skinMaterials;
	dsSceneResources* materials;
	dsSceneResources* sceneGraph;
	dsScene* scene;
	dsView* view;

	dsKeyframeAnimation* idleAnimation;
	dsKeyframeAnimation* walkAnimation;
	dsKeyframeAnimation* runAnimation;
	dsDirectAnimation* holdTorchAnimation;
	AnimationState characterAnimations[2];

	uint32_t fingerCount;
	uint32_t maxFingers;
	bool ignoreTime;
} TestAnimation;

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

	DS_LOG_ERROR_F("TestAnimation", "Allocator '%s' has %llu bytes allocated with %u allocations.",
		name, (unsigned long long)allocator->size, allocator->currentAllocations);
	return false;
}

static void cycleSpeed(AnimationState* state)
{
	if (state->targetSpeed == IDLE_SPEED)
		state->targetSpeed = WALK_SPEED;
	else if (state->targetSpeed == WALK_SPEED)
		state->targetSpeed = RUN_SPEED;
	else
		state->targetSpeed = IDLE_SPEED;
}

static bool processEvent(dsApplication* application, dsWindow* window, const dsEvent* event,
	void* userData)
{
	TestAnimation* testAnimation = (TestAnimation*)userData;
	DS_ASSERT(!window || window == testAnimation->window);
	switch (event->type)
	{
		case dsAppEventType_WindowClosed:
			DS_VERIFY(dsWindow_destroy(window));
			testAnimation->window = NULL;
			return false;
		case dsAppEventType_SurfaceInvalidated:
			DS_VERIFY(dsView_setSurface(testAnimation->view, "windowColor",
				testAnimation->window->surface, dsGfxSurfaceType_ColorRenderSurface));
			DS_VERIFY(dsView_setSurface(testAnimation->view, "windowDepth",
				testAnimation->window->surface, dsGfxSurfaceType_DepthRenderSurface));
			// Fall through
		case dsAppEventType_WindowResized:
			DS_VERIFY(dsView_setDimensions(testAnimation->view,
				testAnimation->window->surface->width, testAnimation->window->surface->height,
				testAnimation->window->surface->rotation));
			// Need to update the view again if the surfaces have been set.
			if (event->type == dsAppEventType_SurfaceInvalidated)
				dsView_update(testAnimation->view);
			return true;
		case dsAppEventType_WillEnterForeground:
			testAnimation->ignoreTime = true;
			return true;
		case dsAppEventType_KeyDown:
			if (event->key.repeat)
				return false;

			if (event->key.key == dsKeyCode_ACBack)
				dsApplication_quit(application, 0);
			else if (event->key.key == dsKeyCode_1)
				cycleSpeed(testAnimation->characterAnimations);
			else if (event->key.key == dsKeyCode_2)
				cycleSpeed(testAnimation->characterAnimations + 1);
			return false;
		case dsAppEventType_TouchFingerDown:
			++testAnimation->fingerCount;
			testAnimation->maxFingers =
				dsMax(testAnimation->fingerCount, testAnimation->maxFingers);
			return true;
		case dsAppEventType_TouchFingerUp:
			if (testAnimation->fingerCount == 0)
				return true;

			--testAnimation->fingerCount;
			if (testAnimation->fingerCount == 0)
			{
				switch (testAnimation->maxFingers)
				{
					case 1:
					case 2:
						cycleSpeed(
							testAnimation->characterAnimations + testAnimation->maxFingers - 1);
						break;
					default:
						break;
				}
				testAnimation->maxFingers = 0;
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

	TestAnimation* testAnimation = (TestAnimation*)userData;
	for (uint32_t i = 0; i < DS_ARRAY_SIZE(testAnimation->characterAnimations); ++i)
	{
		AnimationState* animationState = testAnimation->characterAnimations + i;
		if (animationState->speed == animationState->targetSpeed)
			continue;

		if (animationState->speed < animationState->targetSpeed)
		{
			animationState->speed += UPDATE_STEP;
			animationState->speed = dsMin(animationState->speed, animationState->targetSpeed);
		}
		else
		{
			animationState->speed -= UPDATE_STEP;
			animationState->speed = dsMax(animationState->speed, animationState->targetSpeed);
		}

		float idleWeight, walkWeight, runWeight, timeScale;
		if (animationState->speed < WALK_SPEED)
		{
			float t = (animationState->speed - IDLE_SPEED)/(WALK_SPEED - IDLE_SPEED);
			idleWeight = (1.0f - t)*ACTIVE_WEIGHT;
			walkWeight = t*ACTIVE_WEIGHT;
			runWeight = 0.0f;
			timeScale = dsLerp(IDLE_SCALE, WALK_SCALE, t);
		}
		else
		{
			float t = (animationState->speed - WALK_SPEED)/(RUN_SPEED - WALK_SPEED);
			idleWeight = 0.0f;
			walkWeight = (1.0f - t)*ACTIVE_WEIGHT;
			runWeight = t*ACTIVE_WEIGHT;
			timeScale = dsLerp(WALK_SCALE, RUN_SCALE, t);
		}

		dsKeyframeAnimationEntry* idleEntry = dsAnimation_findKeyframeAnimationEntry(
			animationState->animation, testAnimation->idleAnimation);
		DS_ASSERT(idleEntry);
		idleEntry->timeScale = timeScale;
		idleEntry->weight = idleWeight;
		if (animationState->speed >= WALK_SPEED)
			idleEntry->time = 0;

		dsKeyframeAnimationEntry* walkEntry = dsAnimation_findKeyframeAnimationEntry(
			animationState->animation, testAnimation->walkAnimation);
		DS_ASSERT(idleEntry);
		walkEntry->timeScale = timeScale;
		walkEntry->weight = walkWeight;
		if (animationState->speed == IDLE_SPEED)
			walkEntry->time = 0;

		dsKeyframeAnimationEntry* runEntry = dsAnimation_findKeyframeAnimationEntry(
			animationState->animation, testAnimation->runAnimation);
		DS_ASSERT(idleEntry);
		runEntry->timeScale = timeScale;
		runEntry->weight = runWeight;
		if (animationState->speed == IDLE_SPEED)
			runEntry->time = 0;
	}

	DS_VERIFY(dsScene_update(testAnimation->scene, lastFrameTime));
	DS_VERIFY(dsView_update(testAnimation->view));
}

static void draw(dsApplication* application, dsWindow* window, void* userData)
{
	DS_UNUSED(application);
	DS_UNUSED(window);
	TestAnimation* testAnimation = (TestAnimation*)userData;
	DS_ASSERT(testAnimation->window == window);
	dsRenderer* renderer = testAnimation->renderer;
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	DS_VERIFY(dsView_draw(testAnimation->view, commandBuffer, NULL));
}

static bool setup(TestAnimation* testAnimation, dsApplication* application, dsAllocator* allocator)
{
	dsRenderer* renderer = application->renderer;
	dsResourceManager* resourceManager = renderer->resourceManager;
	testAnimation->allocator = allocator;
	testAnimation->renderer = renderer;

	dsEventResponder responder = {&processEvent, testAnimation, 0, 0};
	DS_VERIFY(dsApplication_addEventResponder(application, &responder));
	DS_VERIFY(dsApplication_setUpdateFunction(application, &update, testAnimation));

	uint32_t width = dsApplication_adjustWindowSize(application, 0, 800);
	uint32_t height = dsApplication_adjustWindowSize(application, 0, 600);
	testAnimation->window = dsWindow_create(application, allocator, "Test Animation", NULL,
		NULL, width, height, dsWindowFlags_Resizeable | dsWindowFlags_DelaySurfaceCreate,
		dsRenderSurfaceUsage_ClientRotations);
	if (!testAnimation->window)
	{
		DS_LOG_ERROR_F("TestAnimation", "Couldn't create window: %s", dsErrorString(errno));
		return false;
	}

	if (DS_ANDROID || DS_IOS)
		dsWindow_setStyle(testAnimation->window, dsWindowStyle_FullScreen);

	if (!dsWindow_createSurface(testAnimation->window))
	{
		DS_LOG_ERROR_F("TestAnimation", "Couldn't create window surface: %s", dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsWindow_setDrawFunction(testAnimation->window, &draw, testAnimation));

	dsSceneLoadContext* loadContext = dsSceneLoadContext_create(allocator, renderer);
	if (!loadContext)
	{
		DS_LOG_ERROR_F("TestAnimation", "Couldn't create load context: %s", dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsSceneLightingLoadConext_registerTypes(loadContext));
	DS_VERIFY(dsSceneAnimationLoadConext_registerTypes(loadContext));

	dsSceneLoadScratchData* scratchData = dsSceneLoadScratchData_create(allocator,
		renderer->mainCommandBuffer);
	if (!scratchData)
	{
		DS_LOG_ERROR_F("TestAnimation", "Couldn't create load scratch data: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		return false;
	}

	testAnimation->builtinResources = dsSceneResources_create(allocator, 5);
	if (!testAnimation->builtinResources)
	{
		DS_LOG_ERROR_F("TestAnimation", "Couldn't create scene resources: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	dsShaderVariableGroupDesc* groupDesc =
		dsInstanceTransformData_createShaderVariableGroupDesc(resourceManager, allocator);
	if (!groupDesc)
	{
		DS_LOG_ERROR_F("TestAnimation",
			"Couldn't create instance transform shader variable desc: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneResources_addResource(testAnimation->builtinResources,
		"instanceTransformDesc", dsSceneResourceType_ShaderVariableGroupDesc, groupDesc, true));

	groupDesc = dsViewTransformData_createShaderVariableGroupDesc(resourceManager, allocator);
	if (!groupDesc)
	{
		DS_LOG_ERROR_F("TestAnimation", "Couldn't create view transform shader variable desc: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneResources_addResource(testAnimation->builtinResources,
		"viewTransformDesc", dsSceneResourceType_ShaderVariableGroupDesc, groupDesc, true));

	groupDesc = dsInstanceForwardLightData_createShaderVariableGroupDesc(resourceManager, allocator,
		DS_DEFAULT_FORWARD_LIGHT_COUNT);
	if (!groupDesc)
	{
		DS_LOG_ERROR_F("TestAnimation",
			"Couldn't create instance forward light shader variable desc: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneResources_addResource(testAnimation->builtinResources,
		"instanceForwardLightDesc", dsSceneResourceType_ShaderVariableGroupDesc, groupDesc, true));

	groupDesc = dsSceneSkinningData_createTextureInfoShaderVariableGroupDesc(resourceManager,
		allocator);
	if (!groupDesc)
	{
		DS_LOG_ERROR_F("TestAnimation",
			"Couldn't create scene skinning data texture info shader variable desc: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneResources_addResource(testAnimation->builtinResources,
		"skinningTextureInfoDesc", dsSceneResourceType_ShaderVariableGroupDesc, groupDesc, true));

	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData,
		&testAnimation->builtinResources, 1));

	testAnimation->baseResources = dsSceneResources_loadResource(allocator, NULL, loadContext,
		scratchData, dsFileResourceType_Embedded, "BaseResources.dssr");
	if (!testAnimation->baseResources)
	{
		DS_LOG_ERROR_F("TestAnimation", "Couldn't load base scene resources: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(scratchData,
		&testAnimation->baseResources, 1));

	if (dsSceneSkinningData_useBuffers(resourceManager))
	{
		testAnimation->skinMaterials = dsSceneResources_loadResource(allocator, NULL, loadContext,
			scratchData, dsFileResourceType_Embedded, "SkinBufferMaterials.dssr");
	}
	else
	{
		testAnimation->skinMaterials = dsSceneResources_loadResource(allocator, NULL, loadContext,
			scratchData, dsFileResourceType_Embedded, "SkinTextureMaterials.dssr");
	}
	if (!testAnimation->skinMaterials)
	{
		DS_LOG_ERROR_F("TestAnimation", "Couldn't load skin material scene resources: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(
		scratchData, &testAnimation->skinMaterials, 1));

	testAnimation->materials = dsSceneResources_loadResource(allocator, NULL, loadContext,
		scratchData, dsFileResourceType_Embedded, "Materials.dssr");
	if (!testAnimation->materials)
	{
		DS_LOG_ERROR_F("TestAnimation", "Couldn't load material scene resources: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(
		scratchData, &testAnimation->materials, 1));

	testAnimation->sceneGraph = dsSceneResources_loadResource(allocator, NULL, loadContext,
		scratchData, dsFileResourceType_Embedded, "SceneGraph.dssr");
	if (!testAnimation->baseResources)
	{
		DS_LOG_ERROR_F("TestAnimation", "Couldn't load scene graph: %s",
			dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}
	DS_VERIFY(dsSceneLoadScratchData_pushSceneResources(
		scratchData, &testAnimation->sceneGraph, 1));

	testAnimation->scene = dsScene_loadResource(allocator, NULL, loadContext, scratchData, NULL,
		NULL, dsFileResourceType_Embedded, "Scene.dss");
	if (!testAnimation->scene)
	{
		DS_LOG_ERROR_F("TestAnimation", "Couldn't load scene: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	dsRenderSurface* surface = testAnimation->window->surface;
	dsViewSurfaceInfo viewSurfaces[2];
	viewSurfaces[0].name = "windowColor";
	viewSurfaces[0].surfaceType = dsGfxSurfaceType_ColorRenderSurface;
	viewSurfaces[0].surface = surface;
	viewSurfaces[0].windowFramebuffer = true;
	viewSurfaces[1].name = "windowDepth";
	viewSurfaces[1].surfaceType = dsGfxSurfaceType_DepthRenderSurface;
	viewSurfaces[1].surface = surface;
	viewSurfaces[1].windowFramebuffer = true;

	testAnimation->view = dsView_loadResource(testAnimation->scene, allocator, NULL, scratchData,
		viewSurfaces, DS_ARRAY_SIZE(viewSurfaces), surface->width, surface->height,
		surface->rotation, NULL, NULL, dsFileResourceType_Embedded, "View.dsv");
	if (!testAnimation->view)
	{
		DS_LOG_ERROR_F("TestAnimation", "Couldn't load view: %s", dsErrorString(errno));
		dsSceneLoadContext_destroy(loadContext);
		dsSceneLoadScratchData_destroy(scratchData);
		return false;
	}

	dsSceneLoadContext_destroy(loadContext);
	dsSceneLoadScratchData_destroy(scratchData);

	dsSceneResourceType curType;
	dsCustomSceneResource* customResource;
	if (!dsSceneResources_findResource(&curType, (void**)&customResource,
			testAnimation->baseResources, "idleAnimation") ||
		curType != dsSceneResourceType_Custom ||
		customResource->type != dsSceneKeyframeAnimation_type())
	{
		DS_LOG_ERROR("TestAnimation", "Couldn't find idleAnimation.");
		return false;
	}
	testAnimation->idleAnimation = (dsKeyframeAnimation*)customResource->resource;

	if (!dsSceneResources_findResource(&curType, (void**)&customResource,
			testAnimation->baseResources, "walkAnimation") ||
		curType != dsSceneResourceType_Custom ||
		customResource->type != dsSceneKeyframeAnimation_type())
	{
		DS_LOG_ERROR("TestAnimation", "Couldn't find walkAnimation.");
		return false;
	}
	testAnimation->walkAnimation = (dsKeyframeAnimation*)customResource->resource;

	if (!dsSceneResources_findResource(&curType, (void**)&customResource,
			testAnimation->baseResources, "runAnimation") ||
		curType != dsSceneResourceType_Custom ||
		customResource->type != dsSceneKeyframeAnimation_type())
	{
		DS_LOG_ERROR("TestAnimation", "Couldn't find runAnimation.");
		return false;
	}
	testAnimation->runAnimation = (dsKeyframeAnimation*)customResource->resource;

	if (!dsSceneResources_findResource(&curType, (void**)&customResource,
			testAnimation->baseResources, "holdTorchAnimation") ||
		curType != dsSceneResourceType_Custom ||
		customResource->type != dsSceneDirectAnimation_type())
	{
		DS_LOG_ERROR("TestAnimation", "Couldn't find holdTorchAnimation.");
		return false;
	}
	testAnimation->holdTorchAnimation = (dsDirectAnimation*)customResource->resource;
	dsSceneNode* animationNode;
	if (!dsSceneResources_findResource(&curType, (void**)&animationNode, testAnimation->sceneGraph,
			"characterAnimationNode") ||
		curType != dsSceneResourceType_SceneNode ||
		!dsSceneNode_isOfType(animationNode, dsSceneAnimationNode_type()))
	{
		DS_LOG_ERROR("TestAnimation", "Couldn't find characterAnimationNode.");
		return false;
	}

	const char* nodeNames[] = {"firstCharacterNode", "secondCharacterNode"};
	_Static_assert(DS_ARRAY_SIZE(nodeNames) == DS_ARRAY_SIZE(testAnimation->characterAnimations),
		"Node array sizes don't match.");
	for (unsigned int i = 0; i < DS_ARRAY_SIZE(nodeNames); ++i)
	{
		AnimationState* animationState = testAnimation->characterAnimations + i;

		dsSceneNode* curNode;
		if (!dsSceneResources_findResource(&curType, (void**)&curNode, testAnimation->sceneGraph,
				nodeNames[i]) ||
			curType != dsSceneResourceType_SceneNode)
		{
			DS_LOG_ERROR_F("TestAnimation", "Couldn't find %s.", nodeNames[i]);
			return false;
		}

		dsSceneTreeNode* animationTreeNode = dsSceneNode_findUniqueTreeNode(curNode, animationNode);
		if (!animationTreeNode)
		{
			DS_LOG_ERROR_F("TestAnimation", "Node %s isn't unique in the scene graph.",
				nodeNames[i]);
			return false;
		}

		animationState->animation = dsSceneAnimationNode_getAnimationForInstance(animationTreeNode);
		if (!animationState)
		{
			DS_LOG_ERROR_F("TestAnimation",
				"Couldn't find animation for characterAnimationNode under %s.", nodeNames[i]);
			return false;
		}
		animationState->speed = IDLE_SPEED;
		animationState->targetSpeed = IDLE_SPEED;

		if (i == 0)
		{
			if (!dsAnimation_addDirectAnimation(animationState->animation,
					testAnimation->holdTorchAnimation, HOLD_TORCH_WEIGHT))
			{
				DS_LOG_ERROR_F("TestAnimation", "Couldn't add holdTorchAnimation under %s.",
					nodeNames[i]);
				return false;
			}
		}

		if (!dsAnimation_addKeyframeAnimation(animationState->animation,
				testAnimation->idleAnimation, ACTIVE_WEIGHT, 0.0f, IDLE_SCALE, true))
		{
			DS_LOG_ERROR_F("TestAnimation", "Couldn't add idleAnimation under %s.", nodeNames[i]);
			return false;
		}

		if (!dsAnimation_addKeyframeAnimation(animationState->animation,
				testAnimation->walkAnimation, 0.0f, 0.0f, IDLE_SCALE, true))
		{
			DS_LOG_ERROR_F("TestAnimation", "Couldn't add walkAnimation under %s.", nodeNames[i]);
			return false;
		}

		if (!dsAnimation_addKeyframeAnimation(animationState->animation,
				testAnimation->runAnimation, 0.0f, 0.0f, IDLE_SCALE, true))
		{
			DS_LOG_ERROR_F("TestAnimation", "Couldn't add runAnimation under %s.", nodeNames[i]);
			return false;
		}
	}

	dsView_setPerspectiveProjection(testAnimation->view, dsDegreesToRadiansf(45.0f), 0.1f, 100.0f);

	dsVector3f eyePos = {{0.0f, -5.0f, 3.0f}};
	dsVector3f lookAtPos = {{0.0f, 0.0f, 0.0f}};
	dsVector3f upDir = {{0.0f, 0.0f, 1.0f}};
	dsMatrix44f camera;
	dsMatrix44f_lookAt(&camera, &eyePos, &lookAtPos, &upDir);
	dsView_setCameraMatrix(testAnimation->view, &camera);

	return true;
}

static void shutdown(TestAnimation* testAnimation)
{
	DS_VERIFY(dsView_destroy(testAnimation->view));
	dsScene_destroy(testAnimation->scene);

	dsSceneResources_freeRef(testAnimation->sceneGraph);
	dsSceneResources_freeRef(testAnimation->skinMaterials);
	dsSceneResources_freeRef(testAnimation->materials);
	dsSceneResources_freeRef(testAnimation->baseResources);
	dsSceneResources_freeRef(testAnimation->builtinResources);
	DS_VERIFY(dsWindow_destroy(testAnimation->window));
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

	DS_LOG_INFO_F("TestAnimation", "Render using %s", dsRenderBootstrap_rendererName(rendererType));
	DS_LOG_INFO("TestAnimation", "Press 1 to cycle animations for the first character.");
	DS_LOG_INFO("TestAnimation", "Press 2 to cycle animations for the second character.");

	dsSystemAllocator renderAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&renderAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator applicationAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&applicationAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator testAnimationAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&testAnimationAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsRendererOptions rendererOptions;
	dsRenderer_defaultOptions(&rendererOptions, "TestAnimation", 0);
	rendererOptions.depthBits = 32;
	rendererOptions.stencilBits = 0;
	rendererOptions.surfaceSamples = 4;
	rendererOptions.reverseZ = true;
	rendererOptions.preferHalfDepthRange = true;
	rendererOptions.deviceName = deviceName;
	dsRenderer* renderer = dsRenderBootstrap_createRenderer(rendererType,
		(dsAllocator*)&renderAllocator, &rendererOptions);
	if (!renderer)
	{
		DS_LOG_ERROR_F("TestAnimation", "Couldn't create renderer: %s", dsErrorString(errno));
		return 2;
	}

	dsRenderer_setVsync(renderer, true);
	dsRenderer_setDefaultAnisotropy(renderer, dsMin(4.0f, renderer->maxAnisotropy));
#if DS_DEBUG
	dsRenderer_setExtraDebugging(renderer, true);
#endif

	dsApplication* application = dsSDLApplication_create((dsAllocator*)&applicationAllocator,
		renderer, argc, argv, "DeepSea", "TestAnimation", dsSDLApplicationFlags_None);
	if (!application)
	{
		DS_LOG_ERROR_F("TestAnimation", "Couldn't create application: %s", dsErrorString(errno));
		dsRenderer_destroy(renderer);
		return 2;
	}

	char assetsPath[DS_PATH_MAX];
	DS_VERIFY(dsPath_combine(assetsPath, sizeof(assetsPath), dsResourceStream_getEmbeddedDir(),
		"TestAnimation-assets"));
	dsResourceStream_setEmbeddedDir(assetsPath);

	TestAnimation testAnimation;
	memset(&testAnimation, 0, sizeof(testAnimation));
	if (!setup(&testAnimation, application, (dsAllocator*)&testAnimationAllocator))
	{
		shutdown(&testAnimation);
		return 3;
	}

	int exitCode = dsApplication_run(application);

	shutdown(&testAnimation);
	dsSDLApplication_destroy(application);
	dsRenderer_destroy(renderer);

	if (!validateAllocator((dsAllocator*)&renderAllocator, "render"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&applicationAllocator, "application"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&testAnimationAllocator, "TestAnimation"))
		exitCode = 4;

	return exitCode;
}

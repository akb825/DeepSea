/*
 * Copyright 2019 Aaron Barany
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
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>

#include <DeepSea/Render/Resources/DrawGeometry.h>
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
#include <DeepSea/RenderBootstrap/RenderBootstrap.h>

#include <DeepSea/Scene/ItemLists/InstanceTransformData.h>
#include <DeepSea/Scene/ItemLists/ViewCullList.h>
#include <DeepSea/Scene/ItemLists/SceneInstanceData.h>
#include <DeepSea/Scene/ItemLists/SceneItemList.h>
#include <DeepSea/Scene/ItemLists/SceneModelList.h>
#include <DeepSea/Scene/Nodes/SceneModelNode.h>
#include <DeepSea/Scene/Nodes/SceneNode.h>
#include <DeepSea/Scene/Nodes/SceneTransformNode.h>
#include <DeepSea/Scene/Scene.h>
#include <DeepSea/Scene/SceneGlobalData.h>
#include <DeepSea/Scene/SceneRenderPass.h>
#include <DeepSea/Scene/SceneResources.h>
#include <DeepSea/Scene/View.h>
#include <DeepSea/Scene/ViewTransformData.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

	uint64_t invalidatedFrame;
	float rotation;
} TestScene;

static const char* assetsDir = "TestScene-assets";
static char shaderDir[100];

typedef struct Vertex
{
	dsVector3f position;
	dsVector3f normal;
} Vertex;

static Vertex vertices[] =
{
	// Front face
	{ {{-1.0f,  1.0f,  1.0f}}, {{0.0f, 0.0f, 1.0f}} },
	{ {{ 1.0f,  1.0f,  1.0f}}, {{0.0f, 0.0f, 1.0f}} },
	{ {{ 1.0f, -1.0f,  1.0f}}, {{0.0f, 0.0f, 1.0f}} },
	{ {{-1.0f, -1.0f,  1.0f}}, {{0.0f, 0.0f, 1.0f}} },

	// Right face
	{ {{ 1.0f,  1.0f,  1.0f}}, {{1.0f, 0.0f, 0.0f}} },
	{ {{ 1.0f,  1.0f, -1.0f}}, {{1.0f, 0.0f, 0.0f}} },
	{ {{ 1.0f, -1.0f, -1.0f}}, {{1.0f, 0.0f, 0.0f}} },
	{ {{ 1.0f, -1.0f,  1.0f}}, {{1.0f, 0.0f, 0.0f}} },

	// Back face
	{ {{ 1.0f,  1.0f, -1.0f}}, {{0.0f, 0.0f, -1.0f}} },
	{ {{-1.0f,  1.0f, -1.0f}}, {{0.0f, 0.0f, -1.0f}} },
	{ {{-1.0f, -1.0f, -1.0f}}, {{0.0f, 0.0f, -1.0f}} },
	{ {{ 1.0f, -1.0f, -1.0f}}, {{0.0f, 0.0f, -1.0f}} },

	// Left face
	{ {{-1.0f,  1.0f, -1.0f}}, {{-1.0f, 0.0f, 0.0f}} },
	{ {{-1.0f,  1.0f,  1.0f}}, {{-1.0f, 0.0f, 0.0f}} },
	{ {{-1.0f, -1.0f,  1.0f}}, {{-1.0f, 0.0f, 0.0f}} },
	{ {{-1.0f, -1.0f, -1.0f}}, {{-1.0f, 0.0f, 0.0f}} },

	// Top face
	{ {{-1.0f,  1.0f, -1.0f}}, {{0.0f, 1.0f, 0.0f}} },
	{ {{ 1.0f,  1.0f, -1.0f}}, {{0.0f, 1.0f, 0.0f}} },
	{ {{ 1.0f,  1.0f,  1.0f}}, {{0.0f, 1.0f, 0.0f}} },
	{ {{-1.0f,  1.0f,  1.0f}}, {{0.0f, 1.0f, 0.0f}} },

	// Bottom face
	{ {{-1.0f, -1.0f,  1.0f}}, {{0.0f, -1.0f, 0.0f}} },
	{ {{ 1.0f, -1.0f,  1.0f}}, {{0.0f, -1.0f, 0.0f}} },
	{ {{ 1.0f, -1.0f, -1.0f}}, {{0.0f, -1.0f, 0.0f}} },
	{ {{-1.0f, -1.0f, -1.0f}}, {{0.0f, -1.0f, 0.0f}} },
};

static uint16_t indices[] =
{
	// Front face
	0, 2, 1,
	2, 0, 3,

	// Right face
	4, 6, 5,
	6, 4, 7,

	// Back face
	8, 10, 9,
	10, 8, 11,

	// Left face
	12, 14, 13,
	14, 12, 15,

	// Top face
	16, 18, 17,
	18, 16, 19,

	// Bottom face
	20, 22, 21,
	22, 20, 23,
};

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
}

static bool validateAllocator(dsAllocator* allocator, const char* name)
{
	if (allocator->size == 0)
		return true;

	DS_LOG_ERROR_F("TestScene", "Allocator '%s' has %llu bytes allocated with %u allocations.",
		name, (unsigned long long)allocator->size, allocator->currentAllocations);
	return false;
}

static bool processEvent(dsApplication* application, dsWindow* window, const dsEvent* event,
	void* userData)
{
	TestScene* testScene = (TestScene*)userData;
	dsRenderer* renderer = application->renderer;
	DS_ASSERT(!window || window == testScene->window);
	switch (event->type)
	{
		case dsEventType_WindowClosed:
			DS_VERIFY(dsWindow_destroy(window));
			testScene->window = NULL;
			return false;
		case dsEventType_SurfaceInvalidated:
			dsView_setSurface(testScene->view, "windowColor", testScene->window->surface,
				dsGfxSurfaceType_ColorRenderSurface);
			dsView_setSurface(testScene->view, "windowDepth", testScene->window->surface,
				dsGfxSurfaceType_DepthRenderSurface);
			testScene->invalidatedFrame = renderer->frameNumber;
			// Fall through
		case dsEventType_WindowResized:
			dsView_setDimensions(testScene->view, testScene->window->surface->width,
				testScene->window->surface->height);
			return true;
		case dsEventType_KeyDown:
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
				dsRenderer_setSurfaceSamples(renderer, samples);
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

	dsMatrix44f_makeRotate(&transform, 0, -testScene->rotation, 0);
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

	DS_VERIFY(dsView_draw(testScene->view, commandBuffer, NULL));
}

static dsSceneResources* createSceneResources(dsRenderer* renderer, dsAllocator* allocator)
{
	dsResourceManager* resourceManager = renderer->resourceManager;

	dsShaderVariableGroupDesc* instanceTransformDesc = NULL;
	dsShaderVariableGroupDesc* viewTransformDesc = NULL;
	dsShaderVariableGroupDesc* lightDesc = NULL;
	dsMaterialDesc* materialDesc = NULL;
	dsMaterial* materials[] = {NULL, NULL, NULL};
	dsShaderModule* shaderModule = NULL;
	dsShader* shader = NULL;
	dsGfxBuffer* drawBuffer = NULL;
	dsDrawGeometry* geometry = NULL;

	uint32_t resourceCount = 0;

	++resourceCount;
	instanceTransformDesc =
		dsInstanceTransformData_createShaderVariableGroupDesc(resourceManager, allocator);
	if (!instanceTransformDesc)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create instance transform description: %s",
			dsErrorString(errno));
		goto fail;
	}

	++resourceCount;
	viewTransformDesc =
		dsViewTransformData_createShaderVariableGroupDesc(resourceManager, allocator);
	if (!viewTransformDesc)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create view transform description: %s",
			dsErrorString(errno));
		goto fail;
	}

	++resourceCount;
	lightDesc = dsViewTransformData_createShaderVariableGroupDesc(resourceManager, allocator);
	if (!lightDesc)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create light description: %s",
			dsErrorString(errno));
		goto fail;
	}

	++resourceCount;
	dsMaterialElement materialElems[] =
	{
		{dsInstanceTransformData_shaderVariableGroupName, dsMaterialType_VariableGroup, 0,
			instanceTransformDesc, dsMaterialBinding_Instance, 0},
		{dsViewTransformData_shaderVariableGroupName, dsMaterialType_VariableGroup, 0,
			viewTransformDesc, dsMaterialBinding_Global, 0},
		{"Light", dsMaterialType_VariableGroup, 0, lightDesc, dsMaterialBinding_Global, 0},
		{"materialColor", dsMaterialType_Vec4, 0, NULL, dsMaterialBinding_Material, 0}
	};
	materialDesc = dsMaterialDesc_create(resourceManager, allocator, materialElems,
		DS_ARRAY_SIZE(materialElems));
	if (!materialDesc)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create material description: %s",
			dsErrorString(errno));
		goto fail;
	}

	++resourceCount;
	uint32_t colorIndex = dsMaterialDesc_findElement(materialDesc, "materialColor");
	DS_ASSERT(colorIndex != DS_MATERIAL_UNKNOWN);

	for (uint32_t i = 0; i < DS_ARRAY_SIZE(materials); ++i)
	{
		++resourceCount;
		materials[i] = dsMaterial_create(resourceManager, allocator, materialDesc);
		if (!materials[i])
		{
			DS_LOG_ERROR_F("TestScene", "Couldn't create material: %s", dsErrorString(errno));
			goto fail;
		}

		dsVector4f color = {{0.0f, 0.0f, 0.0f, 1.0f}};
		color.values[i] = 1.0f;
		DS_VERIFY(dsMaterial_setElementData(materials[i], colorIndex, &color, dsMaterialType_Vec4,
			0, 1));
	}

	char path[DS_PATH_MAX];
	if (!dsPath_combine(path, sizeof(path), assetsDir, shaderDir) ||
		!dsPath_combine(path, sizeof(path), path, "TestScene.mslb"))
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create shader path: %s", dsErrorString(errno));
		goto fail;
	}

	++resourceCount;
	shaderModule = dsShaderModule_loadResource(resourceManager, allocator,
		dsFileResourceType_Embedded, path, "TestScene");
	if (!shaderModule)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't load shader: %s", dsErrorString(errno));
		goto fail;
	}

	++resourceCount;
	shader = dsShader_createName(resourceManager, allocator, shaderModule, "Default", materialDesc);
	if (!shader)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create shader: %s", dsErrorString(errno));
		goto fail;
	}

	++resourceCount;
	uint8_t combinedBufferData[sizeof(vertices) + sizeof(indices)];
	memcpy(combinedBufferData, vertices, sizeof(vertices));
	memcpy(combinedBufferData + sizeof(vertices), indices, sizeof(indices));
	drawBuffer = dsGfxBuffer_create(resourceManager, allocator,
		dsGfxBufferUsage_Vertex | dsGfxBufferUsage_Index,
		dsGfxMemory_Static | dsGfxMemory_Draw | dsGfxMemory_GPUOnly, combinedBufferData,
		sizeof(combinedBufferData));
	if (!drawBuffer)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create graphics buffer: %s", dsErrorString(errno));
		goto fail;
	}

	dsVertexFormat vertexFormat;
	DS_VERIFY(dsVertexFormat_initialize(&vertexFormat));
	vertexFormat.elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Position, true));
	vertexFormat.elements[dsVertexAttrib_Normal].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Normal, true));
	DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(&vertexFormat));
	DS_ASSERT(vertexFormat.size == sizeof(Vertex));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_Position].offset == offsetof(Vertex, position));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_Normal].offset == offsetof(Vertex, normal));

	++resourceCount;
	dsVertexBuffer vertexBuffer = {drawBuffer, 0, DS_ARRAY_SIZE(vertices), vertexFormat};
	dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS] = {&vertexBuffer, NULL, NULL,
		NULL};
	dsIndexBuffer indexBuffer = {drawBuffer, sizeof(vertices), DS_ARRAY_SIZE(indices),
		(uint32_t)sizeof(uint16_t)};
	geometry = dsDrawGeometry_create(resourceManager, allocator, vertexBuffers, &indexBuffer);
	if (!geometry)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create geometry: %s", dsErrorString(errno));
		return false;
	}

	dsSceneResources* resources = dsSceneResources_create(allocator, resourceCount);
	if (!resources)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create scene resources: %s", dsErrorString(errno));
		goto fail;
	}

	DS_VERIFY(dsSceneResources_addResource(resources, "instanceTransformDesc",
		dsSceneResourceType_ShaderVariableGroupDesc, instanceTransformDesc, true));
	DS_VERIFY(dsSceneResources_addResource(resources, "viewTransformDesc",
		dsSceneResourceType_ShaderVariableGroupDesc, viewTransformDesc, true));
	DS_VERIFY(dsSceneResources_addResource(resources, "lightDesc",
		dsSceneResourceType_ShaderVariableGroupDesc, lightDesc, true));
	DS_VERIFY(dsSceneResources_addResource(resources, "materialDesc",
		dsSceneResourceType_MaterialDesc, materialDesc, true));
	DS_VERIFY(dsSceneResources_addResource(resources, "centerCubeMaterial",
		dsSceneResourceType_Material, materials[0], true));
	DS_VERIFY(dsSceneResources_addResource(resources, "outerCubeMaterial",
		dsSceneResourceType_Material, materials[1], true));
	DS_VERIFY(dsSceneResources_addResource(resources, "groundMaterial",
		dsSceneResourceType_Material, materials[2], true));
	DS_VERIFY(dsSceneResources_addResource(resources, "shaderModule",
		dsSceneResourceType_ShaderModule, shaderModule, true));
	DS_VERIFY(dsSceneResources_addResource(resources, "shader", dsSceneResourceType_Shader, shader,
		true));
	DS_VERIFY(dsSceneResources_addResource(resources, "drawBuffer", dsSceneResourceType_Buffer,
		drawBuffer, true));
	DS_VERIFY(dsSceneResources_addResource(resources, "geometry", dsSceneResourceType_DrawGeometry,
		geometry, true));
	return resources;

fail:
	for (uint32_t i = 0; i < DS_ARRAY_SIZE(materials); ++i)
		dsMaterial_destroy(materials[i]);
	dsMaterialDesc_destroy(materialDesc);
	dsShaderVariableGroupDesc_destroy(instanceTransformDesc);
	dsShaderVariableGroupDesc_destroy(viewTransformDesc);
	dsShaderVariableGroupDesc_destroy(lightDesc);
	dsShader_destroy(shader);
	dsShaderModule_destroy(shaderModule);
	dsDrawGeometry_destroy(geometry);
	dsGfxBuffer_destroy(drawBuffer);
	return NULL;
}

dsScene* createScene(dsRenderer* renderer, dsAllocator* allocator, dsSceneResources* resources)
{
	dsResourceManager* resourceManager = renderer->resourceManager;

	dsSceneInstanceData* instanceTransformData = NULL;
	dsSceneItemList* cullList = NULL;
	dsSceneItemList* modelList = NULL;
	dsRenderPass* renderPass = NULL;
	dsSceneRenderPass* sceneRenderPass = NULL;
	dsSceneGlobalData* viewTransformData = NULL;

	dsSceneResourceType type;
	void* transformDesc;
	DS_VERIFY(dsSceneResources_findResource(&type, &transformDesc, resources,
		"instanceTransformDesc"));
	DS_ASSERT(type == dsSceneResourceType_ShaderVariableGroupDesc);
	instanceTransformData = dsInstanceTransformData_create(allocator, resourceManager,
		(const dsShaderVariableGroupDesc*)transformDesc);
	if (!instanceTransformData)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create instance transform data: %s",
			dsErrorString(errno));
		goto fail;
	}

	modelList = (dsSceneItemList*)dsSceneModelList_create(allocator, "main", &instanceTransformData,
		1, dsModelSortType_Material, NULL, dsViewCullList_cullID());

	// modelList took ownership
	instanceTransformData = NULL;

	if (!modelList)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create model list: %s", dsErrorString(errno));
		goto fail;
	}

	cullList = dsViewCullList_create(allocator, "cull");
	if (!cullList)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create cull list: %s", dsErrorString(errno));
		goto fail;
	}

	dsAttachmentInfo attachments[] =
	{
		{dsAttachmentUsage_Clear | dsAttachmentUsage_KeepAfter, renderer->surfaceColorFormat,
			DS_DEFAULT_ANTIALIAS_SAMPLES},
		{dsAttachmentUsage_Clear, renderer->surfaceDepthStencilFormat, DS_DEFAULT_ANTIALIAS_SAMPLES}
	};

	dsColorAttachmentRef colorAttachment = {0, true};
	uint32_t depthStencilAttachment = 1;
	dsRenderSubpassInfo subpass =
	{
		"TestScene", NULL, &colorAttachment, 0, 1, depthStencilAttachment
	};
	renderPass = dsRenderPass_create(renderer, allocator, attachments, 2, &subpass, 1, NULL, 0);
	if (!renderPass)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create render pass: %s", dsErrorString(errno));
		goto fail;
	}

	dsSurfaceClearValue clearValues[2];
	clearValues[0].colorValue.floatValue.r = 0.0f;
	clearValues[0].colorValue.floatValue.g = 0.1f;
	clearValues[0].colorValue.floatValue.b = 0.2f;
	clearValues[0].colorValue.floatValue.a = 1.0f;
	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;
	dsSubpassDrawLists subpassLists = {&modelList, 1};
	sceneRenderPass = dsSceneRenderPass_create(allocator, renderPass, "window", clearValues,
		DS_ARRAY_SIZE(clearValues), &subpassLists, 1);

	// sceneRenderPass took ownership
	renderPass = NULL;
	modelList = NULL;
	sceneRenderPass = NULL;

	if (!sceneRenderPass)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create scene render pass: %s", dsErrorString(errno));
		goto fail;
	}

	DS_VERIFY(dsSceneResources_findResource(&type, &transformDesc, resources,
		"viewTransformDesc"));
	DS_ASSERT(type == dsSceneResourceType_ShaderVariableGroupDesc);
	viewTransformData = dsViewTransformData_create(allocator, resourceManager,
		(const dsShaderVariableGroupDesc*)transformDesc);
	if (!viewTransformData)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create view transform data: %s",
			dsErrorString(errno));
		goto fail;
	}

	dsScenePipelineItem pipeline = {sceneRenderPass, NULL};
	dsScene* scene = dsScene_create(allocator, renderer, &cullList, 1, &pipeline, 1,
		&viewTransformData, 1, NULL, NULL);
	if (!scene)
		DS_LOG_ERROR_F("TestScene", "Couldn't create scene: %s", dsErrorString(errno));
	return scene;

fail:
	dsSceneInstanceData_destroy(instanceTransformData);
	dsSceneItemList_destroy(modelList);
	dsSceneItemList_destroy(cullList);
	dsSceneRenderPass_destroy(sceneRenderPass);
	dsRenderPass_destroy(renderPass);
	dsSceneGlobalData_destroy(viewTransformData);
	return NULL;
}

static dsView* createView(dsAllocator* allocator, dsScene* scene, dsRenderSurface* surface)
{
	dsViewSurfaceInfo surfaces[2];
	surfaces[0].name = "windowColor";
	surfaces[0].surfaceType = dsGfxSurfaceType_ColorRenderSurface;
	surfaces[0].surface = surface;
	surfaces[1].name = "windowDepth";
	surfaces[1].surfaceType = dsGfxSurfaceType_DepthRenderSurface;
	surfaces[1].surface = surface;

	dsFramebufferSurface framebufferSurfaces[2] =
	{
		{dsGfxSurfaceType_ColorRenderSurface, dsCubeFace_None, 0, 0, (void*)"windowColor"},
		{dsGfxSurfaceType_DepthRenderSurface, dsCubeFace_None, 0, 0, (void*)"windowDepth"}
	};
	dsViewFramebufferInfo framebuffer = {"window", framebufferSurfaces,
		DS_ARRAY_SIZE(framebufferSurfaces), -1.0f, -1.0f, 1,
		{{{0.0f, 0.0f, 0.0f}}, {{1.0f, 1.0f, 1.0f}}}};

	dsView* view = dsView_create(scene, allocator, surfaces, DS_ARRAY_SIZE(surfaces),
		&framebuffer, 1, surface->width, surface->height, NULL, NULL);
	if (!view)
	{
		DS_LOG_ERROR_F(DS_SCENE_LOG_TAG, "Couldn't create view: %s", dsErrorString(errno));
		return NULL;
	}

	dsVector3f eyePos = {{0.0f, 5.0f, 5.0f}};
	dsVector3f lookAtPos = {{0.0f, 0.0f, 0.0f}};
	dsVector3f upDir = {{0.0f, 1.0f, 0.0f}};
	dsMatrix44f camera;
	dsMatrix44f_lookAt(&camera, &eyePos, &lookAtPos, &upDir);
	dsView_setCameraMatrix(view, &camera);
	return view;
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
		NULL, width, height, dsWindowFlags_Resizeable | dsWindowFlags_DelaySurfaceCreate);
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

	testScene->resources = createSceneResources(renderer, allocator);
	if (!testScene->resources)
		return false;

	testScene->scene = createScene(renderer, allocator, testScene->resources);
	if (!testScene->scene)
		return false;

	testScene->view = createView(allocator, testScene->scene, testScene->window->surface);
	if (!testScene->view)
		return false;

	testScene->rotation = 0;

	return true;
}

static void shutdown(TestScene* testScene)
{
	dsSceneNode_freeRef((dsSceneNode*)testScene->primaryTransform);
	dsSceneNode_freeRef((dsSceneNode*)testScene->secondaryTransform);
	DS_VERIFY(dsView_destroy(testScene->view));
	dsScene_destroy(testScene->scene);
	dsSceneResources_freeRef(testScene->resources);
	DS_VERIFY(dsWindow_destroy(testScene->window));
}

int dsMain(int argc, const char** argv)
{
	dsRendererType rendererType = dsRendererType_Default;
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
		else
		{
			printf("Unknown option: %s\n", argv[i]);
			printHelp(argv[0]);
			return 1;
		}
	}

	DS_LOG_INFO_F("TestScene", "Render using %s", dsRenderBootstrap_rendererName(rendererType));
	DS_LOG_INFO("TestScene", "Press '1' to toggle anti-aliasing.");
	DS_LOG_INFO("TestScene", "Press '2' to toggle sub-scene.");

	dsSystemAllocator renderAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&renderAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator applicationAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&applicationAllocator, DS_ALLOCATOR_NO_LIMIT));
	dsSystemAllocator testSceneAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&testSceneAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsRendererOptions rendererOptions;
	dsRenderer_defaultOptions(&rendererOptions, "TestScene", 0);
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

	dsShaderVersion shaderVersions[] =
	{
		{DS_VK_RENDERER_ID, DS_ENCODE_VERSION(1, 0, 0)},
		{DS_MTL_RENDERER_ID, DS_ENCODE_VERSION(1, 1, 0)},
		{DS_GL_RENDERER_ID, DS_ENCODE_VERSION(1, 1, 0)},
		{DS_GL_RENDERER_ID, DS_ENCODE_VERSION(1, 5, 0)},
		{DS_GLES_RENDERER_ID, DS_ENCODE_VERSION(1, 0, 0)}
	};
	DS_VERIFY(dsRenderer_shaderVersionToString(shaderDir, DS_ARRAY_SIZE(shaderDir), renderer,
		dsRenderer_chooseShaderVersion(renderer, shaderVersions, DS_ARRAY_SIZE(shaderVersions))));

	dsApplication* application = dsSDLApplication_create((dsAllocator*)&applicationAllocator,
		renderer, argc, argv, "DeepSea", "TestScene");
	if (!application)
	{
		DS_LOG_ERROR_F("TestScene", "Couldn't create application: %s", dsErrorString(errno));
		dsRenderer_destroy(renderer);
		return 2;
	}

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

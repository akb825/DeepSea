/*
 * Copyright 2017 Aaron Barany
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
#include <DeepSea/Core/Memory/SystemAllocator.h>
#include <DeepSea/Core/Streams/Path.h>
#include <DeepSea/Core/Assert.h>
#include <DeepSea/Core/Error.h>
#include <DeepSea/Core/Log.h>
#include <DeepSea/Math/Core.h>
#include <DeepSea/Math/Matrix44.h>
#include <DeepSea/Render/Resources/DrawGeometry.h>
#include <DeepSea/Render/Resources/Framebuffer.h>
#include <DeepSea/Render/Resources/GfxBuffer.h>
#include <DeepSea/Render/Resources/GfxFormat.h>
#include <DeepSea/Render/Resources/Material.h>
#include <DeepSea/Render/Resources/MaterialDesc.h>
#include <DeepSea/Render/Resources/ResourceManager.h>
#include <DeepSea/Render/Resources/Shader.h>
#include <DeepSea/Render/Resources/ShaderModule.h>
#include <DeepSea/Render/Resources/Texture.h>
#include <DeepSea/Render/Resources/TextureData.h>
#include <DeepSea/Render/Resources/VertexFormat.h>
#include <DeepSea/Render/Renderer.h>
#include <DeepSea/Render/RenderPass.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum dsRenderType
{
	dsRenderType_OpenGL,
	dsRenderType_Count
} dsRenderType;

typedef struct TestCube
{
	dsAllocator* allocator;
	dsRenderer* renderer;
	dsWindow* window;
	dsFramebuffer* framebuffer;
	dsRenderPass* renderPass;
	dsShaderModule* shaderModule;
	dsMaterialDesc* materialDesc;
	dsMaterial* material;
	dsShader* shader;
	dsTexture* texture;
	dsGfxBuffer* drawBuffer;
	dsDrawGeometry* geometry;

	uint32_t modelViewProjectionElement;
	float rotation;
	dsMatrix44f view;
	dsMatrix44f projection;
} TestCube;

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

static char assetsDir[DS_PATH_MAX];
static const char* shaderDir;

typedef struct Vertex
{
	dsVector3f position;
	dsVector2f texCoord;
} Vertex;

static Vertex vertices[] =
{
	// Front face
	{ {{-1.0f,  1.0f,  1.0f}}, {{0.0f, 0.0f}} },
	{ {{ 1.0f,  1.0f,  1.0f}}, {{1.0f, 0.0f}} },
	{ {{ 1.0f, -1.0f,  1.0f}}, {{1.0f, 1.0f}} },
	{ {{-1.0f, -1.0f,  1.0f}}, {{0.0f, 1.0f}} },

	// Right face
	{ {{ 1.0f,  1.0f,  1.0f}}, {{0.0f, 0.0f}} },
	{ {{ 1.0f,  1.0f, -1.0f}}, {{1.0f, 0.0f}} },
	{ {{ 1.0f, -1.0f, -1.0f}}, {{1.0f, 1.0f}} },
	{ {{ 1.0f, -1.0f,  1.0f}}, {{0.0f, 1.0f}} },

	// Back face
	{ {{ 1.0f,  1.0f, -1.0f}}, {{0.0f, 0.0f}} },
	{ {{-1.0f,  1.0f, -1.0f}}, {{1.0f, 0.0f}} },
	{ {{-1.0f, -1.0f, -1.0f}}, {{1.0f, 1.0f}} },
	{ {{ 1.0f, -1.0f, -1.0f}}, {{0.0f, 1.0f}} },

	// Left face
	{ {{-1.0f,  1.0f, -1.0f}}, {{0.0f, 0.0f}} },
	{ {{-1.0f,  1.0f,  1.0f}}, {{1.0f, 0.0f}} },
	{ {{-1.0f, -1.0f,  1.0f}}, {{1.0f, 1.0f}} },
	{ {{-1.0f, -1.0f, -1.0f}}, {{0.0f, 1.0f}} },

	// Top face
	{ {{-1.0f,  1.0f, -1.0f}}, {{0.0f, 0.0f}} },
	{ {{ 1.0f,  1.0f, -1.0f}}, {{1.0f, 0.0f}} },
	{ {{ 1.0f,  1.0f,  1.0f}}, {{1.0f, 1.0f}} },
	{ {{-1.0f,  1.0f,  1.0f}}, {{0.0f, 1.0f}} },

	// Bottom face
	{ {{-1.0f, -1.0f,  1.0f}}, {{0.0f, 0.0f}} },
	{ {{ 1.0f, -1.0f,  1.0f}}, {{1.0f, 0.0f}} },
	{ {{ 1.0f, -1.0f, -1.0f}}, {{1.0f, 1.0f}} },
	{ {{-1.0f, -1.0f, -1.0f}}, {{0.0f, 1.0f}} },
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

typedef dsRenderer* (*CreateRendererFunction)(dsAllocator* allocator);
typedef void (*DestroyRendererFunction)(dsRenderer* renderer);
typedef const char* (*GetShaderDirFunction)(dsRenderer* renderer);

static void printHelp(const char* programPath)
{
	printf("usage: %s [OPTIONS]\n", dsPath_getFileName(programPath));
	printf("options:\n");
	printf("  -h, --help      print this help message and exit\n");
#if DS_HAS_OPENGL
	printf("      --opengl    render using OpenGL\n");
#endif
	printf("default renderer: %s\n", renderTypeNames[defaultRenderType]);
}

static bool validateAllocator(dsAllocator* allocator, const char* name)
{
	if (allocator->size == 0)
		return true;

	DS_LOG_ERROR_F("TestCube", "Allocator '%s' has %llu bytes allocated with %u allocations.",
		name, (unsigned long long)allocator->size, allocator->currentAllocations);
	return false;
}

static bool createFramebuffer(TestCube* testCube)
{
	uint32_t width, height;
	if (!dsWindow_getSize(&width, &height, testCube->window))
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't get window size: %s", dsErrorString(errno));
		return false;
	}

	if (testCube->framebuffer)
		dsFramebuffer_destroy(testCube->framebuffer);

	dsRenderSurface* surface = testCube->window->surface;
	dsFramebufferSurface surfaces[] =
	{
		{dsFramebufferSurfaceType_ColorRenderSurface, dsCubeFace_PosX, 0, 0, surface},
		{dsFramebufferSurfaceType_DepthRenderSurface, dsCubeFace_PosX, 0, 0, surface}
	};
	testCube->framebuffer = dsFramebuffer_create(testCube->renderer->resourceManager,
		testCube->allocator, surfaces, 2, width, height, 1);

	if (!testCube->framebuffer)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create framebuffer: %s", dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsRenderer_makePerspective(&testCube->projection, testCube->renderer,
		(float)dsDegreesToRadians(45.0f), (float)width/(float)height, 0.1f, 100.0f));

	return true;
}

static bool processEvent(dsApplication* application, dsWindow* window, const dsEvent* event,
	void* userData)
{
	DS_UNUSED(application);

	TestCube* testCube = (TestCube*)userData;
	DS_ASSERT(!window || window == testCube->window);
	switch (event->type)
	{
		case dsEventType_WindowClosed:
			DS_VERIFY(dsWindow_destroy(window));
			testCube->window = NULL;
			return false;
		case dsEventType_WindowResized:
			if (!createFramebuffer(testCube))
				abort();
			return true;
		default:
			return true;
	}
}

static void update(dsApplication* application, double lastFrameTime, void* userData)
{
	DS_UNUSED(application);

	TestCube* testCube = (TestCube*)userData;

	// radians/s
	const double rate = M_PI_2;
	testCube->rotation += (float)(lastFrameTime*rate);
	while (testCube->rotation > 2*M_PI)
		testCube->rotation = (float)(testCube->rotation - 2*M_PI);

	dsMatrix44f model;
	dsMatrix44f_makeRotate(&model, 0, testCube->rotation, 0);

	dsMatrix44f modelView, modelViewProjection;
	dsMatrix44_affineMul(modelView, testCube->view, model);
	dsMatrix44_mul(modelViewProjection, testCube->projection, modelView);
	DS_VERIFY(dsMaterial_setElementData(testCube->material, 0, &modelViewProjection,
		dsMaterialType_Mat4, 0,  1));
}

static void draw(dsApplication* application, dsWindow* window, void* userData)
{
	DS_UNUSED(application);
	DS_UNUSED(window);
	TestCube* testCube = (TestCube*)userData;
	DS_ASSERT(testCube->window == window);
	dsRenderer* renderer = testCube->renderer;
	dsCommandBuffer* commandBuffer = renderer->mainCommandBuffer;

	dsSurfaceClearValue clearValues[2];
	clearValues[0].colorValue.floatValue.r = 0.1f;
	clearValues[0].colorValue.floatValue.g = 0.2f;
	clearValues[0].colorValue.floatValue.b = 0.4f;
	clearValues[0].colorValue.floatValue.a = 1.0f;
	clearValues[1].depthStencil.depth = 1.0f;
	clearValues[1].depthStencil.stencil = 0;
	DS_VERIFY(dsRenderPass_begin(commandBuffer, testCube->renderPass, testCube->framebuffer, NULL,
		clearValues, 2, false));
	DS_VERIFY(dsShader_bind(commandBuffer, testCube->shader, testCube->material, NULL, NULL));

	dsDrawIndexedRange drawRange = {testCube->geometry->indexBuffer.count, 1, 0, 0, 0};
	DS_VERIFY(dsRenderer_drawIndexed(commandBuffer,renderer, testCube->geometry, &drawRange));

	DS_VERIFY(dsShader_unbind(commandBuffer, testCube->shader));
	DS_VERIFY(dsRenderPass_end(commandBuffer, testCube->renderPass));
}

static bool setup(TestCube* testCube, dsApplication* application, dsAllocator* allocator)
{
	dsRenderer* renderer = application->renderer;
	dsResourceManager* resourceManager = renderer->resourceManager;
	testCube->allocator = allocator;
	testCube->renderer = renderer;

	dsEventResponder responder = {&processEvent, testCube, 0, 0};
	DS_VERIFY(dsApplication_addEventResponder(application, &responder));
	DS_VERIFY(dsApplication_setUpdateFunction(application, &update, testCube));

	testCube->window = dsWindow_create(application, allocator, "Test Cube",
		NULL, 800, 600, dsWindowFlags_Resizeable);
	if (!testCube->window)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create window: %s", dsErrorString(errno));
		return false;
	}

	DS_VERIFY(dsWindow_setDrawFunction(testCube->window, &draw, testCube));

	if (!createFramebuffer(testCube))
		return false;

	dsAttachmentInfo attachments[] =
	{
		{dsAttachmentUsage_Clear, renderer->surfaceColorFormat, DS_DEFAULT_ANTIALIAS_SAMPLES},
		{dsAttachmentUsage_Clear, renderer->surfaceDepthStencilFormat, DS_DEFAULT_ANTIALIAS_SAMPLES}
	};

	dsColorAttachmentRef colorAttachment = {0, false};
	uint32_t depthStencilAttachment = 1;
	dsRenderSubpassInfo subpass =
	{
		"TestCube", NULL, &colorAttachment, 0, 1, depthStencilAttachment
	};
	testCube->renderPass = dsRenderPass_create(renderer, allocator, attachments, 2, &subpass, 1,
		NULL, 0);
	if (!testCube->renderPass)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create render pass: %s", dsErrorString(errno));
		return false;
	}

	DS_ASSERT(shaderDir);
	char path[DS_PATH_MAX];
	if (!dsPath_combine(path, sizeof(path), assetsDir, shaderDir) ||
		!dsPath_combine(path, sizeof(path), path, "TestCube.mslb"))
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create shader path: %s", dsErrorString(errno));
		return false;
	}

	testCube->shaderModule = dsShaderModule_loadFile(resourceManager, allocator, path, "TestCube");
	if (!testCube->shaderModule)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create shader path: %s", dsErrorString(errno));
		return false;
	}

	dsMaterialElement materialElems[] =
	{
		{"modelViewProjection", dsMaterialType_Mat4, 0, NULL, false, 0},
		{"tex", dsMaterialType_Texture, 0, NULL, false, 0}
	};
	testCube->materialDesc = dsMaterialDesc_create(resourceManager, allocator, materialElems,
		(uint32_t)DS_ARRAY_SIZE(materialElems));
	if (!testCube->materialDesc)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create material description: %s",
			dsErrorString(errno));
		return false;
	}

	testCube->material = dsMaterial_create(allocator, testCube->materialDesc);
	if (!testCube->material)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create material: %s", dsErrorString(errno));
		return false;
	}

	testCube->shader = dsShader_createName(resourceManager, allocator, testCube->shaderModule,
		"Default", testCube->materialDesc, dsPrimitiveType_TriangleList,
		DS_DEFAULT_ANTIALIAS_SAMPLES);
	if (!testCube->shader)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create shader: %s", dsErrorString(errno));
		return false;
	}

	if (!dsPath_combine(path, sizeof(path), assetsDir, "texture.pvr"))
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create texture path: %s", dsErrorString(errno));
		return false;
	}

	testCube->texture = dsTextureData_loadFileToTexture(resourceManager, allocator, NULL, path,
		NULL, dsTextureUsage_Texture, dsGfxMemory_Static | dsGfxMemory_GpuOnly);
	if (!testCube->texture)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't load texture: %s", dsErrorString(errno));
		return false;
	}

	uint32_t texElement = dsMaterialDesc_findElement(testCube->materialDesc, "tex");
	DS_ASSERT(texElement != DS_MATERIAL_UNKNOWN);
	DS_VERIFY(dsMaterial_setTexture(testCube->material, texElement, testCube->texture));

	uint8_t combinedBufferData[sizeof(vertices) + sizeof(indices)];
	memcpy(combinedBufferData, vertices, sizeof(vertices));
	memcpy(combinedBufferData + sizeof(vertices), indices, sizeof(indices));
	testCube->drawBuffer = dsGfxBuffer_create(resourceManager, allocator,
		dsGfxBufferUsage_Vertex | dsGfxBufferUsage_Index,
		dsGfxMemory_Static | dsGfxMemory_GpuOnly | dsGfxMemory_Draw, combinedBufferData,
		sizeof(combinedBufferData));
	if (!testCube->drawBuffer)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create graphics buffer: %s", dsErrorString(errno));
		return false;
	}

	dsVertexFormat vertexFormat;
	DS_VERIFY(dsVertexFormat_initialize(&vertexFormat));
	vertexFormat.elements[dsVertexAttrib_Position].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32Z32, dsGfxFormat_Float);
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_Position, true));
	vertexFormat.elements[dsVertexAttrib_TexCoord0].format =
		dsGfxFormat_decorate(dsGfxFormat_X32Y32, dsGfxFormat_Float);
	DS_VERIFY(dsVertexFormat_setAttribEnabled(&vertexFormat, dsVertexAttrib_TexCoord0, true));
	DS_VERIFY(dsVertexFormat_computeOffsetsAndSize(&vertexFormat));
	DS_ASSERT(vertexFormat.size == sizeof(Vertex));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_Position].offset == offsetof(Vertex, position));
	DS_ASSERT(vertexFormat.elements[dsVertexAttrib_TexCoord0].offset == offsetof(Vertex, texCoord));

	dsVertexBuffer vertexBuffer = {testCube->drawBuffer, 0, (uint32_t)DS_ARRAY_SIZE(vertices),
		vertexFormat};
	dsVertexBuffer* vertexBuffers[DS_MAX_GEOMETRY_VERTEX_BUFFERS] = {&vertexBuffer, NULL, NULL,
		NULL};
	dsIndexBuffer indexBuffer = {testCube->drawBuffer, sizeof(vertices),
		(uint32_t)DS_ARRAY_SIZE(indices), 16};
	testCube->geometry = dsDrawGeometry_create(resourceManager, allocator, vertexBuffers,
		&indexBuffer);
	if (!testCube->geometry)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create geometry: %s", dsErrorString(errno));
		return false;
	}

	testCube->modelViewProjectionElement = dsMaterialDesc_findElement(testCube->materialDesc,
		"modelViewProjection");
	DS_ASSERT(testCube->modelViewProjectionElement != DS_MATERIAL_UNKNOWN);
	testCube->rotation = 0;
	dsVector3f eyePos = {{0.0f, 5.0f, 5.0f}};
	dsVector3f lookAtPos = {{0.0f, 0.0f, 0.0f}};
	dsVector3f upDir = {{0.0f, 1.0f, 0.0f}};
	dsMatrix44f camera;
	dsMatrix44f_lookAt(&camera, &eyePos, &lookAtPos, &upDir);
	dsMatrix44f_affineInvert(&testCube->view, &camera);

	return true;
}

static void shutdown(TestCube* testCube)
{
	if (testCube->geometry)
		dsDrawGeometry_destroy(testCube->geometry);
	if (testCube->drawBuffer)
		dsGfxBuffer_destroy(testCube->drawBuffer);
	if (testCube->texture)
		dsTexture_destroy(testCube->texture);
	if (testCube->shader)
		dsShader_destroy(testCube->shader);
	if (testCube->material)
		dsMaterial_destroy(testCube->material);
	if (testCube->materialDesc)
		dsMaterialDesc_destroy(testCube->materialDesc);
	if (testCube->shaderModule)
		dsShaderModule_destroy(testCube->shaderModule);
	if (testCube->renderPass)
		dsRenderPass_destroy(testCube->renderPass);
	if (testCube->framebuffer)
		dsFramebuffer_destroy(testCube->framebuffer);
	if (testCube->window)
		dsWindow_destroy(testCube->window);
}

int main(int argc, const char** argv)
{
	dsRenderType renderType = defaultRenderType;
	for (int i = 1; i < argc; ++argv)
	{
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			printHelp(argv[0]);
			return 0;
		}
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
	DS_VERIFY(dsPath_combine(assetsDir, sizeof(assetsDir), assetsDir, "TestCube-assets"));

	DS_LOG_INFO_F("TestCube", "Render using %s", renderTypeNames[renderType]);

	CreateRendererFunction createRendererFunc = NULL;
	DestroyRendererFunction destroyRendererFunc = NULL;
	GetShaderDirFunction getShaderDirFunc = NULL;
	switch (renderType)
	{
#if DS_HAS_OPENGL
		case dsRenderType_OpenGL:
			createRendererFunc = &dsTestCube_createGLRenderer;
			destroyRendererFunc = &dsTestCube_destroyGLRenderer;
			getShaderDirFunc = &dsTestCube_getGLShaderDir;
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
	dsSystemAllocator testCubeAllocator;
	DS_VERIFY(dsSystemAllocator_initialize(&testCubeAllocator, DS_ALLOCATOR_NO_LIMIT));

	dsRenderer* renderer = createRendererFunc((dsAllocator*)&renderAllocator);
	if (!renderer)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create renderer: %s", dsErrorString(errno));
		return 2;
	}

	shaderDir = getShaderDirFunc(renderer);

	dsApplication* application = dsSDLApplication_create((dsAllocator*)&applicationAllocator,
		renderer);
	if (!application)
	{
		DS_LOG_ERROR_F("TestCube", "Couldn't create application: %s", dsErrorString(errno));
		destroyRendererFunc(renderer);
		return 2;
	}

	TestCube testCube;
	memset(&testCube, 0, sizeof(testCube));
	if (!setup(&testCube, application, (dsAllocator*)&testCubeAllocator))
	{
		shutdown(&testCube);
		return 3;
	}

	int exitCode = dsApplication_run(application);

	shutdown(&testCube);
	dsSDLApplication_destroy(application);
	destroyRendererFunc(renderer);

	if (!validateAllocator((dsAllocator*)&renderAllocator, "render"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&applicationAllocator, "application"))
		exitCode = 4;
	if (!validateAllocator((dsAllocator*)&testCubeAllocator, "TestCube"))
		exitCode = 4;

	return exitCode;
}
